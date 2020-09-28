/***************************************************************************
 *
 * Copyright (c) 2002 BalaBit IT Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: main.c,v 1.5 2007/03/19 05:57:52 xliu Exp $
 *
 ***************************************************************************/

#include "syslog-ng.h"
#include "cfg.h"
#include "messages.h"
#include "memtrace.h"
#include "children.h"
#include "memtrace.h"
#include "misc.h"
#include "stats.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#include <grp.h>

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <stropts.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <paths.h>

#include <stdio.h>

#define _TIME_ZONE_INGORE_DS_
//#include "time_zone.h"
#define ZONE_CONFIG_PATH    "/tmp/zone.conf"
#define ZONE_CONFIG_DEST    "/mnt/flash/.zone.conf"

#ifdef _CENTEC_ /* Modified by zhoucheng 2008-06-02 12:23, bug 5343 */
static char cfgfilename[128] = "/switch/etc/syslog-ng.conf";
#else
static char cfgfilename[128] = PATH_SYSLOG_NG_CONF;
#endif /* !_CENTEC_ */

static char pidfilename[128] = PATH_PIDFILE;

static gboolean do_fork = 1;
gboolean sig_hup_received = FALSE;
static gboolean sig_term_received = FALSE;
static gboolean sig_child_received = FALSE;
#ifdef HAVE_MERGE
static gboolean sig_alarm_received = FALSE;
static gboolean sig_usr2_received = FALSE;
gboolean sig_usr1_received = FALSE; /* Added by zhoucheng 2008-01-18 17:41, bug 4185 */
#endif
static gchar *chroot_dir = NULL;
static gchar *run_as_user = NULL;
static uid_t uid = 0;
static gid_t gid = 0;

extern void log_merge_sync_fifo();
extern void log_merge_sync_fifo_all();

/* Added by zhoucheng for debug */
#undef SYSLOG_NG_DEBUG

void usage(void)
{
  printf("Usage: syslog-ng [options]\n"
	 "Accept and manage system log messages\n\n"
	 "Options:\n"
	 "  -s, --syntax-only                Only read and parse config file\n"
	 "  -d, --debug                      Turn on debugging messages\n"
	 "  -v, --verbose                    Be a bit more verbose\n"
	 "  -e, --stderr                     Log internal messages to stderr\n"
	 "  -F, --foreground                 Don't fork into background\n"
	 "  -f <fname>, --cfgfile=<fname>    Set config file name, default=" PATH_SYSLOG_NG_CONF "\n"
	 "  -V, --version                    Display version number ( " VERSION ")\n"
	 "  -p <fname>, --pidfile=<fname>    Set pid file name, default=" PATH_PIDFILE "\n"
	 "  -C <dir>, --chroot=<dir>         Chroot to directory\n"
	 "  -u <user>, --user=<user>         Switch to user\n"
	 "  -g <group>, --group=<group>      Switch to group\n"
	 "  --ignore-persistent              Ignore persistent configuration file\n"
#ifdef YYDEBUG
	 "  -y, --yydebug                    Turn on yacc debug messages\n"
#endif
	 );

  exit(0);
}

#ifdef DEBUG_MEM_LEAK
long	 new_marker_count = 0;
long	 new_msg_count = 0;
long	 new_internal_count = 0;
long	 new_template_count = 0;
long	 free_marker_count = 0;
long	 free_msg_count = 0;
long	 free_internal_count = 0;
long	 free_template_count = 0;
long     new_file_dd_count = 0;
long     free_file_dd_count = 0;

void reset_debug_count()
{
	 new_marker_count = 0;
	 new_msg_count = 0;
	 new_internal_count = 0;
	 new_template_count = 0;

	 free_marker_count = 0;
	 free_msg_count = 0;
	 free_internal_count = 0;
	 free_template_count = 0;

     new_file_dd_count = 0;
     free_file_dd_count = 0;

}

void write_debug_count()
{
    FILE  *fp = NULL;
    fp = fopen("debug_log.txt", "ab+");
    if (fp != NULL)
    {   
        struct  tm * tmptm = NULL;
        char tmptime[32] = {0};
        char debug_str[256] = {0};
        time_t now_time;

        time(&now_time);
        tmptm = localtime(&now_time);
        strftime(tmptime, 32, "%X", tmptm);
        snprintf(debug_str, 128, "%s : %ld, %ld, %ld, %ld,   %ld, %ld, %ld, %ld\n", tmptime, 
                new_marker_count, new_msg_count, new_internal_count, new_template_count,
                free_marker_count, free_msg_count, free_internal_count, free_template_count);
        printf("%s\n", debug_str);
        fputs(debug_str, fp);
        fclose(fp);
    } 
}
#endif

