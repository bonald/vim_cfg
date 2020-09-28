/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/utsname.h>

#include "command.h"
#include "memory.h"
#include "vtysh.h"
#include "ctc_cli.h"
#include "sal.h"

#define VTYSH_DEFUNSH(daemon, funcname, cmdname, cmdstr, ...) \
  char *cmdname ## _help[] = {__VA_ARGS__ , NULL}; \
  int funcname (struct cmd_element *, struct vty *, int, char **); \
  struct cmd_element cmdname = \
  { \
    cmdstr, \
    funcname, \
    cmdname ## _help, \
    daemon \
  }; \
  int funcname \
  (struct cmd_element *self, struct vty *vty, int argc, char **argv)

/* ALIAS macro which define existing command's alias. */
#define VTYSH_ALIAS(funcname, cmdname, cmdstr, ...) \
  char *cmdname ## _help[] = {__VA_ARGS__ , NULL}; \
  struct cmd_element cmdname = \
  { \
    cmdstr, \
    funcname, \
    cmdname ## _help \
  };

/* Struct VTY. */
struct vty *vty;

/* VTY shell pager name. */
char *vtysh_pager_name = NULL;

/* VTY shell client structure. */
struct vtysh_client
{
  int fd;
} vtysh_client[VTYSH_INDEX_MAX];

/* When '^Z' is received from vty, move down to the enable mode. */
int
vtysh_end ()
{
    vty->node = EXEC_MODE;
    return CMD_SUCCESS;
}

VTYSH_DEFUNSH (VTYSH_ALL,
	 vtysh_end_all,
	 vtysh_end_all_cmd,
	 "end",
	 "End current mode and down to previous mode")
{
  return vtysh_end (vty);
}

void
vclient_close (struct vtysh_client *vclient)
{
  if (vclient->fd > 0)
    close (vclient->fd);
  vclient->fd = -1;
}


/* Following filled with debug code to trace a problematic condition
   under load - it SHOULD handle it.
*/
#define ERR_WHERE_STRING "vtysh(): vtysh_client_config(): "
int
vtysh_client_config (struct vtysh_client *vclient, char *line)
{
  int ret;
  char *buf;
  size_t bufsz;
  char *pbuf;
  size_t left;
  char *eoln;
  int nbytes;
  int i;
  int readln;

  if (vclient->fd < 0)
    return CMD_SUCCESS;

  ret = write (vclient->fd, line, strlen (line) + 1);
  if (ret <= 0)
    {
      vclient_close (vclient);
      return CMD_SUCCESS;
    }
	
  /* Allow enough room for buffer to read more than a few pages from socket
   */
  bufsz = 5 * sysconf(_SC_PAGESIZE) + 1;
  buf = malloc(bufsz);
  memset(buf, 0, bufsz);
  pbuf = buf;

  while (1)
    {
      if (pbuf >= ((buf + bufsz) -1))
	{
	  fprintf (stderr, ERR_WHERE_STRING \
		   "warning - pbuf beyond buffer end.\n");
	  return CMD_WARNING;
	}

      readln = (buf + bufsz) - pbuf - 1;
      nbytes = read (vclient->fd, pbuf, readln);

      if (nbytes <= 0)
	{

	  if (errno == EINTR)
	    continue;

	  fprintf(stderr, ERR_WHERE_STRING "(%u)", errno);
	  perror("");

	  if (errno == EAGAIN || errno == EIO)
	    continue;

	  vclient_close (vclient);
	  free(buf);
	  return CMD_SUCCESS;
	}

      pbuf[nbytes] = '\0';

      if (nbytes >= 4)
	{
	  i = nbytes - 4;
	  if (pbuf[i] == '\0' && pbuf[i + 1] == '\0' && pbuf[i + 2] == '\0')
	    {
	      ret = pbuf[i + 3];
	      break;
	    }
	}
      pbuf += nbytes;

      /* See if a line exists in buffer, if so parse and consume it, and
	 reset read position */
      if ((eoln = strrchr(buf, '\n')) == NULL)
	continue;

      if (eoln >= ((buf + bufsz) - 1))
	{
	  fprintf (stderr, ERR_WHERE_STRING \
		   "warning - eoln beyond buffer end.\n");
	}
      //vtysh_config_parse(buf);

      eoln++;
      left = (size_t)(buf + bufsz - eoln);
      memmove(buf, eoln, left);
      buf[bufsz-1] = '\0';
      pbuf = buf + strlen(buf);
    }

  /* parse anything left in the buffer */
  //vtysh_config_parse (buf);

  free(buf);
  return ret;
}

