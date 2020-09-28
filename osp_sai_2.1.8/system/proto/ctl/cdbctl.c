
#include "proto.h"
#include "cdbclient.h"
#include "param_check.h"
#include "lib_fs.h"

#include <getopt.h>
#include <unistd.h>
#include "sdb_defines.h"

/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
        sal_printf("CLI utility. Command line shell."
            "The part of the klish project.\n");
        sal_printf("Options:\n");
        sal_printf("\t-g, --grep\tGrep string.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
        sal_printf("\t-f, --file\tFile path which store commands.\n");
        sal_printf("\t-t, --timeout\tSet execute timeout.\n");
    }
}

#define MAX_CDBCTL_ARGS_NUM 256

typedef struct
{
    int32 argc;
    char  *argv[MAX_CDBCTL_ARGS_NUM];
} cdbctl_args_t;

char g_cmdfile_lines[MAX_CDBCTL_ARGS_NUM][FILE_READ_LINE_LEN];
extern uint32 g_cdbclient_continue;

int32
cdbctl_get_cmds_from_file(char *file_name, cdbctl_args_t *pargs)
{
    FILE *fp = NULL;
    int32 isDir = FALSE;
    char *q = NULL;
    char *saveptr = NULL;
    int32 line_num = 0;
    int32 str_len = 0;

    pargs->argc = 0;

    isDir = lib_fs_is_directory(file_name);
    if (isDir)
    {
        return -1;
    }

    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return -2;
    }

    while (NULL != sal_fgets(g_cmdfile_lines[line_num], FILE_READ_LINE_LEN, fp))
    {
        q = NULL;
        saveptr = NULL;
        
        for (q = strtok_r(g_cmdfile_lines[line_num], " ", &saveptr); q; q = strtok_r(NULL, " ", &saveptr))
        {
            str_len = sal_strlen(q);
            if (str_len > 2)
            {
                if (q[str_len-1] == CMD_LINE_CHAR)
                {
                    q[str_len-1] = CMD_ZERO_CHAR;
                }
            }
            pargs->argv[pargs->argc] = q;
            pargs->argc++;
            if (pargs->argc > MAX_CDBCTL_ARGS_NUM)
            {
                sal_fclose(fp);
                return -3;
            }
        }
        line_num++;
        g_cmdfile_lines[line_num][0] = CMD_ZERO_CHAR;
    }

    sal_fclose(fp);
    
    return PM_E_NONE;
}

extern int rsa_init_crypto();

void cdbctl_sighquit(int sig)
{
    exit(0);
}

void cdbctl_sighint(int sig)
{
    cdbclt_sigint_clean();
    exit(0);
}

int32
main(int32 argc, char **argv)
{
    int32 rc = PM_E_NONE;
    char *p;
    char *progname;
    char *grep_str;
    int vrf = 0;
    uint32 timeout = 0;
    uint32 inFile = FALSE;
    cdbctl_args_t args;
    int32 i = 0;

    sal_memset(&args, 0, sizeof(args));

    /* Added by kcao for bug 39945, add SIGQUIT process */
    signal(SIGQUIT, cdbctl_sighquit);
    signal(SIGINT, cdbctl_sighint);
    
    static const char *shortopts = "hg:f:t:";
    
    /* Command line options. */
    struct option longopts[] =
    {
        { "help",        no_argument,       NULL,  'h'},
        { "grep",        required_argument, NULL,  'g'},
        { "file",        required_argument, NULL,  'f'},
        { "timeout",     required_argument, NULL,  't'},
        { 0 }
    };

    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);
    while (1)
    {
        int32 opt;
      
        opt = getopt_long(argc, argv, shortopts, longopts, 0);
        if (EOF == opt)
        {
            break;
        }

        switch (opt)
        {
        case 'g':
            grep_str = optarg;
            cdbctl_set_grep_str(grep_str);
            break;
        case 'f':
            inFile = TRUE;
            break;
        case 't':
            timeout = atoi(optarg);
            if (timeout == 0 || timeout > 300) {
                fprintf(stderr, "%% Invalid timeout, range [1, 300]\n");
                exit (1);
            }
            break;
        case 'h':
        default:
            usage(0, progname);
            exit (1);
            break;
        }
    }

    rsa_init_crypto();

    if (inFile)
    {
        rc = cdbctl_get_cmds_from_file(argv[2], &args);
        if (rc == -1)
        {
            sal_printf("%% command file %s is directory\n", argv[2]);
        }
        else if (rc == -2)
        {
            sal_printf("%% command file %s not exist\n", argv[2]);
        }
        else if (rc == -3)
        {
            sal_printf("%% command number is larger than %u \n", MAX_CDBCTL_ARGS_NUM);
            exit (1);
        }
    }
    else
    {
        if (optind >= argc)
        {
            fprintf(stderr, "%% Please specify command string\r\n");
            exit (1);
        }
        else if ((argc - optind) > 1)
        {
#if 0
            fprintf(stderr, "%% Too many argument\r\n");
            exit (1);
#endif
        }  

        args.argc = argc - optind;
        if (args.argc > MAX_CDBCTL_ARGS_NUM)
        {
            sal_printf("%% command number is larger than %u \n", MAX_CDBCTL_ARGS_NUM);
            exit (1);
        }
        for (i = 0; i < args.argc; i++)
        {
            args.argv[i] = argv[optind + i];
        }
    }

    lib_init(PM_ID_CTL);
    sdb_init(PM_ID_CTL);
    ctc_task_init();

    {
        /* setting vrf */
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
#ifndef SO_PERMVRF
#define SO_PERMVRF      101
#endif
        setsockopt(sock, SOL_SOCKET, SO_PERMVRF, (u_int8_t *)&vrf, sizeof(vrf));
        close(sock);
    }

    if (timeout)
    {
        ipc_set_sync_timeout(timeout);
    }

    ctclib_if_init();
    rc = ipc_connect_ccs(5);
    if (rc == 0)
    {
        for (i = 0; i < args.argc; i++)
        {
            p = args.argv[i];
            rc = cdbclt_process_cmd(p);
            if (rc < 0)
            {
                break;
            }
        }
    }

    if (!g_cdbclient_continue)
    {
        ipc_sock_deinit();
    }
    ctc_task_main();
    exit(rc);
}

