/* mshell - a job manager */

#include <stdio.h>
#include "pipe.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
  
  int fds[2];
  int status;
  
  status = pipe(fds);
  assert(status == 0);

  switch(pid = fork()){
  case -1:
    unix_error("(Error do_pipe)");
    break;
  case 0:
    close(fds[0]);
    dup2(fds[1], stdout);
    execvp(cmds[0][0], cmds[0]+1);
    unix_error("(Error son do_pipe)");
    break;
  default:
    break;
  }

  

    return;
}
