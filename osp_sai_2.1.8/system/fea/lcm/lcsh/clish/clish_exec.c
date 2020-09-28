#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include "cli.h"
//#include <readline/readline.h>
#include "clish.h"
#include "sys/mman.h"
#include <dirent.h>
#include "cli_mode_types.h"
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include "clish_filesystem.h"
#include "clish_exec.h"
//#include "afx.h"
#include "genlog.h"

extern void *clish_signal_set (int signo, void (*func)(int));
void clish_record_signal(int sig);

/* When WAIT_ANY is defined, use it.  */
#ifdef WAIT_ANY
#define CHILD_PID         WAIT_ANY
#else
#define CHILD_PID         -1
#endif /* WAIT_ANY. */

/* Define default pager.  */
#if ((defined CNOS_OS) && (defined _CTC_X86_64_))
char *pager = "busybox";
#else
#if CTC_IS_UML
char *pager = "/centec_switch/sbin/more";
#else
char *pager = "more";
#endif
#endif

//afx_timer_t *pTimer_alarm;
//afx_timer_t *pTimer_boot;

/* Maintain current process list.  */
struct process *process_list;
int external_command = 0;

volatile sig_atomic_t g_last_exec_status = 0;

int get_last_exec_result(void)
{
    return g_last_exec_status;
}

/* Allocate a new process for pipeline.  */
struct process *
clish_pipe_add (struct process *prev)
{
  struct process *proc;

  proc = calloc (1, sizeof (struct process));
  if (! proc)
    return NULL;

  if (prev)
    prev->next = proc;

  return proc;
}

/* Free pipeline memory.  */
static void
clish_pipe_free (struct process *proc)
{
  struct process *next;

  while (proc)
    {
      next = proc->next;
      free (proc);
      proc = next;
    }
}

/* Return 1 when all child processes are completed.  */
static int
clish_process_completed ()
{
  struct process *p;

  for (p = process_list; p; p = p->next)
    if (! p->completed)
      return 0;
  return 1;
}

/* Mark process stautus.  */
static int
clish_update_process_status (pid_t pid, int status)
{
  struct process *p;

  if (pid > 0)
    {
      for (p = process_list; p; p = p->next)
	if (p->pid == pid)
	  {
	    p->status = status;
	    
	    if (! WIFSTOPPED (status))
	      p->completed = 1;
	    
	    return 0;
	  }
      fprintf (stderr, "No child process %d\n", pid);

      return -1;
    }
  else if (pid == 0 || errno == ECHILD)
    return -1;
  else
    {
      perror ("waitpid");
      return -1;
    }
}

/* Exec child process.  */
static void
clish_exec_process (struct process *p, int in_fd, int out_fd, int err_fd)
{
  /* Set the handler to the default.  */
  clish_signal_set (SIGINT, SIG_DFL);
  clish_signal_set (SIGQUIT, SIG_DFL);
  clish_signal_set (SIGCHLD, SIG_DFL);
  clish_signal_set (SIGALRM, SIG_DFL);
  clish_signal_set (SIGPIPE, SIG_DFL);
  clish_signal_set (SIGUSR1, SIG_DFL);

  /* No job control.  */
  clish_signal_set (SIGTSTP, SIG_IGN);
  clish_signal_set (SIGTTIN, SIG_IGN);
  clish_signal_set (SIGTTOU, SIG_IGN);

  /* Standard input, output and error.  */
  if (in_fd != STDIN_FILENO)
    {
      if (dup2 (in_fd, STDIN_FILENO) < 0)
      {
          perror ("dup2(in_fd)");
          exit (1);
      }
      close (in_fd);
    }
  if (out_fd != STDOUT_FILENO)
    {
      if (dup2 (out_fd, STDOUT_FILENO) < 0)
      {
          perror ("dup2(out_fd)");
          exit (1);
      }
      close (out_fd);
    }
  if (err_fd != STDERR_FILENO)
    {
      if (dup2 (err_fd, STDERR_FILENO) < 0)
      {
          perror ("dup2(err_fd)");
          exit (1);
      }
      close (err_fd);
    }

  /* Internal process.  */
  if (p->internal)
    {
      (*(p->func))(p->func_val, p->func_argc, p->func_argv);
      exit (0);
    }

  execvp (p->argv[0], p->argv);

  /* Not reached heer unless error occurred.  */
  perror ("execvp");
  exit (1);
}