int32
vtysh_show_and_del_file(char *file_name)
{
#define SHOW_LINE_LEN   256
    char  line[SHOW_LINE_LEN];

    FILE* fp = NULL;
    fp = fopen(file_name, "r");
    if (!fp)
    {
        return 0;
    }
    while (NULL != fgets(line, SHOW_LINE_LEN, fp))
    {
        printf("%s", line);
    }
    fclose(fp);
    unlink(file_name);

    return 0;
}

int
vtysh_client_execute (struct vtysh_client *vclient, char *line, FILE *fp)
{
  int ret;
  char buf[1001];
  int nbytes;
  int i;

  if (vclient->fd < 0)
    return CMD_SUCCESS;

  ret = write (vclient->fd, line, strlen (line) + 1);
  if (ret <= 0)
    {
      vclient_close (vclient);
      return CMD_SUCCESS;
    }
	
  while (1)
    {
      nbytes = read (vclient->fd, buf, sizeof(buf)-1);

      if (nbytes <= 0 && errno != EINTR)
	{
	  vclient_close (vclient);
	  return CMD_SUCCESS;
	}

      if (nbytes > 0)
	{
	  buf[nbytes] = '\0';
	  fprintf (fp, "%s", buf);
	  fflush (fp);
      vtysh_show_and_del_file("/tmp/sdk_debug");
	  if (nbytes >= 4)
	    {
	      i = nbytes - 4;
	      if (buf[i] == '\0' && buf[i + 1] == '\0' && buf[i + 2] == '\0')
		{
		  ret = buf[i + 3];
		  break;
		}
	    }
	}
    }
  return ret;
}

void
vtysh_pager_init ()
{
  vtysh_pager_name = getenv ("VTYSH_PAGER");
  if (! vtysh_pager_name)
    vtysh_pager_name = "more";
}

/* Command execution over the vty interface. */
void
vtysh_execute_func (char *line, int pager)
{
  int ret, cmd_stat;
  vector vline;
  struct cmd_element *cmd;
  FILE *fp = NULL;

  /* Split readline string up into the vector */
  vline = cmd_make_strvec (line);

  if (vline == NULL)
    return;

  ret = cmd_execute_command_vtysh_client (vline, vty, &cmd);

  cmd_free_strvec (vline);

  switch (ret)
    {
    case CMD_WARNING:
      if (vty->type == VTY_FILE)
	printf ("Warning...\n");
      break;
    case CMD_ERR_AMBIGUOUS:
      printf ("%% Ambiguous command.\n");
      break;
    case CMD_ERR_NO_MATCH:
      printf ("%% Unknown command.\n");
      break;
    case CMD_ERR_INCOMPLETE:
      printf ("%% Command incomplete.\n");
      break;
    case CMD_SUCCESS_DAEMON:
      {
	if (pager && vtysh_pager_name)
	  {
	    fp = popen ("more", "w");
	    if (fp == NULL)
	      {
		perror ("popen");
		exit (1);
	      }
	  }
	else
	  fp = stdout;

	if (! strcmp(cmd->string,"configure terminal"))
	  {
	      cmd_stat = vtysh_client_execute (&vtysh_client[VTYSH_INDEX_SDK],
					       line, fp);
	    if (cmd_stat != CMD_WARNING)
	      cmd_stat = vtysh_client_execute (&vtysh_client[VTYSH_INDEX_OVS], line, fp);
	    if (cmd_stat)
	      {
                line = "end";
                vline = cmd_make_strvec (line);

                if (vline == NULL)
		  {
		    if (pager && vtysh_pager_name && fp)
		      {
			if (pclose (fp) == -1)
			  {
			    perror ("pclose");
			    exit (1);
			  }
			fp = NULL;
		      }
		    return;
		  }

                ret = cmd_execute_command_vtysh_client (vline, vty, &cmd);
                cmd_free_strvec (vline);
                if (ret != CMD_SUCCESS_DAEMON)
                  break;
	      }
	    else
	      if (cmd->func)
		{
		  (*cmd->func) (cmd, vty, 0, NULL);
		  break;
                }
	  }

	if (cmd->daemon & VTYSH_SDK)
	  if (vtysh_client_execute (&vtysh_client[VTYSH_INDEX_SDK], line, fp)
	      != CMD_SUCCESS)
	    break;
	if ((vtysh_client[VTYSH_INDEX_OVS].fd != 0) && (cmd->daemon & VTYSH_OVS))
	  if (vtysh_client_execute (&vtysh_client[VTYSH_INDEX_OVS], line, fp)
	      != CMD_SUCCESS)
	    break;
	if (cmd->func)
	  (*cmd->func) (cmd, vty, 0, NULL);
    }
    }
  if (pager && vtysh_pager_name && fp)
    {
      if (pclose (fp) == -1)
	{
	  perror ("pclose");
	  exit (1);
	}
      fp = NULL;
    }
}

