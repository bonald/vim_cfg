#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ssm_fsm.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"

/*modified by yejl for bug 46513, support the synce on agg member port, 2018-11-30*/
#if 0
static int32
ssm_port_add_to_agg_process(tbl_interface_t *p_db_if)
{
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_key_t ssm_port_key;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;
    
    sal_memset(&ssm_port_key, 0, sizeof(ssm_port_key));
    sal_memcpy(ssm_port_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_ssm_if = tbl_ssm_port_get_ssm_port(&ssm_port_key);
    if (p_db_ssm_if)
    {
        p_db_ssm_if->synce_enable = FALSE;
        p_db_ssm_if->ssm_pdu_direct = SSM_PDU_ALL;
        p_db_ssm_if->priority = SSM_IF_DEFAULT_PRIORITY;
        p_db_ssm_if->replace_clockid_en = FALSE;
        p_db_ssm_if->receive_pdu_enable = FALSE;
        p_db_ssm_if->send_quality = SSM_QL_INV3;
        p_db_ssm_if->receive_quality = SSM_QL_INV3;
        p_db_ssm_if->force_rx_quality = SSM_QL_INV3;
        p_db_ssm_if->force_tx_quality = SSM_QL_INV3;
        p_db_ssm_if->dnu_group_id = SSM_DEFAULT_DNU_GROUP;
        p_db_ssm_if->hop = SSM_DEFAULT_PATH_HOP;
        sal_memset(p_db_ssm_if->receive_clock_id, 0x00, SSM_CLOCK_ID_LEN);
        
        /*Added by yejl for bug 46459, 2018-02-27*/
        CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_timeout_timer);
        CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_send_timer);
        /*end*/

        /*Add to DB*/
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_SYNCE_ENABLE));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_SSM_PDU_DIRECT));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_PRIORITY));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_SEND_QUALITY));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_RECEIVE_QUALITY));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_FORCE_TX_QUALITY));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_FORCE_RX_QUALITY));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_DNU_GROUP_ID));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_HOP));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER));
        PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_db_ssm_if, TBL_SSM_PORT_FLD_SSM_SEND_TIMER));
    }

    /*select the clock source again*/
    PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_if, FALSE));
    ssm_opts_recheck_device();
    if (p_ssm_glb->synce_enable && !p_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_ssm_glb->device_role));
    }
    return PM_E_NONE;
}
#endif

static int32
ssm_interface_sync_after_add(tbl_interface_t *p_db_if)
{
    tbl_ssm_port_t  ssm_port;
    tbl_ssm_port_t  *p_db_ssm_port = NULL;
    int32           rc = PM_E_NONE;
    sal_memset(&ssm_port, 0, sizeof(ssm_port));
    sal_strncpy(ssm_port.key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&(ssm_port.key));
    if (NULL == p_db_ssm_port)
    {
        ssm_port.ifindex = p_db_if->ifindex;
        ssm_port.synce_enable = FALSE;
        ssm_port.ssm_pdu_direct = SSM_PDU_ALL;
        ssm_port.priority = SSM_IF_DEFAULT_PRIORITY;
        ssm_port.replace_clockid_en = FALSE;
        ssm_port.receive_pdu_enable = FALSE;
        ssm_port.send_quality = SSM_QL_INV3;
        ssm_port.receive_quality = SSM_QL_INV3;
        ssm_port.force_rx_quality = SSM_QL_INV3;
        ssm_port.force_tx_quality = SSM_QL_INV3;
        ssm_port.dnu_group_id = SSM_DEFAULT_DNU_GROUP;
        ssm_port.hop = SSM_DEFAULT_PATH_HOP;
        sal_memset(ssm_port.receive_clock_id, 0x00, SSM_CLOCK_ID_LEN);
        ssm_port.ssm_send_timer = NULL;
        ssm_port.ssm_timeout_timer = NULL;
        rc = tbl_ssm_port_add_ssm_port(&ssm_port);
    }
    return rc;
}

static int32
ssm_interface_sync_before_del(tbl_interface_t *p_db_if)
{
    tbl_ssm_port_key_t ssm_key;
    tbl_ssm_port_t     *p_db_ssm_port = NULL;    
    int32              rc = PM_E_NONE;

    sal_memset(&ssm_key, 0, sizeof(ssm_key));
    sal_strncpy(ssm_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&ssm_key);
    if (p_db_ssm_port)
    {
        rc = tbl_ssm_port_del_ssm_port(&ssm_key);
    }

    return rc;
}

