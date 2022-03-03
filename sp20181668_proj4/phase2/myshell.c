/*
system programming project 4
phase 2 redirection and piping in your shell
20181668 yejin lee
copyright 2021. yejinlee. All rights reserved.
*/

/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

// global variable
int is_pipe;

/* functions for pipe command*/
void pipetoken(char* str[], char *cmd1[], char *cmd2[]);
void cmd_pipe(char* cmd1[], char* cmd2[]);
void sig_chldhandler(int sig);

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int *argc);
int builtin_command(char **argv, char argc);

int main() 
{
	char cmdline[MAXLINE]; // Command line
	
	// signal handler for signal (SIGCHLD)
	Signal(SIGCHLD, sig_chldhandler);	
	
	do{
		// Read 
		printf("CSE4100-SP-P4> ");                   
		char *tmp = fgets(cmdline, MAXLINE, stdin); 
		if (feof(stdin))
			exit(0);

		is_pipe = 0; // initialize pipe flag
		
		// Evaluate 
		eval(cmdline);
	}while(1);
	
	return 0;
}
/* $end shellmain */

void pipetoken(char* str[], char *cmd1[], char *cmd2[]){ // tokenize string based on pipe symbol
	int i, idx;
	
	for(i = 0 ; str[i] != 0 ; i++){
		if(!strcmp(str[i],"|")){ // save the index if there is pipe symbol
			idx = i;
			break;
		}
		cmd1[i] = str[i]; // copy string to first string array
		cmd1[i+1] = 0;
	}

	for(i = idx + 1 ; str[i] != 0 ; i++){ // copy string to second string array
		cmd2[i - idx - 1] = str[i];
	}
	cmd2[i - idx - 1] = 0;

	return;
}

void cmd_pipe(char* cmd1[], char* cmd2[]){ // execute command with pipe
	int fd[2];
	char* tmp1[20] = {0};
	char* tmp2[20] = {0};

	if(pipe(fd) == -1){ // terminate program if pipe creation is fail
		perror("Fail to create pipe");
		exit(1);
	}
	
	if(Fork() == 0){ // child process
		dup2(fd[1], 1); // replace stdout(1) with fd[1] 
		close(fd[0]);
		//close(fd[1]);
		// result of command is written in fd
		execvp(cmd1[0], cmd1);
		fprintf(stderr, "Error : 1st execvp failed %s\n", strerror(errno));
		exit(1);
	}
	
	else{
		dup2(fd[0], 0); // replace stdin(0) with fd[0]
		//close(fd[0]);
		close(fd[1]);
		// tokenize next command
		pipetoken(cmd2, tmp1, tmp2);
		// repeat if there are any commands left
		if(tmp2[0] != 0) cmd_pipe(tmp1, tmp2);
		else{
			execvp(cmd2[0], cmd2);
			fprintf(stderr, "Error : 2nd execvp failed %s\n", strerror(errno));
			exit(1);			
		}
	}
	return;
}

void sig_chldhandler(int sig){
	int status;
	pid_t id = waitpid(-1, &status, WNOHANG);
	return;
}
 
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	pid_t pid;           /* Process id */
	int argc = 0; // the number of arguments
	int i;
	int didx[2], sidx[2], bidx[2], dflg = 0, sflg = 0, bflg = 0;

	// set flag if there is pipe symbol in the command
	// save index and count the number if there is symbol " or ' or `
	for(i = 0 ; i < strlen(cmdline) ; i++){ 
		if(cmdline[i] == '|'){
			is_pipe = 1;
		}
		else if(cmdline[i] == 34){ // symbol  "
			didx[dflg++] = i;
		}
		else if(cmdline[i] == 39){ // symbol '
			sidx[sflg++] = i;
		}
		else if(cmdline[i] == 96){ // symbol `
			bidx[bflg++] = i;
		}
		
	}
	
	// remove special characters ""
	if(dflg == 2){
		cmdline[didx[0]] = ' ';
		cmdline[didx[1]] = ' ';
	}

	// remove special characters ''
	if(sflg == 2){
		cmdline[sidx[0]] = ' ';
		cmdline[sidx[1]] = ' ';
	}
	
	// use command execution results as arguments if command has `command` form
	if(bflg == 2){
		is_pipe = 1;
		cmdline[bidx[0]] = '|';
		cmdline[bidx[1]] = ' ';
		for(i = bidx[1] - 1; i > bidx[0] ; i--){
			cmdline[i + 1] = cmdline[i];
		}
		cmdline[bidx[0] + 1] = ' ';
	}
	
	strcpy(buf, cmdline); // copy cmdline to buffer
	bg = parseline(buf, argv, &argc); // tokenize command in buffer
	
	if (argv[0] == NULL)return;   /* Ignore empty lines */
    
	if (!builtin_command(argv, argc)) { //quit -> exit(0), & -> ignore, other -> run
		char bpath[50] = "/bin/";
		strcat(bpath, argv[0]); // convert command to path string
		
		if((pid = Fork()) == 0){ // child process
			if(is_pipe == 1){ // pipe symbol is in command
				char *tmp1[20] = {0};
				char *tmp2[20] = {0};
				pipetoken(argv, tmp1, tmp2); // tokenize command based on pipe symbol
				cmd_pipe(tmp1, tmp2);
			}
			else if (execve(bpath, argv, environ) < 0) {	//ex) /bin/ls ls -al &
				printf("%s: Command not found.\n", argv[0]);
				exit(0);
			}
		}

		else{ // parent process
			/* Parent waits for foreground job to terminate */
			if (!bg){ 
				int status;
				Wait(&status);
			}
			else//when there is backgrount process!
				printf("%d %s", pid, cmdline);
		}
	}
	
	return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv, char argc) 
{
	if (!strcmp(argv[0], "quit")) /* quit command */
		exit(0);
		
	if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
		return 1;
		
	if(!strcmp(argv[0], "cd")){ // command cd
		char *dpath; // directory path
		if(argc > 1)dpath = argv[1]; // save argument as path
		else if(argc == 1){
			if((dpath = (char*)getenv("HOME")) == NULL) // if HOME env value does not exist
				dpath = ".";
		}
		
		if(chdir(dpath) != 0){ // fail to change directory
			printf("Error : Invalid path : %s\n", dpath);
		}		
		return 1;
	}

	if(!strcmp(argv[0], "exit")) // command exit
		exit(0);

	return 0; /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv, int *argc) 
{
	char *delim;         /* Points to first space delimiter */
	int bg;              /* Background job? */

	buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* Ignore leading spaces */
		buf++;

	/* Build the argv list */
	*argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[(*argc)++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
	}
	argv[(*argc)] = NULL;
    
	if ((*argc) == 0)  /* Ignore blank line */
		return 1;

	/* Should the job run in the background? */
	if ((bg = (*argv[(*argc)-1] == '&')) != 0)
		argv[--(*argc)] = NULL;

	return bg;
}
/* $end parseline */


