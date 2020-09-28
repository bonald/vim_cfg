
#include "proto.h"

#include "ptp_datatype.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"
#include "ptp.h"
#include "ptp_api.h"
#include "ptp_error.h"
#include "ptp_constant.h"
#include "ptp_ops.h"
#include "ptp_desc.h"
#include "ptp_adpt.h"
#include "ptp_util.h"


#ifndef ptp_cmd_related

int32
ptp_api_set_hsrv_adjust_offset(int64 offset)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    g_p_master->hsrv_adjust_offset = offset;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET);
    return PTP_E_API_NONE;
}

int32
ptp_api_set_hsrv_adjust_ffo(float64 ffo)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    g_p_master->hsrv_adjust_ffo = ffo;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO);
    return PTP_E_API_NONE;
}

int32
ptp_api_set_hsrv_update_system_time(int16 utc_offset)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    g_p_master->hsrv_update_system_time = utc_offset;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME);
    return PTP_E_API_NONE;
}

int32
ptp_api_set_hsrv_device_type(uint8 hsrv_device_type)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    g_p_master->hsrv_device_type = hsrv_device_type;
    tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE);
    return PTP_E_API_NONE;
}

/****************************************************************************
 * Name    : ptp_api_set_two_step
 * Purpose : set clock steps
 * Input   : step
 * Output  : N/A
 * Return  : PTP_E_API_xxxxxx - the ui error code of this operation
 * Note    : N/A
 ****************************************************************************/
int32
ptp_api_set_default_ds_two_step_flag(uint32 two_step_flag)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;
    
    g_p_master->default_ds.two_step_flag = two_step_flag;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);
    
    rc = ptp_ops_switch_device_type(g_p_master->device_type);
    
    return rc;
}

/****************************************************************************
 * Name    : ptp_api_get_two_step
 * Purpose : get clock steps
 * Input   : N/A
 * Output  : p_step
 * Return  : PTP_E_API_xxxxxx - the ui error code of this operation
 * Note    : N/A
 ****************************************************************************/
int32
ptp_api_get_default_ds_two_step_flag(uint32* p_two_step)//no called
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_two_step);

    *p_two_step = g_p_master->default_ds.two_step_flag;
    return PTP_E_API_NONE;
}

int32
ptp_api_set_time_prop_intern_current_utc_offset(int16 current_utc_offset)
{
    tbl_ptp_global_t* p_ptp_glb = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    p_ptp_glb->time_prop_intern.current_utc_offset = current_utc_offset;
    rc = tbl_ptp_global_set_ptp_global_field(p_ptp_glb, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);

    return rc;
}