static void 
sig_hup_handler(int signo)
{
  sig_hup_received = TRUE;
}

static void
sig_term_handler(int signo)
{
  sig_term_received = TRUE;
}

static void
sig_segv_handler(int signo)
{
#ifdef _CENTEC_ /* Modified by zhoucheng 2008-06-06 14:03 */
    exit(1);
#else
    kill(getpid(), SIGSEGV);
#endif /* !_CENTEC_ */
  return;
}

#ifdef _CENTEC_
static void
sig_bus_handler(int signo)
{
    exit(1);
    return;
}
#endif /* !_CENTEC_ */

static void
sig_child_handler(int signo)
{
  sig_child_received = TRUE;
}

/* IMI will send SIGUSR2 to tell syslog-ng to sync merge buffer */
#ifdef HAVE_MERGE
static void
sig_usr2_handler(int signo)
{
    sig_usr2_received = TRUE;
}

static void
sig_alarm_handler(int signo)
{
    sig_alarm_received = TRUE;
}
#endif

#ifdef _CENTEC_ /* Modified by zhoucheng 2008-01-18 17:39, bug 4185*/
static void
sig_usr1_handler(int signo)
{
    sig_usr1_received = TRUE;
}
#endif /* !_CENTEC_ */

static void
setup_signals(void)
{
  struct sigaction sa;
  
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);
  sa.sa_handler = sig_hup_handler;
  sigaction(SIGHUP, &sa, NULL);
  sa.sa_handler = sig_term_handler;
  sigaction(SIGTERM, &sa, NULL);
  sa.sa_handler = sig_term_handler;
  sigaction(SIGINT, &sa, NULL);
  sa.sa_handler = sig_child_handler;
  sigaction(SIGCHLD, &sa, NULL);

  /* Added by zhoucheng 2007-12-05 09:55 */
#ifdef HAVE_MERGE
  sa.sa_handler = sig_usr2_handler;
  sigaction(SIGUSR2, &sa, NULL);

  sa.sa_handler = sig_alarm_handler;
  sigaction(SIGALRM, &sa, NULL);

  /* Modified by zhoucheng 2008-01-18 10:13, bug 4185 */
  sa.sa_handler = sig_usr1_handler; 
  sigaction(SIGUSR1, &sa, NULL);

#endif

  sa.sa_handler = sig_segv_handler;
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGSEGV, &sa, NULL);

#ifdef _CENTEC_
  sa.sa_handler = sig_bus_handler;
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGBUS, &sa, NULL);
#endif
  
}

gboolean
stats_timer(gpointer st)
{
  stats_generate_log();
  return TRUE;
}

#ifdef DEBUG_MERGE
void debug_print(const char * str)
{
    int fd = 0;
    if ((fd = open("/dev/console", O_WRONLY | O_NONBLOCK, 0)) >= 0)
    {
         struct  tm * tmptm = NULL;
         gchar tmptime[32] = {0};
         gchar debug_str[128] = {0};
         time_t now_time;

         time(&now_time);
         tmptm = localtime(&now_time);
         strftime(tmptime, 32, "%X", tmptm);
         snprintf(debug_str, 128, "%s %s\n", tmptime, str);
        (void) write(fd, debug_str, strlen(debug_str));
        (void) close(fd);
    }
}
#endif

