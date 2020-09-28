
#include "proto.h"
#include "dhclient.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcrelay.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
#include "gen/tbl_dhcpv6_debug_define.h"
#include "gen/tbl_dhcpv6_debug.h"

#include "fei.h"
#include "genlog.h"
#include "iv.h"
#include "dhcpd.h"
#include "ctc_task.h"
#include "dhclient_func.h"
#include "cfg_cmd.h"

DEBUG_DEFINE(dhcp_debug);
DEBUG_DEFINE(dhcpv6_debug);


extern int32
dhclient_process_intf_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

extern int32
dhclient_process_route_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);
int32
dhclient_process_mgmt_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

extern int32
dhclient_process_dhclient_show(char **argv, int32 argc, cfg_cmd_para_t *para);

/* modified by liwh for bug 50219, 2018-12-19 */
extern int32
dhclient_cmd_pre_delete(char **argv, int32 argc, cfg_cmd_para_t *para);
/* liwh end */

/* modified by liwh for bug 50646, 2019-01-02 */
extern int32
dhclient_cmd_pre_shutdown(char **argv, int32 argc, cfg_cmd_para_t *para);
/* liwh end */

int _is_startup_done()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->startup_done) {
        return 1;
    }
    return 0;
}

int is_dhcp_relay_enabled()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->dhcp_service_enable && pGlobal->dhcp_relay_enable) {
        return 1;
    }
    return 0;
}

int is_dhcp_service_enabled()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->dhcp_service_enable) {
        return 1;
    }
    return 0;
}

static int32
dhclient_process_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id) {
            dhclient_proc_startup_done();
        } else if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id) {
            dhclient_proc_init_done();
        } else if (TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE == field_id 
                || TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE == field_id) {
#if 0
            dhcprelay_service_onoff(pGlobal->dhcp_service_enable 
                && pGlobal->dhcp_relay_enable);
#endif                
        }
        break;

    case CDB_OPER_GET:
        break;

    default:
        break;
    }
    
    return rc;
}

int32
dhclient_module_init()
{
    ipc_register_pkt_rx_fn(GLB_PKT_DHCP, dhclient_pdu_rx);
    cdb_register_cfg_tbl_cb(TBL_DHCLIENT, dhclient_cmd_process);
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, dhclient_process_sys_global);
    cdb_pm_subscribe_tbl(TBL_INTERFACE, NULL, dhclient_process_intf_if);
    cdb_pm_subscribe_tbl(TBL_ROUTE_IF, NULL, dhclient_process_route_if);
    cdb_pm_subscribe_tbl(TBL_MANAGE_IF, NULL, dhclient_process_mgmt_if);
    cdb_register_cfg_act_cb(ACT_SHOW_DHCLIENT, dhclient_process_dhclient_show);
    cdb_register_cfg_act_cb(ACT_DHCP_L3IF_PRE_DELETE, dhclient_cmd_pre_delete);
    cdb_register_cfg_act_cb(ACT_DHCP_L3IF_PRE_SHUTDOWN, dhclient_cmd_pre_shutdown);

    return PM_E_NONE;
}

void dhclient_cowork_release(void *pArg)
{
    DhcpCoworkArg_t *pCowork = (DhcpCoworkArg_t *)pArg;
    ctc_task_t *pTask = (ctc_task_t *)pCowork->pArgCowork;

    if (pCowork->isread) {
        ctc_task_delete_read(pTask);
    } else {
        //ctc_task_delete_write(pTask); write and read in same fd
    }
    free(pCowork);
    return;
}

void dhclient_cowork_handler(void *pArg)
{
    DhcpCoworkArg_t *pCowork = (DhcpCoworkArg_t *)pArg;
    pCowork->cb(pCowork->fd, pCowork->isread, pCowork->pArgDhcp);
    return;
}

int dhclient_cowork_watch(int fd, int isread, void *io, OmapiProcCB *proc, void **ppArg)
{
    DhcpCoworkArg_t *pArg = NULL;
    ctc_task_t *pTask = NULL;
    int ret = -1;

    *ppArg = NULL;
    if ((pArg = calloc(1, sizeof(DhcpCoworkArg_t))) == NULL) {
        /* out of memory */
        goto err_out;
    }

    if (isread) {
        pTask = ctc_task_add_read(TASK_PRI_NORMAL, dhclient_cowork_handler, pArg, fd);
    } else {
        pTask = ctc_task_add_read(TASK_PRI_NORMAL, dhclient_cowork_handler, pArg, fd);
    }
    if (NULL == pTask) {
        goto err_out;
    }
    pArg->cb = proc;
    pArg->fd = fd;
    pArg->isread = isread;
    pArg->pArgCowork = (void *)pTask;
    pArg->pArgDhcp = io;

    *ppArg = pArg;
    pArg = NULL;
    pTask = NULL;
    ret = 0;
    
err_out:
    if (NULL != pArg) {
        free(pArg);
    }
    if (NULL != pTask) {
        if (isread) {
            ctc_task_delete_read(pTask);
        } else {
            //ctc_task_delete_write(pTask); write and read in same fd
        }
    }
    return ret;
}

void dhclient_cowork_init(void)
{
    omapi_io_cowork_register(dhclient_cowork_watch, dhclient_cowork_release);
    return;
}

int32
dhclient_init(int pm_id)
{     
    lib_master_t *lib_master = lib_init(pm_id);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
    pm_id_t pmid = (PM_ID_DHCLIENT == pm_id) ? M_MOD_DHCLIENT : M_MOD_DHCLIENT6;

    DEBUG_INIT(dhcp_debug, pmid);
    DEBUG_INIT(dhcpv6_debug, pmid);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(0);

    /* client to FEA */
    fei_init(lib_get_master(), NULL, FALSE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    dhclient_mgmt_init();
    dhclient_module_init();

    log_sys(pmid, E_INFORMATIONAL, "Dhclient ready to service.");
    dhclient_cowork_init();
    ctc_task_main_cowork(omapi_io_cowork_run);
    return PM_E_NONE;
}