int32
ptp_api_get_time_prop_intern_current_utc_offset(int16* current_utc_offset)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(current_utc_offset);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *current_utc_offset = g_p_master->time_prop_intern.current_utc_offset;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_slave_only(uint8 slave_only)
{
    tbl_ptp_global_t* p_ptp_glb = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    p_ptp_glb->default_ds.slave_only = slave_only;
    p_ptp_glb->default_ds.clock_quality.clk_class = slave_only ? PTP_CLOCK_CLASS_SLAVE : PTP_CLOCK_CLASS_DEF;
    rc = tbl_ptp_global_set_ptp_global_field(p_ptp_glb, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    if (p_ptp_glb->global_enable)
    {
        ptp_ops_set_global_enable(FALSE);
        ptp_ops_set_global_enable(TRUE);
    }
    return rc;
}

int32
ptp_api_get_default_ds_slave_only(uint8* slave_only)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(slave_only);
    if (!ptp_ops_is_oc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *slave_only = g_p_master->default_ds.slave_only;
    
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_domain_number(uint8 domain_number)
{
    tbl_ptp_global_t* p_ptp_glb = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    p_ptp_glb->default_ds.domain_number = domain_number;
    rc = tbl_ptp_global_set_ptp_global_field(p_ptp_glb, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    return rc;
}

int32
ptp_api_get_default_ds_domain_number(uint8* p_domain)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_domain);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_domain = g_p_master->default_ds.domain_number;
    return PTP_E_API_NONE;
}

int32 
ptp_api_current_slave_port_not_null(char* ifname)
{
    char ifname_null[IFNAME_SIZE];
    sal_memset(ifname_null, 0, sizeof(ifname_null));

    if (sal_memcmp(ifname, ifname_null, sizeof(ifname_null)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int32
ptp_api_set_system_time(uint32 set_system_time)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;
    tbl_ptp_port_t *p_db_ptp_port = NULL;

    g_p_master->set_system_time = set_system_time;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME);

    if (ptp_api_current_slave_port_not_null(g_p_master->current_slave_port))
    {
        tbl_ptp_port_key_t port_key;
        sal_memset(&port_key, 0, sizeof(port_key));
        sal_memcpy(port_key.name, g_p_master->current_slave_port, IFNAME_SIZE);
        p_db_ptp_port = tbl_ptp_port_get_ptp_port(&port_key);
        if (p_db_ptp_port->link_up && PTP_STATE_SLAVE == p_db_ptp_port->port_ds.port_state)
        {
            ptp_adpt_update_system_time(g_p_master->time_prop_ds.current_utc_offset);
        }
    }
    
    return rc;
}

int32
ptp_api_get_system_time(uint32* set_system_time)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(set_system_time);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *set_system_time = g_p_master->set_system_time;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_time_prop_intern_leap61(uint8 leap61)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->time_prop_intern.leap61 = leap61;
    if (leap61)
    {
        g_p_master->time_prop_intern.leap59 = FALSE;
    }
    
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);

    return rc;
}

int32
ptp_api_get_time_prop_intern_leap61(uint8* p_leap61)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_leap61);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_leap61 = g_p_master->time_prop_intern.leap61;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_time_prop_intern_leap59(uint8 leap59)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->time_prop_intern.leap59 = leap59;
    if (leap59)
    {
        g_p_master->time_prop_intern.leap61 = FALSE;
    }
    
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);

    return rc;
}

int32
ptp_api_get_time_prop_intern_leap59(uint8* p_leap59)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_leap59);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_leap59 = g_p_master->time_prop_intern.leap59;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_primary_domain(uint8 primary_domain)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->default_ds.primary_domain = primary_domain;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    return rc;
}

int32
ptp_api_get_default_ds_primary_domain(uint8* primary_domain)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(primary_domain);
    if (!ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *primary_domain = g_p_master->default_ds.primary_domain;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_priority1(uint8 priority1)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->default_ds.priority1 = priority1;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    return rc;
}

int32
ptp_api_get_default_ds_priority1(uint8* p_priority1)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_priority1);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_priority1 = g_p_master->default_ds.priority1;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_priority2(uint8 priority2)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->default_ds.priority2 = priority2;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    return rc;
}

int32
ptp_api_get_default_ds_priority2(uint8* p_priority2)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_priority2);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_priority2 = g_p_master->default_ds.priority2;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_time_prop_intern_time_source(uint8 time_source)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->time_prop_intern.time_source = time_source;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);

    return rc;
}

int32
ptp_api_get_time_prop_intern_time_source(uint8* p_time_source)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_time_source);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_time_source = g_p_master->time_prop_intern.time_source;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_clk_accuracy(uint8 clk_accuracy)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    g_p_master->default_ds.clock_quality.clk_accuracy = clk_accuracy;
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);

    return rc;
}

