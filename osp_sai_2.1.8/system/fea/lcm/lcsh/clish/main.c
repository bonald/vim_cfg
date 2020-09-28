#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "cli.h"
#include "clish.h"
#include "clish_cli.h"

#include "clish_exec.h"
#include "clish_filesystem.h"
#include "lc_define.h"
#include "genlog.h"
#include "lcsh_client.h"
#ifdef CNOS_OS
#include "ctc_task.h"
#endif

static clish_master_t *g_p_clish_master = NULL; 
#ifdef CNOS_OS
ctc_task_t *g_tty_timeout_thread = NULL;
#define LCM_TASK_FUNC(_FUNC_)       \
void _FUNC_(void * p_data)
#define LCM_TASK_RETURN(err)    return
#else
ctclib_thread_t *g_tty_timeout_thread = NULL;
#define LCM_TASK_FUNC(_FUNC_)       \
int32 _FUNC_(thread_t * p_data)

#define LCM_TASK_RETURN(err)    return err
#endif

struct cli g_stcli;
static struct cli_tree *cli_tree = NULL;
struct globals g_globals;
int g_bDoStartupCfg = 0;
static pid_t g_oldpid = -1;
static struct termios g_oldtermios; /* save tty and restore it atexit */

//static afx_timer_t *g_pTimer = NULL;
//static afx_mio_t *g_pTtyRead = NULL;
//static afx_mio_t *g_pSigRead = NULL;
//static afx_mio_t *g_pCfmRead = NULL;
TOFunction *rl_timeout_function = NULL;

char *input_line = NULL; /* for clish_parse_line */

#define M_CLIENT_IP_LEN 256
char g_szClientIp[M_CLIENT_IP_LEN] = {0};

extern unsigned short clish_win_row_orig;
extern unsigned short clish_win_row;
extern unsigned short clish_win_col;

extern int term_winsize_get (int sock,
		      unsigned short  *row, unsigned short  *col);
extern void clish_signal_init ();

extern int clish_cli_init(struct cli_tree *cli_tree);
extern void clish_process_signal(void);
#ifdef CNOS_OS
void clish_loop_init();
#else
void clish_loop_init(void * p_master);
#endif

/* Help information display. */
static void
usage (int status, char *progname)
{
  if (status != 0)
    printf ("Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\n\
Daemon which manages kernel routing table management and \
redistribution between different routing protocols.\n\n\
-f, --file               Execute this config file\n\
-u, --user               Login user\n\
-H, --help               Display this help and exit\n\
-h, --host               Specify the remote host ip\n\
-s, --startup            Loading startup configuration file\n\
\n\
Report bugs to %s\n", progname, "marketing@centecnetworks.com");
    }
  exit (status);
}

/* VTY shell options, we use GNU getopt library. */
struct option longopts[] = 
{
  { "file",                 required_argument,       NULL, 'f'},
  { "user",                 required_argument,       NULL, 'u'},
  { "help",                 no_argument,             NULL, 'H'},
  { "host",                 required_argument,       NULL, 'h'},
  { "startup",              required_argument,       NULL, 's'},
  { 0 }
};

static char *clish_no_completion(const char *ignore, int invoking_key)
{
    return NULL;
}

int
fd_set_nonblocking (int fd, int on)
{
    int ret;

    ret = ioctl(fd, FIONBIO, (void *) &on);
    if (ret < 0) 
    {
        return -1;
    }

    return 0;
}


static char *__gen_compl(const char *text, int state)
{
    static char **matches;
    static int next;

    if (state == 0) {
        if (rl_point != rl_end)
        {
            matches = NULL;
        }
        else
        {
            char *p = rl_line_buffer;
            SKIP_WHITE_SPACE(p);
            if (g_stcli.mode > EXEC_MODE && !strncasecmp(p, "do ", 3))
            {
                p += 3;
                matches = cli_complete(cli_tree, EXEC_MODE, g_stcli.privilege, p);
            } else
            {
                matches = cli_complete(cli_tree, g_stcli.mode, g_stcli.privilege, rl_line_buffer);
            }
        }
        next = 0;

        if (gen_partly_match())
        {
            rl_completion_append_character = 0;
        }
    }

    if (matches)
        return matches[next++];
    else
        return NULL;
}