int 
main_loop_run(GlobalConfig **cfg)
{
  GMainLoop *main_loop;
  gint iters;
  guint stats_timer_id = 0;

#ifdef SYSLOG_NG_DEBUG
  int *pTmp = (int*)malloc(3*sizeof(int));
  pTmp = pTmp;
  int hd = -1;
#endif

#ifdef DEBUG_MEM_LEAK
  reset_debug_count();
#endif
  /*modified by weij for bug 19911, 2012-08-29: delete start message*/
  //msg_notice("logging starting up",  evt_tag_str("version", VERSION), NULL);
  main_loop = g_main_loop_new(NULL, TRUE);
  if ((*cfg)->stats_freq > 0)
    stats_timer_id = g_timeout_add((*cfg)->stats_freq * 1000, stats_timer, NULL);
  while (g_main_loop_is_running(main_loop))
    {
#ifdef SYSLOG_NG_DEBUG
    if ((hd=open("demo", O_RDONLY)) > 0)
    {
        printf("open demo success!\n");
        close(hd);
        return 0;
    }
#endif
      if ((*cfg)->time_sleep > 0)
        {
          struct timespec ts;
          
          ts.tv_sec = (*cfg)->time_sleep / 1000;
          ts.tv_nsec = ((*cfg)->time_sleep % 1000) * 1E6;
          
          nanosleep(&ts, NULL);
        }
      g_main_context_iteration(g_main_loop_get_context(main_loop), TRUE);
      if (sig_usr2_received)
        {
            /* workaround for bug23190, by zhaoy, 2013-6-8 */
            sig_usr2_received = FALSE;
            /* setting Time zone information */
            FILE *pFile = fopen(ZONE_CONFIG_PATH, "r");
            
            if (NULL != pFile)
            {
#define ZONE_LINE_BUF   256
                char szBuf[ZONE_LINE_BUF];
                char *newline_position;
                while (!feof(pFile))
                {
                    fgets(szBuf, ZONE_LINE_BUF, pFile);
                    if (memcmp(szBuf, "TZ=",3)==0 && strlen(szBuf)>3)
                    {
                        /* Should make a TZ=JST-01:00:00 style variable, never include
                           \n or \" character, otherwise tzset will wrongly set daylight saving info because of
                           the extra \n or \" in the end of line. */
                        newline_position = strchr(szBuf, '\n');
                        if (newline_position)
                        {
                            *newline_position = '\0';
                        }
                        setenv("TZ",szBuf+3,1);
                        tzset();
                        break;
                    }
                }
                fclose(pFile);
            }
                                    
#if 0 /* Deleted by Alexander Liu 2006-12-10 */
          msg_notice("SIGHUP received, reloading configuration", NULL);
#endif 

          /* Added by zhoucheng 2007-12-27 14:04, bug 3929 */
          if (configuration->merge_enable)
          {
              log_merge_sync_fifo_all();  
          }
#if 0
          /* Added by zhoucheng 2007-12-25 14:24, bug 3880 */
          g_main_context_iteration(g_main_loop_get_context(main_loop), TRUE);
#endif    
          (*cfg) = cfg_reload_config(cfgfilename, (*cfg));
          //sig_usr2_received = FALSE;
          if ((*cfg)->stats_freq > 0)
            {
              if (stats_timer_id != 0)
                g_source_remove(stats_timer_id);
              stats_timer_id = g_timeout_add((*cfg)->stats_freq * 1000, stats_timer, NULL);
            }
          stats_cleanup_orphans();
        }

      if (sig_hup_received) {
#ifdef HAVE_MERGE   
           if (configuration->merge_enable) {
              log_merge_sync_fifo_all();
           }
#endif
           (*cfg) = cfg_reload_config(cfgfilename, (*cfg));
           if ((*cfg)->stats_freq > 0) {
              if (stats_timer_id != 0) {
                g_source_remove(stats_timer_id);
                }
              stats_timer_id = g_timeout_add((*cfg)->stats_freq * 1000, stats_timer, NULL);
           }        
           sig_hup_received = 0;
      }
      
      if (sig_term_received)
        {
          msg_notice("SIGTERM received, terminating", NULL);
          sig_term_received = FALSE;
          break;
        }
      if (sig_child_received)
	{
	  pid_t pid;
	  int status;

          do
	    {
	      pid = waitpid(-1, &status, WNOHANG);
	      child_manager_sigchild(pid, status);
	    }
          while (pid > 0);
	  sig_child_received = FALSE;
	}
#ifdef _CENTEC_ /* Added by zhoucheng 2007-12-21 09:02 : bug 3846 */
#ifdef HAVE_MERGE   
    if (configuration->merge_enable && sig_usr2_received && 0)
    {           
        /* Modified by zhoucheng 2007-12-25 17:30, bug 3885*/
        log_merge_sync_fifo_all();  
        sig_usr2_received = FALSE;      
    }
    if (configuration->merge_enable && sig_alarm_received)        
    {
        log_merge_sync_fifo();            
#ifdef DEBUG_MERGE
        debug_print("sig_alarm received!");
#endif
        sig_alarm_received = FALSE;        
    }
#endif
#endif
}
    
  msg_notice("logging shutting down", 
             evt_tag_str("version", VERSION),
             NULL);
  iters = 0;
  while (g_main_context_iteration(NULL, FALSE) && iters < 3)
    {
      iters++;
    }
  g_main_loop_unref(main_loop);
  return 0;
}

