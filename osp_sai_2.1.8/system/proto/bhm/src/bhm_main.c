
#include "proto.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_module_define.h"
#include "bhm.h"

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
        sal_printf("\t-v, --version\tPrint version.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
    }
}

int32
bhm_sock_init(lib_master_t *master)
{
    int32 sock = INVALID_SOCK_ID;
    
    master->trap_sock = INVALID_SOCK_ID;
    
    /* client to CDS */
    ipc_connect_ccs_cds(10);
    
    /* BHM socket */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "BHMServer");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_sock_set_pmid(sock, master->pm_id);
    ctc_sock_bind(sock, SOCK_ADDR_BHM);
    master->bhm_sock = sock;

    ipc_connect_snmp_trap();
    
    return PM_E_NONE;
}

int32
bhm_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_BHM);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;

    bhm_sock_init(lib_master);
    bhm_module_init();
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif   
    ctc_task_main();

    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "d:h:v";

/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
 { "daemon",      no_argument,       NULL, 'd'},
 { "help",        no_argument,       NULL, 'h'},
 { "version",     no_argument,       NULL, 'v'},
 { 0 }
};
#endif /* HAVE_GETOPT_H */

    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {   
         int32 opt;
   
#ifdef HAVE_GETOPT_H
         opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
         opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
   
         if (EOF == opt)
         {   
             break;
         }
     
         switch (opt)
         {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
             usage(FALSE, progname);
             break;
         default:
             usage(TRUE, progname);
             break;
         }
    }
 
    bhm_init(daemon_mode);
    exit(0);
}
