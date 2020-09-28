#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "switch.h"
#include "glb_macro.h"
#include "vlanclass.h"
#include "if_mgr.h"


int32
vlanclass_set_entry_dynamic_count(tbl_vlanclass_rule_t* p_db_vclass_rule, bool add)
{
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    int32 rc = PM_E_NONE;
    if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_MAC)
    {
        if (add)
        {
            p_brg_global->mac_based_vlan_class++;
        }
        else
        {
            p_brg_global->mac_based_vlan_class--;
        }
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS));
    }
    else if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_IPV4)
    {
        if (add)
        {
            p_brg_global->ipv4_based_vlan_class++;
        }
        else
        {
            p_brg_global->ipv4_based_vlan_class--;
        }
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS));
    }
    else if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_IPV6)
    {
        if (add)
        {
            p_brg_global->ipv6_based_vlan_class++;
        }
        else
        {
            p_brg_global->ipv6_based_vlan_class--;
        }
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS));
    }
    return rc;
}


static tbl_vlanclass_group_t *
vlanclass_get_vlanclass_group()
{
    int i = 0;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    for (i = 0; i<= GLB_VLANCLASS_GROUP_ID_MAX; i++)
    {
        sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
        vclass_gkey.group_id = i;
        p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
        if (p_db_vclass_group)
        {
            return p_db_vclass_group;
        }
    }
    return NULL;
}  

bool
vlanclass_other_group_is_exist(uint32 group_id)
{
    int i = 0;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    for (i = 0; i<= GLB_VLANCLASS_GROUP_ID_MAX; i++)
    {
        if (i == group_id)
        {
            continue;
        }
        sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
        vclass_gkey.group_id = i;
        p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
        if (p_db_vclass_group)
        {
            return TRUE;
        }
    }
    return FALSE;
}
    
int32
vlanclass_add_rule(tbl_vlanclass_rule_t *p_vclass_rule)
{
    int32 rc = PM_E_NONE;
    
    /* Add local CDB */
    PM_E_RETURN(tbl_vlanclass_rule_add_vlanclass_rule(p_vclass_rule));
    return rc;
}

int32
vlanclass_del_rule(uint32 rule_id)
{
    int32 rc = PM_E_NONE;
    tbl_vlanclass_group_t *p_vclass_group = NULL;
    tbl_vlanclass_rule_key_t vclass_key;

    p_vclass_group = vlanclass_get_vlanclass_group();
    if (NULL == p_vclass_group)
    {
        /*if vlanclass group is NULL, delete rule id local CDB directly*/
        sal_memset(&vclass_key, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_key.rule_id = rule_id;
        PM_E_RETURN(tbl_vlanclass_rule_del_vlanclass_rule(&vclass_key));
    }
    else
    {
        /*if vlanclass group is not NULL, delete rule id from group if the rule is in the group*/
        if (GLB_BMP_ISSET(p_vclass_group->rule_id, rule_id))
        {
            /* del rule form group */
            PM_E_RETURN(vlanclass_del_rule_from_group(p_vclass_group, rule_id));
        }
        /* delete local CDB */
        sal_memset(&vclass_key, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_key.rule_id = rule_id;
        PM_E_RETURN(tbl_vlanclass_rule_del_vlanclass_rule(&vclass_key));
    }
    return rc;
}

int32
vlanclass_add_rule_to_group(uint32 group_id, tbl_vlanclass_rule_t *p_vclass_rule)
{
    int32 rc = PM_E_NONE;
    uint64 action = 0;
    bool group_is_exist = FALSE;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    tbl_vlanclass_group_t vclass_group;
    
    vclass_gkey.group_id = group_id;
    p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
    if (NULL == p_db_vclass_group)
    {
        /*if other group is exist, return*/
        group_is_exist = vlanclass_other_group_is_exist(group_id);
        if (group_is_exist)
        {
            return PM_E_EXIST;
        }
        /*add the vlanclass_group CDB*/
        sal_memset(&vclass_group, 0, sizeof(tbl_vlanclass_group_t));
        vclass_group.key.group_id = group_id;
        GLB_BMP_SET(vclass_group.rule_id, p_vclass_rule->key.rule_id);
        PM_E_RETURN(tbl_vlanclass_group_add_vlanclass_group(&vclass_group));
        action = GLB_VLAN_CLASS_RULE_ACTION_ADD;
        vclass_group.action_rule_id = (action << 32) + p_vclass_rule->key.rule_id;
        PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(&vclass_group, TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID));
    }
    else
    {
        /*update the vlanclass_group CDB*/
        GLB_BMP_SET(p_db_vclass_group->rule_id, p_vclass_rule->key.rule_id);
        PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_db_vclass_group, TBL_VLANCLASS_GROUP_FLD_RULE_ID));
        action = GLB_VLAN_CLASS_RULE_ACTION_ADD;
        p_db_vclass_group->action_rule_id = (action << 32) + p_vclass_rule->key.rule_id;
        PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_db_vclass_group, TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID));
    }
    
    return rc;
}