void
vtysh_execute_no_pager (char *line)
{
  vtysh_execute_func (line, 0);
}

void
vtysh_execute (char *line)
{
  vtysh_execute_func (line, 0);
}

/* Configration make from file. */
int
vtysh_config_from_file (struct vty *vty, FILE *fp)
{
  int ret;
  vector vline;
  struct cmd_element *cmd;

  while (fgets (vty->buf, VTY_BUFSIZ, fp))
    {
      if (vty->buf[0] == '!' || vty->buf[1] == '#')
	continue;

      vline = cmd_make_strvec (vty->buf);

      /* In case of comment line */
      if (vline == NULL)
	continue;

      /* Execute configuration command : this is strict match */
      ret = cmd_execute_command_strict (vline, vty, &cmd);

      /* Try again with setting node to CTC_SDK_MODE */
      if (ret != CMD_SUCCESS 
	  && ret != CMD_SUCCESS_DAEMON
	  && ret != CMD_WARNING)
	{
        vtysh_execute ("end");
        vtysh_execute ("configure terminal");
        vty->node = CTC_SDK_MODE;
        ret = cmd_execute_command_strict (vline, vty, &cmd);
	}	  

      cmd_free_strvec (vline);

      switch (ret)
	{
	case CMD_WARNING:
	  if (vty->type == VTY_FILE)
	    printf ("Warning...\n");
	  break;
	case CMD_ERR_AMBIGUOUS:
	  printf ("%% Ambiguous command.\n");
	  break;
	case CMD_ERR_NO_MATCH:
	  printf ("%% Unknown command: %s", vty->buf);
	  break;
	case CMD_ERR_INCOMPLETE:
	  printf ("%% Command incomplete.\n");
	  break;
	case CMD_SUCCESS_DAEMON:
	  {
	    if (cmd->daemon & VTYSH_SDK)
	      if (vtysh_client_execute (&vtysh_client[VTYSH_INDEX_SDK],
					vty->buf, stdout) != CMD_SUCCESS)
		break;
	    if (cmd->daemon & VTYSH_OVS)
	      if (vtysh_client_execute (&vtysh_client[VTYSH_INDEX_OVS],
					vty->buf, stdout) != CMD_SUCCESS)
		break;
	    if (cmd->func)
	      (*cmd->func) (cmd, vty, 0, NULL);
	  }
	}
    }
  return CMD_SUCCESS;
}

/* We don't care about the point of the cursor when '?' is typed. */
int
vtysh_rl_describe ()
{
  int ret;
  int i;
  vector vline;
  vector describe;
  int width;
  struct desc *desc;

  vline = cmd_make_strvec (rl_line_buffer);

  /* In case of '> ?'. */
  if (vline == NULL)
    {
      vline = vtysh_vector_init (1);
      vtysh_vector_set (vline, '\0');
    }
  else 
    if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
      vtysh_vector_set (vline, '\0');

  describe = cmd_describe_command (vline, vty, &ret);

  printf ("\n");

  /* Ambiguous and no match error. */
  switch (ret)
    {
    case CMD_ERR_AMBIGUOUS:
      cmd_free_strvec (vline);
      printf ("%% Ambiguous command.\n");
      rl_on_new_line ();
      return 0;
      break;
    case CMD_ERR_NO_MATCH:
      cmd_free_strvec (vline);
      printf ("%% There is no matched command.\n");
      rl_on_new_line ();
      return 0;
      break;
    }  

  /* Get width of command string. */
  width = 0;
  for (i = 0; i < vector_max (describe); i++)
    if ((desc = vector_slot (describe, i)) != NULL)
      {
	int len;

	if (desc->cmd[0] == '\0')
	  continue;

	len = strlen (desc->cmd);
	if (desc->cmd[0] == '.')
	  len--;

	if (width < len)
	  width = len;
      }

  for (i = 0; i < vector_max (describe); i++)
    if ((desc = vector_slot (describe, i)) != NULL)
      {
	if (desc->cmd[0] == '\0')
	  continue;

	if (! desc->str)
	  printf ("  %-s\n",
		  desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd);
	else
	  printf ("  %-*s  %s\n",
		  width,
		  desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
		  desc->str);
      }

  cmd_free_strvec (vline);
  vtysh_vector_free (describe);

  rl_on_new_line();

  return 0;
}