gboolean
daemonize(void)
{
  pid_t pid;

  if (!do_fork)
    return 1;

  pid = fork();
  if (pid == 0) 
    {
      int fd;
      
      fd = open(pidfilename, O_CREAT | O_WRONLY | O_NOCTTY | O_TRUNC, 0600);
      if (fd != -1) 
	{
	  gchar buf[32];

	  g_snprintf(buf, sizeof(buf), "%d", (int) getpid());
	  write(fd, buf, strlen(buf));
	  close(fd);
	}
      return TRUE;
    }
  else if (pid == -1) 
    {
      msg_error("Error during fork",
                evt_tag_errno(EVT_TAG_OSERROR, errno),
                NULL);
      return FALSE;
    }
  exit(0);
}

void
setup_std_fds(gboolean log_to_stderr)
{
  int fd;

  if (do_fork)
    {
      fd = open("/dev/null", O_RDONLY);
      if (fd != -1)
	{
	  dup2(fd, STDIN_FILENO);
	  if (fd != STDIN_FILENO)
	    close(fd);
	}
      fd = open("/dev/null", O_WRONLY);
      if (fd != -1)
	{
	  dup2(fd, STDOUT_FILENO);
	  if (!log_to_stderr)
	    dup2(fd, STDERR_FILENO);
	  if (fd != STDOUT_FILENO && fd != STDERR_FILENO)
	    close(fd);
	}
      setsid();
    }
}

int
setup_creds(void)
{
  if (chroot_dir) 
    {
      if (chroot(chroot_dir) < 0) 
	{
	  msg_error("Error during chroot()",
	            evt_tag_errno(EVT_TAG_OSERROR, errno),
	            NULL);
	  return 0;
	}
    }

  if (uid || gid || run_as_user) 
    {
      setgid(gid);
      if (run_as_user)
      initgroups(run_as_user, gid);
      setuid(uid);
    }
  return 1;
}

#ifdef YYDEBUG
extern int yydebug;
#endif

static void
watch_child(char *argv[])
{
    char *prog;
    int failcount = 0;
    time_t start;
    time_t stop;
    int status;
    pid_t pid;
    int i;
    int nullfd;
    
    if (*(argv[0]) == '(')
    {
        return;
    }

    if ((pid = fork()) < 0)
    {
        msg_error("fork failed",  evt_tag_errno(EVT_TAG_OSERROR, errno), NULL);
    }
    else if (pid > 0)
    {
        exit(0);
    }

    if (setsid() < 0)
    {
        msg_error("setsid failed",  evt_tag_errno(EVT_TAG_OSERROR, errno), NULL);
    }
    
    if ((i = open("/dev/tty", O_RDWR)) >= 0) 
    {
        ioctl(i, TIOCNOTTY, NULL);
        close(i);
    }

    /*
     * RBCOLLINS - if cygwin stackdumps when squid is run without
     * -N, check the cygwin1.dll version, it needs to be AT LEAST
     * 1.1.3.  execvp had a bit overflow error in a loop..
     */
    /* Connect stdio to /dev/null in daemon mode */
    nullfd = open(_PATH_DEVNULL, O_RDWR);
    if (nullfd < 0)
    {
        msg_error("connect stdio to /dev/null failed",  evt_tag_errno(EVT_TAG_OSERROR, errno), NULL);
    }
    dup2(nullfd, 0);
    dup2(nullfd, 1);
    dup2(nullfd, 2);
    if (nullfd > 2)
    {
        close(nullfd);
    }
    for (;;) 
    {
        if ((pid = fork()) == 0) 
        {
            /* child */
            prog = strdup(argv[0]);
            argv[0] = strdup("(syslog-ng)");
            execvp(prog, argv);
            msg_error("execvp failed",  evt_tag_errno(EVT_TAG_OSERROR, errno), NULL);
        }
        
        /* parent */
        time(&start);
        signal(SIGINT, NULL);
        pid = waitpid(-1, &status, 0);
        time(&stop);

        if (stop - start < 10)
        {
            failcount++;
        }
        else
        {
            failcount = 0;
        }
        if (failcount == 5) 
        {
            msg_error("Exiting due to repeated, frequent failures", NULL);
            exit(1);
        }
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 0)
            {
                exit(0);
            }
        }
        if (WIFSIGNALED(status)) 
        {
            switch (WTERMSIG(status)) 
            {
            case SIGKILL:
                exit(0);
                break;
            default:
                break;
            }
        }
        signal(SIGINT, SIG_DFL);
        sleep(3);
    }

    /* NOTREACHED */
    return;
}

