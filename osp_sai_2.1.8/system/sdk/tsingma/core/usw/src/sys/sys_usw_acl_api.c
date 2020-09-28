/**
   @file sys_usw_acl_api.c

   @date 2015-11-01

   @version v3.0

 */

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_common.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_hash.h"
#include "ctc_parser.h"
#include "ctc_acl.h"
#include "ctc_linklist.h"
#include "ctc_spool.h"
#include "ctc_field.h"
#include "ctc_packet.h"

#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_parser.h"
#include "sys_usw_parser_io.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_l3if.h"
#include "sys_usw_port.h"
#include "sys_usw_acl.h"

#include "drv_api.h"

extern sys_acl_master_t* p_usw_acl_master[CTC_MAX_LOCAL_CHIP_NUM];
#define SYS_ACL_UDF_PRI_2_IDX(prio)  (0xF - prio)
#define SYS_ACL_UDF_IDX_2_PRI(idx) SYS_ACL_UDF_PRI_2_IDX(idx)
#define SYS_ACL_MAX_TCAM_IDX_NUM  16*1024
#define _ACL_OTHER
/* Internal function for internal cli */
int32
sys_usw_acl_set_global_cfg(uint8 lchip, uint8 property, uint32 value)
{
    sys_acl_register_t acl_register;
    SYS_ACL_INIT_CHECK();
    /* get first */
    sal_memcpy(&acl_register, &(p_usw_acl_master[lchip]->acl_register), sizeof(sys_acl_register_t));

    /* set user input */
    if (SYS_ACL_IGS_SERVICE_ACL_EN == property)
    {
        acl_register.ingress_service_acl_en = value;
    }
    else if (SYS_ACL_EGS_SERVICE_ACL_EN == property)
    {
        acl_register.egress_service_acl_en = value;
    }
    else if(SYS_ACL_BRG_PKT_VLAN_CID_HIGH_PRIO == property)
    {
        acl_register.brg_pkt_vlan_cid_high_prio = value;
    }
    else if (SYS_ACL_XGPON_GEM_PORT_EN == property)
    {
        acl_register.xgpon_gem_port_en = value;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_acl_set_register(lchip, &acl_register));

    /* save it for internal usage */
    sal_memcpy(&(p_usw_acl_master[lchip]->acl_register), &acl_register, sizeof(sys_acl_register_t));

    return CTC_E_NONE;
}
#if 0
int32 sys_usw_acl_vlan_tag_op_untranslate(uint8 lchip, uint8 op_in, uint8 mo_in, uint8* op_out)
{
    if(mo_in && (op_in==1))
    {
        *op_out = CTC_VLAN_TAG_OP_REP_OR_ADD;
        return CTC_E_NONE;
    }

    switch(op_in)
    {
        case 0:
            *op_out = CTC_VLAN_TAG_OP_NONE;
            break;
        case 1:
            *op_out = CTC_VLAN_TAG_OP_REP;
            break;
        case 2:
            *op_out = CTC_VLAN_TAG_OP_ADD;
            break;
        case 3:
            *op_out = CTC_VLAN_TAG_OP_DEL;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
#endif
#define _ACL_GROUP_
/*
 * create an acl group.
 * For init api, group_info can be NULL.
 * For out using, group_info must not be NULL.
 *
 */
int32
sys_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    sys_acl_group_t* pg     = NULL;
    int32          ret;

    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);
    SYS_ACL_CHECK_RSV_GROUP_ID(group_id); /* reserve group cannot be created by user */
    /* check if priority bigger than biggest priority */

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: group_id: %u \n", group_id);

    /*
     *  group_id is uint32.
     *  #1 check block_num from p_p_usw_acl_master[lchip]. precedence cannot bigger than block_num.
     *  #2 malloc a sys_acl_group_t, add to hash based on group_id.
     */

    SYS_USW_ACL_LOCK(lchip);

    /* check if group exist */
    _sys_usw_acl_get_group_by_gid(lchip, group_id, &pg);
    if (pg)
    {
        ret = CTC_E_EXIST;
        goto error0;
    }

    ret = _sys_usw_acl_create_group(lchip, group_id, group_info);
    if (ret)
    {
        goto error0;
    }

    SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NONE);

error0:
    SYS_USW_ACL_UNLOCK(lchip);
    return ret;
}

/*
 * destroy an empty group.
 */
int32
sys_usw_acl_destroy_group(uint8 lchip, uint32 group_id)
{
    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);
    SYS_ACL_CHECK_RSV_GROUP_ID(group_id); /* reserve group cannot be destroyed */

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: group_id: %u \n", group_id);

    /*
     * #1 check if entry all removed.
     * #2 remove from hash. free sys_acl_group_t.
     */
    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_destroy_group(lchip, group_id));

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
 * install a group (empty or NOT) to hardware table
 */
int32
sys_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: gid %u  \n", group_id);

    SYS_USW_ACL_LOCK(lchip);
    /* get group node */
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_install_group(lchip, group_id, group_info));

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/*
 * uninstall a group (empty or NOT) from hardware table
 */
int32
sys_usw_acl_uninstall_group(uint8 lchip, uint32 group_id)
{
    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: gid %u \n", group_id);

    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_uninstall_group(lchip, group_id));

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
 * get group info by group id. NOT For hash group.
 */
int32
sys_usw_acl_get_group_info(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    sys_acl_group_info_t* pinfo = NULL;  /* sys group info */
    sys_acl_group_t     * pg    = NULL;

    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);
    SYS_ACL_CHECK_RSV_GROUP_ID(group_id); /* rsv group has no group_info */

    CTC_PTR_VALID_CHECK(group_info);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: gid: %u \n", group_id);

    _sys_usw_acl_get_group_by_gid(lchip, group_id, &pg);
    SYS_ACL_CHECK_GROUP_UNEXIST(pg);

    pinfo = &(pg->group_info);

    /* get ctc group info based on pinfo (sys group info) */
    CTC_ERROR_RETURN(_sys_usw_acl_unmap_group_info(lchip, group_info, pinfo));

    return CTC_E_NONE;
}

#define _ACL_ENTRY_
/*
 * install entry to hardware table
 */
int32
sys_usw_acl_install_entry(uint8 lchip, uint32 eid)
{
    sys_acl_entry_t* pe;
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u \n", eid);

    SYS_USW_ACL_LOCK(lchip);
	 _sys_usw_acl_get_entry_by_eid(lchip, eid, &pe);
    if (!pe)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_install_entry(lchip, pe, SYS_ACL_ENTRY_OP_FLAG_ADD, 1));
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/*
 * uninstall entry from hardware table
 */
int32
sys_usw_acl_uninstall_entry(uint8 lchip, uint32 eid)
{
    sys_acl_entry_t* pe;
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u \n", eid);

    SYS_USW_ACL_LOCK(lchip);
    _sys_usw_acl_get_entry_by_eid(lchip, eid, &pe);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_install_entry(lchip, pe, SYS_ACL_ENTRY_OP_FLAG_DELETE, 1));
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field)
{
    /*check*/
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: eid %u -- key_field %u\n", entry_id, key_field->type);

    SYS_USW_ACL_LOCK(lchip);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_operate_key_field(lchip, entry_id, key_field, 1));
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field)
{
    sys_acl_entry_t* pe_lkup = NULL;
    /*check*/
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u -- key_field %u\n", entry_id, key_field->type);

    SYS_USW_ACL_LOCK(lchip);
    _sys_usw_acl_get_entry_by_eid(lchip, entry_id, &pe_lkup);

    if (!pe_lkup)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }
    /* hash entry not support remove key field */
    if (!ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_SUPPORT);
    }

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_operate_key_field(lchip, entry_id, key_field, 0));
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_add_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 is_half_ad = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_index = 0;
    ds_t   old_action = {0};
    sys_acl_entry_t* pe_lkup = NULL;
    int32 ret = 0;

    /*param check*/
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(action_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u -- action_field %u\n", entry_id, action_field->type);

    SYS_USW_ACL_LOCK(lchip);
    /*get entry and check*/
    _sys_usw_acl_get_entry_by_eid(lchip, entry_id, &pe_lkup);
    if (!pe_lkup)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    /*installed entry not support to add key field*/
    if(FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        /*1. rebuild buffer; if hash ad, copy buffer to new_buffer*/
        /*2. update action buffer(if hash, up new_buffer)*/
        _sys_usw_acl_get_table_id(lchip, pe_lkup, &key_id, &act_id, &hw_index);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_rebuild_buffer_from_hw(lchip, key_id, act_id, hw_index, pe_lkup));
        if(!ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
        {
            sal_memcpy(old_action, pe_lkup->buffer->action, sizeof(ds_t));
        }
    }

    if(ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
    {
        if( CTC_ACL_KEY_INTERFACE == pe_lkup->key_type )
        {
            is_half_ad = 1;
        }
        CTC_ERROR_GOTO(_sys_usw_acl_add_dsacl_field(lchip, action_field, pe_lkup, is_half_ad, 1), ret, error0);
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_acl_add_dsflow_field(lchip, action_field, pe_lkup, 1), ret, error0);
    }

    /*3. if entry is installed,
     *   for tcam, add action to hw, free buffer
     *   for hash, call add_hw, input old_action and new_action(to update spool)
    */
    if(FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_hw(lchip, pe_lkup, old_action));
    }
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
error0:
    if (pe_lkup->buffer && FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        mem_free(pe_lkup->buffer);
    }
    SYS_USW_ACL_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_acl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 is_half_ad = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_index = 0;
    ds_t   old_action = {0};
    sys_acl_entry_t* pe_lkup = NULL;

    /*param check*/
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(action_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u -- action_field %u\n", entry_id, action_field->type);

    SYS_USW_ACL_LOCK(lchip);
    /*get entry and check*/
    _sys_usw_acl_get_entry_by_eid(lchip, entry_id, &pe_lkup);
    if (!pe_lkup)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }
    /*installed entry not support to add key field*/
    if(FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        /* SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry installed \n");*/
	    /* return CTC_E_INVALID_CONFIG;*/

        /*1. rebuild buffer; if hash ad, copy buffer to new_buffer*/
        /*2. update action buffer(if hash, up new_buffer)*/
        _sys_usw_acl_get_table_id(lchip, pe_lkup, &key_id, &act_id, &hw_index);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_rebuild_buffer_from_hw(lchip, key_id, act_id, hw_index, pe_lkup));
        if(!ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
        {
            sal_memcpy(old_action, pe_lkup->buffer->action, sizeof(ds_t));
        }
    }

    if(ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
    {
        if(CTC_ACL_KEY_INTERFACE == pe_lkup->key_type)
        {
            is_half_ad = 1;
        }
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_dsacl_field(lchip, action_field, pe_lkup, is_half_ad, 0));
    }
    else
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_dsflow_field(lchip, action_field, pe_lkup, 0));
    }

    /*3. if entry is installed,
     *   for tcam, add action to hw, free buffer
     *   for hash, call add_hw, input old_action and new_action(to update spool)
    */
    if(FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_hw(lchip, pe_lkup, old_action));
    }
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
 * add entry
 */
int32
sys_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* p_ctc_entry)
{
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_ctc_entry);

    SYS_ACL_CHECK_GROUP_ID(group_id);
    CTC_MAX_VALUE_CHECK(p_ctc_entry->key_type, CTC_ACL_KEY_NUM - 1);
    CTC_MAX_VALUE_CHECK(p_ctc_entry->hash_field_sel_id, 0xF);
    CTC_MAX_VALUE_CHECK(p_ctc_entry->mode, 0x1);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: group_id %u --eid %u -- key_type %u\n",  group_id, p_ctc_entry->entry_id, p_ctc_entry->key_type);

    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_entry(lchip, group_id, p_ctc_entry));

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
 * remove entry from software table
 */
int32
sys_usw_acl_remove_entry(uint8 lchip, uint32 entry_id)
{
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u \n", entry_id);

    /* check raw entry */
    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_entry(lchip, entry_id));

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/*
 * remove all entries from a group
 */
int32
sys_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id)
{
    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_GROUP_ID(group_id);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: gid %u \n", group_id);

    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_all_entry(lchip, group_id));

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/*
 * set priority of entry
 */
int32
_sys_usw_acl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority)
{
    sys_acl_entry_t* pe = NULL;
    sys_acl_group_t* pg = NULL;
    sys_acl_block_t* pb = NULL;
    uint8 key_size = 0;

    /* get sys entry */
    _sys_usw_acl_get_nodes_by_eid(lchip, entry_id, &pe, &pg, &pb);
    if(!pe)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }
    if(!pg)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if (!ACL_ENTRY_IS_TCAM(pe->key_type)) /* hash entry */
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Hash entry has no priority \n");
		return CTC_E_INVALID_CONFIG;
    }

    /* tcam entry check pb */
    CTC_PTR_VALID_CHECK(pb);

    key_size = _sys_usw_acl_get_key_size(lchip, 0, pe, NULL);

    CTC_ERROR_RETURN(fpa_usw_set_entry_prio(p_usw_acl_master[lchip]->fpa, &pe->fpae, &pb->fpab, key_size, priority));

    return CTC_E_NONE;
}

int32
sys_usw_acl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority)
{
    SYS_ACL_INIT_CHECK();
    SYS_ACL_CHECK_ENTRY_PRIO(priority);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid %u -- prio %u \n", entry_id, priority);

    SYS_USW_ACL_LOCK(lchip);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_set_entry_priority(lchip, entry_id, priority));
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
 * get multiple entries
 */