/* Wait all of child processes.  Clear process list after all of child
   process is finished.  */
void
clish_wait_for_process ()
{
  pid_t pid;
  int status;

  do
  {
    pid = waitpid (CHILD_PID, &status, WUNTRACED);
  } while (clish_update_process_status (pid, status) == 0
	 && clish_process_completed () == 0);
  if ((WEXITSTATUS(status) & 0xff) == 17)
  {
      /* more or other prog timed out */
      rl_set_timeout_flag(1);
  }
  g_last_exec_status = status;

  clish_pipe_free (process_list);
  process_list = NULL;

  if (external_command)
    {
      external_command = 0;
#if defined(__NetBSD__) || defined(_CENTEC_)
//      readline_set_time ();
#endif /* __NetBSD__ */
    }
}

/* Run pipe lines.  */
static void
clish_pipe_line (struct process *process)
{
  int in_fd;
  int out_fd;
  int err_fd;
  int pipe_fd[2];
  int ret;
  pid_t pid;
  struct process *p;
  int bFirst = 1;

  in_fd = STDIN_FILENO;
  err_fd = STDERR_FILENO;
  process_list = process;

  for (p = process; p; p = p->next)
    {
      /* Prepare pipe.  */
      if (p->next)
	{
	  ret = pipe (pipe_fd);
	  if (ret < 0)
	    {
	      perror ("pipe");
	  	  if (bFirst != 1)
	  	  {
	     	  close(in_fd);
	  		  break;
	  	  }
	      return;
	    }
	  out_fd = pipe_fd[1];
	}
      else
	{
	  /* Redirection of output.  */
	  if (p->path)
	    {
	      out_fd = open (p->path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	      if (out_fd < 0)
		{
          char szFullName[M_FULLPATH_MAX_LEN];
          if (gen_check_and_gen_showname(p->path, szFullName) == 0)
          {
              fprintf (stderr, "%% Can't open output file %s\n", szFullName);
          }
          else
          {
              fprintf (stderr, "%% Can't open output file %s\n", p->path);
          }
	 	  if (bFirst != 1)
	  	  {
	   		  close(in_fd);
	  		  break;
	  	  }
		  return;
		}
	    }
	  else
	    out_fd = STDOUT_FILENO;
	}

      /* Fork. */
      pid = fork ();
      if (pid < 0)
	{
	  perror ("fork");
	  if (bFirst != 1)
	  {
	    close(in_fd);
	  	break;
	  }
	  return;
	}
    bFirst++;
    clish_signal_set (SIGINT, SIG_IGN);
    clish_signal_set (SIGTSTP, SIG_IGN);

      if (pid == 0)
	{
	  /* Pipe's incoming fd is not used by child process.  */
	  if (p->next)
	    close (pipe_fd[0]);

	  /* This is child process.  */
	  clish_exec_process (p, in_fd, out_fd, err_fd);
	}
      else
	/* This is parent process.  */
	p->pid = pid;

      if (in_fd != STDIN_FILENO)
	close (in_fd);
      if (out_fd != STDOUT_FILENO)
	close (out_fd);
      in_fd = pipe_fd[0];
    }

  /* Wait for all of child processes.  */
  clish_wait_for_process ();

  clish_signal_set (SIGINT, clish_record_signal);
  clish_signal_set (SIGTSTP, clish_record_signal);
}

/* Send SIGKILL to all of child processes.  */
void
clish_stop_process ()
{
  struct process *p;

  if (process_list)
    {
      for (p = process_list; p; p = p->next)
	if (p->pid > 0 && ! p->completed)
	  kill (p->pid, SIGKILL);

      clish_wait_for_process ();
    }
}

/* Execute command in child process.  */
int
clish_exec (char *command, ...)
{
  va_list args;
  int argc;
  char **argv;
  struct process *proc;

  /* Count arguments.  */
  va_start (args, command);
  for (argc = 1; va_arg (args, char *) != NULL; ++argc)
    continue;
  va_end (args);

  /* Malloc argv. Add 1 for NULL termination in last argv.  */
  argv = calloc (1, (argc + 1) * sizeof (*argv));
  if (! argv)
    return -1;

  /* Copy arguments to argv. */
  argv[0] = command;

  va_start (args, command);
  for (argc = 1; (argv[argc] = va_arg (args, char *)) != NULL; ++argc)
    continue;
  va_end (args);

  proc = clish_pipe_add (NULL);
  if (! proc)
    return -1;
  proc->argv = argv;

  /* Execute command. */
  external_command = 1;
  clish_pipe_line (proc);

  /* Free argv. */
  free (argv);

  return 0;
}

/* Execute command in child process.  Arguemnt 'argv' is passed to
   execvp().  */
int
clish_execvp (char **argv)
{
  struct process *proc;

  proc = clish_pipe_add (NULL);
  if (! proc)
    return -1;
  proc->argv = argv;

  /* Execute command. */
  external_command = 1;
  clish_pipe_line (proc);

  return 0;
}

/* Set internal func process.  */
static struct process *
clish_internal_pipe (INTERNAL_FUNC func, void *val, int argc, char **argv)
{
  struct process *proc;

  proc = clish_pipe_add (NULL);
  if (! proc)
    return NULL;

  proc->func = func;
  proc->func_val = val;
  proc->func_argc = argc;
  proc->func_argv = argv;
  proc->internal = 1;

  return proc;
}

/* More for show commands.  */
int
clish_more (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv)
{
  struct process *proc_top;
  struct process *proc;
  char *argv_more[3];

  /* Initialize more array. */
  argv_more[0] = pager;
#ifdef CNOS_OS
#ifdef _CTC_X86_64_
  argv_more[1] = "more";
#else
  argv_more[1] = NULL;
#endif
#else
  argv_more[1] = clish_get_timeout_str();
#endif
  argv_more[2] = NULL;

  /* Internal command.  */
  proc_top = proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc_top)
    return -1;

  /* More */
  if (cli->lines)
    {
      proc = clish_pipe_add (proc);
      if (! proc)
	return -1;
      proc->argv = argv_more;
    }

  clish_pipe_line (proc_top);

  return 0;
}

