/* mshell - a job manager */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"

#define BOLD "\033[00;01m"
#define NORM "\033[00;00m"

void do_help() {
  printf("available commands are:\n");
  printf(" exit - cause the shell to exit\n");
  printf(BOLD "\t exit\n" NORM);
  printf(" jobs - display status of jobs in the current session\n");
  printf(BOLD "\t jobs\n" NORM);
  printf(" fg   - run a job identified by its pid or job id in the foreground\n");
  printf(BOLD "\t fg " NORM "pid" BOLD "|" NORM "jobid \n");
  printf(" bg   - run a job identified by its pid or job id in the background\n");
  printf(BOLD "\t bg " NORM "pid" BOLD "|" NORM "jobid \n");
  printf(" stop - stop a job identified by its pid or job id\n");
  printf(BOLD "\t stop " NORM "pid" BOLD "|" NORM "jobid \n");
  printf(" kill - kill a job identified by its pid or job id\n");
  printf(BOLD "\t kill " NORM "pid" BOLD "|" NORM "jobid \n");
  printf(" help - print this message\n");
  printf(BOLD "\t help\n" NORM);
  printf("\n");
  printf("ctrl-z and ctrl-c can be used to send a SIGTSTP and a SIGINT, respectively\n\n");
}

/* treat_argv - Determine pid or jobid and return the associated job structure */
struct job_t *treat_argv(char **argv) {
  struct job_t *jobp = NULL;

  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return NULL;
  }

  /* Parse the required PID or %JID arg */
  if (isdigit((int) argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = jobs_getjobpid(pid))) {
      printf("(%d): No such process\n", (int) pid);
      return NULL;
    }
  } else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = jobs_getjobjid(jid))) {
      printf("%s: No such job\n", argv[1]);
      return NULL;
    }
  } else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return NULL;
  }

  return jobp;
}


/* do_bg - Execute the builtin bg command */
void do_bg(char **argv) {
  struct job_t * job;
  pid_t pid;

  
  verbose_printf("do_bg : entering\n");
  if((job = treat_argv(argv)) == NULL)
    unix_error("(No job to put in Background) ");

  pid = job->jb_pid;
  if(job->jb_state != ST)
    unix_error("(Job isn't in stoped)\n");


  kill(pid, SIGCONT);
  job->jb_state = BG;
  
  verbose_printf("do_bg : process %d put in background\n", pid);
  verbose_printf("do_bg : exit\n");
  return;
}

/* waitfg - Block until process pid is no longer the foreground process */
void waitfg(pid_t pid) {
  verbose_printf("wait_fg : entering\n");
  while(pid==jobs_fgpid());
  verbose_printf("wait_fg : exit\n");
  return;

}

/* do_fg - Execute the builtin fg command */
void do_fg(char **argv) {
  struct job_t * job;
  pid_t pid;
  
  verbose_printf("do_fg : entering\n");
  if((job = treat_argv(argv)) == NULL)
    unix_error("(No job to put in foreground) ");

  pid = job->jb_pid;
  if(job->jb_state == ST){
    verbose_printf("do_fg : wake jid : %d, pid %d\n", job->jb_jid, pid);
    kill(pid, SIGCONT);
  }

  job->jb_state = FG;
  verbose_printf("do_fg : exit\n");

  waitfg(pid);

  return;
}

/* do_stop - Execute the builtin stop command */
void do_stop(char **argv) {
  struct job_t * job;
  pid_t pid;

  
  verbose_printf("do_stop : entering\n");
  if((job = treat_argv(argv)) == NULL)
    unix_error("(No job to stop) ");

  pid = job->jb_pid;
  if(job->jb_state != BG)
    unix_error("(Job isn't in background)\n");


  kill(pid, SIGTSTP);
  job->jb_state = ST;
  verbose_printf("do_stop : process %d stoped\n", pid);
  verbose_printf("do_stop : exit\n");
  return;
}

/* do_kill - Execute the builtin kill command */
void do_kill(char **argv) {
  struct job_t * job;
  pid_t pid;

  
  verbose_printf("do_kill : entering\n");
  if((job = treat_argv(argv)) == NULL)
    unix_error("(No job to kill) ");

  pid = job->jb_pid;
  if(job->jb_state != BG)
    unix_error("(Job isn't in background)\n");


  kill(pid, SIGKILL);
  if(!jobs_deletejob(pid))
    unix_error("(error while delete job in job_list)\n");
 
  verbose_printf("do_kill : process %d killed\n", pid);
  verbose_printf("do_kill : exit\n");

  return;
}




/* do_exit - Execute the builtin exit command */
void do_exit() {

  int i;
  pid_t pid;
  struct job_t * job;

  verbose_printf("do_exit : entering\n");
  
  for(i = 0 ; i < jobs_maxjid() ; i++){

    job = jobs_getjobjid(i+1);
    pid = job->jb_pid;
    
    kill(pid, SIGKILL);
    verbose_printf("kill job %d of pid %d\n", i+1, pid);
    if(!jobs_deletejob(pid))
      unix_error("(error while delete job in job_list)\n");
  }
  
  verbose_printf("do_exit : exit\n");
  exit(EXIT_SUCCESS);
  return;
}

/* do_jobs - Execute the builtin fg command */
void do_jobs() {


  verbose_printf("do_jobs : entering\n");

  jobs_listjobs();

  verbose_printf("do_jobs : exit\n");

  return;
}