int32
sys_usw_acl_get_multi_entry(uint8 lchip, ctc_acl_query_t* query)
{
    uint32* p_array = NULL;
    uint32                entry_index = 0;
    uint32                entry_count = 0;
    sys_acl_group_t       * pg        = NULL;
    struct ctc_slistnode_s* pe        = NULL;
    ctc_fpa_entry_t* fpe = NULL;
    uint16         block_idx = 0;
    uint16         start_idx = 0;
    uint16         end_idx = 0;
    uint8          step = 0;
    uint8          block_id = 0;
    sys_acl_block_t* pb = NULL;
    sys_acl_league_t* p_sys_league = NULL;

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(query);
    if(query->entry_size)
    {
        CTC_PTR_VALID_CHECK(query->entry_array);
    }

    SYS_ACL_DBG_FUNC();
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: mode-%u lkuplevel-%u gid-%u entry_sz-%u\n", query->query_mode, query->lkup_level, query->group_id, query->entry_size);

    SYS_USW_ACL_LOCK(lchip);
    switch (query->query_mode)
    {
        case CTC_ACL_QUERY_MODE_GROUPID:
        {
            if (query->group_id >= CTC_ACL_GROUP_ID_MAX)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Invalid group id \n");
                SYS_USW_ACL_RETURN_UNLOCK(CTC_E_BADID);
            }
            /* get group node */
            _sys_usw_acl_get_group_by_gid(lchip, query->group_id, &pg);
            SYS_USW_ACL_CHECK_GROUP_UNEXIST_UNLOCK(pg);
            if (query->entry_size == 0)
            {
                query->entry_count = pg->entry_count;
            }
            else
            {
                p_array = query->entry_array;
                if (NULL == p_array)
                {
                    SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PTR);
                }
                if (query->entry_size > pg->entry_count)
                {
                    query->entry_size = pg->entry_count;
                }

                CTC_SLIST_LOOP(pg->entry_list, pe)
                {
                    *p_array = ((sys_acl_entry_t *) pe)->fpae.entry_id;
                    p_array++;
                    entry_index++;
                    if (entry_index == query->entry_size)
                    {
                        break;
                    }
                }

                query->entry_count = query->entry_size;
            }
            break;
        }
        case CTC_ACL_QUERY_MODE_LKUP_LEVEL:
        {
            if ((query->lkup_level) > ((CTC_INGRESS == query->dir ? ACL_IGS_BLOCK_MAX_NUM-1: ACL_EGS_BLOCK_MAX_NUM-1)))
            {
                SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
            }

            block_id = ((query->dir == CTC_INGRESS) ? query->lkup_level: query->lkup_level + ACL_IGS_BLOCK_MAX_NUM);
            pb = &p_usw_acl_master[lchip]->block[block_id];
            p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);

            if (!p_sys_league->lkup_level_bitmap)
            {
                block_id = ((query->dir == CTC_INGRESS) ? p_sys_league->merged_to: p_sys_league->merged_to + ACL_IGS_BLOCK_MAX_NUM);
                pb = &p_usw_acl_master[lchip]->block[block_id];
                p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);
            }

            start_idx = p_sys_league->lkup_level_start[query->lkup_level];
            end_idx = p_sys_league->lkup_level_start[query->lkup_level] + p_sys_league->lkup_level_count[query->lkup_level];

            step  = 1;
            p_array = query->entry_array;
            for (block_idx = start_idx; block_idx < end_idx; block_idx = block_idx + step)
            {
                if((block_idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_640]) && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640]))      /*640 bit key*/
                {
                    step = 8;
                }
                else if(block_idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_320] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320]))   /*320 bit key*/
                {
                    step = 4;
                }
                else if(block_idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_160] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160]))   /*160 bit key*/
                {
                    step = 2;
                }
                else                                            /*80 bit key*/
                {
                    step = 1;
                }

                fpe = pb->fpab.entries[block_idx];
                if (fpe)
                {
                    if (query->entry_size == 0)
                    {
                        entry_count++;
                    }
                    else
                    {
                        if (NULL == p_array)
                        {
                            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PTR);
                        }
                        *p_array = fpe->entry_id;
                        p_array++;
                        entry_count++;

                        if (entry_count == query->entry_size)
                        {
                            break;
                        }
                    }
                }
            }
            query->entry_count = entry_count;
            break;
        }
        default:
        {
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
        }
    }
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}
int32
sys_usw_acl_copy_entry(uint8 lchip, ctc_acl_copy_entry_t* copy_entry)
{
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_index = 0;
    sys_acl_entry_t * pe_src    = NULL;
    sys_acl_entry_t * pe_dst    = NULL;
    sys_acl_buffer_t* p_buffer  = NULL;
    sys_acl_block_t * pb_dst    = NULL;
    sys_acl_group_t * pg_dst    = NULL;
    sys_acl_vlan_edit_t* pv_get = NULL;
    uint8           block_id    = 0;
    uint32          block_index = 0;
    uint8           key_size    = 0;
    int32           ret         = 0;
    uint8           range_type  = 0;
    uint8           truncation_profile_id = 0;
    sys_cpu_reason_info_t cpu_rason_info;

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(copy_entry);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_ACL_LOCK(lchip);
    /* check src entry */
    _sys_usw_acl_get_entry_by_eid(lchip, copy_entry->src_entry_id, &pe_src);
    if (!pe_src)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (!ACL_ENTRY_IS_TCAM(pe_src->key_type)) /* hash entry not support copy */
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Hash acl entry not support copy \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_SUPPORT);
    }

    /* check dst entry */
    _sys_usw_acl_get_entry_by_eid(lchip, copy_entry->dst_entry_id, &pe_dst);

    if (pe_dst)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry exist \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_EXIST);
    }

    _sys_usw_acl_get_group_by_gid(lchip, copy_entry->dst_group_id, &pg_dst);
    SYS_USW_ACL_CHECK_GROUP_UNEXIST_UNLOCK(pg_dst);

    if (pg_dst->group_info.dir!=pe_src->group->group_info.dir)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Src group direction is not the same as dst group direction \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    if (pe_src->nexthop_id)
    {
        uint32 fwdptr = 0;
        CTC_ERROR_GOTO(sys_usw_nh_get_dsfwd_offset(lchip, pe_src->nexthop_id, &fwdptr, 0), ret, cleanup_0);
    }

    MALLOC_ZERO(MEM_ACL_MODULE, pe_dst, sizeof(sys_acl_entry_t));
    if (!pe_dst)
    {
       ret = CTC_E_NO_MEMORY;
       goto cleanup_0;
    }
	sal_memcpy(pe_dst,pe_src,sizeof(sys_acl_entry_t));
	/*the pointer need re-assign value*/
    pe_dst->group = pg_dst;
	pe_dst->hash_action = NULL;
	pe_dst->copp_ptr = 0;
	pe_dst->fpae.entry_id    = copy_entry->dst_entry_id;
    pe_dst->head.next        = NULL;
	pe_dst->vlan_edit        = NULL;
	pe_dst->vlan_edit_valid  = 0;

	key_size = _sys_usw_acl_get_key_size(lchip, 0, pe_dst, NULL);
    block_id = pg_dst->group_info.block_id;
    if(CTC_EGRESS == pg_dst->group_info.dir)
    {
        block_id += p_usw_acl_master[lchip]->igs_block_num;
    }
    pb_dst = &p_usw_acl_master[lchip]->block[block_id];

    /* get block index, based on priority */
    CTC_ERROR_GOTO(fpa_usw_alloc_offset(p_usw_acl_master[lchip]->fpa, &(pb_dst->fpab), key_size,
                                    pe_src->fpae.priority, &block_index), ret, cleanup_1);
    pe_dst->fpae.offset_a = block_index;

    MALLOC_ZERO(MEM_ACL_MODULE, p_buffer, sizeof(sys_acl_buffer_t));
    if (!p_buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto cleanup_2;
    }
    pe_dst->buffer = p_buffer;

    if(FPA_ENTRY_FLAG_INSTALLED == pe_src->fpae.flag)
    {
       _sys_usw_acl_get_table_id(lchip, pe_src, &key_id, &act_id, &hw_index);
       CTC_ERROR_GOTO(_sys_usw_acl_rebuild_buffer_from_hw(lchip, key_id, act_id, hw_index, pe_src), ret, cleanup_3);
    }
    sal_memcpy(p_buffer, pe_src->buffer, sizeof(sys_acl_buffer_t));
    if(FPA_ENTRY_FLAG_INSTALLED == pe_src->fpae.flag && pe_src->buffer)
    {
       mem_free(pe_src->buffer);
    }

    for(range_type = ACL_RANGE_TYPE_PKT_LENGTH; range_type < ACL_RANGE_TYPE_NUM; range_type++)
    {
        if(CTC_IS_BIT_SET(pe_dst->rg_info.flag, range_type))
        {
            (p_usw_acl_master[lchip]->field_range.range[pe_dst->rg_info.range_id[range_type]].ref)++;
        }
    }

    if(pe_src->vlan_edit_valid)
    {
        CTC_ERROR_GOTO(ctc_spool_add(p_usw_acl_master[lchip]->vlan_edit_spool, pe_src->vlan_edit, NULL, &pv_get), ret, cleanup_3);
        pe_dst->vlan_edit = pv_get;
        pe_dst->vlan_edit_valid = 1;
    }

    if(pe_src->ether_type)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Acl] Copy entry add compress ether type: %X \n", pe_src->ether_type);
        CTC_ERROR_GOTO(sys_usw_register_add_compress_ether_type(lchip, pe_dst->ether_type, 0, NULL, &pe_dst->ether_type_index), ret, cleanup_4);
    }

    sal_memset(&cpu_rason_info, 0, sizeof(sys_cpu_reason_info_t));
    if(pe_src->cpu_reason_id)
    {
        cpu_rason_info.reason_id = pe_dst->cpu_reason_id;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Acl] Copy entry add cpu reason id: %u \n", pe_src->cpu_reason_id);
        CTC_ERROR_GOTO(sys_usw_cpu_reason_alloc_exception_index(lchip, pg_dst->group_info.dir, &cpu_rason_info), ret, cleanup_5);
    }

    if (CTC_BMP_ISSET(pe_dst->action_bmp, CTC_ACL_FIELD_ACTION_TRUNCATED_LEN))
    {
        ctc_acl_field_action_t field_action;
        sal_memset(&field_action, 0, sizeof(ctc_acl_field_action_t));
        field_action.type = CTC_ACL_FIELD_ACTION_TRUNCATED_LEN;
        if (p_usw_acl_master[lchip]->get_ad_func[pe_dst->action_type])
        {
            CTC_ERROR_GOTO(p_usw_acl_master[lchip]->get_ad_func[pe_dst->action_type](lchip, &field_action, pe_dst), ret, cleanup_6);
        }
        CTC_ERROR_GOTO(sys_usw_register_add_truncation_profile(lchip, field_action.data0, 0, &truncation_profile_id), ret, cleanup_6);
    }

    /* add to hash */
    if(!ctc_hash_insert(p_usw_acl_master[lchip]->entry, pe_dst))
    {
    	ret = CTC_E_NO_MEMORY;
	     goto cleanup_7;
    }
    /* add to group */
    if(!ctc_slist_add_head(pg_dst->entry_list, &(pe_dst->head)))
    {
    	ret = CTC_E_NO_MEMORY;
	    goto cleanup_8;
    }
    /* mark flag */
    pe_dst->fpae.flag = FPA_ENTRY_FLAG_UNINSTALLED;

    /* add to block */
    pb_dst->fpab.entries[block_index] = &pe_dst->fpae;

    /*build group info*/
    _sys_usw_acl_add_key_common_field_group_none(lchip, pe_dst);
    _sys_usw_acl_add_group_key_port_field(lchip, pe_dst->fpae.entry_id,pe_dst->group);

    /* set new priority */
    CTC_ERROR_GOTO(_sys_usw_acl_set_entry_priority(lchip, copy_entry->dst_entry_id, pe_src->fpae.priority),ret,cleanup_9);

    (pg_dst->entry_count)++;
    _sys_usw_acl_update_key_count(lchip, pe_dst, 1);
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
 cleanup_9:
     ctc_slist_delete_node(pg_dst->entry_list, &(pe_dst->head));
 cleanup_8:
     ctc_hash_remove(p_usw_acl_master[lchip]->entry, pe_dst);
 cleanup_7:
    if (CTC_BMP_ISSET(pe_dst->action_bmp, CTC_ACL_FIELD_ACTION_TRUNCATED_LEN))
    {
        sys_usw_register_remove_truncation_profile(lchip, 1, truncation_profile_id);
    }
 cleanup_6:
    if (pe_dst->cpu_reason_id)
    {
        sys_usw_cpu_reason_free_exception_index(lchip, pg_dst->group_info.dir, &cpu_rason_info);
    }
 cleanup_5:
    if (pe_dst->ether_type)
    {
        sys_usw_register_remove_compress_ether_type(lchip, pe_dst->ether_type);
    }
 cleanup_4:
    if (pe_src->vlan_edit_valid)
    {
        ctc_spool_remove(p_usw_acl_master[lchip]->vlan_edit_spool, pe_dst->vlan_edit, NULL);
    }
 cleanup_3:
    if(pe_dst->buffer)
    {
        mem_free(pe_dst->buffer)
    }
	for(range_type = ACL_RANGE_TYPE_PKT_LENGTH; range_type < ACL_RANGE_TYPE_NUM; range_type++)
    {
        if(CTC_IS_BIT_SET(pe_dst->rg_info.flag, range_type))
        {
            (p_usw_acl_master[lchip]->field_range.range[pe_dst->rg_info.range_id[range_type]].ref)--;
            (p_usw_acl_master[lchip]->field_range.free_num)++;
        }
    }
cleanup_2:
	  fpa_usw_free_offset(&(pb_dst->fpab),pe_dst->fpae.offset_a);
cleanup_1:
    if(pe_dst)
    {
        mem_free(pe_dst);
    }
