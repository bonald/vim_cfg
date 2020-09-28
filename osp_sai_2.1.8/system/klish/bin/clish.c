/*
 * --------------------------------------
 * clish.c
 *
 * A console client for libclish
 * --------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <signal.h>
#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif
#include <ctype.h>

#include <sys/socket.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include "lub/list.h"
#include "lub/system.h"
#include "clish/shell.h"
#include "clish/internal.h"
#include "genlog.h"
#include "proto.h"
#include "ctc_sock.h"
#include "glb_tempfile_define.h"
#include "lib_fs.h"

extern int clish_auth(char *pszTty);
extern int clish_alloc_vty(char *pszTty);
extern void clish_del_login_sess(void);
extern int clish_get_vty_config(int *pnPrivilege, int *pnTimeout);
extern int isConsole(char *pszTty);
extern void loginout_audit(int mModule, int nServerity, char *pszMsg);
extern int clish_term_winsize_set (int sock, u_int16_t row, u_int16_t col);
extern int clish_term_winsize_get (int sock, u_int16_t *row, u_int16_t *col);
extern int clish_banner_show(char *mode);

extern u_int16_t g_term_lines;

#define QUOTE(t) #t
/* #define version(v) printf("%s\n", QUOTE(v)) */
#define version(v) printf("%s\n", v)

int glb_pri = 1;
int glb_timeout = 0;
int glb_is_rpc = BOOL_FALSE;
int glb_is_mnt_valid = BOOL_FALSE;

/* Hooks */
static clish_shell_hooks_t my_hooks = {
    NULL, /* don't worry about init callback */
    clish_access_callback,
    NULL, /* don't worry about cmd_line callback */
    clish_script_callback,
    NULL, /* don't worry about fini callback */
    clish_config_callback,
    clish_log_callback,
    NULL  /* don't register any builtin functions */
};

static void help(int status, const char *argv0);

#define STARTUP_CONFIG_BASE "/mnt/flash/"
#define FACTORY_CONFIG      "/mnt/flash/boot/.factory-config.conf"

typedef struct
{
    char *pszcmd; /* command string, show version etc */
} clish_ctc_show_diagnostic_t;

clish_ctc_show_diagnostic_t clish_ctc_show_diagnostic_cmd_array[] =
{
    {"show version"},
    {"show clock"},  
    {"show running-config"},
    {"show startup-config"},
    {"show reboot-info"},
    {"show memory summary total"},
    {"show memory all"},
    {"show cpu utilization"},
    {"show stm prefer current"},
    {"show file system"},
    {"ls"},
    {"show environment"},
    {"show transceiver detail"},
    {"show interface status"},
    {"show interface summary"},
    {"show interface"},
    {"show errdisable recovery"},
    {"show channel-group summary"},
    {"show mac-address-table"},
    {"show mac-address-table add-fdb-fail"},
//    {"show ip igmp groups"},
    {"show ip route summary"},
    {"show ip route"},
    {"show ip arp"},
//    {"show vrrp"},
#ifdef SAIPRODUCT
    {"show mlag peer"},
    {"show mlag interface"},
    {"show ip igmp snooping groups"},
#endif
#ifdef OFPRODUCT
    {"ovs-ofctl dump-meters br0 -O openflow13"},
    {"ovs-ofctl dump-flows br0 -O openflow13"},
    {"ovs-ofctl dump-groups br0 -O openflow13"},
    {"ovs-ofctl show br0"},
    {"show openflow controller status"},
#endif
    {"show logging buffer 1000"},
};

static int clish_do_show_diagnostic(clish_shell_t *shell)
{
    int size = sizeof(clish_ctc_show_diagnostic_cmd_array)/sizeof(clish_ctc_show_diagnostic_cmd_array[0]);
    int i = 0;

    for (i = 0; i < size; i++)
    {
        sal_fprintf(stdout, "\n------------------ %s ------------------\n\n", clish_ctc_show_diagnostic_cmd_array[i].pszcmd);
        sal_fflush(stdout);
        clish_shell_forceline(shell, clish_ctc_show_diagnostic_cmd_array[i].pszcmd, NULL);
    }
    
    return 0;
}

