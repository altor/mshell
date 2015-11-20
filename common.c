/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "common.h"

void unix_error(char *msg) {
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/* print a message only if verbose mode is active */
void verbose_printf(const char *format, ...) {

  if(verbose){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
  return;
}
