/* Signal handlers for CLI shell.  */

#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
//#include <readline/readline.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

#include "cli.h"
#include "clish.h"
#include "sys/mman.h"
#include "cli_mode_types.h"
#include "clish_exec.h"

extern struct cli g_stcli;
extern struct process *process_list;

extern int clish_notify_signal_thread(int sig);

void clish_wait_for_process ();

static int g_nSigFlag = 0;
#define M_SIG_FLAG_INT      0x01
#define M_SIG_FLAG_TSTP     0x02
#define M_SIG_FLAG_ALRM     0x04
#define M_SIG_FLAG_USR1     0x08
#define M_SIG_FLAG_USR2     0x10
#define M_SIG_FLAG_TERM     0x20
/*modified by weij for bug 14280, 2011-03-08*/
#define M_SIG_FLAG_HUP      0x40

void clish_record_signal();

/* Signal set function declaration. */
void *clish_signal_set (int, void (*)(int));

/* SIGINT handler.  This function takes care user's ^C input.  */
void
clish_sigint (int sig)
{
  /* Reinstall signal. */
  clish_signal_set (SIGINT, clish_record_signal);

  /* Wait child process.  */
  if (process_list)
    clish_wait_for_process ();

  /* Reset terminal.  */
  //rl_reset_terminal("linux");

  g_stcli.mode = EXEC_MODE;
}

/* SIGTSTP handler.  This function takes care users's ^Z input.  */
void
clish_sigtstp (int sig)
{
  /* Reinstall signal. */
  clish_signal_set (SIGTSTP, clish_record_signal);

  if (process_list)
    return;

  /* Reset terminal.  */
  //rl_reset_terminal("linux");

  g_stcli.mode = EXEC_MODE;
}

/* Time out handler.  */
void
clish_sigalarm (int sig)
{
#if 0
  int idle;
  int timeout;
  
  /* Get tty idle value and time out value.  */
  idle = imish_ttyidle ();
  timeout = imish_timeout_get ();

  if (idle >= 0 && idle < timeout)
    /* Reset time out.  */
    alarm (timeout - idle);
  else
    {
      /* Time out.  */
      printf ("\nVty connection is timed out.\n");

      /* Changed by Zhang Weifeng, temporarily not let imish exit */
      exit (0);
    }
#endif
}

/* Notification from IMI for synchronization.  */
void
clish_user1 (int sig)
{
//  imish_process_sigusr1();

  return;
}

/* SIGUSR2 handler.  This function takes care to move user to CONFIG_MODE.  */
void
clish_user2 (int sig)
{
  /* Reinstall signal. */
  clish_signal_set (SIGUSR2, clish_record_signal);

  /* Wait child process.  */
  if (process_list)
    clish_wait_for_process ();
  else
    printf ("\n");

  /* Reset terminal.  */
  //rl_reset_terminal("linux");

  //rl_callback_handler_install(clish_prompt(), clish_parse_line);
  return;
}

/* Notification of imish exit from IMI. */
void
clish_sigterm (int sig)
{
  /* Exit imish. */
  char * tty =NULL;
  tty = ttyname (STDIN_FILENO);
  if (tty && !strstr (tty, "console"))
  {
      printf ("\n%% Connection is closed by administrator!\n");
  }
  
  exit (0);
  return;
}

/* Signale wrapper. */
void *
clish_signal_set (int signo, void (*func)(int))
{
  int ret;
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = func;
  sigemptyset (&sig.sa_mask);
  sig.sa_flags = 0;
#ifdef SA_RESTART
  sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */

  ret = sigaction (signo, &sig, &osig);

  if (ret < 0) 
    return (SIG_ERR);
  else
    return (osig.sa_handler);
}

/* Initialization of signal handlers. */
void
clish_signal_init ()
{
  clish_signal_set (SIGINT, clish_record_signal);
  clish_signal_set (SIGTSTP, clish_record_signal);
  clish_signal_set (SIGALRM, SIG_IGN);
  clish_signal_set (SIGUSR1, clish_record_signal);
  clish_signal_set (SIGUSR2, clish_record_signal);
  clish_signal_set (SIGPIPE, SIG_IGN);
  clish_signal_set (SIGQUIT, SIG_IGN);
  clish_signal_set (SIGTTIN, SIG_IGN);
  clish_signal_set (SIGTTOU, SIG_IGN);
  clish_signal_set (SIGTERM, clish_sigterm);
  clish_signal_set (SIGHUP,  clish_record_signal);
}

/* set sig flag */
void
set_sig_flag(int sig)
{
    int flag = 0;
    switch(sig)
    {
    case SIGINT:
        flag = M_SIG_FLAG_INT;
        break;
    case SIGTSTP:
        flag = M_SIG_FLAG_TSTP;
        break;
    case SIGALRM:
        flag = M_SIG_FLAG_ALRM;
        break;
    case SIGUSR1:
        flag = M_SIG_FLAG_USR1;
        break;
    case SIGUSR2:
        flag = M_SIG_FLAG_USR2;
        break;
    case SIGTERM:
        flag = M_SIG_FLAG_TERM;
        break;
    case SIGHUP:
        flag = M_SIG_FLAG_HUP;
        break;
    default:
        break;
    }
    
    g_nSigFlag |= flag;
    return;
}

void
clear_sig_flag_all(void)
{
    g_nSigFlag = 0;
}

void
clear_sig_flag(int sig)
{
    int flag = 0;
    switch(sig)
    {
    case SIGINT:
        flag = M_SIG_FLAG_INT;
        break;
    case SIGTSTP:
        flag = M_SIG_FLAG_TSTP;
        break;
    case SIGALRM:
        flag = M_SIG_FLAG_ALRM;
        break;
    case SIGUSR1:
        flag = M_SIG_FLAG_USR1;
        break;
    case SIGUSR2:
        flag = M_SIG_FLAG_USR2;
        break;
    case SIGTERM:
        flag = M_SIG_FLAG_TERM;
        break;
    case SIGHUP:
        flag = M_SIG_FLAG_HUP;
        break;
    default:
        break;
    }
    
    g_nSigFlag &= ~flag;
    return;
}

void
clish_process_signal(void)
{
    if (g_nSigFlag & M_SIG_FLAG_INT)
    {
        clish_sigint(SIGINT);
    }
    
    if (g_nSigFlag & M_SIG_FLAG_TSTP)
    {
        clish_sigtstp(SIGTSTP);
    }

    if (g_nSigFlag & M_SIG_FLAG_ALRM)
    {
        clish_sigalarm(SIGALRM);
    }

    if (g_nSigFlag & M_SIG_FLAG_USR1)
    {
        clish_user1(SIGUSR1);
    }
    
    if (g_nSigFlag & M_SIG_FLAG_USR2)
    {
        clish_user2(SIGUSR2);
    }
    
    if (g_nSigFlag & M_SIG_FLAG_TERM)
    {
        clish_sigterm(SIGTERM);
    }

    if (g_nSigFlag & M_SIG_FLAG_HUP)
    {
        exit(0);
    }

    if (g_nSigFlag & (M_SIG_FLAG_TSTP | M_SIG_FLAG_INT))
    {
#ifndef CONTROL
#define CONTROL(X)  ((X) - '@')
#endif
        char buf[] = {CONTROL('U')};

        ioctl(STDIN_FILENO, TIOCSTI, buf);
        ioctl(STDIN_FILENO, TIOCSTI, "\n");
    }
    clear_sig_flag_all(); 
    return;
}

void
clish_record_signal(int sig)
{
    set_sig_flag(sig);
    clish_notify_signal_thread(sig);
    clish_set_timezone();
    return;
}
