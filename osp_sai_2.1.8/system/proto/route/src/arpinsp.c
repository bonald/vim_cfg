#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "cdb_const.h"
#include "glb_l2_define.h"
#include "glb_arpacl_define.h"
#include "arp_define.h"
#include "arpinsp_api.h"
#include "arpinsp.h"
#include "arpacl.h"

ctc_task_t *arpinsp_logtimer = NULL;

/* log buffer queue for arp inspection */
S_ArpInspLogBuf g_arpinsp_logBuffer;

bool
arpinsp_is_NULL(tbl_arpinsp_t * p_db_arpinsp)
{
    if (p_db_arpinsp->acllog_filter == 0 && p_db_arpinsp->dhcplog_filter == 0 && p_db_arpinsp->enable_state == 0 &&
        p_db_arpinsp->filter_state == 0 && sal_strlen(p_db_arpinsp->filter) == 0)
    {
        return TRUE;
    }
    else 
    {
        return FALSE;
    }
}

int32
arpinsp_set_arpinsp_enable_state(tbl_arpinsp_t *p_arpinsp)
{    
    int32 rc = PM_E_NONE;

    /*Add to DB*/
    PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_arpinsp, TBL_ARPINSP_FLD_ENABLE_STATE));
    return rc;
}

int32
arpinsp_set_arpinsp_filter_state(tbl_arpinsp_t *p_arpinsp)
{
    int32 rc = PM_E_NONE;

    /*Add to DB*/
    PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_arpinsp, TBL_ARPINSP_FLD_FILTER));
    PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_arpinsp, TBL_ARPINSP_FLD_FILTER_STATE));
    return rc;
}

int32
arpinsp_set_arpinsp_acl_match(tbl_arpinsp_t *p_arpinsp)
{
    int32 rc = PM_E_NONE;

    /*Add to DB*/
    PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_arpinsp, TBL_ARPINSP_FLD_ACLLOG_FILTER));
    return rc;
}

int32
arpinsp_set_arpinsp_dhcp_binding(tbl_arpinsp_t *p_arpinsp)
{
    int32 rc = PM_E_NONE;

    /*Add to DB*/
    PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_arpinsp, TBL_ARPINSP_FLD_DHCPLOG_FILTER));
    return rc;
}

int32
arpinsp_set_logbuf_entrynum(uint32 arpinsp_log_entry_num)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global(); 
    tbl_route_global_t rt_glb;
    S_ArpInspLogItem *pItem = NULL;

    /* if the logs in buffer are more than the size we set, we should delete it */
    while (p_rt_glb->arpinsp_logbuf_curnum > arpinsp_log_entry_num)
    {
        /* delete the oldest one */
        pItem = TAILQ_FIRST(&g_arpinsp_logBuffer);
        TAILQ_REMOVE(&g_arpinsp_logBuffer, pItem, entry);
        
        /* free it */
        arpinsp_free_logitem(pItem);
        pItem = NULL;
        p_rt_glb->arpinsp_logbuf_curnum--;
    }
    
    /* update local CDB */
    sal_memset(&rt_glb, 0, sizeof(tbl_route_global_t));
    rt_glb.arpinsp_logbuf_entrynum = arpinsp_log_entry_num;
    PM_E_RETURN(tbl_route_global_set_route_global_field(&rt_glb, TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM));
    return rc;
    
}

int32
arpinsp_set_logbuf_number_and_second(uint32 arpinsp_log_num, uint32 arpinsp_log_second)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t rt_glb;
    sal_memset(&rt_glb, 0, sizeof(tbl_route_global_t));

    /* update local CDB */
    rt_glb.arpinsp_logbuf_lognum = arpinsp_log_num;
    rt_glb.arpinsp_logbuf_logsec = arpinsp_log_second;
    PM_E_RETURN(tbl_route_global_set_route_global_field(&rt_glb, TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM));
    PM_E_RETURN(tbl_route_global_set_route_global_field(&rt_glb, TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC));

    if (arpinsp_log_num != 0)
    {
        if (CTC_TASK_IS_RUNNING(arpinsp_logtimer))
        {
            CTC_TASK_STOP_TIMER(arpinsp_logtimer);
        }
        arpinsp_logtimer = CTC_TASK_ADD_TIME(arpinsp_write_log_buffer_timer_cb, NULL, arpinsp_log_second);
    }
    return rc;
}   

int32
arpinsp_set_arpinsp_validate(uint32 arpinsp_validate_flag)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t rt_glb;
    sal_memset(&rt_glb, 0, sizeof(tbl_route_global_t));
    
    /* update local CDB */
    rt_glb.arpinsp_validate_flag = arpinsp_validate_flag;
    PM_E_RETURN(tbl_route_global_set_route_global_field(&rt_glb, TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG));
    return rc;
}

int32
arpinsp_log_time_init()
{
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global(); 
    if (0 == p_rt_glb->arpinsp_logbuf_logsec)
    {
        return PM_E_NONE;
    }
    else
    {
        if (CTC_TASK_IS_RUNNING(arpinsp_logtimer))
        {
            CTC_TASK_STOP_TIMER(arpinsp_logtimer);
        }
        arpinsp_logtimer = CTC_TASK_ADD_TIME(arpinsp_write_log_buffer_timer_cb, NULL, p_rt_glb->arpinsp_logbuf_logsec);
    }   
    return PM_E_NONE;
}

int32
arpinsp_start(route_master_t *master)
{
    /*init log bufer entry number and log number&seconds*/
    arpinsp_set_logbuf_entrynum(GLB_ARPINSP_LOGBUF_DEFAULT_NUM);
    arpinsp_set_logbuf_number_and_second(GLB_ARPINSP_LOGLIST_DEFAULT_NUM, GLB_ARPINSP_LOGLIST_DEFAULT_INTERVAL);
    
    /* Initialize log list and log buffer */
    TAILQ_INIT(&g_arpinsp_logBuffer);

    /*process arpinsp and show arpinsp*/
    cdb_register_cfg_tbl_cb(TBL_ARPINSP, arpinsp_cmd_process_arpinsp);
    cdb_register_cfg_act_cb(ACT_SHOW_ARPINSP, arpinsp_cmd_process_show);
    cdb_register_cfg_act_cb(ACT_CLEAR_ARPINSP, arpinsp_cmd_process_clear);

    arpinsp_log_time_init();
    arpratelimit_init();
    
    return PM_E_NONE;
}

int32
arpinsp_stop(route_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_ARPINSP, NULL);
    return PM_E_NONE;
}