cleanup_0:
    SYS_USW_ACL_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_acl_get_hash_field_sel(uint8 lchip, uint8 hash_key_type, uint8 field_sel_id,
                                                                        uint16 key_field_type, uint8* o_value )
{
    ds_t   ds_sel;
    uint8  value = 0;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(o_value);
    CTC_MAX_VALUE_CHECK(field_sel_id, 0xF);

    sal_memset(&ds_sel, 0, sizeof(ds_sel));
    if (CTC_ACL_KEY_HASH_MAC == hash_key_type)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, field_sel_id, &ds_sel));
        switch(key_field_type)
        {
        case CTC_FIELD_KEY_PORT:
                value = GetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel);
               if(ACL_HASH_GPORT_TYPE_GPORT == value)
               {
                    value = CTC_FIELD_PORT_TYPE_GPORT;
               }
               else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == value)
               {
                    value = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
               }
            break;
        case CTC_FIELD_KEY_METADATA:
            value = GetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel);
            if(ACL_HASH_GPORT_TYPE_METADATA != value)
            {
                value = 0;
            }
            break;

        case CTC_FIELD_KEY_ETHER_TYPE:
            value = GetFlowL2HashFieldSelect(V, etherTypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MAC_DA:
            value = GetFlowL2HashFieldSelect(V, macDaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MAC_SA:
            value = GetFlowL2HashFieldSelect(V, macSaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_VALID:
            value = GetFlowL2HashFieldSelect(V, vlanIdValidEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_SVLAN_ID:
            value = GetFlowL2HashFieldSelect(V, vlanIdEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_COS:
            value = GetFlowL2HashFieldSelect(V, cosEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_CFI:
            value = GetFlowL2HashFieldSelect(V, cfiEn_f, &ds_sel);
            break;

        default:
            value = 0;
            break;
        }

    }
    else if (CTC_ACL_KEY_HASH_IPV4 == hash_key_type)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, field_sel_id, &ds_sel));
        switch(key_field_type)
        {
        case CTC_FIELD_KEY_PORT:
            value = GetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel);
           if(ACL_HASH_GPORT_TYPE_GPORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_GPORT;
           }
           else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
           }
        break;

        case CTC_FIELD_KEY_METADATA:
            value = GetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel);
            if(ACL_HASH_GPORT_TYPE_METADATA != value)
            {
                value = 0;
            }
            break;

        case CTC_FIELD_KEY_IP_DSCP:
            value = GetFlowL3Ipv4HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_PRECEDENCE:
            value = GetFlowL3Ipv4HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_ECN:
            value = GetFlowL3Ipv4HashFieldSelect(V, ecnEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_TYPE:
            value = GetFlowL3Ipv4HashFieldSelect(V, layer4TypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_SRC_PORT:
            value = GetFlowL3Ipv4HashFieldSelect(V, l4SourcePortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
            value = GetFlowL3Ipv4HashFieldSelect(V, l4DestPortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_DA:
            value = GetFlowL3Ipv4HashFieldSelect(V, ipDaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_SA:
            value = GetFlowL3Ipv4HashFieldSelect(V, ipSaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_TYPE:
            value = GetFlowL3Ipv4HashFieldSelect(V, icmpTypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_CODE:
            value = GetFlowL3Ipv4HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_VN_ID:
            value = GetFlowL3Ipv4HashFieldSelect(V, vxlanVniEn_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            value = GetFlowL3Ipv4HashFieldSelect(V, greKeyEn_f, &ds_sel);
            break;
        default:
            value = 0;
            break;
        }

    }
    else if (CTC_ACL_KEY_HASH_MPLS == hash_key_type)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, field_sel_id, &ds_sel));
        switch(key_field_type)
        {
        case CTC_FIELD_KEY_PORT:
            value = GetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel);
           if(ACL_HASH_GPORT_TYPE_GPORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_GPORT;
           }
           else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
           }
        break;

        case CTC_FIELD_KEY_METADATA:
            value = GetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel);
            if(ACL_HASH_GPORT_TYPE_METADATA != value)
            {
                value = 0;
            }
            break;

        case CTC_FIELD_KEY_LABEL_NUM:
            value = GetFlowL3MplsHashFieldSelect(V, labelNumEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL0:
            value = GetFlowL3MplsHashFieldSelect(V, mplsLabel0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL1:
            value = GetFlowL3MplsHashFieldSelect(V, mplsLabel1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL2:
            value = GetFlowL3MplsHashFieldSelect(V, mplsLabel2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP0:
            value = GetFlowL3MplsHashFieldSelect(V, mplsExp0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP1:
            value = GetFlowL3MplsHashFieldSelect(V, mplsExp1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP2:
            value = GetFlowL3MplsHashFieldSelect(V, mplsExp2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT0:
            value = GetFlowL3MplsHashFieldSelect(V, mplsSbit0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT1:
            value = GetFlowL3MplsHashFieldSelect(V, mplsSbit1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT2:
            value = GetFlowL3MplsHashFieldSelect(V, mplsSbit2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL0:
            value = GetFlowL3MplsHashFieldSelect(V, mplsTtl0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL1:
            value = GetFlowL3MplsHashFieldSelect(V, mplsTtl1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL2:
            value = GetFlowL3MplsHashFieldSelect(V, mplsTtl2En_f, &ds_sel);
            break;

        default:
            value = 0;
            break;
        }

    }
    else if (CTC_ACL_KEY_HASH_IPV6 == hash_key_type)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, field_sel_id, &ds_sel));
        switch(key_field_type)
        {
        case CTC_FIELD_KEY_PORT:
            value = GetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel);
           if(ACL_HASH_GPORT_TYPE_GPORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_GPORT;
           }
           else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
           }
        break;

        case CTC_FIELD_KEY_METADATA:
            value = GetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel);
            if(ACL_HASH_GPORT_TYPE_METADATA != value)
            {
                value = 0;
            }
            break;

        case CTC_FIELD_KEY_IP_DSCP:
            value = GetFlowL3Ipv6HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_PRECEDENCE:
            value = GetFlowL3Ipv6HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_ECN:
            value = GetFlowL3Ipv6HashFieldSelect(V, ecnEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_TYPE:
            value = GetFlowL3Ipv6HashFieldSelect(V, layer4TypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_SRC_PORT:
            value = GetFlowL3Ipv6HashFieldSelect(V, l4SourcePortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
            value = GetFlowL3Ipv6HashFieldSelect(V, l4DestPortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IPV6_DA:
            value = GetFlowL3Ipv6HashFieldSelect(V, ipDaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IPV6_SA:
            value = GetFlowL3Ipv6HashFieldSelect(V, ipSaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_TYPE:
            value = GetFlowL3Ipv6HashFieldSelect(V, icmpTypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_CODE:
            value = GetFlowL3Ipv6HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_VN_ID:
            value = GetFlowL3Ipv6HashFieldSelect(V, vxlanVniEn_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            value = GetFlowL3Ipv6HashFieldSelect(V, greKeyEn_f, &ds_sel);
            break;
        default:
            value = 0;
            break;
        }

    }
    else if (CTC_ACL_KEY_HASH_L2_L3 == hash_key_type)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, field_sel_id, &ds_sel));
        switch(key_field_type)
        {
        case CTC_FIELD_KEY_PORT:
            value = GetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel);
           if(ACL_HASH_GPORT_TYPE_GPORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_GPORT;
           }
           else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == value)
           {
                value = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
           }
        break;

        case CTC_FIELD_KEY_METADATA:
            value = GetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel);
            if(ACL_HASH_GPORT_TYPE_METADATA != value)
            {
                value = 0;
            }
            break;

        case CTC_FIELD_KEY_MAC_DA:
            value = GetFlowL2L3HashFieldSelect(V, macDaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MAC_SA:
            value = GetFlowL2L3HashFieldSelect(V, macSaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_VALID:
            value = GetFlowL2L3HashFieldSelect(V, svlanIdValidEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_SVLAN_ID:
            value = GetFlowL2L3HashFieldSelect(V, svlanIdEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_COS:
            value = GetFlowL2L3HashFieldSelect(V, stagCosEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_STAG_CFI:
            value = GetFlowL2L3HashFieldSelect(V, stagCfiEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_CTAG_VALID:
            value = GetFlowL2L3HashFieldSelect(V, cvlanIdValidEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_CVLAN_ID:
            value = GetFlowL2L3HashFieldSelect(V, cvlanIdEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_CTAG_COS:
            value = GetFlowL2L3HashFieldSelect(V, ctagCosEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_CTAG_CFI:
            value = GetFlowL2L3HashFieldSelect(V, ctagCfiEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L3_TYPE:
            value = GetFlowL2L3HashFieldSelect(V,  layer3TypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_TYPE:
            value = GetFlowL2L3HashFieldSelect(V, layer4TypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_DSCP:
            value = GetFlowL2L3HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_PRECEDENCE:
            value = GetFlowL2L3HashFieldSelect(V, dscpEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_ECN:
            value = GetFlowL2L3HashFieldSelect(V, ecnEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_SRC_PORT:
            value = GetFlowL2L3HashFieldSelect(V, l4SourcePortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
            value = GetFlowL2L3HashFieldSelect(V, l4DestPortEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_DA:
            value = GetFlowL2L3HashFieldSelect(V, ipDaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_SA:
            value = GetFlowL2L3HashFieldSelect(V, ipSaEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_IP_TTL:
            value = GetFlowL2L3HashFieldSelect(V, ttlEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_TYPE:
            value = GetFlowL2L3HashFieldSelect(V, icmpTypeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_ICMP_CODE:
            value = GetFlowL2L3HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_LABEL_NUM:
            value = GetFlowL2L3HashFieldSelect(V, labelNumEn_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL0:
            value = GetFlowL2L3HashFieldSelect(V, mplsLabel0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL1:
            value = GetFlowL2L3HashFieldSelect(V, mplsLabel1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_LABEL2:
            value = GetFlowL2L3HashFieldSelect(V, mplsLabel2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP0:
            value = GetFlowL2L3HashFieldSelect(V, mplsExp0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP1:
            value = GetFlowL2L3HashFieldSelect(V, mplsExp1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_EXP2:
            value = GetFlowL2L3HashFieldSelect(V, mplsExp2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT0:
            value = GetFlowL2L3HashFieldSelect(V, mplsSbit0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT1:
            value = GetFlowL2L3HashFieldSelect(V, mplsSbit1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_SBIT2:
            value = GetFlowL2L3HashFieldSelect(V, mplsSbit2En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL0:
            value = GetFlowL2L3HashFieldSelect(V, mplsTtl0En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL1:
            value = GetFlowL2L3HashFieldSelect(V, mplsTtl1En_f, &ds_sel);
            break;

        case CTC_FIELD_KEY_MPLS_TTL2:
            value = GetFlowL2L3HashFieldSelect(V, mplsTtl2En_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_VN_ID:
            value = GetFlowL2L3HashFieldSelect(V, vxlanVniEn_f, &ds_sel);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            value = GetFlowL2L3HashFieldSelect(V, greKeyEn_f, &ds_sel);
            break;
        default:
            value = 0;
            break;
        }

    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if(o_value)
    {
        *o_value = value;
    }

    return CTC_E_NONE;
}

int32
sys_usw_acl_map_ctc_to_sys_hash_key_type(uint8 key_type ,uint8 *o_key_type)
{
    uint8 key_type_tmp;
    switch(key_type)
    {
    case CTC_ACL_KEY_HASH_MAC:
        key_type_tmp = SYS_ACL_HASH_KEY_TYPE_MAC;
        break;
    case CTC_ACL_KEY_HASH_IPV4:
        key_type_tmp = SYS_ACL_HASH_KEY_TYPE_IPV4;
        break;
    case CTC_ACL_KEY_HASH_L2_L3:
        key_type_tmp = SYS_ACL_HASH_KEY_TYPE_L2_L3;
        break;
    case CTC_ACL_KEY_HASH_IPV6:
        key_type_tmp = SYS_ACL_HASH_KEY_TYPE_IPV6;
        break;
    case CTC_ACL_KEY_HASH_MPLS:
        key_type_tmp = SYS_ACL_HASH_KEY_TYPE_MPLS;
        break;
    default:
        return CTC_E_INVALID_PARAM;
       break;
    }
    *o_key_type = key_type_tmp;
    return CTC_E_NONE;
}

int32
sys_usw_acl_set_field_to_hash_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id,
                                                           ctc_field_key_t* sel_field)
{
    uint8 sys_key_type = 0;
    int32 ret = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_ACL_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(field_sel_id, SYS_ACL_HASH_SEL_PROFILE_MAX-1);
    CTC_PTR_VALID_CHECK(sel_field);

    CTC_ERROR_RETURN(sys_usw_acl_map_ctc_to_sys_hash_key_type(key_type,&sys_key_type));
    SYS_USW_ACL_LOCK(lchip);
    if(0 != p_usw_acl_master[lchip]->hash_sel_profile_count[sys_key_type][field_sel_id])
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Already in used \n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_IN_USE);
    }
    ret = _sys_usw_acl_check_hash_sel_field_union(lchip, sys_key_type, field_sel_id,sel_field);
    if(ret!= CTC_E_NONE )
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "check_hash_sel_field_union failed\n");
        SYS_USW_ACL_RETURN_UNLOCK(sel_field->data?ret:CTC_E_NONE);
    }

    switch(key_type)
    {
    case CTC_ACL_KEY_HASH_MAC:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_add_hash_mac_sel_field(lchip, field_sel_id, sel_field, sel_field->data));
        break;

    case CTC_ACL_KEY_HASH_IPV4:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_add_hash_ipv4_sel_field(lchip, field_sel_id, sel_field, sel_field->data));
        break;

    case CTC_ACL_KEY_HASH_IPV6:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_add_hash_ipv6_sel_field(lchip, field_sel_id, sel_field, sel_field->data));
        break;

    case CTC_ACL_KEY_HASH_L2_L3:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_add_hash_l2l3_sel_field(lchip, field_sel_id, sel_field, sel_field->data));
        break;

    case CTC_ACL_KEY_HASH_MPLS:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(
            _sys_usw_acl_add_hash_mpls_sel_field(lchip, field_sel_id, sel_field, sel_field->data));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_set_hash_sel_field_union(lchip, sys_key_type, field_sel_id,sel_field, sel_field->data));
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_ftm_acl_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    uint8 start = 0;
    uint8 end = 0;
    uint8 block_id = 0;
    
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_ACL_INIT_CHECK();

    if(DRV_IS_DUET2(lchip))
    {
        specs_info->used_size = (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_80])
                                + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_160]*2)
                                + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_320]*4)
                                + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_640]*8);
        return CTC_E_NONE;
    }

    if(specs_info->type == 0)
    {
        start = 0;
        end = p_usw_acl_master[lchip]->igs_block_num;
    }
    else
    {
        start = p_usw_acl_master[lchip]->igs_block_num;
        end = p_usw_acl_master[lchip]->igs_block_num+p_usw_acl_master[lchip]->egs_block_num;
    }

    for (block_id = start; block_id < end; block_id++)
    {       
        specs_info->used_size +=  (p_usw_acl_master[lchip]->block[block_id].fpab.entry_count-p_usw_acl_master[lchip]->block[block_id].fpab.free_count);
    }

    return CTC_E_NONE;
}


int32
sys_usw_acl_ftm_acl_flow_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_ACL_INIT_CHECK();

    specs_info->used_size = (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_HASH]);

    return CTC_E_NONE;
}
/*special process for mapping old API*/
int32 sys_usw_acl_add_port_field(uint8 lchip, uint32 entry_id, ctc_field_port_t port)
{
    ctc_field_key_t   key_field;
    ctc_field_port_t  port_mask;
    sys_acl_entry_t   sys_entry;
    sys_acl_entry_t * pe_lkup = NULL;

    SYS_ACL_INIT_CHECK();

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    SYS_USW_ACL_LOCK(lchip);

    sal_memset(&sys_entry, 0, sizeof(sys_acl_entry_t));
    sys_entry.fpae.entry_id = entry_id;
    sys_entry.fpae.lchip= lchip;

    pe_lkup = ctc_hash_lookup(p_usw_acl_master[lchip]->entry, &sys_entry);
    if(!pe_lkup || !pe_lkup->group)
    {
        SYS_USW_ACL_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if(CTC_ACL_GROUP_TYPE_NONE == pe_lkup->group->group_info.type
        && port.type != CTC_FIELD_PORT_TYPE_NONE)
    {
        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &(port);
        key_field.ext_mask = &port_mask;

        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_operate_key_field(lchip, entry_id, &key_field, 1));
    }

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}
#define _ACL_CID_PAIR_
int32
sys_usw_acl_add_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(cid_pair);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(cid_pair->action_mode, CTC_ACL_CID_PAIR_ACTION_OVERWRITE_ACL);

    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID))
    {
        if(cid_pair->src_cid > CTC_ACL_UNKNOWN_CID )
        {
            return CTC_E_BADID;
        }
    }
    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID))
    {
        if(cid_pair->dst_cid > CTC_ACL_UNKNOWN_CID )
        {
            return CTC_E_BADID;
        }
    }
    if((CTC_ACL_CID_PAIR_ACTION_OVERWRITE_ACL == cid_pair->action_mode)&&(cid_pair->acl_prop.acl_en))
    {
        CTC_MAX_VALUE_CHECK(cid_pair->acl_prop.acl_priority, 0x07);
    }

    SYS_USW_ACL_LOCK(lchip);

    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_FLEX))    /*tcam*/
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_tcam_cid_pair(lchip, cid_pair));
    }
    else      /*hash*/
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_hash_cid_pair(lchip, cid_pair));
    }

    SYS_USW_ACL_UNLOCK(lchip);


    return CTC_E_NONE;
}

int32
sys_usw_acl_remove_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    CTC_PTR_VALID_CHECK(cid_pair);
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_ACL_LOCK(lchip);

    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_FLEX))    /*tcam*/
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_tcam_cid_pair(lchip, cid_pair));
    }
    else   /*hash*/
    {
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_hash_cid_pair(lchip, cid_pair));
    }
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_set_cid_priority(uint8 lchip, uint8 is_src_cid, uint8* p_prio_arry)
{
    uint8 prio_max_value = is_src_cid? 0x7: 0x3;

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prio_arry);
    CTC_MAX_VALUE_CHECK(p_prio_arry[0], prio_max_value);
    CTC_MAX_VALUE_CHECK(p_prio_arry[1], prio_max_value);
    CTC_MAX_VALUE_CHECK(p_prio_arry[2], prio_max_value);
    CTC_MAX_VALUE_CHECK(p_prio_arry[3], prio_max_value);
    CTC_MAX_VALUE_CHECK(p_prio_arry[4], 0x7);
    CTC_MAX_VALUE_CHECK(p_prio_arry[5], 0x7);
    CTC_MAX_VALUE_CHECK(p_prio_arry[6], 0x7);

    CTC_ERROR_RETURN(_sys_usw_acl_set_cid_priority(lchip, is_src_cid, p_prio_arry));

    return CTC_E_NONE;
}

#define _ACL_UDF_ENTRY_
STATIC uint8
sys_usw_acl_map_ctc_to_sys_offset_type(uint8 offset_type)
{
    uint8 ret = 0;
    switch(offset_type)
    {
        case CTC_ACL_UDF_OFFSET_L2 :
            ret = 0;
            break;
        case CTC_ACL_UDF_OFFSET_L3 :
            ret = 1;
            break;
        case CTC_ACL_UDF_OFFSET_L4 :
            ret = 2;
            break;
        default :
            break;
    }
    return ret;
}

STATIC uint8
sys_usw_acl_map_sys_to_ctc_offset_type(uint8 offset_type)
{
    uint8 ret = CTC_ACL_UDF_TYPE_NONE;
    switch(offset_type)
    {
        case 0 :
            ret = CTC_ACL_UDF_OFFSET_L2;
            break;
        case 1 :
            ret = CTC_ACL_UDF_OFFSET_L3;
            break;
        case 2 :
            ret = CTC_ACL_UDF_OFFSET_L4;
            break;
        default :
            break;
    }
    return ret;
}

int32
sys_usw_acl_get_udf_info(uint8 lchip,  uint32 udf_id, sys_acl_udf_entry_t** p_udf_entry)
{
    SYS_USW_ACL_LOCK(lchip);
    _sys_usw_acl_get_udf_info(lchip,  udf_id, p_udf_entry);
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_get_udf_entry(uint8 lchip, ctc_acl_classify_udf_t* p_udf_entry)
{
	sys_acl_udf_entry_t    *p_sys_udf_entry;
	ParserUdfCamResult_m udf_rst;
	ParserUdfCam_m udf_cam;
    uint8 offset_type = 0;
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_udf_entry);
	CTC_MAX_VALUE_CHECK(p_udf_entry->priority, 0xF);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"get_udf_entry, query_type:%d,udf_id: %u,priority:%d \n",p_udf_entry->query_type, p_udf_entry->udf_id,p_udf_entry->priority);

    SYS_USW_ACL_LOCK(lchip);
	if(!p_udf_entry->query_type)
	{
        _sys_usw_acl_get_udf_info(lchip,  p_udf_entry->udf_id, &p_sys_udf_entry);
	}
	else
	{
        p_sys_udf_entry =&p_usw_acl_master[lchip]->udf_entry[p_udf_entry->priority];
	}
	if(!p_sys_udf_entry || !p_sys_udf_entry->key_index_used)
	{
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
	}
    p_udf_entry->udf_id = p_sys_udf_entry->udf_id;
	p_udf_entry->offset_num = p_sys_udf_entry->udf_offset_num;
	p_udf_entry->priority = SYS_ACL_UDF_IDX_2_PRI(p_sys_udf_entry->key_index);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_parser_io_get_parser_udf_cam_result(lchip,&udf_rst,p_sys_udf_entry->key_index));
    p_udf_entry->tcp_option_en = GetParserUdfCamResult(V, skipTcpOption_f, &udf_rst);
    p_udf_entry->offset[0] = GetParserUdfCamResult(V, udfEntryOffset0_f, &udf_rst) << 2;
	p_udf_entry->offset[1] = GetParserUdfCamResult(V, udfEntryOffset1_f, &udf_rst) << 2;
	p_udf_entry->offset[2] = GetParserUdfCamResult(V, udfEntryOffset2_f, &udf_rst) << 2;
	p_udf_entry->offset[3] = GetParserUdfCamResult(V, udfEntryOffset3_f, &udf_rst) << 2;
    if (DRV_FLD_IS_EXISIT(ParserUdfCamResult_t, ParserUdfCamResult_udfStartPosType_f))
    {
        offset_type = GetParserUdfCamResult(V, udfStartPosType_f, &udf_rst);
        p_udf_entry->offset_type = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
    }
    else
    {
        offset_type = GetParserUdfCamResult(V, udfStartPosType0_f, &udf_rst);
        p_udf_entry->offset_type_array[0] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
        offset_type = GetParserUdfCamResult(V, udfStartPosType1_f, &udf_rst);
        p_udf_entry->offset_type_array[1] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
        offset_type = GetParserUdfCamResult(V, udfStartPosType2_f, &udf_rst);
        p_udf_entry->offset_type_array[2] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
        offset_type = GetParserUdfCamResult(V, udfStartPosType3_f, &udf_rst);
        p_udf_entry->offset_type_array[3] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
    }

	SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_parser_io_get_parser_udf_cam(lchip, &udf_cam,p_sys_udf_entry->key_index));
    p_udf_entry->valid = GetParserUdfCam(V, entryValid_f, &udf_cam);

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}
int32
sys_usw_acl_get_udf_hit_index(uint8 lchip,  uint32 udf_id, uint8* udf_hit_index)
{
    sys_acl_udf_entry_t *p_udf_entry;
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: udf_id %u \n", udf_id);
    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_get_udf_info(lchip, udf_id, &p_udf_entry));
    if(p_udf_entry == NULL)
    {
        SYS_USW_ACL_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    *udf_hit_index = p_udf_entry->udf_hit_index;

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}
int32
sys_usw_acl_add_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* p_udf_entry)
{
    sys_acl_udf_entry_t    *p_sys_udf_entry = NULL, *p_sys_udf_entry2 = NULL;
    ParserUdfCam_m udf_cam;
    ParserUdfCamResult_m udf_rst;
    uint8 offset_type = 0;
    sys_usw_opf_t   opf;
    uint32 offset = 0;
    uint8  remainder = 0;
    uint8  is_update = 0;
    uint8  udf_type_none[CTC_ACL_UDF_FIELD_NUM] = {CTC_ACL_UDF_TYPE_NONE};
    int    ret = 0;

    sal_memset(&opf, 0, sizeof(opf));

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_udf_entry);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"acl_add_udf_entry,udf_id: %u,priority:%d \n", p_udf_entry->udf_id,p_udf_entry->priority);

    CTC_MAX_VALUE_CHECK(p_udf_entry->priority, 0xF);
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_type, CTC_ACL_UDF_TYPE_NUM-1);
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_type_array[0], CTC_ACL_UDF_TYPE_NUM-1);
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_type_array[1], CTC_ACL_UDF_TYPE_NUM-1);
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_type_array[2], CTC_ACL_UDF_TYPE_NUM-1);
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_type_array[3], CTC_ACL_UDF_TYPE_NUM-1);

    remainder = (p_udf_entry->offset[0] % 4) || (p_udf_entry->offset[1] % 4) || (p_udf_entry->offset[2] % 4) || (p_udf_entry->offset[3] % 4);
    if(remainder)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The offset :offset[0]:%d,offset[1]:%d,offset[2]:%d,offset[3]:%d\n", p_udf_entry->offset[0],p_udf_entry->offset[1],p_udf_entry->offset[2],p_udf_entry->offset[3]);
        return CTC_E_INVALID_PARAM;
    }

    CTC_MAX_VALUE_CHECK(p_udf_entry->offset[0]/4, MCHIP_CAP(SYS_CAP_ACL_UDF_OFFSET_MAX));
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset[1]/4, MCHIP_CAP(SYS_CAP_ACL_UDF_OFFSET_MAX));
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset[2]/4, MCHIP_CAP(SYS_CAP_ACL_UDF_OFFSET_MAX));
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset[3]/4, MCHIP_CAP(SYS_CAP_ACL_UDF_OFFSET_MAX));
    CTC_MAX_VALUE_CHECK(p_udf_entry->offset_num, 4);
    SYS_USW_ACL_LOCK(lchip);

    _sys_usw_acl_get_udf_info(lchip,  p_udf_entry->udf_id, &p_sys_udf_entry2);

	if(p_sys_udf_entry2 )
	{
        is_update = 1;
	}

	p_sys_udf_entry = &p_usw_acl_master[lchip]->udf_entry[p_udf_entry->priority];

	if(p_sys_udf_entry->key_index_used && !is_update)
	{
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_EXIST);
	}
    if(!is_update)
    {
    	opf.pool_type  = p_usw_acl_master[lchip]->opf_type_udf;
        opf.pool_index = 0;
    	opf.multiple = 0;
        ret =  sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset);
    	if(ret < 0)
    	{
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NO_RESOURCE);
    	}

    	p_sys_udf_entry->udf_hit_index = offset & 0xF;
    	p_sys_udf_entry->key_index = SYS_ACL_UDF_PRI_2_IDX(p_udf_entry->priority);
    }
    p_sys_udf_entry->udf_offset_num = p_udf_entry->offset_num;
    sal_memset(&udf_rst, 0, sizeof(ParserUdfCamResult_m));
    SetParserUdfCamResult(V, skipTcpOption_f, &udf_rst, p_udf_entry->tcp_option_en?1:0);
    SetParserUdfCamResult(V, udfEntryOffset0_f, &udf_rst, p_udf_entry->offset[0]/4);
    SetParserUdfCamResult(V, udfEntryOffset1_f, &udf_rst, p_udf_entry->offset[1]/4);
    SetParserUdfCamResult(V, udfEntryOffset2_f, &udf_rst, p_udf_entry->offset[2]/4);
    SetParserUdfCamResult(V, udfEntryOffset3_f, &udf_rst, p_udf_entry->offset[3]/4);
    SetParserUdfCamResult(V, udfMappedIndex_f, &udf_rst, p_sys_udf_entry->udf_hit_index);

    if (CTC_ACL_UDF_TYPE_NONE != p_udf_entry->offset_type)      /*Compatible with D2*/
    {
        offset_type = sys_usw_acl_map_ctc_to_sys_offset_type(p_udf_entry->offset_type);
        SetParserUdfCamResult(V, udfStartPosType_f, &udf_rst, offset_type);
        SetParserUdfCamResult(V, udfStartPosType0_f, &udf_rst, offset_type);
        SetParserUdfCamResult(V, udfStartPosType1_f, &udf_rst, offset_type);
        SetParserUdfCamResult(V, udfStartPosType2_f, &udf_rst, offset_type);
        SetParserUdfCamResult(V, udfStartPosType3_f, &udf_rst, offset_type);
    }
    else if (sal_memcmp(p_udf_entry->offset_type_array, udf_type_none, sizeof(p_udf_entry->offset_type_array)))
    {
        if (DRV_FLD_IS_EXISIT(ParserUdfCamResult_t, ParserUdfCamResult_udfStartPosType_f))
        {
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_SUPPORT);
        }
        offset_type = sys_usw_acl_map_ctc_to_sys_offset_type(p_udf_entry->offset_type_array[0]);
        SetParserUdfCamResult(V, udfStartPosType0_f, &udf_rst, offset_type);
        offset_type = sys_usw_acl_map_ctc_to_sys_offset_type(p_udf_entry->offset_type_array[1]);
        SetParserUdfCamResult(V, udfStartPosType1_f, &udf_rst, offset_type);
        offset_type = sys_usw_acl_map_ctc_to_sys_offset_type(p_udf_entry->offset_type_array[2]);
        SetParserUdfCamResult(V, udfStartPosType2_f, &udf_rst, offset_type);
        offset_type = sys_usw_acl_map_ctc_to_sys_offset_type(p_udf_entry->offset_type_array[3]);
        SetParserUdfCamResult(V, udfStartPosType3_f, &udf_rst, offset_type);
    }
    else
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "UDF offset type is invalid!\n");
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
    }

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_parser_io_set_parser_udf_cam_result(lchip, &udf_rst,p_sys_udf_entry->key_index));

    if(!is_update)
    {
        sal_memset(&udf_cam, 0, sizeof(ParserUdfCam_m));
        SetParserUdfCam(V, entryValid_f, &udf_cam, 0);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_parser_io_set_parser_udf_cam(lchip, &udf_cam,p_sys_udf_entry->key_index));
    }
	p_sys_udf_entry->udf_id = p_udf_entry->udf_id;
    p_sys_udf_entry->key_index_used = 1;

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_remove_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* p_udf_entry)
{
	sys_acl_udf_entry_t    *p_sys_udf_entry;
    sys_usw_opf_t   opf;
	ParserUdfCam_m udf_cam;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_udf_entry);

	SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"remove_udf_entry,udf_id: %u\n", p_udf_entry->udf_id);


	SYS_USW_ACL_LOCK(lchip);
	_sys_usw_acl_get_udf_info(lchip,  p_udf_entry->udf_id, &p_sys_udf_entry);

	if(!p_sys_udf_entry || !p_sys_udf_entry->key_index_used)
	{
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
	}
	opf.pool_type  = p_usw_acl_master[lchip]->opf_type_udf;
    opf.pool_index = 0;
	opf.multiple = 0;
    sys_usw_opf_free_offset(lchip, &opf, 1, p_sys_udf_entry->udf_hit_index);
   /*save to DB*/
    p_sys_udf_entry->key_index_used = 0;
    p_sys_udf_entry->ip_op = 0;
    p_sys_udf_entry->ip_frag = 0;
    p_sys_udf_entry->mpls_num = 0;
    p_sys_udf_entry->l4_type = 0;
    p_sys_udf_entry->udf_offset_num = 0;
    p_sys_udf_entry->ip_protocol = 0;

    sys_usw_parser_io_get_parser_udf_cam(lchip, &udf_cam, p_sys_udf_entry->key_index);

    SetParserUdfCam(V, entryValid_f, &udf_cam, 0);

    sys_usw_parser_io_set_parser_udf_cam(lchip, &udf_cam, p_sys_udf_entry->key_index);

    /* SYSTEM MODIFIED for SDK bug 106285 */
    sal_memset(p_sys_udf_entry, 0, sizeof(sys_acl_udf_entry_t));

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field)
{
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"add_udf_entry_key_filed,udf_id: %u,key_field_type:%d \n", udf_id,key_field->type);

    SYS_ACL_INIT_CHECK();
    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_check_udf_entry_key_field_union(lchip,  udf_id,  key_field,1));
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_udf_entry_key_field(lchip,  udf_id,  key_field,1));
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_set_udf_entry_key_field_union( lchip,  udf_id, key_field,1));
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_remove_udf_entry_key_field(uint8 lchip, uint32 udf_id, ctc_field_key_t* key_field)
{
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"remove_udf_entry_key_filed,udf_id: %u,key_field_type:%d \n", udf_id,key_field->type);

    SYS_ACL_INIT_CHECK();
    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_check_udf_entry_key_field_union(lchip,  udf_id,  key_field,0));
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_udf_entry_key_field(lchip,  udf_id,  key_field,0));
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_set_udf_entry_key_field_union( lchip,  udf_id, key_field,0));
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_dump_udf_entry_info(uint8 lchip)
{
    uint8 loop = 0;
    uint8 offset_type = 0;
    sys_acl_udf_entry_t    *p_sys_udf_entry;
    ctc_acl_classify_udf_t  udf_entry;
    ParserUdfCamResult_m udf_rst;
    ParserUdfCam_m udf_cam;
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"=================== UDF Overall Status ===================\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"UDF Key Table Name :%s\n","ParserUdfCam");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"OffsetType :0-Invalid; 1-L2 Header; 2:L3 Header; 3-L4 Header; 4-Raw packet header\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4s %-8s %-8s %-6s %-5s %-11s(%-12s|%-12s|%-12s|%-12s)%-10s\n","Pri","Key-idx","Pri-Used","UDF-ID","Valid","Offset-Type", \
                                                                     "Offset0-Type","Offset1-Type","Offset2-Type","Offset3-Type","Offset-Num");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------------------------------------------------------------------------------------------------------\n");

    SYS_USW_ACL_LOCK(lchip);

    sal_memset(&udf_entry, 0, sizeof(ctc_acl_classify_udf_t));
    for(loop = 0; loop < SYS_ACL_MAX_UDF_ENTRY_NUM;loop++)
    {
        p_sys_udf_entry =&p_usw_acl_master[lchip]->udf_entry[loop];

        sys_usw_parser_io_get_parser_udf_cam_result(lchip,&udf_rst,SYS_ACL_UDF_PRI_2_IDX(loop));
        udf_entry.tcp_option_en = GetParserUdfCamResult(V, skipTcpOption_f, &udf_rst);
        udf_entry.offset[0] = GetParserUdfCamResult(V, udfEntryOffset0_f, &udf_rst) << 2;
        udf_entry.offset[1] = GetParserUdfCamResult(V, udfEntryOffset1_f, &udf_rst) << 2;
        udf_entry.offset[2] = GetParserUdfCamResult(V, udfEntryOffset2_f, &udf_rst) << 2;
        udf_entry.offset[3] = GetParserUdfCamResult(V, udfEntryOffset3_f, &udf_rst) << 2;
        if (DRV_FLD_IS_EXISIT(ParserUdfCamResult_t, ParserUdfCamResult_udfStartPosType_f))
        {
            offset_type = GetParserUdfCamResult(V, udfStartPosType_f, &udf_rst);
            udf_entry.offset_type = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
        }
        else
        {
            offset_type = GetParserUdfCamResult(V, udfStartPosType0_f, &udf_rst);
            udf_entry.offset_type_array[0] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
            offset_type = GetParserUdfCamResult(V, udfStartPosType1_f, &udf_rst);
            udf_entry.offset_type_array[1] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
            offset_type = GetParserUdfCamResult(V, udfStartPosType2_f, &udf_rst);
            udf_entry.offset_type_array[2] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
            offset_type = GetParserUdfCamResult(V, udfStartPosType3_f, &udf_rst);
            udf_entry.offset_type_array[3] = sys_usw_acl_map_sys_to_ctc_offset_type(offset_type);
        }
        sys_usw_parser_io_get_parser_udf_cam(lchip, &udf_cam,SYS_ACL_UDF_PRI_2_IDX(loop));
        udf_entry.valid = GetParserUdfCam(V, entryValid_f, &udf_cam);

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4d %-8d %-8d %-6d %-5d %-11d(%-12d|%-12d|%-12d|%-12d)%-10d(%-2d|%-2d|%-2d|%-2d)\n", \
        loop,SYS_ACL_UDF_PRI_2_IDX(loop),p_sys_udf_entry->key_index_used,p_sys_udf_entry->udf_id,udf_entry.valid,udf_entry.offset_type,udf_entry.offset_type_array[0],udf_entry.offset_type_array[1], \
        udf_entry.offset_type_array[2],udf_entry.offset_type_array[3],p_sys_udf_entry->udf_offset_num,udf_entry.offset[0],udf_entry.offset[1],udf_entry.offset[2],udf_entry.offset[3]);
    }
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _ACL_CRYPT_ERROR_TO_CPU_
int32
sys_usw_acl_set_wlan_crypt_error_to_cpu_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint8 step = 1;
    uint16 idx = 0;
    IpeFwdCtl_m ipe_fwd;
    IpeAclQosCtl_m ipe_acl_qos_ctl;
    uint16 lport = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    sys_acl_group_t* pg = NULL;
    sys_acl_entry_t* pe_lkup = NULL;
    ctc_acl_property_t acl_prop;
    ctc_acl_group_info_t acl_group;
    ctc_acl_entry_t acl_entry;
    ctc_field_key_t key_field;
    ctc_acl_field_action_t action_field;
    ctc_acl_to_cpu_t acl_to_cpu;
    sys_acl_block_t* pb = NULL;
    sys_acl_league_t* p_sys_league = NULL;

    SYS_ACL_INIT_CHECK();

    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_get_gchip_id(lchip, &gchip));

    sal_memset(&ipe_fwd, 0, sizeof(IpeFwdCtl_m));
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
    sal_memset(&acl_group, 0, sizeof(ctc_acl_group_info_t));
    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&action_field, 0, sizeof(ctc_acl_field_action_t));
    sal_memset(&acl_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));

    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

    cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));

    if (enable && (!p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu))
    {
        /*check resource*/
        pb = &p_usw_acl_master[lchip]->block[p_usw_acl_master[lchip]->igs_block_num -1];
        p_sys_league = &(p_usw_acl_master[lchip]->league[p_usw_acl_master[lchip]->igs_block_num -1]);
        if (p_sys_league->lkup_level_bitmap == 0)
        {
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NO_RESOURCE);
        }
        if (pb->fpab.entry_count != pb->fpab.free_count)
        {
            step  = 1;
            for (idx = 0; idx < pb->fpab.entry_count; idx = idx + step)
            {
                if((idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_640]) && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640]))      /*640 bit key*/
                {
                    step = 8;
                }
                else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_320] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320]))   /*320 bit key*/
                {
                    step = 4;
                }
                else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_160] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160]))   /*160 bit key*/
                {
                    step = 2;
                }
                else                                            /*80 bit key*/
                {
                    step = 1;
                }

                if (pb->fpab.entries[idx])
                {
                    if (pb->fpab.entries[idx]->entry_id <= CTC_ACL_MAX_USER_ENTRY_ID)
                    {
                        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NO_RESOURCE);
                    }
                }
            }
        }

        /*create acl group*/
        acl_group.type = CTC_ACL_GROUP_TYPE_PORT_CLASS;
        acl_group.dir = CTC_INGRESS;
        acl_group.priority = p_usw_acl_master[lchip]->igs_block_num -1;
        acl_group.lchip = lchip;
        acl_group.un.port_class_id = 1;
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_create_group(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR, &acl_group));

        /*add acl entry*/
        acl_entry.entry_id = 0xFFFFFFFF;
        acl_entry.key_type = CTC_ACL_KEY_CID;
        acl_entry.mode = 1;
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_entry(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR, &acl_entry));

        /*add key and action field*/
        action_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
        action_field.ext_data = &acl_to_cpu;
        acl_to_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
        acl_to_cpu.cpu_reason_id = CTC_PKT_CPU_REASON_DROP;

        _sys_usw_acl_get_entry_by_eid(lchip, 0xFFFFFFFF, &pe_lkup);
        if (!pe_lkup)
        {
    		SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
        }
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_dsacl_field_wlan_error_to_cpu(lchip, &action_field, pe_lkup, 0, 1));

        /*install acl group*/
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_install_group(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR, NULL));

        for (lport=0; lport<SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            if (lport >= SYS_INTERNAL_PORT_START)
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

                /*enable acl on ipe level 7*/
                acl_prop.acl_en = 1;
                acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;
                acl_prop.direction = CTC_INGRESS;
                acl_prop.acl_priority = p_usw_acl_master[lchip]->igs_block_num -1;
                acl_prop.class_id = 1;

                SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_port_set_acl_property(lchip, gport, &acl_prop));
            }
        }

        p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu = 1;
    }
    else if ((!enable) && p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu)
    {
        for (lport=0; lport<SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            if (lport >= SYS_INTERNAL_PORT_START)
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

                /*enable acl on ipe level 7*/
                acl_prop.acl_en = 0;
                acl_prop.direction = CTC_INGRESS;
                acl_prop.acl_priority = p_usw_acl_master[lchip]->igs_block_num -1;

                SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_port_set_acl_property(lchip, gport, &acl_prop));
            }
        }

        /* check if group exist */
        _sys_usw_acl_get_group_by_gid(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR, &pg);
        if (pg)
        {
    		/*uninstall acl group*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_uninstall_group(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR));

            /*remove all entry*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_all_entry(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR));

            /*destroy acl group*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_destroy_group(lchip, SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR));
        }

        p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu = 0;
    }

    if (p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu || p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu)
    {
        SetIpeFwdCtl(V, hardErrorDebugEn_f, &ipe_fwd, 1);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

        SetIpeAclQosCtl(V, hardErrorDebugEn_f, &ipe_acl_qos_ctl, 1);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    }
    else if ((!p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu) && (!p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu))
    {
        SetIpeFwdCtl(V, hardErrorDebugEn_f, &ipe_fwd, 0);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

        SetIpeAclQosCtl(V, hardErrorDebugEn_f, &ipe_acl_qos_ctl, 0);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    }

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(uint8 lchip, uint8 enable)
{
    uint8 i = 0;
    uint32 cmd = 0;
    uint8 step = 1;
    uint16 idx = 0;
    IpeFwdCtl_m ipe_fwd;
    IpeAclQosCtl_m ipe_acl_qos_ctl;
    uint16 lport = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    sys_acl_group_t* pg = NULL;
    sys_acl_entry_t* pe_lkup = NULL;
    ctc_acl_property_t acl_prop;
    ctc_acl_group_info_t acl_group;
    ctc_acl_entry_t acl_entry;
    ctc_field_key_t key_field;
    ctc_acl_field_action_t action_field;
    ctc_acl_to_cpu_t acl_to_cpu;
    sys_acl_block_t* pb = NULL;
    sys_acl_league_t* p_sys_league = NULL;
    uint8 l3type_list[] = {
            CTC_PARSER_L3_TYPE_IP,
            CTC_PARSER_L3_TYPE_IPV4,
            CTC_PARSER_L3_TYPE_IPV6,
            CTC_PARSER_L3_TYPE_MPLS,
            CTC_PARSER_L3_TYPE_MPLS_MCAST,
            CTC_PARSER_L3_TYPE_ARP,
            CTC_PARSER_L3_TYPE_FCOE,
            CTC_PARSER_L3_TYPE_TRILL,
            CTC_PARSER_L3_TYPE_ETHER_OAM,
            CTC_PARSER_L3_TYPE_SLOW_PROTO,
            CTC_PARSER_L3_TYPE_CMAC,
            CTC_PARSER_L3_TYPE_PTP,
            CTC_PARSER_L3_TYPE_SATPDU
        };

    SYS_ACL_INIT_CHECK();

    SYS_USW_ACL_LOCK(lchip);

    SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_get_gchip_id(lchip, &gchip));

    sal_memset(&ipe_fwd, 0, sizeof(IpeFwdCtl_m));
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
    sal_memset(&acl_group, 0, sizeof(ctc_acl_group_info_t));
    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&action_field, 0, sizeof(ctc_acl_field_action_t));
    sal_memset(&acl_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));

    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

    cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));

    if (enable && (!p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu))
    {
        /*check resource*/
        pb = &p_usw_acl_master[lchip]->block[p_usw_acl_master[lchip]->igs_block_num -1];
        p_sys_league = &(p_usw_acl_master[lchip]->league[p_usw_acl_master[lchip]->igs_block_num -1]);
        if (p_sys_league->lkup_level_bitmap == 0)
        {
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NO_RESOURCE);
        }
        if (pb->fpab.entry_count != pb->fpab.free_count)
        {
            step  = 1;
            for (idx = 0; idx < pb->fpab.entry_count; idx = idx + step)
            {
                if((idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_640]) && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640]))      /*640 bit key*/
                {
                    step = 8;
                }
                else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_320] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320]))   /*320 bit key*/
                {
                    step = 4;
                }
                else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_160] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160]))   /*160 bit key*/
                {
                    step = 2;
                }
                else                                            /*80 bit key*/
                {
                    step = 1;
                }

                if (pb->fpab.entries[idx])
                {
                    if (pb->fpab.entries[idx]->entry_id <= CTC_ACL_MAX_USER_ENTRY_ID)
                    {
                        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NO_RESOURCE);
                    }
                }
            }
        }

        /*create acl group*/
        acl_group.type = CTC_ACL_GROUP_TYPE_PORT_CLASS;
        acl_group.dir = CTC_INGRESS;
        acl_group.priority = p_usw_acl_master[lchip]->igs_block_num -1;
        acl_group.lchip = lchip;
        acl_group.un.port_class_id = 2;
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_create_group(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR, &acl_group));

        /*add acl entry*/
        for (i=0; i<sizeof(l3type_list); i++)
        {
            acl_entry.key_type = CTC_ACL_KEY_CID;
            acl_entry.mode = 1;
            acl_entry.entry_id = 0xFFFFFFFE - i;
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_entry(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR, &acl_entry));
        }

        /*add key and action field*/
        for (i=0; i<sizeof(l3type_list); i++)
        {
            key_field.type = CTC_FIELD_KEY_L3_TYPE;
            key_field.mask = 0xff;
            key_field.data = l3type_list[i];
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_operate_key_field(lchip, 0xFFFFFFFE - i, &key_field, 1));

            action_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
            action_field.ext_data = &acl_to_cpu;
            acl_to_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
            acl_to_cpu.cpu_reason_id = CTC_PKT_CPU_REASON_DROP;

            _sys_usw_acl_get_entry_by_eid(lchip, 0xFFFFFFFE - i, &pe_lkup);
            if (!pe_lkup)
            {
        		SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NOT_EXIST);
            }
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_add_dsacl_field_wlan_error_to_cpu(lchip, &action_field, pe_lkup, 0, 1));
        }

        for (lport=0; lport<SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            if (lport < SYS_INTERNAL_PORT_START)
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

                /*enable acl on ipe level 7*/
                acl_prop.acl_en = 1;
                acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;
                acl_prop.direction = CTC_INGRESS;
                acl_prop.acl_priority = p_usw_acl_master[lchip]->igs_block_num -1;
                acl_prop.class_id = 2;

                SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_port_set_acl_property(lchip, gport, &acl_prop));
            }
        }

        /*install acl group*/
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_install_group(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR, NULL));

        p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu = 1;
    }
    else if ((!enable) && p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu)
    {
        for (lport=0; lport<SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            if (lport < SYS_INTERNAL_PORT_START)
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

                /*enable acl on ipe level 7*/
                acl_prop.acl_en = 0;
                acl_prop.direction = CTC_INGRESS;
                acl_prop.acl_priority = p_usw_acl_master[lchip]->igs_block_num -1;

                SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_port_set_acl_property(lchip, gport, &acl_prop));
            }
        }

        /* check if group exist */
        _sys_usw_acl_get_group_by_gid(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR, &pg);
        if (pg)
        {
    		/*uninstall acl group*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_uninstall_group(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR));

            /*remove all entry*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_remove_all_entry(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR));

            /*destroy acl group*/
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_destroy_group(lchip, SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR));
        }

        p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu = 0;
    }

    if (p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu || p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu)
    {
        SetIpeFwdCtl(V, hardErrorDebugEn_f, &ipe_fwd, 1);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

        SetIpeAclQosCtl(V, hardErrorDebugEn_f, &ipe_acl_qos_ctl, 1);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    }
    else if ((!p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu) && (!p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu))
    {
        SetIpeFwdCtl(V, hardErrorDebugEn_f, &ipe_fwd, 0);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

        SetIpeAclQosCtl(V, hardErrorDebugEn_f, &ipe_acl_qos_ctl, 0);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    }

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _ACL_LEAGUE_
STATIC void  _sys_usw_acl_league_free(void* val)
{
    sys_usw_acl_league_node_t* p_val = (sys_usw_acl_league_node_t*)val;
    if(val)
    {
        if(p_val->p_buffer)
        {
            mem_free(p_val->p_buffer);
        }
        mem_free(val);
    }
    return;
}
STATIC int32 _sys_usw_acl_league_cmp(void* val, void* val1)
{
    sys_usw_acl_league_node_t* p_val = (sys_usw_acl_league_node_t*)val;
    sys_usw_acl_league_node_t* p_val1 = (sys_usw_acl_league_node_t*)val1;

    if(p_val->prio > p_val1->prio)
    {
        return -1;
    }
    return 0;
}
int32
sys_usw_acl_set_league_mode(uint8 lchip, ctc_acl_league_t* league)
{
    int32 ret = 0;
    uint16 bmp = 0;
    uint16 tmp_bmp = 0;
    uint8 block_id = 0;
    uint8 tmp_block_id = 0;
    uint8 idx = 0;
    uint8 max_idx = 0;
    uint32 new_size = 0;
    uint32 new_count = 0;
    uint32 count = 0;
    uint8 b_do_merge = FALSE;
    sys_acl_league_t* p_sys_league = NULL;
    sys_acl_league_t* p_tmp_league = NULL;
    ctc_fpa_entry_t** p_tmp_entries = NULL;
    ctc_global_acl_property_t acl_property;
    ctc_linklist_t* node_list[CTC_FPA_KEY_SIZE_NUM] = {NULL};
    uint8  key_size;
    uint8  step ;
    uint16 sub_entry_num[CTC_FPA_KEY_SIZE_NUM] = {0};
    uint16 sub_free_num[CTC_FPA_KEY_SIZE_NUM] = {0};
    uint16 base = 0;
    sys_usw_acl_league_node_t** node_array = NULL;

    SYS_ACL_DBG_FUNC();

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(league);
    CTC_MAX_VALUE_CHECK(league->acl_priority, (CTC_INGRESS == league->dir ? ACL_IGS_BLOCK_MAX_NUM-2: ACL_EGS_BLOCK_MAX_NUM-2));
    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"dir: %u,prio:%d,bitmap:0x%x, ext-tcam 0x%x\n", league->dir,league->acl_priority,league->lkup_level_bitmap, league->ext_tcam_bitmap);

    block_id = (CTC_INGRESS == league->dir ? league->acl_priority: league->acl_priority + ACL_IGS_BLOCK_MAX_NUM);

    bmp = 0xFFFF;
    bmp = ((uint16)(bmp << (16-league->acl_priority))) >> (16-league->acl_priority);
    /*merge or cancel merge level can not less than acl priority itself*/
    if (league->lkup_level_bitmap & bmp)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
    }

    bmp = 0xFFFF;
    max_idx = (CTC_INGRESS == league->dir ? ACL_IGS_BLOCK_MAX_NUM: ACL_EGS_BLOCK_MAX_NUM);
    bmp = ((uint16)(bmp >> max_idx)) << max_idx;
    /*merge or cancel merge level can not bigger than max valid acl priority*/
    if (league->lkup_level_bitmap & bmp)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
    }

    bmp = league->lkup_level_bitmap | (1<<league->acl_priority);
    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);

    /*this level has been already merged*/
    if (p_sys_league->lkup_level_bitmap == 0)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
    }

    /*MUST check parameter before expand tcam*/
    if (p_sys_league->lkup_level_bitmap != bmp)
    {
        for (idx=league->acl_priority+1; idx<max_idx; idx++)
        {
            if (CTC_IS_BIT_SET(bmp, idx) && (!CTC_IS_BIT_SET(bmp, idx-1)))
            {
                CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, end_1);
            }

            tmp_block_id = (CTC_INGRESS == league->dir ? idx: idx + ACL_IGS_BLOCK_MAX_NUM);
            p_tmp_league = &(p_usw_acl_master[lchip]->league[tmp_block_id]);

            /*merge block*/
            if (CTC_IS_BIT_SET(bmp, idx) && (!CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
            {
                /*block has already been merged*/
                tmp_bmp = p_tmp_league->lkup_level_bitmap;
                if (tmp_bmp == 0)
                {
                    CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, end_1);
                }
                /*block has already merged others*/
                CTC_BIT_UNSET(tmp_bmp,idx);
                if (tmp_bmp != 0)
                {
                    CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, end_1);
                }
                if((!league->auto_move_en) && (p_usw_acl_master[lchip]->block[tmp_block_id].fpab.entry_count != p_usw_acl_master[lchip]->block[tmp_block_id].fpab.free_count))
                {
                    CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, end_1);
                }
            }
            /*cancel merge block*/
            else if (!CTC_IS_BIT_SET(bmp, idx) && (CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
            {
                CTC_ERROR_GOTO(_sys_usw_acl_get_entry_count_on_lkup_level(lchip, block_id, idx, &count), ret, end_1);
                /*level not empty*/
                if (count)
                {
                    CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, end_1);
                }
            }
        }
    }

    /*expand tcam firstly*/
    if(league->ext_tcam_bitmap)
    {
        if(DRV_IS_DUET2(lchip) || league->dir == CTC_EGRESS)
        {
            CTC_ERROR_GOTO(CTC_E_NOT_SUPPORT, ret, end_1);
        }
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_adjust_ext_tcam(lchip, league->ext_tcam_bitmap));
    }

    /*nothing changed*/
    if (p_sys_league->lkup_level_bitmap == bmp)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NONE);
    }

    max_idx = (CTC_INGRESS == league->dir ? ACL_IGS_BLOCK_MAX_NUM: ACL_EGS_BLOCK_MAX_NUM);
    new_count = p_usw_acl_master[lchip]->block[block_id].fpab.entry_count;

    sal_memset(&acl_property, 0, sizeof(acl_property));
    acl_property.dir = league->dir;

    tmp_block_id = (CTC_INGRESS == league->dir ? league->acl_priority: league->acl_priority + ACL_IGS_BLOCK_MAX_NUM);

    /*create temp entry list and traverse the first block*/
    if(league->auto_move_en)
    {
        MALLOC_ZERO(MEM_ACL_MODULE, node_array, SYS_ACL_MAX_TCAM_IDX_NUM*sizeof(sys_usw_acl_league_node_t*));
        if(NULL == node_array)
        {
            CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, end_1);
        }
        for(key_size=CTC_FPA_KEY_SIZE_80; key_size < CTC_FPA_KEY_SIZE_NUM; key_size++)
        {
            node_list[key_size] = ctc_list_create(_sys_usw_acl_league_cmp, _sys_usw_acl_league_free);
            if(NULL == node_list[key_size])
            {
                ret = CTC_E_NO_MEMORY;
                goto end_1;
            }
        }
        CTC_ERROR_GOTO(_sys_usw_acl_league_traverse_block(lchip, tmp_block_id, &base, (ctc_linklist_t**)node_list, \
            (uint16*)sub_entry_num, (uint16*)sub_free_num, node_array), ret, end_1);
    }
    for (idx=league->acl_priority+1; idx<max_idx; idx++)
    {
        tmp_block_id = (CTC_INGRESS == league->dir ? idx: idx + ACL_IGS_BLOCK_MAX_NUM);
        p_tmp_league = &(p_usw_acl_master[lchip]->league[tmp_block_id]);

        acl_property.lkup_level = idx;
        CTC_ERROR_GOTO(sys_usw_global_ctl_get(lchip, CTC_GLOBAL_ACL_PROPERTY, &acl_property), ret, end_1);
        /*merge block*/
        if (CTC_IS_BIT_SET(bmp, idx) && (!CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
        {
            if(league->auto_move_en)
            {
                CTC_ERROR_GOTO(_sys_usw_acl_league_traverse_block(lchip, tmp_block_id, &base, (ctc_linklist_t**)node_list, \
                    (uint16*)sub_entry_num, (uint16*)sub_free_num, node_array), ret, end_1);
            }
            else if (p_usw_acl_master[lchip]->block[tmp_block_id].fpab.entry_count != p_usw_acl_master[lchip]->block[tmp_block_id].fpab.free_count)
            {
                ret = CTC_E_INVALID_PARAM;
                goto end_1;
            }

            b_do_merge = TRUE;
            new_count += p_usw_acl_master[lchip]->block[tmp_block_id].entry_num;
            acl_property.random_log_pri = league->acl_priority;


        }
        /*cancel merge block*/
        else if (!CTC_IS_BIT_SET(bmp, idx) && (CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
        {
            CTC_ERROR_GOTO(_sys_usw_acl_get_entry_count_on_lkup_level(lchip, block_id, idx, &count), ret, end_1);
            /*level not empty*/
            if (count)
            {
                ret = CTC_E_INVALID_PARAM;
                goto end_1;
            }
            acl_property.random_log_pri = idx;
        }
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(sys_usw_global_ctl_set(lchip, CTC_GLOBAL_ACL_PROPERTY, &acl_property));
    }


    /*write reg*/
    ret = _sys_usw_acl_set_league_table(lchip, league);
    if (ret)
    {
       goto end_1;
    }

    /*update SW table*/
    if (b_do_merge)
    {
        new_size = sizeof(ctc_fpa_entry_t*) * new_count;
        MALLOC_ZERO(MEM_ACL_MODULE, p_tmp_entries, new_size);
        if (NULL == p_tmp_entries)
        {
            ret = CTC_E_NO_MEMORY;
            goto end_1;
        }
        if(league->auto_move_en)
        {
            /*1. update merged block DB*/
            uint16 total_free_num = 0;
            uint16 start_offset = 0;
            uint16 temp_cnt = 0;
            step = 1;

            temp_cnt = p_usw_acl_master[lchip]->block[block_id].fpab.entry_count;
            mem_free(p_usw_acl_master[lchip]->block[block_id].fpab.entries);
            sal_memset(&(p_usw_acl_master[lchip]->block[block_id].fpab),0, sizeof(ctc_fpa_block_t)) ;
            for(key_size=CTC_FPA_KEY_SIZE_80; key_size < CTC_FPA_KEY_SIZE_NUM; key_size++)
            {
                p_usw_acl_master[lchip]->block[block_id].fpab.start_offset[key_size] = start_offset;
                p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[key_size] = sub_entry_num[key_size];
                p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[key_size] = sub_free_num[key_size];
                p_usw_acl_master[lchip]->block[block_id].fpab.sub_rsv_count[key_size] = 0;
                total_free_num += sub_free_num[key_size]*step;
                start_offset += sub_entry_num[key_size]*step;
                step *= 2;
            }
            p_usw_acl_master[lchip]->block[block_id].fpab.free_count = total_free_num;
            p_usw_acl_master[lchip]->block[block_id].fpab.entry_count = temp_cnt;
        }
        else
        {
            uint32 old_size = 0;
            old_size = sizeof(sys_acl_entry_t*) * p_usw_acl_master[lchip]->block[block_id].fpab.entry_count;
            sal_memcpy(p_tmp_entries, p_usw_acl_master[lchip]->block[block_id].fpab.entries, old_size);
            mem_free(p_usw_acl_master[lchip]->block[block_id].fpab.entries);
        }
        /*2. merge block*/
        for (idx = league->acl_priority + 1; idx < max_idx; idx++)
        {
            tmp_block_id = (CTC_INGRESS == league->dir ? idx: idx + ACL_IGS_BLOCK_MAX_NUM);
            p_tmp_league = &(p_usw_acl_master[lchip]->league[tmp_block_id]);

            if (CTC_IS_BIT_SET(bmp, idx) && (!CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
            {
                _sys_usw_acl_set_league_merge(lchip, p_sys_league, p_tmp_league, league->auto_move_en);
            }

        }

        /*3. move entry from old index to new index and update associated SW table*/
        if(league->auto_move_en)
        {
            CTC_ERROR_GOTO(_sys_usw_acl_league_move_entry(lchip, (ctc_linklist_t**)node_list, p_tmp_entries, \
                (uint16*)sub_entry_num, league->acl_priority, node_array), ret, end_1);
        }
        p_usw_acl_master[lchip]->block[block_id].fpab.entries = p_tmp_entries;
    }
    else
    { /*cancel block */
        for (idx = max_idx - 1; idx > league->acl_priority; idx--)
        {
            tmp_block_id = (CTC_INGRESS == league->dir ? idx: idx + ACL_IGS_BLOCK_MAX_NUM);
            p_tmp_league = &(p_usw_acl_master[lchip]->league[tmp_block_id]);

            if (!CTC_IS_BIT_SET(bmp, idx) && (CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, idx)))
            {
                _sys_usw_acl_set_league_cancel(lchip, p_sys_league, p_tmp_league);
            }
        }
    }

end_1:
    SYS_USW_ACL_UNLOCK(lchip);
    for(key_size=CTC_FPA_KEY_SIZE_80; key_size < CTC_FPA_KEY_SIZE_NUM; key_size++)
    {
        if(node_list[key_size])
        {
            ctc_list_delete_all_node(node_list[key_size]);
            ctc_list_free(node_list[key_size]);
        }
    }
    if(node_array)
    {
        mem_free(node_array);
    }
    return ret;
}

int32
sys_usw_acl_get_league_mode(uint8 lchip, ctc_acl_league_t* league)
{
    uint8 block_id = 0;
    sys_acl_league_t* p_sys_league = NULL;

    SYS_ACL_DBG_FUNC();

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(league);
    CTC_MAX_VALUE_CHECK(league->acl_priority, (CTC_INGRESS == league->dir ? ACL_IGS_BLOCK_MAX_NUM-1: ACL_EGS_BLOCK_MAX_NUM-1));
    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"sys_usw_acl_get_league_mode,dir:%u,prio:%d \n", league->dir,league->acl_priority);

    block_id = (CTC_INGRESS == league->dir ? league->acl_priority: league->acl_priority + ACL_IGS_BLOCK_MAX_NUM);
    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);
    league->lkup_level_bitmap = p_sys_league->lkup_level_bitmap;

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_acl_reorder_entry(uint8 lchip, ctc_acl_reorder_t* reorder)
{
    int32 ret = 0;
    uint8 block_id = 0;
    sys_acl_league_t* p_sys_league = NULL;

    SYS_ACL_DBG_FUNC();

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(reorder);
    CTC_MAX_VALUE_CHECK(reorder->acl_priority, (CTC_INGRESS == reorder->dir ? ACL_IGS_BLOCK_MAX_NUM-1: ACL_EGS_BLOCK_MAX_NUM-1));
    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"sys_usw_acl_reorder_entry,mode:%u,dir:%u,prio:%d \n", reorder->mode,reorder->dir,reorder->acl_priority);

    block_id = (CTC_INGRESS == reorder->dir ? reorder->acl_priority: reorder->acl_priority + ACL_IGS_BLOCK_MAX_NUM);
    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);

    /*acl priority'lookup level is empty*/
    if (!p_sys_league->lkup_level_bitmap)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NONE);
    }

    /*no entry*/
    if (p_usw_acl_master[lchip]->block[p_sys_league->block_id].fpab.entry_count == p_usw_acl_master[lchip]->block[p_sys_league->block_id].fpab.free_count)
    {
        SYS_USW_ACL_RETURN_UNLOCK(CTC_E_NONE);
    }

    switch (reorder->mode)
    {
        case CTC_ACL_REORDER_MODE_SCATTER:
        {
            ret = _sys_usw_acl_reorder_entry_scatter(lchip, p_sys_league);
            break;
        }
        case CTC_ACL_REORDER_MODE_DOWNTOUP:
        {
            ret = _sys_usw_acl_reorder_entry_down_to_up(lchip, p_sys_league);
            break;
        }
        case CTC_ACL_REORDER_MODE_UPTODOWN:
        {
            ret = _sys_usw_acl_reorder_entry_up_to_down(lchip, p_sys_league);
            break;
        }
        default :
        {
            SYS_USW_ACL_RETURN_UNLOCK(CTC_E_INVALID_PARAM);
        }
    }

    SYS_USW_ACL_UNLOCK(lchip);
    return ret;
}

