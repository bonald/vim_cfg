
#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "switch.h"
#include "errdisable.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"


#ifdef CONFIG_ERRDISABLE

ctc_task_t *g_p_flap_timer = NULL;
FLAP_TIMER_FUNC g_flap_time_func[ERRDIS_FLAP_REASON_MAX];

int32
errdisable_create_reason(errdis_rsn_t reason)
{
    tbl_errdisable_t errdisable;
    tbl_errdisable_t *p_errdisable = NULL;
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_errdisable_key_t errdisable_key;
    int rc = PM_E_NONE;

    /* 1. check exist */
    errdisable_key.reason = reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL != p_db_errdisable)
    {
        return PM_E_NONE;
    }

    /* 2. create new session */
    p_errdisable = &errdisable;
    sal_memset(p_errdisable, 0, sizeof(tbl_errdisable_t));
    p_errdisable->key.reason = reason;
    rc = tbl_errdisable_add_errdisable(p_errdisable);

    return rc;
}

int32
errdisable_destroy_reason(errdis_rsn_t reason)
{
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_errdisable_key_t errdisable_key;

    /* 1. check exist */
    errdisable_key.reason = reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }

    /* 2. remove errdisable reason */
    errdisable_key.reason = reason;
    return tbl_errdisable_del_errdisable(&errdisable_key);
}


int32
errdisable_flap_create_flap_reason(errdis_flap_rsn_t flap_rsn)
{
    tbl_errdisable_flap_t errdis_flap;
    tbl_errdisable_flap_t *p_errdis_flap = NULL;
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_flap_key_t errdis_flap_key;
    int rc = PM_E_NONE;

    /* 1. check exist */
    errdis_flap_key.flap_rsn = flap_rsn;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
    if (NULL != p_db_errdis_flap)
    {
        return PM_E_NONE;
    }

    /* 2. create new flap reason */
    p_errdis_flap = &errdis_flap;
    sal_memset(p_errdis_flap, 0, sizeof(tbl_errdisable_t));
    p_errdis_flap->key.flap_rsn = flap_rsn;
    rc = tbl_errdisable_flap_add_errdisable_flap(p_errdis_flap);

    return rc;
}

int32
errdisable_flap_destroy_flap_reason(errdis_flap_rsn_t flap_rsn)
{ 
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_flap_key_t errdis_flap_key;

    /* 1. check exist */
    errdis_flap_key.flap_rsn = flap_rsn;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
    if (NULL == p_db_errdis_flap)
    {
        return PM_E_NONE;
    }

    /* 2. remove flap reason */
    errdis_flap_key.flap_rsn = flap_rsn;
    return tbl_errdisable_flap_del_errdisable_flap(&errdis_flap_key);
}


int32
errdisable_if_is_errdisable (tbl_interface_t *p_if)
{
    if (!p_if)
        return FALSE;
    
    if (p_if->errdisable_reason)
    {
        return TRUE;
    }
    return FALSE;
}

int32 
errdisable_is_detect_en(errdis_rsn_t reason)
{
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_errdisable_key_t errdisable_key;

    if (ERRDIS_RSN_MAX <= reason)
        return FALSE;  

    errdisable_key.reason = reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }

    return ((p_db_errdisable->errdisable_en)?(TRUE):(FALSE));
}

int32 
errdisable_is_recovery_en(errdis_rsn_t reason)
{
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_errdisable_key_t errdisable_key;

    if (ERRDIS_RSN_MAX <= reason)
    {
        return FALSE;  
    }
    errdisable_key.reason = reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }

    return ((p_db_errdisable->recovery_en)?(TRUE):(FALSE));
}

int32
errdisable_set_errdisable_interval(uint32 interval)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.errdisable_interval = interval;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL);

    return PM_E_NONE;
}

