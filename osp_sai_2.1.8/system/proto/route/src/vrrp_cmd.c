
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_static_rt_cnt_define.h"
#include "gen/tbl_static_rt_cnt.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_ospf_define.h"
#include "gen/tbl_ospf.h"
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "sal.h"
#include "sal_types.h"
#include "route.h"
#include "lib_netlink.h"
#include "rt_fea.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "vrrp.h"
#include "vrrp_api.h"
#include "track.h"
#include "track_api.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "nd_api.h"

int32
_vrrp_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                     *fp = NULL;
    tbl_iter_args_t          args;     
    tbl_vrrp_global_t        *p_db_vrrp_glb = NULL;
    int32                    field_id = 0;
    field_parse_t            field;

     sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_vrrp_global_dump_one(p_db_vrrp_glb, &args);
            
    sal_fclose(fp);
    return PM_E_NONE;
}

static int32 _vrrp_cmd_set_global_version(tbl_vrrp_global_t *p_glb, 
    tbl_vrrp_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                       rc = PM_E_NONE;
    
    if (p_db_glb->vrrp_session_count)
    {
        CFG_FUNC_ERR_RET("Please delete all vrrp sessions before change vrrp version");
    }

    rc = tbl_vrrp_global_set_vrrp_global_field(p_glb, field_id);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_global_set_dot1x_global_field set dot1x enable fail");
        return rc;
    }
        
    return rc;
}

int32
vrrp_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    tbl_vrrp_global_t         vrrp_glb; 
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;
    int32                     field_id = 0;
    cdb_field_t               *p_field = NULL;
    int32                     value = 0;
    int32                     i = 0;
    
    CFG_DBG_FUNC();

    sal_memset(&vrrp_glb, 0, sizeof(tbl_vrrp_global_t));

    p_node = cdb_get_tbl(TBL_VRRP_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        /* check exist */
        p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
        if (!p_db_vrrp_glb)
        {
            CFG_CONFLICT_RET("Tbl_vrrp_global not found");
        }

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_VRRP_GLOBAL_FLD_VRRP_VERSION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            vrrp_glb.vrrp_version = value;
            rc = _vrrp_cmd_set_global_version(&vrrp_glb, p_db_vrrp_glb, field_id, para);
            break;
        
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _vrrp_cmd_get_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_vrrp_cmd_create_session(uint32 vrid, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_t        sess;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;

    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));

    /* if vrrp session is exist, return */
    sess.key.vrid = vrid;
    p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess.key);
    if (p_db_sess)
    {
        return PM_E_NONE;
    }

    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }

    /* check vrrp session number */
    if(p_db_vrrp_glb->vrrp_session_count >= GLB_VRRP_MAX_SESSION)
    {
        CFG_CONFLICT_RET("Exceeding maximum VRRP session limit");
    }

    /* Set the VRID and initialization values */
    sess.state            = VRRP_STATE_INIT;
    sess.admin_state      = VRRP_ADMIN_STATE_DOWN;
    sess.vip_status       = FALSE;
    sess.owner            = FALSE;
    sess.priority         = GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY;
    sess.config_priority  = GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY;
    sess.advt_interval    = GLB_VRRP_ADVT_INT_DFLT;
    sess.preempt_mode     = TRUE;
    sess.num_ip_addrs     = 1;
    sess.vrrp_uptime      = 0;
    sess.ip_protocol      = VRRP_PROTOCOL_IP;
    sess.master_ip_status = FALSE;
    sal_memset (&sess.master_ip, 0, sizeof(addr_t));
    sal_memset (&sess.primary_ip, 0, sizeof(addr_t));
    sess.master_priority  = 0;
    sess.master_advt_interval   = 0;
    sess.master_down_interval   = 0;
    sess.learn_master     = FALSE; 

    /* Set the Virtual MAC Address */
    sess.vmac[0]          = 0x00;
    sess.vmac[1]          = 0x00;
    sess.vmac[2]          = 0x5e;
    sess.vmac[3]          = 0x00;
    sess.vmac[4]          = 0x01;
    sess.vmac[5]          = vrid;
    sess.time_mode        = GLB_VRRP_SECONDS_MODE;  
    sess.preempt_delay    = GLB_VRRP_PREEMPT_DELAY_DFLT;   

    if(GLB_VRRP_SECONDS_MODE == sess.time_mode)
    {
        if (sess.priority > VRRP_SKEW_PRIORITY)
        {
            sess.skew_time = 0;
        }
        else
        {
            sess.skew_time = 1;
        }
    }
    else
    {
        sess.skew_time = 0;  /* msec mode default skew_time is 0 */
    }
  
    rc = tbl_vrrp_session_add_vrrp_session(&sess);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add vrrp session fail, %s", pm_err_desc(rc));
    }
    
    if (0 == p_db_vrrp_glb->vrrp_session_count)
    {
        /* start vrrp timer */
        vrrp_sys_init_timer(p_db_vrrp_glb, TRUE);
    }

    p_db_vrrp_glb->vrrp_session_count++;
    rc = tbl_vrrp_global_set_vrrp_global_field(p_db_vrrp_glb, TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp global fail, %s", pm_err_desc(rc));
    }

    return rc;
}

