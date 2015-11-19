/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>


#include "jobs.h"
#include "common.h"
#include "sighandlers.h"

/*
 * wrapper for the sigaction function
 */
int sigaction_wrapper(int signum, handler_t * handler) {
    
	struct sigaction action;
	action.sa_flags = SA_RESTART;
	sigemptyset(&action.sa_mask);
	action.sa_handler = handler;
	if(sigaction(signum, &action, NULL) < 0){
		unix_error("(Error sigaction_wrapper) ");
	}

    	return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {
  int status;
  pid_t pid;
  if (verbose)
      printf("sigchld_handler: entering\n");

  while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0) {

    if(WIFEXITED(status)) {
      if(verbose)
	printf("child terminated normally\n");
      jobs_deletejob(pid);
    }

    if(WIFSIGNALED(status)) {
      if(verbose)
	printf("child terminate because of a non catched signal\n");
      jobs_deletejob(pid);
    }
    
    if(WIFSTOPPED(status)) {
      if(verbose)
	printf("child stopped\n");
      (jobs_getjobpid(pid))->jb_state = ST;
    }
    
  }
    

  if (verbose)
      printf("sigchld_handler: exiting\n");

    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {

	pid_t pid;

    if (verbose)
        printf("sigint_handler: entering\n");

	pid = jobs_fgpid();
	if(kill(pid, sig) < 0){
		unix_error("(Error sigint_handler) ");
	}

    if (verbose)
        printf("sigint_handler: exiting\n");

    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {

	pid_t pid;
	
    if (verbose)
        printf("sigtstp_handler: entering\n");

	pid = jobs_fgpid();

	if(kill(pid, sig) < 0){
		unix_error("(Error sigint_handler) ");
	}


    if (verbose)
        printf("sigtstp_handler: exiting\n");

    return;
}
