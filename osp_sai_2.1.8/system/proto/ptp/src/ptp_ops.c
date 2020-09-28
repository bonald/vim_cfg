
#include "proto.h"
#include "glb_ptp_define.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_constant.h"
#include "ptp_error.h"
#include "ptp_adpt.h"
#include "ptp_proto.h"
#include "ptp_util.h"
#include "ptp_bmc.h"
#include "ptp_msg_forward.h"
#include "ptp_msg_handle.h"
#include "ptp_msg_issue.h"
#include "ptp_ops.h"
#include "ptp_desc.h"

typedef int32
(*PTP_LISTLOOP_CB_FUNC)(tbl_ptp_port_t* p_port);

#define PTP_LOG_INTERN_INTERVAL_BMC -1
#define PTP_LOG_INTERN_INTERVAL_AGE 10

uint32
ptp_ops_is_tc(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    switch (g_p_master->device_type)
    {
    case PTP_DEVICE_E2ETC:
    case PTP_DEVICE_P2PTC:
        return TRUE;

    default:
        return FALSE;
    }
}

uint32
ptp_ops_is_oc(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    return (PTP_DEVICE_OC == g_p_master->device_type);
}

uint32
ptp_ops_ifp_invalid(tbl_ptp_port_t * p_port)
{
    tbl_interface_t *p_db_if = NULL;
    
    if (NULL == p_port)
    {
        return TRUE;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_port->ifindex);
    if (NULL == p_db_if)
    {
        return TRUE;
    }

    switch (p_db_if->hw_type)
    {
    case GLB_IF_TYPE_PORT_IF:
    case GLB_IF_TYPE_LINKAGG_IF:
        return FALSE;
    default:
        return TRUE;
    }
}

int32
ptp_ops_global_init(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    #define PTP_DEVICE_MAX 0xFF
    g_p_master->device_type = PTP_DEVICE_MAX;
    g_p_master->default_ds.two_step_flag = PTP_TWO_STEP_DEF;
    //g_p_master->sync_interface_output = PTP_SYNC_INTF_DEF;//not support
    PTP_OP_ERROR_RETURN(ptp_ops_switch_device_type(PTP_DEVICE_DEF));

    return PTP_E_SUCCESS;
}

static void
_ptp_ops_port_list_loop(PTP_LISTLOOP_CB_FUNC function)
{
    tbl_ptp_port_master_t * p_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t * p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (ptp_ops_ifp_invalid(p_db_ptp_port)) 
        {
            continue;
        }
        
        if (p_db_ptp_port)
        {
            PTP_OP_ERROR_CONTINUE((*function)(p_db_ptp_port));
        }
    }

    return;
}

static void
_ptp_ops_device_init_default_ds(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    g_p_master->default_ds.clock_quality.clk_accuracy = PTP_CLOCK_ACCURACY_DEF;
    g_p_master->default_ds.clock_quality.clk_class = PTP_CLOCK_CLASS_DEF;
    g_p_master->default_ds.clock_quality.offset_scaled_log_variance = PTP_DEFAULT_CLOCK_VARIANCE;

    g_p_master->default_ds.domain_number = PTP_DOMAIN_DEF;
    g_p_master->default_ds.priority1 = PTP_PRIORITY_DEF;
    g_p_master->default_ds.priority2 = PTP_PRIORITY_DEF;
    g_p_master->default_ds.slave_only = FALSE;

    //g_p_master->default_ds.two_step_flag = PTP_TWO_STEP_DEF;

    g_p_master->default_ds.delay_mechanism = PTP_DELAY_MECHANISM_DEF;
    g_p_master->default_ds.primary_domain = PTP_DOMAIN_DEF;

    /*add for when switch device type, clear all config*/
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);
    return;
}

static void
_ptp_ops_device_init_current_ds(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    sal_memset(&g_p_master->current_ds, 0x00, sizeof(ptp_current_t));

    /*add for when switch device type, clear all config*/
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_CURRENT_DS);
    return;
}

