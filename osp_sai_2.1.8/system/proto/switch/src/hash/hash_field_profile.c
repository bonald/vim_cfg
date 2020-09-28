#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_hash_field_value_define.h"
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
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"
#include "ctclib_slist.h"

ctclib_opb_t    hash_field_pro_index_opb;

int32
hash_field_profile_set_description(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION));
    return rc;
}

int32
hash_field_profile_set_l2_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_L2_SET));
    return rc;
}

int32
hash_field_profile_set_ip_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_IP_SET));
    return rc;
}

int32
hash_field_profile_set_ipv6_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET));
    return rc;
}

int32
hash_field_profile_set_gre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_GRE_SET));
    return rc;
}

int32
hash_field_profile_set_l3vpn_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET));
    return rc;
}

int32
hash_field_profile_set_nvgre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET));
    return rc;
}

int32
hash_field_profile_set_mpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{   
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET));
    return rc;
}

int32
hash_field_profile_set_vpws_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET));
    return rc;
}

int32
hash_field_profile_set_vpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET));
    return rc;
}

int32
hash_field_profile_set_vxlan_field_value(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET));
    return rc;
}

int32
hash_field_profile_set_hash_arithmetic(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC));
    return rc;
}

int32
hash_field_profile_set_mode_symmetry(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY));
    return rc;
}

int32
hash_field_profile_set_bitmap_disable(tbl_hash_field_profile_t* p_hash_fld_pro)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_hash_field_profile_set_hash_field_profile_field(p_hash_fld_pro, TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE));
    return rc;
}

/*added by wanh for bug 54047, 2019-10-16*/
int32
hash_field_profile_create(uint32 index, char *name)
{
    tbl_hash_field_profile_t hash_fld_pro;
    tbl_hash_field_profile_t* p_db_hash_fld_pro = NULL;
    int32 rc = PM_E_NONE;
    
    sal_memset(&hash_fld_pro, 0, sizeof(hash_fld_pro));
    sal_strncpy(hash_fld_pro.key.name, name, sizeof(hash_fld_pro.key.name));
    p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&(hash_fld_pro.key));

    if (NULL != p_db_hash_fld_pro)
    {
        return PM_E_NONE;
    }
    
    hash_fld_pro.index = index;
    hash_fld_pro.l2_set = GLB_HASH_FIELD_PROFILE_L2_SET_DEFAULT;
    hash_fld_pro.ip_set = GLB_HASH_FIELD_PROFILE_IP_SET_DEFAULT;
    hash_fld_pro.ipv6_set = GLB_HASH_FIELD_PROFILE_IPV6_SET_DEFAULT;
    hash_fld_pro.gre_set = GLB_HASH_FIELD_PROFILE_GRE_SET_DEFAULT;
    hash_fld_pro.vxlan_set = GLB_HASH_FIELD_PROFILE_VXLAN_SET_DEFAULT;
    hash_fld_pro.nvgre_set = GLB_HASH_FIELD_PROFILE_NVGRE_SET_DEFAULT;
    hash_fld_pro.mpls_set = GLB_HASH_FIELD_PROFILE_MPLS_SET_DEFAULT;
/*added by yejl for bug 54080, 2019-10-24*/
    //hash_fld_pro.vpws_set = GLB_HASH_FIELD_PROFILE_VPWS_SET_DEFAULT;
    //hash_fld_pro.vpls_set = GLB_HASH_FIELD_PROFILE_VPLS_SET_DEFAULT;
    //hash_fld_pro.l3vpn_set = GLB_HASH_FIELD_PROFILE_L3VPN_SET_DEFAULT;
/*end*/
    hash_fld_pro.hash_arithmetic = GLB_HASH_FIELD_PROFILE_HASH_ARITHMETIC_DEFAULT;
    hash_fld_pro.mode_symmetry = 0;
    hash_fld_pro.bitmap_disable = 0;
    
    PM_E_RETURN(tbl_hash_field_profile_add_hash_field_profile(&hash_fld_pro));
    
    return PM_E_NONE;
}
/*wanh end*/

/*added by wanh for bug 54047, 2019-10-16*/
int32
hash_field_profile_check_in_use(uint32 field_id)
{
    uint32 field_id_inuse = 0;    
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    tbl_hash_value_profile_t *p_db_hash_value_pro = NULL; 
    ctclib_slistnode_t *listnode = NULL;

    tbl_hash_value_global_t *p_hash_value_glb = NULL;
    p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();

    /*check hash value*/
    CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_db_hash_value_pro, listnode)
    {
        field_id_inuse = p_db_hash_value_pro->agg_static / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }
        
        field_id_inuse = p_db_hash_value_pro->agg_dynamic / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }
        

        field_id_inuse = p_db_hash_value_pro->ecmp_static_l3 / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }

        field_id_inuse = p_db_hash_value_pro->ecmp_static_nvgre / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }

        field_id_inuse = p_db_hash_value_pro->ecmp_static_vxlan / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }


        field_id_inuse = p_db_hash_value_pro->ecmp_dynamic / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }


        field_id_inuse = p_db_hash_value_pro->ecmp_dynamic_flow_id / GLB_HASH_OFFSET_VALUE_SIZE;
        if (field_id_inuse == field_id)
        {
            return GLB_HASH_FIELD_IN_USE;
        }    
    }

    /*check hash value global*/
    field_id_inuse = p_hash_value_glb->agg_hash / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_all / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_static / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_static_l3 / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_static_mpls / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_static_nvgre/ GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_static_vxlan / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_dynamic / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->ecmp_dynamic_flow_id / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }

    field_id_inuse = p_hash_value_glb->lbid / GLB_HASH_OFFSET_VALUE_SIZE;
    if (field_id_inuse == field_id)
    {
        return GLB_HASH_FIELD_IN_USE;
    }
   
    return GLB_HASH_FIELD_NOT_IN_USE;
}
/*wanh end*/

/*modified by wanh for bug 54047, 2019-10-16*/
int32
hash_field_profile_init(void)
{
    int32 rc = PM_E_NONE;

    rc = hash_field_profile_create(GLB_HASH_FIELD_PORT_CHANNEL_INDEX, GLB_HASH_FIELD_PORT_CHANNEL_STR);
    if (PM_E_NONE != rc )
    {
        return rc;
    }
    
    rc = hash_field_profile_create(GLB_HASH_FIELD_ECMP_INDEX, GLB_HASH_FIELD_ECMP_STR);
    if (PM_E_NONE != rc )
    {
        return rc;
    }
       
    return rc;
}
/*wanh end*/

int32
hash_field_profile_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_HASH_FIELD_PROFILE, hash_field_profile_cmd_process_hash); 
    cdb_register_cfg_act_cb(ACT_SHOW_HASH_FIELD, hash_field_profile_cmd_show_process_hash);

    ctclib_opb_create(&hash_field_pro_index_opb, GLB_HASH_FIELD_PROFILE_START_INDEX, GLB_HASH_FIELD_PROFILE_MAX_INDEX, "hash field profile index");
    return PM_E_NONE;
}

