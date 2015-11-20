/* mshell - a job manager */

#ifndef _COMMON_H_
#define _COMMON_H_

#define MAXLINE    1024         /* max line size                      */
#define MAXARGS    20           /* max args on a command line         */
#define MAXCMDS    10           /* max commands in a command pipeline */

int verbose;                    /* if true, print additional output   */

extern void unix_error(char *msg);

/* print a message only if verbose mode is active */
extern void verbose_printf(const char *format, ...);

#endif