#define _ACL_WB_
int32
sys_usw_acl_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    int32 loop = 0;
    int32 loop_j = 0;
    sys_acl_field_range_wb_data_t range_wb_data;
    uint8 field_range_cnt = 0;
    sys_wb_acl_master_t* p_wb_acl_master = NULL;
    sys_wb_acl_field_range_t*  p_wb_range;
    ctc_wb_data_t wb_data;
    sys_acl_block_t* p_block = NULL;
    sys_acl_traverse_data_t  user_data;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }

    sal_memset(&wb_data, 0, sizeof(ctc_wb_data_t));
    p_block = (sys_acl_block_t*)mem_malloc(MEM_ACL_MODULE, sizeof(sys_acl_block_t)*(ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM));
    wb_data.buffer = mem_malloc(MEM_ACL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if ((NULL == wb_data.buffer) || (NULL == p_block))
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    sal_memcpy(p_block, p_usw_acl_master[lchip]->block, sizeof(sys_acl_block_t)*(ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM));

    /*sync acl entry*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_acl_entry_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_ENTRY);
    user_data.data0 = (void*)&wb_data;
    user_data.data1 = (void*)p_block;
    user_data.value0 = lchip;
    user_data.data2 = (void*)&range_wb_data;
    sal_memset(&range_wb_data, 0, sizeof(sys_acl_field_range_wb_data_t));   /*for range id info*/
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_acl_master[lchip]->entry, _sys_usw_acl_wb_sync_entry, (void*) (&user_data)), ret, done);

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

    /*sync acl_master */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_acl_master_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_MASTER);

    p_wb_acl_master = (sys_wb_acl_master_t*)wb_data.buffer;
    p_wb_acl_master->lchip = lchip;
    p_wb_acl_master->version = SYS_WB_VERSION_ACL;
    p_wb_acl_master->sort_mode = p_usw_acl_master[lchip]->sort_mode;
    for (loop = 0; loop < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; loop++)
    {
        sal_memcpy((uint8*)&p_wb_acl_master->block[loop], &p_block[loop].fpab, sizeof(sys_wb_acl_block_t));
    }

    for(loop=0;loop<SYS_ACL_HASH_KEY_TYPE_NUM;loop++)
    {
        for(loop_j=0;loop_j<SYS_ACL_HASH_SEL_PROFILE_MAX;loop_j++)
        {
            sal_memcpy((uint8*)&p_wb_acl_master->hash_sel_key_union_filed[loop][loop_j],&p_usw_acl_master[lchip]->hash_sel_key_union_filed[loop][loop_j],sizeof(sys_hash_sel_field_union_t));
        }
    }

    for(loop=0; loop<SYS_ACL_MAX_UDF_ENTRY_NUM; loop++)
    {
        sal_memcpy( (uint8*)&p_wb_acl_master->udf_entry[loop],&p_usw_acl_master[lchip]->udf_entry[loop], sizeof(sys_wb_acl_udf_entry_t));
    }

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*sync acl group*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_acl_group_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_GROUP);
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_acl_master[lchip]->group, _sys_usw_acl_wb_sync_group, (void*) (&wb_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

    /*sync acl range info*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_acl_field_range_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_FIELD_RANGE);

    field_range_cnt = range_wb_data.range_id_valid_cnt;
    for(loop = 0; loop < SYS_ACL_FIELD_RANGE_NUM && field_range_cnt; loop++)
    {
        p_wb_range = (sys_wb_acl_field_range_t*)((uint8*)wb_data.buffer + wb_data.valid_cnt*sizeof(sys_wb_acl_field_range_t));
        if(p_usw_acl_master[lchip]->field_range.range[loop].range_type != ACL_RANGE_TYPE_NONE && CTC_IS_BIT_SET(range_wb_data.range_id_bitmap, loop))
        {
            p_wb_range->range_id = loop;
            p_wb_range->range_type = p_usw_acl_master[lchip]->field_range.range[loop].range_type;
            p_wb_range->min = p_usw_acl_master[lchip]->field_range.range[loop].min;
            p_wb_range->max = p_usw_acl_master[lchip]->field_range.range[loop].max;
            p_wb_range->ref_cnt = range_wb_data.range_id_ref[loop];
            wb_data.valid_cnt++;
            field_range_cnt--;
        }
    }

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }
    CTC_ERROR_GOTO(sys_usw_ofb_wb_sync(lchip, p_usw_acl_master[lchip]->ofb_type_cid,
                                                CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_TCAM_CID), ret, done);
done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    if (p_block)
    {
        mem_free(p_block);
    }
    return ret;
}

int32
sys_usw_acl_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    int32 loop = 0;
    int32 loop_j = 0;
    uint16 entry_cnt = 0;
    ctc_wb_query_t wb_query;
    sys_wb_acl_master_t* p_wb_acl_master = NULL;
    sys_wb_acl_group_t wb_acl_group;
    sys_wb_acl_group_t* p_wb_acl_group = &wb_acl_group;
    sys_wb_acl_entry_t wb_acl_entry;
    sys_wb_acl_entry_t* p_wb_acl_entry = &wb_acl_entry;
    sys_acl_group_t* p_acl_group = NULL;
    sys_acl_entry_t* p_acl_entry = NULL;
    sys_wb_acl_field_range_t wb_acl_range;
    sys_wb_acl_field_range_t* p_wb_acl_range = &wb_acl_range;
    sys_acl_cid_action_t sys_cid_action;
    DsCategoryIdPairHashLeftKey_m cid_hash_key;
    uint32* p_data_entry = NULL;
    uint32* p_mask_entry = NULL;
    tbl_entry_t              cid_tcam_key;
    uint32 ad_index = 0;
    uint32 cmd = 0;
    uint32 tbl_key = 0;
    uint32 tbl_ad = 0;
    uint32 temp_index = 0;
    sys_usw_opf_t opf = {0};

    sal_memset(&cid_tcam_key, 0, sizeof(cid_tcam_key));
    sal_memset(&cid_hash_key, 0, sizeof(cid_hash_key));
    sal_memset(&sys_cid_action, 0, sizeof(sys_cid_action));
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    p_data_entry = (uint32*)mem_malloc(MEM_ACL_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    p_mask_entry = (uint32*)mem_malloc(MEM_ACL_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    p_wb_acl_master = (sys_wb_acl_master_t*)mem_malloc(MEM_ACL_MODULE, sizeof(sys_wb_acl_master_t));
    wb_query.buffer = mem_malloc(MEM_ACL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if ((NULL == wb_query.buffer) || (NULL == p_data_entry) || (NULL == p_mask_entry) || (NULL == p_wb_acl_master))
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(p_data_entry, 0, sizeof(uint32)*MAX_ENTRY_WORD);
    sal_memset(p_mask_entry, 0, sizeof(uint32)*MAX_ENTRY_WORD);
    cid_tcam_key.data_entry = p_data_entry;
    cid_tcam_key.mask_entry = p_mask_entry;

    /*restore acl master*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(p_wb_acl_master, 0, sizeof(sys_wb_acl_master_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_acl_master_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_MASTER);
    CTC_ERROR_GOTO((ctc_wb_query_entry(&wb_query)), ret, done);
    if (1 != wb_query.valid_cnt || 1 != wb_query.is_end)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "query acl master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)p_wb_acl_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);

    p_usw_acl_master[lchip]->sort_mode = p_wb_acl_master->sort_mode;
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_ACL, p_wb_acl_master->version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    for(loop = 0; loop < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; loop++)
    {
        sal_memcpy(&p_usw_acl_master[lchip]->block[loop].fpab, &p_wb_acl_master->block[loop], sizeof(sys_wb_acl_block_t));
    }

    for(loop=0;loop<SYS_ACL_HASH_KEY_TYPE_NUM;loop++)
    {
        for(loop_j=0;loop_j<SYS_ACL_HASH_SEL_PROFILE_MAX;loop_j++)
        {
            sal_memcpy(&p_usw_acl_master[lchip]->hash_sel_key_union_filed[loop][loop_j], &p_wb_acl_master->hash_sel_key_union_filed[loop][loop_j],sizeof(sys_wb_hash_sel_field_union_t));
        }
    }

    /*restore udf and opf */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[lchip]->opf_type_udf;
    for(loop=0; loop<SYS_ACL_MAX_UDF_ENTRY_NUM; loop++)
    {
        sal_memcpy(&p_usw_acl_master[lchip]->udf_entry[loop],&p_wb_acl_master->udf_entry[loop], sizeof(sys_wb_acl_udf_entry_t));

        if(p_wb_acl_master->udf_entry[loop].key_index_used)
        {
            ret = sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_wb_acl_master->udf_entry[loop].udf_hit_index);
        }
    }

    /*restore acl group*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&wb_acl_group, 0, sizeof(sys_wb_acl_group_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_acl_group_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_GROUP);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        p_acl_group = mem_malloc(MEM_ACL_MODULE, sizeof(sys_acl_group_t));
        if (NULL == p_acl_group)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_acl_group, 0, sizeof(sys_acl_group_t));

        sal_memcpy((uint8*)&wb_acl_group, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        CTC_ERROR_GOTO(_sys_usw_acl_wb_mapping_group(p_wb_acl_group, p_acl_group, 0), ret, done);
        ctc_hash_insert(p_usw_acl_master[lchip]->group, (void*) p_acl_group);
        if (p_acl_group->group_id == SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR)
        {
            p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu = 1;
        }
        if (p_acl_group->group_id == SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR)
        {
            p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu = 1;
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore acl entry*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&wb_acl_entry, 0, sizeof(sys_wb_acl_entry_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_acl_entry_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_ENTRY);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        p_acl_entry = mem_malloc(MEM_ACL_MODULE, sizeof(sys_acl_entry_t));
        if (NULL == p_acl_entry)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_acl_entry, 0, sizeof(sys_acl_entry_t));

        sal_memcpy((uint8*)&wb_acl_entry, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        /* restore lchip before mapping entry*/
        p_acl_entry->fpae.lchip = lchip;
        CTC_ERROR_GOTO(_sys_usw_acl_wb_mapping_entry(p_wb_acl_entry, p_acl_entry, 0), ret, done);
        ctc_hash_insert(p_usw_acl_master[lchip]->entry, (void*) p_acl_entry);

        if (CTC_FLAG_ISSET(p_acl_entry->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH))
        {
            if (ACL_ENTRY_IS_TCAM(p_acl_entry->key_type))
            {
                _sys_usw_acl_bind_nexthop(lchip, p_acl_entry, p_acl_entry->nexthop_id, 0);
            }
            else
            {
                _sys_usw_acl_bind_nexthop(lchip, p_acl_entry, p_acl_entry->nexthop_id, 1);
            }
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore acl range*/
    sal_memset(&wb_acl_range, 0, sizeof(sys_wb_acl_field_range_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_acl_field_range_t, CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_FIELD_RANGE);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_acl_range, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_usw_acl_master[lchip]->field_range.range[p_wb_acl_range->range_id].range_type = p_wb_acl_range->range_type;
        p_usw_acl_master[lchip]->field_range.range[p_wb_acl_range->range_id].min = p_wb_acl_range->min;
        p_usw_acl_master[lchip]->field_range.range[p_wb_acl_range->range_id].max = p_wb_acl_range->max;
        p_usw_acl_master[lchip]->field_range.range[p_wb_acl_range->range_id].ref = p_wb_acl_range->ref_cnt;
        p_usw_acl_master[lchip]->field_range.free_num--;
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore cid tcam */
    /*MUST restore ofb first*/
    CTC_ERROR_GOTO(sys_usw_ofb_wb_restore(lchip, p_usw_acl_master[lchip]->ofb_type_cid,
                                                CTC_FEATURE_ACL, SYS_WB_APPID_ACL_SUBID_TCAM_CID), ret, done);
    tbl_key = DsCategoryIdPairTcamKey_t;
    for(loop=0; loop<128; loop++)
    {
        cmd = DRV_IOR(tbl_key, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &cid_tcam_key));
        if(GetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, p_data_entry)
          && GetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f, p_data_entry))
        {
            CTC_ERROR_GOTO(sys_usw_ofb_alloc_offset_from_position(lchip, p_usw_acl_master[lchip]->ofb_type_cid,
                                                                                            0, loop, NULL),ret, done);
        }
        else if(GetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, p_data_entry)
          || GetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f, p_data_entry))
        {
            CTC_ERROR_GOTO(sys_usw_ofb_alloc_offset_from_position(lchip, p_usw_acl_master[lchip]->ofb_type_cid,
                                                                                            1, loop, NULL),ret, done);
        }
    }

    /*restore cid hash action spool*/
    for(loop=0; loop < 512; loop++)
    {
        if(loop < 256)
        {
            tbl_key = DsCategoryIdPairHashLeftKey_t;
            tbl_ad = DsCategoryIdPairHashLeftAd_t;
            temp_index = loop;
        }
        else
        {
            tbl_key = DsCategoryIdPairHashRightKey_t;
            tbl_ad = DsCategoryIdPairHashRightAd_t;
            temp_index  = loop-256;
        }

        cmd = DRV_IOR(tbl_key, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, temp_index, cmd, &cid_hash_key));

        if(GetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, &cid_hash_key))
        {
            ad_index = GetDsCategoryIdPairHashLeftKey(V, array_0_adIndex_f, &cid_hash_key);
            sys_cid_action.is_left = (loop < 256) ? 1 : 0;
            sys_cid_action.ad_index = ad_index;

            cmd = DRV_IOR(tbl_ad, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &sys_cid_action.ds_cid_action));

            CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->cid_spool, &sys_cid_action, NULL, NULL));
        }

        if(GetDsCategoryIdPairHashLeftKey(V, array_1_valid_f, &cid_hash_key))
        {
            ad_index = GetDsCategoryIdPairHashLeftKey(V, array_1_adIndex_f, &cid_hash_key);
            sys_cid_action.is_left = (loop < 256) ? 1 : 0;
            sys_cid_action.ad_index = ad_index;

            cmd = DRV_IOR(tbl_ad, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &sys_cid_action.ds_cid_action));
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->cid_spool, &sys_cid_action, NULL, NULL));
        }

    }


