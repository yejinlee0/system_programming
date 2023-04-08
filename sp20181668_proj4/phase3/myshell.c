/*
system programming project 4
phase 3 run processes in background in your shell
20181668 yejin 
copyright 2021. yejinlee. All rights reserved.
*/

/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

// global variable
int is_pipe;
int jidx = 1; // indicate next job id
int lflag = 0; // less command flag

// structure
typedef struct job_t {
	pid_t pid; // process id
	int jobid; // job id
	char state; // f : fg, b : bg, s : stop
	char cmdline[100]; // command
}Job_t;

Job_t jobs[20]; // array for jobs

/* functions for pipe command*/
void pipetoken(char* str[], char *cmd1[], char *cmd2[]);
void cmd_pipe(char* cmd1[], char* cmd2[]);

// wait function
void wait_fg(pid_t pid);

/* functions for signal handler */
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

/* functions for job control */
void insertjob(pid_t pid, char state, char* cmdline);
void deletejob(pid_t pid);
int find_jidx(); 
int pidtojid(pid_t pid);
Job_t *jidtojob(int jid);
Job_t *pidtojob(pid_t pid);
pid_t find_fgpid();
void cmd_jobs();
void cmd_bg(char **argv);
void cmd_fg(char **argv);

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int *argc);
int builtin_command(char **argv, char argc);

