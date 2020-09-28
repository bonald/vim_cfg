
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "cli.h"
//#include <readline/readline.h>
#include <time.h>
#include <sys/time.h>
#include "clish_exec.h"
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include "clish.h"
#include "clish_filesystem.h"
#include "clish_cli.h"

#define MAX_TIME_LEN 200
#define CLISH_MAXARGS      32
#define MAX_LINE_CHAR    50
#define M_APP_RATIO_BUFSZ   32

/* Output modifier.  */
CLI (clish_modifier_begin,
     clish_modifier_begin_cli,
     "|| begin LINE",
     "Output modifiers",
     "Begin with the line that matches",
     "Regular Expression")
{
    return CLISH_MODIFIER_BEGIN;
}

CLI (clish_modifier_include,
     clish_modifier_include_cli,
     "|| include LINE",
     "Output modifiers",
     "Include lines that match",
     "Regular Expression")
{
    return CLISH_MODIFIER_INCLUDE;
}

CLI (clish_modifier_exclude,
     clish_modifier_exclude_cli,
     "|| exclude LINE",
     "Output modifiers",
     "Exclude lines that match",
     "Regular Expression")
{
    return CLISH_MODIFIER_EXCLUDE;
}

CLI (clish_modifier_grep,
     clish_modifier_grep_cli,
     "|| grep WORD",
     "Output modifiers",
     "Grep output",
     "Regular Expression")
{
    return CLISH_MODIFIER_GREP;
}

CLI (clish_modifier_grep_option,
     clish_modifier_grep_option_cli,
     "|| grep WORD WORD",
     "Output modifiers",
     "Grep output",
     "Grep option",
     "Regular Expression")
{
    return CLISH_MODIFIER_GREP;
}


/* modified by cuixl for bug 17129 , 2011-11-24 */
CLI (clish_overwrite_redirect_to_file,
     clish_overwrite_redirect_to_file_cli,
     "> GFILENAME overwrite",
     "Redirect output",
     "Output file",
     "overwrite old file")
{
     return CLISH_MODIFIER_REDIRECT;
}


/* check redirect file name */
int
clish_check_redirect_name(struct cli *cli, char *pszFilename)
{
  char szFullName[M_FULLPATH_MAX_LEN];

  if (NULL == cli || NULL == pszFilename)
  {
      return CLISH_MODIFIER_FILENAME_ERROR;
  }
  
  if (gen_check_and_get_filename(cli, pszFilename, szFullName, M_FULLPATH_MAX_LEN) != 0)
  {
      cli_out(cli, "%% Invalid redirect file name: %s\n", pszFilename);
      return CLISH_MODIFIER_FILENAME_ERROR;
  }

  if (syscmd_is_directory(szFullName))
  {
      cli_out(cli, "%% Is directory error: %s\n", pszFilename);
      return CLISH_MODIFIER_FILENAME_ERROR;
  }

  return CLISH_MODIFIER_REDIRECT;
}

CLI (clish_redirect_to_file,
     clish_redirect_to_file_cli,
     "> GFILENAME",
     "Redirect output",
     "Output file")
{
    return clish_check_redirect_name(cli, argv[0]);
}

int clish_show_cli_init(struct cli_tree *ctree)
{
    int i = 0;

    /* Output modifier set up.  */
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, 0,
            &clish_modifier_begin_cli);
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, 0,
            &clish_modifier_include_cli);
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, 0,
            &clish_modifier_exclude_cli);
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN,
            &clish_modifier_grep_cli);
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN,
            &clish_modifier_grep_option_cli);
            
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN,
            &clish_overwrite_redirect_to_file_cli);
    cli_install_gen (ctree, MODIFIER_MODE, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN,
            &clish_redirect_to_file_cli); 
            
    /* Set show flag to "show" and "*s=show" node.  */
    for (i = 0; i < MAX_MODE; i++)
    {

        if (i != EXEC_MODE)
        {
            cli_set_node_flag (ctree, i, "read", CLI_FLAG_SHOW);
            cli_set_node_flag (ctree, i, "show", CLI_FLAG_SHOW);
            cli_set_node_flag (ctree, i, "s", CLI_FLAG_SHOW);
        }
    }
    
    return 0;
}