/* result of cmd_complete_command() call will be stored here
   and used in new_completion() in order to put the space in
   correct places only */
int complete_status;

char *
command_generator (const char *text, int state)
{
  vector vline;
  static char **matched = NULL;
  static int index = 0;

  /* First call. */
  if (! state)
    {
      index = 0;

#if 0
      if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
	return NULL;
#endif
      vline = cmd_make_strvec (rl_line_buffer);
      if (vline == NULL)
	return NULL;

      if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
        vtysh_vector_set (vline, '\0');

      matched = cmd_complete_command (vline, vty, &complete_status);
    }

  if (matched && matched[index])
    return matched[index++];

  return NULL;
}

typedef char *rl_compentry_func_t (const char *, int);
extern char **completion_matches (char *, rl_compentry_func_t *);

char **
new_completion (char *text, int start, int end)
{
  char **matches;

  matches = completion_matches (text, command_generator);

  if (matches)
    {
      rl_point = rl_end;
//      if (complete_status == CMD_COMPLETE_FULL_MATCH)
//	rl_pending_input = ' ';
    }

  return matches;
}

char **
vtysh_completion (char *text, int start, int end)
{
  int ret;
  vector vline;
  char **matched = NULL;

#if 0
  if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
    return NULL;
#endif

  vline = cmd_make_strvec (rl_line_buffer);
  if (vline == NULL)
    return NULL;

  /* In case of 'help \t'. */
  if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
    vtysh_vector_set (vline, '\0');

  matched = cmd_complete_command (vline, vty, &ret);

  cmd_free_strvec (vline);

  return (char **) matched;
}

int
vtysh_exit (struct vty *vty)
{
  switch (vty->node)
    {
    case EXEC_MODE:
      exit (0);
      break;
    case CTC_SDK_MODE:
    case CTC_CMODEL_MODE:
      vty->node = EXEC_MODE;
      break;
    case CTC_SDK_OAM_CHAN_MODE:
      vty->node = CTC_SDK_MODE;
      break;
    case CTC_DEBUG_MODE:
      vty->node = EXEC_MODE;
      break;
    case CTC_SDK_V1_MODE:
      vty->node = EXEC_MODE;
      break;
    default:
      vty->node = EXEC_MODE;
      break;
    }
  return CMD_SUCCESS;
}

VTYSH_DEFUNSH (VTYSH_ALL,
	 vtysh_exit_all,
	 vtysh_exit_all_cmd,
	 "exit",
	 "Exit current mode and down to previous mode")
{
  return vtysh_exit (vty);
}

VTYSH_ALIAS (vtysh_exit_all,
       vtysh_quit_all_cmd,
       "quit",
       "Exit current mode and down to previous mode");

#if 0
DEFUNSH (VTYSH_SDK|VTYSH_OVS,
	 vtysh_interface,
	 vtysh_interface_cmd,
	 "interface IFNAME",
	 "Select an interface to configure"
	 "Interface's name")
{
  vty->node = INTERFACE_NODE;
  return CMD_SUCCESS;
}
#endif

VTYSH_DEFUNSH (VTYSH_SDK|VTYSH_OVS,
	 vtysh_exit_interface,
	 vtysh_exit_interface_cmd,
	 "exit",
	 "Exit current mode and down to previous mode")
{
  return vtysh_exit (vty);
}

VTYSH_ALIAS (vtysh_exit_interface,
       vtysh_quit_interface_cmd,
       "quit",
       "Exit current mode and down to previous mode");

