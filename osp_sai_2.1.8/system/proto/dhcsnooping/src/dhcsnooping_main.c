
#include <unistd.h>

#include "proto.h"
#include "dhcsnooping.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "fei.h"
#include "genlog.h"
#include "iv.h"
#include "dhcpd.h"
#include "ctc_task.h"
#include "lib_fs.h"

#include "dhcsnooping_func.h"

DEBUG_DEFINE(dhcp_debug);
#ifndef DHS_F_V4
#define DHS_F_V4 1
#define DHS_F_V6 2
#endif

int32 dhcsnooping_db_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 dhcsnooping_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 dhcbinding_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
extern int32 dhcsnooping_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);
extern int dhcsnooping_start();
extern void dhcsnooping_if_delete_cb(char *pszIntfName);
extern int dhcsnooping_service_onoff();
/* modified by liwh for bug 50994, 2019-02-13 */
extern int dhcsnooping_reserve_service_onoff(unsigned char bOn);
/* liwh end */
extern int32 dhcsnooping_process_intf_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

extern int32 dhcsnooping_process_route_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);
extern void dhcsnooping_binding_hash_del(tbl_dhcbinding_t *pItem);

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
dhcsnooping_process_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *pGlobal = (tbl_sys_global_t *)p_tbl;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id) {
            dhcsnooping_proc_init_done();
        } else if (TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE == field_id 
                || TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE == field_id) {
            dhcsnooping_service_onoff();
            /* modified by liwh for bug 50994, 2019-02-13 */ 
            dhcsnooping_reserve_service_onoff(pGlobal->dhcp_service_enable 
                && pGlobal->dhcp_snooping_enable);
            /* liwh end */           
        } else if (TBL_SYS_GLOBAL_FLD_HOSTNAME == field_id) {
            if ('\0' != pGlobal->hostname[0]) {
                tbl_dhcsnooping_t *pSnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
                if (pSnoopingCfg && pSnoopingCfg->hostname_as_remote_id) {
                    tbl_dhcsnooping_t stItem;
                    sal_memset(&stItem, 0x00, sizeof(tbl_dhcsnooping_t));
                    stItem.hostname_as_remote_id = 1;
                    snprintf(stItem.remote_id_string, 
                        DHCSNOOPING_MAX_REMOTE_ID_STRING,
                        "%s", pGlobal->hostname);
                    tbl_dhcsnooping_set_dhcsnooping_field(&stItem, 
                        TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID);
                    tbl_dhcsnooping_set_dhcsnooping_field(&stItem, 
                        TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING);
                }
            }
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
dhcpsnooping_delete_tbl_by_ifname(tbl_interface_t *p_db_if)
{
    tbl_dhcbinding_t    *pItem = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *nextnode = NULL;
    uint8 bRun = 0;
    uint8 bReq = 0;

    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) 
    {
        /*not this interface, continue*/
        if (sal_strcmp(pItem->ifname, p_db_if->key.name)) 
        {
            continue;
        }

        if (DHCBINDING_STATE_REQUEST == pItem->state) 
        {
            bReq = 1;
        }
        else 
        {
            bRun = 1;
        } 

        if (bRun) 
        {
            dhcsnooping_binding_hash_del(pItem);
        }

        if (bReq) 
        {
            tbl_dhcbinding_del_dhcbinding(&pItem->key);
        }
    }    

    return PM_E_NONE;
}

int32 
dhcsnooping_process_intf_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    if (NULL != p_if_key) 
    {
        p_db_if = tbl_interface_get_interface(p_if_key);
    }
    
    switch (oper) 
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_SET:
        /*fix bug 43867: if change eth port from l2 to l3, delete dhcp snooping binding table of this interface*/
        if (TBL_INTERFACE_FLD_MODE == field_id) 
        {
            /*after mode change to l3*//*snooping is not trust*/
            if (GLB_IF_MODE_L3 == p_db_if->mode && FALSE == p_db_if->dhcp_snooping_trust)
            {
                rc = dhcpsnooping_delete_tbl_by_ifname(p_db_if);
                if (PM_E_NONE != rc)
                {
                    return PM_E_FAIL;
                }
            }
        } 
       
        break;

    case CDB_OPER_DEL:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