int32
ptp_api_get_default_ds_clk_accuracy(uint8* p_clock_accuracy)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_clock_accuracy);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_clock_accuracy = g_p_master->default_ds.clock_quality.clk_accuracy;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_default_ds_clk_class(uint8 clk_class)
{
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PM_E_NONE;

    switch(clk_class)
    {
    case 6:
    case 7:
    case 52:
    case 187:
        g_p_master->default_ds.clock_quality.clk_class = clk_class;
        g_p_master->time_prop_intern.ptp_timescale = TRUE;
        break;
    case 13:
    case 14:
    case 58:
    case 193:
        g_p_master->default_ds.clock_quality.clk_class = clk_class;
        g_p_master->time_prop_intern.ptp_timescale = FALSE;
        break;
    case 248:
        g_p_master->default_ds.clock_quality.clk_class = clk_class;
        g_p_master->time_prop_intern.ptp_timescale = TRUE;
        break;
    default:
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);
    rc += tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN);

    return rc;
}

int32
ptp_api_get_default_ds_clk_class(uint8* p_clock_class)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_clock_class);
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_clock_class = g_p_master->default_ds.clock_quality.clk_class;
    return PTP_E_API_NONE;
}

int32
ptp_api_set_device_type(ptp_device_type_t type)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    switch (type)
    {
    case PTP_DEVICE_OC:
        type = PTP_DEVICE_OC;
        break;
    case PTP_DEVICE_BC:
        type = PTP_DEVICE_BC;
        break;
    case PTP_DEVICE_E2ETC:
        type = PTP_DEVICE_E2ETC;
        break;
    case PTP_DEVICE_P2PTC:
        type = PTP_DEVICE_P2PTC;
        break;
    default:
        return PTP_E_API_INVALID_CMD_INPUT;
    }

    if (type != g_p_master->device_type)
    {
        /*protocol running error should not report as configure error*/
        /*so there is no need to check the return value*/
        ptp_ops_switch_device_type(type);

        return PTP_E_API_CLEAR_CONFIGURATION;
    }

    return PTP_E_API_NONE;
}

int32
ptp_api_get_device_type(ptp_device_type_t* p_type)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_type);
    *p_type = g_p_master->device_type;
    return PTP_E_API_NONE;
}

sal_time_t
ptp_time_boottime_msec(sal_time_t *tp)
{
    struct sal_timeval tv;

    ctc_task_boottime(&tv);
    if (tp)
    {
        *tp = (sal_time_t)(tv.tv_usec / 1000);
    }

    return (sal_time_t)(tv.tv_usec / 1000);
}

int32
ptp_api_set_clock_id(glb_ptp_clock_id_t clock_id)
{
    int32 rc = PM_E_NONE;
    tbl_ptp_global_t* g_p_master = tbl_ptp_global_get_ptp_global();
    
    sal_memcpy(g_p_master->default_ds.clock_identity, clock_id, GLB_PTP_CLOCK_ID_LEN);

    PM_E_RETURN(tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS));

    if (g_p_master->global_enable)
    {
        ptp_ops_set_global_enable(FALSE);
        ptp_ops_set_global_enable(TRUE);
    }

    return PTP_E_SUCCESS;
}

int32
ptp_api_set_global_enable(uint32 enable)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;
    glb_ptp_clock_id_t clk_id_zero;
#ifdef DUET2
    int32 second = 0;
    int32 nanoseconds = 0;
    uint64 msec = 0;
#endif

    if (FALSE == p_sys_glb->startup_done)
    {
        if (TRUE == enable)
        {
            sal_memset(clk_id_zero, 0x00, sizeof(clk_id_zero));
            if (0 == sal_memcmp(clk_id_zero, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN))
            {
                glb_ptp_clock_id_t clk_id = { 0x00, 0x00, 0x00, 0xFF, 0xFE };
                sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
                sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
                ptp_util_clockid_cpy(g_p_master->default_ds.clock_identity, clk_id);
                tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_DEFAULT_DS);
            }
        }
    }
    
    ptp_ops_set_global_enable(enable);
    
    //g_p_master->global_enable = enable;
    //rc = tbl_ptp_global_set_ptp_global_field(g_p_master, TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE);

    if (enable)
    {
        //ptp_nsm_set_flex_key_by_mac();
        //ptp_nsm_set_flex_global_action();

        #ifdef DUET2
        //add by weij for bug 45114, use uninterrupt mode
        ptp_adpt_get_hardware_second(&second, &nanoseconds);
        msec = ptp_time_boottime_msec(NULL);
        g_p_master->glb_enable_hw_msec_time = ((uint64)second*1000) + ((uint64)nanoseconds/1000000) - g_p_master->total_offset/1000000;
        g_p_master->glb_enable_pf_msec_time = msec;
        #endif
    }
    else
    {
        //ptp_nsm_unset_flex_key_by_mac();
    }

    return rc;
}