CTC_CLI (vtysh_write_terminal,
       vtysh_write_terminal_cmd,
       "write terminal",
       "Write running configuration to memory, network, or terminal",
       "Write to terminal")
{
  //int ret;
  //char line[] = "write terminal\n";
  FILE *fp = NULL;

  if (vtysh_pager_name)
    {
      fp = popen ("more", "w");
      if (fp == NULL)
	{
	  perror ("popen");
	  exit (1);
	}
    }
  else
    fp = stdout;

  vtysh_vty_out (vty, "Building configuration...%s", VTY_NEWLINE);
  vtysh_vty_out (vty, "%sCurrent configuration:%s", VTY_NEWLINE,
	   VTY_NEWLINE);

#if 0
  vtysh_config_write (fp);

  ret = vtysh_client_config (&vtysh_client[VTYSH_INDEX_SDK], line);
  ret = vtysh_client_config (&vtysh_client[VTYSH_INDEX_OVS], line);

  vtysh_config_dump (fp);

  if (vtysh_pager_name && fp)
    {
      fflush (fp);
      if (pclose (fp) == -1)
	{
	  perror ("pclose");
	  exit (1);
	}
      fp = NULL;
    }
#endif
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_write_memory,
       vtysh_write_memory_cmd,
       "write memory",
       "Write running configuration to memory, network, or terminal"
       "Write configuration to the file (same as write file)")
{
#if 1
    return CMD_SUCCESS;
#else
  int ret;
  mode_t old_umask;
  char line[] = "write terminal\n";
  FILE *fp;
  char *integrate_sav = NULL;

  /* config files have 0600 perms... */ 
  old_umask = umask (0077);

  integrate_sav = malloc (strlen (integrate_default) 
			    + strlen (CONF_BACKUP_EXT) + 1);
  strcpy (integrate_sav, integrate_default);
  strcat (integrate_sav, CONF_BACKUP_EXT);


  printf ("Building Configuration...\n");

  /* Move current configuration file to backup config file */
  unlink (integrate_sav);
  rename (integrate_default, integrate_sav);

  fp = fopen (integrate_default, "w");
  if (fp == NULL)
    {
      printf ("%% Can't open configuration file %s.\n", integrate_default);
      umask (old_umask);
      return CMD_SUCCESS;
    }
  else
    printf ("[OK]\n");
	  

  vtysh_config_write (fp);

  ret = vtysh_client_config (&vtysh_client[VTYSH_INDEX_SDK], line);
  ret = vtysh_client_config (&vtysh_client[VTYSH_INDEX_OVS], line);

  vtysh_config_dump (fp);

  fclose (fp);

  umask (old_umask);
  return CMD_SUCCESS;
#endif
}

VTYSH_ALIAS (vtysh_write_memory,
       vtysh_copy_runningconfig_startupconfig_cmd,
       "copy running-config startup-config",  
       "Copy from one file to another"
       "Copy from current system configuration"
       "Copy to startup configuration");

VTYSH_ALIAS (vtysh_write_memory,
       vtysh_write_file_cmd,
       "write file",
       "Write running configuration to memory, network, or terminal"
       "Write configuration to the file (same as write memory)");

VTYSH_ALIAS (vtysh_write_terminal,
       vtysh_show_running_config_cmd,
       "show running-config",
       SHOW_STR
       "Current operating configuration");

/* Execute command in child process. */
int
execute_command (char *command, int argc, char *arg1, char *arg2)
{
  int ret;
  pid_t pid;
  int status;

  /* Call fork(). */
  pid = fork ();

  if (pid < 0)
    {
      /* Failure of fork(). */
      fprintf (stderr, "Can't fork: %s\n", strerror (errno));
      exit (1);
    }
  else if (pid == 0)
    {
      /* This is child process. */
      switch (argc)
	{
	case 0:
	  ret = execlp (command, command, NULL);
	  break;
	case 1:
	  ret = execlp (command, command, arg1, NULL);
	  break;
	case 2:
	  ret = execlp (command, command, arg1, arg2, NULL);
	  break;
	}

      /* When execlp suceed, this part is not executed. */
      fprintf (stderr, "Can't execute %s: %s\n", command, strerror (errno));
      exit (1);
    }
  else
    {
      /* This is parent. */
      execute_flag = 1;
      ret = wait4 (pid, &status, 0, NULL);
      execute_flag = 0;
    }
  return 0;
}

#if 0
CTC_CLI (vtysh_ping,
       vtysh_ping_cmd,
       "ping WORD",
       "Send echo messages",
       "Ping destination address or hostname")
{
  execute_command ("ping", 1, argv[0], NULL);
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_traceroute,
       vtysh_traceroute_cmd,
       "traceroute WORD",
       "Trace route to destination",
       "Trace route to destination address or hostname")
{
  execute_command ("traceroute", 1, argv[0], NULL);
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_telnet,
       vtysh_telnet_cmd,
       "telnet WORD",
       "Open a telnet connection",
       "IP address or hostname of a remote system")
{
  execute_command ("telnet", 1, argv[0], NULL);
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_telnet_port,
       vtysh_telnet_port_cmd,
       "telnet WORD PORT",
       "Open a telnet connection",
       "IP address or hostname of a remote system",
       "TCP Port number")
{
  execute_command ("telnet", 2, argv[0], argv[1]);
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_start_shell,
       vtysh_start_shell_cmd,
       "start-shell",
       "Start UNIX shell")
{
  execute_command ("sh", 0, NULL, NULL);
  return CMD_SUCCESS;
}