static int32
_vrrp_cmd_delete_session(uint32 vrid, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_t        sess;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;
    tbl_vrrp_session_master_t *p_master = NULL;

    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));
 
    sess.key.vrid = vrid;
    p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess.key);
    if (!p_db_sess)
    {
        CFG_CONFLICT_RET("VRID does not exist (cannot delete)");
    }

    if (VRRP_STATE_INIT != p_db_sess->state)
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }

    p_master = tbl_vrrp_session_get_master();  
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_session is not exist");    
    }

    /* decrease msec session number */
    if (GLB_VRRP_MILLI_SECONDS_MODE == p_db_sess->time_mode)
    {
        p_db_vrrp_glb->vrrp_msec_session_count--;
    }

    if (0 != p_db_sess->trackobj_id)
    {
        track_del_tracked_vrrp(p_db_sess->trackobj_id, p_db_sess->key.vrid, para);
    }

    rc = tbl_vrrp_session_del_vrrp_session(&sess.key);
    if (rc < 0)
    {
        VRRP_LOG_ERR("Delete vrrp session fail, %s", pm_err_desc(rc));
    }

    p_db_vrrp_glb->vrrp_session_count--;
    rc = tbl_vrrp_global_set_vrrp_global_field(p_db_vrrp_glb, TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp global fail, %s", pm_err_desc(rc));
    }

    if (0 == p_db_vrrp_glb->vrrp_session_count)
    {
        /* stop vrrp timer */
        vrrp_sys_init_timer(p_db_vrrp_glb, FALSE);
    }
    
    return rc;
}

static int32
_vrrp_cmd_set_session_time_mode(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;    
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;

    
    if (p_db_sess->time_mode == value)
    {
        return rc;    
    }
    
    p_db_sess->time_mode = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_TIME_MODE);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session time mode fail");
        return rc;
    }

    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }

    if (GLB_VRRP_MILLI_SECONDS_MODE == value)
    {
        /* start msec vrrp timer */
        if (0 == p_db_vrrp_glb->vrrp_msec_session_count)
        {           
            vrrp_sys_init_msec_timer(p_db_vrrp_glb, TRUE);
        }

        p_db_vrrp_glb->vrrp_msec_session_count++;
    }
    else
    {
        p_db_vrrp_glb->vrrp_msec_session_count--;

        /* stop msec vrrp timer */
        if (0 == p_db_vrrp_glb->vrrp_msec_session_count)
        {
            vrrp_sys_init_msec_timer(p_db_vrrp_glb, FALSE);
        }
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_advt_interval(tbl_vrrp_session_t *p_db_sess, int32 value, uint8 time_mode, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;

    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    if ((p_db_sess->advt_interval == value)
        && (p_db_sess->time_mode == time_mode))
    {
        return rc;
    }

    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }

    if ((GLB_VRRP_SECONDS_MODE == time_mode)
        && (GLB_VRRP_VERSION_V3 == p_db_vrrp_glb->vrrp_version)
        && (value > VRRP_V3_MAX_ADVT_INTERVAL))
    {
        CFG_CONFLICT_RET("Advertisement-interval must be 1 to 40 seconds for vrrp version v3");
    }

    if (GLB_VRRP_MILLI_SECONDS_MODE == time_mode)
    {
        if((value % 100)!=0)
        {
            CFG_CONFLICT_RET("Advertisement-interval msec must be times of 100 milli-second");
        }    
    }

    if (p_db_sess->learn_master && (GLB_VRRP_MILLI_SECONDS_MODE == time_mode))
    {
        CFG_FUNC_ERR_RET("VRRP can not set msec mode when learnmaster mode is set.");
    }
    
    p_db_sess->advt_interval = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ADVT_INTERVAL);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session advertisement interval fail");
        return rc;
    }

    rc = _vrrp_cmd_set_session_time_mode(p_db_sess, time_mode, TBL_VRRP_SESSION_FLD_PREEMPT_MODE, para);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session advertisement time_mode fail");
        return rc;
    }

    return rc;
}

