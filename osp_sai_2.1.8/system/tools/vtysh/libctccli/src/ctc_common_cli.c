#ifdef SDK_IN_USERMODE
#include <dirent.h>
#endif
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_common_cli.h"

bool vtysh_source_quiet_on = FALSE;

#define WHITE_SPACE(C)        ((C) == '\t' || (C) == ' ')
#define EMPTY_LINE(C)         ((C) == '\0' ||(C) == '\r' || (C) == '\n' )
#if 0
static int
_parser_string_atrim(char* output, const char* input)
{
    char *p = NULL;

    if (!input)
    {
        return -1;
    }

    if (!input)
    {
        return -1;
    }

    /*trim left space*/
    while(*input != '\0')
    {
        if(WHITE_SPACE(*input))
        {
            ++input;
        }
        else
        {
            break;
        }
    }
    sal_strcpy(output, input);

    /*trim right space*/
    p = output + sal_strlen(output) - 1;

    while(p >= output)
    {
      /*skip empty line*/
        if(WHITE_SPACE(*p) || ('\r' == (*p)) || ('\n' == (*p)))
        {
            --p;
        }
        else
        {
            break;
        }
    }

    *(++p) = '\0';

    return 0;
}


/* Cmd format: delay <M_SEC> */
CTC_CLI(cli_com_delay,
    cli_com_delay_cmd,
    "delay M_SEC",
    "delay time",
    "delay million seconds")
{
    uint32 delay_val = 0;

    CTC_CLI_GET_UINT32("million seconds", delay_val, argv[0]);
    sal_task_sleep(delay_val*1000);

    return CLI_SUCCESS;
}

/* Cmd format: source quiet (on|off) */
CTC_CLI(cli_com_source_quiet,
    cli_com_source_quiet_cmd,
    "source quiet (on|off)",
    "Common cmd",
    "Source quiet",
    "on",
    "off")
{
    if (0 == sal_strncmp(argv[0], "on", sal_strlen("on")))
    {
        vtysh_source_quiet_on = TRUE;
    }
    else if (0 == sal_strncmp(argv[0], "off", sal_strlen("off")))
    {
        vtysh_source_quiet_on = FALSE;
    }
    else
    {
        ctc_cli_out("%% Error! The 1th para is Invalid, %s\n",argv[0]);
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* Cmd format: source <file_name> */
CTC_CLI(cli_com_source_file,
    cli_com_source_file_cmd,
    "source INPUT_FILE",
    "Common cmd",
    "Input file path")
{
    #define MAX_CLI_STRING_LEN 512
    int32 ret = 0;
    FILE *fp = NULL;
    char *filename = NULL;
    char string[MAX_CLI_STRING_LEN]= {0};
    char line[MAX_CLI_STRING_LEN] = {0};

    filename = argv[0];

    fp = fopen(filename, "r");
    if(NULL == fp)
    {
        ctc_cli_out("%% Failed to open the file <%s>\n", filename);
        return CLI_ERROR;
    }

    while(!feof(fp))
    {
        sal_memset(string, 0, sizeof(string));
        fgets(string, MAX_CLI_STRING_LEN, fp);

        /*comment line*/
        if('#' == string[0])
        {
          continue;
        }

        /*trim left and right space*/
        sal_memset(line, 0, sizeof(line));
        ret = _parser_string_atrim(line, string);
        if (ret < 0)
        {
            ctc_cli_out("ERROR! Fail to Paser line %s", string);
        }

        if(EMPTY_LINE(line[0]))
        {
          continue;
        }
        sal_strcat(line, "\n");
        if (!vtysh_source_quiet_on)
        {
            ctc_cli_out("%s", line);
        }

        ret = vtysh_vty_command(vty, line);
        if(ret && vtysh_source_quiet_on)
        {
            ctc_cli_out("%s", line);
        }
    }

    fclose(fp);
    fp = NULL;

    return CLI_SUCCESS;
}

CTC_CLI (show_history,
       cli_com_show_history_cmd,
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
            sal_printf("%d  %s%s", print_index, vty->hist[index], VTY_NEWLINE);
            print_index++;
        }
        index++;
    }
    return CLI_SUCCESS;
}

#ifdef SDK_IN_USERMODE
/* Cmd format: pwd */
CTC_CLI(cli_com_show_file_dir,
    cli_com_show_file_dir_cmd,
    "pwd",
    "Common cmd")
{

    char path[256] = {0};

    sal_memset(path, 0, 256);
    if(getcwd(path, 256) == NULL)
    {
        ctc_cli_out("%% Show current work directory failed!\n");
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("%s\n", path);
    }

    return CLI_SUCCESS;
}