CTC_CLI(vtysh_rm_debug,
        vtysh_rm_debug_cmd,
        "rm FILENAME",
        "Delete file",
        "File name")
{
  execute_command ("rm", 1, argv[0], NULL);
  return CMD_SUCCESS;
}

CTC_CLI(vtysh_tftp_debug,
    vtysh_tftp_debug_cmd,
    "tftp server-ip IPADDRESS filename FILENAME",
    "Tftp transmit",
    "Server ip address",
    "Input server ip address",
    "File name",
    "Input file name")
{
    uint32 ret = 0;
    char strCmd[256] = {0};
    kal_memset(strCmd,0,256);
    sprintf(strCmd,"tftp -v -Z %s -m binary -s 2048 -c get -r %s",argv[0],argv[1]);

    chdir("/mnt/flash/boot");
    ret = system(strCmd);
    if( ret == 0 ){
        ctc_cli_out("tftp command execute OK.\n");
    }else{
        ctc_cli_out("%% tftp command execute NG.\n");
        return CMD_WARNING;
    }
    return CMD_SUCCESS;
}

CTC_CLI(vtysh_boot_debug,
        vtysh_boot_debug_cmd,
        "boot system-image IMAGE",
        "Boot command",
        "System image",
        "Input system image file")
{
    chdir("/mnt/flash/boot");
    unlink("uImage");
    if (symlink(argv[0], "uImage") != 0)
    {
        ctc_cli_out("%% Setting the next boot image file faild: %s\n",
                strerror(errno));
        return CMD_WARNING;
    }

    return CMD_SUCCESS;
}

CTC_CLI(vtysh_show_file_dir,
        vtysh_show_file_dir_cmd,
        "pwd",
        "System command pwd")
{
  execute_command ("pwd", 0, NULL, NULL);
  return CMD_SUCCESS;
}

CTC_CLI(vtysh_change_file_dir,
        vtysh_change_file_dir_cmd,
        "cd FILE_PATH",
        "System command cd",
        "File path")
{
    int32 i;
    char path[256] = {0};
    char* work_dir = NULL;

    kal_memset(path, 0, 256);

    work_dir = argv[0];

    /*ingore one dot*/
    if(0 == kal_strcmp(work_dir, "."))
    {
        return CMD_WARNING;
    }

    /*switch to parent directory*/
    if(0 == kal_strcmp(work_dir, ".."))
    {
        if(getcwd(path, 256) == NULL)
        {
            ctc_cli_out("%% Get current work directory failed!\n");
            return CMD_WARNING;
        }
        i = kal_strlen(path);

        while(i)
        {
            if('/' == path[i])
                break;
            else
                path[i] = 0;

             --i;
        }

        chdir(path);
    }
    else /*enter subdirectory*/
    {
        int32 ret;
        if(getcwd(path, 256) == NULL)
        {
            ctc_cli_out("%% Get current work directory failed!\n");
            return CMD_WARNING;
        }

        i = kal_strlen(path);
        path[i] = '/';
        kal_strcat(path, work_dir);
        chdir(path);

        ret = chdir(path);
        if (ret == -1)
        {
            kal_memset(path, 0, sizeof(path));
            kal_memcpy(path, work_dir, sizeof(path));
            ret = chdir(path);
            if (ret == -1)
            {
                ctc_cli_out("%% Invalid path: %s!\n", path);
                return CMD_WARNING;
            }
        }

    }

    return CMD_SUCCESS;
}

CTC_CLI(vtysh_reboot,
        vtysh_reboot_cmd,
        "reboot",
        "Reboot halt and perform a cold restart")
{
  execute_command ("reboot", 0, NULL, NULL);
  return CMD_SUCCESS;
}

CTC_CLI(vtysh_list_files,
        vtysh_list_files_cmd,
        "ls",
        "System command ls")
{
  execute_command ("ls", 0, NULL, NULL);
  return CMD_SUCCESS;
}

CTC_CLI (vtysh_show_history,
       vtysh_show_history_cmd,
       "show history",
       CTC_CLI_SHOW_STR,
       "Display the session command history")
{
    int index;
    int print_index = 1;
    for (index = vty->hindex + 1; index != vty->hindex;)
    {
        if (index == VTY_MAXHIST)
        {
        index = 0;
        continue;
        } 
        if (vty->hist[index] != NULL)
        {
            printf ("%d  %s%s", print_index, vty->hist[index], VTY_NEWLINE); 
            print_index++;
        }
        index++;    
    } 
    return CMD_SUCCESS;
}
#endif
void
vtysh_install_default (enum node_type node)
{
  //install_element (node, &config_list_cmd);
}