int32
errdisable_set_if_status(tbl_interface_t *p_if, errdis_rsn_t reason, uint32 err_dis_enable)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int ret = 0;
    int rc = PM_E_NONE;

    if (!p_if)
    {
        return PM_E_FAIL;
    }

    if (err_dis_enable)
    {
        PM_E_RETURN(if_mgr_set_up(p_if, FALSE, TRUE, FALSE));
        GLB_SET_FLAG(p_if->errdisable_reason, 1<<reason);
        tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
    }
    else
    {
        if (ERRDIS_MONITOR_LINK == reason)
        {
            GLB_UNSET_FLAG(p_if->errdisable_reason, 1 << ERRDIS_MONITOR_LINK);
        }
        else if (ERRDIS_UDLD_DISABLE == reason)
        {
            GLB_UNSET_FLAG(p_if->errdisable_reason, 1 << ERRDIS_UDLD_DISABLE);
        }
        else
        {
            p_if->errdisable_reason = ERRDIS_NO_ERRDIS;
        }
        tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
        
        if (!p_if->errdisable_reason)
        {
            PM_E_RETURN(if_mgr_set_up(p_if, TRUE, TRUE, FALSE));
        }
    }

    if (INTERFACE_IS_LAG_GROUP(p_if))
    {
        p_lag = p_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            if(err_dis_enable)
            {
                GLB_SET_FLAG(p_if_mem->errdisable_reason, 1<<reason);
            }
            else
            {
                p_if_mem->errdisable_reason = ERRDIS_NO_ERRDIS;
            }
            tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
        }
    }

    return ret;
}

void
errdisable_timer_handler (void* arg)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if =  (tbl_interface_t *)(arg);

    if (p_db_if == NULL)
    {
        return;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    p_db_if->errdisable_timer = NULL;
    logid_sys(LOG_INTERFACE_4_ERRDISABLE_EXIT, ifname_ext, "timeout");
    errdisable_set_if_status(p_db_if, ERRDIS_NO_ERRDIS, FALSE);

    return;
}

int32
errdisable_handle_exception(errdis_rsn_t reason, tbl_interface_t *p_db_if, uint32 enable)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_sys_global_t *sys_glb = NULL;

    if (!p_db_if)
    {
        return PM_E_FAIL;
    }
    
    if (ERRDIS_RSN_MAX <= reason)
    {
        return PM_E_INVALID_PARAM;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    if (enable)
    {
        /* Added by yejl for bug40757 , 2016-09-13 */
        if (errdisable_if_is_errdisable(p_db_if))
        {
            return PM_E_NONE;
        }

        logid_sys(LOG_INTERFACE_4_ERRDISABLE_ENTER, ifname_ext, cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, reason));
        errdisable_set_if_status(p_db_if, reason, TRUE);
        if (errdisable_is_recovery_en(reason))
        {
            if (CTC_TASK_IS_RUNNING(p_db_if->errdisable_timer))
            {
                CTC_TASK_STOP_TIMER (p_db_if->errdisable_timer);
            }
            
            sys_glb = tbl_sys_global_get_sys_global();
            if(!sys_glb)
            {
                return PM_E_FAIL;
            }

            p_db_if->errdisable_timer = 
                CTC_TASK_ADD_TIME(errdisable_timer_handler, p_db_if, sys_glb->errdisable_interval);
        }
        
    }
    else 
    {
        logid_sys(LOG_INTERFACE_4_ERRDISABLE_EXIT, ifname_ext, "timeout");
        errdisable_set_if_status(p_db_if, reason, FALSE);

        if (CTC_TASK_IS_RUNNING(p_db_if->errdisable_timer) && (!p_db_if->errdisable_reason))
        {
            CTC_TASK_STOP_TIMER (p_db_if->errdisable_timer);
        }
    }

    return PM_E_NONE;
}