/* Cmd format: cd <path> */
CTC_CLI(cli_com_change_file_dir,
    cli_com_change_file_dir_cmd,
    "cd FILE_PATH",
    "Common cmd",
    "File path")
{
    int32 i;
    char path[256] = {0};
    char* work_dir = NULL;

    sal_memset(path, 0, 256);

    work_dir = argv[0];

    /*ingore one dot*/
    if(0 == sal_strcmp(work_dir, "."))
    {
        return CLI_ERROR;
    }

    /*switch to parent directory*/
    if(0 == sal_strcmp(work_dir, ".."))
    {
        if(getcwd(path, 256) == NULL)
        {
            ctc_cli_out("%% Get current work directory failed!\n");
            return CLI_ERROR;
        }
        i = sal_strlen(path);

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
            return CLI_ERROR;
        }

        i = sal_strlen(path);
        path[i] = '/';
        sal_strcat(path, work_dir);
        chdir(path);

        ret = chdir(path);
        if (ret == -1)
        {
            sal_memset(path, 0, sizeof(path));
            sal_memcpy(path, work_dir, sizeof(path));
            ret = chdir(path);
            if (ret == -1)
            {
                ctc_cli_out("%% Invalid path: %s!\n", path);
                return CLI_ERROR;
            }
        }

    }

    return CLI_SUCCESS;
}

/* Cmd format: ls <path>*/
CTC_CLI(cli_com_list_file,
    cli_com_list_file_cmd,
    "ls",
    "Common cmd")
{
    DIR* dp;
    char path[256] = {0};
    struct dirent* dirp;
    int16 count = 0;
    int16 i = 0;
    uint32 len;

    sal_memset(path, 0, 256);
    if(getcwd(path, 256) == NULL)
    {
        ctc_cli_out("%% Open work directory failed!\n");
        return CLI_ERROR;
    }

    len = sal_strlen(path);
    path[len] = '/';

    if(!(dp = opendir(path)))
    {
        ctc_cli_out("%% Can't read directory!\n");
        return CLI_ERROR;
    }

    while((dirp = readdir(dp)))
    {
        /*ignore dot and dot-dot*/
        if(0 == sal_strcmp(dirp->d_name, ".") ||0 == sal_strcmp(dirp->d_name, ".."))
        {
            continue;
        }

        if(DT_DIR == dirp->d_type)
        {
            if(0 == count%4)
            {
                ctc_cli_out("\n");
            }
            ctc_cli_out("<%-s>", dirp->d_name);
            if(sal_strlen(dirp->d_name) < 18)
            {
                for(i = 0; i < 18 - sal_strlen(dirp->d_name); i++)
                {
                    ctc_cli_out(" ");
                }
            }

            if(sal_strlen(dirp->d_name) >= 18)
            {
                ctc_cli_out("  ");
            }
             ++count;
        }
    }
    closedir(dp);

    dp = opendir(path);
    while((dirp = readdir(dp)))
    {
        /*ignore dot and dot-dot*/
        if(0 == sal_strcmp(dirp->d_name, ".") ||0 == sal_strcmp(dirp->d_name, ".."))
        {
            continue;
        }

        if(DT_DIR != dirp->d_type)
        {
            if(0 == count%4)
            {
                ctc_cli_out("\n");
            }
            ctc_cli_out("%-20s", dirp->d_name);

            if(sal_strlen(dirp->d_name) >= 20)
            {
                ctc_cli_out("  ");
            }

            ++count;
        }
    }
    ctc_cli_out("\n");

    closedir(dp);

    return CLI_SUCCESS;

}

/* Cmd format: tclsh <file.tcl> */
CTC_CLI(cli_com_tclsh,
    cli_com_tclsh_cmd,
    "tclsh SRC_FILE DST_FILE",
    "Common cmd",
    "Source file(format: *.tcl)",
    "Destionation file")
{
    char* src_filename = NULL, *dest_filename = NULL;
    char command[256] = {0};

    sal_memset(command, 0, 256);

    src_filename = argv[0];
    dest_filename = argv[1];

    sal_strcat(command, "tclsh ");
    sal_strcat(command, src_filename);
    if(NULL != dest_filename)
    {
        sal_strcat(command, " > ");
        sal_strcat(command, dest_filename);
    }

    system(command);

    return CLI_SUCCESS;

}

/* Cmd format: vi <file_name> */
CTC_CLI(cli_com_vi,
    cli_com_vi_cmd,
    "vi FILE_PATH",
    "Common cmd",
    "fFile path")
{
    char* filename = NULL;
    char command[256] = {0};

    sal_memset(command, 0, 256);

    filename = argv[0];

    sal_strcat(command, "vi ");
    sal_strcat(command, filename);
    system(command);

    return CLI_SUCCESS;

}
#endif
#endif

int32 ctc_com_cli_init(uint8 cli_tree_mode)
{
#if 0
    /* register some common cli */

    install_element(cli_tree_mode, &cli_com_source_quiet_cmd);
    install_element(cli_tree_mode, &cli_com_source_file_cmd);
    install_element(cli_tree_mode, &cli_com_show_history_cmd);

    #ifdef SDK_IN_USERMODE
    install_element(cli_tree_mode, &cli_com_delay_cmd);
    install_element(cli_tree_mode, &cli_com_list_file_cmd);
    install_element(cli_tree_mode, &cli_com_change_file_dir_cmd);
    install_element(cli_tree_mode, &cli_com_show_file_dir_cmd);
    install_element(cli_tree_mode, &cli_com_tclsh_cmd);
    install_element(cli_tree_mode, &cli_com_vi_cmd);
    #endif
#endif

    return 0;
}
