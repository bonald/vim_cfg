#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <crypt.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "clish_filesystem.h"
#include "clish_exec.h"
#include "clish.h"
#include "clish_cli.h"
#include "cli.h"
#include "param_check.h"
//#include "afx.h"
//#include "clish_readpass.h"
//#include "cfgmgr.h"
//#include "cfm_message.h"
//#include "genlog.h"
//#include "map_cli.h"
//#include "debug_cli.h"
//#include "glb_define.h"
//#include "glb_hw_define.h"
//#include "cli_cmd_desc.h"

/* Terminal window size. */
unsigned short clish_win_row_orig;
unsigned short clish_win_row;
unsigned short clish_win_col;
unsigned short enable_terminal = 0;

#ifndef ISLEAP
#define ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#endif

#ifdef _SYS_LC_
extern int lcsh_cli_init(struct cli_tree *cli_tree);
extern int lcsh_sram_cli_init(struct cli_tree *cli_tree);
extern int lcsh_tcam_cli_init(struct cli_tree *cli_tree);
#endif

extern int lcsh_show_hagt_init(struct cli_tree *cli_tree);
extern int diag_lcsh_cli_init(struct cli_tree *cli_tree);
extern int diag_humber_cli_init(struct cli_tree *cli_tree);
extern int vps_cli_init(struct cli_tree *cli_tree);
extern int diag_cli_init(struct cli_tree *cli_tree);
extern int clish_show_cli_init(struct cli_tree *cli_tree);
extern void logging_cli_init (struct cli_tree *pctree);
extern int clish_exec (char *command, ...);
extern int clish_execvp (char **argv);
extern int clish_cfm_return (struct cli *cli, int ret);

extern struct globals g_globals;
//extern afx_timer_t *pTimer_alarm;
//extern afx_timer_t *pTimer_boot;

S_ZoneInfo  g_stZoneInfo;  /* zone information*/

system_file_t g_system_file_arr[] = 
{
    {STARTUP_CONFIG_FILE_PATH, "startup configuration file"},
    {M_CUR_BOOT_IMAGE_LINK_FULLNAME, "boot configuration file"},
    {M_BOOT_IMAGE_PATH, "boot image directory"},
    {NULL, NULL}
};
extern network_op_t g_clish_tftp_ops;

int
term_winsize_set (int sock,
		      unsigned short row, unsigned short col)
{
  struct winsize w;
  int ret;

  w.ws_row = row;
  w.ws_col = col;

  ret = ioctl (sock, TIOCSWINSZ, &w);
  if (ret < 0)
    return -1;

  return 0;
}

int
term_winsize_get (int sock,
		      unsigned short  *row, unsigned short  *col)
{
  struct winsize w;
  int ret;

  ret = ioctl (sock, TIOCGWINSZ, &w);
  if (ret < 0)
    return -1;

  *row = w.ws_row;
  *col = w.ws_col;

  return 0;
}

/* "exit" function.  */
void
clish_mode_exit (struct cli *cli)
{
  switch (cli->mode)
    {
    case EXEC_MODE:
      exit(0);
      break;
    case CONFIG_MODE:
      cli->mode = EXEC_MODE;
      break;
    case DIAG_MODE:
      cli->mode = EXEC_MODE;
      break;
    default:
      cli->mode = CONFIG_MODE;
      break;
    }
}


CLI(exit_config,
    exit_cmd,
    "exit",
    "End current mode and down to previous mode")
{
    clish_mode_exit (cli);
    return CLI_SUCCESS;
}

/* "quit" alias for EXEC MODE.  */
ALI (exit_config,
     quit_cmd,
     "quit",
     "Exit current mode and down to previous mode");

/* Generic "end" command.  */
CLI (end,
     end_cmd,
     "end",
     "End current mode and change to EXEC mode")
{
    if (cli->mode != EXEC_MODE)
    {
        cli->mode = EXEC_MODE;
    }
    return CLI_SUCCESS;
}

CLI (config_terminal,
     config_terminal_cmd,
     "configure terminal",
     "Enter configuration mode",
     "Configure from the terminal")
{
    cli->mode = CONFIG_MODE;
    return CLI_SUCCESS;
}

