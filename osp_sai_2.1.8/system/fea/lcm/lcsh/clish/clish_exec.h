#ifndef _CLISH_EXEC_H
#define _CLISH_EXEC_H
//#include "afx.h"

/* Internal function.  */
typedef int (*INTERNAL_FUNC) (void *, int, char **);

#define PROCPS_BUFSIZE 1024

/* Process structure.  */
struct process
{
  /* Single linked list.  */
  struct process *next;

  /* Execute argv.  */
  char **argv;

  /* Process id of child process.  */
  pid_t pid;

  /* Status.  */
  int status;

  /* Completed.  */
  int completed;

  /* When the command is internal, set this flag.  */
  int internal;

  /* Internal function pointer.  */
  INTERNAL_FUNC func;

  /* Internal function value.  */
  void *func_val;

  /* Internal function argument count.  */
  int func_argc;

  /* Internal function arguments.  */
  char **func_argv;

  /* For redirect of output.  */
  char *path;
};

int clish_execvp (char **argv);
int get_last_exec_result(void);
int clish_sync_timezone(char* name);
void clish_set_timezone(void);
#if 0 /*weij*/
void clish_schedule_reboot_alarm_cb();
#endif
void clish_schedule_reboot();

/* Execute command with modifier.  */
int clish_more (INTERNAL_FUNC func, struct cli *cli,
        int argc, char **argv);
int clish_begin (INTERNAL_FUNC func, struct cli *cli,
        int argc, char **argv, char *regexp);
int clish_include (INTERNAL_FUNC func, struct cli *cli,
        int argc, char **argv, char *regexp);
int clish_exclude (INTERNAL_FUNC func, struct cli *cli,
        int argc, char **argv, char *regexp);
int clish_grep (INTERNAL_FUNC func, struct cli *cli, 
        int argc, char **argv, int argc_modifier, char **argv_modifier);
int clish_redirect (INTERNAL_FUNC func, struct cli *cli, 
        int argc, char **argv, char *path);

#endif