int
clish_process_startup_banner(char *line, FILE *fp)
{
    char *fname = NULL;
    FILE *banner_fp = NULL;
    char buf[1024];
    
    if (sal_strstr(line, "banner exec"))
    {
        fname = BANNER_EXEC_FILE;
    }
    else if (sal_strstr(line, "banner motd"))
    {
        fname = BANNER_MOTD_FILE;
    }
    else if (sal_strstr(line, "banner login"))
    {
        fname = BANNER_LOGIN_FILE;
    }
    else
    {
        fname = NULL;
        return 0;
    }

    banner_fp = sal_fopen(fname, "w+");
    if (NULL == banner_fp)
    {
        return -1;
    }
    while (NULL != sal_fgets(buf, 1024, fp))
    {
        if (sal_strstr(buf, BANNER_KEY_WORD))
        {
            sal_fclose(banner_fp);
            banner_fp = NULL;
            break;
        }
        sal_fprintf(banner_fp, "%s", buf);
    }
    if (banner_fp)
    {
        sal_fclose(banner_fp);
        banner_fp = NULL;
    }

    return -1;
}

int
clish_load_startup_config(clish_shell_t *shell, const char *startup_filename)
{
    /* Iterate lines */
    FILE *fp = NULL;
    char  startup_path[256];
    char  line[1024];
    const char *p = startup_filename;
    int rc = 0;
    int has_vlan_range = 0;
    int first_vlanif = 1;
    int factory_config = 0;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    
    while ('\0' != p[0])
    {
        if (isalpha(p[0]) || isdigit(p[0]) || '.' == p[0] || '/' == p[0] || '_' == p[0] || '-' == p[0])
        {
            break;
        }
        p++;
    }

    if ('/' != p[0])
    {
        /* append base path */
        snprintf(startup_path, 256, STARTUP_CONFIG_BASE"%s", p);
    }
    else
    {
        snprintf(startup_path, 256, "%s", p);
    }
    
    gettimeofday(&tv1, NULL);
    fp = fopen(startup_path, "r");
    if (!fp)
    {
        /* added by kcao for bug 46587 2018-06-07, load factory if has not startup-config */
        fp = fopen(FACTORY_CONFIG, "r");
        if (!fp)
        {
            /* modified by kcao for bug 33632, 2015-06-04 */
            /* should set startup_done, wait for interface up set to CCS, and then write to create new startup-config.conf */
            clish_shell_forceline(shell, "config-startup-done", NULL);
            sleep(3);
            printf("Not found startup config file: %s, create new one\n", startup_path);
            clish_shell_forceline(shell, "configure terminal", NULL);
            clish_shell_forceline(shell, "line vty 0 7", NULL);
            clish_shell_forceline(shell, "no login", NULL);
            clish_shell_forceline(shell, "privilege level 4", NULL);

#if 0   /* add by wus for bug52967 modify password "admin" in 2019-07-15*/
            clish_shell_forceline(shell, "login local", NULL);
            clish_shell_forceline(shell, "exit", NULL);
            clish_shell_forceline(shell, "line console 0", NULL);
            clish_shell_forceline(shell, "login local", NULL);
            clish_shell_forceline(shell, "privilege level 4", NULL);
            clish_shell_forceline(shell, "exit", NULL);
            clish_shell_forceline(shell, "username admin privilege 4 password admin", NULL);
            clish_shell_forceline(shell, "service http disable", NULL);
            clish_shell_forceline(shell, "service telnet disable", NULL);
            clish_shell_forceline(shell, "ip ssh server disable", NULL);
#endif /*wus end*/

            clish_shell_forceline(shell, "end", NULL);

            clish_shell_forceline(shell, "write", NULL);
            return 0;
        }
        else
        {
            factory_config = 1;
        }
    }

    clish_shell_forceline(shell, "configure terminal", NULL);
    while (NULL != fgets(line, 1024, fp))
    {
        if (sal_strstr(line, "vlan range"))
        {
            has_vlan_range = 1;
        }
        if ('!' == line[0])
        {
            /* back to config-mode */
            if (0 != clish_shell__get_depth(shell))
            {
                clish_shell_forceline(shell, "exit", NULL);
            }
            continue;
        }
        if (sal_strstr(line, "exit"))
        {
            /* Added by kcao for bug 46485 2018-04-02, already in config-mode, not exit to exec-mode */
            if (0 == clish_shell__get_depth(shell))
            {
                continue;
            }
        }
        /* Added by kcao for bug 46737, if has mass vlan config the first vlan interface maybe loss config for switch -> routed msg gather */
        if (has_vlan_range)
        {
            if (sal_strstr(line, "interface vlan"))
            {
                if (first_vlanif)
                {
                    sleep(3);
                    first_vlanif = 0;
                }
            }
        }
        rc = clish_process_startup_banner(line, fp);
        if (rc < 0)
        {
            continue;
        }
        rc = clish_shell_forceline(shell, line, NULL);
        if (rc < 0)
        {
            printf("load cmd failed: %s", line);
        }
    }
    clish_shell_forceline(shell, "end", NULL);
    clish_shell_forceline(shell, "config-startup-done", NULL);
    fclose(fp);

    gettimeofday(&tv2, NULL);
    usleep(5000);
    if (factory_config)
    {
        sleep(3); /* wait for all config sync to ccs process */
        printf("Not found startup config file: %s, create new one from factory-config\n", startup_path);
        clish_shell_forceline(shell, "write", NULL);
        log_sys(M_MOD_LIB, E_WARNING, "Load factory-config takes %.3f seconds", ((double)((tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec)))/1000000.0);
    }
    else
    {
        log_sys(M_MOD_LIB, E_WARNING, "Load startup-config takes %.3f seconds", ((double)((tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec)))/1000000.0);
    }
    
    return 0;
}

