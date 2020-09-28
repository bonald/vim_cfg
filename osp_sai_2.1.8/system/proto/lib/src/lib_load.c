
#include "proto.h"
#include "lib_load.h"
#include "gen/tbl_sys_load_define.h"
#include "gen/tbl_sys_load.h"

ctc_task_t *g_p_lib_load_timer = NULL;

CDS_SYNC_LOCAL_TBL_SET_CB g_cds_sync_set_func = NULL;

static void
_lib_load_timer_func(void *p_data)
{
    tbl_sys_load_t *p_db_load = tbl_sys_load_get_sys_load();
    lib_master_t *p_master = (lib_master_t*)p_data;
    int32 queue_depth = 0;

    queue_depth = ctc_sock_get_txqueue_depth(p_master->data_sock);
    if (queue_depth < 0)
    {
        queue_depth = 0;
    }
    
    switch (p_master->pm_id)
    {
    case PM_ID_SWITCH:
        p_db_load->cdb_queue_depth_switch = queue_depth;
        tbl_sys_load_set_sys_load_field(p_db_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH);
        break;

    case PM_ID_ROUTED:
        p_db_load->cdb_queue_depth_routed = queue_depth;
        tbl_sys_load_set_sys_load_field(p_db_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED);
        break;

    case PM_ID_FEA:
        p_db_load->cdb_queue_depth_fea = queue_depth;
        tbl_sys_load_set_sys_load_field(p_db_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA);
        break;

    case PM_ID_CDS:
        p_db_load->cdb_queue_depth_cds = queue_depth;
        tbl_sys_load_set_sys_load_field(p_db_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS);
        if (g_cds_sync_set_func)
        {
            g_cds_sync_set_func(TBL_SYS_LOAD, TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS, p_db_load);
        }
        break;

    default:
        break;
    }
    
    g_p_lib_load_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lib_load_timer_func, p_master, SYS_LOAD_UPDATE_INTERVAL);

    return;
}

int32
lib_load_set_cds_sync_set_func(CDS_SYNC_LOCAL_TBL_SET_CB fn)
{
    g_cds_sync_set_func = fn;

    return PM_E_NONE;
}

int32
lib_load_init()
{
    lib_master_t *p_master = lib_get_master();

    if (PM_ID_CDS == p_master->pm_id
     || PM_ID_SWITCH == p_master->pm_id
     || PM_ID_ROUTED == p_master->pm_id
     || PM_ID_FEA == p_master->pm_id
     )
    {
        CTC_TASK_STOP_TIMER(g_p_lib_load_timer);
        g_p_lib_load_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lib_load_timer_func, p_master, SYS_LOAD_UPDATE_INTERVAL);
    }

    return PM_E_NONE;
}

int32
lib_load_fini()
{
    lib_master_t *p_master = lib_get_master();
    
    if (PM_ID_CDS == p_master->pm_id
     || PM_ID_SWITCH == p_master->pm_id
     || PM_ID_ROUTED == p_master->pm_id
     || PM_ID_FEA == p_master->pm_id)
    {
        CTC_TASK_STOP_TIMER(g_p_lib_load_timer);
    }
    
    return PM_E_NONE;
}

