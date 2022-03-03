/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#define NTHREADS 30
#define SBUFSIZE 200

typedef struct{
	int *buf; // buffer array
	int n; // maximum number of slots
	int front; // buf[front+1 % n] is first item
	int rear; // buf[rear % n] is last item
	sem_t mutex; // protects accesses to buf
	sem_t slots; // counts available slots
	sem_t items; // counts available items
}sbuf_t;

sbuf_t sbuf; // buffer

void sbuf_init(sbuf_t *sp, int n){
	sp->buf = Calloc(n, sizeof(int));
	sp->n = n; // buffer holds max of n items
	sp->front = sp->rear = 0; // empty buffer
	Sem_init(&sp->mutex, 0, 1); // binary semaphore for locking
	Sem_init(&sp->slots, 0, n);
	Sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t *sp){ // clean up buffer
	Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item){ // insert item onto the rear of shared buffer
	P(&sp->slots); // wait for available slot
	P(&sp->mutex); // lock buffer
	sp->buf[(++sp->rear)%(sp->n)] = item; // insert item
	V(&sp->mutex); // unlock buffer
	V(&sp->items); // announce available item
}

int sbuf_remove(sbuf_t *sp){ // remove and return the first item from buffer
	int item;
	P(&sp->items); // wait for available item
	P(&sp->mutex); // lock buffer
	item = sp->buf[(++sp->front)%(sp->n)]; // remove item
	V(&sp->mutex); // unlock buffer
	V(&sp->slots); // announce available slot
	return item;
}

int byte_cnt = 0; // number of total byte
char str[MAXLINE]; // input string

typedef struct __item{
	int ID;  // stock id
	int left_stock; // number of left stock
	int price; // stock price
	int readcnt; // number of readers currently in the critical section
	sem_t mutex; // protects access to the shared readcnt variable
	sem_t w; // controls access to the critical sections that access shared object
	struct __item *left;
	struct __item *right;
}item;

item *root; // root of binary tree

item* insert(item* root, int ID, int left_stock, int price){ // insert node to binary tree
	if(root == NULL){
		root = (item*)malloc(sizeof(item)); // allocate memory
		root->right = root->left = NULL; // copy values to new node
		root->ID = ID;
		root->left_stock = left_stock;
		root->price = price;
		Sem_init(&(root->mutex), 0, 1);
		Sem_init(&(root->w), 0, 1);
		return root;
	}
	else{ // find the position for new node
		if(ID < root->ID)
			root->left = insert(root->left, ID, left_stock, price);
		else
			root->right = insert(root->right, ID, left_stock, price);
	}
	return root;
}

item* fMin(item* root){ // find position of leaf node in the tree
	item* min = root;
	while(min->left != NULL)
		min = min->left;
	return min;
}

void freenode(item* r){ // delete nodes from tree
	if(r == NULL)return;
	freenode(r->left);
	freenode(r->right);
	free(r);
	return;
}

void print(item* root){ // print contents of node
	if(root == NULL) return;
	printf("ID : %d -> %d %d\n", root->ID, root->left_stock, root->price);
	print(root->left);
	print(root->right);
}

void preorderPrint(item* root){ // print node preorder
	if(root == NULL) return;
	printf("%d %d %d\n", root->ID, root->left_stock, root->price);
	print(root->left);
	print(root->right);
}

void strsave(item* root, FILE* fp){ // rewrite stock file at the end of program
	if(root == NULL) return;
	fprintf(fp,"%d %d %d\n", root->ID, root->left_stock, root->price);
	strsave(root->left, fp);
	strsave(root->right, fp);
	return;
}

void preorderStr(item* root){ // concatenate information of node
	char tmp[15];

	if(root == NULL) return;
	
	P(&(root->mutex));
	root->readcnt++;
	if((root->readcnt) == 1)P(&(root->w));
	V(&(root->mutex));

	sprintf(tmp,"%d", root->ID);
	strcat(str, tmp);
	strcat(str, " ");

	sprintf(tmp,"%d", root->left_stock);
	strcat(str, tmp);
	strcat(str, " ");
	
	sprintf(tmp,"%d", root->price);
	strcat(str, tmp);
	strcat(str, " ");
	
	P(&(root->mutex));
	root->readcnt--;
	if((root->readcnt) == 0)V(&(root->w));
	V(&(root->mutex));

	preorderStr(root->left);
	preorderStr(root->right);

	return;
}

item *find_id(item *root, int id){ // find specific id from tree
	item *node = root;
	while(node){
		if(id == node->ID){
			P(&(node->mutex)); // protect access for readcnt
			node->readcnt++; // increase the number of readers
			if((node->readcnt) == 1)P(&(node->w)); // protect access for left_stock
			V(&(node->mutex)); // unlock access for readcnt
			return node;
		}
		else if(id < node->ID)node = node->left;

		else node = node->right;
	}
	return NULL;
}

void echo(int connfd);