static int32
_ssm_port_change_running(tbl_interface_t *p_db_if, uint32 is_running)
{
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_key_t ssm_key;
    tbl_ssm_port_t     *p_db_ssm_port = NULL;    
    int32              rc = PM_E_NONE;

    sal_memset(&ssm_key, 0, sizeof(ssm_key));
    sal_strncpy(ssm_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&ssm_key);
    if (p_db_ssm_port && p_db_ssm_port->synce_enable)
    {
        if (is_running)
        {
            PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, TRUE));
        }
        else
        {
            PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, FALSE));
        }
        ssm_opts_recheck_device();

        if (p_ssm_glb->synce_enable && !p_ssm_glb->ssm_enable)
        {
            PM_E_RETURN(ssm_fsm_select_non_ssm(p_ssm_glb->device_role));
        }
    }
    return PM_E_NONE;
}

static int32 
ssm_interface_sync_field (uint32 field_id, tbl_interface_t *p_db_if)
{
    uint32 is_running = FALSE;
    int32  rc = PM_E_NONE;
    
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_RUNNING:
        is_running = tbl_interface_is_running(p_db_if);
        if (is_running)
        {
            rc = _ssm_port_change_running(p_db_if, TRUE);
        }
        else
        {
            rc = _ssm_port_change_running(p_db_if, FALSE);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}


static int32
ssm_interface_sync_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];   
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        if (!p_ds_node)
        {
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        if (NULL == p_if_key)
        {
            SSM_LOG_ERR("Tbl_interface_key is NULL\n");
            return PM_E_NOT_EXIST;
        }
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            SSM_LOG_ERR("tbl_interface %s is NULL\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
            return PM_E_NOT_EXIST;
        }
        
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            break;
        }
        if (DS_LAG == p_ds_node->id)
        {
            p_lag = (ds_lag_t *)p_ds;
            if (NULL == p_lag)
            {
                SSM_LOG_ERR("Ds_lag is NULL\n");
            }
     //       rc = ssm_port_add_to_agg_process(p_db_if);  /*modified by yejl for bug 46513, support the synce on agg member port, 2018-11-30*/
        }
        break;
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }        
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            return PM_E_NONE;
        }
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            return PM_E_NONE;
        }
        rc = ssm_interface_sync_before_del(p_db_if);
        break;

    case CDB_OPER_SET:
        
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

static int32
ssm_interface_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            /* add sub DS */
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            return PM_E_NONE;
        }
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            return PM_E_NONE;
        }
        rc = ssm_interface_sync_after_add(p_db_if);
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (p_db_if)
        {
            rc = ssm_interface_sync_field(field_id, p_db_if);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}


static int32
ssm_global_sys_startup_done_process(tbl_sys_global_t* p_sys_glb)
{
    tbl_ssm_global_t ssm_glb;
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();
    ssm_clock_id_t clk_id = {0x00, 0x00, 0x00, 0xFF, 0xFE};
    ssm_clock_id_t clk_id_zero;
    int32 rc = PM_E_NONE;

    sal_memset(clk_id_zero, 0x00, sizeof(clk_id_zero));
    if (0 == sal_memcmp(clk_id_zero, p_ssm_glb->device_clock_id, SSM_CLOCK_ID_LEN))
    {
        sal_memset(&ssm_glb, 0, sizeof(ssm_glb));
        sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
        sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
        sal_memcpy(ssm_glb.device_clock_id, clk_id, SSM_CLOCK_ID_LEN);
        
        /*Add to DB*/
        PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID));
    }
  
    return PM_E_NONE;
}

static int32
ssm_global_sync_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t* p_sys_glb = NULL;
   
    if (p_ds_node)
    {
        return PM_E_NONE;
    }
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_sys_glb = (tbl_sys_global_t*)p_tbl;
        if (NULL == p_sys_glb)
        {
            return PM_E_INVALID_PARAM;
        }
        switch (field_id)
        {
            case TBL_SYS_GLOBAL_FLD_STARTUP_DONE:
                PM_E_RETURN(ssm_global_sys_startup_done_process(p_sys_glb));
                break;
        }
        break;

    default:
        return PM_E_NONE;
    }

    return PM_E_NONE;

}

int32
ssm_sync_init()
{
    cdb_pm_subscribe_tbl(TBL_INTERFACE, ssm_interface_sync_before_cb, ssm_interface_sync_after_cb);
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, ssm_global_sync_sys_global);

    return PM_E_NONE;
}