static char **clish_completion(const char *text, int start, int end)
{
    return rl_completion_matches(text, __gen_compl);
}

static int clish_describe_func(void *val, int argc, char **argv)
{
    struct cli _cli;
    char *p = (char *)rl_line_buffer;

    memset(&_cli, 0, sizeof (struct cli));
    _cli.out_func = (CLI_OUT_FUNC)fprintf;
    _cli.out_val = stdout;
    
    SKIP_WHITE_SPACE(p);
    if (g_stcli.mode > EXEC_MODE 
        && !strncasecmp("do ", rl_line_buffer, strlen("do ")))
    {
        _cli.mode = EXEC_MODE;
        p += 3;
        cli_describe(&_cli, cli_tree, _cli.mode, g_stcli.privilege, p, 80);
    } else
    {
        cli_describe(&_cli, cli_tree, g_stcli.mode, g_stcli.privilege, rl_line_buffer, 80);
    }
    return 0;
}

static int clish_possible_completions(int arg1, int arg2)
{
    struct cli *cli = &g_stcli;

    clish_more(clish_describe_func, cli, 0, NULL);
    rl_on_new_line();
    return 0;
}

int clish_login(struct cli *cli)
{
	return 0;

}

void 
clish_print_version()
{
    return;
}

char *
get_current_logindir(void)
{
    return g_globals.logindir;
}

void clish_load_config_file(char *filename)
{
    FILE *fp;
    char buf[CLISH_BUFSIZE + 1];
    struct cli *cli = &g_stcli;

    cli->mode = CONFIG_MODE;

    fp = fopen (filename, "r");
    if (fp == NULL)
        return;

    while (fgets (buf, CLISH_BUFSIZE, fp))
    {
        char *p = buf + strlen(buf) - 1;
        if (p >= buf && '\n' == *p)
        {
            *p = '\0';
            p--;
        }
        if (p >= buf && '\r' == *p)
        {
            *p = '\0';
            p--;
        }

        if (p < buf || strlen(buf) <= 0)
        {
            continue;
        }

        clish_parse (buf, EXEC_MODE);
        clish_process_signal();
        usleep(50000); /* delay 50 ms */
    }
    
    if (NULL != fp)
    {
        fclose(fp);
    }
    
    return ;
}

int clish_start(char *filename, char *username)
{
#ifndef _SYS_LC_
    int ret = 0;
    int conTimeout;
    int vtyTimeout;
#endif
    char szCmd[PATH_MAX];

    // TODO: _GLB_DISTRIBUTE_SYSTEM_
    if (lcsh_clnt_start() < 0) 
    {
        fprintf(stderr, "Connecting to CFM server failed!\n");
        return -1;
        // TODO: return -1;
    }

    //port_expand_module_init();

    memset(&g_stcli, 0, sizeof (struct cli));
    g_stcli.out_func = (CLI_OUT_FUNC)fprintf;
    g_stcli.out_val = stdout;
    g_stcli.mode = EXEC_MODE;
    g_stcli.privilege = PRIVILEGE_MAX;
    g_stcli.lines = -1;

    memset(&g_globals, 0x00, sizeof(struct globals));
    strcpy(g_globals.username, M_SYSTEM_ADMIN_NAME);
    strcpy(g_globals.logindir, FULL_NAME_FLASH);
    g_globals.timeout = LINE_TIMEOUT_DEFAULT_SEC; /* default five minutes */
    
    /* Init winsize. */
    term_winsize_get (STDIN_FILENO, &clish_win_row, &clish_win_col);
    clish_win_row_orig = clish_win_row;
    clish_win_row = 0xFFFF; /* meaings terminal rows not changed */
    clish_signal_init();

    cli_tree = cli_tree_new();
    clish_cli_init(cli_tree);
    
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_completion_entry_function = clish_no_completion;
    rl_attempted_completion_function = clish_completion;
    rl_bind_key('?', clish_possible_completions);

    if (filename != NULL)
    {
        clish_load_config_file(filename);
        exit(0);
    }

#ifndef _SYS_LC_
    ret = clish_login(&g_stcli);
    if (ret != 0)
    {
        exit(0);
    }

    cfm_get_exec_timeout(cfm_clnt, &conTimeout, &vtyTimeout);
    tty = ttyname (STDIN_FILENO);
    if (tty && strstr (tty, "console"))
        g_globals.timeout = conTimeout; 
    else
        g_globals.timeout = vtyTimeout; 
#endif

    /* login success, let chdir */
    if (!syscmd_is_directory(get_current_logindir()))
    {
        if (syscmd_file_exist(get_current_logindir()))
        {
            fprintf(stderr, "%% Can't access login directory\n");
            exit(5);
        }

        snprintf(szCmd, PATH_MAX, "mkdir -p %s", get_current_logindir());
        system(szCmd);
    }

    if (0 != chdir(get_current_logindir()))
    {
        fprintf(stderr, "%% Access login directory failed: %s\n",
                strerror(errno));
        exit(6);
    }

    clish_print_version();
    rl_callback_handler_install(clish_prompt(), clish_parse_line);
    stifle_history(32);

    /* initialize */
#ifdef CNOS_OS
    clish_loop_init();
#else
    clish_loop_init((void *)g_p_clish_master->p_thread_master);
#endif

    return 0;
}

