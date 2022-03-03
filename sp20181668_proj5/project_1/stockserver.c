/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

typedef struct{ // represents a pool of connected descriptors
	int maxfd; // largest descriptor in read_set
	fd_set read_set; // set of all active descriptors
	fd_set ready_set; // subset of descriptors ready for reading
	int nready; // number of ready descriptors from select
	int maxi; // high water index into client array
	int clientfd[FD_SETSIZE]; // set of active descriptors
	rio_t clientrio[FD_SETSIZE]; // set of active read buffers
}pool;

int byte_cnt = 0; // number of total byte
char str[MAXLINE]; // input string

typedef struct __item{
	int ID;  // stock id
	int left_stock; // number of left stock
	int price; // stock price
	int readcnt; // no use in select server
	sem_t mutex; // no use in select server
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
	sprintf(tmp,"%d", root->ID);
	strcat(str, tmp);
	strcat(str, " ");

	sprintf(tmp,"%d", root->left_stock);
	strcat(str, tmp);
	strcat(str, " ");
	
	sprintf(tmp,"%d", root->price);
	strcat(str, tmp);
	strcat(str, " ");
	
	preorderStr(root->left);
	preorderStr(root->right);

	return;
}

item *find_id(item *root, int id){ // find specific id from tree
	item *node = root;
	while(node){
		if(id == node->ID){
			return node;
		}

		else if(id < node->ID)node = node->left;

		else node = node->right;
	}
	return NULL;
}

void echo(int connfd);

void init_pool(int listenfd, pool *p){ // initialize pool
	int i;
	p->maxi = -1;
	for(i = 0 ; i < FD_SETSIZE ; i++)
		p->clientfd[i] = -1;

	p->maxfd = listenfd;
	FD_ZERO(&p->read_set);
	FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p){ // add client to the pool
	int i;
	p->nready--;
	for(i = 0 ; i < FD_SETSIZE ; i++)
		if(p->clientfd[i] < 0){
			p->clientfd[i] = connfd;
			Rio_readinitb(&p->clientrio[i], connfd);
			FD_SET(connfd, &p->read_set);
			if(connfd > p->maxfd)p->maxfd = connfd;
			if(i > p->maxi)p->maxi = i;
			break;
		}
	if(i == FD_SETSIZE)
		app_error("add_client error : Too manty clients");

}

void check_clients(pool *p){ // interaction with ready descriptor
	int i, connfd, n;
	char buf[MAXLINE];
	rio_t rio;

	for(i = 0 ; (i<=p->maxi) && (p->nready > 0);i++){
		connfd = p->clientfd[i];
		rio = p->clientrio[i];
		if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))){ // check ready descriptor
			p->nready--;
			if((n=Rio_readlineb(&rio, buf, MAXLINE))!=0){ // read text line
				byte_cnt += n;
				printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
			
				if(!strcmp(buf, "show\n")){ // show
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
						strcpy(buf, "Not enough left stock\n\0");
						Rio_writen(connfd, buf, strlen(buf));
					}
					else{ // there is enough stock
						snode->left_stock -= buynum;
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
						snode->left_stock += sellnum;
						strcpy(buf, "[sell] success\n\0");
						Rio_writen(connfd, buf, strlen(buf));
					}
				}
				else{ // other commands
					Rio_writen(connfd, buf, n);
				}
			}
			else{
				Close(connfd);
				FD_CLR(connfd, &p->read_set);
				p->clientfd[i] = -1;
			}
		}
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
	static pool pool;

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
	init_pool(listenfd, &pool);

	while (1) {
		pool.ready_set = pool.read_set;
		pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);	
		if(FD_ISSET(listenfd, &pool.ready_set)){ // add client to the pool
			clientlen = sizeof(struct sockaddr_storage);
			connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
			Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
			printf("Connected to (%s, %s)\n", client_hostname, client_port);
			add_client(connfd, &pool);
		}		
		check_clients(&pool);
	}
	exit(0);
}
/* $end echoserverimain */