/* Making connection to protocol daemon. */
int
vtysh_connect (struct vtysh_client *vclient, char *path)
{
  int ret;
  int sock, len;
  struct sockaddr_un addr;
  struct stat s_stat;
  uid_t euid;
  gid_t egid;

  memset (vclient, 0, sizeof (struct vtysh_client));
  vclient->fd = -1;

  /* Stat socket to see if we have permission to access it. */
  euid = geteuid();
  egid = getegid();
  ret = stat (path, &s_stat);
  if (ret < 0 && errno != ENOENT)
    {
      fprintf  (stderr, "vtysh_connect(%s): stat = %s\n", 
		path, strerror(errno)); 
      exit(1);
    }
  
  if (ret >= 0)
    {
      if (! S_ISSOCK(s_stat.st_mode))
	{
	  fprintf (stderr, "vtysh_connect(%s): Not a socket\n",
		   path);
	  exit (1);
	}
      
      if (euid != s_stat.st_uid 
	  || !(s_stat.st_mode & S_IWUSR)
	  || !(s_stat.st_mode & S_IRUSR))
	{
	  fprintf (stderr, "vtysh_connect(%s): No permission to access socket\n",
		   path);
	  exit (1);
	}
    }

  sock = socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
#ifdef DEBUG
      fprintf(stderr, "vtysh_connect(%s): socket = %s\n", path, strerror(errno));
#endif /* DEBUG */
      return -1;
    }

  memset (&addr, 0, sizeof (struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path, strlen (path));
#ifdef HAVE_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof (addr.sun_family) + strlen (addr.sun_path);
#endif /* HAVE_SUN_LEN */

  ret = connect (sock, (struct sockaddr *) &addr, len);
  if (ret < 0)
    {
#ifdef DEBUG
      fprintf(stderr, "vtysh_connect(%s): connect = %s\n", path, strerror(errno));
#endif /* DEBUG */
      close (sock);
      return -1;
    }
  vclient->fd = sock;

  return 0;
}

void
vtysh_connect_all()
{
    memset(&vtysh_client, 0x00, sizeof(vtysh_client));
  /* Clear each daemons client structure. */
  vtysh_connect (&vtysh_client[VTYSH_INDEX_SDK], SDK_PATH);
}

/* To disable readline's filename completion */
int
vtysh_completion_entry_function (int ignore, int invoking_key)
{
  return 0;
}

/* Disable readline's filename completion.  */
char *
vtysh_completion_empty (const char *ignore, int invoking_key)
{
  return NULL;
}

void
vtysh_readline_init ()
{
  /* readline related settings. */
  rl_bind_key ('?', vtysh_rl_describe);
  rl_completion_entry_function = vtysh_completion_empty;
  rl_attempted_completion_function = (CPPFunction *)new_completion;
  /* do not append space after completion. It will be appended
     in new_completion() function explicitly */
  rl_completion_append_character = '\0';
}

char *
vtysh_prompt ()
{
  struct utsname names;
  static char buf[100];
  const char*hostname;
  extern struct host host;

  hostname = host.name;

  if (!hostname)
    {
      uname (&names);
      hostname = names.nodename;
    }

  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname);

  return buf;
}

struct cmd_node exec_node =
{
  EXEC_MODE,
  "\rCTC_CLI# ",
};

struct cmd_node sdk_node =
{
  CTC_SDK_MODE,
  "\rCTC_CLI(ctc-sdk)# ",
};

struct cmd_node cmodel_node =
{
  CTC_CMODEL_MODE,
  "\rCTC_CLI(ctc-cmodel)# ",
};

struct cmd_node oam_chan_node =
{
  CTC_SDK_OAM_CHAN_MODE,
  "\rCTC_CLI(oam_chan)# ",
};

struct cmd_node fecd_node =
{
   CTC_FECD_MODE,
   "\rCTC_CLI(ctc-fecd)# ",
};

struct cmd_node debug_node =
{
   CTC_DEBUG_MODE,
   "\rCTC_CLI(ctc-debug)# ",
};

/**************************  <<< HUMBER SDK CLI V1.0 begin   **************************/
struct cmd_node sdk_v1_node =
{
   CTC_SDK_V1_MODE,
   "\rCTC_CLI(ctc-sdkv1)# ",
};