void stock_func(int connfd){ // interaction with clients
	int n;
	char buf[MAXLINE];
	rio_t rio;
	
	Rio_readinitb(&rio, connfd);
	while((n = Rio_readlineb(&rio, buf, MAXLINE))!=0){ // read text line
		byte_cnt+=n;
		printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
			
		if(!strcmp(buf, "show\n")){ //show
			int num = 0;
			for(int z = 0 ; z < MAXLINE ; z++)str[z] = '\0';
			preorderStr(root); // make string of stock information
			num = strlen(str);
			str[num] = '\n';
			str[num+1] = '\0';
			num++;
			Rio_writen(connfd, str, num);
		}
		else if(!strncmp(buf, "buy", 3)){ // buy
			int buyid, buynum;
			buf[strlen(buf) - 1] = '\0';
			for(int z = 0 ; z < 3 ; z++)buf[z] = ' ';
			sscanf(buf, "%d %d",&buyid, &buynum);
			item *snode = find_id(root, buyid); // find node from binary tree
			if(snode == NULL){ // there is no node
				strcpy(buf, "Invalid stock ID\n\0");
				Rio_writen(connfd, buf, strlen(buf));
			}
					
			else if(snode->left_stock < buynum){ // there is not enough stock
				P(&(snode->mutex)); // protect access for readcnt
				snode->readcnt--; // decrease the number of readers
				if((snode->readcnt) == 0)V(&(snode->w)); // unlock access for left_stock
				V(&(snode->mutex)); // unlock access for readcnt
				strcpy(buf, "Not enough left stock\n\0");
				Rio_writen(connfd, buf, strlen(buf));
			}
			else{ // there is enough stock
				P(&(snode->mutex)); // protect access for readcnt
				snode->readcnt--; // decrease the number of readers
				if((snode->readcnt) == 0)V(&(snode->w)); // unlock access for left_stock
				V(&(snode->mutex)); // unlock access for readcnt
				
				P(&(snode->w)); // protect access for left_stock
				snode->left_stock -= buynum;
				V(&(snode->w)); // unlock access for left_stock
				strcpy(buf, "[buy] success\n\0");
				Rio_writen(connfd, buf, strlen(buf));
			}
					
		}
		else if(!strncmp(buf, "sell", 4)){ // sell
			int sellid, sellnum;
			buf[strlen(buf) - 1] = '\0';
			for(int z = 0 ; z < 4 ; z++)buf[z] = ' ';
			sscanf(buf, "%d %d",&sellid, &sellnum);
			item *snode = find_id(root, sellid); // find node from binary tree
			if(snode == NULL){ // there is no node
				strcpy(buf, "Invalid stock ID\n\0");
				Rio_writen(connfd, buf, strlen(buf));
			}
			else{
				P(&(snode->mutex)); // protect access for readcnt
				snode->readcnt--; // decrease the number of readers
				if((snode->readcnt) == 0)V(&(snode->w)); // unlock access for left_stock
				V(&(snode->mutex)); // unlock access for readcnt
				
				P(&(snode->w)); // protect access for left_stock
				snode->left_stock += sellnum;
				V(&(snode->w)); // unlock access for left_stock
				strcpy(buf, "[sell] success\n\0");
				Rio_writen(connfd, buf, strlen(buf));
			}
		}
		else{ // other commands
			Rio_writen(connfd, buf, n);
		}
	}

}

void *thread(void *vargp){
	Pthread_detach(pthread_self());
	while(1){
		int connfd = sbuf_remove(&sbuf); // remove connfd from buffer
		stock_func(connfd); // service client
		Close(connfd);
	}
}

void sigint_handler(int signo){ // signal handler for sigint
	printf("Terminate server\n");
	int r = remove("stock.txt"); // remove file
	if(r == -1) printf("Error : fail to remove stock.txt\n");
	else{ // create stock.txt file
		FILE *fp;
		fp = fopen("stock.txt", "w");
		strsave(root,fp);
		fclose(fp);
	}
	freenode(root);
	exit(1);
}

int main(int argc, char **argv) 
{
	Signal(SIGINT, sigint_handler); // check signal like ctrl + c
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
	char client_hostname[MAXLINE], client_port[MAXLINE];
	pthread_t tid;
	int i;

	FILE *fp = NULL;
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	
	fp = fopen("stock.txt", "r");
	if(fp == NULL){
		printf("Error : stock.txt file does not exist.\n");
		return 0;
	}
	else{ // move information of stock from disk to memory
		int stid, leftst, stprice;
		while(EOF!=fscanf(fp, "%d %d %d\n", &stid, &leftst, &stprice)){
			root = insert(root, stid, leftst, stprice);
		}
		fclose(fp);
	}
	
	printf("If you want to terminate server, press Ctrl + C\n");
	
	listenfd = Open_listenfd(argv[1]);
	
	sbuf_init(&sbuf, SBUFSIZE); // initialize sbuf
	// create the set of worker threads
	for(i = 0 ; i < NTHREADS ; i++)Pthread_create(&tid, NULL, thread, NULL);

	while (1) {
		clientlen = sizeof(struct sockaddr_storage); 
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", client_hostname, client_port);
		sbuf_insert(&sbuf, connfd); // insert connfd in buffer
	}
	exit(0);
}
/* $end echoserverimain */
