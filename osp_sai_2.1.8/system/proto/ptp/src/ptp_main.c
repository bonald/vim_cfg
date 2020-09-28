
#include "proto.h"
#include "ptp.h"
#include "fei.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_util.h"
#include "ptp_sock.h"
#include "ptp_constant.h"
#include "ptp_ops.h"
#include "ptp_debug.h"

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
ptp_sock_init()
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
ptp_global_init()
{   
    int32 ret = 0;
    ret = ptp_ops_global_init();
    if (ret < 0)
    {
        PTP_LOG_ERR( "Error initialize global proprietary.");
        //lib_stop(g_p_ptpd);
        return ret;
    }

    //logging_module_init();
    return PM_E_NONE;
}
    
int32
ptp_module_init()
{
    cdb_register_cfg_tbl_cb(TBL_PTP_DEBUG, ptp_cmd_process_ptp_debug);
    
    cdb_register_cfg_tbl_cb(TBL_PTP_GLOBAL, ptp_cmd_process_ptp_global);
    cdb_register_cfg_tbl_cb(TBL_PTP_PORT, ptp_cmd_process_ptp_port);
    cdb_register_cfg_tbl_cb(TBL_PTP_FOREIGN, ptp_cmd_process_ptp_foreign);
    cdb_register_cfg_tbl_cb(TBL_PTP_RESIDENCE_TIME_CACHE, ptp_cmd_process_ptp_foreign);

    cdb_register_cfg_act_cb(ACT_SHOW_PTP, ptp_cmd_process_show);
    cdb_register_cfg_act_cb(ACT_CLEAR_PTP, ptp_cmd_process_clear);

    ptp_sync_init();

    ptp_global_init();

    ipc_register_pkt_rx_fn(GLB_PKT_PTP, ptp_packet_rx);
    return PM_E_NONE;
}

int32
ptp_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_PTP);
    /*ptp global init*/
    
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
//#ifdef LDB_SYNC_TO_RDB
//    rdb_connect();
//#endif
    ptp_sock_init();
    ptp_module_init();
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
 
    ptp_init(daemon_mode);
    exit(0);
}

