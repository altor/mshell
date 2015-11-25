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

  verbose_printf("sigchld_handler: entering\n");

  while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0) {
    if(WIFEXITED(status)){
      
      verbose_printf("sigchld_handler : child terminated normally\n");
      jobs_deletejob(pid);
    }

    if(WIFSIGNALED(status)) {
	verbose_printf("sigchld_handler : child terminate because of a non catched signal\n");
      jobs_deletejob(pid);
    }
    
    if(WIFSTOPPED(status)) {
      verbose_printf("sigchld_handler : child stopped\n");
      (jobs_getjobpid(pid))->jb_state = ST;
    }
  }

  verbose_printf("sigchld_handler: exiting\n");
  return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {

  pid_t pid;

  verbose_printf("sigint_handler: entering\n");

  if((pid = jobs_fgpid()) == 0){
    verbose_printf("no job in foreground\n");
    return;
  }
  
  if(kill(pid, sig) < 0){
    unix_error("(Error sigint_handler) ");
  }

  verbose_printf("sigint_handler: exiting\n");

  return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {

  pid_t pid;
  
  verbose_printf("sigtstp_handler: entering\n");


  if((pid = jobs_fgpid()) == 0){
    verbose_printf("sigtstp_handler : no job in foreground\n");
    return;
  }
   
  if(kill(pid, sig) < 0){
    unix_error("(Error sigtstp_handler) ");
  }


  verbose_printf("sigtstp_handler: exiting\n");

  return;
}
