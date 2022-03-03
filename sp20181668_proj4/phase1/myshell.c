/*
system programming project 4
phase 1 building and testing your shell
20181668 yejin lee
copyright 2021. yejinlee. All rights reserved.
*/

/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int *argc);
int builtin_command(char **argv, char argc);

int main() 
{
	char cmdline[MAXLINE]; // Command line 
	
	do{
		// Read 
		printf("CSE4100-SP-P4> ");                   
		char *tmp = fgets(cmdline, MAXLINE, stdin); 
		if (feof(stdin))
			exit(0);

		// Evaluate 
		eval(cmdline);
	}while(1);
	
	return 0;
}
/* $end shellmain */
  
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

	// save index and count the number if there is symbol " or ' or `
	for(i = 0 ; i < strlen(cmdline) ; i++){ 
		if(cmdline[i] == 34){ // symbol  "
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
	
	// remove special characters ``
	if(bflg == 2){
		cmdline[bidx[0]] = ' ';
		cmdline[bidx[1]] = ' ';
	}
    
	strcpy(buf, cmdline); // copy cmdline to buffer
	bg = parseline(buf, argv, &argc); // tokenize command in buffer

	// case that command is like "echo `pwd`"
	// remove arguments except first
	if((bflg == 2) && (!strcmp(argv[0], "echo"))){
		strcpy(argv[0], argv[1]);
		for(i = 1 ; i < argc ; i++){
			argv[i] = 0;
		}
	}
	
	if (argv[0] == NULL)return;   /* Ignore empty lines */
    
	if (!builtin_command(argv, argc)) { //quit -> exit(0), & -> ignore, other -> run
		char bpath[50] = "/bin/";
		strcat(bpath, argv[0]); // convert command to path string
		
		if((pid = Fork()) == 0){ // child process
			if (execve(bpath, argv, environ) < 0) {	//ex) /bin/ls ls -al &
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


