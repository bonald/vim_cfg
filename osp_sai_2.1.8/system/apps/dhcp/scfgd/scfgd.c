/****************************************************************************
* $Id$
*  Smart Config Implements
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-23 11:19
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "pal.h"
#include "thread.h"
#include "lib.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include "log.h"

#include "dhc_common.h"
#include "scfg_client.h"
#include "scfg_popen.h"
#include "chsm_client.h"
#include "glb_hw_define.h"
#include "SCfgMsg.h"
#include "scfg_client_cb.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#ifndef FULL_NAME_FLASH
#define FULL_NAME_FLASH "/mnt/flash"
#endif

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

struct lib_globals *scfg_zg = NULL;

extern void protocol_module_init_bitmap();

#ifdef HAVE_GETOPT_H
static struct option longopts[] =
{
    { "daemon",      no_argument,       NULL, 'd'},
    { "help",        no_argument,       NULL, 'h'},
    { 0 }
};
#endif /* HAVE_GETOPT_H */

/****************************************************************************
 *
* Function
* 
****************************************************************************/

/* Help information display. */
void scfg_usage (char *progname)
{
    printf ("Usage : %s [OPTION...]\n\
    SmartConfig Daemon\n\n\
    -d, --daemon       Runs in daemon mode\n\
    -h, --help         Display this help and exit\n",
    progname);
    exit (-1);
}

/* SIGHUP handler. */
void sighup (int sig)
{
}

/* SIGINT handler. */
void sigint (int sig)
{
    log_sys(M_MOD_SCFG, E_INFORMATIONAL, "Shutdown by SIGINT");
    exit (0);
}

/* SIGUSR1 handler. */
void sigusr1 (int sig)
{
}

void sigchild (int sig)
{
    int32 save_errno = errno;
    pid_t pid;
    int32 status;

    status = 0;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0 
            || (pid < 0 && errno == EINTR))
    {
        if (pid > 0)
        {
            scfg_process_sigchild(pid, WEXITSTATUS(status));
        }
    }

    errno = save_errno;
    return;
}

/* Initialization of signal handles.  */
void signal_init ()
{
    pal_signal_init ();
    pal_signal_set (SIGHUP, sighup);
    pal_signal_set (SIGINT, sigint);
    pal_signal_set (SIGTERM, sigint);
    pal_signal_set (SIGUSR1, sigusr1);
    pal_signal_set (SIGCHLD, sigchild);
}

int scfg_start(int daemon)
{
    struct thread thread;
    result_t ret;

    memory_init (IPI_PROTO_DHCLIENT); /* fake it */

    /* Allocate lib globals. */
    if (NULL == (scfg_zg = lib_create ()))
    {
        fprintf(stderr, "%% Start SmartConfig failed: out of memory\n");
        exit(-1);
    }

    ret = lib_start (scfg_zg);
    if (ret < 0)
    {
        lib_stop (scfg_zg);
        return ret;
    }

    /* Fake default entity. we will not use CLIs */ 
    scfg_zg->protocol = IPI_PROTO_DHCLIENT;
    scfg_zg->log = openzlog (scfg_zg, 0, IPI_PROTO_DHCLIENT, LOGDEST_MAIN);
    scfg_zg->master = thread_master_create ();

    protocol_module_init_bitmap();

    if (daemon)
        pal_daemonize(0, 0);

    scfg_popen_init();

    if (chsm_client_init(scfg_zg) < 0)
    {
        log_sys(M_MOD_SCFG, E_ERROR, "Initialize chsm client failed. Shutdown...");
        exit (-1);
    }  
    if (!scfg_init_deploy_start())
    {
        log_sys(M_MOD_SCFG, E_ERROR, "Initialize system environment failed");
        exit(-1);
    }

    scfg_client_module_init(scfg_zg);

    log_sys(M_MOD_SCFG, E_INFORMATIONAL, "Ready to service");
    while (thread_fetch (scfg_zg, &thread))
    {
        thread_call (&thread);
    }
    return 0;
}

/* Main routine of scfgd. */
int main (int argc, char **argv)
{
    result_t ret;
    char *p;
    s_int32_t daemon_mode = 0;
    char *progname;

    /* Set umask before anything for security */
    umask (0027);

    /* Get program name. */
    progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
    /* Command line option parse. */
    while (1)
    {
        int opt;

#ifdef HAVE_GETOPT_H
        opt = getopt_long (argc, argv, "dh", longopts, 0);
#else
        opt = getopt (argc, argv, "dh");
#endif /* HAVE_GETOPT_H */

        if (opt == EOF)
            break;

        switch (opt)
        {
            case 0:
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 'h':
                scfg_usage (progname);
                break;
            default:
                scfg_usage (progname);
                break;
        }
    }

    /* Initialize signal.  */
    signal_init ();

    /* Start 802.1X module.  */
    ret = scfg_start (daemon_mode);

    /* Not reached. */
    exit (ret);
}

#ifdef _GLB_DISTRIBUTE_SYSTEM_
//#error "SmartConfig can't support distribute system at present!!!"
#endif

