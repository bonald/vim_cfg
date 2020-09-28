#include "proto.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "switch.h"
#include "brg.h"
#include "errdisable.h"
#include "lib_image.h"

int32
brg_alloc_id(cdb_id_type_t type)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    int32 rc = PM_E_NONE;

    switch (type)
    {
    case CDB_ID_INSTANCE:
        rc = lib_bmp_alloc_id(p_brg_glb->instance, p_brg_glb->max_instance);
        if (rc >= 0)
        {
            tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_INSTANCE);
        }
        break;

    case CDB_ID_PVLAN_ISOLATE_ID:
        rc = lib_bmp_alloc_id(p_brg_glb->pvlan_isolate_id, p_brg_glb->max_pvlan_isolate_id);
        if (rc >= 0)
        {
            tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    
    return rc;
}

int32
brg_release_id(cdb_id_type_t type, int32 id)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    int32 rc = PM_E_NONE;

    switch (type)
    {
    case CDB_ID_INSTANCE:
        rc = lib_bmp_release_id(p_brg_glb->instance, id, p_brg_glb->max_instance);
        if (0 == rc)
        {
            tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_INSTANCE);
        }
        break;

    case CDB_ID_PVLAN_ISOLATE_ID:
        rc = lib_bmp_release_id(p_brg_glb->pvlan_isolate_id, id, p_brg_glb->max_pvlan_isolate_id);
        if (0 == rc)
        {
            tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    
    return rc;
}

int32
brg_alloc_pvlan_group_id(tbl_pvlan_t *p_db_pvlan, tbl_interface_t *p_if, vid_t community_vlan)
{
    ds_pvlan_community_t *p_ds_pvlan = NULL;
    ds_pvlan_community_t p_community;
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if =NULL;
 
    sal_memset(&p_community, 0, sizeof(ds_pvlan_community_t));
    p_community.key.community_vid= community_vlan;
    p_db_if = tbl_interface_get_interface(&p_if->key);
    
    if (p_db_if == NULL)
    {
        return PM_E_FAIL;
    }
    
    p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);

    if (p_ds_pvlan != NULL)
    {
        PM_E_RETURN(ds_pvlan_community_set_ports(p_db_pvlan, p_ds_pvlan, TRUE, p_db_if));
        return p_ds_pvlan->group_id;
    }
    else
    {   
        rc = lib_bmp_alloc_id(p_db_pvlan->community_group_id, GLB_ISOLATE_GRP_MAX);
        if (rc >= 0)
        {
            p_community.group_id = rc;
            ds_pvlan_community_add_pvlan_community(p_db_pvlan, &p_community);
            p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);
            p_ds_pvlan->group_id = rc;
            ds_pvlan_community_set_pvlan_community_field(p_db_pvlan, p_ds_pvlan, DS_PVLAN_COMMUNITY_FLD_GROUP_ID);  
            tbl_pvlan_set_pvlan_field(p_db_pvlan, TBL_PVLAN_FLD_COMMUNITY_GROUP_ID);
            ds_pvlan_community_set_ports(p_db_pvlan, p_ds_pvlan, TRUE, p_db_if);
        }
    }
    return rc;
}

int32
brg_release_pvlan_group_id(tbl_pvlan_t *p_db_pvlan, tbl_interface_t *p_db_if, vid_t community_vlan)
{
    ds_pvlan_community_t *p_ds_pvlan = NULL;
    ds_pvlan_community_t p_community;
    int32 rc = PM_E_NONE;
 
    sal_memset(&p_community, 0, sizeof(ds_pvlan_community_t));
    p_community.key.community_vid = community_vlan;
    
    p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);
    
    if (p_ds_pvlan == NULL)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(ds_pvlan_community_set_ports(p_db_pvlan, p_ds_pvlan, FALSE, p_db_if)); 
    p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);
    if(p_ds_pvlan->ports.obj_list->count == 0)
    {    
        rc = lib_bmp_release_id(p_db_pvlan->community_group_id, p_ds_pvlan->group_id, GLB_ISOLATE_GRP_MAX);
        
        if (0 == rc)
        {
            PM_E_RETURN(ds_pvlan_community_del_pvlan_community(p_db_pvlan, p_ds_pvlan));
            PM_E_RETURN(tbl_pvlan_set_pvlan_field(p_db_pvlan, TBL_PVLAN_FLD_COMMUNITY_GROUP_ID));
        }
    }
    return rc;
}    