void
save_tty_settings(void)
{
    g_oldpid = getpid();
    tcgetattr(STDIN_FILENO, &g_oldtermios);
    return;
}

void
clish_atexit(void)
{
    int flags;

    if (getpid() == g_oldpid)
    {
        flags = fcntl(fileno(stderr), F_GETFL, 0);
        flags &= ~O_NONBLOCK;
        fcntl(fileno(stderr), F_SETFL, flags);

        tcsetattr(STDIN_FILENO, TCSANOW, &g_oldtermios);
    }
    return;
}

int
main (int argc, char **argv, char **env)
{
    char *p;
    int opt;
    char *progname;
    char *file_name = NULL;
    char *username = NULL;
#ifdef CNOS_OS    
    ctc_task_master_t *p_master = NULL;
#else
    ctclib_thread_master_t* p_master;
    ctclib_thread_t thread;
#endif
    
    /* Set umask before anything for security.  */
    umask (0027);

    ioctl(0, TIOCSCTTY, 1);

    /* Preserve name of myself.  */
    progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

    snprintf(g_szClientIp, M_CLIENT_IP_LEN, "127.0.0.1");

    /* Option handling.  */
    while (1) 
    {
        opt = getopt_long (argc, argv, "f:u:h:Hs", longopts, 0);

        if (opt == EOF)
            break;

        switch (opt) 
        {
            case 'f':
                file_name = optarg;
                break;
            case 'h':
                /* client host ip */
                snprintf(g_szClientIp, M_CLIENT_IP_LEN, "%s", optarg);
                break;
            case 'H':
                usage (0, progname);
                break;
            case 's':
                g_bDoStartupCfg = 1;
                break;
            case 'u':
                username = optarg;
                break;
            default:
                usage (1, progname);
                break;
        }
    }

    save_tty_settings();
    atexit(clish_atexit);


    g_p_clish_master = CLISH_MALLOC(CTCLIB_MEM_LCSH_MODULE, sizeof(clish_master_t));
    if (!g_p_clish_master)
    {
        fprintf(stderr, "Init clish fail: no memory!\n");
    }

#ifdef CNOS_OS
    p_master = ctc_task_init();
    if(!p_master)
    {
        return -1;
    }
    g_p_clish_master->p_thread_master = p_master;
#else
    p_master = ctclib_thread_master_create();
    if(!p_master)
    {
        return -1;
    }
    g_p_clish_master->p_thread_master = p_master;
#endif    

    clish_set_timezone();

    clish_start (file_name, username);

#ifdef CNOS_OS
    ctc_task_main();
#else
    while (ctclib_thread_fetch(p_master, &thread))
    {
        ctclib_thread_call(&thread);   
    }
#endif
    /* Shell is terminated.  */
    printf ("\n");

    /* Rest in peace.  */
    return 0;
}

