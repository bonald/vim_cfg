
#include "proto.h"
#include "ptp_datatype.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "ptp.h"
#include "ptp_api.h"
#include "ptp_util.h"
#include "ptp_ops.h"
#include "ptp_error.h"

static int32
_ptp_interface_sync_before_del(tbl_interface_t *p_db_if)
{
    tbl_ptp_port_key_t key;
    tbl_ptp_port_t  *p_db_ptp_port = NULL;    
    int32 rc = PTP_E_SUCCESS;

    sal_memset(&key, 0, sizeof(key));
    sal_memcpy(&key, &p_db_if->key, sizeof(key));
    p_db_ptp_port = tbl_ptp_port_get_ptp_port(&key);
    if (p_db_ptp_port)
    {
        rc = ptp_api_sync_del_port(p_db_ptp_port);
    }

    return rc;
}

int32
_ptp_port_set_agg_config_to_new_member_ptp_port(tbl_ptp_port_t *p_db_ptp_port, tbl_ptp_port_t *p_db_ptp_port_agg)
{
    int32 rc = PTP_E_SUCCESS;
    if (p_db_ptp_port_agg->enable)
    {
        p_db_ptp_port->enable = p_db_ptp_port_agg->enable;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ENABLE);
    }

    if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port_agg->ingress_latency)
    {
        p_db_ptp_port->ingress_latency = p_db_ptp_port_agg->ingress_latency;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_INGRESS_LATENCY);
    }

    if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port_agg->egress_latency)
    {
        p_db_ptp_port->egress_latency = p_db_ptp_port_agg->egress_latency;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_EGRESS_LATENCY);
    }

    if (PTP_ASYMMETRY_CORRECTION_DEF != p_db_ptp_port_agg->asymmetry_correction.v)
    {
        p_db_ptp_port->asymmetry_correction.v = p_db_ptp_port_agg->asymmetry_correction.v;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);
    }

    return rc;
}

int32
_ptp_port_reset_config(tbl_ptp_port_t *p_db_ptp_port)
{
    ptp_ops_intf_fini_port(p_db_ptp_port);

    ptp_ops_intf_init_port_config(p_db_ptp_port);
    
    return PTP_E_SUCCESS;
}

int32 _ptp_port_add_to_agg(tbl_interface_key_t *p_if_key, uint8 lag_id)
{
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    tbl_ptp_port_t *p_db_ptp_port_agg = NULL;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_agg = NULL;
    int32 rc = PTP_E_SUCCESS;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    p_db_ptp_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)p_if_key);
    if (NULL == p_db_ptp_port)
    {
        PTP_LOG_ERR("Ptp port %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", lag_id);

    p_db_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if_agg)
    {
        PTP_LOG_ERR("Interface %s does not exist\n", IFNAME_ETH2FULL(if_key.name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    p_db_ptp_port_agg = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)&if_key);
    if (NULL == p_db_ptp_port_agg)
    {
        PTP_LOG_ERR("Ptp port %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    p_db_ptp_port->lag_id = lag_id;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_LAG_ID);
    
    rc = _ptp_port_reset_config(p_db_ptp_port);

    rc = _ptp_port_set_agg_config_to_new_member_ptp_port(p_db_ptp_port, p_db_ptp_port_agg);
    
    return rc;
}

int32
_ptp_port_reset_ptp_port_config_for_leave_agg(tbl_ptp_port_t *p_db_ptp_port, tbl_ptp_port_t *p_db_ptp_port_agg)
{
    if (p_db_ptp_port_agg->enable)
    {
        p_db_ptp_port->enable = FALSE;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ENABLE);
    }

    if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port_agg->ingress_latency)
    {
        p_db_ptp_port->ingress_latency = PTP_LATENCY_CORRECTION_DEF;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_INGRESS_LATENCY);
    }

    if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port_agg->egress_latency)
    {
        p_db_ptp_port->egress_latency = PTP_LATENCY_CORRECTION_DEF;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_EGRESS_LATENCY);
    }

    if (PTP_ASYMMETRY_CORRECTION_DEF != p_db_ptp_port_agg->asymmetry_correction.v)
    {
        p_db_ptp_port->asymmetry_correction.v = PTP_ASYMMETRY_CORRECTION_DEF;
        tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);
    }

    return PTP_E_SUCCESS;
}

uint32 _ptp_port_rm_from_agg(tbl_interface_key_t *p_if_key, uint8 lag_id)
{
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    tbl_ptp_port_t *p_db_ptp_port_agg = NULL;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_agg = NULL;
    int32 rc = PTP_E_SUCCESS;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    p_db_ptp_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)p_if_key);
    if (NULL == p_db_ptp_port)
    {
        PTP_LOG_ERR("Ptp port %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", lag_id);

    p_db_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if_agg)
    {
        PTP_LOG_ERR("Interface %s does not exist\n", IFNAME_ETH2FULL(if_key.name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    p_db_ptp_port_agg = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)&if_key);
    if (NULL == p_db_ptp_port_agg)
    {
        PTP_LOG_ERR("Ptp port %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
        return PTP_E_NOT_EXIST;
    }

    p_db_ptp_port->lag_id = 0;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_LAG_ID);

    rc = _ptp_port_reset_ptp_port_config_for_leave_agg(p_db_ptp_port, p_db_ptp_port_agg);
    
    return rc;
}

