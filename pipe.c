/* mshell - a job manager */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "jobs.h"
#include "pipe.h"
#include "cmd.h"



int link_cmd(char * cmds[MAXCMDS][MAXARGS], int nbcmd, int pipe_in){

  pid_t pid;
  int fds[2];
  
  
  if(nbcmd == 0)
    return pipe_in;
  
  if(pipe(fds) != 0)
    unix_error("(Error do_pipe : can not create pipe)");

  switch(pid = fork()){
  case -1:
    unix_error("(Error do_pipe : can not create fork)");
    break;
  case 0:
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    dup2(pipe_in, STDIN_FILENO);
    execvp(cmds[0][0], cmds[0]);
    unix_error("(Error do_pipe : can not load son executable)");
    break;
  default:
    break;
  }

  close(pipe_in);
  close(fds[1]);
  return link_cmd(cmds + 1, nbcmd - 1, fds[0]);
}



void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {

  pid_t pid;
  int fds[2];
  int pipe_out;
  int state;
  
  if(pipe(fds) != 0)
    unix_error("(Error do_pipe : can not create pipe)");
  
  /* Creation du premier processus de la chaine */
  verbose_printf("do_pipe : create first process\n");
   switch(pid = fork()){
  case -1:
    unix_error("(Error do_pipe : can not create fork)");
    break;
  case 0:
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    execvp(cmds[0][0], cmds[0]);

    unix_error("(Error do_pipe : can not load son executable)");
    break;
  default:
    break;
  }

   close(fds[1]);
   pipe_out = link_cmd(cmds + 1, nbcmd - 2, fds[0]);
   
   verbose_printf("do_pipe : create last process\n");
   /* Création du dernier processus de la chaine */
   switch(fork()){
   case -1:
     unix_error("(Error do_pipe : can not create fork)");
     break;
   case 0:
     dup2(pipe_out, STDIN_FILENO);
     execvp(cmds[nbcmd - 1][0], cmds[nbcmd - 1]);
     unix_error("(Error do_pipe : can not load son executable)");
     break;
   default:
     break;
   }

   close(pipe_out);

   if(bg)
     state = BG;
   else
     state = FG;

   if(jobs_addjob(pid, state, cmds[0][0]) == 0)
     unix_error("do_pipe error : error while adding job)");

   if(bg)
     waitfg(pid);
  
}