char *
clish_prompt(void)
{
    static char prompt[MAXHOSTNAMELEN + 32];
    struct utsname names;
    char sign = '#';

    uname (&names);
    /* modified by kcao for bug 13681, 2010-12-03 : distinguish from imish */
    snprintf (prompt, MAXHOSTNAMELEN + 32, "%s%s%c ", "lcsh",
              cli_prompt_str (g_stcli.mode), sign);
    return prompt;
}

void
clish_parse_line(char *line)
{
    extern char *input_line;
    char tem_line[LINE_MESSAGE_MAX] = {'0'};
    
    if (!line || ((char *) EOF == line))
    {
        /* EOF */
        return;
    }

    /* Free last user command.  */
    if (input_line)
    {
        XFREE (MTYPE_TMP, input_line);
        input_line = NULL;
    }

    input_line = XSTRDUP(MTYPE_TMP, line);
    XFREE(0, line);
    if (NULL == input_line)
    {
        fprintf(stderr, "WARNING: Out of memory!!!\n");
        return;
    }

    strncpy(tem_line, input_line, strlen(input_line));
    /* don't do timeout when execute command */
    clish_parse (NULL, 0);

    if (input_line && *input_line)
    {
        if (!(g_stcli.cel && (g_stcli.cel->flags & CLI_FLAG_HIDDEN)))
        {
#ifndef _SYS_LC_
            /* lcsh need support show history for convenience */
            if ((INTERNAL_DEBUG_MODE != g_stcli.mode) && (DIAG_MODE != g_stcli.mode)) 
#endif /* _SYS_LC_ */
            {
                add_history(tem_line);
            }
        }
    }

    rl_callback_handler_install(clish_prompt(), clish_parse_line);
    return;
}