int32 
errdisable_lacp_inherit_parent(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem)
{
    if (errdisable_if_is_errdisable(p_db_if_lag))
    {
        if (CTC_TASK_IS_RUNNING(p_db_if_lag->errdisable_timer))
        {
            if (p_db_if_mem->errdisable_timer)
                CTC_TASK_STOP_TIMER (p_db_if_mem->errdisable_timer);
            
            p_db_if_mem->errdisable_timer = 
                CTC_TASK_ADD_TIME(errdisable_timer_handler, p_db_if_mem, ctc_timer_get_remain_sec(p_db_if_lag->errdisable_timer));
        }
        else
        {
            /* do nothing */
        }
    }
    
    return PM_E_NONE;
}

static void
_errdisable_flap_timer_cb()
{
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_flap_key_t errdis_flap_key;
    uint32 rsn = 0;
    
    g_p_flap_timer = CTC_TASK_ADD_TIME(_errdisable_flap_timer_cb, NULL, GLB_ERRDISABLE_TIMER_INTERVAL);
    for (rsn = 0; rsn < ERRDIS_FLAP_REASON_MAX; rsn++)
    {
        errdis_flap_key.flap_rsn = rsn;
        p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
        if (p_db_errdis_flap == NULL)
        {
            return;
        }
        p_db_errdis_flap->flap_time_cnt++;
        if (p_db_errdis_flap->flap_time_cnt >= p_db_errdis_flap->flap_time_threshold)
        {
            p_db_errdis_flap->flap_time_cnt = 0;
            g_flap_time_func[rsn]();
        }
    }
    return;
}

static int32
_errdisable_if_link_flap_timer_cb()
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t* p_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_if, listnode)
    {
        if (p_if->hw_type != GLB_IF_TYPE_PORT_IF && p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_if))
        {
            continue;
        }
        p_if->link_flap_cnt = 0;
    }
    return PM_E_NONE;
}

int32
errdisable_init_reason()
{
    errdis_rsn_t reason;
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_errdisable_key_t errdisable_key;

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        errdisable_key.reason = reason;
        p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
        if (!p_db_errdisable)
        {
            return PM_E_NONE;
        }
        if (ERRDIS_FDB_LOOP != reason)
        {
            errdisable_set_detect_en(p_db_errdisable, TRUE);
        }
    }

    return PM_E_NONE;
}

int32
errdisable_flap_init()
{
    errdis_flap_rsn_t flap_rsn = ERRDIS_FLAP_REASON_LINK;
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_flap_key_t errdis_flap_key;
    tbl_errdisable_t *p_db_errdis = NULL;
    tbl_errdisable_key_t errdis_key;

    errdis_flap_key.flap_rsn = flap_rsn;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
    if (!p_db_errdis_flap)
    {
        return PM_E_NOT_EXIST;
    }
    
    p_db_errdis_flap->errdisable_rsn = ERRDIS_LINK_FLAP;
    errdis_key.reason = p_db_errdis_flap->errdisable_rsn;
    p_db_errdis = tbl_errdisable_get_errdisable(&errdis_key);
    if (NULL == p_db_errdis)
    {
        return PM_E_NOT_EXIST;
    }
    
    errdisable_set_flap(p_db_errdis_flap, GLB_LINK_FLAP_MAX_FLAP_COUNT, GLB_LINK_FLAP_MAX_FLAP_SECOND);
    
    g_flap_time_func[flap_rsn] = _errdisable_if_link_flap_timer_cb;
    g_p_flap_timer = CTC_TASK_ADD_TIME(_errdisable_flap_timer_cb, NULL, GLB_ERRDISABLE_TIMER_INTERVAL);    
    
    return PM_E_NONE;
}

int32
errdisable_fdb_loop_init()
{
    tbl_errdisable_key_t errdisable_key;
    tbl_errdisable_t *p_db_errdisable = NULL;
    errdis_rsn_t reason = ERRDIS_FDB_LOOP;
    
    errdisable_key.reason= reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }
    errdisable_set_fdb_loop(p_db_errdisable, GLB_FDB_LOOP_BUCKET_SIZE_DEF, GLB_FDB_LOOP_BUCKET_RATE_DEF);
    
    return PM_E_NONE;
}