static int32
_vrrp_cmd_get_session(uint32  vrid, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                     *fp = NULL;
    tbl_vrrp_session_key_t   key;
    tbl_vrrp_session_t       *p_db_sess = NULL;
    tbl_iter_args_t          args;    

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == vrid)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vrrp_session_dump_one, &args);
    }
    else
    {
        key.vrid = vrid;  
        p_db_sess = tbl_vrrp_session_get_vrrp_session(&key);
        if (!p_db_sess)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("vrrp session %u does not exist", vrid);
        }

        tbl_vrrp_session_dump_one(p_db_sess, &args);
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_vrrp_cmd_set_session_learn_master(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;

    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }
    else if(GLB_VRRP_MILLI_SECONDS_MODE == p_db_sess->time_mode)
    {
        CFG_CONFLICT_RET("VRRP can not set learnmaster mode when msec mode is set.");
    }
    
    if (p_db_sess->learn_master == value)
    {
        return rc;    
    }
   
    p_db_sess->learn_master = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_LEARN_MASTER);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session time mode fail");
        return rc;
    }

    if (!value)
    {
        p_db_sess->master_down_interval = vrrp_compute_masterdowninterval(p_db_sess, p_db_sess->advt_interval, 
            p_db_sess->priority);
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_preempt_mode(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    
    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }
    
    if (p_db_sess->preempt_mode == value)
    {
        return rc;    
    }

    p_db_sess->preempt_mode = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PREEMPT_MODE);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session preempt mode fail");
        return rc;
    }
    
    p_db_sess->preempt_delay = GLB_VRRP_PREEMPT_DELAY_DFLT;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PREEMPT_DELAY);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session preempt delay fail");
        return rc;
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_preempt_delay(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;

    if (!p_db_sess->preempt_mode)
    {
        CFG_CONFLICT_RET("VRRP can not set preempt delay when preempt mode is false.");    
    }
    
    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }
    
    if (p_db_sess->preempt_delay == value)
    {
        return rc;    
    }

    p_db_sess->preempt_delay = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PREEMPT_DELAY);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session preempt delay fail");
        return rc;
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_priority(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;

    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    if (value == VRRP_DEFAULT_IP_OWNER_PRIORITY && !p_db_sess->owner)
    {
        CFG_CONFLICT_RET("MUST be virtual IP owner to have priority 255");

    }
    else if (value != VRRP_DEFAULT_IP_OWNER_PRIORITY && p_db_sess->owner)
    {
        CFG_CONFLICT_RET("IP Address owner priority MUST be 255");
    }
    
    if (p_db_sess->config_priority == value)
    {
        return rc;    
    }

    p_db_sess->config_priority = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session config priority fail");
        return rc;
    }

    p_db_sess->priority = value;
    if(p_db_sess->trackobj_id != 0)
    {
        vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
    }

    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PRIORITY);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session priority fail");
        return rc;
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_vip(tbl_vrrp_session_t *p_db_sess, char *vip, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_t        sess;
    tbl_vrrp_session_master_t *p_master = NULL;
    tbl_vrrp_session_t        *p_db_other_sess = NULL;
    uint32                    vrid = 0;
    bool                      ip_owner = FALSE;
    bool                      subnet_match = FALSE;

    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));

    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    if (0 == sal_strcmp(vip, "null"))
    {
        if (p_db_sess->owner)
        {
            p_db_sess->owner = FALSE;
            vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
        }

        vrrp_reset_virtual_ip(p_db_sess);
        
        return rc;
    }
    else
    {
        CFG_PARSER_IP_ADDR(vip, &sess.vip.u.prefix4, para->p_rs);
        sess.vip.family = AF_INET;

        /*check whether this vip has been used by another vrrp session*/
        p_master = tbl_vrrp_session_get_master(); 
        if (!p_master)
        {
            CFG_CONFLICT_RET("Tbl_vrrp_session is not exist");    
        }
    
        for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
        {
            p_db_other_sess = p_master->session_array[vrid];
            if (!p_db_other_sess)
            {
                continue;
            }
            
            if ((p_db_other_sess->key.vrid != p_db_sess->key.vrid)
                && (0 == sal_memcmp(&p_db_other_sess->vip, &sess.vip, sizeof(addr_t))))
            {
                CFG_CONFLICT_RET("This virtual IP is already used by another VRRP session!");
            }
        }

        /*check if it is ip address owner */
        /*vip must belong to the interface subnet. added by ychen in 2007-03-15*/
        if(GLB_INVALID_IFINDEX != p_db_sess->ifindex)
        {
            vrrp_addr_subnet_match(p_db_sess->ifname, &sess.vip, &ip_owner, &subnet_match);

            if (!subnet_match)
            {
                CFG_FUNC_ERR_RET("Virtual IP must in the same subnet with the configured interface!");
            }
        }

        if (ip_owner)
        {
            if (p_db_sess->trackobj_id)
            {
                CFG_CONFLICT_RET("Can't configure as ip owner when configured track object!");
            }

            p_db_sess->owner = TRUE;
               
            /* Owner priority MUST be 255. */
            p_db_sess->priority = VRRP_DEFAULT_IP_OWNER_PRIORITY;
        }
        else
        {
             p_db_sess->owner = FALSE;
             vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
        }
        
        p_db_sess->vip_status = TRUE;
        sal_memcpy(&p_db_sess->vip, &sess.vip, sizeof(addr_t));
        
        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_VIP);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Set vrrp session vip fail");
            return rc;
        }

        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_VIP_STATUS);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Set vrrp session vip status fail");
            return rc;
        }
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_interface(tbl_vrrp_session_t *p_db_sess, char *ifname, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_t        sess;
    tbl_vrrp_session_master_t *p_master = NULL;
    tbl_vrrp_session_t        *p_db_other_sess = NULL;
    uint32                    vrid = 0;
    tbl_interface_t           *p_db_if = NULL;
    tbl_interface_t           intf;
    bool                      ip_owner = FALSE;
    uint8                     vrrp_if_count = 0;
    bool                      subnet_match = FALSE;
    uint32                    primary_ip = 0;    
    tbl_route_if_t            *p_db_rtif = NULL;
    tbl_route_if_key_t        rtif_key;

    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));

    if ((VRRP_STATE_INIT != p_db_sess->state) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    /* get vrrp session master */
    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_session is not exist");    
    }

    if (0 == sal_strcmp(ifname, "null"))
    {
        if (GLB_INVALID_IFINDEX == p_db_sess->ifindex)
        {
            return rc;
        }
        
        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_memcpy(rtif_key.name, p_db_sess->ifname, IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
        if (!p_db_rtif)
        {
            CFG_CONFLICT_RET("Invalid interface! Only routed port, vlan interface and layer3 agg interface is allowed!");        
        }

        p_db_rtif->vrrp_session_num--;

        if (p_db_sess->owner)
        {
            p_db_sess->owner = FALSE;
            vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
        }
        
        p_db_sess->ifindex = GLB_INVALID_IFINDEX;
        sal_memset(p_db_sess->ifname, 0, IFNAME_SIZE);
    }
    else
    {
        sal_memset(&intf, 0, sizeof(tbl_interface_t));    
        sal_memcpy(intf.key.name, ifname, sal_strlen(ifname));
        
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Can't find specified interface %s", ifname);
        }

        /* return fail for three conditions:
           1. l2 switch port;
           2. not physical if/linkagg if/vlanif 
           3. agg member */
        if ((GLB_IF_MODE_L2 == p_db_if->mode)
            || ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
            && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
            && (GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type))
            || ((NULL != p_db_if->lag) && sal_memcmp(ifname, "agg", sal_strlen("agg"))))
        {
            CFG_CONFLICT_RET("Invalid interface! Only routed port, vlan interface and layer3 agg interface is allowed!");    
        }
    
        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_memcpy(rtif_key.name, ifname, sal_strlen(ifname));
        p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
        if (!p_db_rtif)
        {
            CFG_CONFLICT_RET("Invalid interface! Only routed port, vlan interface and layer3 agg interface is allowed!");        
        }

        /* check vip and interface vip are in same subnet */
        if (p_db_sess->vip_status)
        {
            vrrp_addr_subnet_match(ifname, &p_db_sess->vip, &ip_owner, &subnet_match);
            if (!subnet_match)
            {
                CFG_CONFLICT_RET("Virtual IP must in the same subnet with the configured interface!");
            }
        }

        for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
        {
            p_db_other_sess = p_master->session_array[vrid];
            if (!p_db_other_sess)
            {
                continue;
            }
            
            if ((p_db_other_sess->key.vrid != p_db_sess->key.vrid)
                && (p_db_other_sess->ifindex == p_db_if->ifindex))
            {
                vrrp_if_count++;
            }
        }

        if(vrrp_if_count >= VRRP_SESS_MAX)
        {
            CFG_CONFLICT_RET("One interface can only support 3 VRRP sessions. Please choose another layer3 interface!");
        }

        if (p_db_if->ifindex == p_db_sess->ifindex)
        {
            return rc;    
        }

        if (ip_owner)
        {           
            p_db_sess->owner = TRUE;
               
            /* Owner priority MUST be 255. */
            p_db_sess->priority= VRRP_DEFAULT_IP_OWNER_PRIORITY;
        }
        else
        {
            p_db_sess->owner = FALSE;
            vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
                 
            /* Non-owner priority CANT be 255 (reset default of 100). */
            if (VRRP_DEFAULT_IP_OWNER_PRIORITY == p_db_sess->priority)
            {
                p_db_sess->priority = p_db_sess->config_priority;
            }
        }
                
        /* set the interface */
        p_db_sess->ifindex = p_db_if->ifindex;
        sal_memcpy(p_db_sess->ifname, p_db_if->key.name, IFNAME_SIZE);

        /* get interface primary ip */
        vrrp_get_interface_primary_ip(p_db_if->key.name, &primary_ip);
        p_db_sess->primary_ip.family = AF_INET;
        p_db_sess->primary_ip.u.prefix4.s_addr = primary_ip;

        p_db_rtif->vrrp_session_num++;
    }

    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_IFNAME);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session ifname fail");
        return rc;
    }

    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_IFINDEX);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session ifindex fail");
        return rc;
    }

    return rc;
}