int
clish_parse(char *line, int mode)
{
    struct cli_node *node;
    u_char privilege;
    int ret;
    int first = 1;
    int orig_mode = 0;
    int orig_ret = 0;
    char *invalid = NULL;
    static int do_cmd_mode = 0;
    static int do_cmd = 0;
    int old_mode = 0;
    struct gblock_element *gbe = NULL;
    int result = 0;
    static int ifrange_cmd = 0;

    /* When line is specified, mode is also specified.  */
    if (line)
    {
        privilege = PRIVILEGE_MAX;
    }
    else
    {
        /* Normal case we use cli of clish line.  */
        mode = g_stcli.mode;
        privilege = g_stcli.privilege;
        line = input_line;
    }

    /* Line length check.  */
    if (strlen (line) >= LINE_MESSAGE_MAX - 1)
    {
        if (g_bDoStartupCfg)
        {
            log_sys(M_MOD_LCM, E_ERROR, "Input line is too long: %s", line);
            fprintf(stderr, "%% Input line is too long: %s\n", line);
        }
        else
        {
            printf ("%% Input line is too long\n");
        }
        return CLISH_PARSE_GEN_ERROR;
    }

    if ((!strncmp(line, "do ", 3)) && mode != EXEC_MODE)
    {
        do_cmd = 2;
        do_cmd_mode = g_stcli.mode;
    }

again:

    /* Parse user input.  */
    ret = cli_parse (cli_tree, mode, privilege, line, 1, 0);

    if (CLI_PARSE_NO_MATCH == ret)
    {
        if ((gbe = genblock_parse(cli_tree, mode, line)))
        {
            struct cli_element fakecel;

            memset(&fakecel, 0x0, sizeof (struct cli_element));
            fakecel.module = gbe->modules;
            /* Check this is console or not.  */

            g_stcli.str = line;
            g_stcli.cel = &fakecel;
            g_stcli.ctree = cli_tree;
            /* Execute local function.  */
            cli_free_arguments (cli_tree);
            cli_set_arg(cli_tree, line, strlen(line), 0);
            (*gbe->func)(&g_stcli, cli_tree->argc, cli_tree->argv);
            /* Free arguments.  */
            cli_free_arguments (cli_tree);
            /* Check close.  */
            if (g_stcli.status == CLI_CLOSE)
            {
                exit (0);
            }
            return result;
        }
    }

    /* Second try check.  */
    if (ret != CLI_PARSE_SUCCESS
            && ret != CLI_PARSE_EMPTY_LINE
            && mode != EXEC_MODE && first == 1)
    {
        first = 0;
        invalid = cli_tree->invalid;

        orig_ret = ret;
        orig_mode = mode;
        mode = CONFIG_MODE;

        goto again;
    }

    /* Second try.  */
    if (!first)
    {
        /* Move down to the CONFIG if it succeeds.  */
        if ((ret == CLI_PARSE_SUCCESS)
                && (strncmp(line,"exit", strlen(line)))
                && (strncmp(line,"quit", strlen(line)))
                && (strncmp(line,"end", strlen(line))))
        {
            g_stcli.mode = mode;
        }
        else
        {
            ret = orig_ret;
            g_stcli.mode = mode = orig_mode;
            cli_tree->invalid = invalid;
        }
    }

    /* Check return value.  */
    switch (ret)
    {
        case CLI_PARSE_NO_MODE:
            /* Ignore no mode line.  */
            break;
        case CLI_PARSE_EMPTY_LINE:
            /* Ignore empty line.  */
            break;
        case CLI_PARSE_SUCCESS:
            /* Node to be executed.  */
            node = cli_tree->exec_node;

            /* Set cli structure.  */
            g_stcli.str = line;
            g_stcli.cel = node->cel;
            g_stcli.ctree = cli_tree;

            /* Show command has special treatment.  */
            if ((cli_tree->show_node || (node->cel->flags & CLI_FLAG_SHOW))
                    && !(node->cel->flags & CLI_FLAG_NO_PAGER))
            {
                /* Check modifier.  */
                if (cli_tree->modifier_node)
                {
                    /* Run modifier command to check which modifier is this.  */
                    node = cli_tree->modifier_node;
                    ret = (*node->cel->func) (&g_stcli, 1, cli_tree->argv_modifier);
                    node = cli_tree->exec_node;

                    /* Put '\0' at the pipe.  */
                    *(cli_tree->pipe) = '\0';

                    if (CLISH_MODIFIER_REDIRECT == ret)
                    {
                        /* process redirect filename */
                        char szFullName[M_FULLPATH_MAX_LEN];
                        if (gen_check_and_get_filename(&g_stcli, cli_tree->argv_modifier[0],
                                    szFullName, M_FULLPATH_MAX_LEN) == 0)
                        {
                            XFREE(MTYPE_CLI_ARGUMENT, cli_tree->argv_modifier[0]);
                            cli_tree->argv_modifier[0] = XSTRDUP(MTYPE_CLI_ARGUMENT, szFullName);
                        }
                    }
                    /* Run command with modifier.  */
                    switch (ret)
                    {
                        case CLISH_MODIFIER_BEGIN:
                            clish_begin ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                                    cli_tree->argc, cli_tree->argv,
                                    cli_tree->argv_modifier[0]);
                            break;
                        case CLISH_MODIFIER_INCLUDE:
                            clish_include ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                                    cli_tree->argc, cli_tree->argv,
                                    cli_tree->argv_modifier[0]);
                            break;
                        case CLISH_MODIFIER_EXCLUDE:
                            clish_exclude ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                                    cli_tree->argc, cli_tree->argv,
                                    cli_tree->argv_modifier[0]);
                            break;
                        case CLISH_MODIFIER_GREP:
                            clish_grep ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                                    cli_tree->argc, cli_tree->argv,
                                    cli_tree->argc_modifier, cli_tree->argv_modifier);
                            break;
                        case CLISH_MODIFIER_REDIRECT:
                            clish_redirect ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                                    cli_tree->argc, cli_tree->argv,
                                    cli_tree->argv_modifier[0]);
                            break;
                        case CLISH_MODIFIER_FILENAME_ERROR:
                            cli_free_arguments (cli_tree);
                            return result;
                        case CLISH_MODIFIER_CANCELLED:
                            fprintf(stderr, "%% Aborted\n");
                            cli_free_arguments (cli_tree);
                            return result;
                        default:
                            fprintf(stderr, "%% Unknown modifier\n");
                            return CLISH_PARSE_EXEC_FAIL;
                            break;
                    }
                }
                else 
                {
                    /* Run command with pager.  */
                    clish_more ((INTERNAL_FUNC) node->cel->func, &g_stcli,
                            cli_tree->argc, cli_tree->argv);
                }
            }
            else
            {
                /* Execute local function.  */
                (*node->cel->func) (&g_stcli, cli_tree->argc, cli_tree->argv);
            }
            break;

        case CLI_PARSE_AMBIGUOUS:
            if (g_bDoStartupCfg)
            {
                log_sys(M_MOD_LCM, E_ERROR, "Ambiguous command: %s\n", line);
                fprintf(stderr, "%% Ambiguous command: %s\n", line);
            }
            else
            {
                printf ("%% Ambiguous command:  \"%s\"\n", line);
            }
            break;

        case CLI_PARSE_INCOMPLETE:
            if (g_bDoStartupCfg)
            {
                log_sys(M_MOD_LCM, E_ERROR, "Incomplete command: %s", line);
                fprintf(stderr, "%% Incomplete command: %s\n", line);
            }
            else
            {
                printf ("%% Incomplete command\n\n");
            }
            break;

        case CLI_PARSE_INCOMPLETE_PIPE:
            if (g_bDoStartupCfg)
            {
                log_sys(M_MOD_LCM, E_ERROR, "Incomplete command before pipe: %s", line);
                fprintf(stderr, "%% Incomplete command before pipe: %s\n", line);
            }
            else
            {
                printf ("%% Incomplete command before pipe\n\n");
            }
            break;

        case CLI_PARSE_NO_MATCH:
            if (cli_tree->invalid)
            {
                if (g_bDoStartupCfg)
                {
                    log_sys(M_MOD_LCM, E_ERROR, "Invalid command: %s", line);
                    fprintf(stderr, "%% Invalid command: %s\n", line);
                }
                else
                {
                    if (do_cmd)
                    {
                        old_mode = g_stcli.mode;
                        g_stcli.mode = do_cmd_mode;
                        printf ("%*c^\n",
                                (int)(strlen (clish_prompt ()) + (strlen(g_stcli.str) - strlen(cli_tree->invalid))), ' ');
                        do_cmd = 0;
                        g_stcli.mode = old_mode;              
                    } 
                    else if (ifrange_cmd)
                    {
                        printf ("%*c^\n",
                                (int)(strlen (clish_prompt ()) + strlen("config-if-range") - strlen("config-if")
                                + (cli_tree->invalid - line)), ' ');
                    }
                    else
                    {

                        printf ("%*c^\n",
                                (int)(strlen (clish_prompt ()) + (cli_tree->invalid - line)), ' ');
                    }
                    printf ("%% Invalid input detected at '^' marker.\n\n");

                    result = CLISH_PARSE_GEN_ERROR;
                }
            }
            else
            {
                if (g_bDoStartupCfg)
                {
                    log_sys(M_MOD_LCM, E_ERROR, "Unrecognized command: %s", line);
                    fprintf(stderr, "%% Unrecognized command: %s\n", line);
                }
                else
                {
                    printf ("%% Unrecognized command\n");
                }
            }
            break;

        case CLI_PARSE_ARGV_TOO_LONG:
            if (g_bDoStartupCfg)
            {
                log_sys(M_MOD_LCM, E_ERROR, "Argument is too long: %s", line);
                fprintf(stderr, "%% Argument is too long: %s\n", line);
            }
            else
            {
                printf ("%% Argument is too long\n");
            }
            break;

        default:
            break;
    }

    if (do_cmd > 0)
    {
        if (1 == do_cmd)
        {
            g_stcli.mode = do_cmd_mode;
        }
        do_cmd--;
    }

    /* Free arguments.  */
    cli_free_arguments (cli_tree);

    /* Check close.  */
    if (g_stcli.status == CLI_CLOSE)
        exit (0);

    return result;
}