int32
brg_set_brg_max_id(cdb_id_type_t type, int32 max)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();

    switch (type)
    {
    case CDB_ID_INSTANCE:
        p_brg_glb->max_instance = max;
        tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_MAX_INSTANCE);
        break;

    case CDB_ID_PVLAN_ISOLATE_ID:
        p_brg_glb->max_pvlan_isolate_id = max;
        tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

int32
brg_global_set_max_static_fdb(uint32 max_static_fdb)
{
    tbl_brg_global_t brg_glb;
    uint32 rc = PM_E_NONE;
    
    /* 1. process */

    /* 2. update local CDB */
    brg_glb.max_static_fdb = max_static_fdb;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB));

    return PM_E_NONE;
}

int32
brg_global_set_max_l2mcast_fdb(uint32 max_l2mcast_fdb)
{
    tbl_brg_global_t brg_glb;
    uint32 rc = PM_E_NONE;
    
    /* 1. process */

    /* 2. update local CDB */
    brg_glb.max_l2mcast_fdb = max_l2mcast_fdb;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB));

    return PM_E_NONE;
}

int32
brg_global_set_oper_aging_time(uint32 aging_time)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    uint32 rc = PM_E_NONE;

    if (BRG_INVALID_AGEING_TIME == aging_time)
    {
        p_brg_glb->oper_fdb_aging_time = p_brg_glb->fdb_aging_time;
    }
    else
    {
        p_brg_glb->oper_fdb_aging_time = aging_time;
    }

    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_glb, TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME));
    
    return PM_E_NONE;
}

int32
brg_global_set_aging_time(uint32 aging_time)
{
    tbl_brg_global_t brg_glb;
    uint32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.fdb_aging_time = aging_time;
    brg_glb.oper_fdb_aging_time = aging_time;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME));
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME));

    return PM_E_NONE;
}

int32
brg_global_set_hw_learning_en(uint32 enable)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.hw_learning_enable = enable;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE));

    return PM_E_NONE;
}

int32
brg_global_set_ipsg_max_port_binding(uint32 max_num)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.ipsg_max_port_binding = max_num;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING));

    return PM_E_NONE;
}

int32
brg_global_init_ipsg_max_port_binding()
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.ipsg_max_port_binding = IPSOUR_DEFAULT_NUMBER;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING));

    return PM_E_NONE;
}

int32
brg_global_init_vlan_tpid_type()
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.vlan_tpid_type1 = GLB_DEFAULT_TPID;
    brg_glb.vlan_tpid_type2 = 0x9100;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1));
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2));

    return PM_E_NONE;
}

int32
brg_global_set_port_isolate_mode(uint32 mode)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.port_isolate_mode = mode;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE));

    return PM_E_NONE;
}

int
brg_global_set_trap_delay_linkup_time(uint32 linkup_time)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.trap_delay_up = linkup_time;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP));
    
    return rc;
}

int
brg_global_set_trap_delay_linkdown_time(uint32 linkdown_time)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.trap_delay_down = linkdown_time;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN));
    
    return rc;
}

int32
brg_global_set_l2protocol_cos(uint32 l2protocol_cos)
{
    tbl_brg_global_t brg_glb;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    brg_glb.l2protocol_cos = l2protocol_cos;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(&brg_glb, TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS));
    
    return rc;
}

int32
brg_get_warmboot_info()
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    uint32 warmboot_type = 0;
    uint32 warmboot_time = 0;
    uint32 warmboot_count = 0;
    int32 rc = PM_E_NONE;
    
    rc = lib_parse_warmboot_file(&warmboot_type, &warmboot_time, &warmboot_count);
    if (rc < 0)
    {
        return PM_E_NONE;
    }
    p_sys_glb->warmboot_type = warmboot_type;
    p_sys_glb->warmboot_time = warmboot_time;
    p_sys_glb->warmboot_count = warmboot_count;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE);
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME);
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT);
    
    return PM_E_NONE;
}

