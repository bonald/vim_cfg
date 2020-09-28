
#include "proto.h"
#include "ssm.h"
#include "fei.h"

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
ssm_sock_init()
{
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(10);
    
    /* client to FEA */
    fei_init(lib_get_master(), NULL, TRUE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    /* client to trap agent */
    ipc_connect_snmp_trap();
    
    return PM_E_NONE;
}
    
int32
ssm_module_init()
{
    cdb_register_cfg_tbl_cb(TBL_SSM_GLOBAL, ssm_cmd_process_ssm_global);
    
    cdb_register_cfg_tbl_cb(TBL_SSM_PORT, ssm_cmd_process_ssm_port);

    cdb_pm_subscribe_tbl(TBL_SSM_GLOBAL, ssm_set_field_sync, NULL);
    
    cdb_register_cfg_act_cb(ACT_SHOW_SSM, ssm_cmd_process_show_ssm);
    
    cdb_register_cfg_tbl_cb(TBL_SSM_DEBUG, ssm_cmd_process_ssm_debug);
    
    ipc_register_pkt_rx_fn(GLB_PKT_SSM, ssm_pdu_rx);

    /*ssm global initialization*/
    ssm_global_init(); 
    
    /*Subscribe Table Interface*/
    ssm_sync_init();
        
    
    return PM_E_NONE;
}

int32
ssm_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_SSM);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;

    ssm_sock_init();
    ssm_module_init();
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
 
    ssm_init(daemon_mode);
    exit(0);
}