uint64
ptp_api_get_global_enable_time()
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    uint64 msec = 0;
    uint64 boot_time_msec = 0;

    msec = ptp_time_boottime_msec(NULL);
    boot_time_msec = g_p_master->glb_enable_hw_msec_time + (msec - g_p_master->glb_enable_pf_msec_time);
    return boot_time_msec;
}

/****************************************************************************
 * Name    : ptp_api_get_global_enable
 * Purpose : get weather the clock is enabled
 * Input   : N/A
 * Output  : p_enable
 * Return  : PTP_E_API_xxxxxx - the ui error code of this operation
 * Note    : N/A
 ****************************************************************************/
int32
ptp_api_get_global_enable(uint32* p_enable)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_API_PTR_CHECK(p_enable);
    *p_enable = g_p_master->global_enable;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_port_port_ds_log_announce_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_announce_interval)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.log_announce_interval = log_announce_interval;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}

int32
ptp_api_get_port_port_ds_log_announce_interval(tbl_ptp_port_t *p_port, int8* log_announce_interval)
{
    PTP_API_PTR_CHECK(log_announce_interval);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *log_announce_interval = p_port->port_ds.log_announce_interval;
    return PTP_E_API_NONE;
}



int32
ptp_api_set_port_port_ds_log_sync_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_sync_interval)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.log_sync_interval = log_sync_interval;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}

int32
ptp_api_get_port_port_ds_log_sync_interval(tbl_ptp_port_t *p_port, int8* log_sync_interval)
{
    PTP_API_PTR_CHECK(log_sync_interval);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *log_sync_interval = p_port->port_ds.log_sync_interval;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_port_port_ds_log_min_delay_req_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_min_delay_req_interval)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.log_min_delay_req_interval = log_min_delay_req_interval;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}

int32
ptp_api_get_port_port_ds_log_min_delay_req_interval(tbl_ptp_port_t *p_port, int8* log_min_delay_req_interval)
{
    PTP_API_PTR_CHECK(log_min_delay_req_interval);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (PTP_DELAY_MECHANISM_E2E != p_port->port_ds.delay_mechanism)
    {
        return PTP_E_API_INVALID_DELAY_MECHANISM;
    }

    *log_min_delay_req_interval = p_port->port_ds.log_min_delay_req_interval;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_port_port_ds_log_min_pdelay_req_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_min_pdelay_req_interval)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.log_min_pdelay_req_interval = log_min_pdelay_req_interval;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}


int32
ptp_api_get_port_port_ds_log_min_pdelay_req_interval(tbl_ptp_port_t *p_port, int8* log_min_pdelay_req_interval)
{
    PTP_API_PTR_CHECK(log_min_pdelay_req_interval);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (PTP_DELAY_MECHANISM_P2P != p_port->port_ds.delay_mechanism)
    {
        return PTP_E_API_INVALID_DELAY_MECHANISM;
    }

    *log_min_pdelay_req_interval = p_port->port_ds.log_min_pdelay_req_interval;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_port_port_ds_announce_receipt_timeout(tbl_ptp_port_t *p_db_ptp_port, uint8 announce_receipt_timeout)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.announce_receipt_timeout = announce_receipt_timeout;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}