int32
brg_fea_init()
{
    tbl_sys_global_t sys_glb;
    int32 rc = PM_E_NONE;
    
    sys_glb.max_frame_size = GLB_DFT_MAX_FRAME_SIZE_0;
    sys_glb.jumbo_frame_size = GLB_DFT_MAX_FRAME_SIZE_1;

    sys_glb.http_port = GLB_HTTP_PORT_DEF;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HTTP_PORT));
    sys_glb.rpcapi_port = GLB_HTTP_PORT_DEF;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_PORT));
    sys_glb.https_port = GLB_HTTPS_PORT_DEF;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HTTPS_PORT));        
    sys_glb.https_disable = TRUE;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE));
    sys_glb.rpcapi_ssl_port = GLB_HTTPS_PORT_DEF;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT));        
    sys_glb.rpcapi_disable = TRUE;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE));

    PM_E_RETURN(brg_global_set_max_static_fdb(BRG_MAX_STATIC_FDB_NUMBER));
    PM_E_RETURN(brg_global_set_max_l2mcast_fdb(BRG_MAX_L2MCAST_FDB_NUMBER));
    PM_E_RETURN(brg_global_set_aging_time(GLB_FDB_DEFAULT_AGING_TIME));
    PM_E_RETURN(brg_global_init_ipsg_max_port_binding());

    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE));
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE));

    sys_glb.mib_port_stats_read_num = GLB_MIB_PORT_STATS_READ_NUM_PER_ROUND;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM));
    sys_glb.mib_flow_stats_read_num = GLB_MIB_FLOW_STATS_READ_NUM_PER_ROUND;
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM));
    
    PM_E_RETURN(brg_set_brg_max_id(CDB_ID_PVLAN_ISOLATE_ID, GLB_ISOLATE_GRP_MAX));
    errdisable_set_errdisable_interval(ERRDISABLE_DEFAULT_INTERVAL);
    PM_E_RETURN(brg_get_warmboot_info());

    return rc;
}

#define MGMT_IF_NAME                "eth0"
#define MGMT_IF_POLLING_INTERVAL    3

static void
_brg_mgmt_if_timer_process(tbl_sys_global_t* p_sys_global)
{
#ifndef _GLB_UML_SYSTEM_
    uint32 mgmt_if_running = FALSE;
    int32 rc = 0;

    rc = sal_system(NETNS_CMD_PREFIX" ifconfig "MGMT_IF_NAME" | grep RUNNING >/dev/null 2>&1");
    mgmt_if_running = (0 == rc) ? TRUE : FALSE;

    if (p_sys_global->mgmt_if_running != mgmt_if_running)
    {
        logid_sys(LOG_INTERFACE_4_UPDOWN, "mgmt-if "MGMT_IF_NAME, mgmt_if_running ? "up" : "down");
        p_sys_global->mgmt_if_running = mgmt_if_running;
    }
#endif
    return;
}

static void
_brg_mgmt_if_timer_func(void *p_data)
{
    tbl_sys_global_t* p_sys_global = tbl_sys_global_get_sys_global();

    p_sys_global->mgmt_if_timer = NULL;
    
    _brg_mgmt_if_timer_process(p_sys_global);
    
    p_sys_global->mgmt_if_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _brg_mgmt_if_timer_func, NULL, MGMT_IF_POLLING_INTERVAL);

    return;
}

int32
brg_mgmt_if_init()
{
    tbl_sys_global_t* p_sys_global = tbl_sys_global_get_sys_global();

    p_sys_global->mgmt_if_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _brg_mgmt_if_timer_func, NULL, MGMT_IF_POLLING_INTERVAL);

    brg_global_init_vlan_tpid_type();
    
    return PM_E_NONE;
}

int32
brg_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_BRG_GLOBAL, brg_cmd_process_brg_global);
    brg_mgmt_if_init();
    
    return PM_E_NONE;
}

int32
brg_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_BRG_GLOBAL, NULL);
    return PM_E_NONE;
}