int
clish_load_config(clish_shell_t *shell, const char *startup_filename)
{
    /* Iterate lines */
    FILE *fp = NULL;
    char  startup_path[256];
    char  line[1024];
    const char *p = startup_filename;
    int rc = 0;

    while ('\0' != p[0])
    {
        if (isalpha(p[0]) || isdigit(p[0]) || '.' == p[0] || '/' == p[0] || '_' == p[0] || '-' == p[0])
        {
            break;
        }
        p++;
    }

    if ('/' != p[0])
    {
        /* append base path */
        snprintf(startup_path, 256, STARTUP_CONFIG_BASE"%s", p);
    }
    else
    {
        snprintf(startup_path, 256, "%s", p);
    }
    
    fp = fopen(startup_path, "r");
    if (!fp)
    {
        printf("Not found config file: %s\n", startup_path);    
        return 0;
    }

    clish_shell_forceline(shell, "configure terminal", NULL);
    while (NULL != fgets(line, 1024, fp))
    {
        if (strstr(line, "!"))
        {
            /* back to config-mode */
            if (0 != clish_shell__get_depth(shell))
            {
                clish_shell_forceline(shell, "exit", NULL);
            }
            continue;
        }
        rc = clish_shell_forceline(shell, line, NULL);
        if (rc < 0)
        {
            printf("load cmd failed: %s", line);
        }
    }
    clish_shell_forceline(shell, "end", NULL);
    fclose(fp);

    return 0;
}

clish_shell_t *g_shell = NULL;

clish_shell_t *get_global_shell(void)
{
    return g_shell;
}

extern void clish_shell__set_stat(clish_shell_t *this, clish_shell_state_t state);
extern int clish_signal_notify(int sig);
extern void __clish_pipe_term();
extern void ctc_kill_child();
void clish_sigterm (int sig)
{    
    printf ("\n%% Connection is closed by administrator!\n");
    clish_del_login_sess();
    /* removed by kcao for bug 37470
        loginout_audit(M_MOD_APP, E_ALERT, "logout");
    */
    clish_shell__set_stat(g_shell, SHELL_STATE_CLOSING);
    clish_signal_notify(SIGTERM);
    __clish_pipe_term();
    ctc_kill_child();
    return;
}

void clish_sighup(int sig)
{
    clish_del_login_sess();
    /* removed by kcao for bug 37470
    loginout_audit(M_MOD_APP, E_ALERT, "logout");
    */
    clish_shell__set_stat(g_shell, SHELL_STATE_CLOSING);
    clish_signal_notify(SIGHUP);
    __clish_pipe_term();
    ctc_kill_child();
    return;
}

void clish_sighquit(int sig)
{
    clish_del_login_sess();
    clish_shell__set_stat(g_shell, SHELL_STATE_CLOSING);
    clish_signal_notify(SIGQUIT);
    __clish_pipe_term();
    ctc_kill_child();
    return;
}

#define ZONE_CONFIG_PATH        "/tmp/zone.conf"

