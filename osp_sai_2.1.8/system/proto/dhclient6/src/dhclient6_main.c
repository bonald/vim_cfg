
#include "proto.h"
#include "dhclient6.h"
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
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"

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

extern int32
dhclient_process_dhclient6_show(char **argv, int32 argc, cfg_cmd_para_t *para);

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

/* --- dhcpv6bg --- */
static int
_check_dhcpv6dbg_field(char *pszkey, char *pszValue, tbl_dhcpv6_debug_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int32 nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "invalid value for %s", pszkey); 
        goto err_out;
    }
    nValue = atoi(pszValue);
    switch ((tbl_dhcpv6_debug_field_id_t)field_id) {
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP:

        GLB_SET_FLAG_COND(nValue, pitem->flags, (1 << field_id));
        break;
    case TBL_DHCPV6_DEBUG_FLD_MAX:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_dhcpv6dbg_cmd_set(tbl_dhcpv6_debug_field_id_t field_id, tbl_dhcpv6_debug_t *pitem, 
    tbl_dhcpv6_debug_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (field_id >= TBL_DHCPV6_DEBUG_FLD_MAX) {
        return PM_E_FAIL;
    }
    
    switch ((tbl_dhcpv6_debug_field_id_t)field_id) {
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET:
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP:

        if ((pitem->flags & (1 << field_id)) 
                == (p_db_item->flags & (1 << field_id))) {
            return PM_E_NONE;
        }

        break;
    case TBL_DHCPV6_DEBUG_FLD_MAX:
        break;
    }
    

    PM_E_RETURN(tbl_dhcpv6_debug_set_dhcpv6_debug_field(pitem, field_id));
    return rc;
}

int32
dhcpv6dbg_cmd_get(tbl_dhcpv6_debug_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dhcpv6_debug_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

uint32
dhclient6_stm_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }
    
    return FALSE;
}


int32
dhcpv6dbg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_dhcpv6_debug_field_id_t field_id = TBL_DHCPV6_DEBUG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_dhcpv6_debug_t item;
    tbl_dhcpv6_debug_t *p_db_item = NULL;
    int32 value = 0;

    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);    
    p_db_item = tbl_dhcpv6_debug_get_dhcpv6_debug();
    switch (para->oper) {
    case CDB_OPER_SET:
        if (0 == sal_strcmp(argv[i], "all"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

            GLB_SET_FLAG_COND(value, item.flags, DHCPV6DBG_FLAG_CLIENTV6_ERROR);
            GLB_SET_FLAG_COND(value, item.flags, DHCPV6DBG_FLAG_CLIENTV6_EVENTS);
            GLB_SET_FLAG_COND(value, item.flags, DHCPV6DBG_FLAG_CLIENTV6_PACKET);
            GLB_SET_FLAG_COND(value, item.flags, DHCPV6DBG_FLAG_CLIENTV6_DUMP);

            rc = tbl_dhcpv6_debug_set_dhcpv6_debug_field(&item, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR);
            rc = tbl_dhcpv6_debug_set_dhcpv6_debug_field(&item, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS);
            rc = tbl_dhcpv6_debug_set_dhcpv6_debug_field(&item, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET);
            rc = tbl_dhcpv6_debug_set_dhcpv6_debug_field(&item, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP);
        }
        else
        {
            if ((argc % 2) != 0) {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }

            for (i = 0; i < argc; i += 2) {
                if (_check_dhcpv6dbg_field(argv[i], argv[i + 1], &item, para) == -1) {
                    return PM_E_CFG_INVALID_PARAM;
                }

                cdb_get_field_id(argv[i], p_node, &p_field);
                if (CDB_ACCESS_RD == p_field->access) {
                    CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
                }
            }

            for (i = 0; i < argc; i += 2) {
                field_id = cdb_get_field_id(argv[i], p_node, &p_field);
                rc |= _dhcpv6dbg_cmd_set(field_id, &item, p_db_item, para);
            }
        }
        break;

    case CDB_OPER_GET:
        /* 
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        */
        
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = dhcpv6dbg_cmd_get(p_db_item, para, p_node, &field);    

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
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
    //cdb_pm_subscribe_tbl(TBL_MANAGE_IF, NULL, dhclient_process_mgmt_if);
    //cdb_register_cfg_act_cb(ACT_SHOW_DHCLIENT, dhclient_process_dhclient_show);
    cdb_register_cfg_act_cb(ACT_SHOW_DHCLIENT6, dhclient_process_dhclient6_show);
    cdb_register_cfg_tbl_cb(TBL_DHCPV6_DEBUG, dhcpv6dbg_cmd_process);
    cdb_register_cfg_act_cb(ACT_DHCPV6_L3IF_PRE_DELETE, dhclient_cmd_pre_delete);
    cdb_register_cfg_act_cb(ACT_DHCPV6_L3IF_PRE_SHUTDOWN, dhclient_cmd_pre_shutdown);
    
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