/* Output modifier begin.  */
int
clish_begin (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv,
	     char *regexp)
{
  struct process *proc_top;
  struct process *proc;
  char *argv_more[4];
  char *option = NULL;

  /* Initialize more array. */
  argv_more[0] = pager;
#ifdef CNOS_OS
#ifdef _CTC_X86_64_
  argv_more[1] = "more";
#else
  argv_more[1] = NULL;
#endif
#else
  argv_more[1] = clish_get_timeout_str();
#endif
  argv_more[2] = NULL;
  argv_more[3] = NULL;

  /* Internal command.  */
  proc_top = proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc_top)
    return -1;

  /* More */
  proc = clish_pipe_add (proc);
  if (! proc)
    return -1;

  /* Prepare string for "more +/regexp".  */
  option = calloc (1, strlen ("+/") + strlen (regexp) + 1);
  if (! option)
    return -1;
  strcat (option, "+/");
  strcat (option, regexp);
  argv_more[2] = option;
  proc->argv = argv_more;

  clish_pipe_line (proc_top);

  free (option);

  return 0;
}

/* Output modifier include.  */
int
clish_include (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv,
	       char *regexp)
{
  struct process *proc_top;
  struct process *proc;
  char *argv_grep[3];
  char *argv_more[3];

  /* Initialize grep array. */
  argv_grep[0] = "egrep";
  argv_grep[1] = regexp;
  argv_grep[2] = NULL;

  /* Initialize more array. */
  argv_more[0] = pager;
#ifdef CNOS_OS
#ifdef _CTC_X86_64_
  argv_more[1] = "more";
#else
  argv_more[1] = NULL;
#endif
#else
  argv_more[1] = clish_get_timeout_str();
#endif
  argv_more[2] = NULL;

  /* Internal command.  */
  proc_top = proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc_top)
    return -1;

  /* egrep.  */
  proc = clish_pipe_add (proc);
  if (! proc)
    return -1;
  proc->argv = argv_grep;

  /* More */
  if (cli->lines)
    {
      proc = clish_pipe_add (proc);
      if (! proc)
	return -1;
      proc->argv = argv_more;
    }

  clish_pipe_line (proc_top);

  return 0;
}