done:
    if(wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    if (p_data_entry)
    {
        mem_free(p_data_entry);
    }

    if (p_mask_entry)
    {
        mem_free(p_mask_entry);
    }

    if (p_wb_acl_master)
    {
        mem_free(p_wb_acl_master);
    }

    return ret;
}
STATIC int32
_sys_usw_acl_dump_vlan_edit_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_acl_vlan_edit_t* p_vlan_edit = (sys_acl_vlan_edit_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-12u%-12u%-12s\n", *p_cnt, \
            p_vlan_edit->stag_op, p_vlan_edit->svid_sl, p_vlan_edit->scos_sl, p_vlan_edit->scfi_sl, \
            p_vlan_edit->ctag_op, p_vlan_edit->cvid_sl, p_vlan_edit->ccos_sl, p_vlan_edit->ccfi_sl, \
            p_vlan_edit->tpid_index, p_vlan_edit->profile_id, "Static");
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-12u%-12u%-12u\n", *p_cnt, \
            p_vlan_edit->stag_op, p_vlan_edit->svid_sl, p_vlan_edit->scos_sl, p_vlan_edit->scfi_sl, \
            p_vlan_edit->ctag_op, p_vlan_edit->cvid_sl, p_vlan_edit->ccos_sl, p_vlan_edit->ccfi_sl, \
            p_vlan_edit->tpid_index, p_vlan_edit->profile_id, p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_acl_dump_cid_ad_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_acl_cid_action_t* p_cid_ad = (sys_acl_cid_action_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u%-20uStatic\n", (*p_cnt),p_cid_ad->ad_index, p_cid_ad->is_left);
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u%-20u%-20u\n", (*p_cnt),p_cid_ad->ad_index, p_cid_ad->is_left, p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_dump_hash_ad_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    uint32* p_ds = (uint32*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-24uStatic\n", (*p_cnt),p_ds[ACL_HASH_AD_INDEX_OFFSET]);
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-24u%-20u\n", (*p_cnt), p_ds[ACL_HASH_AD_INDEX_OFFSET],  p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}
int32
sys_usw_acl_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    uint32 loop = 0;
    uint32 loop1 = 0;
    char*   hash_type_str[SYS_ACL_HASH_KEY_TYPE_NUM] = {"Mac", "Ipv4", "L2l3", "Mpls","Ipv6"};
    sys_acl_block_t* pb = NULL;
    sys_hash_sel_field_union_t* p_sel_un = NULL;
    sys_acl_league_t* p_league = NULL;
    sys_traverse_t  param;
    uint32  spool_cnt = 1;

    SYS_ACL_INIT_CHECK();
    SYS_USW_ACL_LOCK(lchip);

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# ACL");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-12s:%-12s%-12u%-12s%-12u%-12s%-12u%-12s%-12u\n","Key Count", \
        "80 bits",p_usw_acl_master[lchip]->key_count[0], "160 bits",p_usw_acl_master[lchip]->key_count[1],\
        "320 bits",p_usw_acl_master[lchip]->key_count[2], "640 bits",p_usw_acl_master[lchip]->key_count[3]);

    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of vlan edit", p_usw_acl_master[lchip]->opf_type_vlan_edit);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of hash ad", p_usw_acl_master[lchip]->opf_type_ad);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of hash cid ad", p_usw_acl_master[lchip]->opf_type_cid_ad);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of udf", p_usw_acl_master[lchip]->opf_type_udf);

    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Ingress sevice acl en", p_usw_acl_master[lchip]->acl_register.ingress_service_acl_en);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Egress sevice acl en", p_usw_acl_master[lchip]->acl_register.egress_service_acl_en);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Bridge pkt vlan-cid high-pri", p_usw_acl_master[lchip]->acl_register.brg_pkt_vlan_cid_high_prio);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Xgpon gem port en", p_usw_acl_master[lchip]->acl_register.xgpon_gem_port_en);

    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Wlan crypt error to cpu", p_usw_acl_master[lchip]->wlan_crypt_error_to_cpu);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Dot1ae crypt error to cpu", p_usw_acl_master[lchip]->dot1ae_crypt_error_to_cpu);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Sort mode", p_usw_acl_master[lchip]->sort_mode);

    SYS_DUMP_DB_LOG(p_f, "\n%-36s\n","Block information:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16s%-16s%-16s\n","Block ID","Entry numer", "Entry count","Free count","Move number");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16s%-16s%-16s%-16s\n","","Key size","Start offset", "Sub cnt","Sub free cnt","Reserve cnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------------------------------");
    for(loop=0; loop < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; loop++)
    {
        pb = &p_usw_acl_master[lchip]->block[loop];
        SYS_DUMP_DB_LOG(p_f, "\n%-16u%-16u%-16u%-16u%-16u\n", loop, pb->entry_num, pb->fpab.entry_count, pb->fpab.free_count, pb->fpab.move_num);

        if(pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80])
        {
            SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16u%-16u%-16u%-16u\n", "","80 bits",pb->fpab.start_offset[CTC_FPA_KEY_SIZE_80],\
                pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80],pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80],pb->fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_80]);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16u%-16u%-16u%-16u\n", "","160 bits",pb->fpab.start_offset[CTC_FPA_KEY_SIZE_160],\
                pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160],pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_160],pb->fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160]);
            SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16u%-16u%-16u%-16u\n", "","320 bits",pb->fpab.start_offset[CTC_FPA_KEY_SIZE_320],\
                pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320],pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_320],pb->fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320]);
            SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16u%-16u%-16u%-16u\n", "","640 bits",pb->fpab.start_offset[CTC_FPA_KEY_SIZE_640],\
                pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640],pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_640],pb->fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640]);
        }

    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-36s\n","League information:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "---------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-22s%-16s\n","Block ID","Lkup level bmp", "Merged to block id");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-22s%-16s%-16s\n","","Level", "Start", "Count");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "---------------------------------------------------------------------------");
    for(loop=0; loop < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; loop++)
    {
        p_league = &p_usw_acl_master[lchip]->league[loop];
        SYS_DUMP_DB_LOG(p_f, "\n%-16u0x%-20x%-16u\n", p_league->block_id, p_league->lkup_level_bitmap, p_league->merged_to);
        for(loop1=0; loop1 < ACL_IGS_BLOCK_MAX_NUM; loop1++)
        {
            SYS_DUMP_DB_LOG(p_f, "%-16s%-22u%-16u%-16u\n","",loop1, p_league->lkup_level_start[loop1], p_league->lkup_level_count[loop1]);
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-36s\n","Hash select information:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-12s%-14s%-14s%-16s%-16s%-16s%-16s%-16s\n","Key Type","Hash sel id", "Profile count","L4 field type","L4 field bmp", "port type", "l2l3 u13 type", "l3 u1 type","v6 ip mode" );
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------------------");
    for(loop=0; loop < SYS_ACL_HASH_KEY_TYPE_NUM; loop++)
    {
        for(loop1=0; loop1 < SYS_ACL_HASH_SEL_PROFILE_MAX; loop1++)
        {
            p_sel_un = &(p_usw_acl_master[lchip]->hash_sel_key_union_filed[loop][loop1]);
            SYS_DUMP_DB_LOG(p_f, "%-16s%-12u%-14u%-14u%-16u%-16u%-16u%-16u%-16u\n", hash_type_str[loop], loop1,\
                p_usw_acl_master[lchip]->hash_sel_profile_count[loop][loop1],\
                p_sel_un->l4_port_field, p_sel_un->l4_port_field_bmp, p_sel_un->key_port_type, \
                                    p_sel_un->l2_l3_key_ul3_type, p_sel_un->l3_key_u1_type, p_sel_un->ipv6_key_ip_mode);
        }
    }

    SYS_DUMP_DB_LOG(p_f, "\n");

    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_acl_master[lchip]->opf_type_vlan_edit, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_acl_master[lchip]->opf_type_ad, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_acl_master[lchip]->opf_type_cid_ad, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_acl_master[lchip]->opf_type_udf, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");

    if(p_usw_acl_master[lchip]->vlan_edit_spool->count)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;

        SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","Vlan edit spool");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-12s%-12s%-12s\n","Node","StagOp","SvidSl","ScosSl","ScfiSl","CtagOp","CvidSl","CcosSl","CcfiSl","TpidIdx","ProfileIdx","Refcnt");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        ctc_spool_traverse(p_usw_acl_master[lchip]->vlan_edit_spool, (spool_traversal_fn)_sys_usw_acl_dump_vlan_edit_spool_db , (void*)(&param));

    }
    if(p_usw_acl_master[lchip]->cid_spool->count)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;

        SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","Cid ad spool");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "DsCategoryIdPairHashRightAd/DsCategoryIdPairHashLeftAd\n");
        SYS_DUMP_DB_LOG(p_f, "%-12s%-20s%-20s%-20s\n", "Node","Table index", "Is left", "Ref count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        ctc_spool_traverse(p_usw_acl_master[lchip]->cid_spool, (spool_traversal_fn)_sys_usw_acl_dump_cid_ad_spool_db , (void*)(&param));
    }
    if(p_usw_acl_master[lchip]->ad_spool->count)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;

        SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","FlowHash ad spool");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "DsFlow\n");
        SYS_DUMP_DB_LOG(p_f, "%-12s%-24s%-20s\n", "Node","Table index", "Ref count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
        ctc_spool_traverse(p_usw_acl_master[lchip]->ad_spool, (spool_traversal_fn)_sys_usw_acl_dump_hash_ad_spool_db , (void*)(&param));
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_ofb_dump_db(lchip, p_usw_acl_master[lchip]->ofb_type_cid, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}

#define _ACL_INIT_

/*
 * init acl module
 */
int32
sys_usw_acl_init(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg)
{
    int32 ret = CTC_E_NONE;
    uint8       idx1 = 0;
    uint8       block_id = 0;
    uint8       lkup_level = 0;
    uint16      start_offset = 0;
    uint32      size = 0;
    uint32      pb_sz[ACL_IGS_BLOCK_MAX_NUM+ACL_EGS_BLOCK_MAX_NUM] = {0};
    uint32      entry_num = 0;
    sys_acl_league_t* p_sys_league = NULL;
    uint8 work_status = 0;

    CTC_PTR_VALID_CHECK(acl_global_cfg);
    LCHIP_CHECK(lchip);

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    /* check init */
    if (p_usw_acl_master[lchip])
    {
        return CTC_E_NONE;
    }

    /* malloc master */
    MALLOC_ZERO(MEM_ACL_MODULE, p_usw_acl_master[lchip], sizeof(sys_acl_master_t));
    if (NULL == p_usw_acl_master[lchip])
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    p_usw_acl_master[lchip]->group = ctc_hash_create(SYS_ACL_USW_GROUP_HASH_SIZE/SYS_ACL_USW_GROUP_HASH_BLOCK_SIZE,
                                        SYS_ACL_USW_GROUP_HASH_BLOCK_SIZE,
                                        (hash_key_fn) _sys_usw_acl_hash_make_group,
                                        (hash_cmp_fn) _sys_usw_acl_hash_compare_group);

    /* init max priority */
    p_usw_acl_master[lchip]->max_entry_priority = CTC_ACL_ENTRY_PRIORITY_HIGHEST;
     _sys_usw_acl_install_build_key_fn(lchip);
     _sys_usw_acl_get_action_fn(lchip);

    /* init block_num_max */
    {
        for (block_id = 0; block_id < ACL_IGS_BLOCK_MAX_NUM; block_id++)
        {
            ACL_GET_TABLE_ENTYR_NUM(lchip, DsAclQosKey80Ing0_t + block_id, &pb_sz[block_id]);

#ifdef EMULATION_ENV /*EMULATION testing*/
            pb_sz[block_id] = 64;
#endif
        }
        for (block_id = 0; block_id < ACL_EGS_BLOCK_MAX_NUM; block_id++)
        {
            ACL_GET_TABLE_ENTYR_NUM(lchip, DsAclQosKey80Egr0_t + block_id, &pb_sz[ACL_IGS_BLOCK_MAX_NUM + block_id]);
#ifdef EMULATION_ENV /*EMULATION testing*/
            pb_sz[ACL_IGS_BLOCK_MAX_NUM + block_id] = 64;
#endif

        }

        p_usw_acl_master[lchip]->igs_block_num = 8;
        p_usw_acl_master[lchip]->egs_block_num = 3;

    }
    p_usw_acl_master[lchip]->field_range.free_num = SYS_ACL_FIELD_RANGE_NUM;

    /* init sort algor / hash table  */
    {
        p_usw_acl_master[lchip]->fpa = fpa_usw_create(_sys_usw_acl_get_block_by_pe_fpa,
                                     _sys_usw_acl_entry_move_hw_fpa,
                                     sizeof(ctc_slistnode_t));

        p_usw_acl_master[lchip]->entry = ctc_hash_create(SYS_ACL_USW_ENTRY_HASH_SIZE/SYS_ACL_USW_ENTRY_HASH_BLOCK_SIZE,
                                            SYS_ACL_USW_ENTRY_HASH_BLOCK_SIZE,
                                            (hash_key_fn) _sys_usw_acl_hash_make_entry,
                                            (hash_cmp_fn) _sys_usw_acl_hash_compare_entry);

        CTC_ERROR_GOTO((_sys_usw_acl_spool_init(lchip)), ret, ERROR_FREE_MEM);

        /*ACL ingress and egress*/
        for (idx1 = 0; idx1 < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; idx1++)
        {
            p_usw_acl_master[lchip]->block[idx1].entry_num = pb_sz[idx1];
            p_usw_acl_master[lchip]->block[idx1].fpab.entry_count = pb_sz[idx1];
            p_usw_acl_master[lchip]->block[idx1].fpab.free_count  = pb_sz[idx1];
            start_offset = 0;
            p_usw_acl_master[lchip]->block[idx1].fpab.start_offset[CTC_FPA_KEY_SIZE_80]    = start_offset;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]  = 0;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_80]   = 0;
            start_offset = 0;
            p_usw_acl_master[lchip]->block[idx1].fpab.start_offset[CTC_FPA_KEY_SIZE_160]    = start_offset;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] = pb_sz[idx1]/4;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160]  = pb_sz[idx1]/4;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160]  = 0;
            start_offset += pb_sz[idx1]/2;
            p_usw_acl_master[lchip]->block[idx1].fpab.start_offset[CTC_FPA_KEY_SIZE_320]    = start_offset;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = pb_sz[idx1]*3/32;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320]  = pb_sz[idx1]*3/32;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;
            start_offset += pb_sz[idx1]*3/8;
            p_usw_acl_master[lchip]->block[idx1].fpab.start_offset[CTC_FPA_KEY_SIZE_640]    = start_offset;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = pb_sz[idx1]/64;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640]  = pb_sz[idx1]/64;
            p_usw_acl_master[lchip]->block[idx1].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;
            size = sizeof(sys_acl_entry_t*) * pb_sz[idx1];

            MALLOC_ZERO(MEM_ACL_MODULE, p_usw_acl_master[lchip]->block[idx1].fpab.entries, size);
            if (NULL == p_usw_acl_master[lchip]->block[idx1].fpab.entries && size)
            {
                goto ERROR_FREE_MEM;
            }

            /* init acl lookup level merge bitmap */
            p_sys_league = &(p_usw_acl_master[lchip]->league[idx1]);
            p_sys_league->block_id = idx1;
            lkup_level = ((idx1 < ACL_IGS_BLOCK_MAX_NUM) ? idx1: (idx1-ACL_IGS_BLOCK_MAX_NUM));
            CTC_BIT_SET(p_sys_league->lkup_level_bitmap, lkup_level);
            p_sys_league->lkup_level_start[lkup_level] = 0;
            p_sys_league->lkup_level_count[lkup_level] = pb_sz[idx1];
            p_sys_league->merged_to = lkup_level;
        }

        if (!(p_usw_acl_master[lchip]->fpa &&
              p_usw_acl_master[lchip]->entry &&
              p_usw_acl_master[lchip]->ad_spool &&
              p_usw_acl_master[lchip]->vlan_edit_spool &&
              p_usw_acl_master[lchip]->cid_spool))
        {
            goto ERROR_FREE_MEM;
        }
    }

    CTC_ERROR_GOTO(_sys_usw_acl_init_league_bitmap(lchip), ret, ERROR_FREE_MEM);

    SYS_ACL_CREATE_LOCK(lchip);

    CTC_ERROR_GOTO(_sys_usw_acl_init_global_cfg(lchip, acl_global_cfg), ret, ERROR_FREE_MEM);

    if (!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER))
    {
        CTC_ERROR_GOTO(_sys_usw_acl_create_rsv_group(lchip), ret, ERROR_FREE_MEM);
    }

    CTC_ERROR_GOTO(_sys_usw_acl_opf_init(lchip), ret, ERROR_FREE_MEM);

    CTC_ERROR_GOTO(_sys_usw_acl_ofb_init(lchip), ret, ERROR_FREE_MEM);

    CTC_ERROR_GOTO(_sys_usw_acl_init_vlan_edit(lchip), ret, ERROR_FREE_MEM);


    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_ACL, sys_usw_acl_ftm_acl_cb);
    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_ACL_FLOW, sys_usw_acl_ftm_acl_flow_cb);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_ACL, sys_usw_acl_wb_sync), ret, ERROR_FREE_MEM);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_acl_wb_restore(lchip), ret, ERROR_FREE_MEM);
    }
    /*capability*/
    sys_usw_ftm_query_table_entry_num(lchip, DsFlowL2HashKey_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL_HASH_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing0_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL0_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing1_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL1_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing2_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL2_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing3_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL3_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing4_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL4_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing5_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL5_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing6_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL6_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Ing7_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL7_IGS_TCAM_ENTRY_NUM) = entry_num;

    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Egr0_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL0_EGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Egr1_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL1_EGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsAclQosL3Key160Egr2_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ACL2_EGS_TCAM_ENTRY_NUM) = entry_num;

    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_ACL, sys_usw_acl_dump_db));
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

 ERROR_FREE_MEM:
    sys_usw_acl_deinit(lchip);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Init failed \n");
    return CTC_E_INIT_FAIL;
}