int 
main(int argc, char *argv[])
{
  GlobalConfig *cfg;

#if HAVE_GETOPT_LONG
  struct option syslog_ng_options[] = 
    {
      { "cfgfile", required_argument, NULL, 'f' },
      { "pidfile", required_argument, NULL, 'p' },
      { "debug", no_argument, NULL, 'd' },
      { "syntax-only", no_argument, NULL, 's' },
      { "verbose", no_argument, NULL, 'v' },
      { "foreground", no_argument, NULL, 'F' },
      { "help", no_argument, NULL, 'h' },
      { "version", no_argument, NULL, 'V' },
      { "chroot", required_argument, NULL, 'C' },
      { "user", required_argument, NULL, 'u' },
      { "group", required_argument, NULL, 'g' },
      { "stderr", no_argument, NULL, 'e' },
#ifdef YYDEBUG
      { "yydebug", no_argument, NULL, 'y' },
#endif
      { NULL, 0, NULL, 0 }
    };
#endif
  int syntax_only = 0;
  int log_to_stderr = 0;
  PersistentConfig *persist = NULL;
  const gchar *persist_file = PATH_PERSIST_CONFIG;
  int opt, rc;

#if HAVE_GETOPT_LONG
  while ((opt = getopt_long(argc, argv, "sFf:p:dvhyVC:u:g:e", syslog_ng_options, NULL)) != -1)
#else
  while ((opt = getopt(argc, argv, "sFf:p:dvhyVC:u:g:e")) != -1)
#endif
    {
      switch (opt) 
	{
	case 'f':
	  strncpy(cfgfilename, optarg, sizeof(cfgfilename));
	  break;
	case 'p':
	  strncpy(pidfilename, optarg, sizeof(pidfilename));
	  break;
	case 's':
	  syntax_only = 1;
	  break;
	case 'd':
	  debug_flag++;
	  break;
	case 'v':
	  verbose_flag++;
	  break;
	case 'e':
	  log_to_stderr = 1;
	  break;
	case 'F':
	  do_fork = 0;
	  break;
	case 'V':
#if 0
	  printf(PACKAGE " " VERSION "\n");
#endif
	  return 0;
	case 'C':
	  chroot_dir = optarg;
	  break;
	case 'u':
	  if (!resolve_user(optarg, &uid))
	    usage();
	  run_as_user = optarg;
	  break;
	case 'g':
	  if (!resolve_group(optarg, &gid))
	    usage();
	  break;
#ifdef YYDEBUG
	case 'y':
	  yydebug = 1;
	  break;
#endif
        case 'R':
          persist_file = optarg;
          break;
	case 'h':
	default:
	  usage();
	  break;
	}
      
    }
  
  
  z_mem_trace_init("syslog-ng.trace");
  tzset();
  setup_signals();
  msg_init(log_to_stderr);
  child_manager_init();

  if (do_fork)
  {
      watch_child(argv);
  }
  /* Added by zhoucheng for debug */
#ifdef SYSLOG_NG_DEBUG
{
    int flag = 0;
    do
    {
    }
    while (flag);
}
#endif
  cfg = cfg_new(cfgfilename);
  if (!cfg)
    {
      return 1;
    }

  if (syntax_only)
    {
      return 0;
    }

  persist = persist_config_new();
  persist_config_load(persist, persist_file);

  if (!cfg_init(cfg, persist))
    {
      return 2;
    }

#if 1
  /* do daemonize at watch_child
   * write pid file at here
   */
  {
      int fd;
      
      fd = open(pidfilename, O_CREAT | O_WRONLY | O_NOCTTY | O_TRUNC, 0600);
      if (fd != -1) 
      {
          gchar buf[32];
          
          g_snprintf(buf, sizeof(buf), "%d", (int) getpid());
          write(fd, buf, strlen(buf));
          close(fd);
      }
  }
#else
  if (!daemonize())
    {
      return 2;
    }
#endif

  /* from now on internal messages are written to the system log as well */
#ifndef SYSLOG_NG_DEBUG /* added by zhoucheng for debug */
  msg_syslog_started();
#endif  
  setup_creds();
  setup_std_fds(log_to_stderr);

  rc = main_loop_run(&cfg);
  cfg_deinit(cfg, persist);
  
  if (persist)
    {
      persist_config_save(persist, persist_file);
      persist_config_free(persist);
    }

  cfg_free(cfg);
  child_manager_deinit();
  msg_deinit();
  z_mem_trace_dump();
  return rc;
}