static void
_ptp_ops_device_init_time_prop_ds(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    g_p_master->time_prop_intern.current_utc_offset = PTP_UTC_OFFSET_DEF;
    g_p_master->time_prop_intern.current_utc_offset_valid = FALSE;
    g_p_master->time_prop_intern.time_traceable = FALSE;
    g_p_master->time_prop_intern.frequency_traceable = FALSE;
    g_p_master->time_prop_intern.leap59 = FALSE;
    g_p_master->time_prop_intern.leap61 = FALSE;
    g_p_master->time_prop_intern.ptp_timescale = TRUE;
    g_p_master->time_prop_intern.time_source = PTP_TIME_SOURCE_DEF;

    /*add for when switch device type, clear all config*/
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);
    return;
}

static void
_ptp_ops_port_init_ds(tbl_ptp_port_t *p_port)
{
    p_port->port_ds.log_announce_interval = PTP_ANNOUNCE_INTERVAL_DEF;
    p_port->port_ds.log_sync_interval = PTP_SYNC_INTERVAL_DEF;
    p_port->port_ds.log_min_delay_req_interval = PTP_MIN_DELAYREQ_INTERVAL_DEF;
    p_port->port_ds.log_min_pdelay_req_interval = PTP_MIN_PDELAYREQ_INTERVAL_DEF;
    p_port->port_ds.announce_receipt_timeout = PTP_ANNOUNCE_RECEIPT_TIMEOUT_DEF;
    p_port->port_ds.delay_mechanism = PTP_DELAY_MECHANISM_DEF;
    p_port->port_ds.port_state = PTP_STATE_DISABLED;
    p_port->port_ds.version_number = PTP_VERSION_DEF;

    return;
}

void
ptp_ops_init_dynamic_properties(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return;
    }
    
    sal_memset(&g_p_master->current_ds, 0x00, sizeof(ptp_current_t));
    sal_memset(&g_p_master->parent_ds, 0x00, sizeof(ptp_parent_t));
    sal_memset(&g_p_master->time_prop_ds, 0x00, sizeof(ptp_time_properties_t));
    g_p_master->parent_ds.parent_stats = PTP_PARENT_STATS_DEF;
    g_p_master->parent_ds.grandmaster_clock_quality.clk_accuracy = PTP_CLOCK_ACCURACY_DEF;
    g_p_master->parent_ds.grandmaster_clock_quality.clk_class = g_p_master->default_ds.slave_only ? PTP_CLOCK_CLASS_SLAVE : PTP_CLOCK_CLASS_DEF;
    g_p_master->parent_ds.grandmaster_clock_quality.offset_scaled_log_variance = PTP_DEFAULT_CLOCK_VARIANCE;
    g_p_master->time_prop_ds.current_utc_offset = PTP_UTC_OFFSET_DEF;
    g_p_master->time_prop_ds.time_source = PTP_TIME_SOURCE_DEF;

    /*add for when switch device type, clear all config*/
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_CURRENT_DS);
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_PARENT_DS);
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_DS);
    
    return;
}

void
ptp_ops_port_init_prop(tbl_ptp_port_t* p_port)
{
    p_port->port_ds.faulty_flag = 0;
    p_port->port_ds.peer_mean_path_delay = 0;
    sal_memset(&p_port->p, 0x00, sizeof(p_port->p));
    sal_memset(&p_port->c, 0x00, sizeof(p_port->c));
    ptp_bmc_cleanup_foreign_db(p_port);
    p_port->rbest = NULL;

    return;
}

void
ptp_ops_intf_init_port_config(tbl_ptp_port_t* p_port)
{
    _ptp_ops_port_init_ds(p_port);
    ptp_ops_port_init_prop(p_port);
    sal_memset(&p_port->port_addr, 0x00, sizeof(glb_ptp_port_address_t));
    sal_memset(&p_port->msg_proc_ops, 0x00, sizeof(ptp_msg_proc_ops_t));
    p_port->port_addr.protocol = PTP_NET_PROTO_DEF;
    p_port->vlan_info.cos = 0;
    p_port->vlan_info.vid = 0;
      /*modified by wangl for bug 17032,2012-08-07*/
    p_port->asymmetry_correction.v=PTP_ASYMMETRY_CORRECTION_DEF;
    p_port->ingress_latency = PTP_LATENCY_CORRECTION_DEF;
    p_port->egress_latency = PTP_LATENCY_CORRECTION_DEF;
    
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_PORT_ADDR);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_VLAN_INFO);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_INGRESS_LATENCY);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_EGRESS_LATENCY);

    return;
}

