#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_hash_field_value_define.h"
#include "ctclib_opb.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_hash_field_profile_define.h" 
#include "gen/tbl_hash_field_profile.h" 
#include "gen/tbl_hash_value_profile_define.h" 
#include "gen/tbl_hash_value_profile.h" 
#include "gen/tbl_hash_value_global_define.h" 
#include "gen/tbl_hash_value_global.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "switch.h"
#include "acl.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"

ctclib_opb_t    hash_value_pro_index_opb;

tbl_hash_value_profile_t *
hash_value_profile_lookup_by_id(uint8 profile_id)
{
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_hash_value_pro, listnode)
    {
        if (p_hash_value_pro && p_hash_value_pro->index == profile_id)
        {
            return p_hash_value_pro;
        }
    }
    return NULL;
}

int32
hash_value_profile_unlock(uint8 profile_id)
{
    int32 rc = PM_E_NONE;
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;

    p_hash_value_pro = hash_value_profile_lookup_by_id(profile_id);
    if (p_hash_value_pro)
    {   
        if (p_hash_value_pro->ref_cnt == 0)
        {
            return PM_E_NONE;
        }
        else
        {
            p_hash_value_pro->ref_cnt --;
            PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_REF_CNT));
        }
    }
    return rc;
}

int32
hash_value_profile_lock(uint8 profile_id)
{
    int32 rc = PM_E_NONE;
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;

    p_hash_value_pro = hash_value_profile_lookup_by_id(profile_id);
    if (p_hash_value_pro)
    {
        p_hash_value_pro->ref_cnt ++;
        PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_REF_CNT));
    }
    return rc;
}

int32
hash_value_on_modify_hash_value_profile_egress(uint32 profile_id)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if && p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            if (p_db_if->hash_value_profile_id_agg == profile_id)
            {
                return PM_E_FAIL;
            }
        }
    }
    return PM_E_NONE;
}

int32
hash_value_on_modify_profile_agg_hash(uint32 profile_id)
{
    int32 rc = PM_E_NONE;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode1 = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode1)
    {
        if (p_db_if && p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            if (p_db_if->hash_value_profile_id_agg == profile_id)
            {
                p_hash_value_pro = hash_value_profile_lookup_by_id(profile_id);
                if (p_hash_value_pro)
                {
                    p_db_if->hash_value_agg_hash = p_hash_value_pro->agg_input;
                    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH));
                }
            }
        }
    }
    return PM_E_NONE;
}

int32
hash_value_on_modify_global_agg_hash(uint32 profile_id)
{
    int32 rc = PM_E_NONE;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_hash_value_global_t * p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if && p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            if (p_db_if->hash_value_profile_id_agg == profile_id)
            {
                p_db_if->hash_value_agg_hash = p_hash_value_glb->agg_hash;
                PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH));
            }
        }
    }
    return PM_E_NONE;
}


int32
hash_value_field_set_description(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION));
    return rc;
}

int32
hash_value_field_set_port_channel_dynamic(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_field_set_port_channel_static(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_field_set_port_channel_all(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_field_set_ecmp_dynamic(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_field_set_ecmp_static(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_field_set_ecmp_all(tbl_hash_value_profile_t* p_hash_value_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID));
    PM_E_RETURN(tbl_hash_value_profile_set_hash_value_profile_field(p_hash_value_pro, TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_set_ecmp_dynamic(tbl_hash_value_global_t* p_hash_value_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_set_ecmp_static(tbl_hash_value_global_t* p_hash_value_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_set_ecmp_all(tbl_hash_value_global_t* p_hash_value_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_set_port_channel(tbl_hash_value_global_t* p_hash_value_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_set_lbid(tbl_hash_value_global_t* p_hash_value_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_LBID));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(p_hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));
    return rc;
}

int32
hash_value_global_init(void)
{
    tbl_hash_value_global_t hash_value_glb;
    int32 rc = PM_E_NONE;

    sal_memset(&hash_value_glb, 0, sizeof(hash_value_glb));
    hash_value_glb.bitmap_command = GLB_COMMAND_ECMP_SET_ALL_GLOBAL;
    hash_value_glb.bitmap_data = GLB_HASH_VALUE_GLOBAL_BITMAP_DATA_DEFAULT;
    hash_value_glb.agg_hash = GLB_HASH_AGG_HASH_VALUE_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_all = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_dynamic = GLB_HASH_ECMP_DYNAMIC_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_dynamic_flow_id = GLB_HASH_ECMP_DYNAMIC_FLOW_ID_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_static = GLB_HASH_ECMP_STATIC_ALL_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_static_l3 = GLB_HASH_ECMP_STATIC_L3_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_static_nvgre = GLB_HASH_ECMP_STATIC_NVGRE_DEFAULT_GLOBAL;
    hash_value_glb.ecmp_static_vxlan = GLB_HASH_ECMP_STATIC_VXLAN_DEFAULT_GLOBAL;
    hash_value_glb.lbid = GLB_HASH_LBID_HASH_VALUE_DEFAULT_GLOBAL;

    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_LBID));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND));
    PM_E_RETURN(tbl_hash_value_global_set_hash_value_global_field(&hash_value_glb, TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA));

    return rc;    
}


int32
hash_value_profile_and_global_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_HASH_VALUE_PROFILE, hash_value_profile_cmd_process_hash);
    cdb_register_cfg_tbl_cb(TBL_HASH_VALUE_GLOBAL, hash_value_global_cmd_process_hash);
    cdb_register_cfg_act_cb(ACT_SHOW_HASH_VALUE, hash_value_cmd_show_process_hash);

    ctclib_opb_create(&hash_value_pro_index_opb, 1, GLB_HASH_VALUE_PROFILE_MAX_NUM, "hash value index");
    return PM_E_NONE;
}