static int32
_vrrp_cmd_set_session_enable(tbl_vrrp_session_t *p_db_sess, int32 value, int32 field_id, 
    cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_interface_t           *p_db_if = NULL;
    tbl_interface_t           intf;

    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    if (!value)
    {
        /* Make sure it is currently running */
        if (value == p_db_sess->enable)
        {
            CFG_CONFLICT_RET("You must enable this session first");
        }

        p_db_sess->enable = value;
        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);
        if (rc < 0)
        {
            VRRP_LOG_ERR("Set vrrp session enable fail");
            return rc;
        }

        p_db_sess->admin_state = VRRP_ADMIN_STATE_DOWN;

        if (VRRP_STATE_INIT == p_db_sess->state)
        {
            return rc;
        }
        
        /* Disable the session */
        rc = vrrp_shutdown_sess(p_db_sess);
        if (rc)
        {
            p_db_sess->enable = TRUE;
            tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);
            CFG_CONFLICT_RET("Unexpected error when disable vrrp router %d", p_db_sess->key.vrid);
        }
        
        return rc;    
    }
    else
    {        
        /* Make sure it is currently running */
        if (value == p_db_sess->enable)
        {
            CFG_CONFLICT_RET("You must disable this session first");
        }

        
        /* make sure virtual IP address is set */
        if (!p_db_sess->vip_status)
        {
            CFG_CONFLICT_RET("Virtual-ip address is not set when enable vrrp");
        }

        /* make sure interface is set */
        if (GLB_INVALID_IFINDEX == p_db_sess->ifindex)
        {
            CFG_CONFLICT_RET("Interface is not set when enable vrrp");
        }

        /* set vrrp session enable */
        p_db_sess->enable = value;
        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);
        if (rc < 0)
        {
            VRRP_LOG_ERR("Set vrrp session enable fail");
            return rc;
        }

        p_db_sess->admin_state = VRRP_ADMIN_STATE_UP;

        /* set vrrp state to init */
        p_db_sess->state = VRRP_STATE_INIT;

        /* get interface status */
        sal_memcpy(intf.key.name, p_db_sess->ifname, IFNAME_SIZE);  

        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Can't find specified interface");
        }
        
        if (!tbl_interface_is_running(p_db_if))
        {
            p_db_sess->shutdown_flag = TRUE;
            return rc;
        }

        /* enable the session */
        rc = vrrp_noshut_sess(p_db_sess);
        if (rc)
        {
            p_db_sess->enable = FALSE;
            tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);
            CFG_CONFLICT_RET("Unexpected error when enable vrrp router %d", p_db_sess->key.vrid);          
        }
    }

    return rc;
}