/* Added by kcao for bug 43747, sync timezone when klish start */
void
clish_set_timezone_ctc(void)
{
    FILE *readfile=NULL;
    char szBuf[256];
    char *newline_position;
    
    readfile = fopen(ZONE_CONFIG_PATH, "r");
    if (readfile == NULL)
    {
        return;
    }
    
    while (!feof(readfile))
    {
        fgets(szBuf, 256, readfile);
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

extern void setStartupCfgFlag(int onoff);
extern void setShowDiagFlag(int onoff);
extern int rsa_init_crypto();

static int g_session_sock = -1;

int clish_connect_login()
{
    pid_t spid = getpid();
    struct sockaddr_un raddr;
    int sock;
    char szBuf[16];
    
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    raddr.sun_family = AF_UNIX;
    snprintf(raddr.sun_path, sizeof(raddr.sun_path), "%s", GLB_CLI_SOCK_PATH);
    if (connect(sock, (struct sockaddr *)&raddr, sizeof(raddr))) {
        close(sock);
        return -1;
    }

    snprintf(szBuf, 16, "%u", spid);
    write(sock, szBuf, strlen(szBuf));
    g_session_sock = sock;
    return 0;
}

/*--------------------------------------------------------- */
extern int clish_shell__is_timeoutd(clish_shell_t *this);
int main(int argc, char **argv)
{
	int running;
	int result = -1;
	clish_shell_t *shell = NULL;
    char *curtty = ttyname(STDIN_FILENO);

	/* Command line options */
	const char *startup_filename = NULL;
	const char *config_filename = NULL;
	const char *socket_path = KONFD_SOCKET_PATH;
	bool_t lockless = BOOL_FALSE;
	bool_t stop_on_error = BOOL_FALSE;
	bool_t interactive = BOOL_TRUE;
	bool_t quiet = BOOL_FALSE;
	bool_t utf8 = BOOL_FALSE;
	bool_t bit8 = BOOL_FALSE;
	bool_t log = BOOL_FALSE;
	const char *xml_path = getenv("CLISH_PATH");
	const char *view = getenv("CLISH_VIEW");
	const char *viewid = getenv("CLISH_VIEWID");
	FILE *outfd = stdout;
	bool_t istimeout = BOOL_FALSE;
	int timeout = 0;
	bool_t cmd = BOOL_FALSE; /* -c option */
	lub_list_t *cmds; /* Commands defined by -c */
	lub_list_node_t *iter;
#if 1 /* fix bug 30043 */
    const char *histfile = NULL;
#else
	const char *histfile = "~/.clish_history";
#endif
	char *histfile_expanded = NULL;
    /* Modified by kcao for bug 32811, change limit to 32 */
	unsigned int histsize = 32;
	bool_t show_diag = BOOL_FALSE; /* -g option */
    u_int16_t row, col;

/* for hardware sshd env */
#if 1
#ifndef _GLB_UML_SYSTEM_
#ifndef _CTC_X86_64_
#ifdef HAVE_DEBIAN
    char temp_path[1024] = {0};
    strcpy(temp_path, "/switch/sbin:/switch/usr/sbin:/switch/bin:/switch/usr/bin:/switch/usr/local/openvswitch/bin:/sbin:/usr/sbin:/bin:/usr/bin");
    setenv("PATH", temp_path, 1);

    char temp_ld_library_path[1024] = {0};
#if (defined _CTC_ARM_LS1023A_)
    strcpy(temp_ld_library_path, "/lib/aarch64-linux-gnu:/usr/lib/aarch64-linux-gnu:/switch/lib:/switch/usr/lib");
#elif (defined _CTC_ARM_CTC5236_)
    strcpy(temp_ld_library_path, "/lib/aarch64-linux-gnu:/usr/lib/aarch64-linux-gnu:/switch/lib:/switch/usr/lib");
#else
    strcpy(temp_ld_library_path, "/lib/powerpc-linux-gnu:/usr/lib/powerpc-linux-gnu:/switch/lib:/switch/usr/lib");
#endif
    setenv("LD_LIBRARY_PATH", temp_ld_library_path, 1);

    const char *ovs_dir = getenv("OVS_DIR");
    if (!ovs_dir)
    {
        putenv("OVS_DIR=/usr/local/openvswitch");
    }
#else
    const char *path = getenv("PATH");
    const char *ovs_dir = getenv("OVS_DIR");
    if (path && !strstr(path, "/usr/local/openvswitch/bin"))
    {
        char temp_path[512] = {0};
        strcpy(temp_path, path);
        strcat(temp_path, ":/usr/local/openvswitch/bin");
        setenv("PATH", temp_path, 1);

        if (!ovs_dir)
        {
            putenv("OVS_DIR=/usr/local/openvswitch");
        }
    }

    /* python */
    setenv("PYTHONPATH", "/usr/lib/python2.7:/usr/lib/python2.7/site-packages:/usr/lib/python2.7/site-packages/rpc_server/lib/", 0);
    setenv("PYTHONHOME", "/usr", 0);
#endif
#else
    const char *path = getenv("PATH");
    char temp_path[1024] = {0};
    strcpy(temp_path, "/usr/local/ctc_python/bin:/switch/bin:/switch/usr/sbin:/switch/usr/bin:");
    strcat(temp_path, path);
    strcat(temp_path, ":/switch/usr/local/openvswitch/bin");
    setenv("PATH", temp_path, 1);

    const char *ld_library_path = getenv("LD_LIBRARY_PATH");
    char temp_ld_library_path[1024] = {0};
    strcpy(temp_ld_library_path, "/usr/local/ctc_python/:/usr/local/ctc_python/lib/python2.7/:/usr/local/ctc_python/lib/python2.7/site-packages/:/usr/local/ctc_python/lib/python2.7/lib-dynload/");
    strcat(temp_ld_library_path, ":/switch/lib/:");
    strcat(temp_ld_library_path, ld_library_path);
    strcat(temp_ld_library_path, ":/lib64/:/usr/lib/:/lib/");
    setenv("LD_LIBRARY_PATH", temp_ld_library_path, 1);

    setenv("PYTHONPATH", "/usr/local/ctc_python/:/usr/local/ctc_python/lib/python2.7/:/usr/local/ctc_python/lib/python2.7/site-packages/:/usr/local/ctc_python/lib/python2.7/lib-dynload/", 0);
    setenv("PYTHONHOME", "/usr/local/ctc_python/", 0);
#endif
#endif
#endif
    clish_set_timezone_ctc();

    rsa_init_crypto();
    {
        /* move to vrf 0 */
        int vrf = 0;
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
#ifndef SO_PERMVRF
#define SO_PERMVRF      101
#endif
	    setsockopt(sock, SOL_SOCKET, SO_PERMVRF, (u_int8_t *)&vrf, sizeof(vrf));
	    close(sock);
    }

    /* Added by Alexander Liu */
    ioctl(0, TIOCSCTTY, 1);
    
	/* Signal vars */
	struct sigaction sigpipe_act;
	sigset_t sigpipe_set;

	static const char *shortopts = "hvs:r:ledx:w:i:bqu8okt:c:f:z:gpC:";
#ifdef HAVE_GETOPT_H
	static const struct option longopts[] = {
		{"help",	0, NULL, 'h'},
		{"version",	0, NULL, 'v'},
		{"startup",	1, NULL, 's'},
		{"source",	1, NULL, 'r'},
		{"lockless",	0, NULL, 'l'},
		{"stop-on-error", 0, NULL, 'e'},
		{"dry-run",	0, NULL, 'd'},
		{"xml-path",	1, NULL, 'x'},
		{"view",	1, NULL, 'w'},
		{"viewid",	1, NULL, 'i'},
		{"background",	0, NULL, 'b'},
		{"quiet",	0, NULL, 'q'},
		{"utf8",	0, NULL, 'u'},
		{"8bit",	0, NULL, '8'},
		{"log",		0, NULL, 'o'},
		{"check",	0, NULL, 'k'},
		{"timeout",	1, NULL, 't'},
		{"command",	1, NULL, 'c'},
        {"klishcommand", 1, NULL, 'C'},
		{"histfile",	1, NULL, 'f'},
		{"histsize",	1, NULL, 'z'},
		{"diag",	1, NULL, 'g'},
		{"rpc",	0, NULL, 'p'},
		{NULL,		0, NULL, 0}
	};
#endif

	/* Ignore SIGPIPE */
	sigemptyset(&sigpipe_set);
	sigaddset(&sigpipe_set, SIGPIPE);
	sigpipe_act.sa_flags = 0;
	sigpipe_act.sa_mask = sigpipe_set;
	sigpipe_act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sigpipe_act, NULL);
	signal(SIGTERM, clish_sigterm);
	signal(SIGHUP, clish_sighup);
	signal(SIGQUIT, clish_sighquit);

#if HAVE_LOCALE_H
	/* Set current locale */
	setlocale(LC_ALL, "");
#endif

    if (3 == argc)
    {
        if (!strcmp(argv[1], "-h"))
        {
            argc = 1;
        }
    }

	/* Var initialization */
	cmds = lub_list_new(NULL);

	/* Parse command line options */
	while(1) {
		int opt;
#ifdef HAVE_GETOPT_H
		opt = getopt_long(argc, argv, shortopts, longopts, NULL);
#else
		opt = getopt(argc, argv, shortopts);
#endif
		if (-1 == opt)
			break;
		switch (opt) {
		case 's':
#if 1
            startup_filename = optarg;
#else
			socket_path = optarg;
#endif
            setStartupCfgFlag(1);
			break;
		case 'r':
            /* When r opt is present, we will not use s opt, then
               clish will do the user auth for us, we can then load
               configuration successfully. */
            config_filename = optarg;
			break;
		case 'l':
			lockless = BOOL_TRUE;
			break;
		case 'e':
			stop_on_error = BOOL_TRUE;
			break;
		case 'b':
			interactive = BOOL_FALSE;
			break;
		case 'q':
			quiet = BOOL_TRUE;
			break;
		case 'u':
			utf8 = BOOL_TRUE;
			break;
		case '8':
			bit8 = BOOL_TRUE;
			break;
		case 'o':
			log = BOOL_TRUE;
			break;
		case 'd':
			my_hooks.script_fn = clish_dryrun_callback;
			break;
		case 'x':
			xml_path = optarg;
			break;
		case 'w':
			view = optarg;
			break;
		case 'i':
			viewid = optarg;
			break;
		case 'k':
			lockless = BOOL_TRUE;
			my_hooks.script_fn = clish_dryrun_callback;
			my_hooks.config_fn = NULL;
			break;
		case 't':
			istimeout = BOOL_TRUE;
			timeout = atoi(optarg);
			break;
        case 'c': /* Modified by Alexander Liu for netconf 2018-04-25 14:42 */
            { /* XXX: ssh subsystem will call kilsh -c xxx */
                char *argv[4];
                extern char **environ;
                
                argv[0] = "sh";
                argv[1] = "-c";
                argv[2] = optarg;
                argv[3] = NULL;
                execve("/bin/sh", argv, environ);
                exit (1);
            }
            break;

		case 'C': {
				char *str;
				cmd = BOOL_TRUE;
				quiet = BOOL_TRUE;
				str = strdup(optarg);
				lub_list_add(cmds, str);
			}
			break;
		case 'f':
			if (!strcmp(optarg, "/dev/null"))
				histfile = NULL;
			else
				histfile = optarg;
			break;
		case 'z': {
				int itmp = 0;
				itmp = atoi(optarg);
				if (itmp <= 0) {
					fprintf(stderr, "Error: Illegal histsize option value.\n");
					help(-1, argv[0]);
					goto end;
				}
				histsize = itmp;
			}
			break;
		case 'h':
			help(0, argv[0]);
			exit(0);
			break;
		case 'v':
			version(VERSION);
			exit(0);
			break;
		case 'g':
			setShowDiagFlag(BOOL_TRUE);
			show_diag = BOOL_TRUE;
			quiet = BOOL_TRUE;
			break;
		case 'p':
			setLogOperCMDLevel(LOG_OPER_CMD_LEVEL_JSON_PRC);
            glb_is_rpc = BOOL_TRUE;
            //printf("RPC is %d",glb_is_rpc);
			break;
		default:
			help(-1, argv[0]);
			goto end;
			break;
		}
	}

    if (isConsole(curtty))
    {
        glb_is_mnt_valid = BOOL_TRUE;
    }

    if (glb_is_mnt_valid)
    {
        chdir("/mnt");
    }
    else
    {
        chdir("/mnt/flash");
    }

    extern int isStartupCfg();
    if (!isStartupCfg() && isConsole(curtty)) {
        char szPrompt[256];
        char g_buf[32];
        int nr;
        snprintf(szPrompt, 256, "\n\nCon0 is now available.\nPress RETURN to get started.\n\n");
        printf("%s", szPrompt);
        while(1) {
            nr = cli_readpassphrase("", g_buf, 32, 
                    CLI_RPP_ECHO_ON | CLI_RPP_CONFIRM_MODE, 0);
            if (nr >= 0 && '\0' == g_buf[0]) {
                printf("\n");
                break;
            }

        }
    }

	/* Validate command line options */
	if (utf8 && bit8) {
		fprintf(stderr, "The -u and -8 options can't be used together.\n");
		goto end;
	}

    /* Added by Alexander 2014-10-30 11:29 */
    /* Try to allocate vty for this client */
    if ((NULL == startup_filename) && NULL != curtty && !show_diag) {
       /* Note: when loading config, we do not need to alloc vty since we are execute 'source'
        * command in a klish, the vty is inherited from the parent klish session, and we only 
        * need to get the priviledge. */
        if (NULL == config_filename) {
            if (clish_alloc_vty(curtty)) {
                exit (1);
            }
        }
        /* connect to login module for detect CLI abnormal exit */
        clish_connect_login();
    }

	/* Create shell instance */
	if (quiet)
		outfd = fopen("/dev/null", "w");
	g_shell = shell = clish_shell_new(&my_hooks, NULL, NULL, outfd, stop_on_error);
	if (!shell) {
		fprintf(stderr, "Can't run clish.\n");
		goto end;
	}
	/* Load the XML files */
	clish_shell_load_scheme(shell, xml_path);
	/* Set communication to the konfd */
	clish_shell__set_socket(shell, socket_path);
	/* Set lockless mode */
	if (lockless)
		clish_shell__set_lockfile(shell, NULL);
	/* Set interactive mode */
	if (!interactive)
		clish_shell__set_interactive(shell, interactive);
	/* Set startup view */
	if (view)
		clish_shell__set_startup_view(shell, view);
	/* Set startup viewid */
	if (viewid)
		clish_shell__set_startup_viewid(shell, viewid);
	/* Set UTF-8 or 8-bit mode */
	if (utf8 || bit8)
		clish_shell__set_utf8(shell, utf8);
	else {
#if HAVE_LANGINFO_CODESET
		/* Autodetect encoding */
		if (!strcmp(nl_langinfo(CODESET), "UTF-8"))
			clish_shell__set_utf8(shell, BOOL_TRUE);
#else
		/* The default is 8-bit if locale is not supported */
		clish_shell__set_utf8(shell, BOOL_FALSE);
#endif
	}
	/* Set logging */
	if (log)
		clish_shell__set_log(shell, log);

    /* Added by Alexander: 2014-10-28 */
    if (NULL == startup_filename && curtty && !show_diag) {
       /* Note: when loading config, we do not need to alloc vty since we are execute 'source'
        * command in a klish, the vty is inherited from the parent klish session, and we only 
        * need to get the priviledge. */
        if (NULL == config_filename) {
            /*modify by zhaoqd for bug 53533 2019-09-20*/
            int acl_ret = 0;
            acl_ret = clish_auth(curtty);
            if (-1 == acl_ret)
            {
                loginout_audit(M_MOD_APP, E_ALERT, "login failed (authentication error)");
                goto end;
            }
            else if (-2 == acl_ret)
            {
                loginout_audit(M_MOD_APP, E_ALERT, "login failed (acl deny)");
                goto end;
            }   
        }
        clish_get_vty_config(&glb_pri, &glb_timeout);
        if (glb_timeout > 0) {
            timeout = glb_timeout;
        } else if (glb_timeout == 0) {
            timeout = INT_MAX;
        }
        if(!glb_is_rpc)/*add by chenyq for bug 43331, disable timeout for rpc vty*/
        {
            istimeout = 1;
        }
        else
        {
            /* Fix bug 46913, change colonum to 1024 */
            clish_term_winsize_get(0, &row, &col);
            col = 1024;
            row = 0;
            g_term_lines = 0;
            clish_term_winsize_set(0, row, col);
            glb_pri = 4;
        }
        /* removed by kcao for bug 37470
        loginout_audit(M_MOD_APP, E_ALERT, "login success");
        */
        extern int clish_session_update_login_user(void);
        clish_session_update_login_user();
    }

	/* Set idle timeout */
	if (istimeout)
		clish_shell__set_timeout(shell, timeout);
	/* Set history settings */
	clish_shell__stifle_history(shell, histsize);
	if (histfile)
		histfile_expanded = lub_system_tilde_expand(histfile);
	if (histfile_expanded)
		clish_shell__restore_history(shell, histfile_expanded);

	/* Set source of command stream: files or interactive tty */
	if(optind < argc) {
		int i;
		/* Run the commands from the files */
		for (i = argc - 1; i >= optind; i--)
			clish_shell_push_file(shell, argv[i], stop_on_error);
	} else {
		/* The interactive shell */
		clish_shell_push_fd(shell, fdopen(dup(fileno(stdin)), "r"),
			stop_on_error);
	}

    if (!isStartupCfg() && !glb_is_rpc)
    {
        clish_banner_show("exec");
    }
    
	/* Execute startup */
	running = clish_shell_startup(shell);
	if (running) {
		fprintf(stderr, "Can't startup clish.\n");
		goto end;
	}

	if (cmd) {
		/* Iterate cmds */
		glb_pri = 4; /* add by liul for bug44399, 'klish -c' can excute all cmds */
		for(iter = lub_list__get_head(cmds);
			iter; iter = lub_list_node__get_next(iter)) {
			char *str = (char *)lub_list_node__get_data(iter);
			clish_shell_forceline(shell, str, NULL);
		}
	} else if (show_diag) {
        clish_do_show_diagnostic(shell);
        exit(0);
	} else if (startup_filename) {
        clish_load_startup_config(shell, startup_filename);
	} else if (config_filename) {
        clish_load_config(shell, config_filename);
	}
    else {
		/* Main loop */
		if (!isConsole(curtty)) {
		    log_sys(M_MOD_APP, E_INFORMATIONAL, "ready to service");
		}
		result = clish_shell_loop(shell);
    /* removed by kcao for bug 37470
    loginout_audit(M_MOD_APP, E_ALERT, "logout");
    */
    }

end:
    if (isConsole(curtty) && clish_shell__is_timeoutd(shell)) {
        istimeout = BOOL_FALSE;
    } else {
        istimeout = BOOL_TRUE;
    }
    
	/* Cleanup */
	if (shell) {
		if (histfile_expanded) {
			clish_shell__save_history(shell, histfile_expanded);
			free(histfile_expanded);
		}
		clish_shell_delete(shell);
	}
	if (quiet)
		fclose(outfd);

	/* Delete each cmds element */
	while ((iter = lub_list__get_head(cmds))) {
		lub_list_del(cmds, iter);
		free(lub_list_node__get_data(iter));
		lub_list_node_free(iter);
	}
	lub_list_free(cmds);

    clish_del_login_sess();

    if (BOOL_FALSE == istimeout) {
        char g_leave[32];
        int nr;
        while (1) {
            nr = cli_readpassphrase("", g_leave, 32, 
                    CLI_RPP_ECHO_ON | CLI_RPP_CONFIRM_MODE, 0);
            if (nr >= 0 && '\0' == g_leave[0]) {
                break;
            }
        }

    }
	return result;
}

/*--------------------------------------------------------- */
/* Print help message */
static void help(int status, const char *argv0)
{
	const char *name = NULL;

	if (!argv0)
		return;

	/* Find the basename */
	name = strrchr(argv0, '/');
	if (name)
		name++;
	else
		name = argv0;

	if (status != 0) {
		fprintf(stderr, "Try `%s -h' for more information.\n",
			name);
	} else {
		printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
		printf("CLI utility. Command line shell."
			"The part of the klish project.\n");
		printf("Options:\n");
		printf("\t-v, --version\tPrint version.\n");
		printf("\t-h, --help\tPrint this help.\n");
		printf("\t-s <filename>, --startup=<filename>\tLoad startup-config "
			"\n\t\tof the konfd daemon.\n");
        printf("\t-r <filename>, --source=<filename>\tLoad config explicitly.\n");
		printf("\t-l, --lockless\tDon't use locking mechanism.\n");
		printf("\t-e, --stop-on-error\tStop script execution on error.\n");
		printf("\t-b, --background\tStart shell using non-interactive mode.\n");
		printf("\t-q, --quiet\tDisable echo while executing commands\n\t\tfrom the file stream.\n");
		printf("\t-d, --dry-run\tDon't actually execute ACTION scripts.\n");
		printf("\t-x <path>, --xml-path=<path>\tPath to XML scheme files.\n");
		printf("\t-w <view_name>, --view=<view_name>\tSet the startup view.\n");
		printf("\t-i <vars>, --viewid=<vars>\tSet the startup viewid variables.\n");
		printf("\t-u, --utf8\tForce UTF-8 encoding.\n");
		printf("\t-8, --8bit\tForce 8-bit encoding.\n");
		printf("\t-o, --log\tEnable command logging to syslog's local0.\n");
		printf("\t-k, --check\tCheck input files for syntax errors only.\n");
		printf("\t-t <timeout>, --timeout=<timeout>\tIdle timeout in seconds.\n");
		printf("\t-C <klishcommand>, --klishcommand=<command>\tExecute specified command(s).\n\t\tMultiple options are possible.\n");
        printf("\t-c <command>, --command=<command>\tExecute specified command(s) like sh -c.\n");
        printf("\t-f <path>, --histfile=<path>\tFile to save command history.\n");
        printf("\t-z <num>, --histsize=<num>\tCommand history size in lines.\n");
	}
}