/*
 * deinit acl module
 */
int32
sys_usw_acl_deinit(uint8 lchip)
{
    uint8       idx1;
    uint32      free_entry_hash = 1;

    LCHIP_CHECK(lchip);
    if (NULL == p_usw_acl_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*ACL ingress and egress*/
    for (idx1 = 0; idx1 < ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM; idx1++)
    {
        mem_free(p_usw_acl_master[lchip]->block[idx1].fpab.entries);
    }

    if(p_usw_acl_master[lchip]->acl_mutex)
    {
        sal_mutex_destroy(p_usw_acl_master[lchip]->acl_mutex);
    }

    /*free vlan edit data*/
    if (p_usw_acl_master[lchip]->vlan_edit_spool)
    {
        ctc_spool_free(p_usw_acl_master[lchip]->vlan_edit_spool);
    }

    /*free ad data*/
    if (p_usw_acl_master[lchip]->ad_spool)
    {
        ctc_spool_free(p_usw_acl_master[lchip]->ad_spool);
    }

    /*free cid data*/
    if (p_usw_acl_master[lchip]->cid_spool)
    {
        ctc_spool_free(p_usw_acl_master[lchip]->cid_spool);
    }

    /*free acl key*/
    ctc_hash_traverse(p_usw_acl_master[lchip]->entry, (hash_traversal_fn)_sys_usw_acl_free_entry_node_data, &free_entry_hash);
    ctc_hash_free(p_usw_acl_master[lchip]->entry);

    /*free acl group*/
    ctc_hash_traverse(p_usw_acl_master[lchip]->group, (hash_traversal_fn)_sys_usw_acl_free_group_node_data, NULL);
    ctc_hash_free(p_usw_acl_master[lchip]->group);

    /*opf free*/
    sys_usw_opf_deinit(lchip, p_usw_acl_master[lchip]->opf_type_ad);
    sys_usw_opf_deinit(lchip, p_usw_acl_master[lchip]->opf_type_vlan_edit);
    sys_usw_ofb_deinit(lchip, p_usw_acl_master[lchip]->ofb_type_cid);
    sys_usw_opf_deinit(lchip, p_usw_acl_master[lchip]->opf_type_cid_ad);
    sys_usw_opf_deinit(lchip, p_usw_acl_master[lchip]->opf_type_udf);

    fpa_usw_free(p_usw_acl_master[lchip]->fpa);

    mem_free(p_usw_acl_master[lchip]);

    return CTC_E_NONE;
}