static int32
_vrrp_cmd_set_tracobj_id(tbl_vrrp_session_t *p_db_sess, int32 value, uint8 priority_delta, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32      rc = PM_E_NONE;
    tbl_track_object_key_t   obj_key;
    tbl_track_object_t      *p_db_object = NULL;

    if ((p_db_sess->state != VRRP_STATE_INIT) || (VRRP_ADMIN_STATE_UP == p_db_sess->admin_state))
    {
        CFG_CONFLICT_RET("You must disable this session first");
    }

    if(0 == value)
    {
        if(p_db_sess->trackobj_id != 0)
        {
            if(track_del_tracked_vrrp(p_db_sess->trackobj_id, p_db_sess->key.vrid, para) < 0)
            {
                CFG_CONFLICT_RET("Session unset track object failure");
            }
        }
        rc = vrrp_disable_track_object(p_db_sess);

        return rc;
    }

    if (p_db_sess->trackobj_id == value && p_db_sess->priority_delta == priority_delta)
    {
        return rc;
    }

    if (p_db_sess->owner)
    {
        CFG_CONFLICT_RET("IP address owner can not perform track function");
    }

    obj_key.trackobj_id = value;
    p_db_object = tbl_track_object_get_track_object(&obj_key);
    if (!p_db_object)
    {
        CFG_CONFLICT_RET("track object %u does not exist", value);
    }

    if(p_db_sess->trackobj_id != 0)
    {
        if(track_del_tracked_vrrp(p_db_sess->trackobj_id, p_db_sess->key.vrid, para) < 0)
        {
            CFG_CONFLICT_RET("Session unset track object failure");
        }
    }

    p_db_sess->trackobj_id = value;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_TRACKOBJ_ID);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session trackobj_id fail");
        return rc;
    }

    p_db_sess->priority_delta = priority_delta;
    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PRIORITY_DELTA);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set vrrp session track priority_delta fail");
        return rc;
    }

    if(track_add_tracked_vrrp(p_db_sess->trackobj_id, p_db_sess->key.vrid, para) < 0)
    {
        rc = vrrp_disable_track_object(p_db_sess);
    }

    return rc;
}