int32
ptp_api_get_port_port_ds_announce_receipt_timeout(tbl_ptp_port_t *p_port, uint8* announce_receipt_timeout)
{
    PTP_API_PTR_CHECK(announce_receipt_timeout);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *announce_receipt_timeout = p_port->port_ds.announce_receipt_timeout;
    return PTP_E_API_NONE;
}


int32
ptp_api_set_port_asymmetry_correction(tbl_ptp_port_t *p_db_ptp_port, int64 asymmetry_correction)
{
    int32 rc = PM_E_NONE;
    
    PTP_API_INVALID_INTERFACE_RET(p_db_ptp_port);
    
    p_db_ptp_port->asymmetry_correction.v = asymmetry_correction;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);

    //sync tbl field
    //ptp_ops_set_intf_asymmetry_correction(p_db_ptp_port->ifindex, asymmetry_correction);

    p_db_ptp_port->asymmetry_correction.v = p_db_ptp_port->asymmetry_correction.v << PTP_SCALED_NS_BITS;

    //rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION);

    return rc;
}

int32
ptp_api_get_port_asymmetry_correction(tbl_ptp_port_t *p_port, int32* p_asymmetry_correction)
{
    PTP_API_PTR_CHECK(p_asymmetry_correction);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    *p_asymmetry_correction = p_port->asymmetry_correction.v >> PTP_SCALED_NS_BITS;
    return PTP_E_API_NONE;
}

int32
ptp_api_set_port_hsrv_path_delay(tbl_ptp_port_t *p_db_ptp_port, int64 path_delay)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->hsrv_path_delay = path_delay;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_HSRV_PATH_DELAY);

    return rc;
}

int32
ptp_api_set_port_ingress_latency(tbl_ptp_port_t *p_db_ptp_port, int32 ingress_latency)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->ingress_latency = ingress_latency;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_INGRESS_LATENCY);
    //sync tbl field
    //ptp_adpt_set_intf_ingress_latency_correction(p_db_ptp_port->ifindex, ingress_latency);

    return rc;
}

int32
ptp_api_set_port_egress_latency(tbl_ptp_port_t *p_db_ptp_port, int32 egress_latency)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->egress_latency = egress_latency;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_EGRESS_LATENCY);
    //sync tbl field
    //ptp_adpt_set_intf_egress_latency_correction(p_db_ptp_port->ifindex, egress_latency);

    return rc;
}

int32
ptp_api_get_port_latency_correction(tbl_ptp_port_t *p_port, int32* ingress_latency, int32* egress_latency)
{
    PTP_API_PTR_CHECK(ingress_latency);
    PTP_API_PTR_CHECK(egress_latency);
    PTP_API_INVALID_INTERFACE_RET(p_port);

   *ingress_latency = p_port->ingress_latency;
   *egress_latency = p_port->egress_latency;
   
    return PTP_E_API_NONE;
}

int32
ptp_api_set_port_port_ds_delay_mechanism(tbl_ptp_port_t *p_db_ptp_port, uint8 delay_mechanism)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->port_ds.log_min_delay_req_interval = PTP_MIN_DELAYREQ_INTERVAL_DEF;
    p_db_ptp_port->port_ds.log_min_pdelay_req_interval = PTP_MIN_PDELAYREQ_INTERVAL_DEF;
    p_db_ptp_port->port_ds.delay_mechanism= delay_mechanism;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_DS);

    return rc;
}

int32
ptp_api_get_port_port_ds_delay_mechanism(tbl_ptp_port_t *p_port, ptp_delay_mechanism_t* p_mechanism)
{
    PTP_API_PTR_CHECK(p_mechanism);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    *p_mechanism = p_port->port_ds.delay_mechanism;
    return PTP_E_API_NONE;
}

int32
ptp_api_set_port_vlan_info(tbl_ptp_port_t *p_db_ptp_port, uint32 vlan_id, uint8 cos)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->vlan_info.vid = vlan_id;
    p_db_ptp_port->vlan_info.cos = cos;
    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_VLAN_INFO);

    return rc;
}

