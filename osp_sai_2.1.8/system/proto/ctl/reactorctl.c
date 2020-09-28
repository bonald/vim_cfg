
#include "proto.h"
#include "cdbclient.h"
#include "cdbclient_subscribe.h"
#include "param_check.h"
#include "lib_fs.h"

#include <getopt.h>
#include <unistd.h>

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
        sal_printf("Reactor to subscribe a CDB path or CDB tables.\n");
        sal_printf("Options:\n");
        sal_printf("\t-c, --cdb\tFull CDB path.\n");
        sal_printf("\t-t, --table\tTables joined by ','.\n");
        sal_printf("\t-l, --log\tLog to syslog.\n");
        sal_printf("\t-b, --bash\tBash script path.\n");
        sal_printf("\t-s, --sync\tSync database.\n");
        sal_printf("\t-p, --protobuf\tReturn protobuf format.\n");
    }
}

#define REACTOR_ACTION_PRINT    0
#define REACTOR_ACTION_LOG      1
#define REACTOR_ACTION_SCRIPT   2
extern char g_cdbctl_subscribe_script_path[CMD_BUF_256];

int32
main(int32 argc, char **argv)
{
    int32 rc = PM_E_NONE;
    char *p;
    char *progname;
    cdb_subscribe_format_t format = CDB_SUB_FORMAT_RAW;
    cdb_subscribe_type_t type = CDB_SUB_CDB;
    uint32 action = REACTOR_ACTION_PRINT;
    uint32 sync = FALSE;

    static const char *shortopts = "hctlpsb:";
    
    /* Command line options. */
    struct option longopts[] =
    {
        { "help",        no_argument,       NULL,  'h'},
        { "cdb",         no_argument,       NULL,  'c'},
        { "table",       no_argument,       NULL,  't'},
        { "log",         no_argument,       NULL,  'l'},
        { "protobuf",    no_argument,       NULL,  'p'},
        { "sync",        no_argument,       NULL,  's'},
        { "bash",        required_argument, NULL,  'b'},
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
        case 'c':
            type = CDB_SUB_CDB;
            break;
        case 't':
            type = CDB_SUB_TBL;
            break;
        case 'l':
            action = REACTOR_ACTION_LOG;
            break;
        case 'p':
            format = CDB_SUB_FORMAT_PROTOBUF;
            break;
        case 's':
            sync = TRUE;
            break;
        case 'b':
            action = REACTOR_ACTION_SCRIPT;
            sal_memset(g_cdbctl_subscribe_script_path, 0, sizeof(g_cdbctl_subscribe_script_path));
            sal_strncpy(g_cdbctl_subscribe_script_path, optarg, CMD_BUF_256);
            break;
        case 'h':
        default:
            usage(0, progname);
            exit (1);
            break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "%% Please specify CDB path\r\n");
        exit (1);
    } else if ((argc - optind) > 1) {
        fprintf(stderr, "%% Too many argument\r\n");
        exit (1);
    }  

    /* 1. init lib and task */
    lib_init(PM_ID_CTL);
    ctc_task_init();

    switch (action)
    {
    case REACTOR_ACTION_PRINT:
        cdbclt_subscribe_register_action_cb(cdbctl_subscribe_action_print);
        break;
    case REACTOR_ACTION_LOG:
        cdbclt_subscribe_register_action_cb(cdbctl_subscribe_action_log);
        break;
    case REACTOR_ACTION_SCRIPT:
        cdbclt_subscribe_register_action_cb(cdbctl_subscribe_action_script);
        break;
    default:
        cdbclt_subscribe_register_action_cb(cdbctl_subscribe_action_print);
        break;
    }
    
    /* 2. sync connect to CCS */
    rc = ipc_connect_ccs_subscribe(5);
    if (rc == 0)
    {
        /* 3. set react path */
        p = argv[argc - 1]; /* pointer to command string */

        if (CDB_SUB_CDB == type)
        {
            rc = cdbclt_subscribe_init_cdb(p, format);
        }
        else
        {
            if (format == CDB_SUB_FORMAT_PROTOBUF)
            {
                rc = cdbclt_subscribe_init_tbl_new(p, format, sync);
            }
            else
            {
                rc = cdbclt_subscribe_init_tbl(p, format, sync);
            }
        }
        if (rc < 0)
        {
            ipc_sock_deinit();
            exit(rc);
        }
    }

    /* 4. run task loop and wait reactor msg from CDS */
    ctc_task_main();
    exit(rc);
}