/* Output modifier exclude.  */
int
clish_exclude (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv,
	       char *regexp)
{
  struct process *proc_top;
  struct process *proc;
  char *argv_grep[4];
  char *argv_more[3];

  /* Initialize grep array. */
  argv_grep[0] = "egrep";
  argv_grep[1] = "-v";
  argv_grep[2] = regexp;
  argv_grep[3] = NULL;

  /* Initialize more array. */
  argv_more[0] = pager;
#ifdef CNOS_OS
#ifdef _CTC_X86_64_
  argv_more[1] = "more";
#else
  argv_more[1] = NULL;
#endif
#else
  argv_more[1] = clish_get_timeout_str();
#endif
  argv_more[2] = NULL;

  /* Internal command.  */
  proc_top = proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc_top)
    return -1;

  /* Exclude using "egrep -v".  */
  proc = clish_pipe_add (proc);
  if (! proc)
    return -1;
  proc->argv = argv_grep;

  /* More */
  if (cli->lines)
    {
      proc = clish_pipe_add (proc);
      if (! proc)
	return -1;
      proc->argv = argv_more;
    }

  clish_pipe_line (proc_top);

  return 0;
}

/* Output modifier grep (hidden command).  */
int
clish_grep (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv,
	    int argc_modifier, char **argv_modifier)
{
  struct process *proc_top;
  struct process *proc;
  char *argv_grep[4];
  char *argv_more[3];

  /* Internal command.  */
  proc_top = proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc_top)
    return -1;

  /* Pipe for grep".  */
  proc = clish_pipe_add (proc);
  if (! proc)
    return -1;

  /* Prepare grep command.  */
  argv_grep[0] = "grep";
  if (argc_modifier == 1)
    {
      argv_grep[1] = argv_modifier[0];
      argv_grep[2] = NULL;
    }
  else if (argc_modifier == 2)
    {
      argv_grep[1] = argv_modifier[0];
      argv_grep[2] = argv_modifier[1];
      argv_grep[3] = NULL;
    }
  proc->argv = argv_grep;

  /* Initialize more array. */
  argv_more[0] = pager;
#ifdef CNOS_OS
#ifdef _CTC_X86_64_
  argv_more[1] = "more";
#else
  argv_more[1] = NULL;
#endif
#else
  argv_more[1] = clish_get_timeout_str();
#endif
  argv_more[2] = NULL;

  if (cli->lines)
    {
      proc = clish_pipe_add (proc);
      if (! proc)
	return -1;
      proc->argv = argv_more;
    }

  clish_pipe_line (proc_top);

  return 0;
}

/* Output modifier redirect.  */
int
clish_redirect (INTERNAL_FUNC func, struct cli *cli, int argc, char **argv,
		char *path)
{
  struct process *proc;

  /* Internal command.  */
  proc = clish_internal_pipe (func, cli, argc, argv);
  if (! proc)
    return -1;

  proc->path = path;

  clish_pipe_line (proc);

  return 0;
}

/* Check environment variable to set pager.  */
void
clish_set_pager ()
{
  char *custom_pager;

  custom_pager = getenv ("IMI_PAGER");
  if (custom_pager)
    {
      pager = custom_pager;
      printf ("pager is set to %s\n", pager);
    }
}

static int clish_read_to_buf(const char *filename, void *buf)
{
    FILE *fd;

    fd = fopen(filename, "r");
    if(fd < 0)
        return -1;
    fgets(buf, PROCPS_BUFSIZE-1, fd );
    
    fclose(fd);
    return 0;
}

int clish_sync_timezone(char* name)
{
    static DIR *dir;
    struct dirent *entry;
    int n;
    char *comm;
    char status[32];
    char *status_tail;
    char buf[PROCPS_BUFSIZE];
    int pid;
    struct stat sb;

    dir = opendir("/proc");
    do 
    {
        if((entry = readdir(dir)) == NULL) 
        {
            closedir(dir);
            return 0;
        }
        comm = entry->d_name;
        if (!(*comm >= '0' && *comm <= '9'))
            continue;
        pid = atoi(comm);

        status_tail = status + sprintf(status, "/proc/%d", pid);
        if(stat(status, &sb))
            continue;

        strcpy(status_tail, "/stat");
        n = clish_read_to_buf(status, buf);
        		
        if(n < 0)
           continue;
        		
        comm = strrchr(buf, ')'); /*split into "PID (cmd" and "<rest>" */
        if(comm == 0 || comm[1] != ' ')
            continue;
        *comm = 0;	/*replace ')' with '\0'*/ 
        comm = strrchr(buf, '(');	/*get comm start point*/
        comm++;			/*skip '('*/

        if(strcmp(comm, name)==0 )
        {
            kill(pid, SIGUSR1);
        }
    }
    while(1);
    return 0;
}