int main() 
{
	char cmdline[MAXLINE]; // Command line
	
	Signal(SIGCHLD, sigchld_handler); // set handler for SIGCHLD	
	Signal(SIGINT, sigint_handler); // set handler for SIGINT
	Signal(SIGTSTP, sigtstp_handler); // set handler for SIGTSTP
	
	// initialize job array
	for(int i = 0 ; i < 20 ; i++){
		jobs[i].pid = 0;
		jobs[i].jobid = 0;
		jobs[i].state = 0;
		jobs[i].cmdline[0] = '\0';		
	}
	
	do{
		// Read 
		printf("CSE4100-SP-P4> ");                   
		char *tmp = fgets(cmdline, MAXLINE, stdin); 
		if(feof(stdin))exit(0);

		is_pipe = 0; // initialize pipe flag
		lflag = 0; // initialize less flag
		
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
	sigset_t mask;
	
	if(pipe(fd) == -1){ // terminate program if pipe creation is fail
		perror("Fail to create pipe");
		exit(1);
	}
	
	if(sigemptyset(&mask)==-1)unix_error("sigemptyset error");
	if(sigaddset(&mask, SIGCHLD)==-1)unix_error("sigaddset error");
	if(sigprocmask(SIG_BLOCK, &mask, NULL)==-1)unix_error("sigprocmask error");	
	
	if(Fork() == 0){ // child process
		if(sigprocmask(SIG_UNBLOCK, &mask, NULL)==-1)unix_error("sigprocmask error");
		if(setpgid(0,0) < 0)unix_error("setpgid error");
		dup2(fd[1], 1); // replace stdout(1) with fd[1] 
		close(fd[0]);

		// result of command is written in fd
		execvp(cmd1[0], cmd1);
		fprintf(stderr, "Error : 1st execvp failed %s\n", strerror(errno));
		exit(1);
	}
	
	else{
		if(sigprocmask(SIG_UNBLOCK, &mask, NULL)==-1)unix_error("sigprocmask error");
		dup2(fd[0], 0); // replace stdin(0) with fd[0]

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

void wait_fg(pid_t pid){ // wait until process is not the foreground process
	Job_t* jtmp = pidtojob(pid); // find job using pid
	
	if(!jtmp) return; // return if there is no job
	// check state and wait until it is not foreground process
	while(jtmp -> pid == pid && jtmp -> state == 'f')
		sleep(1);

	return;	
}

/* begin siganl handlers */
void sigchld_handler(int sig){ // for SIGCHLD signal
	int status;
	pid_t pid;

	// wait until child process is terminated
	while(1){
		if((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) <= 0)break;
		// find job using pid which is terminated
		Job_t* curjob = pidtojob(pid);
		
		if(WIFSTOPPED(status)){ // child process is stopped
			curjob->state = 's';
			int jid = pidtojid(pid);
			printf("Job [%d] %d stopped by signal\n", jid, pid);
		}
		else if(WIFSIGNALED(status)){ // child process is terminated
			int jid = pidtojid(pid);
			printf("Job [%d] %d terminated by signal\n", jid, pid);
			deletejob(pid); // delete job
		}
		else if(WIFEXITED(status)){ // process is terminated normally
			deletejob(pid); // delete job
		}
	}

	return;
}

void sigint_handler(int sig){ // for SIGINT signal
	pid_t pid = find_fgpid();
	
	Kill(-pid, SIGINT); // send SIGINT signal to pid (process group)

	
	return;
}

void sigtstp_handler(int sig){ // for SIGTSTP signal
	pid_t pid = find_fgpid();

	if(pid != 0){ // change state of job if pid is in job array
		Job_t* tmp = pidtojob(pid);
		if(tmp)tmp->state = 's';
	}
	
	Kill(-pid, SIGTSTP); // send SIGTSTP signal to pid (process group)
	
	return;
}
/* end siganl handlers */

/* begin job control functions */
void insertjob(pid_t pid, char state, char* cmdline){ // insert new job
	int i;

	if(pid >= 1){
		for(i = 0 ; i < 20 ; i++){
			// add information to empty array element
			if(jobs[i].pid == 0){
				jobs[i].pid = pid;		
				jobs[i].jobid = jidx;
				jidx += 1;
				// index is 1 if array is full
				if(jidx > 20) jidx = 1;
				jobs[i].state = state;
				strcpy(jobs[i].cmdline, cmdline);
	
				return;
			}
		}	
	}
	
	return;
}

void deletejob(pid_t pid){ // delete job
	int i;
	
	if(pid >= 1){	
		for(i = 0 ; i < 20 ; i++){
			// delete job if job has same pid with parameter
			if(jobs[i].pid == pid){
				jobs[i].pid = 0;
				jobs[i].jobid = 0;
				jobs[i].state = 0;
				jobs[i].cmdline[0] = '\0';	
				// find job index and update it
				jidx = find_jidx() + 1;

				return;
			}
		}
	}
	
	return;	
}

int find_jidx(){ // find max job id index from job array
	int i, max = 0;
	for(i = 0 ; i < 20 ; i++)
		if(jobs[i].jobid > max) // save the value if job id is max
			max = jobs[i].jobid;
		
	return max;
}

int pidtojid(pid_t pid){ // change pid to job id
	int i;
	
	if(pid < 1) return 0;

	for(i = 0 ; i < 20 ; i++)
		if(jobs[i].pid == pid) // return job id if find matching pid(parameter)
			return jobs[i].jobid;

	return 0;	
}

Job_t *jidtojob(int jid){ // find job using job id
	int i;
	if(jid < 1) return NULL;

	for(i = 0 ; i < 20 ; i++)
		if(jobs[i].jobid == jid) // return job pointer if find matching job id(parameter)
			return &jobs[i];

	return NULL;	
}

Job_t *pidtojob(pid_t pid){ // find job using pid
	int i;
	if(pid < 1)return NULL;

	for(i = 0 ; i < 20 ; i++)
		if(jobs[i].pid == pid) // return job pointer if find matchinig pid(parameter)
			return &jobs[i];

	return NULL;
}

pid_t find_fgpid(){ // find pid that is in foreground
	int i;

	for(i = 0 ; i < 20 ; i++) 
		if(jobs[i].state == 'f') // return pid if state of job is 'f'
			return jobs[i].pid;

	return 0;	
}

void cmd_jobs(){ // print job list if the command is "jobs"
	int i;

	for(i = 0 ; i < 20 ; i++){
		if(jobs[i].pid != 0){ // case that pid exist
			printf("[%d] %d ", jobs[i].jobid, jobs[i].pid); // print job id and pid
			if(jobs[i].state == 'f'){ // if job is in foreground
				printf("Foreground ");
			}
			else if(jobs[i].state == 'b'){ // if job is in background
				printf("Running ");
			}
			else if(jobs[i].state == 's'){ // if job is stopped
				printf("Stopped ");
			}
			else{ // error
				printf("listjobs error job : %d, state: %c ", i, jobs[i].state);
			}
			printf("%s", jobs[i].cmdline); // print command
		}
	}	
}

void cmd_bg(char **argv){ // send a signal if the command is "bg"
	Job_t *cur_job;
	char* id = argv[1];

	if(id == NULL)return;

	if(id[0] == '%'){ // if argument is job id (for example %1)
		int jid = atoi(&id[1]); // change string to integer value
		if((cur_job = jidtojob(jid)) == 0)return; // case that there is no matching job
	}

	else if(isdigit(id[0])){ // if argument is pid (for example 12345)
		pid_t pid = atoi(id); // change string to integer value
		if((cur_job = pidtojob(pid)) == 0)return; // case that there is no matching job
	}

	else return;
	
	printf("[%d] %d %s", cur_job->jobid, cur_job->pid, cur_job->cmdline); // print job information
	cur_job->state = 'b'; // change that state of job
	Kill(-(cur_job->pid), SIGCONT); // send SIGCONT to pid(process group)
		
	return;
}

void cmd_fg(char **argv){ // send a signal if the command is "fg"
	Job_t *cur_job;
	char* id = argv[1];

	if(id == NULL)return;

	if(id[0] == '%'){ // if argument is job id (for example %1)
		int jid = atoi(&id[1]); // change string to integer value
		if((cur_job = jidtojob(jid)) == 0)return; // case that there is no matching job
	}

	else if(isdigit(id[0])){ // if argument is pid (for example 12345)
		pid_t pid = atoi(id); // change string to integer value
		if((cur_job = pidtojob(pid)) == 0)return; // case that there is no matching job
	}

	else return;
	
	cur_job->state = 'f'; // change that state of job
	Kill(-(cur_job->pid), SIGCONT); // send SIGCONT to pid(process group)
	wait_fg(cur_job->pid); // wait for termination of job

	return;	
}
/* end job control functions */

 
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	pid_t pid;           /* Process id */
	int argc = 0; // the number of arguments
	int i, len;
	int didx[2], sidx[2], bidx[2], dflg = 0, sflg = 0, bflg = 0;
	sigset_t mask; /* signal set for blocking signal */
	
	strcpy(buf, cmdline); // copy cmdline to buffer
	
	// set flag if there is pipe symbol in the command
	// save index and count the number if there is symbol " or ' or `
	for(i = 0 ; i < strlen(buf) ; i++){ 
		if(buf[i] == '|'){
			is_pipe = 1;
		}
		else if(buf[i] == 34){ // symbol  "
			didx[dflg++] = i;
		}
		else if(buf[i] == 39){ // symbol '
			sidx[sflg++] = i;
		}
		else if(buf[i] == 96){ // symbol `
			bidx[bflg++] = i;
		}
		
	}
	
	// remove special characters ""
	if(dflg == 2){
		buf[didx[0]] = ' ';
		buf[didx[1]] = ' ';
	}

	// remove special characters ''
	if(sflg == 2){
		buf[sidx[0]] = ' ';
		buf[sidx[1]] = ' ';
	}
	
	// use command execution results as arguments if command has `command` form
	if(bflg == 2){
		is_pipe = 1;
		buf[bidx[0]] = '|';
		buf[bidx[1]] = ' ';
		for(i = bidx[1] - 1; i > bidx[0] ; i--){
			buf[i + 1] = buf[i];
		}
		buf[bidx[0] + 1] = ' ';
	}	
	
	bg = parseline(buf, argv, &argc); // tokenize command in buffer

	for(i = 0 ; i < argc ; i++){
		if(!strcmp(argv[i], "less")){
			lflag = 1;
			break;
		}
	}
	
	len = strlen(argv[argc-1]);
	// if there is no space betweem command and &
	// example : "command&"
	if(argv[argc-1][len-1] == '&'){ 
		argv[argc-1][len-1] = '\0';
		bg = 1;
	}
	
	if (argv[0] == NULL)return;   /* Ignore empty lines */
    
	if (!builtin_command(argv, argc)) { //quit -> exit(0), & -> ignore, other -> run
		char bpath[50] = "/bin/";
		strcat(bpath, argv[0]); // convert command to path string
		
		if(sigemptyset(&mask)==-1)unix_error("sigemptyset error"); // initialize mask
		if(sigaddset(&mask, SIGCHLD)==-1)unix_error("sigaddset error"); // add signal to mask
		if(sigprocmask(SIG_BLOCK, &mask, NULL)==-1)unix_error("sigprocmask error"); // block signal
		
		if((pid = Fork()) == 0){ // child process
			if(sigprocmask(SIG_UNBLOCK, &mask, NULL)==-1)unix_error("sigprocmask error"); // unblock signal
			if((lflag == 0)||(lflag == 1 && bg == 1))
				if(setpgid(0,0) < 0)unix_error("setpgid error"); // set process group
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
			if (!bg) 
				insertjob(pid, 'f', cmdline);
			
			else//when there is backgrount process!
				insertjob(pid, 'b', cmdline);
			
			if(sigprocmask(SIG_UNBLOCK, &mask, NULL)==-1)unix_error("sigprocmask error"); // unblock signal
			
			/* Parent waits for foreground job to terminate */
			if (!bg){
				wait_fg(pid);
			}
			else//when there is backgrount process!
				printf("[%d] %d %s", pidtojid(pid), pid, cmdline);
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

	if(!strcmp(argv[0], "exit")){ // command exit
		for(int j = 0 ; j < 20 ; j++){
			if(jobs[j].pid != 0){
				Kill(jobs[j].pid, SIGKILL);
			}
		}
		exit(0);
	}
	
    if (!strcmp(argv[0], "jobs")){ // command jobs
		cmd_jobs();		
		return 1;
	}
	
    if (!strcmp(argv[0], "bg")){ // command bg
		cmd_bg(argv);
		return 1;
	}	

    if (!strcmp(argv[0], "fg")){ // command fg
		cmd_fg(argv);
		return 1;
	}
	
    if (!strcmp(argv[0], "kill")){ // command kill %jobid
		if(argv[1][0] == '%'){
			int jid = atoi(&(argv[1][1]));
			Job_t *tmp;
			if((tmp = jidtojob(jid)) == 0){
				printf("Error : kill error invalid job id\n");
				return 1;
			}
			Kill(tmp->pid, SIGKILL);
			return 1;
		}
	}
	
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