int32
dhcsnooping_process_intf_msg_before(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    int32 rc = PM_E_NONE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    
    switch (oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        if (p_ds_node) {
            /* delete sub DS */
            break;
        }
        if (p_if_key && '\0' != p_if_key->name[0]) {
            dhcsnooping_if_delete_cb(p_if_key->name);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}


int32 
dhcsnooping_process_ipsg_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_dhcbinding_t *p_db_item = NULL;
    tbl_dhcbinding_t dhcpbinding_item;
    tbl_ipsg_fib_t      *p_db_ipsg_fib = NULL;

    p_db_ipsg_fib = (tbl_ipsg_fib_t *)p_tbl;
    if (NULL == p_db_ipsg_fib) 
    {
        return PM_E_FAIL;
    }

    sal_memset(&dhcpbinding_item, 0, sizeof(dhcpbinding_item));
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_SET:
        if (TBL_IPSG_FIB_FLD_SET_DHCBINDING_IPSG_TBL_EXSIT == field_id) 
        {         
            sal_memset(&dhcpbinding_item, 0x00, sizeof(tbl_dhcbinding_t));
            dhcpbinding_item.key.ipv6 = 0;
            dhcpbinding_item.key.vid = p_db_ipsg_fib->key.fid;
            sal_memcpy(dhcpbinding_item.key.mac, p_db_ipsg_fib->key.mac, sizeof(mac_addr_t));
            
            p_db_item = tbl_dhcbinding_get_dhcbinding(&(dhcpbinding_item.key));
            if (NULL == p_db_item)
            {
                dhcpbinding_item.key.ipv6 = 1;
                p_db_item = tbl_dhcbinding_get_dhcbinding(&(dhcpbinding_item.key));
                if (NULL == p_db_item)
                {
                    return PM_E_NOT_EXIST;
                }
            }

            if (p_db_ipsg_fib->set_dhcbinding_ipsg_tbl_exsit == p_db_item->ipsg_tbl_exsit) 
            {
                return PM_E_NONE;
            }
            
            dhcpbinding_item.ipsg_tbl_exsit = p_db_ipsg_fib->set_dhcbinding_ipsg_tbl_exsit;
            
            return tbl_dhcbinding_set_dhcbinding_field(&dhcpbinding_item, TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT);
        } 
       
        break;

    case CDB_OPER_DEL:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return PM_E_NONE;
}

int32
dhcsnooping_module_init()
{
    ipc_register_pkt_rx_fn(GLB_PKT_DHCP, dhcsnooping_pdu_rx);

    cdb_register_cfg_tbl_cb(TBL_DHCSNOOPING, dhcsnooping_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_DHCBINDING, dhcbinding_cmd_process);
    cdb_register_cfg_act_cb(ACT_DHCSNOOPING_DB, dhcsnooping_db_act_process);    

    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, dhcsnooping_process_sys_global);
    cdb_pm_subscribe_tbl(TBL_INTERFACE, dhcsnooping_process_intf_msg_before, dhcsnooping_process_intf_msg_after);
    cdb_pm_subscribe_tbl(TBL_VLAN, dhcsnooping_vlan_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_IPSG_FIB, NULL, dhcsnooping_process_ipsg_msg_after);
    //cdb_pm_subscribe_tbl(TBL_IPSG_FIB, dhcsnooping_ipsour_sync, NULL);
        
    dhcsnooping_start();
    return PM_E_NONE;
}

void dhcsnooping_cowork_release(void *pArg)
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

void dhcsnooping_cowork_handler(void *pArg)
{
    DhcpCoworkArg_t *pCowork = (DhcpCoworkArg_t *)pArg;
    pCowork->cb(pCowork->fd, pCowork->isread, pCowork->pArgDhcp);
    return;
}

int dhcsnooping_cowork_watch(int fd, int isread, void *io, OmapiProcCB *proc, void **ppArg)
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
        pTask = ctc_task_add_read(TASK_PRI_NORMAL, dhcsnooping_cowork_handler, pArg, fd);
    } else {
        pTask = ctc_task_add_read(TASK_PRI_NORMAL, dhcsnooping_cowork_handler, pArg, fd);
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

void dhcsnooping_cowork_init(void)
{
    omapi_io_cowork_register(dhcsnooping_cowork_watch, dhcsnooping_cowork_release);
    return;
}

int32
dhcsnooping_init()
{     
    lib_master_t *lib_master = lib_init(PM_ID_DHCSNOOPING);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
    pm_id_t pmid = M_MOD_DHCPSNOOPING;

    DEBUG_INIT(dhcp_debug, pmid);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(0);

    /* client to HAL */
    fei_init(lib_get_master(), NULL, FALSE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    dhcsnooping_module_init();

    log_sys(pmid, E_INFORMATIONAL, "Ready to service.");
    dhcsnooping_cowork_init();
    ctc_task_main_cowork(omapi_io_cowork_run);
    return PM_E_NONE;
}

static void usage()
{
    sal_printf("Usage: dhcsnooping [-d] [-v] [-h]\n");
    sal_printf("    -d: daemon mode\n");
    sal_printf("    -v: show lib version\n");
    sal_printf("    -h: print this\n");
    return;
}

int
main(int argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "dhv";
 
    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1) {   
         int32 opt;
         opt = getopt(argc, argv, shortopts);
         if (EOF == opt) {   
             break;
         }
     
         switch (opt) {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
         default:
             usage();
             exit(1);
         }
    }

    if (daemon_mode) {
        daemon(1, 1);
    }
 
    dhcsnooping_init();
    exit(0);
}