CLI(mode_diag,
    mode_diag_cmd,
    "diag",
    "Diagnostics Mode")
{
    struct passwd *pw;
    char *pszPin = NULL;
    char *pszEnc = NULL;

    pw = getpwnam("root");
    if (NULL == pw)
    {
        cli_out(cli, "%% Internal error\n");
        return CLI_ERROR;
    }

    pszPin = getpass("Password: ");
    if (NULL == pszPin || strlen(pszPin) <= 0)
    {
        cli_out(cli, "%% Aborted\n");
        return CLI_ERROR;
    }

    pszEnc = crypt(pszPin, pw->pw_passwd);
    if (NULL == pszEnc || 0 != strcmp(pszEnc, pw->pw_passwd))
    {
        cli_out(cli, "%% Authentication failed\n");
        return CLI_SUCCESS;
    }
    
    cli->mode = DIAG_MODE;
    return CLI_SUCCESS;
}

#if (0)
CLI (clish_line_console,
     clish_line_console_cmd,
     "line console",
     "Configure a terminal line",
     "Primary terminal line")
{
    cli->mode = LINE_MODE;
    return CLI_SUCCESS;
}

CLI (clish_line_vty,
     clish_line_vty_cmd,
     "line vty",
     "Configure a terminal line",
     "Virtual terminal")
{
    cli->mode = VTY_MODE;
    return CLI_SUCCESS;
}

CLI (clish_line_exec_timeout_sec,
     clish_line_exec_timeout_sec_cmd,
     "exec-timeout <1-14400> (<0-59>|)",
     "Set timeout value",
     "Timeout value in minutes",
     "Timeout in seconds")
{
    int timeout;
    int conTimeout = 0;
    int vtyTimeout = 0;

    timeout = strtoul (argv[0], NULL, 10);
    timeout *= 60;
    if (argc > 1)
        timeout += strtoul (argv[1], NULL, 10);
        
    if (cli->mode == LINE_MODE)
        conTimeout = timeout;
    else
        vtyTimeout = timeout;

    cfm_set_exec_timeout (cfm_clnt, conTimeout, vtyTimeout);

    return CLI_SUCCESS;
}

CLI (clish_no_line_exec_timeout_sec,
     clish_no_line_exec_timeout_sec_cmd,
     "no exec-timeout",
     CLI_NO_STR,
     "Set the EXEC timeout")
{
    int timeout;
    int conTimeout = 0;
    int vtyTimeout = 0;

    timeout = LINE_TIMEOUT_DEFAULT_SEC;
    
    if (cli->mode == LINE_MODE)
        conTimeout = timeout;
    else
        vtyTimeout = timeout;

    cfm_set_exec_timeout (cfm_clnt, conTimeout, vtyTimeout);

    return CLI_SUCCESS;
}
#endif

CLI (clish_do,
     clish_do_cli,
     "do LINE",
     "To run exec commands in config mode",
     "Exec Command")
{
    char *input;

    /* This is unique command.  */
    input = argv[0];

    clish_parse (input, EXEC_MODE);
    XFREE (MTYPE_TMP, input);
    return CLI_SUCCESS;
}

int clish_cli_init(struct cli_tree *cli_tree)
{
    int i;

    lcsh_cli_init(cli_tree);
    diag_lcsh_cli_init(cli_tree);

    /* modified by kcao for support begin/include/exclude/grep, 2010-12-03 */
    clish_show_cli_init(cli_tree);

    for (i = 0; i < MAX_MODE; i++)
    {
        cli_install_gen(cli_tree, i, PRIVILEGE_NORMAL, 0, &exit_cmd);
        cli_install_gen(cli_tree, i, PRIVILEGE_NORMAL, 0, &quit_cmd);
        if (i != EXEC_MODE)
        {
            cli_install_gen(cli_tree, i, PRIVILEGE_NORMAL, 0, &end_cmd);
        }
        if (i > EXEC_MODE)
        {
            cli_install_gen(cli_tree, i, PRIVILEGE_NORMAL, 0, &clish_do_cli);
        }
    }

    cli_sort(cli_tree);
    return 0;
}

int
get_addr_by_dns (int family, char *host, char *ipaddr, int size)
{
    int ret;
    struct addrinfo *res, hints;
    struct sockaddr_in *addr;

    if (family != AF_INET)
        return -1;

    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    ret = getaddrinfo (host, NULL, &hints, &res);
    if (!ret)
    {
        /* Consider the first entry only. */

        if (family == AF_INET)
        {
            addr = (struct sockaddr_in *)res->ai_addr;
            inet_ntop (family, &addr->sin_addr.s_addr, ipaddr, size);
            freeaddrinfo (res);
            return 0;
        }
        else
        {
            freeaddrinfo (res);
            return -1;
        }
    }

    return -1;
}