int32
vrrp_cmd_process_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    tbl_vrrp_session_t        sess; 
    tbl_vrrp_session_t        *p_db_sess = NULL;
    int32                     field_id = 0;
    cdb_field_t               *p_field = NULL;
    int32                     value = 0;
    int32                     i = 0;
    uint8                     priority_delta = 0;
    uint32                    vrid = 0;
    field_parse_t             field;
    
    CFG_DBG_FUNC();

    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));

    p_node = cdb_get_tbl(TBL_VRRP_SESSION);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(vrid, argv[0], para->p_rs);

        rc = _vrrp_cmd_create_session(vrid, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(vrid, argv[0], para->p_rs);

        rc = _vrrp_cmd_delete_session(vrid, para);
        break;
        
    case CDB_OPER_SET:
        PM_CFG_GET_UINT(vrid, argv[i], para->p_rs);

        sess.key.vrid = vrid;
        p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess.key);
        if (!p_db_sess)
        {
            CFG_CONFLICT_RET("VRID does not exist (cannot delete)");
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_VRRP_SESSION_FLD_ADVT_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if (0 == sal_memcmp(argv[3], "sec", 3))
            {
                rc = _vrrp_cmd_set_session_advt_interval(p_db_sess, value, GLB_VRRP_SECONDS_MODE, field_id, para);
            }
            else
            {
                rc = _vrrp_cmd_set_session_advt_interval(p_db_sess, value, GLB_VRRP_MILLI_SECONDS_MODE, field_id, para);
            }
            break;
            
        case TBL_VRRP_SESSION_FLD_LEARN_MASTER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _vrrp_cmd_set_session_learn_master(p_db_sess, value, field_id, para);            
            break;

        case TBL_VRRP_SESSION_FLD_PREEMPT_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _vrrp_cmd_set_session_preempt_mode(p_db_sess, value, field_id, para);            
            break;

        case TBL_VRRP_SESSION_FLD_PREEMPT_DELAY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _vrrp_cmd_set_session_preempt_delay(p_db_sess, value, field_id, para);            
            break;

        case TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _vrrp_cmd_set_session_priority(p_db_sess, value, field_id, para);            
            break;

        case TBL_VRRP_SESSION_FLD_VIP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

            rc = _vrrp_cmd_set_session_vip(p_db_sess, argv[2], field_id, para);
            break;

        case TBL_VRRP_SESSION_FLD_IFNAME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

            rc = _vrrp_cmd_set_session_interface(p_db_sess, argv[2], field_id, para);
            break;
            
        case TBL_VRRP_SESSION_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _vrrp_cmd_set_session_enable(p_db_sess, value, field_id, para);      
            break;

        case TBL_VRRP_SESSION_FLD_TRACKOBJ_ID:
            if(argc > 2)
            {
                PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
                if(argc == 4 && sal_strlen(argv[i+2]))
                {
                    PM_CFG_GET_UINT(priority_delta, argv[i+2], para->p_rs);
                }
                else
                {
                    priority_delta = VRRP_DEFAULT_DELTA_PRIORITY;
                }
            }
            rc = _vrrp_cmd_set_tracobj_id(p_db_sess, value, priority_delta, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(vrid, argv[0], para->p_rs);
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _vrrp_cmd_get_session(vrid, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_vrrp_cmd_get_vip(tbl_vrrp_vip_t  *vip_entry, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field, 
    char* ip_addr)
{
    FILE                     *fp = NULL;
    tbl_vrrp_vip_t           *p_db_vip = NULL;
    tbl_iter_args_t          args;    

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == vip_entry->key.u.prefix4.s_addr)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vrrp_vip_dump_one, &args);
    }
    else
    {
        vip_entry->key.family = AF_INET;
        p_db_vip = tbl_vrrp_vip_get_vrrp_vip(&vip_entry->key);
        if (!p_db_vip)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("vrrp vip %s does not exist", ip_addr);
        }

        tbl_vrrp_vip_dump_one(p_db_vip, &args);
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
vrrp_cmd_process_vip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    tbl_vrrp_vip_t            vip_entry; 
    field_parse_t             field;
    
    CFG_DBG_FUNC();

    sal_memset(&vip_entry, 0, sizeof(tbl_vrrp_vip_t));

    p_node = cdb_get_tbl(TBL_VRRP_VIP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            CFG_PARSER_IP_ADDR(argv[0], &vip_entry.key.u.prefix4, para->p_rs);
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _vrrp_cmd_get_vip(&vip_entry, para, p_node, &field, argv[0]);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_vrrp_cmd_get_vmac(tbl_vrrp_vmac_t  *vmac_entry, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field, 
    char* mac_addr)
{
    FILE                     *fp = NULL;
    tbl_vrrp_vmac_t          *p_db_vmac = NULL;
    tbl_iter_args_t          args;    

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!mac_addr)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vrrp_vmac_dump_one, &args);
    }
    else
    {
        p_db_vmac = tbl_vrrp_vmac_get_vrrp_vmac(&vmac_entry->key);
        if (!p_db_vmac)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("vrrp vmac %s does not exist", mac_addr);
        }

        tbl_vrrp_vmac_dump_one(p_db_vmac, &args);
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
vrrp_cmd_process_vmac(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    tbl_vrrp_vmac_t           vmac_entry; 
    field_parse_t             field;
    
    CFG_DBG_FUNC();

    sal_memset(&vmac_entry, 0, sizeof(tbl_vrrp_vmac_t));

    p_node = cdb_get_tbl(TBL_VRRP_VMAC);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            CFG_PARSER_MAC(vmac_entry.key.mac, argv[0], para->p_rs);
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _vrrp_cmd_get_vmac(&vmac_entry, para, p_node, &field, argv[0]);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


void
vrrp_show_session (cfg_cmd_para_t *para, FILE *fp, tbl_vrrp_session_t *p_db_sess)
{
    char                *state = NULL;
    bool                found = 0;
    uint32              count = 0;
    char                strState[VRRP_MAX_STATE_LEN];
    tbl_interface_t     *p_db_if = NULL;
    tbl_interface_t     intf;
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;
    ds_connected_t      *ifc_tmp = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    char                *vip = NULL;
    char                *owner = NULL;
    char                ipbuf[VRRP_IP_LENGTH];
    char                ifname_ext[IFNAME_EXT_SIZE];
    
    sal_memset(&intf, 0, sizeof(tbl_interface_t));
    sal_memset(&rt_if, 0, sizeof(tbl_route_if_t));
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    
    sal_fprintf(fp, "VRID %d\n", p_db_sess->key.vrid);

    /* Current State */
    if (p_db_sess->state == VRRP_STATE_INIT)
    {
        state = "Initialize";  
    }
    else if (p_db_sess->state == VRRP_STATE_MASTER)
    {
        state = "Master";
    }
    else
    {
        state = "Backup";
    }

    /*when state is init, we should show the reason*/
    if (VRRP_STATE_INIT == p_db_sess->state)
    {
        count = sal_snprintf(strState, VRRP_MAX_STATE_LEN, " %-20s: %s", "State", state);
          
        if (GLB_INVALID_IFINDEX != p_db_sess->ifindex)
        {
            sal_memcpy(intf.key.name, p_db_sess->ifname, IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);
                VRRP_LOG_ERR("Can't find specified interface for ifname %s", ifname_ext);
                return;
            }

            if (!tbl_interface_is_running(p_db_if))
            {
                sal_strcat(strState, "(Interface down");
            }
    
            sal_memcpy(rt_if.key.name, p_db_sess->ifname, IFNAME_SIZE);
        
            p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
            if (!p_db_rtif)
            {
                IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);
                VRRP_LOG_ERR("Can't find route interface for ifname %s", ifname_ext);
                return;
            }
        
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
            {
        
                if (!GLB_FLAG_ISSET(ifc_tmp->flags, RT_IF_ADDR_SECONDARY))
                {
                    found = TRUE;
                    break;
                }     
            }
        
            if (!found)
            {
                if (sal_strlen(strState) > count)
                {
                    sal_strcat(strState, ", No primary IP address");
                }
                else
                {
                    sal_strcat(strState, "(No primary IP address");
                }
            }

            if (!p_db_sess->enable)
            {
                if (sal_strlen(strState) > count)
                {
                    sal_strcat(strState, ", Not enabled");
                }
                else
                {
                    sal_strcat(strState, "(Not enabled");
                }
            } 
            
            if (sal_strlen(strState) > count)
            {
                sal_strcat(strState, ")");
            }

            strState[VRRP_MAX_STATE_LEN - 1] = '\0';
        }
      
        sal_fprintf(fp, "%s\n", strState);
    }
    else
    {
        sal_fprintf(fp, " %-20s: %s\n", "State", state);
    }

    /* Virtual IP address */
    if (!p_db_sess->vip_status)
    {
        vip = "Unset";
        owner = NULL;
    }
    else 
    {
        cdb_addr_val2str(ipbuf, VRRP_IP_LENGTH, &p_db_sess->vip);
        vip = ipbuf;

        if (p_db_sess->owner)
        {
            owner = "IP owner";
        }
        else
        {
            owner = "Not IP owner";
        }
    }

    if (owner)
    {
        sal_fprintf(fp, " %-20s: %s(%s)\n", "Virtual IP", vip, owner);
    }
    else
    {
        sal_fprintf(fp, " %-20s: %s\n", "Virtual IP", vip);
    }

    
    /*interface*/
    if (GLB_INVALID_IFINDEX == p_db_sess->ifindex)
    {
        sal_fprintf(fp, " %-20s: %s\n", "Interface", "Unset");
    }
    else
    {
        IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);
        sal_fprintf(fp, " %-20s: %s\n", "Interface", ifname_ext);
    }

    sal_fprintf(fp, " %-20s: %02x%02x.%02x%02x.%02x%02x\n", "VMAC", p_db_sess->vmac[0], 
        p_db_sess->vmac[1], p_db_sess->vmac[2], p_db_sess->vmac[3], 
        p_db_sess->vmac[4], p_db_sess->vmac[5]);          
       
    /* Advertisement interval */
    if(GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
    {
        sal_fprintf(fp, " %-20s: %d second(s)\n", "Advt timer", p_db_sess->advt_interval);
    }
    else
    {
        sal_fprintf(fp, " %-20s: %d milli-second(s)\n", "Advt timer", p_db_sess->advt_interval);
    }

    /* Preempt-mode */
    sal_fprintf(fp, " %-20s: %s\n", "Preempt mode", p_db_sess->preempt_mode? "TRUE" : "FALSE");
 
    /* Priority */
    sal_fprintf(fp, " %-20s: %-15d", "Conf pri", p_db_sess->config_priority);
    sal_fprintf(fp, "%-10s: %d\n", "Run pri", p_db_sess->priority);

    if(p_db_sess->trackobj_id != 0)
    {
        sal_fprintf(fp, " %-20s: %d\n", "Track Object", p_db_sess->trackobj_id);
        sal_fprintf(fp, " %-20s: %d\n", "Decre pri", p_db_sess->priority_delta);
    }

    /* if master, set master info here*/
    if (VRRP_STATE_MASTER == p_db_sess->state)
    {
        p_db_sess->master_ip_status = TRUE;
        sal_memcpy(&p_db_sess->master_ip, &p_db_sess->primary_ip, sizeof(addr_t));
        p_db_sess->master_priority = p_db_sess->priority;
        p_db_sess->master_advt_interval = p_db_sess->advt_interval;
        p_db_sess->master_down_interval = vrrp_compute_masterdowninterval(p_db_sess, p_db_sess->advt_interval, p_db_sess->priority);
    }
    
    if (!p_db_sess->master_ip_status)
    {
        sal_fprintf(fp, " %-20s: %s\n", "Master router ip", "Unknown");
        sal_fprintf(fp, " %-20s: %-15s\n", "Master priority", "Unknown");
        sal_fprintf(fp, " %-20s: Unknown\n", "Master advt timer");
        sal_fprintf(fp, " %-20s: Unknown\n", "Master down timer");
    }
    else 
    {
        cdb_addr_val2str(ipbuf, VRRP_IP_LENGTH, &p_db_sess->master_ip);
        sal_fprintf(fp, " %-20s: %s\n", "Master router ip", ipbuf);
        sal_fprintf(fp, " %-20s: %-15d\n", "Master priority", p_db_sess->master_priority);

        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            sal_fprintf(fp, " %-20s: %d second(s)\n", "Master advt timer", p_db_sess->master_advt_interval);
        }
        
        if(GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            sal_fprintf(fp, " %-20s: %d second(s)\n", "Master down timer", 
                (p_db_sess->master_down_interval - p_db_sess->preempt_delay));
        }
        else
        {
            sal_fprintf(fp, " %-20s: %d milli-second(s)\n", "Master down timer", p_db_sess->advt_interval * 3);
        }
    }

    sal_fprintf(fp, " %-20s: %d second(s)\n", "Preempt delay", p_db_sess->preempt_delay);

    if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
    {
        if (p_db_sess->learn_master)
        {
            sal_fprintf(fp, " %-20s: TRUE\n", "Learn master mode");
        }
        else
        {
            sal_fprintf(fp, " %-20s: FALSE\n",  "Learn master mode");
        }
    }

    return;        
}