static int32
_ptp_interface_sync_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t *p_if = NULL;
    int32 rc = PTP_E_SUCCESS;
    ds_lag_t *p_lag = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
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
            PTP_LOG_ERR("p_tbl is NULL\n");
            return PTP_E_ERROR;
        }

        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            PTP_LOG_ERR("Interface %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
            return PTP_E_NOT_EXIST;
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
                PTP_DBG_SYNC("p_ds is NULL\n");
            }
            
            /* LAG member port add to LAG group, need to remove all PTP config and disable PTP and set LAG member flag */
            if ((NULL != p_if_key) && (NULL != p_lag))
            {
                PTP_DBG_SYNC("Add interface %s to agg %d\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext), p_lag->lag_id);
                rc = _ptp_port_add_to_agg(p_if_key, p_lag->lag_id);
            }
        }
        break;
        
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            p_if_key = (tbl_interface_key_t*)p_tbl;
            if (NULL == p_if_key)
            {
                PTP_LOG_ERR("p_tbl is NULL\n");
                return PTP_E_ERROR;
            }

            p_db_if = tbl_interface_get_interface(p_if_key);
            if (NULL == p_db_if)
            {
                PTP_LOG_ERR("Interface %s does not exist\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext));
                return PTP_E_NOT_EXIST;
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
                    PTP_DBG_SYNC("p_ds is NULL\n");
                }
                    
                /* LAG member port remove from LAG group, need to unset LAG member flag */
                if ((NULL != p_if_key) && (NULL != p_lag))
                {
                    PTP_DBG_SYNC("Remove interface %s from agg %d\n", IFNAME_ETH2FULL(p_if_key->name, ifname_ext), p_lag->lag_id);
                    _ptp_port_rm_from_agg(p_if_key, p_lag->lag_id);
                }
            }
        }
        else
        {
            p_if_key = (tbl_interface_key_t*)p_tbl;
            p_db_if = tbl_interface_get_interface(p_if_key);
            if (NULL == p_db_if)
            {
                return PTP_E_SUCCESS;
            }
            if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
            {
                return PTP_E_SUCCESS;
            }
            rc = _ptp_interface_sync_before_del(p_db_if);
        }
        break;

    case CDB_OPER_SET:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = (tbl_interface_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            return PTP_E_NOT_EXIST;
        }

        if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF || p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_RUNNING:
            case TBL_INTERFACE_FLD_UP:
                rc = ptp_api_sync_set_port_up_running(p_db_if, p_if);
                break;

            default:
                break;
            }
        }
        break;

    default:
        return PTP_E_INVALID_PARAM;
    }
    return rc;
}


static int32
_ptp_interface_sync_after_add(tbl_interface_t *p_db_if)
{
    tbl_ptp_port_key_t key;
    tbl_ptp_port_t  *p_db_ptp_port = NULL;
    int32           rc = PTP_E_SUCCESS;

    sal_memset(&key, 0, sizeof(key));
    sal_memcpy(&key, &p_db_if->key, sizeof(key));
    p_db_ptp_port = tbl_ptp_port_get_ptp_port(&key);
    if (NULL == p_db_ptp_port)
    {
        rc = ptp_api_sync_add_port(p_db_if);
    }

    return rc;
}


static int32
_ptp_interface_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PTP_E_SUCCESS;

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
            return PTP_E_NOT_EXIST;
        }
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
        {
            return PTP_E_SUCCESS;
        }

        rc = _ptp_interface_sync_after_add(p_db_if);
        break;

    case CDB_OPER_SET:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            return PTP_E_NOT_EXIST;
        }

        if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF || p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_RUNNING:
            case TBL_INTERFACE_FLD_UP:
                //rc = ptp_api_sync_set_port_up_running(p_db_if, p_if);
                break;

            default:
                break;
            }
        }
        break;

    default:
        return PTP_E_INVALID_PARAM;
    }
    
    return rc;
}

static int32
_ptp_global_proc_startup_done()
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    glb_ptp_clock_id_t clk_id = { 0x00, 0x00, 0x00, 0xFF, 0xFE };
    glb_ptp_clock_id_t clk_id_zero;

    if((NULL == g_p_master) || (NULL == p_sys_glb))
    {
        return PTP_E_SUCCESS;
    }

    sal_memset(clk_id_zero, 0x00, sizeof(clk_id_zero));
    if (0 == sal_memcmp(clk_id_zero, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN))
    {
        sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
        sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
        ptp_util_clockid_cpy(g_p_master->default_ds.clock_identity, clk_id);

        tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);
    }
    
    return PTP_E_SUCCESS;
}

static int32
_ptp_global_sync_sys_global_cb(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PTP_E_SUCCESS;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id)
        {
            _ptp_global_proc_startup_done();
        }
        break;

    default:
        break;
    }
    
    return rc;

}

int32
ptp_sync_init()
{
    cdb_pm_subscribe_tbl(TBL_INTERFACE, _ptp_interface_sync_before_cb, _ptp_interface_sync_after_cb);
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, _ptp_global_sync_sys_global_cb, NULL);

    return PTP_E_SUCCESS;
}