// TODO: compare with imish_tty_read
#ifdef CNOS_OS
void clish_tty_read()
#else
LCM_TASK_FUNC(clish_tty_read)
#endif
{
#ifndef CNOS_OS
    ctclib_thread_master_t *p_thread_master = NULL;

    p_thread_master = (ctclib_thread_master_t *)(((ctclib_thread_t *)p_data)->arg);
    ctclib_thread_add_read(p_thread_master, clish_tty_read, p_thread_master, STDIN_FILENO);
#endif
    rl_completion_append_character = ' ';
    gen_set_partly_match(0);
    rl_callback_read_char();

    LCM_TASK_RETURN(0);
}

static int g_sigpipe[2] = {-1, -1}; /* break thread loop when signal reached */

int
clish_notify_signal_thread(int sig)
{
    int ch = 'S';
    
    if (g_sigpipe[1] == -1)
    {
        return 0;
    }

    write(g_sigpipe[1], &ch, 1);
    return 0;
}

LCM_TASK_FUNC( clish_signal_thread)
{
    int ch = '-';
    
    /* read all messages in pipe */
    while (read(g_sigpipe[0], &ch, 1) == 1)
    {
        ;
    }

    clish_process_signal();
    LCM_TASK_RETURN(0);
}

LCM_TASK_FUNC(clish_tty_timeout)
{
    /* modified by kcao for bug 13681, 2010-12-03 : lcsh does not timeout */
    LCM_TASK_RETURN(0);
}