int32
vrrp_cmd_show_vrrp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    FILE                      *fp = NULL;
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;

    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_global not found");
    }
    
    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_vrrp_session is not exist");    
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (0 == argc)
    {  
        sal_fprintf(fp, "vrrp version         : %d\n", p_db_vrrp_glb->vrrp_version);
        
        sal_fprintf(fp, "vrrp session count   : %d\n\n", p_db_vrrp_glb->vrrp_session_count);
        
        for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
        {
            p_db_sess = p_master->session_array[vrid];
            if (!p_db_sess)
            {
                continue;
            }

            vrrp_show_session (para, fp, p_db_sess);
        }
    }
    else
    {
        PM_CFG_GET_UINT(vrid, argv[0], para->p_rs);
        p_db_sess = p_master->session_array[vrid];

        if (!p_db_sess)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("No such vrrp session exist");
        }

        vrrp_show_session (para, fp, p_db_sess);
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

void
rt_cmd_set_vrrp_process()
{
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;
    
    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if ((p_db_vrrp_glb) && (0 == p_db_vrrp_glb->vrrp_version))
    {
        p_db_vrrp_glb->vrrp_version = GLB_VRRP_VERSION_V2;
        tbl_vrrp_global_set_vrrp_global_field(p_db_vrrp_glb, TBL_VRRP_GLOBAL_FLD_VRRP_VERSION);
    }
    
    return;
}