void
clish_set_timezone(void)
{
    FILE *readfile=NULL;
    char szBuf[CLISH_BUFSIZE];
    char *newline_position;
    
    readfile = fopen(ZONE_CONFIG_PATH,"r");
    if (readfile == NULL)
    {
        return;
    }
    
    while (!feof(readfile))
    {
        fgets(szBuf, CLISH_BUFSIZE, readfile);
        /* Should make a TZ=JST-01:00:00 style variable, never include
           \n or \" character, otherwise tzset will wrongly set daylight saving info because of
           the extra \n or \" in the end of line. */
        if (memcmp(szBuf, "TZ=",3)==0 && strlen(szBuf)>3)
        {
            /* Should make a TZ=JST-01:00:00 style variable, never include
               \n or \" character, otherwise tzset will wrongly set daylight saving info because of
               the extra \n or \" in the end of line. */
            newline_position = sal_strchr(szBuf, '\n');
            if (newline_position)
            {
                *newline_position = '\0';
            }
            setenv("TZ",szBuf+3,1);
            tzset();
            break;
        }
    }
    fclose(readfile);
    return;
}

#if 0 /*weij*/
/*******************************************************************************
 * Name:    imi_schedule_reboot_cb
 * Purpose: schedule reboot callback function
 * Input: 
 *   thread: thread
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
void
clish_schedule_reboot()
{
    time_t date; 
    char strTime[50];
    memset(strTime, 0, sizeof(strTime));
    date = time(NULL);   
    strftime(strTime, sizeof(strTime), "%T %Z %a %b %d %Y", localtime(&date));

    /*log reboot info*/
    log_sys(M_MOD_IMI, E_INFORMATIONAL,"System reboot in %s", strTime);
    if (pTimer_boot)
    {
        afx_timer_destroy(pTimer_boot);
        pTimer_boot = NULL;
    }
#if CTC_IS_UML
        system("reboot -f");
#else
#ifdef HAVE_DEBIAN
        system("reboot -f");
#else
        system("reboot");
#endif
#endif
    return;
}


/*******************************************************************************
 * Name:    imi_schedule_reboot_alarm_cb
 * Purpose: schedule reboot alarm callback function
 * Input: 
 *   thread: thread
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
void
clish_schedule_reboot_alarm_cb()
{
    const char *fn_name = "\n System will reboot by reboot schedule in 1 minute. ";
    static DIR *dir;
    char *comm;
    char status[32];
    struct dirent *entry;
    struct stat sb;
    int fd;
    if (pTimer_alarm)
    {
        afx_timer_destroy((afx_timer_t *)pTimer_alarm);
        pTimer_alarm = NULL;
    }
    if (afx_timer_create(&pTimer_boot, clish_schedule_reboot, NULL) != 0)
    {
        return;
    }
    /* 1 minute */
    afx_timer_start(pTimer_boot, 60000);

    dir = opendir(M_TTY_DIR);
    do 
    {
        if((entry = readdir(dir)) == NULL) 
        {
            closedir(dir);
            return;
        }
        comm = entry->d_name;
#if CTC_IS_UML
        if ((strncmp (comm, "tty", 3) != 0)||(strcmp (comm, "tty") == 0))
            continue;
#else
        if (!(*comm >= '0' && *comm <= '9'))
            continue;
#endif

        sprintf(status, "%s/%s", M_TTY_DIR, comm);

        if (stat(status, &sb))
        {
            continue;
        }

        fd = open(status, O_WRONLY);
        if (fd == -1)
        {
            continue;
        }
        write(fd, fn_name, strlen(fn_name));
        close(fd);
    }
    while(1);
    /*need to inform all the clish*/
    return;
}
#endif