int32
ptp_api_get_port_vlan_info(tbl_ptp_port_t *p_port, ptp_vlan_info_t* p_info)
{
    PTP_API_PTR_CHECK(p_info);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    p_info->vid = p_port->vlan_info.vid;
    p_info->cos = p_port->vlan_info.cos;
    return PTP_E_API_NONE;
}

int32
ptp_api_set_port_enable(tbl_ptp_port_t *p_db_ptp_port, uint32 enable)
{
    int32 rc = PM_E_NONE;

    p_db_ptp_port->enable = enable;
    //rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_ENABLE);

    /*protocol running error should not report as configure error*/
    /*so there is no need to check the return value*/
    ptp_ops_set_port_enable(p_db_ptp_port, enable);

    /*
    if (enable)
    {
        ptp_nsm_set_flex_port_action(ifp->ifindex, GLB_PDU_L2PDU_PORT_ACTION_FWD);
    }
    else
    {
        ptp_nsm_set_flex_port_action(ifp->ifindex, GLB_PDU_L2PDU_PORT_ACTION_DISCARD);
    }
  */

    return rc;
}

int32
ptp_api_get_port_enable(tbl_ptp_port_t *p_port , uint32* enable)
{
    PTP_API_PTR_CHECK(enable);
    PTP_API_INVALID_INTERFACE_RET(p_port);

    *enable = p_port->enable;
    
    return PTP_E_API_NONE;
}
#endif

#ifndef ptp_sync_from_other_tbl
int32
ptp_api_sync_add_port(tbl_interface_t *p_db_if)
{
    tbl_ptp_port_t ptp_port;
    int32 rc = PM_E_NONE;

    sal_memset(&ptp_port, 0, sizeof(ptp_port));

    sal_memcpy(&ptp_port.key, &p_db_if->key, sizeof(tbl_ptp_port_key_t));
    ptp_port.ifindex = p_db_if->ifindex;

    /*init ptp port*/
    if (PTP_E_SUCCESS != ptp_ops_intf_init_port(&ptp_port))
    {
        return PM_E_FAIL;
    }
    rc = tbl_ptp_port_add_ptp_port(&ptp_port);
    
    return rc;
    
}

int32
ptp_api_sync_del_port(tbl_ptp_port_t *p_db_ptp_port)
{
    int32 rc = PM_E_NONE;

    if (PTP_E_SUCCESS != ptp_ops_intf_fini_port(p_db_ptp_port))
    {
        return PM_E_FAIL;
    }
    
    rc = tbl_ptp_port_del_ptp_port(&p_db_ptp_port->key);
    
    return rc;
}

int32
ptp_api_sync_set_port_up_running(tbl_interface_t *p_db_if, tbl_interface_t *p_if_new)
{
    uint32 old_running = FALSE;
    uint32 new_running = FALSE;
    uint32 old_up = FALSE;
    uint32 new_up = FALSE;
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    
    old_running = tbl_interface_is_running(p_db_if);
    new_running = tbl_interface_is_running(p_if_new);
    
    old_up = tbl_interface_is_up(p_db_if);
    new_up = tbl_interface_is_up(p_if_new);

    p_db_ptp_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&(p_db_if->key)));
    if (NULL == p_db_ptp_port)
    {
        return PTP_E_API_INVALID_INTERFACE;
    }

    /*any change happen*/
    if (old_running != new_running || old_up != new_up)
    {
        if (TRUE == new_running && TRUE == new_up)
        {
            /*port up*/
            p_db_ptp_port->link_up = TRUE;
        }
        else
        {
            /*port down*/
            p_db_ptp_port->link_up = FALSE;
        }
        
        PTP_OP_ERROR_RETURN(ptp_ops_link_up_switch(p_db_ptp_port));
    }

    return PM_E_NONE;
}
#endif