int32
errdisable_start(switch_master_t *master)
{
    errdis_rsn_t reason;

    for(reason = 0; reason < ERRDIS_RSN_MAX; reason ++)
    {
        errdisable_create_reason(reason);
    }

    errdis_flap_rsn_t flap_rsn;

    for(flap_rsn = 0; flap_rsn < ERRDIS_FLAP_REASON_MAX; flap_rsn ++)
    {
        errdisable_flap_create_flap_reason(flap_rsn);
    }
    
    errdisable_init_reason();
    
    errdisable_set_errdisable_interval(ERRDISABLE_DEFAULT_INTERVAL);
    errdisable_flap_init();/*errdisable link_flap init*/
    errdisable_fdb_loop_init();/*errdisable fdb_loop init*/
    
    cdb_register_cfg_tbl_cb(TBL_ERRDISABLE, errdisable_cmd_process);
    cdb_register_cfg_act_cb(ACT_ERRDISABLE_SHOW, errdisable_cmd_process_show);

    return PM_E_NONE;
}

int32
errdisable_stop(switch_master_t *master)
{
    errdis_rsn_t reason;

    for(reason = 0; reason < ERRDIS_RSN_MAX; reason ++)
    {
        errdisable_destroy_reason(reason);
    }
    errdis_flap_rsn_t flap_rsn;

    for(flap_rsn = 0; flap_rsn < ERRDIS_FLAP_REASON_MAX; flap_rsn ++)
    {
        errdisable_flap_destroy_flap_reason(flap_rsn);
    }

    cdb_register_cfg_tbl_cb(TBL_ERRDISABLE, NULL);
    
    return PM_E_NONE;
}

int32 
errdisable_set_fdb_loop_detect_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable)
{
    int32 rc = PM_E_NONE;   
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    
    p_brg_glb->fdb_loop_errdis_en = is_enable;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN));
    return rc;
}

int32
errdisable_set_detect_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable)
{
    int32 rc = PM_E_NONE;
  
    p_db_errdisable->errdisable_en = is_enable;
    if (ERRDIS_FDB_LOOP == p_db_errdisable->key.reason)
    {
        errdisable_set_fdb_loop_detect_en(p_db_errdisable, is_enable);
    }
    PM_E_RETURN(tbl_errdisable_set_errdisable_field(p_db_errdisable, TBL_ERRDISABLE_FLD_ERRDISABLE_EN));
    return rc;
}

int32
errdisable_set_recovery_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable)
{
    int32 rc = PM_E_NONE;
    p_db_errdisable->recovery_en = is_enable;

    PM_E_RETURN(tbl_errdisable_set_errdisable_field(p_db_errdisable, TBL_ERRDISABLE_FLD_RECOVERY_EN));
    return rc;
}

int32
errdisable_set_flap(tbl_errdisable_flap_t *p_db_errdis_flap, uint32 count, uint32 second)
{
    int32 rc = PM_E_NONE;
    p_db_errdis_flap->flap_cnt_threshold = count;
    PM_E_RETURN(tbl_errdisable_flap_set_errdisable_flap_field(p_db_errdis_flap, TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD));
    
    p_db_errdis_flap->flap_time_threshold = second;
    PM_E_RETURN(tbl_errdisable_flap_set_errdisable_flap_field(p_db_errdis_flap, TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD));
    
    p_db_errdis_flap->flap_time_cnt = 0;
    PM_E_RETURN(tbl_errdisable_flap_set_errdisable_flap_field(p_db_errdis_flap, TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT));
    
    return rc;
}

int32
errdisable_set_fdb_loop(tbl_errdisable_t *p_db_errdisable, uint32 maxsize, uint32 rate)
{
    int32 rc = PM_E_NONE;
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    
    p_brg_glb->fdb_loop_max_size = maxsize;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE));
    
    p_brg_glb->fdb_loop_add_rate = rate;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE));
    
    return rc;
}
#endif /* CONFIG_ERRDISABLE */