int32
ptp_ops_intf_init_port(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    _ptp_ops_port_init_ds(p_port);
    ptp_ops_port_init_prop(p_port);
    sal_memset(&p_port->port_addr, 0x00, sizeof(glb_ptp_port_address_t));
    sal_memset(&p_port->msg_proc_ops, 0x00, sizeof(ptp_msg_proc_ops_t));
    p_port->port_addr.protocol = PTP_NET_PROTO_DEF;
    p_port->vlan_info.cos = 0;
    p_port->vlan_info.vid = 0;
      /*modified by wangl for bug 17032,2012-08-07*/
    p_port->asymmetry_correction.v=PTP_ASYMMETRY_CORRECTION_DEF;
    p_port->ingress_latency = PTP_LATENCY_CORRECTION_DEF;
    p_port->egress_latency = PTP_LATENCY_CORRECTION_DEF;
    
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_PORT_ADDR);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_VLAN_INFO);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_INGRESS_LATENCY);
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_EGRESS_LATENCY);

    if (PTP_DEVICE_E2ETC == g_p_master->device_type)
    {
        p_port->port_ds.port_identity_port_num = p_port->ifindex;
        p_port->port_ds.delay_mechanism = PTP_DELAY_MECHANISM_E2E;
    }
    else if (PTP_DEVICE_P2PTC == g_p_master->device_type)
    {
        p_port->port_ds.port_identity_port_num = p_port->ifindex;
        p_port->port_ds.delay_mechanism = PTP_DELAY_MECHANISM_P2P;
    }
    else
    {
        p_port->port_ds.port_identity_port_num = p_port->ifindex;
    }
    tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_PORT_DS);
    
    if (ptp_ops_is_tc())
    {
        ptp_forward_register(p_port);
        ptp_handle_register(p_port);
        ptp_issue_register(p_port);
        ptp_trigger_register(p_port);
    }

    return PTP_E_SUCCESS;
}

int32
ptp_ops_intf_fini_port(tbl_ptp_port_t* p_port)
{
    if (p_port->enable)
    {
        ptp_util_cancel_timer(&p_port->ptm_announce_receipt_timer);
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->p_timer_faulty_recover_timer);
        
        PTP_OP_ERROR_RETURN(ptp_ops_set_port_enable(p_port, FALSE));
    }

    ptp_residence_time_destroy_db(p_port->ifindex, 2);
    ptp_bmc_destroy_foreign_db(p_port);
    
    return PTP_E_SUCCESS;
}

static void
_ptp_ops_intern_trigger(void *p_data)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return;
    }
    
    g_p_master->p_internal_timer = NULL;

    if (ptp_ops_is_tc())
    {
        ptp_residence_time_age_db();
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
        g_p_master->p_internal_timer = ptp_util_add_timer_log(_ptp_ops_intern_trigger, NULL, PTP_LOG_INTERN_INTERVAL_AGE);
    }
    else
    {
        ptp_bmc_execute();
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
        g_p_master->p_internal_timer = ptp_util_add_timer_log(_ptp_ops_intern_trigger, NULL, PTP_LOG_INTERN_INTERVAL_BMC);
    }

    return ;
}

int32
ptp_ops_switch_device_type(uint32 dev_type)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return PTP_E_ERROR;
    }
    
    if (g_p_master->global_enable)
    {
        PTP_DBG_BMC("----------------Disable bmc exec in switch device type--------------");
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
    }

    _ptp_ops_device_init_default_ds();
    _ptp_ops_device_init_current_ds();
    _ptp_ops_device_init_time_prop_ds();
    ptp_ops_init_dynamic_properties();

    _ptp_ops_port_list_loop(ptp_ops_intf_fini_port);
    g_p_master->device_type = dev_type;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEVICE_TYPE);
    /*modified by wangl for bug 17032,2012-08-07*/
    //g_p_master->frequency_recovery_enable = FALSE;//not support
    g_p_master->set_system_time = FALSE;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME);

    _ptp_ops_port_list_loop(ptp_ops_intf_init_port);
    PTP_OP_ERROR_RETURN(ptp_adpt_switch_device_type(g_p_master->device_type));