struct cmd_node internal_node =
{
   CTC_INTERNAL_MODE,
   "\rCTC_CLI(ctc-internal)# ",
};

void
vtysh_install_common_cmds(int mode)
{
  /* "exit" and "quit" command. */
  install_element (mode, &vtysh_exit_all_cmd);
  install_element (mode, &vtysh_quit_all_cmd);

  /* "end" command. */
  if (EXEC_MODE != mode)
  {
      install_element (mode, &vtysh_end_all_cmd);
  }
  return;
}

void
vtysh_vty_init ()
{
  /* Make vty structure. */
  vty = vtysh_vty_new ();
  vty->type = VTY_SHELL;
  vty->node = CTC_SDK_MODE;

  /* Initialize commands. */
  ctc_cmd_init (0);

  /* Install top nodes. */
  ctc_install_node (&exec_node, NULL);
  ctc_install_node (&sdk_node, NULL);
  ctc_install_node (&cmodel_node, NULL);
  ctc_install_node (&fecd_node, NULL);
  ctc_install_node (&oam_chan_node, NULL); 
  ctc_install_node (&debug_node, NULL);     
  ctc_install_node (&sdk_v1_node, NULL);
  ctc_install_node (&internal_node, NULL);
  
  vtysh_install_default (EXEC_MODE);
  vtysh_install_default (CTC_SDK_MODE);
  vtysh_install_default (CTC_CMODEL_MODE);
  vtysh_install_default (CTC_SDK_OAM_CHAN_MODE);
  vtysh_install_default (CTC_FECD_MODE);
  vtysh_install_default (CTC_DEBUG_MODE);
  vtysh_install_default (CTC_SDK_V1_MODE);
  vtysh_install_default (CTC_INTERNAL_MODE);

#if 0
  ctc_install_node (&interface_node, NULL);
  vtysh_install_default (INTERFACE_NODE);

  install_element (EXEC_MODE, &vtysh_enable_cmd);
  install_element (CTC_SDK_MODE, &vtysh_config_terminal_cmd);
  install_element (CTC_SDK_MODE, &vtysh_disable_cmd);

  install_element (INTERFACE_NODE, &vtysh_end_all_cmd);
  install_element (INTERFACE_NODE, &vtysh_exit_interface_cmd);
  install_element (INTERFACE_NODE, &vtysh_quit_interface_cmd);
  install_element (CTC_SDK_MODE, &vtysh_interface_cmd);
  install_element (CTC_SDK_MODE, &vtysh_show_running_config_cmd);
  install_element (CTC_SDK_MODE, &vtysh_copy_runningconfig_startupconfig_cmd);
  install_element (CTC_SDK_MODE, &vtysh_write_file_cmd);

  /* write terminal command */
  install_element (CTC_SDK_MODE, &vtysh_write_terminal_cmd);
  install_element (CTC_SDK_MODE, &vtysh_write_terminal_cmd);
#endif
#if 0
  int i;
  for (i = EXEC_MODE; i <= CTC_DEBUG_MODE; i++)
  {
      vtysh_install_common_cmds(i);
  }
  #endif
  
  vtysh_install_common_cmds(EXEC_MODE);
  vtysh_install_common_cmds(CTC_SDK_MODE);
  vtysh_install_common_cmds(CTC_CMODEL_MODE);
  vtysh_install_common_cmds(CTC_DEBUG_MODE);
  vtysh_install_common_cmds(CTC_INTERNAL_MODE);
#if 0
  /* write memory command */
  install_element (CTC_SDK_MODE, &vtysh_write_memory_cmd);
  install_element (CTC_SDK_MODE, &vtysh_write_memory_cmd);
  install_element (INTERFACE_NODE, &vtysh_write_memory_cmd);

  install_element (EXEC_MODE, &vtysh_ping_cmd);
  install_element (EXEC_MODE, &vtysh_traceroute_cmd);
  install_element (EXEC_MODE, &vtysh_telnet_cmd);
  install_element (EXEC_MODE, &vtysh_telnet_port_cmd);
  install_element (CTC_SDK_MODE, &vtysh_ping_cmd);
  install_element (CTC_SDK_MODE, &vtysh_traceroute_cmd);
  install_element (CTC_SDK_MODE, &vtysh_telnet_cmd);
  install_element (CTC_SDK_MODE, &vtysh_telnet_port_cmd);
  install_element (CTC_SDK_MODE, &vtysh_start_shell_cmd);
#endif

#if 0
  extern void vtysh_mode_init(void);
  vtysh_mode_init();
#endif
  return;
}