/* support vrrp priority 0 advt modified by liwh for bug 45503, 2017-10-30 */
int32
routeif_cmd_pre_down(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vrrp_session_master_t *p_master = NULL;
    tbl_vrrp_global_t         *p_db_vrrp_glb = NULL;
    tbl_interface_t              *p_db_if = NULL;
    tbl_interface_t              intf;
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;

    if (1 == argc)
    {
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        /* check exist */
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", argv[0]); 
        }

        /* modified by liwh for bug 48995, 2018-09-25 */
        sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
        sal_strncpy(rtif.key.name, argv[0], IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
        if (p_db_rtif)
        {
            nd_api_if_up_pre_unset(p_db_rtif);
        }
        /* liwh end */

        p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
        if (!p_db_vrrp_glb)
        {
            CFG_CONFLICT_RET("Tbl_vrrp_global not found");
        }
        
        p_master = tbl_vrrp_session_get_master();
        if (!p_master)
        {
            CFG_CONFLICT_RET("Tbl_vrrp_session is not exist");    
        }

        vrrp_if_down(p_db_if);

        usleep(100000); /*sleep 100ms*/
    }
    
    return PM_E_NONE;
}
/* liwh end */

int32
vrrp_start()
{
    cdb_register_cfg_tbl_cb(TBL_VRRP_GLOBAL, vrrp_cmd_process_global);
    cdb_register_cfg_tbl_cb(TBL_VRRP_SESSION, vrrp_cmd_process_session);
    cdb_register_cfg_tbl_cb(TBL_VRRP_VIP, vrrp_cmd_process_vip);
    cdb_register_cfg_tbl_cb(TBL_VRRP_VMAC, vrrp_cmd_process_vmac);

    cdb_register_cfg_act_cb(ACT_SHOW_VRRP, vrrp_cmd_show_vrrp);

    /* support vrrp priority 0 advt modified by liwh for bug 45503, 2017-10-30 */
    cdb_register_cfg_act_cb(ACT_L3IF_PRE_DOWN, routeif_cmd_pre_down);
    /* liwh end */
    
    ipc_register_pkt_rx_fn(GLB_PKT_VRRP, vrrp_pdu_rx);
    return PM_E_NONE;
}