#if 0
    if (g_p_master->global_enable)
    {
        ptp_ops_tocpu_vid_by_global_enable(TRUE);
        ptp_adpt_set_ptp_acl_multicast();
    }
#endif
    if (g_p_master->global_enable)
    {
        PTP_DBG_BMC("----------------Enable bmc exec in switch device type--------------");
        uint8 log = ptp_ops_is_tc() ? PTP_LOG_INTERN_INTERVAL_AGE : PTP_LOG_INTERN_INTERVAL_BMC;
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
        g_p_master->p_internal_timer = ptp_util_add_timer_log(_ptp_ops_intern_trigger, NULL, log);
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_ops_global_power_switch(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    //char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return PTP_E_ERROR;
    }
    
    if (p_port->enable && p_port->link_up)
    {
        if (ptp_ops_is_tc())
        {
            ptp_trigger_register(p_port);
        }
        else
        {
            p_port->recommended_state = g_p_master->global_enable ? PTP_STATE_INITIALIZING : PTP_STATE_DISABLED;
            //PTP_DBG_BMC("global_power_switch in %s %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext), 
            //    ptp_desc_state(p_port->recommended_state));
            ptp_proto_to_state(p_port);
        }
    }

    return PTP_E_SUCCESS;
}

int32
ptp_ops_link_up_switch(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    if (p_port->enable && g_p_master->global_enable)
    {
        if (ptp_ops_is_tc())
        {
            ptp_trigger_register(p_port);
        }
        else
        {
            p_port->recommended_state = p_port->link_up ? PTP_STATE_INITIALIZING : PTP_STATE_DISABLED;
            ptp_proto_to_state(p_port);
        }
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_ops_port_power_switch(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    if (g_p_master->global_enable && p_port->link_up)
    {
        if (ptp_ops_is_tc())
        {
            ptp_trigger_register(p_port);
        }
        else
        {
            p_port->recommended_state = p_port->enable ? PTP_STATE_INITIALIZING : PTP_STATE_DISABLED;
            ptp_proto_to_state(p_port);
        }
    }

    return PTP_E_SUCCESS;
}

int32
ptp_ops_set_global_enable(uint32 enable)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return PTP_E_ERROR;
    }

    ptp_util_cancel_timer(&g_p_master->p_internal_timer);
    
    if (enable)
    {
        int8 log = ptp_ops_is_tc() ? PTP_LOG_INTERN_INTERVAL_AGE : PTP_LOG_INTERN_INTERVAL_BMC;
        PTP_DBG_BMC("----------------Enable bmc exec in enable ptp global--------------");
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
        g_p_master->p_internal_timer = ptp_util_add_timer_log(_ptp_ops_intern_trigger, NULL, log);
    }
    else
    {
        PTP_DBG_BMC("----------------Disable bmc exec in disable ptp global--------------");
        ptp_util_cancel_timer(&g_p_master->p_internal_timer);
    }

    ptp_ops_init_dynamic_properties();
   // ptp_adpt_reset_ffo();
    if(enable != g_p_master->global_enable)
    {
        g_p_master->global_enable = enable;
        tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE);
        PTP_OP_ERROR_RETURN(ptp_adpt_switch_device_type(g_p_master->device_type));
    }
    _ptp_ops_port_list_loop(_ptp_ops_global_power_switch);

    return PTP_E_SUCCESS;
}

int32
ptp_ops_set_port_enable(tbl_ptp_port_t* p_port, uint32 enable)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;
     
    p_port->enable = enable;
    _ptp_ops_port_power_switch(p_port);

    if (enable)
    {
        g_p_master->default_ds.number_ports++;
    }
    else
    {
        g_p_master->default_ds.number_ports--;
    }

    rc = tbl_ptp_port_set_ptp_port_field(p_port, TBL_PTP_PORT_FLD_ENABLE);
    
    return rc;
}

uint32
ptp_ops_portid_identical_with_self(tbl_ptp_port_t* p_port, glb_ptp_port_id_t* p_port_id)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    if (ptp_util_clockid_cmp(g_p_master->default_ds.clock_identity, p_port_id->clock_id))
    {
        return FALSE;
    }

    if (p_port->port_ds.port_identity_port_num != p_port_id->port_num)
    {
        return FALSE;
    }

    return TRUE;
}