#ifndef CNOS_OS
void clish_loop_init(void * p_master)
#else
void clish_loop_init()
#endif
{
    if (g_sigpipe[0] == -1)
    {
        pipe(g_sigpipe);
        fd_set_nonblocking(g_sigpipe[0], 1);
        fd_set_nonblocking(g_sigpipe[1], 1);
    }

#ifdef CNOS_OS
    ctc_task_add_read(TASK_PRI_NORMAL, clish_signal_thread, NULL, g_sigpipe[0]);
    ctc_task_add_read(TASK_PRI_NORMAL, clish_tty_read, NULL, STDIN_FILENO);
    //readline_set_time();
    g_tty_timeout_thread = ctc_task_add_timer(TASK_PRI_NORMAL, clish_tty_timeout, NULL, clish_get_timeout());
#else
    ctclib_thread_master_t *p_thread_master = (ctclib_thread_master_t *)p_master;
    ctclib_thread_add_read(p_thread_master, clish_signal_thread, NULL, g_sigpipe[0]);
    ctclib_thread_add_read(p_thread_master, clish_tty_read, p_thread_master, STDIN_FILENO);
    //readline_set_time();
    g_tty_timeout_thread = ctclib_thread_add_timer(p_thread_master, clish_tty_timeout, NULL, clish_get_timeout());
#endif
    return;
}

extern char g_null_str[2];

int
clish_hook_timeout(void)
{
	return 0;
}

static int g_timed_out = 0; /* timeout flag */
int rl_timed_out(void)
{
    return g_timed_out;
}

void rl_set_timeout_flag(int bflag)
{
    g_timed_out = bflag;
}

static int g_sigint = 0; /* timeout flag */
int rl_sigint(void)
{
    return g_sigint;
}

void rl_set_sigint_flag(int bflag)
{
    g_sigint = bflag;
}

int clish_get_timeout(void)
{
    return g_globals.timeout;
}

char *
clish_get_timeout_str(void)
{
    static char g_timeout_str[128];
    
    snprintf(g_timeout_str, 128, "@%d", clish_get_timeout());
    return g_timeout_str;
}

 clish_master_t *
clish_get_master(void)
{
    return g_p_clish_master;
}