int32
vlanclass_del_rule_from_group(tbl_vlanclass_group_t* p_vclass_group, uint32 rule_id)
{
    int32 rc = PM_E_NONE;
    int32 offset = 0;
    int32 count = 0;
    uint64 action = 0;
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_master_t* p_master = tbl_interface_get_master();;
    ctclib_slistnode_t *listnode = NULL;
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;

    /*if the rule id is not exist, return*/
    sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
    vclass_rkey.rule_id = rule_id;
    p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
    if (NULL == p_db_vclass_rule)
    {
        return PM_E_NOT_EXIST;
    }
        
    /*update the vlanclass_group CDB*/
    GLB_BMP_UNSET(p_vclass_group->rule_id, rule_id);
    PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_vclass_group, TBL_VLANCLASS_GROUP_FLD_RULE_ID));
    action = GLB_VLAN_CLASS_RULE_ACTION_DEL;
    p_vclass_group->action_rule_id = (action << 32) + rule_id;
    PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_vclass_group, TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID));

    /*added by yejl to fix bug 41594, 2016-12-01.*/
    PM_E_RETURN(vlanclass_set_entry_dynamic_count(p_db_vclass_rule, FALSE));
    
    for (offset = 0; offset < GLB_VLANCLASS_RULE_ID_MAX; offset++)
    {
        if (GLB_BMP_ISSET(p_vclass_group->rule_id, offset))
        {
            count++;
        }
    }
    /*If the vclass_bmp is NULL, delete the vlanclass group from interface and CDB*/
    if (0 == count)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            if ((p_db_if->vlanclass_group_id == p_vclass_group->key.group_id) 
                && (p_db_if->vlanclass_active_type != 0))
            {
                PM_E_RETURN(if_mgr_set_vlanclass_del_group_from_if(p_db_if));
            }
        }
        sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
        vclass_gkey.group_id = p_vclass_group->key.group_id;
        PM_E_RETURN(tbl_vlanclass_group_del_vlanclass_group(&vclass_gkey));
    }
    return rc;
}

int32
vlanclass_del_group(tbl_vlanclass_group_t* p_vclass_group)
{
    int32 rc = PM_E_NONE;
    uint32 offset = 0;
    uint64 action = 0;
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_master_t* p_master = tbl_interface_get_master();;
    ctclib_slistnode_t *listnode = NULL;
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;

    /*process and update CDB*/
    for (offset = 0; offset < GLB_VLANCLASS_RULE_ID_MAX; offset++)
    {
        if (!GLB_BMP_ISSET(p_vclass_group->rule_id, offset))
        {
            continue;
        }
        sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_rkey.rule_id = offset;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
        if (NULL == p_db_vclass_rule)
        {
            return PM_E_NOT_EXIST;
        }
        GLB_BMP_UNSET(p_vclass_group->rule_id, offset);
        PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_vclass_group, TBL_VLANCLASS_GROUP_FLD_RULE_ID));
        action = GLB_VLAN_CLASS_RULE_ACTION_DEL;
        p_vclass_group->action_rule_id = (action << 32) + offset;
        PM_E_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_vclass_group, TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID));
        PM_E_RETURN(vlanclass_set_entry_dynamic_count(p_db_vclass_rule, FALSE));
    }

    /*delete the vlanclass group from interface*/
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if ((p_db_if->vlanclass_group_id == p_vclass_group->key.group_id) 
            && (p_db_if->vlanclass_active_type != 0))
        {
            PM_E_RETURN(if_mgr_set_vlanclass_del_group_from_if(p_db_if));
        }
    }
    /*delete vlanclass_group CDB*/
    sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
    vclass_gkey.group_id = p_vclass_group->key.group_id;
    PM_E_RETURN(tbl_vlanclass_group_del_vlanclass_group(&vclass_gkey));

    return rc;
}

int32
vlanclass_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_VLANCLASS_RULE,            vlanclass_cmd_process_vlanclass_rule);
    cdb_register_cfg_tbl_cb(TBL_VLANCLASS_GROUP,           vlanclass_cmd_process_vlanclass_group);
    
    cdb_register_cfg_act_cb(ACT_SHOW_VLANCLASS_RULE,       vlanclass_cmd_process_vlanclass_rule_show);
    cdb_register_cfg_act_cb(ACT_SHOW_VLANCLASS_GROUP,      vlanclass_cmd_process_vlanclass_group_show);
    cdb_register_cfg_act_cb(ACT_SHOW_VLANCLASS_IF,         vlanclass_cmd_process_vlanclass_if_show);
    
    return PM_E_NONE;
}

int32
vlanclass_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_VLANCLASS_RULE,            NULL);
    cdb_register_cfg_tbl_cb(TBL_VLANCLASS_GROUP,           NULL);
    
    return PM_E_NONE;
}

