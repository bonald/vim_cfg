/**
   @file sys_usw_acl.c

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
#include "usw/include/drv_common.h"

#define SYS_USW_ACL_MAX_KEY_SIZE  40
sys_acl_master_t* p_usw_acl_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define __ACL_GET_INFO__

STATIC int32
_sys_usw_acl_get_key_size_bitmap(uint8 lchip, sys_acl_block_t* pb, uint8* p_key_used_bmp)
{
    uint8   index               = 0;
    uint8   key_used_bmp        = 0;
    uint16  sub_entry_used      = 0;
    uint16  short_entry_used    = 0;

    CTC_PTR_VALID_CHECK(pb);

    if (p_usw_acl_master[lchip]->sort_mode)     /*first check key size 80 used or not, then check key size 160 used or not*/
    {
        index = CTC_FPA_KEY_SIZE_80;
        short_entry_used = pb->fpab.sub_entry_count[index] - pb->fpab.sub_free_count[index];
        short_entry_used? CTC_BIT_SET(key_used_bmp, index): CTC_BIT_UNSET(key_used_bmp, index);
        index = CTC_FPA_KEY_SIZE_160;
        sub_entry_used = !key_used_bmp? pb->fpab.entry_count - pb->fpab.free_count: 0;
        sub_entry_used? CTC_BIT_SET(key_used_bmp, index): CTC_BIT_UNSET(key_used_bmp, index);
    }
    else
    {
        for (index=CTC_FPA_KEY_SIZE_80; index<CTC_FPA_KEY_SIZE_NUM; index+=1)
        {
            sub_entry_used = pb->fpab.sub_entry_count[index] - pb->fpab.sub_free_count[index];
            sub_entry_used? CTC_BIT_SET(key_used_bmp, index): CTC_BIT_UNSET(key_used_bmp, index);
        }
    }

    if (p_key_used_bmp)
    {
        *p_key_used_bmp = key_used_bmp;
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  key_used_bitmap: 0x%X \n", key_used_bmp);
    return CTC_E_NONE;
}

uint8
_sys_usw_acl_get_key_size(uint8 lchip, uint8 by_key_type, sys_acl_entry_t * pe, uint8* o_step)
{
    uint8 key_size = 0;
    uint8 step = 0;

    switch (pe->key_type)
    {
        case CTC_ACL_KEY_INTERFACE:
            key_size = CTC_FPA_KEY_SIZE_80;
            step = 1;
            break;

        case CTC_ACL_KEY_CID:
        case CTC_ACL_KEY_MAC:
        case CTC_ACL_KEY_IPV4:
            key_size = CTC_FPA_KEY_SIZE_160;
            step = 2;
            break;

        case CTC_ACL_KEY_IPV6:
        case CTC_ACL_KEY_MAC_IPV4:
        case CTC_ACL_KEY_IPV4_EXT:
        case CTC_ACL_KEY_FWD:
        case CTC_ACL_KEY_COPP:
        case CTC_ACL_KEY_UDF:
            key_size = CTC_FPA_KEY_SIZE_320;
            step = 4;
            break;

        case CTC_ACL_KEY_IPV6_EXT:
        case CTC_ACL_KEY_MAC_IPV6:
        case CTC_ACL_KEY_MAC_IPV4_EXT:
        case CTC_ACL_KEY_FWD_EXT:
        case CTC_ACL_KEY_COPP_EXT:
            key_size = CTC_FPA_KEY_SIZE_640;
            step = 8;
            break;
        case CTC_ACL_KEY_HASH_MAC:
        case CTC_ACL_KEY_HASH_IPV4:
        case CTC_ACL_KEY_HASH_MPLS:
            key_size = CTC_FPA_KEY_SIZE_160;
            step = 1;
            break;

        case CTC_ACL_KEY_HASH_L2_L3:
        case CTC_ACL_KEY_HASH_IPV6:
            key_size = CTC_FPA_KEY_SIZE_320;
            step = 2;
            break;
        default:
            step = 1;
            break;
    }

    if(!by_key_type && ACL_ENTRY_IS_TCAM(pe->key_type) && (pe->key_type != CTC_ACL_KEY_INTERFACE) && (pe->group->group_info.key_size > CTC_FPA_KEY_SIZE_80))
    {
        key_size = pe->group->group_info.key_size;
        step = 1 << key_size;
    }
    if(o_step)
    {
        *o_step = step;
    }

    return key_size;
}

int32
_sys_usw_acl_get_entry_by_eid(uint8 lchip, uint32 eid, sys_acl_entry_t** pe)
{
    sys_acl_entry_t sys_entry;

    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_entry, 0, sizeof(sys_acl_entry_t));
    sys_entry.fpae.entry_id = eid;
    sys_entry.fpae.lchip= lchip;

    *pe = ctc_hash_lookup(p_usw_acl_master[lchip]->entry, &sys_entry);

    return CTC_E_NONE;
}

/*
 * get sys entry node by entry id
 * pg, pb if be NULL, won't care.
 * pe cannot be NULL.
 */
int32
_sys_usw_acl_get_nodes_by_eid(uint8 lchip, uint32 eid, sys_acl_entry_t** pe, sys_acl_group_t** pg, sys_acl_block_t** pb)
{
    sys_acl_entry_t sys_entry;
    sys_acl_entry_t * pe_lkup = NULL;
    sys_acl_group_t * pg_lkup = NULL;
    sys_acl_block_t * pb_lkup = NULL;
    uint8           block_id  = 0;

    CTC_PTR_VALID_CHECK(pe);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_entry, 0, sizeof(sys_acl_entry_t));
    sys_entry.fpae.entry_id = eid;
    sys_entry.fpae.lchip= lchip;

    pe_lkup = ctc_hash_lookup(p_usw_acl_master[lchip]->entry, &sys_entry);

    if (pe_lkup)
    {
        pg_lkup = pe_lkup->group;
        SYS_ACL_CHECK_GROUP_UNEXIST(pg_lkup);

        /* get block */
        if (ACL_ENTRY_IS_TCAM(pe_lkup->key_type))
        {
            block_id = pg_lkup->group_info.block_id;
            if(CTC_EGRESS == pg_lkup->group_info.dir)
            {
                block_id += p_usw_acl_master[lchip]->igs_block_num;
            }
            pb_lkup  = &p_usw_acl_master[lchip]->block[block_id];
        }
    }

    *pe = pe_lkup;
    if (pg)
    {
        *pg = pg_lkup;
    }
    if (pb)
    {
        *pb = pb_lkup;
    }

    return CTC_E_NONE;
}

/*
 * get sys group node by group id
 */
int32
_sys_usw_acl_get_group_by_gid(uint8 lchip, uint32 gid, sys_acl_group_t** o_group)
{
    sys_acl_group_t * p_sys_group_lkup = NULL;
    sys_acl_group_t sys_group;

    CTC_PTR_VALID_CHECK(o_group);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_group, 0, sizeof(sys_acl_group_t));
    sys_group.group_id = gid;

    p_sys_group_lkup = ctc_hash_lookup(p_usw_acl_master[lchip]->group, &sys_group);

    *o_group = p_sys_group_lkup;

    return CTC_E_NONE;
}

#define __ACL_FPA_CALLBACK__

int32
_sys_usw_acl_get_block_by_pe_fpa(ctc_fpa_entry_t* pe, ctc_fpa_block_t** pb)
{
    uint8          block_id;
    sys_acl_entry_t* entry;
    uint8 lchip = 0;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pb);

    *pb = NULL;
    lchip = pe->lchip;
    entry = ACL_OUTER_ENTRY(pe);
    CTC_PTR_VALID_CHECK(entry->group);

    block_id = entry->group->group_info.block_id;
    if(CTC_EGRESS == entry->group->group_info.dir)
    {
        block_id += p_usw_acl_master[lchip]->igs_block_num;
    }

    *pb = &(p_usw_acl_master[lchip]->block[block_id].fpab);

    return CTC_E_NONE;
}


/*
 * move entry in hardware table to an new index.
 */
int32
_sys_usw_acl_entry_move_hw_fpa(ctc_fpa_entry_t* p_fpa_entry, int32 tcam_idx_new)
{
    uint8   lchip   = 0;
    uint32  old_key_id  = 0;
    uint32  old_act_id  = 0;
    uint32  new_key_id  = 0;
    uint32  new_act_id  = 0;
    uint32  cmd     = 0;
    uint8   step = 0;
    uint32  old_hw_index = 0;
    uint32  new_hw_index = 0;
    uint32  old_offset_a = 0;
    sys_acl_buffer_t  tmp_buffer;
    sys_acl_entry_t*  pe_lkup  = NULL;
    tbl_entry_t         tcam_key;

    CTC_PTR_VALID_CHECK(p_fpa_entry);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    lchip = p_fpa_entry->lchip;

    sal_memset(&tcam_key, 0, sizeof(tcam_key));
    sal_memset(&tmp_buffer, 0, sizeof(tmp_buffer));

    if (p_fpa_entry->flag != FPA_ENTRY_FLAG_INSTALLED)
    {
        return CTC_E_NONE;
    }

    /*get entry*/
    _sys_usw_acl_get_entry_by_eid(lchip, p_fpa_entry->entry_id, &pe_lkup);
    if(!pe_lkup)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    _sys_usw_acl_get_key_size(lchip, 1, pe_lkup, &step);
    _sys_usw_acl_get_table_id(lchip, pe_lkup, &old_key_id, &old_act_id, &old_hw_index);
    old_offset_a = p_fpa_entry->offset_a;
    p_fpa_entry->offset_a = tcam_idx_new;
    _sys_usw_acl_get_table_id(lchip, pe_lkup, &new_key_id, &new_act_id, &new_hw_index);
    p_fpa_entry->offset_a = old_offset_a;

    /*get key from hw*/
    cmd = DRV_IOR(old_key_id, DRV_ENTRY_FLAG);
    tcam_key.data_entry = tmp_buffer.key;
    tcam_key.mask_entry = tmp_buffer.mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(old_hw_index, step), cmd, &tcam_key));

    /*get action from hw*/
    cmd = DRV_IOR(old_act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(old_hw_index, step), cmd, &tmp_buffer.action));

    /*add action*/
    cmd = DRV_IOW(new_act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(new_hw_index, step), cmd, &tmp_buffer.action));

    /* add key */
    cmd = DRV_IOW(new_key_id, DRV_ENTRY_FLAG);
    tcam_key.data_entry = tmp_buffer.key;
    tcam_key.mask_entry = tmp_buffer.mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(new_hw_index, step), cmd, &tcam_key));

    /* then delete old key*/
    cmd = DRV_IOD(old_key_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(old_hw_index, step), cmd, &cmd));

    /* set new_index */
    p_fpa_entry->offset_a = tcam_idx_new;

    return CTC_E_NONE;
}

#define __ACL_RANGE_MANAGE__

int32
_sys_usw_acl_lookup_range(uint8 lchip, uint8 range_type,
                                uint32 min, uint32 max, uint8* o_first_valid)
{
    uint8 index = 0;
    uint8 first_valid = 0;
    uint8 hit = 0;
    sys_acl_field_range_t* p_range = NULL;

    p_range = &(p_usw_acl_master[lchip]->field_range);

    for(index = 0; index < SYS_ACL_FIELD_RANGE_NUM; index++)
    {
        if((ACL_RANGE_TYPE_NONE == p_range->range[index].range_type) && (!hit))
        {
            first_valid = index;
            hit = 1;
        }
        if( (range_type == p_range->range[index].range_type)
            && (max == p_range->range[index].max) && (min == p_range->range[index].min) )
        {
            break;
        }
    }

    if(o_first_valid)
    {
        *o_first_valid = first_valid;
    }

    return index;
}


STATIC int32
_sys_usw_acl_add_range_bitmap(uint8 lchip, uint8 type, uint32 min, uint32 max, uint8* o_range_id)
{
    uint8 index = 0;
    uint8 range_id = 0;
    sys_acl_field_range_t* p_range = NULL;
    sys_parser_range_op_ctl_t range_ctl;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_range = &(p_usw_acl_master[lchip]->field_range);

    index = _sys_usw_acl_lookup_range(lchip, type, min, max, &range_id);
    if(index < SYS_ACL_FIELD_RANGE_NUM)
    {
        range_id = index;
        (p_range->range[index].ref)++;
    }
    else
    {
        if(0 == p_range->free_num)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

        }
        range_ctl.type      = type;
        range_ctl.min_value = min;
        range_ctl.max_value = max;
        CTC_ERROR_RETURN(sys_usw_parser_set_range_op_ctl(lchip, range_id, &range_ctl));

        p_range->range[range_id].range_type = type;
        p_range->range[range_id].min        = min;
        p_range->range[range_id].max        = max;
        p_range->range[range_id].ref        = 1;

        (p_range->free_num)--;
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO," range[%u].ref = %u\n", range_id, p_range->range[range_id].ref);
    if(o_range_id)
    {
        *o_range_id = range_id;
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"add field range, range-type: %d, range-id: %d, min: %u, max: %u\n",
                     type, range_id, min, max);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_remove_range_bitmap(uint8 lchip, uint8 range_id)
{
    sys_acl_field_range_t* p_range = NULL;
    sys_parser_range_op_ctl_t range_ctl;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_range = &(p_usw_acl_master[lchip]->field_range);

    if(p_range->range[range_id].ref > 1)
    {
        (p_range->range[range_id].ref)--;
    }
    else
    {
        range_ctl.type      = 0;
        range_ctl.min_value = 0;
        range_ctl.max_value = 0;
        CTC_ERROR_RETURN(sys_usw_parser_set_range_op_ctl(lchip, range_id, &range_ctl));

        p_range->range[range_id].range_type = ACL_RANGE_TYPE_NONE;
        p_range->range[range_id].min        = 0;
        p_range->range[range_id].max        = 0;
        p_range->range[range_id].ref        = 1;

        (p_range->free_num)++;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_remove_field_range(uint8 lchip, uint8 range_type, sys_acl_range_info_t* p_range_info)
{
    CTC_PTR_VALID_CHECK(p_range_info);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(CTC_IS_BIT_SET(p_range_info->flag, range_type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_remove_range_bitmap(lchip, p_range_info->range_id[range_type]));
        p_range_info->range_bitmap         &= (~(1 << p_range_info->range_id[range_type]));
        p_range_info->range_bitmap_mask    &= (~(1 << p_range_info->range_id[range_type]));
        CTC_BIT_UNSET(p_range_info->flag, range_type);
        p_range_info->range_id[range_type] = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_field_range(uint8 lchip, uint8 range_type, uint16 min, uint16 max, sys_acl_range_info_t* p_range_info)
{
    uint8 old_range_id    = 0;
    uint8 old_range_valid = 0;
    uint8 range_id        = 0;
    sys_acl_field_range_t* p_range = NULL;

    CTC_PTR_VALID_CHECK(p_range_info);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: range_type %u, min %u, max %u\n", range_type, min, max);

    p_range = &(p_usw_acl_master[lchip]->field_range);

    if (min > max)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(CTC_IS_BIT_SET(p_range_info->flag, range_type))
    {
        old_range_id = p_range_info->range_id[range_type];
        old_range_valid = 1;
    }

    if(old_range_valid)
    {
        if(   (range_type == p_range->range[old_range_id].range_type)
            && (min == p_range->range[old_range_id].min)
            && (max == p_range->range[old_range_id].max) )
        {   /*add a same range twice. do nothing.*/
            return CTC_E_NONE;
        }
        CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(lchip, range_type, p_range_info));
    }
    CTC_ERROR_RETURN(_sys_usw_acl_add_range_bitmap(lchip, range_type, min, max, &range_id));

    CTC_BIT_SET(p_range_info->flag, range_type);
    p_range_info->range_id[range_type] = range_id;

    /*refresh bitmap/level*/
    if(CTC_IS_BIT_SET(p_range_info->flag, range_type))
    {
        p_range_info->range_bitmap      |= 1 << (p_range_info->range_id[range_type]);
        p_range_info->range_bitmap_mask |= 1 << (p_range_info->range_id[range_type]);
    }

    return CTC_E_NONE;
}

/*
 * below is build sw struct
 */
#define __ACL_HASH_CALLBACK__


uint32
_sys_usw_acl_hash_make_group(sys_acl_group_t* pg)
{
    return pg->group_id;
}

bool
_sys_usw_acl_hash_compare_group(sys_acl_group_t* pg0, sys_acl_group_t* pg1)
{
    return(pg0->group_id == pg1->group_id);
}

uint32
_sys_usw_acl_hash_make_entry(sys_acl_entry_t* pe)
{
    return pe->fpae.entry_id;
}

bool
_sys_usw_acl_hash_compare_entry(sys_acl_entry_t* pe0, sys_acl_entry_t* pe1)
{
    return(pe0->fpae.entry_id == pe1->fpae.entry_id);
}


STATIC uint32
_sys_usw_acl_hash_make_action(uint32* pa)
{
    uint32 size;
    uint8  * k;

    CTC_PTR_VALID_CHECK(pa);

    size = sizeof(ds_t) - sizeof(uint32);
    k    = (uint8 *) pa;
    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_acl_hash_compare_action(uint32* pa0, uint32* pa1)
{
    uint32 size = 0;

    if (!pa0 || !pa1)
    {
        return FALSE;
    }

    size = sizeof(ds_t) - sizeof(uint32);
    if(!sal_memcmp(pa0, pa1, size))
    {
        return TRUE;
    }

    return FALSE;
}



/* if vlan edit in bucket equals */
STATIC bool
_sys_usw_acl_hash_compare_vlan_edit(sys_acl_vlan_edit_t* pv0,
                                           sys_acl_vlan_edit_t* pv1)
{
    uint32 size = 0;

    if (!pv0 || !pv1)
    {
        return FALSE;
    }

    size = sizeof(sys_acl_vlan_edit_t) - sizeof(uint32);
    if (!sal_memcmp(pv0, pv1, size))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_sys_usw_acl_hash_make_vlan_edit(sys_acl_vlan_edit_t* pv)
{
    uint32 size = 0;
    uint8  * k = NULL;

    CTC_PTR_VALID_CHECK(pv);

    size = sizeof(sys_acl_vlan_edit_t) - sizeof(uint32);
    k    = (uint8 *) pv;

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_acl_hash_compare_cid_ad(sys_acl_cid_action_t* pa0,
                                           sys_acl_cid_action_t* pa1)
{
    uint32 size = 0;

    if (!pa0 || !pa1)
    {
        return FALSE;
    }

    size = sizeof(sys_acl_cid_action_t) - sizeof(uint32);
    if ((pa0->is_left == pa1->is_left) && !sal_memcmp(pa0, pa1, size))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_sys_usw_acl_hash_make_cid_ad(sys_acl_cid_action_t* pa)
{
    uint32 size = sizeof(sys_acl_cid_action_t)-sizeof(pa->ad_index)-sizeof(pa->rsv);
    return ctc_hash_caculate(size, pa);
}

#define __ACL_MAP_UNMAP__

/*
 * check port info if it is valid.
 */
STATIC int32
_sys_usw_acl_check_port_info(uint8 lchip, sys_acl_entry_t* pe, ctc_field_key_t* key_field)
{
    uint8  bitmap_status =0;
    uint8  idx;
    ctc_field_port_t* p_field_port = (ctc_field_port_t*)key_field->ext_data;


    CTC_PTR_VALID_CHECK(key_field->ext_data);
   /*  CTC_PTR_VALID_CHECK(key_field->ext_mask);*/

    if(key_field->type == CTC_FIELD_KEY_PORT)
	{
        if(CTC_FIELD_PORT_TYPE_GPORT == p_field_port->type)
        {
            SYS_GLOBAL_PORT_CHECK(p_field_port->gport);
        }
        if(CTC_FIELD_PORT_TYPE_PORT_BITMAP == p_field_port->type && (lchip != p_field_port->lchip) )
        {
            return CTC_E_INVALID_PARAM;
        }

	}

    if((CTC_ACL_KEY_COPP != pe->key_type) && (CTC_ACL_KEY_COPP_EXT != pe->key_type) && (CTC_ACL_KEY_UDF != pe->key_type))
    {
        bitmap_status = ACL_BITMAP_STATUS_16;
    }
    else
    {
        bitmap_status = ACL_BITMAP_STATUS_64;
    }

    switch(p_field_port->type)
    {
    case CTC_FIELD_PORT_TYPE_PORT_BITMAP:

        /* 1. port_bitmap only allow to use 128 bits */
        for (idx = 4; idx < CTC_PORT_BITMAP_IN_WORD; idx++)
        {
            if (p_field_port->port_bitmap[idx])
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        if (ACL_BITMAP_STATUS_64 == bitmap_status)
        {
            if (((p_field_port->port_bitmap[0] || p_field_port->port_bitmap[1]) +
                 (p_field_port->port_bitmap[2] || p_field_port->port_bitmap[3])) > 1)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        else if (ACL_BITMAP_STATUS_16 == bitmap_status)
        {
            if (((p_field_port->port_bitmap[0] & 0xFFFF || 0) +
                 (p_field_port->port_bitmap[1] & 0xFFFF || 0) +
                 (p_field_port->port_bitmap[2] & 0xFFFF || 0) +
                 (p_field_port->port_bitmap[3] & 0xFFFF || 0) +
                 (p_field_port->port_bitmap[0] >> 16 || 0) +
                 (p_field_port->port_bitmap[1] >> 16 || 0) +
                 (p_field_port->port_bitmap[2] >> 16 || 0) +
                 (p_field_port->port_bitmap[3] >> 16 || 0)) > 1)
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        break;

    case CTC_FIELD_PORT_TYPE_VLAN_CLASS:
        SYS_ACL_VLAN_CLASS_ID_CHECK(p_field_port->vlan_class_id);
        break;

    case CTC_FIELD_PORT_TYPE_PORT_CLASS:
        SYS_ACL_PORT_CLASS_ID_CHECK(p_field_port->port_class_id);
        break;

    case CTC_FIELD_PORT_TYPE_L3IF_CLASS:
        CTC_MAX_VALUE_CHECK(p_field_port->l3if_class_id, 0xFF);
        break;

    case CTC_FIELD_PORT_TYPE_LOGIC_PORT:
        CTC_MAX_VALUE_CHECK(p_field_port->logic_port, (DRV_IS_DUET2(lchip)? 0x7FFF: 0xFFFF));
        break;

    case CTC_FIELD_PORT_TYPE_PBR_CLASS:
        CTC_ACL_PBR_CLASS_ID_CHECK(p_field_port->pbr_class_id);
        break;

    case CTC_FIELD_PORT_TYPE_GPORT:
        SYS_GLOBAL_PORT_CHECK(p_field_port->gport);
        break;

    case CTC_FIELD_PORT_TYPE_NONE:
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*
 * check group info if it is valid.
 */
STATIC int32
_sys_usw_acl_check_group_info(uint8 lchip, ctc_acl_group_info_t* pinfo, uint8 type, uint8 is_create, uint8 status)
{
    uint8 idx;

    CTC_PTR_VALID_CHECK(pinfo);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_ACL_CHECK_GROUP_PRIO(pinfo->dir, pinfo->priority);
    CTC_MAX_VALUE_CHECK(pinfo->dir, CTC_BOTH_DIRECTION - 1);
    CTC_MAX_VALUE_CHECK(pinfo->key_size, CTC_ACL_KEY_SIZE_640);

    switch (type)
    {
    case CTC_ACL_GROUP_TYPE_PORT_BITMAP:

        /* 1. port_bitmap only allow to use 128 bits */
        for (idx = 4; idx < CTC_PORT_BITMAP_IN_WORD; idx++)
        {
            if (pinfo->un.port_bitmap[idx])
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        if (ACL_BITMAP_STATUS_64 == status)
        {
            if (((pinfo->un.port_bitmap[0] || pinfo->un.port_bitmap[1]) +
                 (pinfo->un.port_bitmap[2] || pinfo->un.port_bitmap[3])) > 1)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        else if (ACL_BITMAP_STATUS_16 == status)
        {
            if (((pinfo->un.port_bitmap[0] & 0xFFFF || 0) +
                 (pinfo->un.port_bitmap[1] & 0xFFFF || 0) +
                 (pinfo->un.port_bitmap[2] & 0xFFFF || 0) +
                 (pinfo->un.port_bitmap[3] & 0xFFFF || 0) +
                 (pinfo->un.port_bitmap[0] >> 16 || 0) +
                 (pinfo->un.port_bitmap[1] >> 16 || 0) +
                 (pinfo->un.port_bitmap[2] >> 16 || 0) +
                 (pinfo->un.port_bitmap[3] >> 16 || 0)) > 1)
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        break;

    case CTC_ACL_GROUP_TYPE_GLOBAL:
        break;

    case CTC_ACL_GROUP_TYPE_NONE:
        break;

    case CTC_ACL_GROUP_TYPE_VLAN_CLASS:
        SYS_ACL_VLAN_CLASS_ID_CHECK(pinfo->un.vlan_class_id);
        break;

    case CTC_ACL_GROUP_TYPE_PORT_CLASS:
        SYS_ACL_PORT_CLASS_ID_CHECK(pinfo->un.port_class_id);
        break;

    case CTC_ACL_GROUP_TYPE_L3IF_CLASS:
        CTC_MAX_VALUE_CHECK(pinfo->un.l3if_class_id, 0xFF);
        break;

    case CTC_ACL_GROUP_TYPE_SERVICE_ACL:
        CTC_MAX_VALUE_CHECK(pinfo->un.service_id, MCHIP_CAP(SYS_CAP_ACL_SERVICE_ID_NUM));
        break;

    case CTC_ACL_GROUP_TYPE_HASH: /* check nothing*/
        CTC_MAX_VALUE_CHECK(pinfo->dir, CTC_INGRESS);
        break;

    case CTC_ACL_GROUP_TYPE_PBR_CLASS:
        CTC_ACL_PBR_CLASS_ID_CHECK(pinfo->un.pbr_class_id);
        break;

    case CTC_ACL_GROUP_TYPE_PORT:
        SYS_GLOBAL_PORT_CHECK(pinfo->un.gport);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*
 * get sys group info based on ctc group info
 */
STATIC int32
_sys_usw_acl_map_group_info(uint8 lchip, sys_acl_group_info_t* sys, ctc_acl_group_info_t* ctc, uint8 is_create)
{
    if (is_create) /* install group cannot modify: type, priority, direction*/
    {
        sys->type     = ctc->type;
        sys->block_id = ctc->priority;
        sys->dir      = ctc->dir;
        sys->key_size = ctc->key_size;
    }

    if(CTC_ACL_GROUP_TYPE_PORT == sys->type)
    {
        sys->un.gport = ctc->un.gport;
    }
    else
    {
        sal_memcpy(&sys->un, &ctc->un, sizeof(ctc_port_bitmap_t));
        sys->lchip = ctc->lchip;
    }

    return CTC_E_NONE;
}


/*
 * get ctc group info based on sys group info
 */
int32
_sys_usw_acl_unmap_group_info(uint8 lchip, ctc_acl_group_info_t* ctc, sys_acl_group_info_t* sys)
{
    ctc->dir      = sys->dir;
    ctc->type     = sys->type;
    ctc->priority = sys->block_id;
    ctc->lchip    = lchip;

    if(CTC_ACL_GROUP_TYPE_PORT == sys->type)
    {
        ctc->un.gport = sys->un.gport;
    }
    else
    {
        sal_memcpy(&ctc->un, &sys->un, sizeof(ctc_port_bitmap_t));
    }


    return CTC_E_NONE;
}

/*
 * remove accessory property
 */
STATIC int32
_sys_usw_acl_remove_accessory_property(uint8 lchip, sys_acl_entry_t* pe)
{
    uint8 range_type = 0;
    CTC_PTR_VALID_CHECK(pe);

    for(range_type = ACL_RANGE_TYPE_PKT_LENGTH; range_type < ACL_RANGE_TYPE_NUM; range_type++)
    {
        if(CTC_IS_BIT_SET(pe->rg_info.flag, range_type))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(lchip, range_type, &(pe->rg_info)));
        }
    }

    return CTC_E_NONE;
}

#define  __ACL_BUILD_FREE_INDEX__

/* build index of HASH ad */
STATIC int32
_sys_usw_acl_build_hash_ad_index(uint32* pa, uint8* lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(pa);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[*lchip]->opf_type_ad;
    if (CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*lchip, &opf, 1, pa[ACL_HASH_AD_INDEX_OFFSET]));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "hash_ad_index_from_position=%d\n", pa[ACL_HASH_AD_INDEX_OFFSET]);
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*lchip, &opf, 1, &value_32));
        /*pa[31] used as ad_index*/
        pa[ACL_HASH_AD_INDEX_OFFSET] = value_32 & 0xFFFF;
    }
    return CTC_E_NONE;
}

/* free index of HASH ad */
STATIC int32
_sys_usw_acl_free_hash_ad_index(uint32* pa, uint8* lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(pa);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[*lchip]->opf_type_ad;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, pa[ACL_HASH_AD_INDEX_OFFSET]));


    return CTC_E_NONE;
}


STATIC int32
_sys_usw_acl_build_vlan_edit_index(sys_acl_vlan_edit_t* pv, uint8* p_lchip)
{
    sys_usw_opf_t opf;
    uint32               value_32 = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[*p_lchip]->opf_type_vlan_edit;
    if (CTC_WB_ENABLE && CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, pv->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        pv->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_free_vlan_edit_index(sys_acl_vlan_edit_t* pv, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[*p_lchip]->opf_type_vlan_edit;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, (uint32) pv->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_build_cid_ad_index(sys_acl_cid_action_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;
    uint32               value_32 = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(pa->is_left)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    opf.pool_type  = p_usw_acl_master[*p_lchip]->opf_type_cid_ad;
    if (CTC_WB_ENABLE && CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, pa->ad_index));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        pa->ad_index = value_32 & 0xFFFF;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_free_cid_ad_index(sys_acl_cid_action_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(pa->is_left)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    opf.pool_type = p_usw_acl_master[*p_lchip]->opf_type_cid_ad;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, pa->ad_index));

    return CTC_E_NONE;
}

#define  __ACL_ESSENTIAL_API__


STATIC int32
_sys_usw_acl_alloc_tcam_entry(uint8 lchip, sys_acl_group_t* pg, sys_acl_entry_t* pe)
{
    uint8   block_id = 0;
    uint8   key_size = 0;
    uint8   step     = 0;
    uint8   key_used_bitmap = 0;
    uint8   real_step = 0;

    sys_acl_block_t * pb        = NULL;
    uint32          block_index = 0;

    /* get block index. */
    _sys_usw_acl_get_key_size(lchip, 1, pe, &real_step);
    key_size = _sys_usw_acl_get_key_size(lchip, 0, pe, &step);

    block_id = pg->group_info.block_id;
    if(CTC_EGRESS == pg->group_info.dir)
    {
        block_id += p_usw_acl_master[lchip]->igs_block_num;
    }

    pb = &(p_usw_acl_master[lchip]->block[block_id]);

    _sys_usw_acl_get_key_size_bitmap(lchip, pb, &key_used_bitmap);
    if ((CTC_IS_BIT_SET(key_used_bitmap, CTC_FPA_KEY_SIZE_80) && (CTC_FPA_KEY_SIZE_80 != key_size)) || \
       (!CTC_IS_BIT_SET(key_used_bitmap, CTC_FPA_KEY_SIZE_80) && key_used_bitmap && (CTC_FPA_KEY_SIZE_80 == key_size)))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Tcam key 80 can't be used with others!\n");
        return CTC_E_NOT_SUPPORT;
    }

    if(p_usw_acl_master[lchip]->sort_mode)
    {
        CTC_ERROR_RETURN(pb->fpab.free_count < step? CTC_E_NO_RESOURCE: CTC_E_NONE);
        (pb->fpab.free_count) -= step;
        (pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]) -= (CTC_FPA_KEY_SIZE_80 == key_size)? 1: 0;
        block_index = pe->fpae.priority;
    }
    else
    {
        CTC_ERROR_RETURN(fpa_usw_alloc_offset(p_usw_acl_master[lchip]->fpa, &pb->fpab, key_size,
                                    pe->fpae.priority, &block_index));

    }
    /* add to block */
    pb->fpab.entries[block_index] = &pe->fpae;
    /* set block index */
    pe->fpae.offset_a = block_index;
    pe->fpae.step     = step;
    pe->fpae.real_step = real_step;
    fpa_usw_reorder(p_usw_acl_master[lchip]->fpa, &pb->fpab, key_size);
    return CTC_E_NONE;

}


int32
_sys_usw_acl_update_key_count(uint8 lchip, sys_acl_entry_t* pe, uint8 is_add)
{
    int8 cnt = 1;

    if(!is_add)
    {
        cnt = -1;
    }

    switch(pe->key_type)
    {
        case CTC_ACL_KEY_INTERFACE:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_80] += cnt;
            break;

        case CTC_ACL_KEY_CID:
        case CTC_ACL_KEY_MAC:
        case CTC_ACL_KEY_IPV4:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_160] += cnt;
            break;

        case CTC_ACL_KEY_IPV6:
        case CTC_ACL_KEY_IPV4_EXT:
        case CTC_ACL_KEY_MAC_IPV4:
        case CTC_ACL_KEY_FWD:
        case CTC_ACL_KEY_COPP:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_320] += cnt;
            break;

        case CTC_ACL_KEY_IPV6_EXT:
        case CTC_ACL_KEY_MAC_IPV6:
        case CTC_ACL_KEY_MAC_IPV4_EXT:
        case CTC_ACL_KEY_FWD_EXT:
        case CTC_ACL_KEY_COPP_EXT:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_640] += cnt;
            break;

        case CTC_ACL_KEY_HASH_MAC:
        case CTC_ACL_KEY_HASH_IPV4:
        case CTC_ACL_KEY_HASH_MPLS:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_HASH] += cnt;
            break;

        case CTC_ACL_KEY_HASH_L2_L3:
        case CTC_ACL_KEY_HASH_IPV6:
            p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_HASH] += (cnt + cnt);
            break;

        default:
            break;
    }


    return CTC_E_NONE;
}

/*
 * pe, pg, pb are lookup result.
 */
STATIC int32
_sys_usw_acl_remove_tcam_entry(uint8 lchip, sys_acl_entry_t* pe_lkup,
                                      sys_acl_group_t* pg_lkup,
                                      sys_acl_block_t* pb_lkup)
{
    if(!pb_lkup)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "remove_tcam_entry abnormal,fpae is not exist\n");

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid configuration \n");
        return CTC_E_INVALID_CONFIG;

    }
    if (pe_lkup->vlan_edit_valid)
    {
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_acl_master[lchip]->vlan_edit_spool, pe_lkup->vlan_edit, NULL));
    }

    if(pe_lkup->ether_type != 0)
    {
        CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe_lkup->ether_type));
    }

    /* remove from group */
    ctc_slist_delete_node(pg_lkup->entry_list, &(pe_lkup->head));

    /* remove from hash */
    ctc_hash_remove(p_usw_acl_master[lchip]->entry, pe_lkup);

    /* remove accessory property */
    _sys_usw_acl_remove_accessory_property(lchip, pe_lkup);

    /* free index */
    if(p_usw_acl_master[lchip]->sort_mode)
    {
        uint8 step = 0;
        uint8 key_size = 0;

        key_size = _sys_usw_acl_get_key_size(lchip, 0, pe_lkup, &step);
        (pb_lkup->fpab.free_count) += step;
        (pb_lkup->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]) += (CTC_FPA_KEY_SIZE_80 == key_size)? 1: 0;
        /* add to block */
        pb_lkup->fpab.entries[pe_lkup->fpae.offset_a] = NULL;
    }
    else
    {
        fpa_usw_free_offset(&pb_lkup->fpab, pe_lkup->fpae.offset_a);
    }
    _sys_usw_acl_update_key_count(lchip, pe_lkup, 0);

    (pg_lkup->entry_count)--;

    /* remove buffer */
    if(pe_lkup->buffer)
    {
        mem_free(pe_lkup->buffer);
    }

    /* memory free */
    mem_free(pe_lkup);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_acl_remove_hash_entry(uint8 lchip, sys_acl_entry_t* pe_lkup,
                                      sys_acl_group_t* pg_lkup,
                                      sys_acl_block_t* pb_lkup)
{
    uint32      key_id  = 0;
    uint32      hw_index = 0;
    drv_acc_in_t   in;
    drv_acc_out_t  out;

    /* remove from group */
    ctc_slist_delete_node(pg_lkup->entry_list, &(pe_lkup->head));

    /* remove from hash */
    ctc_hash_remove(p_usw_acl_master[lchip]->entry, pe_lkup);

    /* remove accessory property */
    CTC_ERROR_RETURN(_sys_usw_acl_remove_accessory_property(lchip, pe_lkup));

    _sys_usw_acl_update_key_count(lchip, pe_lkup, 0);

    /* clear hw */
    if((pe_lkup->fpae.offset_a != 0) && (pe_lkup->fpae.offset_a != CTC_MAX_UINT32_VALUE))
    {
        _sys_usw_acl_get_table_id(lchip, pe_lkup, &key_id, NULL, &hw_index);
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));

        in.type     = DRV_ACC_TYPE_DEL;
        in.op_type  = DRV_ACC_OP_BY_INDEX;
        in.tbl_id   = key_id;
        in.index    = hw_index;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    }

    /* remove buffer */
    if(pe_lkup->buffer)
    {
        mem_free(pe_lkup->buffer);
    }

 /*    // memory free */
 /*    if(pe_lkup)*/
 /*    {*/
 /*        mem_free(pe_lkup);*/
 /*    }*/

    (pg_lkup->entry_count)--;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_acl_add_tcam_entry(uint8 lchip, sys_acl_group_t* pg, sys_acl_entry_t* pe)
{
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*  key and action */
    CTC_ERROR_RETURN(_sys_usw_acl_alloc_tcam_entry(lchip, pg, pe));

    /* add to hash */
    ctc_hash_insert(p_usw_acl_master[lchip]->entry, pe);

    /* add to group */
    ctc_slist_add_head(pg->entry_list, &(pe->head));

    /* set flag */
    pe->fpae.flag = FPA_ENTRY_FLAG_UNINSTALLED;

    (pg->entry_count)++;

    _sys_usw_acl_update_key_count(lchip, pe, 1);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_add_hash_entry(uint8 lchip, sys_acl_group_t* pg, sys_acl_entry_t* pe)
{
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /* add to hash */
    ctc_hash_insert(p_usw_acl_master[lchip]->entry, pe);

    /* add to group */
    ctc_slist_add_head(pg->entry_list, &(pe->head));

    /* set flag */
    pe->fpae.flag = FPA_ENTRY_FLAG_UNINSTALLED;

    (pg->entry_count)++;

    _sys_usw_acl_update_key_count(lchip, pe, 1);

    return CTC_E_NONE;
}

#define __ACL_DUMP_INFO__

/*1:tag_op; 0:tag_sl; */
const char*
_sys_usw_get_vlan_edit_desc(uint8 is_tag_op, uint8 type)
{
    if(1 == is_tag_op)
    {
        switch (type)
        {
            case CTC_ACL_VLAN_TAG_OP_NONE:
                return "None";
            case CTC_ACL_VLAN_TAG_OP_REP:
                return "Replace";
            case CTC_ACL_VLAN_TAG_OP_ADD:
                return "Add";
            case CTC_ACL_VLAN_TAG_OP_DEL:
                return "Delete";
            case CTC_ACL_VLAN_TAG_OP_REP_OR_ADD:
                return "Replace or add";
            default:
                return "Error type";
        }
    }
    else
    {
        switch (type)
        {
            case CTC_VLAN_TAG_SL_AS_PARSE:
                return "As parser";
            case CTC_VLAN_TAG_SL_ALTERNATIVE:
                return "Alternative";
            case CTC_VLAN_TAG_SL_NEW:
                return "New";
            case CTC_VLAN_TAG_SL_DEFAULT:
                return "Default";
            default:
                return "Error type";
        }
    }
    return NULL;
}

int32
_sys_usw_acl_get_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* p_field_action)
{
    sys_acl_entry_t* pe = NULL;

    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_field_action);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_action_type %u\n", entry_id, p_field_action->type);

    _sys_usw_acl_get_entry_by_eid(lchip, entry_id, &pe);
    if(pe && pe->buffer)
    {
        CTC_ERROR_RETURN(p_usw_acl_master[lchip]->get_ad_func[pe->action_type](lchip, p_field_action,pe));
    }
    else
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

const char*
_sys_usw_get_color_desc(uint8 color_type)
{
    switch (color_type)
    {
        case CTC_QOS_COLOR_NONE:
            return "None";
        case CTC_QOS_COLOR_RED:
            return "Red";
        case CTC_QOS_COLOR_YELLOW:
            return "Yellow";
        case CTC_QOS_COLOR_GREEN:
            return "Green";
        default:
            return "Error color";
    }
    return NULL;
}

const char*
_sys_usw_get_cp_to_cpu_desc(uint8 cp_to_cpu_type)
{
    switch (cp_to_cpu_type)
    {
        case CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER:
            return "Not Cover Cpu Reason";
        case CTC_ACL_TO_CPU_MODE_TO_CPU_COVER:
            return "Cover Cpu Reason";
        case CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU:
            return "Cancel To CPU";
        default:
            return "Error color";
    }
    return NULL;
}

 /* 1: gport*/
 /* 2: logical port*/
STATIC int32
_sys_usw_acl_hash_port_type_judge(uint8 lchip, uint8 key_type,uint8 hash_sel_id)
{
    ds_t ds_sel ={0};
    fld_id_t field_id;
    uint32 table_id = 0;
    uint32 drv_data_temp[4] ={0};

    CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, key_type, hash_sel_id, &ds_sel));
    switch(key_type)
    {
    case CTC_ACL_KEY_HASH_MAC:
        table_id = FlowL2HashFieldSelect_t;
        break;
    case CTC_ACL_KEY_HASH_IPV4:
        table_id = FlowL3Ipv4HashFieldSelect_t;
        break;
    case CTC_ACL_KEY_HASH_L2_L3:
        table_id = FlowL2L3HashFieldSelect_t;
        break;
    case CTC_ACL_KEY_HASH_MPLS:
        table_id = FlowL3MplsHashFieldSelect_t;
        break;
    case CTC_ACL_KEY_HASH_IPV6:
        table_id = FlowL3Ipv6HashFieldSelect_t;
        break;
    default:
        return CTC_E_INTR_INVALID_PARAM;
    }

    drv_usw_get_field_id_by_sub_string(lchip, table_id, &field_id, "globalPortType");
    DRV_GET_FIELD(lchip, table_id, field_id, ds_sel, drv_data_temp);

    if (ACL_HASH_GPORT_TYPE_GPORT == drv_data_temp[0])
    {
        return 1;
    }
    else if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == drv_data_temp[0])
    {
        return 2;
    }

    return CTC_E_NONE;

}

 /*data_type: 1 uint32; 2 ext_data*/
STATIC int32
sys_usw_acl_map_key_field_to_field_name(uint8 lchip, sys_acl_entry_t* pe, uint16 key_field_type, uint32 key_id, uint8* data_type, char** field_name, uint8* start_bit, uint8* bit_len, uint8* field_name_cnt, char** print_str, uint32* p_key_size)
{
    uint32 drv_data_temp[4] = {0,0,0,0};
    uint8 is_hash = !ACL_ENTRY_IS_TCAM(pe->key_type);
    fld_id_t field_id;
    char* str = NULL;
    uint32 key_size = 4;
    CTC_MAX_VALUE_CHECK(key_field_type,CTC_FIELD_KEY_NUM);
    CTC_PTR_VALID_CHECK(field_name);
    CTC_PTR_VALID_CHECK(start_bit);
    CTC_PTR_VALID_CHECK(bit_len);

    *data_type = 1;
    start_bit[0] = 0;
    bit_len[0]   = 32;
    start_bit[1] = 0;
    bit_len[1]   = 0;
    *field_name_cnt = 1;

    switch(key_field_type)
    {
#if 0
        case CTC_FIELD_KEY_RARP:
            field_name[0] = "isRarp";
            str = "Rarp";
            break;
        case CTC_FIELD_KEY_VXLAN_RSV1:
            if (CTC_ACL_KEY_FWD == pe->key_type)
            {
                field_name[0] = "ipAddr1";
            }
            else if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
            {
                field_name[0] = "ipSa";
            }
            else if (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
            {
                field_name[0] = "radioMac";
            }
            else
            {
                field_name[0] = "vxlanReserved1";
            }
            str = "Vxlan RSV 1";
            break;
        case CTC_FIELD_KEY_VXLAN_RSV2:
            if (CTC_ACL_KEY_FWD == pe->key_type)
            {
                field_name[0] = "ipAddr1";
            }
            else if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
            {
                field_name[0] = "ipSa";
            }
            else if (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
            {
                field_name[0] = "radioMac";
            }
            else
            {
                field_name[0] = "vxlanReserved2";
            }
            str = "Vxlan RSV 2";
            break;
        case CTC_FIELD_KEY_VLAN_XLATE_HIT:
            *data_type = 1;
            field_name[1] = "vlanXlate1LookupNoResult";
            bit_len[1]   = 1;
            field_name[0] = "vlanXlate0LookupNoResult";
            bit_len[0]   = 1;
            *field_name_cnt = 2;
            str = "Vlan Xlate Hit";
            break;
        case CTC_FIELD_KEY_L2_TYPE:
            field_name[0] = "layer2Type";
            str = "L2 Type";
            break;
        case CTC_FIELD_KEY_L3_TYPE:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "etherType";
                    start_bit[0] = 7;
                    bit_len[0]   = 4;
                }
                else
                {
                    field_name[0] = "layer3Type";
                }
                str = "L3 Type";
                break;
            }
        case CTC_FIELD_KEY_L4_TYPE:
            {
                if (CTC_ACL_KEY_MAC_IPV4 == pe->key_type)
                {
                    field_name[0] = "layer3HeaderProtocol";
                }
                else if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "ipDa";
                    start_bit[0] = 32;
                    bit_len[0]   = 4;
                }
                else if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "etherType";
                    start_bit[0] = 11;
                    bit_len[0]   = 4;
                }
                else
                {
                    field_name[0] = "layer4Type";
                }
                str = "L4 Type";
                break;
            }
        case CTC_FIELD_KEY_L4_USER_TYPE:
            field_name[0] = "layer4UserType";
            str = "L4 User Type";
            break;
#endif
        /*Ether  field*/
        case CTC_FIELD_KEY_MAC_SA:
            if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
            {
                field_name[0] = "u1_g1_macSa";
            }
            else
            {
                field_name[0] = "macSa";
            }
            str = "Mac Sa";
            *data_type = 2;
            key_size = sizeof(mac_addr_t);
            break;
        case CTC_FIELD_KEY_MAC_DA:
            if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
            {
                field_name[0] = "u1_g1_macDa";
            }
            else
            {
                field_name[0] = "macDa";
            }
            str = "Mac Da";
            *data_type = 2;
            key_size = sizeof(mac_addr_t);
            break;
#if 0
        case CTC_FIELD_KEY_STAG_VALID:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "svlanIdValid";
                }
                else   /*DsFlowL2HashKey  DsAclQosMacKey160  DsAclQosKey80   DsAclQosForwardKey320	DsAclQosCoppKey320	DsAclQosCoppKey640 */
                {
                    field_name[0] = "vlanIdValid";
                }
                str = "STAG Valid";
                break;
            }
#endif
        case CTC_FIELD_KEY_SVLAN_ID:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
                {
                    field_name[0] = "svlanId";
                }
                else if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g1_svlanId";
                }
                else
                {
                    field_name[0] = "vlanId";
                }
                str = "Svlan ID";
                break;
            }
#if 0
        case CTC_FIELD_KEY_STAG_COS:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "stagCos";
                }
                else
                {
                    field_name[0] = "cos";
                }
                str = "STAG COS";
                break;
            }
        case CTC_FIELD_KEY_STAG_CFI:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "stagCfi";
                }
                else
                {
                    field_name[0] = "cfi";
                }
                str = "STAG CFI";
                break;
            }
        case CTC_FIELD_KEY_CTAG_VALID:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "cvlanIdValid";
                }
                else
                {
                    field_name[0] = "vlanIdValid";
                }
                str = "CTAG Valid";
                break;
            }
#endif
        case CTC_FIELD_KEY_CVLAN_ID:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
                {
                    field_name[0] = "cvlanId";
                }
                else if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g1_cvlanId";
                }
                else
                {
                    field_name[0] = "vlanId";
                }
                str = "Cvlan ID";
                break;
            }
#if 0
        case CTC_FIELD_KEY_CTAG_COS:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "ctagCos";
                }
                else
                {
                    field_name[0] = "cos";
                }
                str = "CTAG COS";
                break;
            }
        case CTC_FIELD_KEY_CTAG_CFI:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "ctagCfi";
                }
                else
                {
                    field_name[0] = "cfi";
                }
                str = "CTAG CFI";
                break;
            }
        case CTC_FIELD_KEY_SVLAN_RANGE:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "svlanId";
                }
                else
                {
                    field_name[0] = "vlanId";
                }
                str = "SVLAN RANGE";
                break;
            }
        case CTC_FIELD_KEY_CVLAN_RANGE:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "cvlanId";
                }
                else
                {
                    field_name[0] = "vlanId";
                }
                str = "CVLAN RANGE";
                break;
            }
        case CTC_FIELD_KEY_ETHER_TYPE:
            {
                if (CTC_ACL_KEY_MAC == pe->key_type)  /* DsAclQosMacKey160 */
                {
                    field_name[0] = "cEtherType";
                }
                else if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)  /* DsAclQosMacKey160 */
                {
                    field_name[0] = "etherType";
                    bit_len[0]   = 7;
                }
                else /* DsFlowL2HashKey  DsAclQosL3Key160 DsAclQosL3Key320	DsAclQosMacL3Key320  DsAclQosMacL3Key640  DsAclQosForwardKey320  DsAclQosForwardKey640 */
                {
                    field_name[0] = "etherType";    /* ????*/
                }
                str = "ETHER TYPE";
                break;
            }
        case CTC_FIELD_KEY_VLAN_NUM:
            field_name[0] = "layer2Type";
            str = "VLAN_NUM";
            break;
        case CTC_FIELD_KEY_STP_STATE:
            field_name[0] = "stpStatus";
            str = "STP STATE";
            break;
#endif
            /*IP packet field*/
        case CTC_FIELD_KEY_IP_SA:
            {
                switch(pe->key_type)
                {
                case CTC_ACL_KEY_CID:            /* ipSa	32 */
                case CTC_ACL_KEY_MAC_IPV4_EXT:
                case CTC_ACL_KEY_MAC_IPV4:
                case CTC_ACL_KEY_IPV4:
                case CTC_ACL_KEY_IPV4_EXT:
                case CTC_ACL_KEY_HASH_IPV4:
                case CTC_ACL_KEY_HASH_L2_L3:
                case CTC_ACL_KEY_FWD_EXT:        /* ipSa 128 0-31 */
                    field_name[0] = "ipSa";      /* CTC_ACL_KEY_FWD_EXT:ipSa 128 0-31; other ipSa 32*/
                    break;
                case CTC_ACL_KEY_COPP:           /* ipAddr  128 0-31 */
                case CTC_ACL_KEY_COPP_EXT:       /* ipAddr  32 */
                    field_name[0] = "ipAddr";
                    break;
                case CTC_ACL_KEY_FWD:            /*ipAddr1  64 0-31 */
                    field_name[0] = "ipAddr2";
                    break;
                default:
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                str = "IP SA";
                break;
            }
        case CTC_FIELD_KEY_IP_DA:
            {
                switch(pe->key_type)
                {
                case CTC_ACL_KEY_CID:            /* ipDa	32 */
                case CTC_ACL_KEY_MAC_IPV4_EXT:
                case CTC_ACL_KEY_MAC_IPV4:
                case CTC_ACL_KEY_IPV4:
                case CTC_ACL_KEY_IPV4_EXT:
                case CTC_ACL_KEY_HASH_IPV4:
                case CTC_ACL_KEY_HASH_L2_L3:
                case CTC_ACL_KEY_FWD_EXT:        /* ipDa	128 0-31 */
                    field_name[0] = "ipDa";      /* CTC_ACL_KEY_FWD_EXT:ipDa	128 0-31; other ipDa 32 */
                    break;
                case CTC_ACL_KEY_COPP:           /* ipAddr  128 0-31 */
                case CTC_ACL_KEY_COPP_EXT:       /* ipAddr  32 */
                    field_name[0] = "ipAddr";
                    break;
                case CTC_ACL_KEY_FWD:            /* ipAddr1  64 0-31 */
                    field_name[0] = "ipAddr1";
                    break;
                default:
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                str = "IP DA";
                break;
            }
        case CTC_FIELD_KEY_IPV6_SA:
            {
                switch(pe->key_type)
                {
                case CTC_ACL_KEY_MAC_IPV6:
                    if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipSa_f))
                    {
                        field_name[0] = "ipSa31To0";
                        bit_len[0]   = 32;
                        field_name[1] = "ipSa127To32";
                        bit_len[1]   = 96;
                        *field_name_cnt = 2;
                        break;
                    }
                case CTC_ACL_KEY_IPV6_EXT:
                case CTC_ACL_KEY_FWD_EXT:
                     /* case CTC_ACL_KEY_HASH_IPV6:*/
                    field_name[0] = "ipSa";          /* AclQosIpv6Key640, MacIpv6Key640, fwd640   128 */
                    break;
                case CTC_ACL_KEY_COPP_EXT:
                    field_name[0] = "ipAddr";           /* copp640  128 */
                    break;
                case CTC_ACL_KEY_FWD:
                case CTC_ACL_KEY_IPV6:
                    field_name[0] = "ipAddr2";       /* fwd320   64 */
                    break;
                case CTC_ACL_KEY_CID:
                    field_name[0] = "u1_g2_ipAddr";    /* cid160   64 ipSa;  u1_g2_ipAddr, u1_g2_ipAddrMode*/
                    break;
                case CTC_ACL_KEY_HASH_IPV6:
                    field_name[0] = "u1_g1_ipSa";    /* AclQosIpv6Key320   64  no need special process */
                    break;
                default:
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                str = "IPV6 SA";
                 *data_type = 2;
                 key_size = sizeof(ipv6_addr_t);
                break;
            }
        case CTC_FIELD_KEY_IPV6_DA:
            {
                switch(pe->key_type)
                {
                case CTC_ACL_KEY_MAC_IPV6:
                    if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipDa_f))
                    {
                        field_name[0] = "ipDa31To0";
                        bit_len[0]   = 32;
                        field_name[1] = "ipDa127To32";
                        bit_len[1]   = 96;
                        *field_name_cnt = 2;
                        break;
                    }
                case CTC_ACL_KEY_IPV6_EXT:
                case CTC_ACL_KEY_FWD_EXT:
                     /* case CTC_ACL_KEY_HASH_IPV6:*/
                    field_name[0] = "ipDa";          /* AclQosIpv6Key640, MacIpv6Key640, fwd640   128 */
                    break;
                case CTC_ACL_KEY_COPP_EXT:
                    field_name[0] = "udf";        /* copp640  128; cid160   64 */
                    break;
                case CTC_ACL_KEY_CID:
                    field_name[0] = "u1_g2_ipAddr";    /* cid160   64; u1_g2_ipAddr, u1_g2_ipAddrMode */
                    break;
                case CTC_ACL_KEY_FWD:
                case CTC_ACL_KEY_IPV6:
                    field_name[0] = "ipAddr1";       /* fwd320   64 */
                    break;
                case CTC_ACL_KEY_HASH_IPV6:
                    field_name[0] = "u1_g1_ipDa";    /* AclQosIpv6Key320   64 */
                    break;
                default:
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                str = "IPV6 DA";
                 *data_type = 2;
                 key_size = sizeof(ipv6_addr_t);
                break;
            }
#if 0
        case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
            if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipv6FlowLabel_f) && CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
            {
                field_name[0] = "ipv6FlowLabelLow";
                bit_len[0]   = 4;
                field_name[1] = "ipv6FlowLabelHigh";
                bit_len[1]   = 16;
                *field_name_cnt = 2;
            }
            else
            {
                field_name[0] = "ipv6FlowLabel";
            }
            str = "IPV6 FLOW LABEL";
            break;
        case CTC_FIELD_KEY_IP_PROTOCOL:
            {
                if (1 == _sys_usw_acl_l4type_ipprotocol_judge(lchip, pe->key_type, pe->hash_field_sel_id))
                {
                    field_name[0] = "layer3HeaderProtocol";
                }
                else
                {
                    field_name[0] = "layer4Type";
                }
                str = "IP PROTOCOL";
                break;
            }
        case CTC_FIELD_KEY_IP_DSCP:
            field_name[0] = "dscp";
            str = "IP DSCP";
            break;
        case CTC_FIELD_KEY_IP_PRECEDENCE:
            field_name[0] = "dscp";
            str = "IP PRECEDENCE";
            break;
        case CTC_FIELD_KEY_IP_ECN:
            field_name[0] = "ecn";
            str = "IP ECN";
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if ((CTC_ACL_KEY_COPP == pe->key_type) && (DRV_FLD_IS_EXISIT(DsAclQosCoppKey320_t, DsAclQosCoppKey320_isFragmentPkt_f)))
            {
                field_name[0] = "isFragmentPkt";
                bit_len[0] = 1;
            }
            else if (CTC_ACL_KEY_UDF == pe->key_type)
            {
                field_name[0] = "isFragPkt";
                bit_len[0] = 1;
            }
            else
            {
                field_name[0] = "fragInfo";
            }
            str = "IP FRAG";
            break;
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            field_name[0] = "ipHeaderError";
            str = "IP HDR ERROR";
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            field_name[0] = "ipOptions";
            str = "IP OPTIONS";
            break;
        case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
            field_name[0] = "rangeBitmap";
            str = "IP PKT LEN RANGE";
            break;
        case CTC_FIELD_KEY_IP_TTL:
            field_name[0] = "ttl";
            str = "IP TTL";
            break;

            /*ARP packet field*/
        case CTC_FIELD_KEY_ARP_OP_CODE:
            field_name[0] = "arpOpCode";
            str = "ARP OP CODE";
            break;
        case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
            field_name[0] = "protocolType";
            str = "ARP PROTOCOLTYPE";
            break;
        case CTC_FIELD_KEY_ARP_SENDER_IP:
            field_name[0] = "senderIp";
            str = "ARP SENDER IP";
            break;
        case CTC_FIELD_KEY_ARP_TARGET_IP:
            field_name[0] = "targetIp";
            str = "ARP TARGET IP";
            break;
        case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "arpAddrCheckFailBitMap";
                    start_bit[0] = 2;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "arpDestMacCheckFail";
                }
                str = "ARP MAC DA CHK FAIL";
                break;
            }
        case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "arpAddrCheckFailBitMap";
                    start_bit[0] = 3;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "arpSrcMacCheckFail";
                }
                str = "ARP MAC SA CHK FAIL";
                break;
            }
        case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "arpAddrCheckFailBitMap";
                    start_bit[0] = 1;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "arpSenderIpCheckFail";
                }
                str = "ARP SENDERIP CHK FAIL";
                break;
            }
        case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "arpAddrCheckFailBitMap";
                    start_bit[0] = 0;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "arpTargetIpCheckFail";
                }
                str = "ARP TARGETIP CHK FAIL";
                break;
            }
        case CTC_FIELD_KEY_GARP:
            field_name[0] = "isGratuitousArp";
            str = "GARP";
            break;
        case CTC_FIELD_KEY_SENDER_MAC:
            {
                if (CTC_ACL_KEY_MAC_IPV4 == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                {
                    field_name[0] = "macSa";
                }
                else if(CTC_ACL_KEY_COPP == pe->key_type )
                {
                    field_name[0] = "macDa";
                }
                else
                {
                    field_name[0] = "senderMac";
                }
                str = "ARP SENDER MAC";
                 *data_type = 2;
                key_size = sizeof(mac_addr_t);
                break;
            }
        case CTC_FIELD_KEY_TARGET_MAC:
            {
                if (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                {
                    field_name[0] = "macDa";
                }
                else
                {
                    field_name[0] = "targetMac";
                }
                str = "ARP SENDER MAC";
                 *data_type = 2;
                key_size = sizeof(mac_addr_t);
                break;
            }
#endif
            /*TCP/UDP packet field*/
        case CTC_FIELD_KEY_L4_DST_PORT:
            field_name[0] = "l4DestPort";
            str = "L4 DST PORT";
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if (CTC_ACL_KEY_UDF == pe->key_type)
            {
                field_name[0] = "l4SrcPort";
            }
            else
            {
                field_name[0] = "l4SourcePort";
            }
            str = "L4 SRC PORT";
            break;
#if 0
        case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
            field_name[0] = "rangeBitmap";
            str = "L4 SRC PORT RANGE";
            break;
        case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
            field_name[0] = "rangeBitmap";
            str = "L4 DST PORT RANGE";
            break;
        case CTC_FIELD_KEY_TCP_ECN:
            {
                bit_len[0]   = 2;
                if (CTC_ACL_KEY_IPV4 == pe->key_type)
                {
                    field_name[0] = "shareFields";
                    start_bit[0] = 0;
                }
                else if(CTC_ACL_KEY_HASH_IPV4 == pe->key_type)
                {
                    sys_hash_sel_field_union_t* p_sel_field_union = &(p_usw_acl_master[lchip]->hash_sel_key_union_filed[SYS_ACL_HASH_KEY_TYPE_IPV4][pe->hash_field_sel_id]);
                    field_name[0] = (p_sel_field_union->l3_key_u1_type == 1) ? "u1_g1_tcpFlags" : "tcpFlags";
                    start_bit[0] = 6;
                }
                else
                {
                    field_name[0] = "tcpEcn";
                }
                str = "TCP ECN";
                break;
            }
        case CTC_FIELD_KEY_TCP_FLAGS:
            {
                bit_len[0]   = 6;
                if (CTC_ACL_KEY_IPV4 == pe->key_type)
                {
                    field_name[0] = "shareFields";
                    start_bit[0] = 2;
                }
                else if(CTC_ACL_KEY_HASH_IPV4 == pe->key_type)
                {
                    sys_hash_sel_field_union_t* p_sel_field_union = &(p_usw_acl_master[lchip]->hash_sel_key_union_filed[SYS_ACL_HASH_KEY_TYPE_IPV4][pe->hash_field_sel_id]);
                    field_name[0] = (p_sel_field_union->l3_key_u1_type == 1) ? "u1_g1_tcpFlags" : "tcpFlags";
                    start_bit[0] = 6;
                }
                else
                {
                    field_name[0] = "tcpFlags";
                }
                str = "TCP FLAGS";
                break;
            }
#endif
            /*GRE packet field*/
        case CTC_FIELD_KEY_GRE_KEY:
            {
                if (CTC_ACL_KEY_HASH_IPV4 == pe->key_type)
                {
                    field_name[0] = "greKey";
                }
                else
                {
                    field_name[0] = "l4DestPort";
                    start_bit[0] = 0;
                    bit_len[0]   = 16;
                    field_name[1] = "l4SourcePort";
                    start_bit[1] = 0;
                    bit_len[1]   = 16;
                    *field_name_cnt = 2;
                }
                str = "GRE KEY";
                break;
            }
#if 0
            /*NVGRE packet field*/
        case CTC_FIELD_KEY_NVGRE_KEY:
            {
                if (!ACL_ENTRY_IS_TCAM(pe->key_type))
                {
                    field_name[0] = "greKey";
                    start_bit[0] = 0;
                    bit_len[0]   = 24;
                }
                else
                {
                    field_name[0] = "l4DestPort";
                    start_bit[0] = 8;
                    bit_len[0]   = 8;
                    field_name[1] = "l4SourcePort";
                    start_bit[1] = 0;
                    bit_len[1]   = 16;
                    *field_name_cnt = 2;
                }
                str = "NVGRE KEY";
                break;
            }
            /*VXLAN packet field*/
        case CTC_FIELD_KEY_VXLAN_FLAGS:
            field_name[0] = "l4SourcePort";
            start_bit[0] = 8;
            bit_len[0]   = 8;
            *field_name_cnt = 1;
            str = "Vxlan Flags";
            break;
#endif
        case CTC_FIELD_KEY_VN_ID:
            {
                if (CTC_ACL_KEY_HASH_IPV4 == pe->key_type || CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_IPV6 == pe->key_type)
                {
                    field_name[0] = "vni";
                }
                else
                {
                    field_name[0] = "l4DestPort";
                    start_bit[0] = 0;
                    bit_len[0]   = 16;
                    field_name[1] = "l4SourcePort";
                    start_bit[1] = 0;
                    bit_len[1]   = 8;
                    *field_name_cnt = 2;
                }
                str = "VN ID";
                break;
            }
#if 0
             /*ICMP packet field*/
        case CTC_FIELD_KEY_ICMP_CODE:
            field_name[0] = "l4SourcePort";
            start_bit[0] = 0;
            bit_len[0]   = 8;
            str = "ICMP CODE";
            break;
        case CTC_FIELD_KEY_ICMP_TYPE:
            field_name[0] = "l4SourcePort";
            start_bit[0] = 8;
            bit_len[0]   = 8;
            str = "ICMP TYPE";
            break;

            /*IGMP packet field*/
        case CTC_FIELD_KEY_IGMP_TYPE:
            field_name[0] = "l4SourcePort";
            start_bit[0] = 8;
            bit_len[0]   = 8;
            str = "IGMP TYPE";
            break;

            /*MPLS packet field*/
        case CTC_FIELD_KEY_LABEL_NUM:
            field_name[0] = "labelNum";
            str = "Mpls Label Num";
            break;
#endif
        case CTC_FIELD_KEY_MPLS_LABEL0:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsLabel0";
            }
            else
            {
                field_name[0] = "mplsLabel0";
                start_bit[0] = 12;
                bit_len[0]   = 20;
            }
            str = "Mpls Label0";
            break;
#if 0
        case CTC_FIELD_KEY_MPLS_EXP0:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsExp0";
            }
            else
            {
                field_name[0] = "mplsLabel0";
                start_bit[0] = 9;
                bit_len[0]   = 3;
            }
            str = "Mpls Exp0";
            break;
        case CTC_FIELD_KEY_MPLS_SBIT0:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsSbit0";
            }
            else
            {
                field_name[0] = "mplsLabel0";

                start_bit[0] = 8;
                bit_len[0]   = 1;
            }
            str = "Mpls Sbit0";
            break;
        case CTC_FIELD_KEY_MPLS_TTL0:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsTtl0";
            }
            else
            {
                field_name[0] = "mplsLabel0";
                start_bit[0] = 0;
                bit_len[0]   = 8;
            }
            str = "Mpls TTL0";
            break;
#endif
        case CTC_FIELD_KEY_MPLS_LABEL1:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsLabel1";
            }
            else
            {
                field_name[0] = "mplsLabel1";
                start_bit[0] = 12;
                bit_len[0]   = 20;
            }
            str = "Mpls Label1";
            break;
#if 0
        case CTC_FIELD_KEY_MPLS_EXP1:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsExp1";
            }
            else
            {
                field_name[0] = "mplsLabel1";
                start_bit[0] = 9;
                bit_len[0]   = 3;
            }
            str = "Mpls Exp1";
            break;
        case CTC_FIELD_KEY_MPLS_SBIT1:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsSbit1";
            }
            else
            {
                field_name[0] = "mplsLabel1";
                start_bit[0] = 8;
                bit_len[0]   = 1;
            }
            str = "Mpls Sbit1";
            break;
        case CTC_FIELD_KEY_MPLS_TTL1:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsTtl1";
            }
            else
            {
                field_name[0] = "mplsLabel1";
                start_bit[0] = 0;
                bit_len[0]   = 8;
            }
            str = "Mpls TTL1";
            break;
#endif
        case CTC_FIELD_KEY_MPLS_LABEL2:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsLabel2";
            }
            else
            {
                field_name[0] = "mplsLabel2";
                start_bit[0] = 12;
                bit_len[0]   = 20;
            }
            str = "Mpls Label2";
            break;
#if 0
        case CTC_FIELD_KEY_MPLS_EXP2:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsExp2";
            }
            else
            {
                field_name[0] = "mplsLabel2";
                start_bit[0] = 9;
                bit_len[0]   = 3;
            }
            str = "Mpls Exp2";
            break;
        case CTC_FIELD_KEY_MPLS_SBIT2:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsSbit2";
            }
            else
            {
                field_name[0] = "mplsLabel2";
                start_bit[0] = 8;
                bit_len[0]   = 1;
            }
            str = "Mpls Sbit2";
            break;
        case CTC_FIELD_KEY_MPLS_TTL2:
            if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type || CTC_ACL_KEY_HASH_MPLS == pe->key_type)
            {
                field_name[0] = "mplsTtl2";
            }
            else
            {
                field_name[0] = "mplsLabel2";
                start_bit[0] = 0;
                bit_len[0]   = 8;
            }
            str = "Mpls TTL2";
            break;

        case  CTC_FIELD_KEY_INTERFACE_ID:
            if (!DRV_IS_DUET2(lchip) && CTC_ACL_KEY_FWD == pe->key_type)
            {
                field_name[0] = "l3InterfaceId";
                bit_len[0] = 10;
                start_bit[0] = 0;
                field_name[1] = "layer2Type";
                bit_len[1] = 2;
                start_bit[1] = 0;
                *field_name_cnt = 2;
            }
            else
            {
                field_name[0] = "l3InterfaceId";
            }
            str = "INTERFACE ID";
            break;
        case CTC_FIELD_KEY_NSH_CBIT/*Network Service Header (NSH)*/:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
                    start_bit[0] = 41;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "nshCbit";
                }
                str = "NSH CBIT";
                break;
            }
        case CTC_FIELD_KEY_NSH_OBIT:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
                    start_bit[0] = 40;
                    bit_len[0]   = 1;
                }
                else
                {
                    field_name[0] = "nshObit";
                }
                str = "NSH OBIT";
                break;
            }
        case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
                    start_bit[0] = 8;
                    bit_len[0]   = 8;
                }
                else
                {
                    field_name[0] = "nshNextProtocol";
                }
                str = "NSH NEXT PROTOCOL";
                break;
            }
        case CTC_FIELD_KEY_NSH_SI:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
                    start_bit[0] = 0;
                    bit_len[0]   = 8;
                }
                else
                {
                    field_name[0] = "nshSi";
                }
                str = "NSH SI";
                break;
            }
        case CTC_FIELD_KEY_NSH_SPI:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
                    start_bit[0] = 16;
                    bit_len[0]   = 24;
                }
                else
                {
                    field_name[0] = "nshSpi";
                }
                str = "NSH SPI";
                break;
            }
        case CTC_FIELD_KEY_IS_Y1731_OAM/*Ether OAM packet field*/:
            field_name[0] = "isY1731Oam";
            str = "Y1731 OAM";
            break;
        case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
            {
                if(!CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_IS_Y1731_OAM))
                {
                    if(pe->key_type == CTC_ACL_KEY_COPP_EXT || pe->key_type == CTC_ACL_KEY_FWD_EXT)
                    {
                        field_name[0] = "u2_g7_etherOamLevel";
                    }
                    else
                    {
                        field_name[0] = "u1_g7_etherOamLevel";
                    }
                }
                else
                {
                    field_name[0] = "etherOamLevel";
                }
                str = "ETHER OAM LEVEL";
                break;
            }
        case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
            {
                if(!CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_IS_Y1731_OAM))
                {
                    if(pe->key_type == CTC_ACL_KEY_COPP_EXT || pe->key_type == CTC_ACL_KEY_FWD_EXT)
                    {
                        field_name[0] = "u2_g7_etherOamOpCode";
                    }
                    else
                    {
                        field_name[0] = "u1_g7_etherOamOpCode";
                    }
                }
                else
                {
                    field_name[0] = "etherOamOpCode";
                }
                str = "ETHER OAM OP CODE";
                break;
            }
        case CTC_FIELD_KEY_ETHER_OAM_VERSION:
            {
                if(!CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_IS_Y1731_OAM))
                {
                    if(pe->key_type == CTC_ACL_KEY_COPP_EXT || pe->key_type == CTC_ACL_KEY_FWD_EXT)
                    {
                        field_name[0] = "u2_g7_etherOamVersion";
                    }
                    else
                    {
                        field_name[0] = "u1_g7_etherOamVersion";
                    }
                }
                else
                {
                    field_name[0] = "etherOamVersion";
                }
                str = "ETHER OAM VERSION";
                break;
            }
            /*Slow Protocol packet field*/
        case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
            field_name[0] = "slowProtocolCode";
            str = "SLOW PROTOCOL CODE";
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
            field_name[0] = "slowProtocolFlags";
            str = "SLOW PROTOCOL FLAGS";
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
            field_name[0] = "slowProtocolSubType";
            str = "SLOW PROTOCOL SUB TYPE";
            break;

            /*PTP packet field*/
        case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
            field_name[0] = "ptpMessageType";
            str = "PTP MESSAGE TYPE";
            break;
        case CTC_FIELD_KEY_PTP_VERSION:
            field_name[0] = "ptpVersion";
            str = "PTP VERSION";
            break;

            /*FCoE packet field*/
        case CTC_FIELD_KEY_FCOE_DST_FCID:
            field_name[0] = "fcoeDid";
            str = "FCOE DST FCID";
            break;
        case CTC_FIELD_KEY_FCOE_SRC_FCID:
            field_name[0] = "fcoeSid";
            str = "FCOE SRC FCID";
            break;

            /*Wlan packet field*/
        case CTC_FIELD_KEY_WLAN_RADIO_MAC:
            field_name[0] = "radioMac";
            str = "WLAN RADIO MAC";
             *data_type = 2;
            key_size = sizeof(mac_addr_t);
            break;
        case CTC_FIELD_KEY_WLAN_RADIO_ID:
            field_name[0] = "rid";
            str = "WLAN RADIO ID";
            break;
        case CTC_FIELD_KEY_WLAN_CTL_PKT:
            field_name[0] = "capwapControlPacket";
            str = "WLAN CTL PKT";
            break;

            /*SATPDU packet field*/
        case CTC_FIELD_KEY_SATPDU_MEF_OUI:
            field_name[0] = "mefOui";
            str = "SATPDU MEF OUI";
            break;
        case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
            field_name[0] = "ouiSubType";
            str = "SATPDU OUI SUB TYPE";
            break;
        case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
            field_name[0] = "pduByte";
            str = "SATPDU PDU BYTE";
             *data_type = 2;
            key_size = 2*sizeof(uint32);
            break;

            /*TRILL packet field*/
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            field_name[0] = "ingressNickname";
            str = "INGRESS NICKNAME";
            break;
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            field_name[0] = "egressNickname";
            str = "EGRESS NICKNAME";
            break;
        case CTC_FIELD_KEY_IS_ESADI:
            field_name[0] = "isEsadi";
            str = "ESADI";
            break;
        case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
            field_name[0] = "isTrillChannel";
            str = "TRILL CHANNEL";
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID:
            field_name[0] = "trillInnerVlanId";
            str = "TRILL INNER VLANID";
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
            field_name[0] = "trillInnerVlanValid";
            str = "TRILL INNER VLANID VALID";
            break;
        case CTC_FIELD_KEY_TRILL_LENGTH:
            field_name[0] = "trillLength";
            str = "TRILL LENGTH";
            break;
        case CTC_FIELD_KEY_TRILL_MULTIHOP:
            field_name[0] = "trillMultiHop";
            str = "TRILL MULTIHOP";
            break;
        case CTC_FIELD_KEY_TRILL_MULTICAST:
            field_name[0] = "trillMulticast";
            str = "TRILL MULTICAST";
            break;
        case CTC_FIELD_KEY_TRILL_VERSION:
            field_name[0] = "trillVersion";
            str = "TRILL VERSION";
            break;
        case CTC_FIELD_KEY_TRILL_TTL:
            {
                if (CTC_ACL_KEY_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                {
                    field_name[0] = "u1_g5_ttl";
                }
                else if(CTC_ACL_KEY_FWD_EXT == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    field_name[0] = "u2_g5_ttl";
                }
                else
                {
                    field_name[0] = "ttl";
                }
                str = "TRILL TTL";
                break;
            }

            /*802.1AE packet field*/
        case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
            field_name[0] = "unknownDot1AePacket";
            str = "DOT1AE UNKNOW PKT";
            break;
        case CTC_FIELD_KEY_DOT1AE_ES:
            field_name[0] = "secTagEs";
            str = "DOT1AE ES";
            break;
        case CTC_FIELD_KEY_DOT1AE_SC:
            field_name[0] = "secTagSc";
            str = "DOT1AE SC";
            break;
        case CTC_FIELD_KEY_DOT1AE_AN:
            field_name[0] = "secTagAn";
            str = "DOT1AE AN";
            break;
        case CTC_FIELD_KEY_DOT1AE_SL:
            field_name[0] = "secTagSl";
            str = "DOT1AE SL";
            break;
        case CTC_FIELD_KEY_DOT1AE_PN:
            field_name[0] = "secTagPn";
            str = "DOT1AE PN";
            break;
        case CTC_FIELD_KEY_DOT1AE_SCI:
            field_name[0] = "secTagSci";
            str = "DOT1AE SCI";
            *data_type = 2;
            key_size = 2*sizeof(uint32);
            break;
        case CTC_FIELD_KEY_DOT1AE_CBIT:
            field_name[0] = "secTagCbit";
            str = "DOT1AE Cbit";
            break;
        case CTC_FIELD_KEY_DOT1AE_EBIT:
            field_name[0] = "secTagEbit";
            str = "DOT1AE Ebit";
            break;
        case CTC_FIELD_KEY_DOT1AE_SCB:
            field_name[0] = "secTagScb";
            str = "DOT1AE Scb";
            break;
        case CTC_FIELD_KEY_DOT1AE_VER:
            field_name[0] = "secTagVer";
            str = "DOT1AE Ver";
            break;
            /*UDFuser defined field*/
        case CTC_FIELD_KEY_UDF:
            if (!DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_UDF == pe->key_type))
            {
                field_name[0] = "udf";
            }
            else
            {
                field_name[0] = "u1_g2_udf"; /*u1_g2_udfHitIndex, u1_g2_udfValid, u1_g2_udf */
            }
            str = "UDF";
             *data_type = 2;
            key_size = sizeof(ctc_acl_udf_t);
            break;
#endif
            /*Match port type*/
        case CTC_FIELD_KEY_PORT:
            {
                if (CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                {
                    *data_type = 2;  /*special process */
                    field_name[1] = "portBase";
                    bit_len[1]   = 1;
                    field_name[0] = "portBitmap";
                    bit_len[0]   = 64;
                    *field_name_cnt = 2;
                    str = "Port Bitmap(Base/Value)";
                    break;
                }
                if (CTC_ACL_KEY_UDF == pe->key_type)
                {
                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "aclLabel");
                    DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->mask, drv_data_temp);
                    if (0 != drv_data_temp[0])
                    {
                        field_name[0] = "aclLabel";
                        str = "Class ID";
                        break;
                    }
                    else
                    {
                        drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "logicPort");
                        DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->mask, drv_data_temp);
                        if (0 != drv_data_temp[0])
                        {
                            field_name[0] = "logicPort";
                            str = "Logic Port";
                            break;
                        }
                        else
                        {
                            *data_type = 2;  /*special process */
                            field_name[1] = "portBase";
                            bit_len[1]   = 1;
                            field_name[0] = "portBitmap";
                            bit_len[0]   = 64;
                            *field_name_cnt = 2;
                            str = "Port Bitmap(Base/Value)";
                            break;
                        }
                    }
                }
                else
                {
                    *data_type = 1;  /*special process */
                    if (is_hash)
                    {
                        if (1 == _sys_usw_acl_hash_port_type_judge(lchip, pe->key_type, pe->hash_field_sel_id))
                        {
                            field_name[0] = "globalSrcPort";
                            str = "Gport";
                            break;
                        }
                        else if(2 == _sys_usw_acl_hash_port_type_judge(lchip, pe->key_type, pe->hash_field_sel_id))
                        {
                            field_name[0] = "globalSrcPort";
                            str = "Logical port";
                            break;
                        }
                    }
                    else
                    {
                        drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "aclLabel");
                        DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->mask, drv_data_temp);
                        if (0 != drv_data_temp[0])
                        {
                            field_name[0] = "aclLabel";
                            str = "Class ID";
                            break;
                        }
                        else
                        {
                            drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "globalPortType");
                            DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                            if ((DRV_FLOWPORTTYPE_GPORT == drv_data_temp[0])
                                || (DRV_FLOWPORTTYPE_LPORT == drv_data_temp[0]))
                            {
                                if (DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type))
                                {
                                    field_name[1] = "globalPort15To8";
                                    bit_len[1]   = 8;
                                    field_name[0] = "globalPort7To0";
                                    bit_len[0]   = 8;
                                    *field_name_cnt = 2;
                                }
                                else
                                {
                                    field_name[0] = "globalPort";
                                }
                                str = (DRV_FLOWPORTTYPE_GPORT == drv_data_temp[0])?"Gport":"Logical port";
                                break;
                            }
                            else if(DRV_FLOWPORTTYPE_BITMAP == drv_data_temp[0])
                            {
                                if (DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type))
                                {
                                    field_name[2] = "globalPort15To8";
                                    bit_len[2]   = 8;
                                    field_name[1] = "globalPort7To0";
                                    bit_len[1]   = 8;
                                    field_name[0] = "portBitmap";
                                    bit_len[0]   = 3;
                                    *field_name_cnt = 3;
                                }
                                else if(CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type)
                                {
                                    field_name[1] = "portBase";
                                    bit_len[1]   = 1;
                                    field_name[0] = "portBitmap";
                                    bit_len[0]   = 64;
                                    *field_name_cnt = 2;
                                }
                                else
                                {
                                    field_name[1] = "globalPort";
                                    bit_len[1]   = 16;
                                    field_name[0] = "portBitmap";
                                    bit_len[0]   = 3;
                                    *field_name_cnt = 2;
                                }
                                str = "Port Bitmap(Base/Value)";
                                break;
                            }
                            else if (!DRV_IS_DUET2(lchip))
                            {
                                drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "portBitmap");
                                DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                                if (0 != drv_data_temp[0])
                                {
                                    field_name[1] = "portBase";
                                    bit_len[1] = 1;
                                    field_name[0] = "portBitmap";
                                    bit_len[0] = 64;
                                    *field_name_cnt = 2;
                                }
                                else
                                {
                                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "logicPort");
                                    DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                                    if (0 != drv_data_temp[0])
                                    {
                                        field_name[0] = "logicPort";
                                        *field_name_cnt = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            key_size = sizeof(ctc_field_port_t);
            break;
        case CTC_FIELD_KEY_CLASS_ID:
            field_name[0] = "aclLabel";
            str = "Class ID";
            /*Forwarding information*/
            break;
#if 0
        case CTC_FIELD_KEY_DECAP:
            field_name[0] = "isDecap";
            str = "DECAP";
            break;
        case CTC_FIELD_KEY_ELEPHANT_PKT:
            field_name[0] = "isElephant";
            str = "ELEPHANT PKT";
            break;
        case CTC_FIELD_KEY_VXLAN_PKT:
            {
                if (is_hash)
                {
                    field_name[0] = "isVxlan";
                }
                else
                {
                    field_name[0] = "isVxlanPkt";
                }
                str = "VXLAN PKT";
                break;
            }
        case CTC_FIELD_KEY_ROUTED_PKT:
            {
                if (is_hash)
                {
                    field_name[0] = "ipRoutedPacket";
                }
                else
                {
                    field_name[0] = "routedPacket";
                }
                str = "ROUTED PKT";
                break;
            }
        case CTC_FIELD_KEY_PKT_FWD_TYPE:    /*/????????*/
            field_name[0] = "pktForwardingType";
            str = "FWD TYPE";
            break;
        case CTC_FIELD_KEY_MACSA_LKUP:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 7;
            bit_len[0]   = 1;
            str = "MACSA LKUP";
            break;
        case CTC_FIELD_KEY_MACSA_HIT:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 6;
            bit_len[0]   = 1;
            str = "MACSA HIT";
            break;
        case CTC_FIELD_KEY_MACDA_LKUP:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 5;
            bit_len[0]   = 1;
            str = "MACDA LKUP";
            break;
        case CTC_FIELD_KEY_MACDA_HIT:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 4;
            bit_len[0]   = 1;
            str = "MACDA HIT";
            break;
        case CTC_FIELD_KEY_IPSA_LKUP:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 3;
            bit_len[0]   = 1;
            str = "IPSA LKUP";
            break;
        case CTC_FIELD_KEY_IPSA_HIT:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 2;
            bit_len[0]   = 1;
            str = "IPSA HIT";
            break;
        case CTC_FIELD_KEY_IPDA_LKUP:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 1;
            bit_len[0]   = 1;
            str = "IPDA LKUP";
            break;
        case CTC_FIELD_KEY_IPDA_HIT:
            field_name[0] = "lookupStatusInfo";
            start_bit[0] = 0;
            bit_len[0]   = 1;
            str = "IPDA HIT";
            break;
        case CTC_FIELD_KEY_L2_STATION_MOVE:
            field_name[0] = "l2StationMove";
            str = "L2 STATION MOVE";
            break;
        case CTC_FIELD_KEY_MAC_SECURITY_DISCARD:
            field_name[0] = "macSecurityDiscard";
            str = "MAC SECURITY DISCARD";
            break;

        case CTC_FIELD_KEY_DST_CID:
            {
                if (CTC_ACL_KEY_CID == pe->key_type)
                {
                    field_name[0] = "dstCategoryId";
                }
                else if (DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type))
                {
                    field_name[0] = "globalPort7To0";
                }
                else
                {
                    field_name[0] = "globalPort";
                    start_bit[0] = 0;
                    bit_len[0]   = 8;
                }
                str = "DST CID";
                break;
            }
        case CTC_FIELD_KEY_SRC_CID:
            {
                if (CTC_ACL_KEY_CID == pe->key_type)
                {
                    field_name[0] = "srcCategoryId";
                }
                else if (DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type))
                {
                    field_name[0] = "globalPort15To8";
                }
                else
                {
                    field_name[0] = "globalPort";
                    start_bit[0] = 8;
                    bit_len[0]   = 8;
                }
                str = "SRC CID";
                break;
            }

        case CTC_FIELD_KEY_DISCARD:   /* ?? special process*/
            if ((CTC_ACL_KEY_INTERFACE == pe->key_type) && DRV_FLD_IS_EXISIT(DsAclQosKey80_t, DsAclQosKey80_discard_f))
            {
                field_name[0] = "discard";
            }
            else
            {
                field_name[0] = "discardInfo";
            }
            str = "DISCARD Info";
            break;
        case CTC_FIELD_KEY_CPU_REASON_ID: /*record in DB */
            str = "CPU REASON ID";
            break;
#endif
        case CTC_FIELD_KEY_DST_GPORT:
            field_name[0] = "destMap";
            str = "DST GPORT";
            break;
#if 0
        case CTC_FIELD_KEY_DST_NHID:     /*record in DB */
            str = "DST NHID";
            break;

        case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
            field_name[0] = "isMcastRpfCheckFail";
            str = "MCAST RPF CHECK FAIL";
            break;

        case CTC_FIELD_KEY_VRFID:
            {
                if (CTC_ACL_KEY_HASH_L2_L3 == pe->key_type)
                {
                    field_name[0] = "vrfId9_0";
                }
                else
                {
                    field_name[0] = "vrfId";
                }
                str = "VRFID";
                break;
            }
        case CTC_FIELD_KEY_GEM_PORT:
            if (CTC_ACL_KEY_UDF == pe->key_type)
            {
                field_name[0] = "metadata";
                field_name[1] = "metadataType";
                bit_len[0] = 14;
                bit_len[1] = 2;
                *field_name_cnt = 2;
            }
            else
            {
                field_name[0] = "globalPort";
            }
            str = "Gem Port";
            break;
        case CTC_FIELD_KEY_METADATA:
            {
                if (is_hash)   /* need special process*/
                {
                    field_name[0] = "globalSrcPort";
                }
                else if (DRV_IS_DUET2(lchip) && (CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type || CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type || CTC_ACL_KEY_FWD_EXT == pe->key_type))
                {
                    field_name[0] = "globalPort7To0";
                    bit_len[0]   = 8;
                    field_name[1] = "globalPort15To8";
                    bit_len[1]   = 8;
                    *field_name_cnt = 2;
                }
                else if (CTC_ACL_KEY_UDF == pe->key_type)
                {
                    field_name[0] = "metadata";
                }
                else
                {
                    field_name[0] = "globalPort";
                }
                str = "METADATA";
                break;
            }

            /*Merge Key  1: Vxlan; 2: Gre; 3:Wlan*/
        case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
            {
                if (CTC_ACL_KEY_FWD_EXT == pe->key_type)
                { /* (ipDa 128 90-91 2 mergeDataType, 36-89 54 mergeData  ForwardKey640) */
                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "ipDa");
                    DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                    if (1 == (drv_data_temp[2]>>26 & 0x3))
                    {
                        *data_type = 1;  /*special process */
                        field_name[0] = "ipDa";
                        start_bit[0] = 36;
                        bit_len[0]   = 24;
                        str = "AWARE TUNNEL INFO: Vxlan Vni";
                        break;
                    }
                    else if(2 == (drv_data_temp[2]>>26 & 0x3))
                    {
                        *data_type = 1;  /*special process */
                        field_name[0] = "ipDa";
                        start_bit[0] = 36;
                        bit_len[0]   = 32;
                        str = "AWARE TUNNEL INFO: Gre";
                        break;
                    }
                    else if(3 == (drv_data_temp[2]>>26 & 0x3))
                    {
                        *data_type = 2;  /*special process */
                        field_name[0] = "ipDa";
                        start_bit[0] = 36;
                        bit_len[0]   = 54;
                        str = "AWARE TUNNEL INFO: Wlan";
                        break;
                    }
                }
                else if(CTC_ACL_KEY_MAC_IPV4 == pe->key_type)
                { /* (cvlanId 12 6-7 2 type, 5 1 wlan_ctl_pkt, 0-4 5 radio_id,  macDa MacL3Key320) */
                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "cvlanId");
                    DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                    if (1 == (drv_data_temp[0]>>6 & 0x3))
                    {
                        *data_type = 1;  /*special process */
                        field_name[0] = "macDa";
                        bit_len[0]   = 24;
                        str = "AWARE TUNNEL INFO: Vxlan Vni";
                        break;
                    }
                    else if(2 == (drv_data_temp[0]>>6 & 0x3))
                    {
                        *data_type = 1;  /*special process */
                        field_name[0] = "macDa";
                        bit_len[0]   = 32;
                        str = "AWARE TUNNEL INFO: Gre";
                        break;
                    }
                    else if(3 == (drv_data_temp[0]>>6 & 0x3))
                    {
                        *data_type = 2;  /*special process */
                        field_name[0] = "macDa";
                        start_bit[0] = 0;
                        bit_len[0]   = 48;
                        field_name[1] = "cvlanId";
                        start_bit[1] = 0;
                        bit_len[1]   = 5;
                        *field_name_cnt = 2;
                        str = "AWARE TUNNEL INFO: Wlan";
                        break;
                    }
                }
                else
                {
                    /* IpeLkpMgrToIpeFwdPI.gAcl.mergeDataType(1,0) = CBit(2, 'b', "01", 2);
                    IpeLkpMgrToIpeFwdPI.gAcl.mergeData(53, 0) = (CBit(30, 'd', "0", 1), ParserResult1.uL4UserData.gVxlan.vxlanVni(23, 0));

                    IpeLkpMgrToIpeFwdPI.gAcl.mergeDataType(1, 0) = CBit(2, 'b', "10", 2);
                    IpeLkpMgrToIpeFwdPI.gAcl.mergeData(53, 0) = (CBit(22, 'd', "0", 1), ParserResult1.uL4UserData.gGre.greKey(31, 0));

                    IpeLkpMgrToIpeFwdPI.gAcl.mergeData(47, 0) = ParserResult1.uL4UserData.gCapwap.radioMac(47, 0);
                    IpeLkpMgrToIpeFwdPI.gAcl.mergeData(53, 48) = (ParserResult1.uL4UserData.gCapwap.capwapControlPacket, ParserResult1.uL4UserData.gCapwap.rid(4, 0));
                    */

                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, "mergeDataType");
                    DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
                    if (1 == drv_data_temp[0])
                    {
                        if (CTC_ACL_KEY_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                        {
                            field_name[0] = "u2_gMergeData_mergeData";
                        }
                        else   /* CTC_ACL_KEY_IPV6_EXT, CTC_ACL_KEY_MAC_IPV6*/
                        {
                            field_name[0] = "u1_gMergeData_mergeData";
                        }
                        bit_len[0]   = 24;
                        *data_type = 1;  /*special process */
                        str = "AWARE TUNNEL INFO: Vxlan Vni";
                        break;
                    }
                    else if(2 == drv_data_temp[0])
                    {
                        if (CTC_ACL_KEY_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                        {
                            field_name[0] = "u2_gMergeData_mergeData";
                        }
                        else   /* CTC_ACL_KEY_IPV6_EXT, CTC_ACL_KEY_MAC_IPV6*/
                        {
                            field_name[0] = "u1_gMergeData_mergeData";
                        }
                        bit_len[0]   = 32;
                        *data_type = 1;  /*special process */
                        str = "AWARE TUNNEL INFO: Gre";
                        break;
                    }
                    else if(3 == drv_data_temp[0])
                    {
                        if (CTC_ACL_KEY_IPV4_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV4_EXT == pe->key_type)
                        {
                            field_name[0] = "u2_gMergeData_mergeData";
                        }
                        else   /* CTC_ACL_KEY_IPV6_EXT, CTC_ACL_KEY_MAC_IPV6*/
                        {
                            field_name[0] = "u1_gMergeData_mergeData";
                        }
                        bit_len[0]   = 54;
                        *data_type = 2;  /*special process */
                        str = "AWARE TUNNEL INFO: Wlan";
                        break;
                    }
                }
            }
            key_size=sizeof(ctc_acl_tunnel_data_t);
            break;
            /*HASH key field*/
        case CTC_FIELD_KEY_IS_ROUTER_MAC:
            field_name[0] = "isRouteMac";
            str = "IS ROUTER MAC";
            break;

            /*Other*/
        case CTC_FIELD_KEY_IS_LOG_PKT:
            field_name[0] = "isLogToCpuEn";
            str = "LOG PKT";
            break;
#endif
        default:
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
    }
    if(print_str)
    {
        *print_str = str;
    }
    if(p_key_size)
    {
        *p_key_size  = key_size;
    }
    return CTC_E_NONE;
}

/* return: 1   not add the key field */
int32
_sys_usw_acl_get_key_field_value(uint8 lchip, sys_acl_entry_t* pe, uint16 key_field_type, uint32 key_id, ctc_field_key_t* p_field_key, char** print_str, uint32* key_size)
{
    char* field_name[4] = {NULL};
    fld_id_t field_id;
    uint8 start_bit[4] = {0};
    uint8 bit_len[4] = {0};
    uint8 field_name_cnt = 0;
    uint32 drv_data_temp[4] = {0,0,0,0};
    uint32 drv_mask_temp[4] = {0,0,0,0};
    uint32 drv_ext_data_temp[4] = {0,0,0,0};
    uint32 hw_ipv6_64[2] = {0};
    uint32 shift = 0;
    int field_name_i= 0;
    uint8 data_type = 0;
    ctc_field_port_t field_port;
    uint32 temp_field_data = 0;
    uint32 temp_field_mask = 0;
#if 0
    uint32 drv_ext_mask_temp[4] = {0,0,0,0};
    uint32 ipprotocol_data = 0;
    uint32 ipprotocol_mask = 0;
    uint32 temp_data = 0;
    uint32 temp_mask = 0;
    uint8 ctc_ip_frag = 0;
    uint8 ctc_ip_frag_mask = 0;
    uint8 is_tcam = ACL_ENTRY_IS_TCAM(pe->key_type);
    uint32  cmd     = 0;
    ParserRangeOpCtl_m parser_range;
    uint8 loop_i =0;
#endif
    CTC_PTR_VALID_CHECK(pe->buffer);

    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    if(CTC_ACL_KEY_CID == pe->key_type)  /* CTC_ACL_KEY_CID not support CTC_FIELD_KEY_L4_USER_TYPE */
    {
        CTC_BMP_UNSET(pe->key_bmp, CTC_FIELD_KEY_L4_USER_TYPE);
    }

    if(!CTC_BMP_ISSET(pe->key_bmp, key_field_type))
    {
        return 1;/*not exit*/
    }
    sys_usw_acl_map_key_field_to_field_name(lchip, pe, key_field_type, key_id,&data_type, field_name, start_bit, bit_len, &field_name_cnt, print_str, key_size);
#if 0
    if(((CTC_ACL_KEY_IPV6 == pe->key_type) || (CTC_ACL_KEY_IPV6_EXT == pe->key_type) || (CTC_ACL_KEY_MAC_IPV6 == pe->key_type)) && (CTC_FIELD_KEY_L3_TYPE == key_field_type))
    {
        p_field_key->data = pe->l3_type;
        p_field_key->mask = 0xFF;
        return CTC_E_NONE;
    }
#endif
    p_field_key->data = 0;
    p_field_key->mask = 0;

    for (field_name_i = 0; field_name_i < field_name_cnt; field_name_i++)
    {
        sal_memset(drv_data_temp, 0, sizeof(drv_data_temp));
        sal_memset(drv_mask_temp, 0, sizeof(drv_mask_temp));
#if 0
        if (CTC_FIELD_KEY_DST_NHID == key_field_type)           /* 32bit */
        {
            p_field_key->data = pe->key_dst_nhid;
            p_field_key->mask = 0xFFFF;
            break;
        }
        else if (CTC_FIELD_KEY_CPU_REASON_ID == key_field_type) /* 16bit */
        {
            p_field_key->data = pe->key_reason_id;
            p_field_key->mask = 0xFF;
            break;
        }
#endif
        if ((NULL == field_name[field_name_i]))
        {
            break;
        }

        drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, field_name[field_name_i]);
        DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->key, drv_data_temp);
        DRV_GET_FIELD(lchip, key_id, field_id, pe->buffer->mask, drv_mask_temp);

        if (!DRV_IS_DUET2(lchip) && CTC_ACL_KEY_MAC_IPV6 == pe->key_type && (CTC_FIELD_KEY_IPV6_SA == key_field_type || CTC_FIELD_KEY_IPV6_DA == key_field_type))
        {
            if (!field_name_i)
            {
                temp_field_data = drv_data_temp[0];
                temp_field_mask = drv_mask_temp[0];
                continue;
            }
            else
            {
                drv_data_temp[3] = drv_data_temp[2];
                drv_data_temp[2] = drv_data_temp[1];
                drv_data_temp[1] = drv_data_temp[0];
                drv_data_temp[0] = temp_field_data;
                drv_mask_temp[3] = drv_mask_temp[2];
                drv_mask_temp[2] = drv_mask_temp[1];
                drv_mask_temp[1] = drv_mask_temp[0];
                drv_mask_temp[0] = temp_field_mask;
            }
        }

        if (1 == data_type)
        {
#if 0
            if (CTC_FIELD_KEY_VXLAN_RSV1 == key_field_type)
            {
                if ((CTC_ACL_KEY_FWD == pe->key_type) || (CTC_ACL_KEY_FWD_EXT == pe->key_type))
                {
                    p_field_key->data = drv_data_temp[1] & 0xFFFFFF;
                    p_field_key->mask = drv_mask_temp[1] & 0xFFFFFF;
                }
                else if (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
                {
                    p_field_key->data = drv_data_temp[0] & 0xFFFFFF;
                    p_field_key->mask = drv_mask_temp[0] & 0xFFFFFF;
                }
                else
                {
                    p_field_key->data = drv_data_temp[0];
                    p_field_key->mask = drv_mask_temp[0];
                }
            }
            else if (CTC_FIELD_KEY_VXLAN_RSV2 == key_field_type)
            {
                if ((CTC_ACL_KEY_FWD == pe->key_type) || (CTC_ACL_KEY_FWD_EXT == pe->key_type))
                {
                    p_field_key->data = drv_data_temp[1] >> 24;
                    p_field_key->mask = drv_mask_temp[1] >> 24;
                }
                else if (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
                {
                    p_field_key->data = drv_data_temp[0] >> 24;
                    p_field_key->mask = drv_mask_temp[0] >> 24;
                }
                else
                {
                    p_field_key->data = drv_data_temp[0];
                    p_field_key->mask = drv_mask_temp[0];
                }
            }
            else if ((CTC_FIELD_KEY_VLAN_XLATE_HIT == key_field_type) && (CTC_ACL_KEY_MAC == pe->key_type))
            {
                p_field_key->data |= drv_data_temp[0]<<field_name_i;
                p_field_key->mask |= drv_mask_temp[0]<<field_name_i;
            }
            else
#endif
            if((CTC_FIELD_KEY_IP_SA==key_field_type) && (CTC_ACL_KEY_COPP_EXT==pe->key_type)) /*DsAclQosCoppKey640; CTC_FIELD_KEY_IP_SA:ipAddr 128 32-63; CTC_FIELD_KEY_IP_DA: ipAddr  128 0-31   */
            {
                p_field_key->data = drv_data_temp[1];
                p_field_key->mask = drv_mask_temp[1];
            }
#if 0
            /* CTC_FIELD_KEY_NSH_CBIT           nshCbit         1        ( udf 41    1  ForwardKey640 )
               CTC_FIELD_KEY_NSH_NEXT_PROTOCOL  nshNextProtocol 8        ( udf 8-15  8  ForwardKey640 )
               CTC_FIELD_KEY_NSH_OBIT           nshObit         1        ( udf 40    1  ForwardKey640 )
               CTC_FIELD_KEY_NSH_SI             nshSi           8        ( udf 0-7   8  ForwardKey640 )
               CTC_FIELD_KEY_NSH_SPI            nshSpi          24       ( udf 16-39 24 ForwardKey640 ) */
            else if((CTC_ACL_KEY_FWD_EXT==pe->key_type)
                &&(CTC_FIELD_KEY_NSH_CBIT==key_field_type || CTC_FIELD_KEY_NSH_OBIT==key_field_type || CTC_FIELD_KEY_NSH_NEXT_PROTOCOL==key_field_type || CTC_FIELD_KEY_NSH_SI==key_field_type || CTC_FIELD_KEY_NSH_SPI==key_field_type))
            {
                switch(key_field_type)
                {
                    case CTC_FIELD_KEY_NSH_CBIT:
                        p_field_key->data = drv_data_temp[1]>>9 & 0x1;
                        p_field_key->mask = drv_mask_temp[1]>>9 & 0x1;
                        break;
                    case CTC_FIELD_KEY_NSH_OBIT:
                        p_field_key->data = drv_data_temp[1]>>8 & 0x1;
                        p_field_key->mask = drv_mask_temp[1]>>8 & 0x1;
                        break;
                    case CTC_FIELD_KEY_NSH_SPI:
                        p_field_key->data = (drv_data_temp[0]>>16 & 0xFFFF) | ((drv_data_temp[1] & 0xFF)<<16);
                        p_field_key->mask = (drv_mask_temp[0]>>16 & 0xFFFF) | ((drv_mask_temp[1] & 0xFF)<<16);
                        break;
                    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
                    case CTC_FIELD_KEY_NSH_SI:
                        p_field_key->data |= ((drv_data_temp[0] >> start_bit[field_name_i])&((1LLU << bit_len[field_name_i])-1));
                        p_field_key->mask |= ((drv_mask_temp[0] >> start_bit[field_name_i])&((1LLU << bit_len[field_name_i])-1));
                        break;
                }
            }
#endif
            else /* field_key.data; first is low bits; later is high bits */
            {
                p_field_key->data |= ((drv_data_temp[0]>>start_bit[field_name_i])&((1LLU<<bit_len[field_name_i])-1))<<shift;
                p_field_key->mask |= ((drv_mask_temp[0]>>start_bit[field_name_i])&((1LLU<<bit_len[field_name_i])-1))<<shift;
                shift += bit_len[field_name_i];
            }
#if 0
            if (CTC_FIELD_KEY_METADATA == key_field_type) /*ACL Metadata only support Metadata,not support vrfid, fid.*/
            {
                p_field_key->data &= 0x3FFF;
            }
            else if ((CTC_FIELD_KEY_ETHER_TYPE == key_field_type) && (CTC_ACL_KEY_MAC == pe->key_type || CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type))
            {
                cmd = DRV_IOR(EtherTypeCompressCam_t, EtherTypeCompressCam_etherType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_data_temp[0] & 0x3F, cmd, &(p_field_key->data)))
                p_field_key->mask = 0xFFFF;
            }
            else
#endif
            if ((CTC_FIELD_KEY_DST_GPORT == key_field_type) || (0 == sal_strcasecmp(*print_str,"Gport")))
            {
                uint16 lport = 0;
                uint8  gchip = 0;
                lport = p_field_key->data & 0x1FF;
                gchip = (p_field_key->data >> 9)&0x7F;
                p_field_key->data = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
                p_field_key->mask = 0xFFFFFFFF;
            }
#if 0
            else if (CTC_FIELD_KEY_IP_PKT_LEN_RANGE == key_field_type || CTC_FIELD_KEY_L4_SRC_PORT_RANGE == key_field_type || CTC_FIELD_KEY_L4_DST_PORT_RANGE == key_field_type)
            {
                cmd = DRV_IOR(ParserRangeOpCtl_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_range));
                for (loop_i = 0 ; loop_i < 12; loop_i++)
                {
                    if (CTC_IS_BIT_SET(p_field_key->data, loop_i)) /* data: min, mask: max */
                    {
                        if (CTC_FIELD_KEY_IP_PKT_LEN_RANGE == key_field_type && ACL_RANGE_TYPE_PKT_LENGTH == GetParserRangeOpCtl(V, array_0_opType_f + loop_i*3, &parser_range))
                        {
                            p_field_key->data = GetParserRangeOpCtl(V, array_0_minValue_f + loop_i*3, &parser_range);
                            p_field_key->mask = GetParserRangeOpCtl(V, array_0_maxValue_f + loop_i*3, &parser_range);
                            break;
                        }
                        else if (CTC_FIELD_KEY_L4_SRC_PORT_RANGE == key_field_type && ACL_RANGE_TYPE_L4_SRC_PORT == GetParserRangeOpCtl(V, array_0_opType_f + loop_i*3, &parser_range))
                        {
                            p_field_key->data = GetParserRangeOpCtl(V, array_0_minValue_f + loop_i*3, &parser_range);
                            p_field_key->mask = GetParserRangeOpCtl(V, array_0_maxValue_f + loop_i*3, &parser_range);
                            break;
                        }
                        else if (CTC_FIELD_KEY_L4_DST_PORT_RANGE == key_field_type && ACL_RANGE_TYPE_L4_DST_PORT == GetParserRangeOpCtl(V, array_0_opType_f + loop_i*3, &parser_range))
                        {
                            p_field_key->data = GetParserRangeOpCtl(V, array_0_minValue_f + loop_i*3, &parser_range);
                            p_field_key->mask = GetParserRangeOpCtl(V, array_0_maxValue_f + loop_i*3, &parser_range);
                            break;
                        }
                    }
                }
            }
            else if(CTC_FIELD_KEY_L4_TYPE == key_field_type) /* 1: layer4Type;                            2: MacL3Key320 layer3HeaderProtocol; */
            {                                                /* 3: ForwardKey640 ipDa 128 32-35 4 ipv4 ;  4: KEY_COPP ,KEY_COPP_EXT£ºetherType */
                if (CTC_ACL_KEY_MAC_IPV4 == pe->key_type)    /* 2: MacL3Key320: layer3HeaderProtocol */
                {
                    _sys_usw_acl_map_ip_protocol_to_l4_type(lchip, CTC_PARSER_L3_TYPE_IPV4, p_field_key->data, &temp_data, &temp_mask);
                    p_field_key->data = temp_data;
                    p_field_key->mask = temp_mask;
                }
                else if(CTC_ACL_KEY_FWD_EXT == pe->key_type) /* 3: ForwardKey640 ipDa 128 32-35 4 ipv4  */
                {
                    p_field_key->data = drv_data_temp[1]&0xF;
                    p_field_key->mask = drv_mask_temp[1]&0xF;
                }
                else
                {
                    break;
                }
            }
            else if ((CTC_FIELD_KEY_IP_PROTOCOL == key_field_type) && (2 == _sys_usw_acl_l4type_ipprotocol_judge(lchip, pe->key_type, pe->hash_field_sel_id)))
            { /* 2: layer4Type need map to ip protocol */
                CTC_ERROR_RETURN(_sys_usw_acl_map_l4_type_to_ip_protocol(lchip, pe->l3_type, p_field_key->data, &ipprotocol_data, &ipprotocol_mask));
                if (is_tcam)
                {
                    p_field_key->mask = ipprotocol_mask;
                }
                p_field_key->data = ipprotocol_data;
            }
            else if (CTC_FIELD_KEY_IP_FRAG == key_field_type)
            { /* 3: IP_FRAG need map to ctc_ip_frag */
                if (((CTC_ACL_KEY_COPP == pe->key_type) && DRV_FLD_IS_EXISIT(DsAclQosCoppKey320_t, DsAclQosCoppKey320_isFragmentPkt_f)) || \
                    (CTC_ACL_KEY_UDF == pe->key_type))
                {
                    p_field_key->data = drv_data_temp[0];
                    p_field_key->mask = (!drv_data_temp[0]? drv_mask_temp[0] | 0x2: drv_mask_temp[0]);
                }
                else
                {
                    p_field_key->data = drv_data_temp[0];
                    p_field_key->mask = drv_mask_temp[0];
                }
                CTC_ERROR_RETURN(_sys_usw_acl_unmap_ip_frag(lchip, p_field_key->data, p_field_key->mask, &ctc_ip_frag, &ctc_ip_frag_mask));
                if (is_tcam)
                {
                    p_field_key->mask = ctc_ip_frag_mask;
                }
                p_field_key->data = ctc_ip_frag;
            }
            else if (CTC_FIELD_KEY_PKT_FWD_TYPE == key_field_type)
            { /* 4: PKT_FWD_TYPE need map to ctc_ip_frag */
                uint8 ctc_fwd_type = 0;
                CTC_ERROR_RETURN(_sys_usw_acl_unmap_fwd_type(p_field_key->data, &ctc_fwd_type));
                p_field_key->data = ctc_fwd_type;
            }
            else if(CTC_FIELD_KEY_IP_PRECEDENCE == key_field_type)
            {
                p_field_key->data = (drv_data_temp[0] >> 3)&0x7;
                p_field_key->mask = (drv_mask_temp[0] >> 3)&0x7;
            }
            else if(CTC_FIELD_KEY_STP_STATE == key_field_type)
            {
                p_field_key->mask = 0x3;
                if(3 == drv_mask_temp[0])
                {
                    p_field_key->data = (drv_data_temp[0] == 3)?2:drv_data_temp[0];
                }
                else if(1 == drv_mask_temp[0] && 1 == drv_data_temp[0])
                {
                    p_field_key->data = 3;
                }
             }
#endif
        }
        else
        {/* field_key.ext_data */
            if ((CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type || CTC_ACL_KEY_UDF == pe->key_type) && (0 == sal_strcasecmp(*print_str, "Port Bitmap(Base/Value)")))
            {
                if (0 == field_name_i)  /*CTC_ACL_KEY_COPP, CTC_ACL_KEY_COPP_EXT port-bitmap 64*2*/
                {
                    drv_ext_data_temp[0] = *(uint32*)drv_data_temp;        /* drv_data_temp[0]*/
                    drv_ext_data_temp[1] = *((uint32*)drv_data_temp + 1);  /* drv_data_temp[1]*/
                }
                else /* base:(drv_data_temp[0]&0x1)*/
                {
                    drv_ext_data_temp[2] = *((uint32*)drv_data_temp)& 0x1;
                    sal_memcpy(p_field_key->ext_data, drv_ext_data_temp, sizeof(drv_ext_data_temp));
                }
            }
            else if ((CTC_ACL_KEY_FWD == pe->key_type || CTC_ACL_KEY_IPV6 == pe->key_type || CTC_ACL_KEY_CID == pe->key_type)
                && (CTC_FIELD_KEY_IPV6_SA == key_field_type || CTC_FIELD_KEY_IPV6_DA == key_field_type))
            {
                SYS_USW_REVERT_BYTE(hw_ipv6_64, drv_data_temp);
                _sys_usw_acl_get_ipv6_addr_from_compress(lchip, pe, (CTC_FIELD_KEY_IPV6_DA == key_field_type), hw_ipv6_64, (uint32*)p_field_key->ext_data);

                SYS_USW_REVERT_BYTE(hw_ipv6_64, drv_mask_temp);
                _sys_usw_acl_get_ipv6_addr_from_compress(lchip, pe, (CTC_FIELD_KEY_IPV6_DA == key_field_type), hw_ipv6_64, (uint32*)p_field_key->ext_mask);
            }
#if 0
            else if ((CTC_FIELD_KEY_AWARE_TUNNEL_INFO == key_field_type) &&(0 == sal_strcasecmp(*print_str,"AWARE TUNNEL INFO: Wlan")))
            {
                if (CTC_ACL_KEY_MAC_IPV4 == pe->key_type) /*(cvlanId 12 6-7 2 type, 5 1 wlan_ctl_pkt, 0-4 5 radio_id,  macDa MacL3Key320)*/
                {
                    if(0 == field_name_i)  /* macDa: Wlan Radio Mac;  MacL3Key320)*/
                    {
                        drv_ext_data_temp[0] = *(uint32*)drv_data_temp;
                        drv_ext_data_temp[1] = *((uint32*)drv_data_temp + 1) & 0xFFFF;

                        drv_ext_mask_temp[0] = *(uint32*)drv_mask_temp;
                        drv_ext_mask_temp[1] = *((uint32*)drv_mask_temp + 1) & 0xFFFF;
                    }
                    else                   /* cvlanId: 12 6-7 2 type, 5 1 wlan_ctl_pkt, 0-4 5 radio_id;  MacL3Key320)*/
                    {
                        drv_ext_data_temp[1] |= ((*((uint32*)drv_data_temp)& 0x3F)<< 16) ;
                        drv_ext_mask_temp[1] |= ((*((uint32*)drv_mask_temp)& 0x3F)<< 16) ;

                        sal_memcpy(p_field_key->ext_data, drv_ext_data_temp, sizeof(drv_ext_data_temp));
                        sal_memcpy(p_field_key->ext_mask, drv_ext_mask_temp, sizeof(drv_ext_mask_temp));
                    }
                }
                else if(CTC_ACL_KEY_FWD_EXT == pe->key_type) /* (ipDa 128 90-91 2 mergeDataType, 36-89 54 mergeData  ForwardKey640)  */
                {
                    drv_ext_data_temp[0] = (*((uint32*)drv_data_temp + 1)) >> 4;
                    drv_ext_data_temp[1] = (*((uint32*)drv_data_temp + 2))& 0xFFFFFFF;
                    drv_ext_data_temp[0] = ((drv_ext_data_temp[1]&0xF)<<28| drv_ext_data_temp[0]);
                    drv_ext_data_temp[1] = (drv_ext_data_temp[1]>>4)&0xFFFFFF;
                    sal_memcpy(p_field_key->ext_data, drv_ext_data_temp, sizeof(drv_ext_data_temp));

                    drv_ext_mask_temp[0] = (*((uint32*)drv_mask_temp + 1)) >> 4;
                    drv_ext_mask_temp[1] = (*((uint32*)drv_mask_temp + 2)) & 0xFFFFFFF;
                    drv_ext_mask_temp[0] = ((drv_ext_mask_temp[1]&0xF)<<28| drv_ext_mask_temp[0]);
                    drv_ext_mask_temp[1] = (drv_ext_mask_temp[1]>>4)&0xFFFFFF;
                    sal_memcpy(p_field_key->ext_mask, drv_ext_mask_temp, sizeof(drv_ext_mask_temp));
                }
                else
                {
                    sal_memcpy(p_field_key->ext_data, drv_data_temp, sizeof(drv_data_temp));
                    sal_memcpy(p_field_key->ext_mask, drv_mask_temp, sizeof(drv_mask_temp));
                }
            }
#endif
            else
            {
                switch(key_field_type)
                {/* process data format to user */
                    case CTC_FIELD_KEY_MAC_SA:
                    case CTC_FIELD_KEY_MAC_DA:
                    case CTC_FIELD_KEY_SENDER_MAC:
                    case CTC_FIELD_KEY_TARGET_MAC:
                    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
                        SYS_USW_SET_USER_MAC((uint8*)p_field_key->ext_data, (uint32*)drv_data_temp);
                        SYS_USW_SET_USER_MAC((uint8*)p_field_key->ext_mask, (uint32*)drv_mask_temp);
                        break;
                    case CTC_FIELD_KEY_IPV6_SA:
                    case CTC_FIELD_KEY_IPV6_DA:
                        SYS_USW_REVERT_IP6((uint32*)p_field_key->ext_data, (uint32*)drv_data_temp);
                        SYS_USW_REVERT_IP6((uint32*)p_field_key->ext_mask, (uint32*)drv_mask_temp);
                        break;
#if 0
                    case CTC_FIELD_KEY_UDF:
                        {
                            ctc_acl_udf_t* p_acl_udf_key = p_field_key->ext_data;
                            ctc_acl_udf_t* p_acl_udf_mask = p_field_key->ext_mask;
                            p_acl_udf_key->udf_id = pe->udf_id;
                            p_acl_udf_mask->udf_id = 0xFFFF;
                            sal_memcpy(p_acl_udf_key->udf, drv_data_temp, sizeof(p_acl_udf_key->udf));
                            sal_memcpy(p_acl_udf_mask->udf, drv_mask_temp, sizeof(p_acl_udf_key->udf));
                        }
                        break;
                    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
                    case CTC_FIELD_KEY_DOT1AE_SCI:
                        SYS_USW_REVERT_BYTE((uint32*)p_field_key->ext_data, (uint32*)drv_data_temp);
                        SYS_USW_REVERT_BYTE((uint32*)p_field_key->ext_mask, (uint32*)drv_mask_temp);
                        break;
#endif
                    default:
                        break;
                }
            }
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_show_key(uint8 lchip, sys_acl_entry_t* pe, uint32 key_id, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    ctc_field_key_t field_key;
    uint8 data_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint8 mask_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint16 key_field_recored_i = 0;
    uint16 show_key_field_array[18] = {CTC_FIELD_KEY_PORT,CTC_FIELD_KEY_DST_GPORT,CTC_FIELD_KEY_SVLAN_ID,CTC_FIELD_KEY_CVLAN_ID,
                                                        CTC_FIELD_KEY_MAC_SA,CTC_FIELD_KEY_MAC_DA,CTC_FIELD_KEY_IP_SA,CTC_FIELD_KEY_IP_DA,
                                                        CTC_FIELD_KEY_IPV6_SA,CTC_FIELD_KEY_IPV6_DA,CTC_FIELD_KEY_L4_SRC_PORT,CTC_FIELD_KEY_L4_DST_PORT,
                                                        CTC_FIELD_KEY_GRE_KEY,CTC_FIELD_KEY_VN_ID,CTC_FIELD_KEY_MPLS_LABEL0,CTC_FIELD_KEY_MPLS_LABEL1,
                                                        CTC_FIELD_KEY_MPLS_LABEL2,CTC_FIELD_KEY_CLASS_ID};
    uint8  show_key_field_array_i = 0;
    uint8  grep_i = 0;
    uint32 tempip = 0;
    char buf[CTC_IPV6_ADDR_STR_LEN];
    uint32 tempip_mask = 0;
    char buf_mask[CTC_IPV6_ADDR_STR_LEN];
    char* print_str = NULL;
    uint8 is_tcam = ACL_ENTRY_IS_TCAM(pe->key_type);
    uint8 pgrep_result =0;
#if 0
    uint16 tmp_data = 0;
    hw_mac_addr_t hw_mac_addr={0};
    hw_mac_addr_t hw_mac_addr_mask={0};
    uint32 data_64[2] = {0};
    uint32 mask_64[2] = {0};
#endif
    CTC_PTR_VALID_CHECK(pe);
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    field_key.ext_data = data_temp;
    field_key.ext_mask = mask_temp;
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Primary Key(Value/Mask):\n");

    for (show_key_field_array_i = 0; show_key_field_array_i < 18; show_key_field_array_i++)
    {
        key_field_recored_i = show_key_field_array[show_key_field_array_i];
        if (0 != grep_cnt) /* when grep, only printf these which are greped */
        {
            pgrep_result = 0;
            for (grep_i = 0; grep_i < grep_cnt; grep_i++)
            {
                if(key_field_recored_i == p_grep[grep_i].type)
                {
                    pgrep_result = 1;
                }
            }
            if(0 == pgrep_result)
            {
                continue;
            }
        }

        if ((1 == _sys_usw_acl_get_key_field_value(lchip, pe, key_field_recored_i, key_id, &field_key, &print_str, NULL))
            || (NULL == print_str)
            || ((CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type) && (CTC_FIELD_KEY_L3_TYPE == key_field_recored_i) && (0==field_key.data && 0==field_key.mask)))
        {
            continue;
        }

        if (key_field_recored_i == CTC_FIELD_KEY_MAC_SA || key_field_recored_i == CTC_FIELD_KEY_MAC_DA || key_field_recored_i == CTC_FIELD_KEY_TARGET_MAC || key_field_recored_i == CTC_FIELD_KEY_SENDER_MAC || key_field_recored_i == CTC_FIELD_KEY_WLAN_RADIO_MAC)
        {
            if(is_tcam)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %x.%x.%x/%X.%X.%X \n", print_str, sal_ntohs(*(unsigned short*)field_key.ext_data), sal_ntohs(*(unsigned short*)(field_key.ext_data + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_data + 4)),sal_ntohs(*(unsigned short*)field_key.ext_mask), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 4)));
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %x.%x.%x \n", print_str, sal_ntohs(*(unsigned short*)field_key.ext_data), sal_ntohs(*(unsigned short*)(field_key.ext_data + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_data + 4)));
            }
        }
        else if(key_field_recored_i == CTC_FIELD_KEY_IP_SA || key_field_recored_i == CTC_FIELD_KEY_IP_DA || key_field_recored_i == CTC_FIELD_KEY_ARP_SENDER_IP || key_field_recored_i == CTC_FIELD_KEY_ARP_TARGET_IP)
        {
            tempip = sal_ntohl(field_key.data);
            sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
            if (is_tcam)
            {
                tempip_mask = sal_ntohl(field_key.mask);
                sal_inet_ntop(AF_INET, &tempip_mask, buf_mask, CTC_IPV6_ADDR_STR_LEN);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %-15s/%-15s \n", print_str, buf, buf_mask);
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %-15s\n", print_str, buf);
            }
        }
        else if(key_field_recored_i == CTC_FIELD_KEY_IPV6_SA || key_field_recored_i == CTC_FIELD_KEY_IPV6_DA)
        {
            ipv6_address[0] = sal_ntohl(*((uint32*)field_key.ext_data));
            ipv6_address[1] = sal_ntohl(*((uint32*)field_key.ext_data+1));
            ipv6_address[2] = sal_ntohl(*((uint32*)field_key.ext_data+2));
            ipv6_address[3] = sal_ntohl(*((uint32*)field_key.ext_data+3));
            sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
            if (is_tcam)
            {
                sal_memset(ipv6_address, 0, sizeof(uint32)*4);
                ipv6_address[0] = sal_ntohl(*((uint32*)field_key.ext_mask));
                ipv6_address[1] = sal_ntohl(*((uint32*)field_key.ext_mask+1));
                ipv6_address[2] = sal_ntohl(*((uint32*)field_key.ext_mask+2));
                ipv6_address[3] = sal_ntohl(*((uint32*)field_key.ext_mask+3));
                sal_inet_ntop(AF_INET6, ipv6_address, buf_mask, CTC_IPV6_ADDR_STR_LEN);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s/%s\n", print_str, buf, buf_mask);
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", print_str, buf);
            }
        }
        else
        {
#if 0
            if(CTC_FIELD_KEY_SVLAN_RANGE == key_field_recored_i || CTC_FIELD_KEY_CVLAN_RANGE == key_field_recored_i)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u, %u\n", print_str, pe->minvid, field_key.data);
            }
            else if (CTC_FIELD_KEY_IP_PKT_LEN_RANGE == key_field_recored_i || CTC_FIELD_KEY_L4_SRC_PORT_RANGE == key_field_recored_i || CTC_FIELD_KEY_L4_DST_PORT_RANGE == key_field_recored_i)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", print_str, field_key.data, field_key.mask);
            }
            else
#endif
            if ((CTC_FIELD_KEY_PORT == key_field_recored_i) &&(0 == sal_strcasecmp(print_str,"Port Bitmap(Base/Value)")))
            {
                if(CTC_ACL_KEY_COPP == pe->key_type || CTC_ACL_KEY_COPP_EXT == pe->key_type || CTC_ACL_KEY_UDF == pe->key_type)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u, 0x%"PRIx64"\n", print_str, ((*((uint32*)field_key.ext_data + 2))&0x1)*64, (uint64)(((uint64)(*((uint32*)field_key.ext_data + 1))<<32) | (*((uint32*)field_key.ext_data))));
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u, 0x%x\n", print_str, ((field_key.data>>16)&0x7)*16, field_key.data & 0xFFFF);
                }
            }
#if 0
            else if ((CTC_FIELD_KEY_AWARE_TUNNEL_INFO == key_field_recored_i) &&(0 == sal_strcasecmp(print_str,"AWARE TUNNEL INFO: Wlan")))
            {
                hw_mac_addr[0] = *((uint32*)field_key.ext_data);
                hw_mac_addr[1] = *((uint32*)field_key.ext_data + 1) & 0xFFFF;
                SYS_USW_SET_USER_MAC((uint8*)field_key.ext_data, (uint32*)hw_mac_addr);

                hw_mac_addr_mask[0] = *((uint32*)field_key.ext_mask);
                hw_mac_addr_mask[1] = *((uint32*)field_key.ext_mask + 1) & 0xFFFF;
                SYS_USW_SET_USER_MAC((uint8*)field_key.ext_mask, (uint32*)hw_mac_addr_mask);

                if ((hw_mac_addr_mask[0] || hw_mac_addr_mask[1]) != 0)  /*wlan radio mac   48bit */
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %x.%x.%x/%X.%X.%X \n", "Wlan Radio Mac", sal_ntohs(*(unsigned short*)field_key.ext_data), sal_ntohs(*(unsigned short*)(field_key.ext_data + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_data + 4)), sal_ntohs(*(unsigned short*)field_key.ext_mask), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 4)));
                }
                if ((*((uint8*)field_key.ext_mask + 6) & 0x1F) != 0)   /*wlan radio id   5bit */
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", "Wlan Radio ID", *((uint8*)field_key.ext_data + 6) & 0x1F, *((uint8*)field_key.ext_mask + 6) & 0x1F);
                }
                if ((*((uint8*)field_key.ext_mask + 6) >> 5 & 0x1) != 0)/*wlan Ctl Pkt   1bit */
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", "Wlan Ctl Pkt", *((uint8*)field_key.ext_data + 6) >> 5 & 0x1, *((uint8*)field_key.ext_mask + 6) >> 5 & 0x1);
                }
            }
            else if(CTC_FIELD_KEY_UDF == key_field_recored_i)
            {
                uint32* p_data = (uint32*)((ctc_acl_udf_t*)(field_key.ext_data))->udf;
                uint32* p_mask = (uint32*)((ctc_acl_udf_t*)(field_key.ext_mask))->udf;
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x\n","udf id", ((ctc_acl_udf_t*)(field_key.ext_data))->udf_id);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x/0x%X, 0x%x/0x%X, 0x%x/0x%X, 0x%x/0x%X\n", print_str, *p_data,*p_mask, *(p_data+1),*(p_mask + 1), *(p_data + 2), *(p_mask + 2), *(p_data + 3),*(p_mask + 3));
            }
            else if(CTC_FIELD_KEY_DISCARD == key_field_recored_i)
            {
                /* Ipe: IngAclQosTcamKeyPI.discardInfo(6,0) = (PacketInfo.discard,PacketInfo.discardType(5,0)); */
                if ((CTC_ACL_KEY_INTERFACE == pe->key_type) && DRV_FLD_IS_EXISIT(DsAclQosKey80_t, DsAclQosKey80_discard_f))
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", print_str, field_key.data & 0x1, field_key.mask & 0x1);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", print_str, (field_key.data >> 6) & 0x1, (field_key.mask >> 6) & 0x1);
                    if (field_key.mask & 0x3F)
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", "DISCARD Type", (field_key.data & 0x3F), (field_key.mask & 0x3F));
                    }
                }
            }
            else if (CTC_FIELD_KEY_VLAN_XLATE_HIT == key_field_recored_i)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", "Vlan Xlate Hit 0", (!CTC_IS_BIT_SET(field_key.data, 0)), (CTC_IS_BIT_SET(field_key.mask, 0)));
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", "Vlan Xlate Hit 1", (!CTC_IS_BIT_SET(field_key.data, 1)), (CTC_IS_BIT_SET(field_key.mask, 1)));
            }
            else if((CTC_FIELD_KEY_DOT1AE_SCI == key_field_recored_i) || (CTC_FIELD_KEY_SATPDU_PDU_BYTE == key_field_recored_i))
            {
                data_64[0] = *((uint32*)field_key.ext_data);
                data_64[1] = *((uint32*)field_key.ext_data + 1);
                SYS_USW_REVERT_BYTE((uint32*)field_key.ext_data, (uint32*)data_64);

                mask_64[0] = *((uint32*)field_key.ext_mask);
                mask_64[1] = *((uint32*)field_key.ext_mask + 1);
                SYS_USW_REVERT_BYTE((uint32*)field_key.ext_mask, (uint32*)mask_64);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%"PRIx64"/0x%"PRIx64"\n", print_str, (uint64)(((uint64)(*((uint32*)field_key.ext_data + 1))<<32) | (*((uint32*)field_key.ext_data))), (uint64)(((uint64)(*((uint32*)field_key.ext_mask + 1))<<32) | (*((uint32*)field_key.ext_mask))));
            }
#endif
            else
            {
                if (is_tcam)
                {
#if 0
                    if ((CTC_FIELD_KEY_ETHER_TYPE == key_field_recored_i) || (CTC_FIELD_KEY_ARP_PROTOCOL_TYPE == key_field_recored_i))
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x/0x%X\n", print_str, field_key.data, field_key.mask);
                    }
                    else if (CTC_FIELD_KEY_L3_TYPE == key_field_recored_i && CTC_PARSER_L3_TYPE_ARP != field_key.data)
                    {
                        _sys_usw_acl_unmap_ethertype(field_key.data, &tmp_data);
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u%-9s 0x%x%s/0x%X\n", print_str, field_key.data, "(EthType:", tmp_data, ")", field_key.mask);
                    }
#endif
                    if ((CTC_FIELD_KEY_DST_GPORT == key_field_recored_i) || (0 == sal_strcasecmp(print_str,"Gport")))
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x/0x%X\n", print_str, field_key.data, field_key.mask);
                    }
                    else
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", print_str, field_key.data, field_key.mask);
                    }
                }
                else
                {
#if 0
                    if ((CTC_FIELD_KEY_ETHER_TYPE == key_field_recored_i) || (CTC_FIELD_KEY_ARP_PROTOCOL_TYPE == key_field_recored_i))
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x\n", print_str, field_key.data);
                    }
                    else if (CTC_FIELD_KEY_L3_TYPE == key_field_recored_i && CTC_PARSER_L3_TYPE_ARP != field_key.data)
                    {
                        _sys_usw_acl_unmap_ethertype(field_key.data, &tmp_data);
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u%-9s 0x%x%s\n", print_str, field_key.data, "(EthType:", tmp_data, ")");
                    }
#endif
                    if ((CTC_FIELD_KEY_DST_GPORT == key_field_recored_i) || (0 == sal_strcasecmp(print_str,"Gport")))
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x\n", print_str, field_key.data);
                    }
                    else
                    {
                        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", print_str, field_key.data);
                    }
                }
            }
        }
        print_str = NULL;
    }
    return CTC_E_NONE;
}

/*
show acl action
*/
STATIC int32
_sys_usw_acl_show_action(uint8 lchip, sys_acl_entry_t* pe)
{
    uint8 is_half_ad = 0;
    ctc_acl_field_action_t field_action;
    ctc_acl_to_cpu_t       copy_to_cpu;
    ctc_acl_ipfix_t        ipfix;
    ctc_acl_oam_t          oam;
    ctc_acl_lb_hash_t      lb_hash;
    ctc_acl_inter_cn_t     inter_cn;
    ctc_acl_vlan_edit_t    vlan_edit;
    ctc_acl_packet_strip_t strip_pkt;
    ctc_acl_table_map_t    table_map;

    CTC_PTR_VALID_CHECK(pe);
    sal_memset(&field_action,0, sizeof(ctc_acl_field_action_t));
    sal_memset(&copy_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));
    sal_memset(&ipfix, 0, sizeof(ctc_acl_ipfix_t));
    sal_memset(&oam, 0, sizeof(ctc_acl_oam_t));
    sal_memset(&lb_hash, 0, sizeof(ctc_acl_lb_hash_t));
    sal_memset(&inter_cn, 0, sizeof(ctc_acl_inter_cn_t));
    sal_memset(&vlan_edit, 0, sizeof(ctc_acl_vlan_edit_t));
    sal_memset(&strip_pkt, 0, sizeof(ctc_acl_packet_strip_t));
    sal_memset(&table_map, 0, sizeof(ctc_acl_table_map_t));

    if( CTC_ACL_KEY_INTERFACE == pe->key_type )
    {
        is_half_ad = 1;
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Action:\n");

    field_action.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;   /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &copy_to_cpu;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));

        if(CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER == copy_to_cpu.mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", "COPY TO CPU",_sys_usw_get_cp_to_cpu_desc(copy_to_cpu.mode));
        }
        else if(CTC_ACL_TO_CPU_MODE_TO_CPU_COVER == copy_to_cpu.mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s, %s:%u\n", "COPY TO CPU",_sys_usw_get_cp_to_cpu_desc(copy_to_cpu.mode),"ReasonID",copy_to_cpu.cpu_reason_id );
        }
        else if(CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU == copy_to_cpu.mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", "COPY TO CPU",_sys_usw_get_cp_to_cpu_desc(copy_to_cpu.mode));
        }
    }

    field_action.type = CTC_ACL_FIELD_ACTION_STATS;      /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "STATS ID", pe->stats_id);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;   /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: Log Id:%u,Percent:%s%d", "RANDOM LOG", field_action.data0,"1/2^",CTC_LOG_PERCENT_POWER_NEGATIVE_0-field_action.data1);

    }

    field_action.type = CTC_ACL_FIELD_ACTION_COLOR;     /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", "COLOR",_sys_usw_get_color_desc(field_action.data0));
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DSCP;      /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "DSCP", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;    /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "MICRO FLOW POLICER ID", pe->policer_id);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER;    /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "MACRO FLOW POLICER ID", pe->policer_id);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER;    /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "HBWP POLICER ID", pe->policer_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "COS INDEX", pe->cos_index);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_COPP;                  /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "COPP ID", pe->policer_id);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_SRC_CID;               /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "SRC_CID", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_TRUNCATED_LEN;         /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "TRUNCATED_LEN", field_action.data0);
    }

            /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_REDIRECT)|| CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_REDIRECT_PORT))
    {
       field_action.type = CTC_ACL_FIELD_ACTION_REDIRECT_PORT;   /* only tcam have*/
       CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
       SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u(NH-ID) gport:0x%.4x\n", "REDIRECT", pe->nexthop_id,field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT;   /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "REDIRECT_CANCEL_PKT_EDIT");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT;  /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "REDIRECT_FILTER_ROUTED_PKT");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DISCARD;              /* tcam and hash both have, but not same*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
           SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:\n","DISCARD");
            if (!ACL_ENTRY_IS_TCAM(pe->key_type) || is_half_ad)
            {
                field_action.data0 = CTC_QOS_COLOR_GREEN;
                CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
                if(field_action.data1)
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Green\n","");
                if(field_action.data1)
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Yellow\n","");
                if(field_action.data1)
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Red\n","");
            }
            else
            {

                field_action.data0 = CTC_QOS_COLOR_GREEN;
                CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
               if(field_action.data1)
               SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Green\n","");

               field_action.data0 = CTC_QOS_COLOR_YELLOW;

               CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
               if(field_action.data1)
               SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Yellow\n","");

              field_action.data0 = CTC_QOS_COLOR_RED;
              CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
              if(field_action.data1)
              SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Red\n","");

            }
      }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;     /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:\n","CANCEL DISCARD");

            if ( is_half_ad)
            {
                field_action.data0 = CTC_QOS_COLOR_GREEN;
                CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
                if(field_action.data1)
                {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Green\n","");
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Yellow\n","");
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Red\n","");
                }
            }
            else
            {

                field_action.data0 = CTC_QOS_COLOR_GREEN;
                CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
                if(field_action.data1)
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Green\n","");

               field_action.data0 = CTC_QOS_COLOR_YELLOW;
               CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
               if(field_action.data1)
               SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Yellow\n","");

              field_action.data0 = CTC_QOS_COLOR_RED;
              CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
              if(field_action.data1)
               SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:Red\n","");

            }
    }

    field_action.type = CTC_ACL_FIELD_ACTION_PRIORITY;    /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:\n","PRIORITY");
        field_action.data0 = CTC_QOS_COLOR_GREEN;
        _sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action);
        if(field_action.data1 != 0xFFFFFFFF)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-25s:Green :%d\n","",field_action.data1);
        }

        field_action.data0 = CTC_QOS_COLOR_YELLOW;
        _sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action);
        if(field_action.data1 != 0xFFFFFFFF)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-25s:Yellow :%d\n","",field_action.data1);
        }

        field_action.data0 = CTC_QOS_COLOR_RED;
        _sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action);
        if(field_action.data1 != 0xFFFFFFFF)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-25s:Red :%d\n","",field_action.data1);
        }

     }

    field_action.type = CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG;  /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "DISABLE_ELEPHANT_LOG");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR;    /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "TERMINATE_CID_HDR");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_NAT;           /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "CANCEL_NAT");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_IPFIX;                /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &ipfix;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n",  "Enable Ipfix");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "flow_cfg_id", ipfix.flow_cfg_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "field_sel_id", ipfix.field_sel_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "hash_type", ipfix.hash_type);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "use_mapped_vlan", ipfix.use_mapped_vlan);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "use_cvlan", ipfix.use_cvlan);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "policer_id", ipfix.policer_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "lantency_en", ipfix.lantency_en);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX;        /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "CANCEL_IPFIX");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING;   /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "CANCEL_IPFIX_LEARNING");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_OAM;                     /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &oam;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n",  "Enable OAM");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "dest_gchip", oam.dest_gchip);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "lmep_index", oam.lmep_index);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "mip_en", oam.mip_en);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "oam_type", oam.oam_type);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "link_oam_en", oam.link_oam_en);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "packet_offset", oam.packet_offset);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "mac_da_check_en", oam.mac_da_check_en);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "is_self_address", oam.is_self_address);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "time_stamp_en", oam.time_stamp_en);
        if (DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_u1_g3_twampTsType_f))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "timestamp_format", oam.timestamp_format);
        }
    }

    field_action.type = CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH;      /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &lb_hash;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s:%d(mode:%d) \n", "REPLACE_LB_HASH",lb_hash.lb_hash,lb_hash.mode);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE;      /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "CANCEL_DOT1AE");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT;    /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "DOT1AE_PERMIT_PLAIN_PKT");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_METADATA;        /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "METADATA", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_INTER_CN;     /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &inter_cn;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "INTER_CN", inter_cn.inter_cn);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;   /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n", "VLAN_EDIT");
        if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s - %s\n", "Stag_op", _sys_usw_get_vlan_edit_desc(1, vlan_edit.stag_op));
            if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "svid_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.svid_sl), "New_svid", vlan_edit.svid_new);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "scos_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.scos_sl), "New_scos", vlan_edit.scos_new);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "scfi_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.scfi_sl), "New_scfi", vlan_edit.scfi_new);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12u\n", "tpid_idx", vlan_edit.tpid_index);
            }
        }
        if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s - %s\n", "Ctag_op", _sys_usw_get_vlan_edit_desc(1, vlan_edit.ctag_op));
            if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "cvid_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.cvid_sl), "New_cvid", vlan_edit.cvid_new);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "ccos_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.ccos_sl), "New_ccos", vlan_edit.ccos_new);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %28s - %-12s  %s - %u\n", "ccfi_sl", _sys_usw_get_vlan_edit_desc(0, vlan_edit.ccfi_sl), "New_ccfi", vlan_edit.ccfi_new);
            }
        }
    }

    field_action.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;   /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &strip_pkt;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n",  "STRIP_PACKET");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "start_packet_strip", strip_pkt.start_packet_strip);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "packet_type", strip_pkt.packet_type);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "strip_extra_len", strip_pkt.strip_extra_len);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CRITICAL_PKT;   /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n", "CRITICAL_PKT");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_LOGIC_PORT;    /*only hash have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "LOGIC_PORT", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_SPAN_FLOW;    /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "SPAN_FLOW", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_ALL;   /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n",     "CANCEL_ALL");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP;   /* only tcam have*/

    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        field_action.ext_data = &table_map;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n",     "QOS_TABLE_MAP");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "table_map_id", table_map.table_map_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %25s: %u\n", "table_map_type", table_map.table_map_type);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN;    /*only hash have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "CANCEL_ACL_TCAM_EN", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DENY_BRIDGE;    /*only hash have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n",     "DENY_BRIDGE");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;    /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n",     "DENY_LEARNING");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DENY_ROUTE;    /*only hash have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s \n",     "DENY_ROUTE");
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DEST_CID;     /*only hash have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "DEST_CID", field_action.data0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR;        /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s\n", field_action.data0? "Ecmp rr mode: disable": "Ecmp rr mode: enable");
    }
    field_action.type = CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR;         /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s\n", field_action.data0? "Linkagg rr mode: disable": "Linkagg rr mode: enable");
    }
    field_action.type = CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE;         /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-32s : %u\n", "Load balance hash ecmp profile id", field_action.data0? field_action.data1: 0);
    }
    field_action.type = CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE;         /* tcam and hash both have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-32s : %u\n", "Load balance hash lag profile id", field_action.data0? field_action.data1: 0);
    }
    field_action.type = CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT;         /* only tcam have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        ctc_acl_vxlan_rsv_edit_t vxlan_rsv_edit;
        sal_memset(&vxlan_rsv_edit, 0, sizeof(ctc_acl_vxlan_rsv_edit_t));
        field_action.ext_data = (void*)&vxlan_rsv_edit;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s : %d\n", "Vxlan reserved edit mode", vxlan_rsv_edit.edit_mode);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s : 0x%x\n", "Vxlan flags", vxlan_rsv_edit.vxlan_flags);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s : 0x%x\n", "Vxlan reserved1", vxlan_rsv_edit.vxlan_rsv1);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s : 0x%x\n", "Vxlan reserved2", vxlan_rsv_edit.vxlan_rsv2);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_TIMESTAMP;         /* only tcam  have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        ctc_acl_timestamp_t timestamp;
        sal_memset(&timestamp, 0, sizeof(ctc_acl_timestamp_t));
        field_action.ext_data = (void*)&timestamp;
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s: %d \n", "Timestamp Mode", timestamp.mode);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN;         /* only tcam  have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s: %d \n", "Reflective Bridge Enable", field_action.data0? 1: 0);
    }

    field_action.type = CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS;         /* only tcam  have*/
    if(CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_get_action_field(lchip, pe->fpae.entry_id, &field_action));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-24s: %d \n", "Port Isolation Disable", field_action.data0? 1: 0);
    }

    return CTC_E_NONE;
}
#if 0
 /* 1: layer3HeaderProtocol*/
 /* 2: layer4Type*/
int32
_sys_usw_acl_l4type_ipprotocol_judge(uint8 lchip, uint8 key_type, uint8 hash_sel_id)
{
    ds_t ds_sel ={0};
    if (CTC_ACL_KEY_HASH_IPV4 == key_type || CTC_ACL_KEY_HASH_IPV6 == key_type)
    {
        if (CTC_ACL_KEY_HASH_IPV4 == key_type )
        {
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, hash_sel_id, &ds_sel));
            if (GetFlowL3Ipv4HashFieldSelect(V, layer3HeaderProtocolEn_f, &ds_sel))
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }
        else  /* DsFlowL2HashKey  DsAclQosL3Key160 DsAclQosL3Key320	DsAclQosMacL3Key320  DsAclQosMacL3Key640  DsAclQosForwardKey320  DsAclQosForwardKey640  DsAclQosCoppKey320  DsAclQosCoppKey640*/
        {
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, hash_sel_id, &ds_sel));
            if (GetFlowL3Ipv6HashFieldSelect(V, layer3HeaderProtocolEn_f, &ds_sel))
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }
    }
    else if(CTC_ACL_KEY_IPV4 == key_type || CTC_ACL_KEY_CID == key_type || CTC_ACL_KEY_INTERFACE == key_type || CTC_ACL_KEY_FWD == key_type)
    {
        return 2;
    }
    else
    {
        return 1;
    }

}
#endif
STATIC int32
 _sys_usw_acl_filter_by_key(uint8 lchip, sys_acl_entry_t* pe, ctc_field_key_t* p_grep, uint8 grep_cnt,uint32 key_id)
{
    ctc_field_key_t field_key;
    uint8 data_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint8 mask_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint8 result_grep_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint8 result_data_temp[SYS_USW_ACL_MAX_KEY_SIZE] = {0};
    uint32 result_grep = 0;
    uint32 result_data = 0;
    uint8  grep_i = 0;
    char* print_str = NULL;
    uint32 key_size = 0;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(p_grep);

    sal_memset(&field_key,0, sizeof(ctc_field_key_t));
    field_key.ext_data = data_temp;
    field_key.ext_mask = mask_temp;

    for(grep_i = 0;grep_i< grep_cnt; grep_i++)
    {
        sal_memset(data_temp,0,sizeof(data_temp));
        sal_memset(mask_temp,0,sizeof(mask_temp));

        if ((p_grep[grep_i].type == CTC_FIELD_KEY_ETHER_TYPE) && (CTC_ACL_KEY_MAC != pe->key_type))
        {
            uint8  l3_type = CTC_PARSER_L3_TYPE_NONE;
            _sys_usw_acl_map_ethertype(p_grep[grep_i].data, &l3_type);
            if (l3_type != CTC_PARSER_L3_TYPE_NONE)
            {
                p_grep[grep_i].type = CTC_FIELD_KEY_L3_TYPE;
                p_grep[grep_i].data = l3_type;
            }
        }
        if(1 == _sys_usw_acl_get_key_field_value(lchip, pe, p_grep[grep_i].type, key_id, &field_key,&print_str, &key_size))
        {   /* initial conditions */
            return CTC_E_INVALID_PARAM;    /* grep fail */
        }
        /*special process for discard use data & ext_data*/
        if(CTC_FIELD_KEY_DISCARD == p_grep[grep_i].type)
        {
            uint8 discard_data = 0;
            uint8 discard_mask = 0;
            if(p_grep[grep_i].ext_data)
            {
                discard_data = *(uint32*)p_grep[grep_i].ext_data;
                discard_mask = 0x3F;
            }
            p_grep[grep_i].data = p_grep[grep_i].data<<6|(discard_data & 0x3F);
            p_grep[grep_i].mask = p_grep[grep_i].mask<<6|(discard_mask & 0x3F);
        }
        if(ACL_ENTRY_IS_TCAM(pe->key_type))
        {
            if (p_grep[grep_i].ext_data && p_grep[grep_i].ext_mask)
            {
                uint32 loop;
                for(loop=0; loop < key_size; loop++)
                {
                    result_grep_temp[loop] = (*((uint8*)p_grep[grep_i].ext_data+loop)) & (*((uint8*)p_grep[grep_i].ext_mask+loop));
                    result_data_temp[loop] = (*((uint8*)field_key.ext_data+loop)) & (*((uint8*)field_key.ext_mask+loop));
                }

                if(0 != sal_memcmp(result_grep_temp, result_data_temp, key_size))
                {
                    return CTC_E_INVALID_PARAM;  /* grep fail */
                }
                sal_memset(result_grep_temp,0,sizeof(result_grep_temp));
                sal_memset(result_data_temp,0,sizeof(result_data_temp));
            }
            else
            {
                result_grep = p_grep[grep_i].data & p_grep[grep_i].mask;
                result_data = field_key.data  & field_key.mask & p_grep[grep_i].mask;
                if(result_grep != result_data)
                {
                    return CTC_E_INVALID_PARAM; /* grep fail */
                }
                result_grep = 0;
                result_data = 0;
            }
        }
        else
        {
            if (p_grep[grep_i].ext_data)
            {
                if(0 != sal_memcmp(p_grep[grep_i].ext_data, field_key.ext_data, key_size))
                {
                    return CTC_E_INVALID_PARAM; /* grep fail */
                }
            }
            else
            {
                if(p_grep[grep_i].data != field_key.data)
                {
                    return CTC_E_INVALID_PARAM; /* grep fail */
                }
            }
        }
    }
    return CTC_E_NONE;
}

/*
 * show acl entry to a specific entry with specific key type
 */
STATIC int32
_sys_usw_acl_show_entry(uint8 lchip, sys_acl_entry_t* pe, uint32* p_cnt, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    int32  ret = CTC_E_NONE;
    char* type[] = {"TcamMac-160", "TcamL3-160", "Tcam-Mpls", "TcamIpv6-320", "Hash-Mac",
                        "Hash-Ipv4", "Hash-L2-L3", "Hash-Mpls", "Hash-Ipv6", "Tcam-pbr-ipv4",
                        "Tcam-pbr-ipv6", "TcamL3-320", "TcamMacL3-320", "TcamMacL3-640",
                        "TcamIpv6-640", "TcamMacIpv6-640", "TcamCID-160", "TcamInterface-80",
                        "TcamFwd-320", "TcamFwd-640", "TcamCopp-320", "TcamCopp-640", "TcamUDF-320"};
    sys_acl_group_t* pg;
    char    str[35]     = {0};
    char    format[10]  = {0};
    uint32  key_id       = 0;
    uint32  act_id       = 0;
    uint32  hw_index     = 0;
    char    key_name[50] = {0};
    char    ad_name[50]  = {0};

    CTC_PTR_VALID_CHECK(pe);

    pg = pe->group;
    if (!pg)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if ((CTC_ACL_KEY_NUM != key_type)
        && (pe->key_type != key_type))
    {
        return CTC_E_NONE;
    }

    _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);
    if (FPA_ENTRY_FLAG_INSTALLED == pe->fpae.flag)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_rebuild_buffer_from_hw(lchip, key_id, act_id, hw_index, pe));
    }

    if (0 != grep_cnt)
    {
        if (_sys_usw_acl_filter_by_key(lchip, pe, p_grep, grep_cnt, key_id) != CTC_E_NONE)
        {
            ret = CTC_E_NONE;
            goto clean_up;
        }
    }

    if ((!detail) || (1 == show_type))
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8d", *p_cnt);
        if(SYS_ACL_SHOW_IN_HEX <= pe->fpae.entry_id)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s", CTC_DEBUG_HEX_FORMAT(str, format, pe->fpae.entry_id, 8, U));
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", pe->fpae.entry_id);
        }

        if (SYS_ACL_SHOW_IN_HEX <= pg->group_id)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s", CTC_DEBUG_HEX_FORMAT(str, format, pg->group_id, 8, U));
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", pg->group_id);
        }

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4s", (pe->fpae.flag) ? "Y" : "N");
        if(SYS_ACL_SHOW_IN_HEX <= pe->fpae.priority)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s", CTC_DEBUG_HEX_FORMAT(str, format, pe->fpae.priority, 8, U));
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", pe->fpae.priority);
        }
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8u", pg->group_info.block_id);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-18s", type[pe->key_type]);

        _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);

        if (ACL_ENTRY_IS_TCAM(pe->key_type))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8u",  hw_index/pe->fpae.real_step);
        }
        else   /*hash*/
        {
            if(pe->key_exist)   /*hash key index is not avaliable*/
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s ","keyExist");
            }
    		else if(pe->key_conflict)   /*hash key index is not avaliable*/
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s ","bucketFull");
            }
    		else if(!pe->hash_valid)   /*hash key index is not avaliable*/
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s ","keyIncomplete");
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s",  CTC_DEBUG_HEX_FORMAT(str, format, hw_index, 4, U));
            }
        }

        if(CTC_FLAG_ISSET(pe->action_flag, SYS_ACL_ACTION_FLAG_DISCARD))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s\n", "Discard");
        }
        else if(CTC_FLAG_ISSET(pe->action_flag, SYS_ACL_ACTION_FLAG_REDIRECT))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s\n", "Redirect");
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s\n", "Permit");
        }

        (*p_cnt)++;
    }

    if(detail)
    {
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, key_id, 0, key_name);

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

        if(SYS_ACL_SHOW_IN_HEX <= pe->fpae.entry_id)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#ENTRY_ID: %-12s", CTC_DEBUG_HEX_FORMAT(str, format, pe->fpae.entry_id, 8, U));
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#ENTRY_ID: %-12u", pe->fpae.entry_id);
        }

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Table:\n");

        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, act_id, 0, ad_name);
        if (!ACL_ENTRY_IS_TCAM(pe->key_type))   /*hash*/
        {
            if(CTC_MAX_UINT32_VALUE == pe->fpae.offset_a)   /*hash key index is not avaliable*/
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :%-8s\n", key_name, "Invalid");
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :%-8s\n", ad_name, "Invalid");
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :0x%-8x\n", key_name, hw_index);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :0x%-8x\n", ad_name, pe->ad_index);
            }
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :%-8u\n", key_name, SYS_ACL_MAP_DRV_KEY_INDEX(hw_index, pe->fpae.real_step));
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-32s :%-8u\n", ad_name, SYS_ACL_MAP_DRV_AD_INDEX(hw_index, pe->fpae.real_step));
        }

        CTC_ERROR_GOTO(_sys_usw_acl_show_key(lchip, pe, key_id,p_grep, grep_cnt), ret, clean_up);

        switch (pe->action_type)
        {
            case SYS_ACL_ACTION_TCAM_INGRESS:
            case SYS_ACL_ACTION_TCAM_EGRESS:
            case SYS_ACL_ACTION_HASH_INGRESS:
                _sys_usw_acl_show_action(lchip, pe);
                break;
            default:
                break;
        }
    }
clean_up:
    if(FPA_ENTRY_FLAG_INSTALLED == pe->fpae.flag)
    {
        mem_free(pe->buffer);
    }
    return ret;
}


/*
 * show acl entriy by entry id
 */
int32
_sys_usw_acl_show_entry_by_entry_id(uint8 lchip, uint32 eid, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    sys_acl_entry_t* pe = NULL;

    uint32         count = 1;

    _sys_usw_acl_get_entry_by_eid(lchip, eid, &pe);
    if (!pe)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, pe, &count, key_type, detail, show_type, p_grep, grep_cnt));

    return CTC_E_NONE;
}

/*
 * show acl entries in a group with specific key type
 */
STATIC int32
_sys_usw_acl_show_entry_in_group(uint8 lchip, sys_acl_group_t* pg,_acl_cb_para_t* cb_para)
{
    struct ctc_slistnode_s* pe;

	uint8 key_type = cb_para->key_type;
    uint8 detail = cb_para->detail;
	uint8 loop_en = 0;
	uint8 is_tcam = (cb_para->entry_type == 0) || (cb_para->entry_type ==2);
    uint8 is_hash = (cb_para->entry_type == 1) || (cb_para->entry_type ==2);

    CTC_PTR_VALID_CHECK(pg);
    CTC_SLIST_LOOP(pg->entry_list, pe)
    {
        loop_en =  (ACL_ENTRY_IS_TCAM(((sys_acl_entry_t *) pe)->key_type) && is_tcam) \
		        || (!ACL_ENTRY_IS_TCAM(((sys_acl_entry_t *) pe)->key_type) && is_hash);
	    if(!loop_en)
		{
		   continue;
		}

       _sys_usw_acl_show_entry(lchip, (sys_acl_entry_t *) pe, &cb_para->count, key_type, detail, cb_para->show_type, cb_para->p_grep, cb_para->grep_cnt);
    }

    return CTC_E_NONE;
}

/*
 * show acl entries by group id with specific key type
 */
int32
_sys_usw_acl_show_entry_by_group_id(uint8 lchip, uint32 gid, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    sys_acl_group_t* pg  = NULL;
	_acl_cb_para_t cb_para;
	cb_para.count = 1;
	cb_para.entry_type = 2;
	cb_para.key_type = key_type;
    cb_para.show_type = show_type;
	cb_para.grep_cnt = grep_cnt;
	cb_para.p_grep = p_grep;

    _sys_usw_acl_get_group_by_gid(lchip, gid, &pg);
    SYS_ACL_CHECK_GROUP_UNEXIST(pg);

	cb_para.detail = 0;
    CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_group(lchip, pg, &cb_para));
    if(detail)
    {
        cb_para.detail = 1;
        CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_group(lchip, pg, &cb_para));
    }

    return CTC_E_NONE;
}

/*
 * show acl entries in a block with specific key type
 */
STATIC int32
_sys_usw_acl_show_entry_in_block(uint8 lchip, sys_acl_block_t* pb, uint32* p_cnt, uint8 key_type, uint8 detail,uint8 show_type,  ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    ctc_fpa_entry_t* pe;
    uint16         block_idx;
    uint8          step = 0;

    CTC_PTR_VALID_CHECK(pb);
    CTC_PTR_VALID_CHECK(p_cnt);

    step  = 1;
    for (block_idx = 0; block_idx < pb->fpab.entry_count; block_idx = block_idx + step)
    {
        pe = pb->fpab.entries[block_idx];

        if((block_idx >= pb->fpab.start_offset[3]) && (pb->fpab.sub_entry_count[3]))        /*640 bit key*/
        {
            step = 8;
        }
        else if(block_idx >= pb->fpab.start_offset[2] && (pb->fpab.sub_entry_count[2]))   /*320 bit key*/
        {
            step = 4;
        }
        else if(block_idx >= pb->fpab.start_offset[1] && (pb->fpab.sub_entry_count[1]))   /*160 bit key*/
        {
            step = 2;
        }
        else                                            /*80 bit key*/
        {
            step = 1;
        }

        if (pe)
        {
            if(CTC_ACL_KEY_IPV4 == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV4, detail, show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV4_EXT, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV4, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV4_EXT, detail,show_type, p_grep, grep_cnt));
            }
            else if(CTC_ACL_KEY_IPV6 == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV6, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV6_EXT, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV6, detail,show_type, p_grep, grep_cnt));
            }
            else if(CTC_ACL_KEY_FWD == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_FWD, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_FWD_EXT, detail,show_type, p_grep, grep_cnt));
            }
            else
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, key_type, detail,show_type, p_grep, grep_cnt));
            }
        }
    }

    return CTC_E_NONE;
}

/*
 * show acl entries by group priority
 */
int32
_sys_usw_acl_show_entry_by_priority(uint8 lchip, uint8 prio, uint8 key_type, uint8 detail,uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint32         count = 1;
    sys_acl_block_t* pb;

    CTC_MAX_VALUE_CHECK(prio, (p_usw_acl_master[lchip]->igs_block_num - 1));

    count = 1;
    pb = &p_usw_acl_master[lchip]->block[prio];
    if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
    {
        CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_block(lchip, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
    }

    if(prio < p_usw_acl_master[lchip]->egs_block_num)
    {
        pb = &p_usw_acl_master[lchip]->block[prio + p_usw_acl_master[lchip]->igs_block_num];
        if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
        {
            CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_block(lchip, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
        }
    }

    if(detail)
    {
        count = 1;
        pb = &p_usw_acl_master[lchip]->block[prio];
        if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
        {
            CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_block(lchip, pb, &count, key_type, detail, show_type, p_grep, grep_cnt));
        }

        if (prio < p_usw_acl_master[lchip]->egs_block_num)
        {
            pb = &p_usw_acl_master[lchip]->block[prio + p_usw_acl_master[lchip]->igs_block_num];
            if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_block(lchip, pb, &count, key_type, detail, show_type, p_grep, grep_cnt));
            }
        }
    }

    return CTC_E_NONE;
}

/*
 * show acl entries in a block with specific key type
 */
STATIC int32
_sys_usw_acl_show_entry_in_lkup_level(uint8 lchip, uint8 block_id, uint8 level, sys_acl_block_t* pb, uint32* p_cnt, uint8 key_type, uint8 detail,uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    ctc_fpa_entry_t* pe = NULL;
    uint16         block_idx = 0;
    uint16         start_idx = 0;
    uint16         end_idx = 0;
    uint8          step = 0;
    sys_acl_league_t*    p_sys_league = NULL;

    CTC_PTR_VALID_CHECK(pb);
    CTC_PTR_VALID_CHECK(p_cnt);

    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);
    start_idx = p_sys_league->lkup_level_start[level];
    end_idx = p_sys_league->lkup_level_start[level] + p_sys_league->lkup_level_count[level];

    step  = 1;
    for (block_idx = start_idx; block_idx < end_idx; block_idx = block_idx + step)
    {
        pe = pb->fpab.entries[block_idx];

        if((block_idx >= pb->fpab.start_offset[3]) && (pb->fpab.sub_entry_count[3]))        /*640 bit key*/
        {
            step = 8;
        }
        else if(block_idx >= pb->fpab.start_offset[2] && (pb->fpab.sub_entry_count[2]))   /*320 bit key*/
        {
            step = 4;
        }
        else if(block_idx >= pb->fpab.start_offset[1] && (pb->fpab.sub_entry_count[1]))   /*160 bit key*/
        {
            step = 2;
        }
        else                                            /*80 bit key*/
        {
            step = 1;
        }

        if (pe)
        {
            if(CTC_ACL_KEY_IPV4 == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV4, detail, show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV4_EXT, detail, show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV4, detail, show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV4_EXT, detail,show_type, p_grep, grep_cnt));
            }
            else if(CTC_ACL_KEY_IPV6 == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV6, detail, show_type,p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_IPV6_EXT, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_MAC_IPV6, detail,show_type, p_grep, grep_cnt));
            }
            else if(CTC_ACL_KEY_FWD == key_type)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_FWD, detail,show_type, p_grep, grep_cnt));
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, CTC_ACL_KEY_FWD_EXT, detail,show_type, p_grep, grep_cnt));
            }
            else
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry(lchip, ACL_OUTER_ENTRY(pe), p_cnt, key_type, detail,show_type, p_grep, grep_cnt));
            }
        }
    }

    return CTC_E_NONE;
}

/*
 * show acl entries by lookup level
 */
int32
_sys_usw_acl_show_entry_by_lkup_level(uint8 lchip, uint8 level, uint8 key_type, uint8 detail,uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint32         count = 1;
    sys_acl_block_t* pb;
    sys_acl_league_t* p_sys_league = NULL;

    CTC_MAX_VALUE_CHECK(level, (p_usw_acl_master[lchip]->igs_block_num - 1));

    count = 1;
    p_sys_league = &(p_usw_acl_master[lchip]->league[level]);

    if (p_sys_league->lkup_level_bitmap)
    {
        pb = &p_usw_acl_master[lchip]->block[level];
        if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
        {
            CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, level, level, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
        }
    }
    else
    {
        pb = &p_usw_acl_master[lchip]->block[p_sys_league->merged_to];
        if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
        {
            CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, p_sys_league->merged_to, level, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
        }
    }

    p_sys_league = &(p_usw_acl_master[lchip]->league[level + p_usw_acl_master[lchip]->igs_block_num]);
    if(level < p_usw_acl_master[lchip]->egs_block_num)
    {
        if (p_sys_league->lkup_level_bitmap)
        {
            pb = &p_usw_acl_master[lchip]->block[level + p_usw_acl_master[lchip]->igs_block_num];
            if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, level + p_usw_acl_master[lchip]->igs_block_num, level, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
            }
        }
        else
        {
            pb = &p_usw_acl_master[lchip]->block[p_sys_league->merged_to + p_usw_acl_master[lchip]->igs_block_num];
            if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, p_sys_league->merged_to + p_usw_acl_master[lchip]->igs_block_num, level, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
            }
        }
    }

    if (detail)
    {
        if (p_sys_league->lkup_level_bitmap)
        {
            pb = &p_usw_acl_master[lchip]->block[level];
            if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, level, level, pb, &count, key_type, 1, show_type, p_grep, grep_cnt));
            }
        }
        else
        {
            pb = &p_usw_acl_master[lchip]->block[p_sys_league->merged_to];
            if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
            {
                CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, p_sys_league->merged_to, level, pb, &count, key_type, 1, show_type, p_grep, grep_cnt));
            }
        }

        p_sys_league = &(p_usw_acl_master[lchip]->league[level + p_usw_acl_master[lchip]->igs_block_num]);
        if (level < p_usw_acl_master[lchip]->egs_block_num)
        {
            if (p_sys_league->lkup_level_bitmap)
            {
                pb = &p_usw_acl_master[lchip]->block[level + p_usw_acl_master[lchip]->igs_block_num];
                if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
                {
                    CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, level + p_usw_acl_master[lchip]->igs_block_num, level, pb, &count, key_type, 1, show_type, p_grep, grep_cnt));
                }
            }
            else
            {
                pb = &p_usw_acl_master[lchip]->block[p_sys_league->merged_to + p_usw_acl_master[lchip]->igs_block_num];
                if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
                {
                    CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_lkup_level(lchip, p_sys_league->merged_to + p_usw_acl_master[lchip]->igs_block_num, level, pb, &count, key_type, 1, show_type, p_grep, grep_cnt));
                }
            }
        }
    }

    return CTC_E_NONE;
}

/*
 * callback for hash traverse. traverse group hash table, show each group's entry.
 *
 */
STATIC int32
_sys_usw_acl_hash_traverse_cb_show_entry(sys_acl_group_t* pg, _acl_cb_para_t* cb_para)
{
    uint8 lchip = 0;

    CTC_PTR_VALID_CHECK(pg);
    CTC_PTR_VALID_CHECK(cb_para);
    lchip = pg->lchip;

    CTC_ERROR_RETURN(_sys_usw_acl_show_entry_in_group(lchip, pg,cb_para));
    return CTC_E_NONE;
}

/*
 * callback for hash traverse. traverse group hash table, show each group's group id.
 *
 */
int32
_sys_usw_acl_hash_traverse_cb_show_inner_gid(sys_acl_group_t* pg, _acl_cb_para_t* cb_para)
{
    CTC_PTR_VALID_CHECK(cb_para);

    if(pg->group_id>CTC_ACL_GROUP_ID_NORMAL)
    {
        cb_para->count++;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.%03u  :  gid 0x%0X \n", cb_para->count, pg->group_id);
    }

    return CTC_E_NONE;
}

/*
 * callback for hash traverse. traverse group hash table, show each group's group id.
 *
 */
int32
_sys_usw_acl_hash_traverse_cb_show_outer_gid(sys_acl_group_t* pg, _acl_cb_para_t* cb_para)
{
    CTC_PTR_VALID_CHECK(cb_para);

    if(pg->group_id<CTC_ACL_GROUP_ID_NORMAL)
    {
        cb_para->count++;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.%03u  :  gid 0x%-X \n", cb_para->count, pg->group_id);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_hash_traverse_cb_show_group(sys_acl_group_t* pg, _acl_cb_para_t* cb_para)
{
    char str[35];
    char format[10];
    char* dir[3] = {"IGS", "EGS", "BOTH"};
    char* type[CTC_ACL_GROUP_TYPE_MAX] = {"NONE", "HASH", "PORT_BITMAP", "GLOBAL", "VLAN_CLASS", "PORT_CLASS", "SERVICE_ACL",
                                                                       "L3IF_CLASS", "PBR_CLASS", "GPORT"};

    CTC_PTR_VALID_CHECK(cb_para);

    if(pg)
    {
        cb_para->count++;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6d", cb_para->count);

        if (SYS_ACL_SHOW_IN_HEX <= pg->group_id)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s", CTC_DEBUG_HEX_FORMAT(str, format, pg->group_id, 8, U));
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", pg->group_id);
        }
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s", dir[pg->group_info.dir]);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", pg->group_info.block_id);

        if (pg->group_id == CTC_ACL_GROUP_ID_HASH_MAC)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", "HASH_MAC");
        }
        else if (pg->group_id == CTC_ACL_GROUP_ID_HASH_IPV4)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", "HASH_IPV4");
        }
        else if (pg->group_id == CTC_ACL_GROUP_ID_HASH_MPLS)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", "HASH_MPLS");
        }
        else if (pg->group_id == CTC_ACL_GROUP_ID_HASH_L2_L3)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", "HASH_L2L3");
        }
        else if (pg->group_id == CTC_ACL_GROUP_ID_HASH_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", "HASH_IPV6");
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-14s", type[pg->group_info.type]);
        }

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10u", pg->entry_count);

        if (pg->group_info.type == CTC_ACL_GROUP_TYPE_PORT)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"0x%-6.4x\n", pg->group_info.un.gport);
        }
        else if (pg->group_info.type == CTC_ACL_GROUP_TYPE_PORT_BITMAP)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"0x%-.8x%-.8x%-.8x%-.8x\n", pg->group_info.un.port_bitmap[3], pg->group_info.un.port_bitmap[2],
                pg->group_info.un.port_bitmap[1],pg->group_info.un.port_bitmap[0]);

        }
        else if ((pg->group_info.type == CTC_ACL_GROUP_TYPE_HASH) || (pg->group_info.type == CTC_ACL_GROUP_TYPE_GLOBAL))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s\n", "-");
        }
        else if (pg->group_info.type == CTC_ACL_GROUP_TYPE_PBR_CLASS)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-u\n", pg->group_info.un.pbr_class_id);
        }
        else if (pg->group_info.type == CTC_ACL_GROUP_TYPE_VLAN_CLASS)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-u\n", pg->group_info.un.vlan_class_id);
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-u\n", pg->group_info.un.port_class_id);
        }


    }

    return CTC_E_NONE;
}

/*
 * show all acl entries separate by priority or group
 * sort_type = 0: by priority
 * sort_type = 1: by group
 */
int32
_sys_usw_acl_show_entry_all(uint8 lchip, uint8 sort_type, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    _acl_cb_para_t para;

    sal_memset(&para, 0, sizeof(_acl_cb_para_t));
    para.count = 1; /* No.*/
    para.key_type = key_type;
    para.p_grep = p_grep;
    para.grep_cnt = grep_cnt;
    para.show_type = show_type;

    if (sort_type == 0)
    {   /*sort tcam  --> hash */
        /* tcam entry by priority */

        para.detail   = 0;
		para.entry_type = 0;
        ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                  (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);

        para.entry_type = 1;
        ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                  (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);

        if(detail)
        {
            para.detail = 1;
            para.entry_type = 0;
            ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                      (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);

            para.entry_type = 1;
            ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                      (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);
        }
        /* hash entry by group */
    }
    else /* separate by group */
    {
        para.detail   = 0;
        para.entry_type = 2;
        ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                  (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);
        if(detail)
        {
            para.detail   = 1;
            ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                                      (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_entry, &para);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_show_spool_status(uint8 lchip)
{
    SYS_ACL_INIT_CHECK();

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.   name        max_count    used_cout\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"1     ad_spool    %-13u%-10u\n", p_usw_acl_master[lchip]->ad_spool->max_count, \
                                                        p_usw_acl_master[lchip]->ad_spool->count);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"2     vlan_spool  %-13u%-10u\n", p_usw_acl_master[lchip]->vlan_edit_spool->max_count, \
                                                        p_usw_acl_master[lchip]->vlan_edit_spool->count);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"3     cid_spool   %-13u%-10u\n", p_usw_acl_master[lchip]->cid_spool->max_count, \
                                                        p_usw_acl_master[lchip]->cid_spool->count);

    return CTC_E_NONE;
}

#define __ACL_OTHER__

int32
_sys_usw_acl_check_vlan_edit(uint8 lchip, ctc_acl_vlan_edit_t*   p_ctc, uint8* vlan_edit)
{
    /* check tag op */
    CTC_MAX_VALUE_CHECK(p_ctc->stag_op, CTC_ACL_VLAN_TAG_OP_MAX - 1);
    CTC_MAX_VALUE_CHECK(p_ctc->ctag_op, CTC_ACL_VLAN_TAG_OP_MAX - 1);

    if ((CTC_ACL_VLAN_TAG_OP_NONE == p_ctc->stag_op)
        && (CTC_ACL_VLAN_TAG_OP_NONE == p_ctc->ctag_op))
    {
        *vlan_edit = 0;
        return CTC_E_NONE;
    }

    *vlan_edit = 1;

    if ((CTC_ACL_VLAN_TAG_OP_ADD == p_ctc->stag_op) ||
        (CTC_ACL_VLAN_TAG_OP_REP == p_ctc->stag_op))
    {
        CTC_MAX_VALUE_CHECK(p_ctc->svid_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_ctc->scos_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_ctc->scfi_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);

        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->svid_sl)
        {
            CTC_VLAN_RANGE_CHECK(p_ctc->svid_new);
        }
        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->scos_sl)
        {
            CTC_COS_RANGE_CHECK(p_ctc->scos_new);
        }
        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->scfi_sl)
        {
            CTC_MAX_VALUE_CHECK(p_ctc->scfi_new, 1);
        }
    }

    if ((CTC_ACL_VLAN_TAG_OP_ADD == p_ctc->ctag_op) ||
        (CTC_ACL_VLAN_TAG_OP_REP == p_ctc->ctag_op))
    {
        CTC_MAX_VALUE_CHECK(p_ctc->cvid_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_ctc->ccos_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_ctc->ccfi_sl, CTC_ACL_VLAN_TAG_SL_MAX - 1);

        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->cvid_sl)
        {
            CTC_VLAN_RANGE_CHECK(p_ctc->cvid_new);
        }
        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->ccos_sl)
        {
            CTC_COS_RANGE_CHECK(p_ctc->ccos_new);
        }
        if (CTC_ACL_VLAN_TAG_SL_NEW == p_ctc->ccfi_sl)
        {
            CTC_MAX_VALUE_CHECK(p_ctc->ccfi_new, 1);
        }
    }
    CTC_MAX_VALUE_CHECK(p_ctc->tpid_index, 3);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_group_key_port_field(uint8 lchip, uint32 entry_id,sys_acl_group_t* pg)
{
        ctc_field_port_t key_port;
        ctc_field_port_t key_port_mask;
		ctc_field_key_t key_field;
		uint8 is_add = 1;

		sal_memset(&key_port, 0, sizeof(ctc_field_port_t));
		sal_memset(&key_port_mask, 0, sizeof(ctc_field_port_t));
		sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

		key_field.type = CTC_FIELD_KEY_PORT;
		key_field.ext_data = &key_port;
	    key_field.ext_mask= &key_port_mask;

        switch(pg->group_info.type)
        {
        case CTC_ACL_GROUP_TYPE_PORT_BITMAP:
            key_port.type = CTC_FIELD_PORT_TYPE_PORT_BITMAP;
            sal_memcpy(key_port.port_bitmap,pg->group_info.un.port_bitmap,sizeof(ctc_port_bitmap_t));
			sal_memset(key_port_mask.port_bitmap,0xFF,sizeof(ctc_port_bitmap_t));
            key_port.lchip = pg->group_info.lchip;
            break;
        case CTC_ACL_GROUP_TYPE_VLAN_CLASS:   /**< [HB.GB.GG.D2] A vlan class acl is against a class(group) of vlan*/
            key_port.vlan_class_id= pg->group_info.un.vlan_class_id;
            key_port.type = CTC_FIELD_PORT_TYPE_VLAN_CLASS;
			sal_memset(&key_port_mask.vlan_class_id,0xFF,sizeof(key_port.vlan_class_id));
            break;
        case  CTC_ACL_GROUP_TYPE_PORT_CLASS:   /**< [HB.GB.GG.D2] A port class acl is against a class(group) of port*/
            key_port.port_class_id= pg->group_info.un.port_class_id;
            key_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
			sal_memset(&key_port_mask.port_class_id,0xFF,sizeof(key_port.port_class_id));
            break;
        case CTC_ACL_GROUP_TYPE_L3IF_CLASS:   /**< [D2] A l3if class acl is against a class(group) of l3if*/
            key_port.l3if_class_id= pg->group_info.un.l3if_class_id;
            key_port.type = CTC_FIELD_PORT_TYPE_L3IF_CLASS;
			sal_memset(&key_port_mask.l3if_class_id,0xFF,sizeof(key_port.l3if_class_id));
            break;
        case CTC_ACL_GROUP_TYPE_PBR_CLASS:    /**< [HB.GB.GG.D2] A pbr class is against a class(group) of l3 source interface*/
            key_port.pbr_class_id= pg->group_info.un.pbr_class_id;
            key_port.type = CTC_FIELD_PORT_TYPE_PBR_CLASS;
			sal_memset(&key_port_mask.pbr_class_id,0xFF,sizeof(key_port.pbr_class_id));
            break;
        case CTC_ACL_GROUP_TYPE_PORT:        /**< [GG.D2] Port acl, care gport. */
            key_port.gport= pg->group_info.un.gport;
            key_port.type = CTC_FIELD_PORT_TYPE_GPORT;
			sal_memset(&key_port_mask.gport,0xFF,sizeof(key_port.gport));
            break;
        case CTC_ACL_GROUP_TYPE_SERVICE_ACL:        /**< [GB.GG.D2] SERVICE_ACL, care logic port. */
            key_port.logic_port = pg->group_info.un.service_id;
            key_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
			sal_memset(&key_port_mask.logic_port,0xFF,sizeof(key_port.logic_port));
            break;
		default :
			is_add = 0;
			break;
        }
		if(is_add)
		{
            CTC_ERROR_RETURN(_sys_usw_acl_operate_key_field(lchip, entry_id, &key_field, 1));
		}


    return CTC_E_NONE;
}
int32 _sys_usw_acl_tcam_cid_move_cb(uint8 lchip, uint32 new_offset, uint32 old_offset, void* user_data)
{
    uint32 cmd = 0;
    DsCategoryIdPairTcamKey_m  ds_key;
    DsCategoryIdPairTcamKey_m  ds_mask;
    DsCategoryIdPairTcamAd_m     tcam_ad;
    tbl_entry_t   tcam_key;

    sal_memset(&tcam_key, 0, sizeof(tcam_key));
    sal_memset(&ds_key, 0, sizeof(ds_key));
    sal_memset(&ds_mask, 0, sizeof(ds_mask));
    sal_memset(&tcam_ad, 0, sizeof(tcam_ad));

    cmd = DRV_IOR(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, old_offset, cmd, &tcam_ad));
    cmd = DRV_IOW(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, new_offset, cmd, &tcam_ad));

    tcam_key.data_entry = (uint32*)&ds_key;
    tcam_key.mask_entry = (uint32*)&ds_mask;

    cmd = DRV_IOR(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, old_offset, cmd, &tcam_key));
    cmd = DRV_IOW(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, new_offset, cmd, &tcam_key));

    sal_memset(&tcam_key, 0, sizeof(tcam_key));
    cmd = DRV_IOD(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, old_offset, cmd, &tcam_key));
    return CTC_E_NONE;
}

#define _ACL_GROUP_

int32
_sys_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    sys_acl_group_t* pg_new = NULL;
    int32          ret;
    struct ctc_slist_s* pslist = NULL;

    /* malloc an empty group */
    MALLOC_ZERO(MEM_ACL_MODULE, pg_new, sizeof(sys_acl_group_t));
    if (!pg_new)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }
    pg_new->lchip = lchip;
    if (!SYS_ACL_IS_RSV_GROUP(group_id))
    {
        if (!group_info)
        {
            ret = CTC_E_INVALID_PARAM;
            goto error1;
        }
        CTC_ERROR_GOTO(_sys_usw_acl_check_group_info(lchip, group_info, group_info->type, 1, ACL_BITMAP_STATUS_64), ret, error1);
        CTC_ERROR_GOTO(_sys_usw_acl_map_group_info(lchip, &pg_new->group_info, group_info, 1), ret, error1);
    }
    else
    {
        pg_new->group_info.type = CTC_ACL_GROUP_TYPE_HASH;
    }

    pg_new->group_id = group_id;
    pslist = ctc_slist_new();
    if(pslist)
    {
        pg_new->entry_list = pslist;
    }
    else
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    /* add to hash */
    if (NULL == ctc_hash_insert(p_usw_acl_master[lchip]->group, pg_new))
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }
    return CTC_E_NONE;
error2:
    ctc_slist_free(pg_new->entry_list);
error1:
    mem_free(pg_new);
error0:
    return ret;
}

int32
_sys_usw_acl_destroy_group(uint8 lchip, uint32 group_id)
{
    sys_acl_group_t* pg = NULL;

    /* check if group exist */
    CTC_ERROR_RETURN(_sys_usw_acl_get_group_by_gid(lchip, group_id, &pg));
    if (!pg)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");
		return CTC_E_NOT_EXIST;
    }

    /* check if all entry removed */
    if (0 != pg->entry_list->count)
    {
        return CTC_E_NOT_READY;
    }

    ctc_hash_remove(p_usw_acl_master[lchip]->group, pg);

    /* free slist */
    ctc_slist_free(pg->entry_list);

    /* free pg */
    mem_free(pg);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    sys_acl_group_t       * pg = NULL;
    struct ctc_slistnode_s* pe_node = NULL;
    int32                 ret = CTC_E_NONE;
    int32                 ret_temp = CTC_E_NONE;
    uint32                eid = 0;

    /* get group node */
    CTC_ERROR_RETURN(_sys_usw_acl_get_group_by_gid(lchip, group_id, &pg));
    if (!pg)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");
		return CTC_E_NOT_EXIST;
    }

    /* traverse, stop at first installed entry.*/
    if (pg->entry_list)
    {
        for (pe_node = pg->entry_list->head;
             pe_node && (((sys_acl_entry_t *) pe_node)->fpae.flag != FPA_ENTRY_FLAG_INSTALLED);
             pe_node = pe_node->next)
        {
            ret_temp = _sys_usw_acl_install_entry(lchip, (sys_acl_entry_t *) pe_node, SYS_ACL_ENTRY_OP_FLAG_ADD, 0);
            if(CTC_E_NONE != ret_temp)
            {
                ret = ret_temp;
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"sys_usw_acl_install_group(),install entry (eid: %u)failed!\n",eid);
            }
        }
    }

    return ret;
}

int32
_sys_usw_acl_uninstall_group(uint8 lchip, uint32 group_id)
{
    sys_acl_group_t       * pg = NULL;
    struct ctc_slistnode_s* pe = NULL;

    /* get group node */
    CTC_ERROR_RETURN(_sys_usw_acl_get_group_by_gid(lchip, group_id, &pg));
    if (!pg)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");
		return CTC_E_NOT_EXIST;
    }

    CTC_SLIST_LOOP(pg->entry_list, pe)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_install_entry(lchip, (sys_acl_entry_t *) pe, SYS_ACL_ENTRY_OP_FLAG_DELETE, 0));
    }

    return CTC_E_NONE;
}

#define _ACL_ENTRY_

/*
 * check entry type if it is valid.
 */
STATIC int32
_sys_usw_acl_check_entry_type(uint8 lchip, uint32 group_id, sys_acl_group_t* pg, ctc_acl_entry_t* p_ctc_entry)
{
    CTC_PTR_VALID_CHECK(p_ctc_entry);
    CTC_PTR_VALID_CHECK(pg);
    if ((CTC_EGRESS == pg->group_info.dir) && (CTC_ACL_KEY_CID == p_ctc_entry->key_type || CTC_ACL_KEY_FWD == p_ctc_entry->key_type || CTC_ACL_KEY_FWD_EXT == p_ctc_entry->key_type || CTC_ACL_KEY_COPP == p_ctc_entry->key_type || CTC_ACL_KEY_COPP_EXT == p_ctc_entry->key_type))
    {
        return CTC_E_INVALID_PARAM;
    }
    switch (p_ctc_entry->key_type)
    {
    case CTC_ACL_KEY_MAC:
    case CTC_ACL_KEY_IPV4:
    case CTC_ACL_KEY_IPV4_EXT:
    case CTC_ACL_KEY_MAC_IPV4:
    case CTC_ACL_KEY_MAC_IPV4_EXT:
    case CTC_ACL_KEY_IPV6:
    case CTC_ACL_KEY_IPV6_EXT:
    case CTC_ACL_KEY_MAC_IPV6:
    case CTC_ACL_KEY_CID:
    case CTC_ACL_KEY_INTERFACE:
    case CTC_ACL_KEY_FWD:
    case CTC_ACL_KEY_FWD_EXT:
    case CTC_ACL_KEY_UDF :
        if (CTC_ACL_GROUP_TYPE_HASH == pg->group_info.type)
        {
            return CTC_E_INVALID_PARAM;
        }
    break;

    case CTC_ACL_KEY_COPP:
    case CTC_ACL_KEY_COPP_EXT:
        if ((CTC_ACL_GROUP_TYPE_GLOBAL != pg->group_info.type) && (CTC_ACL_GROUP_TYPE_PORT_BITMAP != pg->group_info.type) && (CTC_ACL_GROUP_TYPE_NONE != pg->group_info.type))
        {
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_ACL_KEY_HASH_MAC:
    case CTC_ACL_KEY_HASH_IPV4:
    case CTC_ACL_KEY_HASH_IPV6:
    case CTC_ACL_KEY_HASH_MPLS:
    case CTC_ACL_KEY_HASH_L2_L3:
        if(((0xFFFF0001 == group_id)&&(CTC_ACL_KEY_HASH_MAC!=p_ctc_entry->key_type))
            ||((0xFFFF0002 == group_id)&&(CTC_ACL_KEY_HASH_IPV4!=p_ctc_entry->key_type))
            ||((0xFFFF0003 == group_id)&&(CTC_ACL_KEY_HASH_MPLS!=p_ctc_entry->key_type))
            ||((0xFFFF0004 == group_id)&&(CTC_ACL_KEY_HASH_L2_L3!=p_ctc_entry->key_type))
            ||((0xFFFF0005 == group_id)&&(CTC_ACL_KEY_HASH_IPV6!=p_ctc_entry->key_type)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The Key Type is not include in The Group.\n  The Group Id is :0x%x ,The Key Type is :%d\n", group_id, p_ctc_entry->key_type);
            return CTC_E_INVALID_PARAM;
        }
        break;

    default: /*pbr_ipv4,pbr_ipv6,mpls*/
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    return CTC_E_NONE;
}

/*
  ad_type:  0-DsAcl, 1-DsFlow
*/
int32
_sys_usw_acl_bind_nexthop(uint8 lchip, sys_acl_entry_t* pe,uint32 nh_id,uint8 ad_type)
{
    sys_nh_update_dsnh_param_t update_dsnh;
	int32 ret = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    /* nh don't use dsfwd, need bind nh and dsacl */
    update_dsnh.data = pe;
    update_dsnh.updateAd = ad_type?_sys_usw_acl_update_dsflow_action:_sys_usw_acl_update_action;
    ret = sys_usw_nh_bind_dsfwd_cb(lchip, nh_id, &update_dsnh);
    if(CTC_E_NONE == ret)
    {
       /*MergeDsFwd Mode need bind ad and nh*/
       CTC_SET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);
    }
    return ret;
}

int32
_sys_usw_acl_unbind_nexthop(uint8 lchip, sys_acl_entry_t* pe)
{
    if (CTC_FLAG_ISSET(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH)
        && pe->nexthop_id)
    {
        sys_nh_update_dsnh_param_t update_dsnh;
        update_dsnh.data = NULL;
        update_dsnh.updateAd = NULL;
        sys_usw_nh_bind_dsfwd_cb(lchip, pe->nexthop_id, &update_dsnh);
        CTC_UNSET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_check_discard_type(uint8 discard_type)
{
    /* not support discard_type which exist after acl process:
            IPEDISCARDTYPE_UC_TO_LAG_GROUP_NO_MEMBER          0x3
            IPEDISCARDTYPE_MICROFLOW_POLICING_FAIL_DROP       0x7
            IPEDISCARDTYPE_STACK_REFLECT_DISCARD              0xd
            IPEDISCARDTYPE_STORM_CTL_DIS                      0x14
            IPEDISCARDTYPE_NO_FORWARDING_PTR                  0x16
            IPEDISCARDTYPE_DS_PLC_DIS                          0x2c
            IPEDISCARDTYPE_DS_ACL_DIS                          0x2d
            IPEDISCARDTYPE_CFLAX_SRC_ISOLATE_DIS              0x31
            IPEDISCARDTYPE_SD_CHECK_DIS                          0x3f
    */
    if ((discard_type >= 0x3f) || (0x3 == discard_type) || (0x7 == discard_type) || (0xd == discard_type)
        || (0x14 == discard_type) || (0x16 == discard_type) || (0x2c == discard_type) || (0x2d == discard_type)
        || (0x31 == discard_type))
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_check_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint8 step = 0;
    uint32 cmd = 0;
    FlowTcamLookupCtl_m flow_ctl;
    IpePreLookupAclCtl_m ipe_pre_lkup_ctl;
    uint8 l3_key_share_field_mode;
    uint8 key_port_is_cid = 0 ;
    uint8 cid_key_v6_use_ipda = 0;
    uint8 cid_key = 0;
    uint8 copp_key = 0;
    uint8 copp_ext_key = 0;
    uint8 copp_basic_key = 0;
    uint8 copp_key_use_ctag = 0;
    uint8 copp_key_use_l3l4_type = 0;
    uint8 copp_basic_key_ipaddr_use_ipsa = 0;
    uint8 copp_ext_key_udf_use_ipv6 = 0;
    uint8 copp_ext_key_ipv6sa =0;
    uint8 fwd_ext_key = 0;
    uint8 fwd_ext_key_use_l2_hdr = 0;
    uint8 fwd_ext_key_udf_use_nsh = 0;
    uint8 arware_tunnel_info = 0;
    uint8 l3_key = 0;
    uint8 mac_key = 0;
    uint8 short_key = 0;
    uint8 short_key_use_svlan = 0;
    uint8 fwd_key = 0;
    uint8 l3_ext_key = 0;
    uint8 mac_l3_key = 0;
    uint8 mac_l3_ext_key = 0;
    uint8 udf_key = 0;
    uint8 ipv6_ext_key = 0;
    uint8 mac_l3_ext_key_mac_da_mode = 0;
    uint8 mac_l3_ext_key_mac_sa_mode = 0;
    uint8 mac_l3_key_mac_sa_mode = 0;
    uint8 mac_l3_key_mac_da_mode = 0;
    uint8 l2type_use_vlanNum = 0;
    uint8 l3_type = pe->l3_type;
    uint8 l4_type = pe->l4_type;
    uint8 l4_user_type = pe->l4_user_type;
    uint8 copp_key_basic_mac_da_mode = 0;
    uint8  discard_type = 0;
    uint32 xgpon_use_gem_port = 0;
    uint8 service_queue_egress_en = 0;

    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_useGemPortLookup_f);
    DRV_IOCTL(lchip, 0, cmd, &xgpon_use_gem_port);

    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &flow_ctl);

    if (CTC_INGRESS == pe->group->group_info.dir)
    {
        step = FlowTcamLookupCtl_gIngrAcl_1_categoryIdFieldValid_f - FlowTcamLookupCtl_gIngrAcl_0_categoryIdFieldValid_f;
        key_port_is_cid = GetFlowTcamLookupCtl(V, gIngrAcl_0_categoryIdFieldValid_f + step *  pe->group->group_info.block_id, &flow_ctl);
        cid_key_v6_use_ipda = GetFlowTcamLookupCtl(V, ingrSgaclKeyIpv6PktUseIpDaField_f, &flow_ctl);
        if (pe->key_type == CTC_ACL_KEY_COPP)
        {
            copp_key_use_ctag = GetFlowTcamLookupCtl(V, coppBasicKeyUseCtag_f, &flow_ctl);
            copp_key_use_l3l4_type = GetFlowTcamLookupCtl(V, coppBasicKeyEtherTypeShareType_f, &flow_ctl);/*default to 1*/
            copp_basic_key_ipaddr_use_ipsa = GetFlowTcamLookupCtl(V, coppKeyIpAddrShareType0_f, &flow_ctl); /*default to 0*/
        }
        if (pe->key_type == CTC_ACL_KEY_COPP_EXT)
        {
            copp_key_use_ctag = GetFlowTcamLookupCtl(V, coppExtKeyUseCtag_f, &flow_ctl);
            copp_key_use_l3l4_type = GetFlowTcamLookupCtl(V, coppExtKeyEtherTypeShareType_f, &flow_ctl);/*default to 1*/
            copp_ext_key_udf_use_ipv6 = 1 & GetFlowTcamLookupCtl(V, coppExtKeyUdfShareType_f, &flow_ctl); /*default to 1*/
            copp_ext_key_ipv6sa = 1 & GetFlowTcamLookupCtl(V, coppKeyIpAddrShareType0_f, &flow_ctl); /*default to 1*/
        }
    }
    else
    {
        step = FlowTcamLookupCtl_gEgrAcl_1_categoryIdFieldValid_f - FlowTcamLookupCtl_gEgrAcl_0_categoryIdFieldValid_f;
        key_port_is_cid = GetFlowTcamLookupCtl(V, gEgrAcl_0_categoryIdFieldValid_f + step *  pe->group->group_info.block_id, &flow_ctl);
        cid_key_v6_use_ipda = GetFlowTcamLookupCtl(V, egrSgaclKeyIpv6PktUseIpDaField_f, &flow_ctl);
    }

    l3_key_share_field_mode = GetFlowTcamLookupCtl(V,gIngrAcl_0_l3Key160ShareFieldType_f,&flow_ctl);
    arware_tunnel_info = !GetFlowTcamLookupCtl(V,array_0_ignorMergeMode_f,&flow_ctl);

    fwd_ext_key_use_l2_hdr = !(GetFlowTcamLookupCtl(V,forwardingAclKeyUnion1Type_f,&flow_ctl) & (1 << (pe->group->group_info.block_id &0x7)));
    fwd_ext_key_udf_use_nsh = GetFlowTcamLookupCtl(V,forceForwardingAclKeyUseNsh_f,&flow_ctl) & (1 << (pe->group->group_info.block_id&0x7));
    short_key_use_svlan =  !GetFlowTcamLookupCtl(V,acl80bitKeyUseCvlan_f,&flow_ctl);

    mac_l3_ext_key_mac_sa_mode = GetFlowTcamLookupCtl(V,aclMacL3ExtKeyArpPktUseSenderMac_f,&flow_ctl);
    mac_l3_ext_key_mac_da_mode= GetFlowTcamLookupCtl(V,aclMacL3ExtKeyArpPktUseTargetMac_f,&flow_ctl);
    mac_l3_key_mac_sa_mode = GetFlowTcamLookupCtl(V,aclMacL3BasicKeyArpPktUseSenderMac_f,&flow_ctl);
    mac_l3_key_mac_da_mode = GetFlowTcamLookupCtl(V,aclMacL3BasicKeyArpPktUseTargetMac_f,&flow_ctl);
    copp_key_basic_mac_da_mode = !GetFlowTcamLookupCtl(V,coppBasicKeyDisableArpShareMacDa_f,&flow_ctl);

    cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ipe_pre_lkup_ctl);
    l2type_use_vlanNum = GetIpePreLookupAclCtl(V, layer2TypeUsedAsVlanNum_f, &ipe_pre_lkup_ctl);

    cid_key = pe->key_type == CTC_ACL_KEY_CID;
    copp_ext_key = pe->key_type == CTC_ACL_KEY_COPP_EXT;
    copp_basic_key = pe->key_type == CTC_ACL_KEY_COPP;
    copp_key = copp_basic_key || copp_ext_key;
    fwd_ext_key =  pe->key_type == CTC_ACL_KEY_FWD_EXT;
    fwd_key =  pe->key_type == CTC_ACL_KEY_FWD;
    l3_key = pe->key_type == CTC_ACL_KEY_IPV4;
    mac_key = pe->key_type == CTC_ACL_KEY_MAC;
    l3_ext_key = pe->key_type == CTC_ACL_KEY_IPV4_EXT;
    mac_l3_key = pe->key_type == CTC_ACL_KEY_MAC_IPV4;
    mac_l3_ext_key = pe->key_type == CTC_ACL_KEY_MAC_IPV4_EXT;
    short_key = pe->key_type == CTC_ACL_KEY_INTERFACE;
    udf_key = pe->key_type == CTC_ACL_KEY_UDF;
    ipv6_ext_key = pe->key_type == CTC_ACL_KEY_IPV6_EXT;

    if ((key_field->type == CTC_FIELD_KEY_ETHER_TYPE) && (CTC_ACL_KEY_MAC != pe->key_type)&&(CTC_ACL_KEY_UDF != pe->key_type) &&
        /* SYSTEM MODIFYED by huwx for acl ether type mask mapping 2018-05-14 */ 
        (0xFFFFFFFF == key_field->mask))
    {
        uint8 tmp_l3_type = 0;
        _sys_usw_acl_map_ethertype(key_field->data, &tmp_l3_type);
        if (tmp_l3_type != CTC_PARSER_L3_TYPE_NONE)
        {
            key_field->type = CTC_FIELD_KEY_L3_TYPE;
            key_field->data = tmp_l3_type;
            key_field->mask = 0xF;
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Key filed:%s (EthType:0x%x) mapping to %s (l3_type:%d)n", "CTC_FIELD_KEY_ETHER_TYPE", key_field->data, \
            "CTC_FIELD_KEY_L3_TYPE", tmp_l3_type);
        }
    }

    if (!key_field->ext_data)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Key filed:%d,keyData:%d\n", key_field->type, key_field->data)
    }
    else
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Key filed:%d ", key_field->type)
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2_type:%d l3_type:%d l4_type:%d,l4 userType:%d \n", pe->l2_type, pe->l3_type, pe->l4_type, pe->l4_user_type);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac_l3_key_mac_sa_mode:%d, mac_l3_key_mac_da_mode:%d \n", mac_l3_key_mac_sa_mode, mac_l3_key_mac_da_mode);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if (pe->key_port_mode == 2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Key Port check conflict,key_port_mode:%d \n", pe->key_port_mode);
            return CTC_E_PARAM_CONFLICT;
        }
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        if ((!xgpon_use_gem_port) || DRV_IS_DUET2(lchip))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Gem port not support \n");
            return CTC_E_NOT_SUPPORT;
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        if ((pe->key_port_mode == 1) || key_port_is_cid)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Key Port check conflict,key_port_mode:%d key_is_cid:%d \n", pe->key_port_mode, key_port_is_cid);
            return CTC_E_PARAM_CONFLICT;
        }
        SYS_ACL_METADATA_CHECK(pe->group->group_info.type);
        CTC_MAX_VALUE_CHECK(key_field->data, 0x3FFF);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        if (!cid_key &&((pe->key_port_mode == 1) || !key_port_is_cid))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Key Port check conflict,key_port_mode:%d key_is_cid:%d \n", pe->key_port_mode, key_port_is_cid);
            return CTC_E_INVALID_PARAM;
        }
		CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
        if(pe->group->group_info.dir == CTC_EGRESS && service_queue_egress_en)
        {
            return CTC_E_NOT_SUPPORT;
        }
        break;
    case CTC_FIELD_KEY_DST_CID:
        if (!cid_key && ((pe->key_port_mode == 1) || !key_port_is_cid))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Key Port check conflict,key_port_mode:%d key_is_cid:%d \n", pe->key_port_mode, key_port_is_cid);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        break;
    case CTC_FIELD_KEY_MAC_SA:                   /**< [D2] Source MAC Address. */
    case CTC_FIELD_KEY_MAC_DA:                  /**< [D2] Destination MAC Address. */
        if (fwd_ext_key && !fwd_ext_key_use_l2_hdr)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key use_l2_hdr:%d \n", fwd_ext_key_use_l2_hdr);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_ext_key
            && key_field->type == CTC_FIELD_KEY_MAC_SA
        && (l3_type  == CTC_PARSER_L3_TYPE_ARP)
        && mac_l3_ext_key_mac_sa_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_ext_key mac_sa_mode:%d \n", mac_l3_ext_key_mac_sa_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && key_field->type == CTC_FIELD_KEY_MAC_SA
        && (l3_type  == CTC_PARSER_L3_TYPE_ARP)
        && mac_l3_key_mac_sa_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_key mac_sa_mode:%d \n", mac_l3_key_mac_sa_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && key_field->type == CTC_FIELD_KEY_MAC_DA
        && (l3_type  == CTC_PARSER_L3_TYPE_ARP)
        && mac_l3_key_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_key mac_da_mode:%d \n", mac_l3_key_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (copp_basic_key
            && key_field->type == CTC_FIELD_KEY_MAC_DA
        && (l3_type  == CTC_PARSER_L3_TYPE_ARP)
        && copp_key_basic_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key mac_sa_mode:%d \n", copp_key_basic_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }

        if (mac_l3_ext_key
            && key_field->type == CTC_FIELD_KEY_MAC_DA
        && (l3_type  == CTC_PARSER_L3_TYPE_ARP)
        && mac_l3_ext_key_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_ext_key mac_da_mode:%d \n", mac_l3_ext_key_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && key_field->type == CTC_FIELD_KEY_MAC_DA
            && pe->macl3basic_key_macda_mode == 2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_basic_key mac_da_mode:%d \n", pe->macl3basic_key_macda_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_STAG_VALID:               /**< [D2] S-Vlan Exist. */
    case CTC_FIELD_KEY_SVLAN_ID:                 /**< [D2] S-Vlan ID. */
    case CTC_FIELD_KEY_STAG_COS:                 /**< [D2] Stag Cos. */
    case CTC_FIELD_KEY_STAG_CFI:                 /**< [D2] Stag Cfi. */
    case CTC_FIELD_KEY_SVLAN_RANGE:              /**< [D2] Svlan Range;
        data: min, mask: max, ext_data: (uint32*)vrange_gid. */
        if (copp_key && copp_key_use_ctag)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key copp_key_use_ctag:%d \n", copp_key_use_ctag);
            return CTC_E_INVALID_PARAM;
        }
        if (fwd_ext_key && !fwd_ext_key_use_l2_hdr)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key use_l2_hdr:%d \n", fwd_ext_key_use_l2_hdr);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_key && (pe->mac_key_vlan_mode == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key vlan_mode:%d \n", pe->mac_key_vlan_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (short_key && !short_key_use_svlan)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "interface key use_svlan:%d \n", short_key_use_svlan);
            return CTC_E_INVALID_PARAM;
        }
        if(fwd_key && GetFlowTcamLookupCtl(V,aclForwardingBasicKeyUseCvlan_f,&flow_ctl))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "forward key in use cvlan mode\n");
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_SVLAN_ID)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_STAG_COS)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 7);
        }
        if (key_field->type == CTC_FIELD_KEY_STAG_CFI)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_CTAG_VALID:               /**< [D2] C-Vlan Exist. */
    case CTC_FIELD_KEY_CVLAN_ID:                 /**< [D2] C-Vlan ID. */
    case CTC_FIELD_KEY_CTAG_COS:                 /**< [D2] Ctag Cos. */
    case CTC_FIELD_KEY_CTAG_CFI:                 /**< [D2] Ctag Cfi. */
    case CTC_FIELD_KEY_CVLAN_RANGE:              /**< [D2] Cvlan Range;
        data: min, mask: max, ext_data: (uint32*)vrange_gid. */
        if (copp_key && !copp_key_use_ctag)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key copp_key_use_ctag:%d \n", copp_key_use_ctag);
            return CTC_E_INVALID_PARAM;
        }
        if (fwd_ext_key && !fwd_ext_key_use_l2_hdr)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key use_l2_hdr:%d \n", fwd_ext_key_use_l2_hdr);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_key && (pe->mac_key_vlan_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key vlan_mode:%d \n", pe->mac_key_vlan_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (short_key && short_key_use_svlan)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "interface key use_svlan:%d \n", short_key_use_svlan);
            return CTC_E_INVALID_PARAM;
        }
        if(fwd_key && !GetFlowTcamLookupCtl(V,aclForwardingBasicKeyUseCvlan_f,&flow_ctl))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "forward key in use svlan mode\n");
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && pe->macl3basic_key_cvlan_mode == 2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_basic_key cvlan_mode:%d \n", pe->macl3basic_key_cvlan_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_CVLAN_ID)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_CTAG_COS)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 7);
        }
        if (key_field->type == CTC_FIELD_KEY_CTAG_CFI)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        if ((fwd_key || fwd_ext_key || l3_key || l3_ext_key || mac_l3_key || mac_l3_ext_key) &&\
            (CTC_PARSER_L3_TYPE_NONE != l3_type || 0x0806 == key_field->data || 0x8035 == key_field->data))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The Key's l3 type must be none l3_type:%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        if (l2type_use_vlanNum )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The Key l2type_use_vlanNum:%d \n", l2type_use_vlanNum);
            return CTC_E_INVALID_PARAM;

        }
        CTC_MAX_VALUE_CHECK(key_field->data, 3);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        if (!l2type_use_vlanNum )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The Key l2type_use_vlanNum:%d \n", l2type_use_vlanNum);
            return CTC_E_INVALID_PARAM;

        }
        CTC_MAX_VALUE_CHECK(key_field->data, 3);
        break;
    case CTC_FIELD_KEY_L3_TYPE:                  /**< [D2] Layer 3 type (ctc_parser_l3_type_t). */
        if (copp_key && !copp_key_use_l3l4_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key  use_l3l4_type:%d \n", copp_key_use_l3l4_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_L4_TYPE:                  /**< [D2] Layer 4 type (ctc_parser_l4_type_t). */
        if (copp_key && !copp_key_use_l3l4_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key  use_l3l4_type:%d \n", copp_key_use_l3l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (fwd_ext_key && (l3_type != CTC_PARSER_L3_TYPE_IPV4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key l3_type must be ipv4, Current L3 Type:%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_PARSER_L3_TYPE_NONE == l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Current L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:             /**< [D2] Layer 4 Usertype (ctc_parser_l4_usertype_t). */
        if (CTC_PARSER_L4_TYPE_NONE == l4_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Current L4 Type :%d \n", l4_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_IP_SA:                    /**< [D2] Source IPv4 Address. */
		if (l3_type != CTC_PARSER_L3_TYPE_IPV4)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
		if (copp_basic_key && (!copp_basic_key_ipaddr_use_ipsa) )
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IP_DA:                    /**< [D2] Destination IPv4 Address. */
        if (l3_type != CTC_PARSER_L3_TYPE_IPV4)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
		if (copp_basic_key && copp_basic_key_ipaddr_use_ipsa)
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IPV6_SA:                  /**< [D2] Source IPv6 Address. */
        if (l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
		if(copp_basic_key)
		{
			SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp basic key no support ipv6 address\n");
            return CTC_E_INVALID_PARAM;
		}
        if (cid_key && cid_key_v6_use_ipda)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "cid_key , use_ipda :%d \n", cid_key_v6_use_ipda);
            return CTC_E_INVALID_PARAM;
        }
        if (copp_ext_key && (!copp_ext_key_ipv6sa && !copp_ext_key_udf_use_ipv6 &&(pe->copp_ext_key_ip_mode ==2)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp ext key , ipsa and udf share same field,mode:%d\n", pe->copp_ext_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IPV6_DA:                  /**< [D2] Destination IPv6 Address. */
        if (l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (cid_key && !cid_key_v6_use_ipda)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "cid_key , use_ipda :%d \n", cid_key_v6_use_ipda);
            return CTC_E_INVALID_PARAM;
        }
		if(copp_basic_key)
		{
			SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp basic key no support ipv6 address\n");
            return CTC_E_INVALID_PARAM;
		}
        if (copp_ext_key && (copp_ext_key_ipv6sa && !copp_ext_key_udf_use_ipv6 &&(pe->copp_ext_key_ip_mode ==2)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp ext key , ipda and udf share same field,mode:%d\n", pe->copp_ext_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:          /**< [D2] Ipv6 Flow label*/
        if (l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFF);
        break;
    case CTC_FIELD_KEY_IP_TTL:

        if (l3_type != CTC_PARSER_L3_TYPE_IPV4
            && l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6 or ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l3_key && (l3_key_share_field_mode != 1) )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key share_field_mode :%d \n", l3_key_share_field_mode);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        break;
    case CTC_FIELD_KEY_IP_DSCP:                  /**< [D2] DSCP. */
        if (l3_key && ((l3_key_share_field_mode == 0 &&\
            GetFlowTcamLookupCtl(V,l3Key160FullRangeBitMapMode_f, &flow_ctl)) || l3_key_share_field_mode == 3  ) )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key share_field_mode :%d \n", l3_key_share_field_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (l3_type != CTC_PARSER_L3_TYPE_IPV4
            && l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6 or ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0x3F);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if (l3_key && (l3_key_share_field_mode != 0   ) )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key share_field_mode :%d \n", l3_key_share_field_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (l3_type != CTC_PARSER_L3_TYPE_IPV4
            && l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6 or ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:              /**< [D2] Ip Protocol. */
    case CTC_FIELD_KEY_IP_PRECEDENCE:            /**< [D2] Precedence. */
    case CTC_FIELD_KEY_IP_ECN:                   /**< [D2] Ecn. */
    case CTC_FIELD_KEY_IP_FRAG:                  /**< [D2] IP Fragment Information. */
    case CTC_FIELD_KEY_IP_HDR_ERROR:             /**< [D2] Ip Header Error. */
    case CTC_FIELD_KEY_IP_OPTIONS:               /**< [D2] Ip Options. */
        if (CTC_FIELD_KEY_IP_FRAG == key_field->type && \
            !DRV_IS_DUET2(lchip) && \
            (copp_basic_key || udf_key) && \
            !(CTC_IP_FRAG_NON == key_field->data || CTC_IP_FRAG_YES == key_field->data))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip frag only support none and yes\n");
            return CTC_E_NOT_SUPPORT;
        }
        if (l3_type != CTC_PARSER_L3_TYPE_IPV4
            && l3_type != CTC_PARSER_L3_TYPE_IPV6)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6 or ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_IP_PROTOCOL)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_IP_PRECEDENCE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x7);
        }
        if (key_field->type == CTC_FIELD_KEY_IP_ECN)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 3);
        }
        if (key_field->type == CTC_FIELD_KEY_IP_FRAG)
        {
            /*key judge*/
        }
        if (key_field->type == CTC_FIELD_KEY_IP_HDR_ERROR
            || key_field->type == CTC_FIELD_KEY_IP_OPTIONS)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1)
        }
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
    case CTC_FIELD_KEY_L4_SRC_PORT:
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:        /**< [D2] Layer 4 Src Port Range;
        data: min, mask: max. */
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:        /**< [D2] Layer 4 Dest Port Range;
        data: min, mask: max. */
        if (CTC_PARSER_L3_TYPE_IPV4 != l3_type
            && CTC_PARSER_L3_TYPE_IPV6 != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv6 or ipv4, L3 Type :%d \n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l4_type == CTC_PARSER_L4_TYPE_NONE
            || CTC_PARSER_L4_TYPE_ICMP == l4_type
        || CTC_PARSER_L4_TYPE_IGMP == l4_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port's L4 Type is conflict, L4 Type :%d \n", l4_type);
            return CTC_E_PARAM_CONFLICT;
        }
        if (pe->key_l4_port_mode != 1 && pe->key_l4_port_mode != 0 )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port's share is conflit, l4_port_mode:%d \n", pe->key_l4_port_mode);
            return CTC_E_PARAM_CONFLICT;
        }

        if (key_field->type == CTC_FIELD_KEY_L4_DST_PORT
            || key_field->type == CTC_FIELD_KEY_L4_SRC_PORT)
        {
            if(CTC_PARSER_L4_USER_TYPE_UDP_VXLAN == l4_user_type || CTC_PARSER_L4_TYPE_GRE == l4_type)
            {
                return CTC_E_INVALID_PARAM;
            }
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_L4_SRC_PORT_RANGE
            || key_field->type == CTC_FIELD_KEY_L4_DST_PORT_RANGE)
        {
            /*func judge*/
        }

        break;
    case CTC_FIELD_KEY_TCP_ECN:                  /**< [D2] TCP Ecn. */
    case CTC_FIELD_KEY_TCP_FLAGS:                /**< [D2] TCP Flags (ctc_acl_tcp_flag_flag_t). */
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4  && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_TCP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l3_key && (l3_key_share_field_mode != 2   ) )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key share_field_mode:%d \n", l3_key_share_field_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_TCP_ECN)
        {
            if (l3_key || cid_key)
            {
                CTC_MAX_VALUE_CHECK(key_field->data, 0x3);/* 2bit */
            }
            else
            {
                CTC_MAX_VALUE_CHECK(key_field->data, 0x7);/* 3bit */
            }
        }
        if (key_field->type == CTC_FIELD_KEY_TCP_FLAGS)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x3F);
        }
        if (pe->key_l4_port_mode != 1 && pe->key_l4_port_mode != 0 )
        {
            return CTC_E_PARAM_CONFLICT;
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
    case CTC_FIELD_KEY_ICMP_TYPE:
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_ICMP_CODE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_ICMP_TYPE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (pe->key_l4_port_mode != 1 && pe->key_l4_port_mode != 0 )
        {
            return CTC_E_PARAM_CONFLICT;
        }
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_IGMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (pe->key_l4_port_mode != 1 && pe->key_l4_port_mode != 0 )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "share filed key_l4_port_mode:%d \n", pe->key_l4_port_mode);
            return CTC_E_PARAM_CONFLICT;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        break;

    case CTC_FIELD_KEY_VXLAN_FLAGS:
    case CTC_FIELD_KEY_VXLAN_RSV1:
    case CTC_FIELD_KEY_VXLAN_RSV2:
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_UDP)
            || (l4_user_type != CTC_PARSER_L4_USER_TYPE_UDP_VXLAN))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if ((CTC_FIELD_KEY_VXLAN_FLAGS != key_field->type) && (fwd_key || fwd_ext_key) && (l3_type != CTC_PARSER_L3_TYPE_IPV4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd and fwd ext key support the field only for ipv4 packet \n");
            return CTC_E_NOT_SUPPORT;
        }
        if ((CTC_FIELD_KEY_VXLAN_FLAGS != key_field->type) && (ipv6_ext_key || CTC_ACL_KEY_MAC_IPV6 == pe->key_type) && (pe->key_mergedata_mode == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Merge Key check conflict,key_mergedata_mode:%d \n", pe->key_mergedata_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_FIELD_KEY_VXLAN_RSV1 == key_field->type)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        }
        else
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        break;

    case CTC_FIELD_KEY_VN_ID:
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_UDP)
        || (l4_user_type != CTC_PARSER_L4_USER_TYPE_UDP_VXLAN))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (copp_ext_key)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp ext key not support the field\n");
            return CTC_E_INVALID_PARAM;
        }
        if (pe->key_l4_port_mode != 2 && pe->key_l4_port_mode != 0 )
        {
            return CTC_E_PARAM_CONFLICT;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        break;
    case CTC_FIELD_KEY_GRE_KEY:                     /**< [D2] Gre Key. */
        if (copp_ext_key)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp ext key not support the field\n");
            return CTC_E_INVALID_PARAM;
        }
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }

        if (pe->key_l4_port_mode != 3 && pe->key_l4_port_mode != 0 )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "share filed key_l4_port_mode:%d \n", pe->key_l4_port_mode);
            return CTC_E_PARAM_CONFLICT;
        }
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:                  /**< [D2] NVGre Key. */
        if (copp_ext_key)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp ext key not support the field\n");
            return CTC_E_INVALID_PARAM;
        }
        if ((l3_type != CTC_PARSER_L3_TYPE_IPV4 && l3_type != CTC_PARSER_L3_TYPE_IPV6)
            || (l4_type != CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "param is mismatch , l3_type :%d l4_type:%d \n", l3_type, l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if (pe->key_l4_port_mode != 3 && pe->key_l4_port_mode != 0 )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "share filed key_l4_port_mode:%d \n", pe->key_l4_port_mode);
            return CTC_E_PARAM_CONFLICT;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        break;
    case CTC_FIELD_KEY_ARP_OP_CODE:
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
    case CTC_FIELD_KEY_ARP_SENDER_IP:
    case CTC_FIELD_KEY_ARP_TARGET_IP:
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
    case  CTC_FIELD_KEY_SENDER_MAC:                   /**< [D2] Ethernet Address of sender of ARP Header */
    case  CTC_FIELD_KEY_TARGET_MAC:                   /**< [D2] Ethernet Address of destination of ARP Header */
    case CTC_FIELD_KEY_GARP:
    case CTC_FIELD_KEY_RARP:
        if (CTC_FIELD_KEY_RARP == key_field->type && DRV_IS_DUET2(lchip))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "RARP is not support\n");
            return CTC_E_NOT_SUPPORT;
        }
        if (CTC_PARSER_L3_TYPE_ARP != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not ARP l3 type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_ext_key
            && (key_field->type == CTC_FIELD_KEY_SENDER_MAC)
        && !mac_l3_ext_key_mac_sa_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_ext_key mac_sa_mode:%d \n", mac_l3_ext_key_mac_sa_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_ext_key
            && (key_field->type == CTC_FIELD_KEY_TARGET_MAC )
        && !mac_l3_ext_key_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_ext_key mac_da_mode:%d \n", mac_l3_ext_key_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && (key_field->type == CTC_FIELD_KEY_SENDER_MAC)
        && !mac_l3_key_mac_sa_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_key mac_sa_mode:%d \n", mac_l3_key_mac_sa_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key
            && (key_field->type == CTC_FIELD_KEY_TARGET_MAC)
        && !mac_l3_key_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_key mac_da_mode:%d \n", mac_l3_key_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (((mac_l3_key && DRV_IS_DUET2(lchip)) || copp_basic_key)
            && key_field->type == CTC_FIELD_KEY_TARGET_MAC )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "key not support the field\n");
            return CTC_E_INVALID_PARAM;
        }
        if (copp_basic_key
            && key_field->type == CTC_FIELD_KEY_SENDER_MAC
        && !copp_key_basic_mac_da_mode)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_key mac_da_mode:%d \n", copp_key_basic_mac_da_mode);
            return CTC_E_INVALID_PARAM;
        }


        if (key_field->type == CTC_FIELD_KEY_ARP_OP_CODE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type== CTC_FIELD_KEY_ARP_PROTOCOL_TYPE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL
            || key_field->type == CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL
        || key_field->type == CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL
        || key_field->type == CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL
        || key_field->type == CTC_FIELD_KEY_GARP)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_LABEL_NUM:                /**< [D2] MPLS Label Number. */
    case  CTC_FIELD_KEY_MPLS_LABEL0:              /**< [D2] Label Field of the MPLS Label 0. */
    case  CTC_FIELD_KEY_MPLS_EXP0:                /**< [D2] Exp Field of the MPLS Label 0.*/
    case  CTC_FIELD_KEY_MPLS_SBIT0:               /**< [D2] S-bit Field of the MPLS Label 0.*/
    case  CTC_FIELD_KEY_MPLS_TTL0:                /**< [D2] Ttl Field of the MPLS Label 0.*/
    case  CTC_FIELD_KEY_MPLS_LABEL1:              /**< [D2] Label Field of the MPLS Label 1. */
    case  CTC_FIELD_KEY_MPLS_EXP1:                /**< [D2] Exp Field of the MPLS Label 1.*/
    case  CTC_FIELD_KEY_MPLS_SBIT1:               /**< [D2] S-bit Field of the MPLS Label 1.*/
    case  CTC_FIELD_KEY_MPLS_TTL1:                /**< [D2] Ttl Field of the MPLS Label 1.*/
    case  CTC_FIELD_KEY_MPLS_LABEL2:              /**< [D2] Label Field of the MPLS Label 2. */
    case CTC_FIELD_KEY_MPLS_EXP2:                /**< [D2] Exp Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_SBIT2:               /**< [D2] S-bit Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_TTL2:                /**< [D2] Ttl Field of the MPLS Label 2.*/
        if (CTC_PARSER_L3_TYPE_MPLS != l3_type && CTC_PARSER_L3_TYPE_MPLS_MCAST!=l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not MPLS l3 type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_LABEL_NUM)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, MCHIP_CAP(SYS_CAP_ACL_LABEL_NUM));
        }
        if (key_field->type == CTC_FIELD_KEY_MPLS_LABEL0
            || key_field->type == CTC_FIELD_KEY_MPLS_LABEL1
        || key_field->type == CTC_FIELD_KEY_MPLS_LABEL2)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_MPLS_EXP0
            || key_field->type == CTC_FIELD_KEY_MPLS_EXP1
        || key_field->type == CTC_FIELD_KEY_MPLS_EXP2)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 7);
        }
        if (key_field->type == CTC_FIELD_KEY_MPLS_TTL0
            || key_field->type == CTC_FIELD_KEY_MPLS_TTL1
        || key_field->type == CTC_FIELD_KEY_MPLS_TTL2)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_MPLS_SBIT0
            || key_field->type == CTC_FIELD_KEY_MPLS_SBIT1
        || key_field->type == CTC_FIELD_KEY_MPLS_SBIT2)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        if (l3_key && (CTC_PARSER_L3_TYPE_MPLS != l3_type) && (CTC_PARSER_L3_TYPE_MPLS_MCAST!=l3_type))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "when l3_key use interface id ,the pkt must be mpls :%d\n", pe->l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if ((l3_ext_key || mac_l3_key)&& (CTC_PARSER_L3_TYPE_MPLS != l3_type) && (CTC_PARSER_L3_TYPE_MPLS_MCAST!=l3_type) && !CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_IS_Y1731_OAM))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "when l3_key or mac_l3_key use interface id ,the pkt must be mpls or ether_oam:%d\n", pe->l3_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1));/*10bit; 1023 is reserved*/
        break;
    case CTC_FIELD_KEY_NSH_CBIT:                 /**< [D2] C-bit of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_OBIT:                 /**< [D2] O-bit of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:        /**< [D2] Next Protocol of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_SI:                  /**< [D2] Service Index of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_SPI:                  /**< [D2] Service Path ID of the Network Service Header. */
        if (!DRV_IS_DUET2(lchip))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "nsh not support\n");
            return CTC_E_NOT_SUPPORT;
        }
        /*forwarding ext key:nsh and udf is union*/
        if (fwd_ext_key && fwd_ext_key_use_l2_hdr)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key use_l2_hdr :%d\n", fwd_ext_key_use_l2_hdr);
            return CTC_E_INVALID_PARAM;
        }
        if (fwd_ext_key && !fwd_ext_key_udf_use_nsh)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key udf_use_nsh :%d\n", fwd_ext_key_udf_use_nsh);
            return CTC_E_INVALID_PARAM;
        }
        if (fwd_ext_key && fwd_ext_key_udf_use_nsh
            && (pe->fwd_key_nsh_mode == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key udf_use_nsh :%d,fwd_key_nsh_mode:%d\n", fwd_ext_key_udf_use_nsh, pe->fwd_key_nsh_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_NSH_CBIT
            || key_field->type == CTC_FIELD_KEY_NSH_OBIT)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x1);
        }
        if (key_field->type == CTC_FIELD_KEY_NSH_NEXT_PROTOCOL)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_NSH_SI)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_NSH_SPI)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        }
        break;
        /*Ether OAM packet field*/
    case CTC_FIELD_KEY_IS_Y1731_OAM:             /**< [D2] Is Y1731 Oam. */
        if(CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHY1731 != l4_user_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Y1731 l4 user type must be ach oam\n");
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0x1);
        break;
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:          /**< [D2] Oam Level. */
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:        /**< [D2] Oam Opcode. */
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:        /**< [D2] Oam Version. */
        if (!(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type
            || (CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_IS_Y1731_OAM))))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch: l3_type:%d l4_type:%d,l4_user_type:%d\n", l3_type, l4_type, l4_user_type);
            return CTC_E_INVALID_PARAM;
        }

        if (key_field->type == CTC_FIELD_KEY_ETHER_OAM_LEVEL)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 7);
        }
        if (key_field->type == CTC_FIELD_KEY_ETHER_OAM_OP_CODE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_ETHER_OAM_VERSION)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x1F);
        }
        break;
        /*Slow Protocol packet field*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:       /**< [D2] Slow Protocol Code. */
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:      /**< [D2] Slow Protocol Flags. */
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:   /**< [D2] Slow Protocol Sub Type. */
        if (CTC_PARSER_L3_TYPE_SLOW_PROTO != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not slow protocol l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_SLOW_PROTOCOL_CODE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        if (key_field->type == CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        }
        break;
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:       /**< [D2] Slow Protocol Code. */
    case CTC_FIELD_KEY_PTP_VERSION:      /**< [D2] Slow Protocol Flags. */
        if (CTC_PARSER_L3_TYPE_PTP != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not PTP  l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_PTP_MESSAGE_TYPE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        }
        if (key_field->type == CTC_FIELD_KEY_PTP_VERSION)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 3);
        }
        break;
    case CTC_FIELD_KEY_FCOE_DST_FCID:       /**< [D2] Slow Protocol Code. */
    case CTC_FIELD_KEY_FCOE_SRC_FCID:      /**< [D2] Slow Protocol Flags. */
        if (CTC_PARSER_L3_TYPE_FCOE != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not FCOE l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:         /**< [D2] Ingress Nick Name. */
    case CTC_FIELD_KEY_EGRESS_NICKNAME:          /**< [D2] Egress Nick Name. */
    case CTC_FIELD_KEY_IS_ESADI:                 /**< [D2] Is Esadi. */
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:         /**< [D2] Is Trill Channel. */
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:       /**< [D2] Trill Inner Vlan Id. */
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID: /**< [D2] Trill Inner Vlan Id Valid. */

    case CTC_FIELD_KEY_TRILL_LENGTH:             /**< [D2] Trill Length. */
    case CTC_FIELD_KEY_TRILL_MULTIHOP:           /**< [D2] Trill Multi-Hop. */
    case CTC_FIELD_KEY_TRILL_MULTICAST:          /**< [D2] Trill MultiCast. */
    case CTC_FIELD_KEY_TRILL_VERSION:            /**< [D2] Trill Version. */
    case CTC_FIELD_KEY_TRILL_TTL:                /**< [D2] Trill Ttl. */
        if (CTC_PARSER_L3_TYPE_TRILL != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not TRILL l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_IS_ESADI
            || key_field->type == CTC_FIELD_KEY_IS_TRILL_CHANNEL
        || key_field->type == CTC_FIELD_KEY_TRILL_MULTIHOP
        || key_field->type == CTC_FIELD_KEY_TRILL_MULTICAST
        || key_field->type == CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        if (key_field->type == CTC_FIELD_KEY_INGRESS_NICKNAME
            || key_field->type == CTC_FIELD_KEY_EGRESS_NICKNAME )
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_TRILL_INNER_VLANID)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_TRILL_LENGTH)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x3FFF);
        }
        if (key_field->type == CTC_FIELD_KEY_TRILL_TTL)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x3F);
        }
        if (key_field->type== CTC_FIELD_KEY_TRILL_VERSION)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 3);
        }
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:        /**< [D2] Unknow 802.1AE packet, SecTag error or Mode not support. */
    case CTC_FIELD_KEY_DOT1AE_ES:                /**< [D2] End Station (ES) bit. */
    case CTC_FIELD_KEY_DOT1AE_SC:                /**< [D2] Secure Channel (SC) bit. */
    case CTC_FIELD_KEY_DOT1AE_AN:                /**< [D2] Association Number (AN). */
    case CTC_FIELD_KEY_DOT1AE_SL:                /**< [D2] Short Length (SL). */
    case CTC_FIELD_KEY_DOT1AE_PN:                /**< [D2] Packet Number. */
    case CTC_FIELD_KEY_DOT1AE_SCI:               /**< [D2] Secure Channel Identifier. */
    case CTC_FIELD_KEY_DOT1AE_CBIT :            /**< [TM] Changed Text. */
    case CTC_FIELD_KEY_DOT1AE_EBIT :            /**< [TM] Encryption. */
    case CTC_FIELD_KEY_DOT1AE_SCB :             /**< [TM] Single Copy Broadcast. */
    case CTC_FIELD_KEY_DOT1AE_VER :             /**< [TM] Version number. */
        if (CTC_PARSER_L3_TYPE_DOT1AE != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not DOT1AE l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_DOT1AE_ES \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_SC \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_CBIT \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_EBIT \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_SCB \
        || key_field->type == CTC_FIELD_KEY_DOT1AE_VER)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        if (key_field->type == CTC_FIELD_KEY_DOT1AE_AN)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 3);
        }
        if (key_field->type == CTC_FIELD_KEY_DOT1AE_SL)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x3F);
        }
        if (key_field->type == CTC_FIELD_KEY_DOT1AE_SCI)
        {
            /*64 bit*/
        }
        break;
        /*SATPDU packet field*/
    case  CTC_FIELD_KEY_SATPDU_MEF_OUI:           /**< [D2] Satpdu Metro Ethernet Forum Oui.*/
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:      /**< [D2] Satpdu Oui Sub type.*/
    case  CTC_FIELD_KEY_SATPDU_PDU_BYTE:          /**< [D2] Satpdu Byte.*/
        if (CTC_PARSER_L3_TYPE_SATPDU != l3_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not SATPDU l3_type:%d\n", l3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_SATPDU_MEF_OUI)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
        if (key_field->type == CTC_FIELD_KEY_SATPDU_PDU_BYTE)
        {
            /*64 bit*/
        }
        break;
        /*Wlan packet field*/
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:         /**< [D2] Wlan Radio Mac.*/
    case CTC_FIELD_KEY_WLAN_RADIO_ID:          /**< [D2] Wlan Radio Id.*/
    case  CTC_FIELD_KEY_WLAN_CTL_PKT:           /**< [D2] Wlan Control Packet.*/
        if (CTC_PARSER_L4_TYPE_UDP != l4_type
            && CTC_PARSER_L4_USER_TYPE_UDP_CAPWAP != l4_user_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "WLAN param is mismatch l3_type:%d l4_user_type:%d\n", l4_type, l4_user_type);
            return CTC_E_INVALID_PARAM;
        }
        if (pe->key_mergedata_mode == 2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Merge Key check conflict,key_mergedata_mode:%d \n", pe->key_mergedata_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (key_field->type == CTC_FIELD_KEY_WLAN_RADIO_ID)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0x1F);
        }
        if (key_field->type == CTC_FIELD_KEY_WLAN_CTL_PKT)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_UDF:
        if (!fwd_ext_key && !udf_key && !ipv6_ext_key && !mac_l3_ext_key)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "UDF must use fwd_ext_key ipv6_ext_key or udf_key or mac l3 ext key\n");
            return CTC_E_INVALID_PARAM;
        }

        if (fwd_ext_key && fwd_ext_key_use_l2_hdr)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key use_l2_hdr:%d\n", fwd_ext_key_use_l2_hdr);
            return CTC_E_INVALID_PARAM;
        }

        if (fwd_ext_key && fwd_ext_key_udf_use_nsh
            && (pe->fwd_key_nsh_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fwd_ext_key udf_use_nsh:%d,fwd_key_nsh_mode:%d\n", fwd_ext_key_udf_use_nsh, pe->fwd_key_nsh_mode);
            return CTC_E_INVALID_PARAM;
        }
        if(copp_ext_key && (copp_ext_key_udf_use_ipv6 || 1 == pe->copp_ext_key_ip_mode))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "copp_ext_key udf and ip v6 address is share & mode is %u\n", pe->copp_ext_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        if(fwd_ext_key && (l3_type != CTC_PARSER_L3_TYPE_IPV4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Only ipv4 packet support aware tunnel info in fwd ext key\n");
            return CTC_E_INVALID_PARAM;
        }
        if (pe->key_mergedata_mode == 1)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Merge Key check conflict,key_mergedata_mode:%d \n", pe->key_mergedata_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_PARSER_L4_TYPE_UDP == l4_type
            && CTC_PARSER_L4_USER_TYPE_UDP_VXLAN == l4_user_type)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed must be decapsulated packet, l4_type:%d,l4_user_type:%d\n", l4_type, l4_user_type);
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_PARSER_L4_TYPE_UDP == l4_type
            && CTC_PARSER_L4_USER_TYPE_UDP_CAPWAP == l4_user_type )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed must be decapsulated packet, l4_type:%d,l4_user_type:%d\n", l4_type, l4_user_type);
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_PARSER_L4_TYPE_GRE == l4_type )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed must be decapsulated packet, l4_type:%d,l4_user_type:%d\n", l4_type, l4_user_type);
            return CTC_E_INVALID_PARAM;
        }
        if (!arware_tunnel_info)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed must be decapsulated packet, arware_tunnel_info:%d\n", arware_tunnel_info);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key && (pe->macl3basic_key_cvlan_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_basic_key cvlan_mode:%d \n", pe->macl3basic_key_cvlan_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (mac_l3_key && (pe->macl3basic_key_macda_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac_l3_basic_key mac_da_mode:%d \n", pe->macl3basic_key_macda_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        if (l3_key && (l3_key_share_field_mode != 3  ) )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3_key share_field_mode:%d\n", l3_key_share_field_mode);
            return CTC_E_INVALID_PARAM;
        }
        SYS_IP_VRFID_CHECK(key_field->data,MAX_CTC_IP_VER);
        break;
    /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11*/
    case CTC_FIELD_KEY_DISCARD:                  /**< [D2] Packet Is Flagged to be Discarded. */
        break;

    case CTC_FIELD_KEY_DECAP:                    /**< [D2] Decapsulation occurred. */
    case CTC_FIELD_KEY_ELEPHANT_PKT:             /**< [D2] Is Elephant. */
    case CTC_FIELD_KEY_VXLAN_PKT:                /**< [D2] Is Vxlan Packet. */
    case CTC_FIELD_KEY_ROUTED_PKT:               /**< [D2] Is Routed packet. */
    case CTC_FIELD_KEY_MACSA_HIT:                /**< [D2] Mac-Sa Lookup Hit. */
    case CTC_FIELD_KEY_MACDA_HIT:                /**< [D2] Mac-Da Lookup Hit. */
    case CTC_FIELD_KEY_IPSA_HIT:                 /**< [D2] L3-Sa Lookup Hit. */
    case CTC_FIELD_KEY_IPSA_LKUP:                /**< [D2] L3-Sa Lookup Enable. */
    case CTC_FIELD_KEY_IPDA_LKUP:                /**< [D2] L3-Da Lookup Enable. */
    case CTC_FIELD_KEY_IPDA_HIT:                 /**< [D2] L3-Da Lookup Hit. */
    case CTC_FIELD_KEY_MACSA_LKUP:               /**< [D2] Mac-Sa Lookup Enable. */
    case CTC_FIELD_KEY_MACDA_LKUP:               /**< [D2] Mac-Da Lookup Enable. */
    case CTC_FIELD_KEY_L2_STATION_MOVE:          /**< [D2] L2 Station Move. */
    case CTC_FIELD_KEY_MAC_SECURITY_DISCARD:     /**< [D2] Mac Security Discard. */
     /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11*/
    //case CTC_FIELD_KEY_DISCARD:                  /**< [D2] Packet Is Flagged to be Discarded. */
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:     /**< [D2] Mcast Rpf Check Fail. */
        if (CTC_FIELD_KEY_DECAP == key_field->type && !DRV_IS_DUET2(lchip) && CTC_INGRESS != pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Egress not support decap \n");
            return CTC_E_NOT_SUPPORT;
        }
        CTC_MAX_VALUE_CHECK(key_field->data, 1);
        break;
    case CTC_FIELD_KEY_PKT_FWD_TYPE:             /**< [D2] Packet Forwarding Type. Refer to ctc_acl_pkt_fwd_type_t */
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:            /**< [D2] CPU Reason Id. */
        CTC_MAX_VALUE_CHECK(key_field->data, CTC_PKT_CPU_REASON_MAX_COUNT);
        break;
    case CTC_FIELD_KEY_DST_GPORT:                /**< [D2] Fwd Destination Port. */
        SYS_GLOBAL_PORT_CHECK(key_field->data);
        break;
    case CTC_FIELD_KEY_DST_NHID:                 /**< [D2] Nexthop Id. */
        /*uint32*/
        break;
    case CTC_FIELD_KEY_STP_STATE:
        CTC_MAX_VALUE_CHECK(key_field->data, 3);
        key_field->mask = 0x3;
        if(2 == key_field->data)
        {
            key_field->data = 3;
        }
        else if(3 == key_field->data)
        {
            key_field->data = 1;
            key_field->mask = 0x1;
        }
        break;
    case CTC_FIELD_KEY_VLAN_XLATE_HIT:
        if (CTC_EGRESS != pe->group->group_info.dir || DRV_IS_DUET2(lchip))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ingress not support vlan xlate \n");
            return CTC_E_NOT_SUPPORT;
        }
        break;

    default:
        break;
    }

    /*check ext_data valid*/
    switch(key_field->type)
    {
    case CTC_FIELD_KEY_MAC_SA:                  /* 48bit*/
    case CTC_FIELD_KEY_MAC_DA:
    case CTC_FIELD_KEY_SENDER_MAC:
    case CTC_FIELD_KEY_TARGET_MAC:
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
        }
    break;
    case CTC_FIELD_KEY_SVLAN_RANGE:  /**< [D2] Svlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
    case CTC_FIELD_KEY_CVLAN_RANGE:  /**< [D2] Cvlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_MAX_VALUE_CHECK(key_field->data, key_field->mask);
            SYS_ACL_VLAN_ID_CHECK(key_field->mask);
        }
    break;
    case CTC_FIELD_KEY_IPV6_SA:                  /**< [D2] Source IPv6 Address. 128bit */
    case CTC_FIELD_KEY_IPV6_DA:                  /**< [D2] Destination IPv6 Address. 128bit */
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
        }
    break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:    /*64 bit*/
    case CTC_FIELD_KEY_DOT1AE_SCI:         /*64 bit*/
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
        }
    break;
    case CTC_FIELD_KEY_UDF:              /*need fix*/
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
        }
    break;
    case CTC_FIELD_KEY_PORT:
        {
            if(is_add)
            {
                ctc_field_port_t* p_field_port = (ctc_field_port_t*)key_field->ext_data;
                CTC_PTR_VALID_CHECK(key_field->ext_data);
                if(copp_key && \
                    (CTC_FIELD_PORT_TYPE_NONE != p_field_port->type) && (CTC_FIELD_PORT_TYPE_PORT_BITMAP !=p_field_port->type))
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The key port type must be none or bitmap, key_port_type:%d\n", p_field_port->type);
                    return CTC_E_NOT_SUPPORT;
                }
                if((cid_key) && \
                    (CTC_FIELD_PORT_TYPE_GPORT==p_field_port->type || CTC_FIELD_PORT_TYPE_LOGIC_PORT==p_field_port->type || CTC_FIELD_PORT_TYPE_PORT_BITMAP==p_field_port->type ))
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The key port type must not be gport ,logical port por bitmap, key_port_type:%d\n", p_field_port->type);
                    return CTC_E_NOT_SUPPORT;
                }
                if (udf_key && CTC_FIELD_PORT_TYPE_GPORT == p_field_port->type)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The key port type must not be gport, key_port_type:%d\n", p_field_port->type);
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_ERROR_RETURN(_sys_usw_acl_check_port_info(lchip, pe, key_field));
            }
        }
    break;
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        if(is_add)
        {
            ctc_acl_tunnel_data_t* p_tunnel_data = (ctc_acl_tunnel_data_t*)key_field->ext_data;
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            CTC_MAX_VALUE_CHECK(p_tunnel_data->type, 0x03);
            CTC_MAX_VALUE_CHECK(p_tunnel_data->vxlan_vni, 0xFFFFFF);
            CTC_MAX_VALUE_CHECK(p_tunnel_data->radio_id, 0x1F);
            CTC_MAX_VALUE_CHECK(p_tunnel_data->wlan_ctl_pkt, 0x01);
        }
    break;
    case CTC_FIELD_KEY_DISCARD:
        if(is_add && (key_field->ext_data) && (fwd_key || fwd_ext_key || copp_key))
        {
            discard_type = *((uint32*)(key_field->ext_data));
            CTC_ERROR_RETURN(_sys_usw_acl_check_discard_type(discard_type));
        }
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        if(is_add)
        {
            SYS_ACL_PORT_CLASS_ID_CHECK(key_field->data);
        }
        break;
    }
    return CTC_E_NONE;

}

 /*delete: all fields(many to one) can't reset to 0;if reset to 0,other fields have problems*/
int32
_sys_usw_acl_set_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint8 mac_key = 0;
    uint8 mac_l3_basic_key = 0;
    uint8 fwd_ext_key = 0;
    uint8 copp_ext_key = 0;
    uint8 copp_ext_key_udf_use_ipv6 = 0;
    uint8 copp_ext_key_ipv6sa = 0;
    uint8 udf_key = 0;
    uint32 cmd = 0;

    FlowTcamLookupCtl_m flow_ctl;

    fwd_ext_key =  pe->key_type == CTC_ACL_KEY_FWD_EXT;
    mac_key = pe->key_type == CTC_ACL_KEY_MAC;
    mac_l3_basic_key = pe->key_type == CTC_ACL_KEY_MAC_IPV4;
    udf_key = pe->key_type == CTC_ACL_KEY_UDF;

    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &flow_ctl);

    copp_ext_key_ipv6sa = 1 & GetFlowTcamLookupCtl(V, coppKeyIpAddrShareType0_f, &flow_ctl); /*default to 1*/
    copp_ext_key_udf_use_ipv6  = 1 & GetFlowTcamLookupCtl(V, coppExtKeyUdfShareType_f, &flow_ctl); /*default to 1*/

    copp_ext_key = pe->key_type == CTC_ACL_KEY_COPP_EXT;

    switch(key_field->type)
    {
     case CTC_FIELD_KEY_L2_TYPE:
        pe->l2_type = is_add?key_field->data:0;
        break;
     case  CTC_FIELD_KEY_L3_TYPE:
        pe->l3_type = is_add?key_field->data:0;
        break;
     case  CTC_FIELD_KEY_L4_TYPE:
        pe->l4_type = is_add?key_field->data:0;
        break;
     case CTC_FIELD_KEY_L4_USER_TYPE:
        pe->l4_user_type = is_add?key_field->data:0;
/* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
        if (CTC_PARSER_L4_USER_TYPE_UDP_VXLAN == pe->l4_user_type)
        {
            pe->key_l4_port_mode = 2;
        }
#endif 
        break;
    case CTC_FIELD_KEY_PORT:
        {
            ctc_field_port_t* p_field_port = (ctc_field_port_t*)key_field->ext_data;
            if(p_field_port->type == CTC_FIELD_PORT_TYPE_PORT_BITMAP || p_field_port->type == CTC_FIELD_PORT_TYPE_GPORT || p_field_port->type == CTC_FIELD_PORT_TYPE_LOGIC_PORT)
            {
                pe->key_port_mode = udf_key? pe->key_port_mode: (is_add ? 1 :0);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        pe->key_port_mode = udf_key? pe->key_port_mode: 2;
        break;
    case CTC_FIELD_KEY_SRC_CID:
        pe->key_port_mode =  2;
        break;
    case CTC_FIELD_KEY_DST_CID:
        pe->key_port_mode = 2;
        break;
    case CTC_FIELD_KEY_STAG_VALID:               /**< [D2] S-Vlan Exist. */
    case CTC_FIELD_KEY_SVLAN_ID:                 /**< [D2] S-Vlan ID. */
    case CTC_FIELD_KEY_STAG_COS:                 /**< [D2] Stag Cos. */
    case CTC_FIELD_KEY_STAG_CFI:                 /**< [D2] Stag Cfi. */
    case CTC_FIELD_KEY_SVLAN_RANGE:              /**< [D2] Svlan Range;
        data: min, mask: max, ext_data: (uint32*)vrange_gid. */
        if (mac_key)
        {
            pe->mac_key_vlan_mode =  1;  /*delete can't reset to 0*/
        }
        break;
    case CTC_FIELD_KEY_CTAG_VALID:               /**< [D2] C-Vlan Exist. */
    case CTC_FIELD_KEY_CVLAN_ID:                 /**< [D2] C-Vlan ID. */
    case CTC_FIELD_KEY_CTAG_COS:                 /**< [D2] Ctag Cos. */
    case CTC_FIELD_KEY_CTAG_CFI:                 /**< [D2] Ctag Cfi. */
    case CTC_FIELD_KEY_CVLAN_RANGE:              /**< [D2] Cvlan Range;
        data: min, mask: max, ext_data: (uint32*)vrange_gid. */
        if (mac_key)
        {
            pe->mac_key_vlan_mode = 2;
        }
        if (mac_l3_basic_key)
        {
            pe->macl3basic_key_cvlan_mode = 1;
        }
        break;
    case CTC_FIELD_KEY_MAC_DA:
        if (mac_l3_basic_key)
        {
            pe->macl3basic_key_macda_mode = is_add ? 1:0;
        }
        break;
    case CTC_FIELD_KEY_NSH_CBIT:                 /**< [D2] C-bit of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_OBIT:                 /**< [D2] O-bit of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:        /**< [D2] Next Protocol of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_SI:                  /**< [D2] Service Index of the Network Service Header. */
    case CTC_FIELD_KEY_NSH_SPI:                 /**< [D2] Service Path ID of the Network Service Header. */
        if (fwd_ext_key)
        {
            pe->fwd_key_nsh_mode = 1;
        }
        break;
    case CTC_FIELD_KEY_IPV6_SA:                  /**< [D2] Source IPv6 Address. */
        if(copp_ext_key && !copp_ext_key_ipv6sa && !copp_ext_key_udf_use_ipv6)
   		{
            pe->copp_ext_key_ip_mode = is_add ?1 :0;
   		}
		break;
    case  CTC_FIELD_KEY_IPV6_DA:                  /**< [D2] Destination IPv6 Address. */
   	    if(copp_ext_key && copp_ext_key_ipv6sa && !copp_ext_key_udf_use_ipv6)
   		{
   		  pe->copp_ext_key_ip_mode = is_add ?1 :0;
   		}
		break;
    case CTC_FIELD_KEY_UDF:
        if (fwd_ext_key)
        {
            pe->fwd_key_nsh_mode = is_add ?2 :0;
        }
		if (copp_ext_key &&(!copp_ext_key_udf_use_ipv6))
        {
            pe->copp_ext_key_ip_mode = is_add ?2 :0;
        }
        break;
    case CTC_FIELD_KEY_GRE_KEY:                     /**< [D2] Gre Key. */
    case CTC_FIELD_KEY_NVGRE_KEY:
        pe->key_l4_port_mode = is_add ?3 :0; /*gre*/
        break;
    case CTC_FIELD_KEY_VN_ID:
        pe->key_l4_port_mode = is_add ?2 :0; /*vxlan*/
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
    case CTC_FIELD_KEY_ICMP_TYPE:
    case CTC_FIELD_KEY_IGMP_TYPE:
    case CTC_FIELD_KEY_L4_DST_PORT:
    case CTC_FIELD_KEY_L4_SRC_PORT:
    case CTC_FIELD_KEY_TCP_ECN:
    case CTC_FIELD_KEY_TCP_FLAGS:
        pe->key_l4_port_mode = 1; /*l4port*/
        break;

    case CTC_FIELD_KEY_WLAN_CTL_PKT:
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
    case CTC_FIELD_KEY_WLAN_RADIO_ID:
        pe->key_mergedata_mode = 1;
        break;
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        pe->key_mergedata_mode = is_add ?2 :0;
        if (mac_l3_basic_key)
        {
            pe->macl3basic_key_macda_mode = is_add ? 2:0;
            pe->macl3basic_key_cvlan_mode = is_add ? 2:0;
        }
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1:
    case CTC_FIELD_KEY_VXLAN_RSV2:
        if (CTC_ACL_KEY_IPV6_EXT == pe->key_type || CTC_ACL_KEY_MAC_IPV6 == pe->key_type)
        {
            pe->key_mergedata_mode = 1;
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        pe->l3_type = is_add?((pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)?\
                                pe->l3_type : CTC_PARSER_L3_TYPE_MPLS):0;
        break;
    default:
        break;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_compress_ipv6_addr(uint8 lchip,sys_acl_entry_t* pe,uint8 is_da, ipv6_addr_t ipv6, ipv6_addr_t ipv6_64)
{
    ctc_global_acl_property_t acl_property;
    uint8 compress_mode = 0;
    uint8 index = 0;

    sal_memset(&acl_property,0,sizeof(ctc_global_acl_property_t));
    acl_property.dir = pe->group->group_info.dir;
    acl_property.lkup_level = pe->group->group_info.block_id;

    sys_usw_get_glb_acl_property(lchip, &acl_property);
    if (pe->key_type == CTC_ACL_KEY_IPV6
        || pe->key_type == CTC_ACL_KEY_FWD)
    {
        compress_mode =  is_da ? acl_property.key_ipv6_da_addr_mode:acl_property.key_ipv6_sa_addr_mode;
    }
    else if(pe->key_type == CTC_ACL_KEY_CID)
    {
        compress_mode =  is_da ? acl_property.cid_key_ipv6_da_addr_mode:acl_property.cid_key_ipv6_sa_addr_mode;
    }
    else
    { /*no compress*/
        sal_memcpy(ipv6_64, ipv6, sizeof(ipv6_addr_t));
        return CTC_E_NONE;
    }

    sal_memset(ipv6_64, 0, sizeof(ipv6_addr_t));
    index = compress_mode;
    compress_mode = compress_mode > 8 ? (compress_mode - 8) : compress_mode;
    ipv6_64[0] = (((uint64)ipv6[index/9]<< (compress_mode*4))& (~((1LLU<<(compress_mode*4))-1))) | (((uint64)ipv6[(index/9)+1] >> (32-(compress_mode*4))) & ((1LLU<<(compress_mode*4))-1)) ;
    ipv6_64[1] = (((uint64)ipv6[(index/9)+1]<< (compress_mode*4))& (~((1LLU<<(compress_mode*4))-1))) | (((uint64)ipv6[(index/9)+2] >> (32-(compress_mode*4))) & ((1LLU<<(compress_mode*4))-1)) ;
    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_ipv6_addr_from_compress(uint8 lchip,sys_acl_entry_t* pe,uint8 is_da, ipv6_addr_t in_ipv6_64, ipv6_addr_t out_ipv6_128)
{
    ctc_global_acl_property_t acl_property;
    uint8 compress_mode = 0;
    uint8 index = 0;

    sal_memset(&acl_property,0,sizeof(ctc_global_acl_property_t));

    acl_property.dir = pe->group->group_info.dir;
    acl_property.lkup_level = pe->group->group_info.block_id;

    sys_usw_get_glb_acl_property(lchip, &acl_property);
    if (pe->key_type == CTC_ACL_KEY_IPV6
        || pe->key_type == CTC_ACL_KEY_FWD)
    {
        compress_mode =  is_da ? acl_property.key_ipv6_da_addr_mode:acl_property.key_ipv6_sa_addr_mode;
    }
    else if(pe->key_type == CTC_ACL_KEY_CID)
    {
        compress_mode =  is_da ? acl_property.cid_key_ipv6_da_addr_mode:acl_property.cid_key_ipv6_sa_addr_mode;
    }
    sal_memset(out_ipv6_128, 0, sizeof(ipv6_addr_t));
    index = compress_mode;
    compress_mode = compress_mode > 8 ? (compress_mode - 8) : compress_mode;
    out_ipv6_128[index/9]     = (((uint64)in_ipv6_64[0] >> (compress_mode*4)) & (0xFFFFFFFFLLU>>(compress_mode*4)));
    out_ipv6_128[(index/9)+1] = (((uint64)in_ipv6_64[0] << (32-(compress_mode*4))) & (~(0xFFFFFFFFLLU>>(compress_mode*4)))) | (((uint64)in_ipv6_64[1] >> (compress_mode*4)) & (0xFFFFFFFFLLU>>(compress_mode*4)));
    out_ipv6_128[(index/9)+2] = ((uint64)in_ipv6_64[1] << (32-(compress_mode*4))) & (~(0xFFFFFFFFLLU>>(compress_mode*4)));
    return CTC_E_NONE;
}

/*
 * install entry to hardware table
 */
int32
_sys_usw_acl_install_entry(uint8 lchip, sys_acl_entry_t* pe, uint8 flag, uint8 move_sw)
{
    sys_acl_group_t* pg = NULL;
    if (!pe ||!pe->group)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        return CTC_E_NOT_EXIST;
    }
    pg = pe->group;
    if (SYS_ACL_ENTRY_OP_FLAG_ADD == flag)
    {
        uint8  is_hash_key = !ACL_ENTRY_IS_TCAM(pe->key_type);
    	if(FPA_ENTRY_FLAG_INSTALLED == pe->fpae.flag )
        {
           return CTC_E_NONE;
        }
        if(is_hash_key)
        {
           /*check hw and roll back*/
           if (!pe->hash_valid)
           {
               SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash valid is %d, Not ready to configuration \n", pe->hash_valid);
               return CTC_E_NOT_READY;
           }

           CTC_ERROR_RETURN(_sys_usw_acl_get_hash_index(lchip, pe, 1,1));
        }

        CTC_ERROR_RETURN(_sys_usw_acl_add_hw(lchip, pe, NULL));
        pe->fpae.flag = FPA_ENTRY_FLAG_INSTALLED;

        if (move_sw)
        {
            /* move to tail */
            ctc_slist_delete_node(pg->entry_list, &(pe->head));
            ctc_slist_add_tail(pg->entry_list, &(pe->head));
        }
    }
    else if (SYS_ACL_ENTRY_OP_FLAG_DELETE == flag)
    {
    	if(FPA_ENTRY_FLAG_UNINSTALLED == pe->fpae.flag)
        {
           return CTC_E_NONE;
        }

        CTC_ERROR_RETURN(_sys_usw_acl_remove_hw(lchip, pe));
        pe->fpae.flag = FPA_ENTRY_FLAG_UNINSTALLED;
        if (move_sw)
        {
            /* move to head */
            ctc_slist_delete_node(pg->entry_list, &(pe->head));
            ctc_slist_add_head(pg->entry_list, &(pe->head));
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* p_ctc_entry)
{
    uint32 tcam_count = 0;
    int32  ret = CTC_E_NONE;
    uint8  sys_key_type = 0;
    uint8  temp_step = 0;
    uint8  key_size = 0;
    sys_acl_entry_t  tmp_entry;

    sys_acl_group_t*    pg_lkup = NULL;
    sys_acl_entry_t*    pe_lkup = NULL;

    sys_acl_entry_t*    p_new_entry = NULL;
    sys_acl_buffer_t*   p_new_buffer = NULL;

    /* check sys group, if group not exist return*/
    _sys_usw_acl_get_group_by_gid(lchip, group_id, &pg_lkup);

    if (!pg_lkup)
    {
		return CTC_E_NOT_EXIST;
    }

    /* check sys entry, if entry exist return */
    _sys_usw_acl_get_entry_by_eid(lchip, p_ctc_entry->entry_id, &pe_lkup);
    if (pe_lkup)
    {
        return CTC_E_EXIST;
    }

    CTC_ERROR_RETURN(_sys_usw_acl_check_entry_type(lchip, group_id, pg_lkup, p_ctc_entry));
    sal_memset(&tmp_entry, 0, sizeof(sys_acl_entry_t));
    tmp_entry.key_type = p_ctc_entry->key_type;
    tmp_entry.group = pg_lkup;
    key_size = _sys_usw_acl_get_key_size(lchip, 0, &tmp_entry, &temp_step);
    if(_sys_usw_acl_get_key_size(lchip, 1, &tmp_entry, NULL) > key_size)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The current entry's key size can't exceed the group's max key size.\n");
        return CTC_E_INVALID_PARAM;
    }
    if (ACL_ENTRY_IS_TCAM(p_ctc_entry->key_type))
    {
       uint8 block_id = 0;
       sys_acl_league_t *p_league;

        /* tcam check */
        if (SYS_ACL_IS_RSV_GROUP(pg_lkup->group_id)) /* tcam entry only in tcam group */
        {
            return CTC_E_INVALID_CONFIG;
        }
        block_id = pg_lkup->group_info.block_id;
        if(CTC_EGRESS == pg_lkup->group_info.dir)
        {
            block_id += p_usw_acl_master[lchip]->igs_block_num;
        }

        p_league = &(p_usw_acl_master[lchip]->league[block_id]);
        if(p_league->lkup_level_bitmap == 0)
        {
           return CTC_E_NO_RESOURCE;
        }

        if(p_usw_acl_master[lchip]->sort_mode)
        {
            sys_acl_block_t* pb = NULL;
            uint16 temp_index;
            uint8  temp_key_size;
            uint8  step = temp_step;

            pb = &(p_usw_acl_master[lchip]->block[block_id]);
            if(p_ctc_entry->priority >= pb->fpab.entry_count)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Acl tcam key index exceed the max index\n");
                return CTC_E_EXCEED_MAX_SIZE;
            }

            if(!p_ctc_entry->priority_valid || p_ctc_entry->priority%step != 0)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Acl priority(index) is %u, key type is %u, the step is %u\n", \
                                                    p_ctc_entry->priority, p_ctc_entry->key_type, step);
                return CTC_E_INVALID_PARAM;
            }

            for(temp_index = p_ctc_entry->priority; temp_index < p_ctc_entry->priority+step; temp_index++)
            {
                if(pb->fpab.entries[temp_index])
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Acl tcam key index already used, used index is %u,line:%d\n",temp_index,__LINE__);
                    return CTC_E_IN_USE;
                }
            }

            for(temp_key_size = key_size;  temp_key_size < CTC_FPA_KEY_SIZE_NUM; temp_key_size++)
            {
                step = 1<<temp_key_size;
                temp_index = p_ctc_entry->priority/step*step;
                if(pb->fpab.entries[temp_index] && pb->fpab.entries[temp_index]->step == step)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Acl tcam key index already used, line:%d\n", __LINE__);
                    return CTC_E_IN_USE;
                }
            }
        }
        tcam_count = (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_80])
                   + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_160]*2)
                   + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_320]*4)
                   + (p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_TCAM_640]*8)
                   + temp_step;

        if(tcam_count > SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_ACL))
        {
            return CTC_E_NO_RESOURCE;
        }

    }
    else
    {
        if((p_usw_acl_master[lchip]->key_count[ACL_ALLOC_COUNT_HASH] + temp_step) > SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_ACL_FLOW))
        {
            return CTC_E_NO_RESOURCE;
        }
    }

    /*malloc new entry and buffer*/
    MALLOC_ZERO(MEM_ACL_MODULE, p_new_entry, sizeof(sys_acl_entry_t));
    MALLOC_ZERO(MEM_ACL_MODULE, p_new_buffer, sizeof(sys_acl_buffer_t));
    if((!p_new_entry) || (!p_new_buffer))
    {
        goto clean_up;
    }
    p_new_entry->buffer = p_new_buffer;
    p_new_entry->group  = pg_lkup;
    p_new_entry->key_type = p_ctc_entry->key_type;
    p_new_entry->mode   = p_ctc_entry->mode;

    /* build new entry inner field */
    p_new_entry->fpae.entry_id = p_ctc_entry->entry_id;
    p_new_entry->fpae.offset_a = CTC_MAX_UINT32_VALUE;  /*0xFFFFFFFF means key_index is not sure*/
    p_new_entry->fpae.lchip    = lchip;
    p_new_entry->head.next     = NULL;
    p_new_entry->hash_field_sel_id  = p_ctc_entry->hash_field_sel_id;
    if (p_ctc_entry->priority_valid)
    {
        p_new_entry->fpae.priority = p_ctc_entry->priority; /* meaningless for HASH */
    }
    else
    {
        p_new_entry->fpae.priority = FPA_PRIORITY_DEFAULT; /* meaningless for HASH */
    }

    if((CTC_ACL_KEY_COPP != p_ctc_entry->key_type) && (CTC_ACL_KEY_COPP_EXT != p_ctc_entry->key_type))
    {
        pg_lkup->group_info.bitmap_status = ACL_BITMAP_STATUS_16;
    }

    CTC_ERROR_GOTO(_sys_usw_acl_add_key_common_field_group_none(lchip, p_new_entry),ret, clean_up);

    if (ACL_ENTRY_IS_TCAM(p_new_entry->key_type))
    {
        CTC_ERROR_GOTO(_sys_usw_acl_add_tcam_entry(lchip, pg_lkup, p_new_entry), ret, clean_up);
    }
    else
    {
        CTC_ERROR_GOTO( _sys_usw_acl_set_hash_sel_bmp(lchip,p_new_entry), ret, clean_up);
        CTC_ERROR_GOTO(_sys_usw_acl_add_hash_entry(lchip, pg_lkup, p_new_entry), ret, clean_up);
        CTC_ERROR_GOTO(sys_usw_acl_map_ctc_to_sys_hash_key_type(p_new_entry->key_type,&sys_key_type), ret, clean_up);
        p_usw_acl_master[lchip]->hash_sel_profile_count[sys_key_type][p_new_entry->hash_field_sel_id]++;
    }

    /*build group info*/
	_sys_usw_acl_add_group_key_port_field(lchip,p_ctc_entry->entry_id,pg_lkup);

    return CTC_E_NONE;

clean_up:
    if(p_new_buffer)
    {
        mem_free(p_new_buffer);
    }
    if(p_new_entry)
    {
        mem_free(p_new_entry);
    }
    return ret;
}

int32
_sys_usw_acl_remove_entry(uint8 lchip, uint32 entry_id)
{
    sys_acl_group_t* pg = NULL;
    sys_acl_entry_t* pe = NULL;
    sys_acl_block_t* pb = NULL;
    uint8 sys_key_type = 0;
	ctc_acl_field_action_t action_field;

    /* check raw entry */
    _sys_usw_acl_get_nodes_by_eid(lchip, entry_id, &pe, &pg, &pb);
    if((!pe) || (!pg))
    {
        CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
    }
    if (FPA_ENTRY_FLAG_INSTALLED == pe->fpae.flag)
    {
        CTC_ERROR_RETURN(CTC_E_INVALID_CONFIG);
    }
    /*reset action to default ,free resource*/
    sal_memset(&action_field,0,sizeof(action_field));
    action_field.type = CTC_ACL_FIELD_ACTION_CANCEL_ALL;
    if(ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_ERROR_RETURN(_sys_usw_acl_add_dsacl_field(lchip, &action_field, pe,(CTC_ACL_KEY_INTERFACE == pe->key_type), 1));
        CTC_ERROR_RETURN(_sys_usw_acl_remove_tcam_entry(lchip, pe, pg, pb));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_acl_add_dsflow_field(lchip, &action_field, pe, 1));
        CTC_ERROR_RETURN(_sys_usw_acl_remove_hash_entry(lchip, pe, pg, pb));
        CTC_ERROR_RETURN(sys_usw_acl_map_ctc_to_sys_hash_key_type(pe->key_type, &sys_key_type));
        if (p_usw_acl_master[lchip]->hash_sel_profile_count[sys_key_type][pe->hash_field_sel_id] > 0)
        {
            p_usw_acl_master[lchip]->hash_sel_profile_count[sys_key_type][pe->hash_field_sel_id]--;
        }

        /* memory free */
        mem_free(pe);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id)
{
    sys_acl_group_t* pg      = NULL;
    ctc_slistnode_t* pe      = NULL;
    ctc_slistnode_t* pe_next = NULL;
    uint32           eid       = 0;

    /* get group node */
    _sys_usw_acl_get_group_by_gid(lchip, group_id, &pg);
    SYS_ACL_CHECK_GROUP_UNEXIST(pg);

    /* check if all uninstalled */
    CTC_SLIST_LOOP(pg->entry_list, pe)
    {
        if (FPA_ENTRY_FLAG_INSTALLED ==
            ((sys_acl_entry_t *) pe)->fpae.flag)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry installed \n");
			return CTC_E_INVALID_CONFIG;

        }
    }

    CTC_SLIST_LOOP_DEL(pg->entry_list, pe, pe_next)
    {
        eid = ((sys_acl_entry_t *) pe)->fpae.entry_id;
        /* no stop to keep consitent */
        _sys_usw_acl_remove_entry(lchip, eid);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_check_hash_sel_field_union(uint8 lchip, uint8 hash_type, uint8 field_sel_id, ctc_field_key_t* sel_field)
{
    uint8 l3_type=CTC_PARSER_L3_TYPE_NONE;
    sys_hash_sel_field_union_t*  p_sel_field_union;
    uint8 l2_l3_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_L2_L3);
    uint8 l3_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_IPV4);
    uint8 ipv6_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_IPV6);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sel_field_union = &(p_usw_acl_master[lchip]->hash_sel_key_union_filed[hash_type][field_sel_id]);
    p_sel_field_union->l3_key_u1_type = 1;  /*default to 1,current no support api (hezc)*/
    p_sel_field_union->ipv6_key_ip_mode = 1;  /*default to 1,current no support api(hezc)*/

    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if (p_sel_field_union->key_port_type == 2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to metadata field\n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        if (p_sel_field_union->key_port_type == 1)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to port field\n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IP_SA:                    /**< [D2] Source IPv4 Address. */
    case CTC_FIELD_KEY_IP_DA:                    /**< [D2] Destination IPv4 Address. */
    case CTC_FIELD_KEY_IPV6_SA:                  /**< [D2] Source IPv6 Address. */
    case CTC_FIELD_KEY_IPV6_DA:                  /**< [D2] Destination IPv6 Address. */
    case CTC_FIELD_KEY_IP_DSCP:                  /**< [D2] DSCP. */
    case CTC_FIELD_KEY_IP_PRECEDENCE:            /**< [D2] Precedence. */
    case CTC_FIELD_KEY_IP_ECN:                   /**< [D2] Ecn. */
    case CTC_FIELD_KEY_IP_FRAG:                  /**< [D2] IP Fragment Information. */
    case CTC_FIELD_KEY_IP_HDR_ERROR:             /**< [D2] Ip Header Error. */
    case CTC_FIELD_KEY_IP_OPTIONS:               /**< [D2] Ip Options. */
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:         /**< [D2] Ip Packet Length Range. */
    case CTC_FIELD_KEY_IP_TTL:                   /**< [D2] Ttl. */
        if (l3_key
            && (sel_field->type == CTC_FIELD_KEY_IP_FRAG || sel_field->type == CTC_FIELD_KEY_IP_HDR_ERROR ||  sel_field->type == CTC_FIELD_KEY_IP_OPTIONS)
            && (p_sel_field_union->l3_key_u1_type == 3))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l3_key
            && sel_field->type == CTC_FIELD_KEY_IP_TTL
            && (p_sel_field_union->l3_key_u1_type == 2 || p_sel_field_union->l3_key_u1_type == 3 ))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        if (ipv6_key
            && (p_sel_field_union->ipv6_key_ip_mode == 1 )
            && !((CTC_FIELD_KEY_IPV6_SA == sel_field->type) || (CTC_FIELD_KEY_IPV6_DA == sel_field->type)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ipv6 key ipv6_key_ip_mode:%d\n", p_sel_field_union->ipv6_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }
        if (l2_l3_key
            && (p_sel_field_union->l2_l3_key_ul3_type == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to mpls field\n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:          /**< [D2] Ipv6 Flow label*/
        if (ipv6_key
            && (p_sel_field_union->ipv6_key_ip_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ipv6 key ipv6_key_ip_mode:%d\n", p_sel_field_union->ipv6_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        /*only u1.g4 support IP_PROTOCOL*/
        if (l3_key
            && (p_sel_field_union->l3_key_u1_type != 0)
            && (p_sel_field_union->l3_key_u1_type != 4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        /*only u1.g2 support IP_PROTOCOL*/
        if (ipv6_key && p_sel_field_union->ipv6_key_ip_mode == 1)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ipv6 key ipv6_key_ip_mode:%d\n", p_sel_field_union->ipv6_key_ip_mode);
            return CTC_E_INVALID_PARAM;
        }

        break;
    case CTC_FIELD_KEY_TCP_ECN:                  /**< [D2] TCP Ecn. */
    case CTC_FIELD_KEY_TCP_FLAGS:                /**< [D2] TCP Flags (ctc_acl_tcp_flag_flag_t). */
        if (l3_key
            && sel_field->type == CTC_FIELD_KEY_TCP_FLAGS
            && (p_sel_field_union->l3_key_u1_type == 2 || p_sel_field_union->l3_key_u1_type == 4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        if (ipv6_key && (p_sel_field_union->ipv6_key_ip_mode == 1 ))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l2_l3_key && (p_sel_field_union->l2_l3_key_ul3_type == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to mpls field\n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:     /* l4_port_field; 3:l4 port */    /**< [D2] Layer 4 Dest Port. */
    case CTC_FIELD_KEY_L4_SRC_PORT:     /* l4_port_field; 3:l4 port */    /**< [D2] Layer 4 Src Port. */
        if ((l3_key || ipv6_key || l2_l3_key)
            && (p_sel_field_union->l4_port_field == 1 || p_sel_field_union->l4_port_field == 2
            || p_sel_field_union->l4_port_field == 4  || p_sel_field_union->l4_port_field == 5))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:        /**< [D2] Layer 4 Src Port Range;
        data: min: mask: max. */
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:        /**< [D2] Layer 4 Dest Port Range;
        data: min: mask: max. */
        if (l3_key
            && (p_sel_field_union->l3_key_u1_type == 1 || p_sel_field_union->l3_key_u1_type == 2 || p_sel_field_union->l3_key_u1_type == 4 ))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        if (ipv6_key
            && (p_sel_field_union->ipv6_key_ip_mode == 1))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ipv6_key The filed is used to full ip\n");
            return CTC_E_INVALID_PARAM;
        }
        if (l2_l3_key
            && (p_sel_field_union->l2_l3_key_ul3_type == 2))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to mpls field\n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_GRE_KEY:   /* l4_port_field; 1:gre/nvgre */
        if (((l3_key || ipv6_key || l2_l3_key)
            &&(p_sel_field_union->l4_port_field ==2 || p_sel_field_union->l4_port_field ==3
               ||p_sel_field_union->l4_port_field ==4 || p_sel_field_union->l4_port_field ==5))
            ||((l3_key || ipv6_key || l2_l3_key) &&(p_sel_field_union->l4_port_field ==1)
            &&((p_sel_field_union->l4_port_field_bmp & 0x02) !=0)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to other field :l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_FIELD_KEY_NVGRE_KEY: /* l4_port_field; 1:gre/nvgre */
        if (((l3_key || ipv6_key || l2_l3_key)
            &&(p_sel_field_union->l4_port_field ==2 || p_sel_field_union->l4_port_field ==3
               ||p_sel_field_union->l4_port_field ==4 || p_sel_field_union->l4_port_field ==5))
            ||((l3_key || ipv6_key || l2_l3_key) &&(p_sel_field_union->l4_port_field ==1)
            &&((p_sel_field_union->l4_port_field_bmp &0x01) !=0)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to other field :l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE: /* l4_port_field; 4:icmp */
    case CTC_FIELD_KEY_ICMP_TYPE: /* l4_port_field; 4:icmp */
        if ((l3_key || ipv6_key || l2_l3_key)
            &&(p_sel_field_union->l4_port_field ==1 || p_sel_field_union->l4_port_field ==2
            ||p_sel_field_union->l4_port_field ==3|| p_sel_field_union->l4_port_field ==5))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to other field :l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_IGMP_TYPE: /* 5:igmp */
        if (ipv6_key              /* ASIC:hash key only ipv6 support igmp */
            &&(p_sel_field_union->l4_port_field ==1 || p_sel_field_union->l4_port_field ==2
            ||p_sel_field_union->l4_port_field ==3 || p_sel_field_union->l4_port_field ==4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to other field :l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:              /**< [D2] Label Field of the MPLS Label 0. */
    case CTC_FIELD_KEY_MPLS_EXP0:                /**< [D2] Exp Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_SBIT0:               /**< [D2] S-bit Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_TTL0:                /**< [D2] Ttl Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_LABEL1:              /**< [D2] Label Field of the MPLS Label 1. */
    case CTC_FIELD_KEY_MPLS_EXP1:                /**< [D2] Exp Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_SBIT1:               /**< [D2] S-bit Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_TTL1:                /**< [D2] Ttl Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_LABEL2:              /**< [D2] Label Field of the MPLS Label 2. */
    case CTC_FIELD_KEY_MPLS_EXP2:                /**< [D2] Exp Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_SBIT2:               /**< [D2] S-bit Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_TTL2:                /**< [D2] Ttl Field of the MPLS Label 2.*/
        if (l2_l3_key && (p_sel_field_union->l2_l3_key_ul3_type == 1 || p_sel_field_union->l2_l3_key_ul3_type == 3))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to Ip field or Unknown_etherType:%d\n",p_sel_field_union->l2_l3_key_ul3_type );
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_VRFID:
    case CTC_FIELD_KEY_IS_ROUTER_MAC:
        if (l3_key
            && (p_sel_field_union->l3_key_u1_type ==1 ||p_sel_field_union->l3_key_u1_type ==3 || p_sel_field_union->l3_key_u1_type ==4))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 key u1_type:%d\n", p_sel_field_union->l3_key_u1_type);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_VN_ID:  /* l4_port_field; 2:vxlan  */
        if ((l3_key || ipv6_key || l2_l3_key)
            && (p_sel_field_union->l4_port_field == 1 || p_sel_field_union->l4_port_field == 3
            || p_sel_field_union->l4_port_field == 4  || p_sel_field_union->l4_port_field == 5))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l4_port_field:%d\n", p_sel_field_union->l4_port_field);
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        if (l2_l3_key && (p_sel_field_union->l2_l3_key_ul3_type == 1 || p_sel_field_union->l2_l3_key_ul3_type == 2 ))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The filed is used to Ip field or Mpls fields:%d\n",p_sel_field_union->l2_l3_key_ul3_type);
            return CTC_E_INVALID_PARAM;
        }
        if (l2_l3_key && ((CTC_PARSER_L3_TYPE_IPV4 ==l3_type) || (CTC_PARSER_L3_TYPE_MPLS ==l3_type) || (CTC_PARSER_L3_TYPE_MPLS_MCAST ==l3_type)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Key filed:%s (EthType:0x%x) in l2_l3_key must be unknownPktEther .\n (l3_type:%d)n", "CTC_FIELD_KEY_ETHER_TYPE", sel_field->data, l3_type);
        }
        break;
    default :
        break;
    }

    return CTC_E_NONE;

}

int32
_sys_usw_acl_set_hash_sel_field_union(uint8 lchip, uint8 hash_type, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    sys_hash_sel_field_union_t*  p_sel_field_union;
    uint8 l2_l3_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_L2_L3);
    uint8 l3_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_IPV4);
    uint8 ipv6_key = (hash_type == SYS_ACL_HASH_KEY_TYPE_IPV6);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sel_field_union = &(p_usw_acl_master[lchip]->hash_sel_key_union_filed[hash_type][field_sel_id]);
    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        p_sel_field_union->key_port_type = is_add ? 1 :0;
        break;
    case CTC_FIELD_KEY_METADATA:
        p_sel_field_union->key_port_type = is_add ? 2 :0;
        break;
    case CTC_FIELD_KEY_IP_SA:                    /**< [D2] Source IPv4 Address. */
    case CTC_FIELD_KEY_IP_DA:                    /**< [D2] Destination IPv4 Address. */
    case CTC_FIELD_KEY_IP_DSCP:                  /**< [D2] DSCP. */
    case CTC_FIELD_KEY_IP_PRECEDENCE:            /**< [D2] Precedence. */
    case CTC_FIELD_KEY_IP_ECN:                   /**< [D2] Ecn. */
    case CTC_FIELD_KEY_IP_FRAG:                  /**< [D2] IP Fragment Information. */
    case CTC_FIELD_KEY_IP_HDR_ERROR:             /**< [D2] Ip Header Error. */
    case CTC_FIELD_KEY_IP_OPTIONS:               /**< [D2] Ip Options. */
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:         /**< [D2] Ip Packet Length Range. */
    case CTC_FIELD_KEY_IP_TTL:                   /**< [D2] Ttl. */
        if (l2_l3_key)
        {
            p_sel_field_union->l2_l3_key_ul3_type = is_add ? 1 :0;
        }
        break;

    /* for 3:l4 port;   bit2:CTC_FIELD_KEY_L4_DST_PORT; bit3:CTC_FIELD_KEY_L4_SRC_PORT; (can exist at the same time)*/
    case CTC_FIELD_KEY_L4_DST_PORT:              /**< [D2] Layer 4 Dest Port. */
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x04;
                p_sel_field_union->l4_port_field = 3;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x04;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x0C) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;
    /* for 3:l4 port;   bit2:CTC_FIELD_KEY_L4_DST_PORT; bit3:CTC_FIELD_KEY_L4_SRC_PORT; (can exist at the same time)*/
    case CTC_FIELD_KEY_L4_SRC_PORT:              /**< [D2] Layer 4 Src Port. */
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x08;
                p_sel_field_union->l4_port_field = 3;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x08;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x0C) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:        /**< [D2] Layer 4 Src Port Range;
        data: min: mask: max. */
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:        /**< [D2] Layer 4 Dest Port Range;
        data: min: mask: max. */
        if (l3_key)
        {
            p_sel_field_union->l3_key_u1_type = is_add ? 3 :0;
        }
        if (ipv6_key)
        {
            p_sel_field_union->ipv6_key_ip_mode =is_add ? 2 :0;
        }
        if (l2_l3_key)
        {
            p_sel_field_union->l2_l3_key_ul3_type = is_add ? 1 :0;
        }
        break;
    case CTC_FIELD_KEY_TCP_ECN:                  /**< [D2] TCP Ecn. */
    case CTC_FIELD_KEY_TCP_FLAGS:                /**< [D2] TCP Flags (ctc_acl_tcp_flag_flag_t). */
        if (l2_l3_key )
        {
            p_sel_field_union->l2_l3_key_ul3_type = is_add ? 1 :0;
        }
        break;
    case CTC_FIELD_KEY_GRE_KEY:  /*for 1:gre/nvgre; bit0:CTC_FIELD_KEY_GRE_KEY;     bit1:CTC_FIELD_KEY_NVGRE_KEY;   (can't exist at the same time)*/
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x01;
                p_sel_field_union->l4_port_field = 1;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x01;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x03) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;
    case CTC_FIELD_KEY_NVGRE_KEY: /*for 1:gre/nvgre; bit0:CTC_FIELD_KEY_GRE_KEY;     bit1:CTC_FIELD_KEY_NVGRE_KEY;   (can't exist at the same time)*/
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x02;
                p_sel_field_union->l4_port_field = 1;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x02;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x03) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;

    case CTC_FIELD_KEY_ICMP_TYPE:  /*for 4:icmp; bit4:CTC_FIELD_KEY_ICMP_CODE; bit5:CTC_FIELD_KEY_ICMP_TYPE; (can exist at the same time)*/
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x20;
                p_sel_field_union->l4_port_field = 4;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x20;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x30) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE:   /*for 4:icmp; bit4:CTC_FIELD_KEY_ICMP_CODE; bit5:CTC_FIELD_KEY_ICMP_TYPE; (can exist at the same time)*/
        if (l3_key || ipv6_key || l2_l3_key)
        {
            if(is_add)
            {
                p_sel_field_union->l4_port_field_bmp |= 0x10;
                p_sel_field_union->l4_port_field = 4;
            }
            else
            {
                p_sel_field_union->l4_port_field_bmp &= ~0x10;
                p_sel_field_union->l4_port_field = ( (p_sel_field_union->l4_port_field_bmp & 0x30) == 0) ? 0 : p_sel_field_union->l4_port_field;
            }
        }
        break;

    case CTC_FIELD_KEY_IGMP_TYPE:    /*for 5:igmp;      CTC_FIELD_KEY_IGMP_TYPE (only one field, no need l4_port_field_bmp)*/
        p_sel_field_union->l4_port_field = is_add ? 5:0;
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:              /**< [D2] Label Field of the MPLS Label 0. */
    case CTC_FIELD_KEY_MPLS_EXP0:                /**< [D2] Exp Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_SBIT0:               /**< [D2] S-bit Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_TTL0:                /**< [D2] Ttl Field of the MPLS Label 0.*/
    case CTC_FIELD_KEY_MPLS_LABEL1:              /**< [D2] Label Field of the MPLS Label 1. */
    case CTC_FIELD_KEY_MPLS_EXP1:                /**< [D2] Exp Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_SBIT1:               /**< [D2] S-bit Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_TTL1:                /**< [D2] Ttl Field of the MPLS Label 1.*/
    case CTC_FIELD_KEY_MPLS_LABEL2:              /**< [D2] Label Field of the MPLS Label 2. */
    case CTC_FIELD_KEY_MPLS_EXP2:                /**< [D2] Exp Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_SBIT2:               /**< [D2] S-bit Field of the MPLS Label 2.*/
    case CTC_FIELD_KEY_MPLS_TTL2:                /**< [D2] Ttl Field of the MPLS Label 2.*/
        if (l2_l3_key)
        {
            p_sel_field_union->l2_l3_key_ul3_type = is_add ? 2 :0;
        }
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        if (l2_l3_key)
        {
            p_sel_field_union->l2_l3_key_ul3_type = is_add ? 3:0;
        }
        break;
    case CTC_FIELD_KEY_VN_ID:   /*for 2:vxlan;     CTC_FIELD_KEY_VN_ID     (only one field, no need l4_port_field_bmp)*/
        if (l3_key || ipv6_key || l2_l3_key)
        {
            p_sel_field_union->l4_port_field = is_add ? 2 :0;
        }
        break;
    default :
        break;
    }

    if (l3_key)
    {
        p_sel_field_union->l3_key_u1_type = 1; /*full ip*/
    }
    if (ipv6_key)
    {
        p_sel_field_union->ipv6_key_ip_mode = 1;   /*full ip*/
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_free_entry_node_data(void* node_data, void* user_data)
{
    uint32 free_entry_hash = 0;
    sys_acl_entry_t* pe = NULL;

    if (user_data)
    {
        free_entry_hash = *(uint32*)user_data;
        if (1 == free_entry_hash)
        {
            pe = (sys_acl_entry_t*)node_data;
            if (NULL != pe->buffer)
            {
                mem_free(pe->buffer);
            }
        }
    }

    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_free_group_node_data(void* node_data, void* user_data)
{
    sys_acl_group_t* pg = NULL;

    if (node_data)
    {
        pg = (sys_acl_group_t*)node_data;

        ctc_slist_free(pg->entry_list);

        mem_free(node_data);
    }

    return CTC_E_NONE;
}
#define _ACL_UDF_ENTRY_

int32
_sys_usw_acl_get_udf_info(uint8 lchip,  uint32 udf_id, sys_acl_udf_entry_t **p_udf_entry)
{
    uint8 loop = 0;
    (*p_udf_entry) = NULL;
    for(loop = 0; loop < SYS_ACL_MAX_UDF_ENTRY_NUM;loop++)
    {
        if(p_usw_acl_master[lchip]->udf_entry[loop].key_index_used
        && p_usw_acl_master[lchip]->udf_entry[loop].udf_id == udf_id)
        {
            (*p_udf_entry) = &p_usw_acl_master[lchip]->udf_entry[loop];
            break;
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_check_udf_entry_key_field_union(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add)
{
    sys_acl_udf_entry_t    *p_sys_udf_entry;

    _sys_usw_acl_get_udf_info(lchip, udf_id, &p_sys_udf_entry);
    if(!p_sys_udf_entry || !p_sys_udf_entry->key_index_used)
	{
		return CTC_E_NOT_EXIST;
	}
	if(!is_add)
	{
		key_field->mask = 0;
	}

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        {
           ctc_field_port_t *p_ctc_field_port;
           CTC_PTR_VALID_CHECK(key_field->ext_data);
           p_ctc_field_port = (ctc_field_port_t*)(key_field->ext_data);
           if (CTC_FIELD_PORT_TYPE_PORT_BITMAP != p_ctc_field_port->type)
           {
                return CTC_E_INVALID_PARAM;
           }
    	}
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        CTC_MAX_VALUE_CHECK(key_field->data, 0x7);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        CTC_MAX_VALUE_CHECK(key_field->data, 0x3);
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        /*ethertype no need to check param valid*/
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        if(p_sys_udf_entry->mpls_num)
		{
			return CTC_E_INVALID_PARAM;
		}
        CTC_MAX_VALUE_CHECK(key_field->data, 1);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(p_sys_udf_entry->mpls_num)
		{
			return CTC_E_INVALID_PARAM;
		}
        CTC_MAX_VALUE_CHECK(key_field->data, CTC_IP_FRAG_MAX-1);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        if(p_sys_udf_entry->ip_op || p_sys_udf_entry->ip_frag)
		{
			return CTC_E_INVALID_PARAM;
		}
        CTC_MAX_VALUE_CHECK(key_field->data, 9);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        break;
    case CTC_FIELD_KEY_GRE_FLAGS:
	case CTC_FIELD_KEY_GRE_PROTOCOL_TYPE:
        if(p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_GRE || p_sys_udf_entry->ip_protocol)
	    {
			return CTC_E_INVALID_PARAM;
		}
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
    case CTC_FIELD_KEY_L4_SRC_PORT:
    case CTC_FIELD_KEY_L4_USER_TYPE:
    case CTC_FIELD_KEY_TCP_OPTIONS:
        if((   p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_TCP
			&& p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_UDP
			&& p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_RDP
			&& p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_SCTP
			&& p_sys_udf_entry->l4_type != CTC_PARSER_L4_TYPE_DCCP)
			|| p_sys_udf_entry->ip_protocol)
	    {
            return CTC_E_INVALID_PARAM;
		}

        if(key_field->type == CTC_FIELD_KEY_L4_DST_PORT ||
        	key_field->type == CTC_FIELD_KEY_L4_SRC_PORT)
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xFFFF);
        }
		if(key_field->type == CTC_FIELD_KEY_L4_USER_TYPE )
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 0xF);
        }
		if(key_field->type == CTC_FIELD_KEY_TCP_OPTIONS )
        {
            CTC_MAX_VALUE_CHECK(key_field->data, 1);
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
         if((p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_TCP
			|| p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_UDP
			|| p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_RDP
			|| p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_SCTP
			|| p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_DCCP
			|| p_sys_udf_entry->l4_type == CTC_PARSER_L4_TYPE_GRE))
	    {
			return CTC_E_INVALID_PARAM;
		}
        CTC_MAX_VALUE_CHECK(key_field->data, 0xFF);
        break;
    case CTC_FIELD_KEY_UDF_ENTRY_VALID:
 	    break;
    default:
        return CTC_E_NOT_SUPPORT;
    }
    return CTC_E_NONE;
}

int32 _sys_usw_acl_set_udf_entry_key_field_union(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add)
{
    sys_acl_udf_entry_t    *p_sys_udf_entry;

    _sys_usw_acl_get_udf_info(lchip, udf_id, &p_sys_udf_entry);
    if(!p_sys_udf_entry || !p_sys_udf_entry->key_index_used)
	{
        return CTC_E_NOT_EXIST;
	}
    switch(key_field->type)
    {
    case CTC_FIELD_KEY_IP_OPTIONS:
        p_sys_udf_entry->ip_op = is_add ? 1:0;
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        p_sys_udf_entry->ip_frag = is_add ? 1:0;
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        p_sys_udf_entry->mpls_num = is_add ? 1:0;
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        p_sys_udf_entry->l4_type = is_add ? key_field->data:0;
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        p_sys_udf_entry->ip_protocol = is_add ? 1:0;
        break;
    }
    return CTC_E_NONE;
}

#define _ACL_LEAGUE_

/*
    This function is used to travese one block and store entry to list.
*/
int32 _sys_usw_acl_league_traverse_block(uint8 lchip, uint8 tmp_block_id, uint16* entry_base,
                                                ctc_linklist_t** node_list, uint16* sub_entry_num,
                                                uint16* sub_free_num, sys_usw_acl_league_node_t** node_array)
{
    uint8  key_size;
    uint8  step = 1;
    uint16 start;
    uint16 end;
    uint16 offset;
    uint16 tmp_count;
    uint16 sub_total_count;
    sys_usw_acl_league_node_t* p_node = NULL;
    ctc_fpa_block_t* p_b = &p_usw_acl_master[lchip]->block[tmp_block_id].fpab;
    ctc_fpa_entry_t* p_e = NULL;
    sys_acl_entry_t* p_entry = NULL;
    if(p_b == NULL)
    {
        return CTC_E_INVALID_PARAM;
    }

    for(key_size=CTC_FPA_KEY_SIZE_80; key_size < CTC_FPA_KEY_SIZE_NUM; step *=2, key_size++)
    {
        if(0 == p_b->sub_entry_count[key_size])
        {
            continue;
        }
        sub_total_count = p_b->sub_entry_count[key_size]- p_b->sub_free_count[key_size];
        tmp_count = 0;
        sub_entry_num[key_size] += p_b->sub_entry_count[key_size];
        sub_free_num[key_size] += p_b->sub_free_count[key_size];
        start = p_b->start_offset[key_size];
        end = start + (p_b->sub_entry_count[key_size])*step;

        for(offset=start,tmp_count=0; (offset < end)&&(tmp_count<sub_total_count); offset+=step)
        {
            p_e = p_b->entries[offset];
            if(NULL == p_e)
            {
                continue;
            }
            tmp_count++;
            if(NULL == (p_node = mem_malloc(MEM_ACL_MODULE, sizeof(sys_usw_acl_league_node_t))))
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_node, 0, sizeof(sys_usw_acl_league_node_t));
            p_node->entry_id = p_e->entry_id;
            p_node->prio = p_e->priority;
            p_node->old_index = offset+(*entry_base);
            p_node->step = p_e->step;
            p_entry = _ctc_container_of(p_e, sys_acl_entry_t, fpae);
            p_node->old_block_id  = p_entry->group->group_info.block_id;
            ctc_listnode_add_sort(node_list[key_size], p_node);
            node_array[p_node->old_index] = p_node;
        }
    }
    (*entry_base) += p_b->entry_count;
    return CTC_E_NONE;
}
/*
    This function  is used to modified soft table and move entry when needed
*/
int32 _sys_usw_acl_league_move_entry(uint8 lchip, ctc_linklist_t** node_list, ctc_fpa_entry_t** p_tmp_entries,
                                                uint16* sub_entry_num, uint8 block_id, sys_usw_acl_league_node_t** node_array)
{
    ctc_listnode_t* node;
    uint16 new_idx = 0;
    uint16 tmp_entry_cnt = 0;
    uint32 old_key;
    uint32 old_ad;
    uint32 old_hw_idx;
    uint32 new_key;
    uint32 new_ad;
    uint32 new_hw_idx;
    uint32 cmd = 0;
    sys_acl_entry_t* p_e = NULL;
    sys_acl_entry_t* p_tmp_entry = NULL;
    uint16 base = 0;
    uint8  key_size = 0;
    uint8  step = 1;
    uint16 tmp_idx = 0;
    sys_usw_acl_league_node_t* p_node = NULL;

    for(key_size=CTC_FPA_KEY_SIZE_80; key_size < CTC_FPA_KEY_SIZE_NUM; key_size++)
    {
        tmp_entry_cnt = 0;
        CTC_LIST_LOOP(node_list[key_size], p_node, node)
        {
            new_idx = base+(tmp_entry_cnt*step);
            tmp_entry_cnt++;
            _sys_usw_acl_get_entry_by_eid(lchip, p_node->entry_id, &p_e);

            /*temp change block id for get table used*/
            p_e->group->group_info.block_id = p_node->old_block_id;
            _sys_usw_acl_get_table_id(lchip, p_e, &old_key, &old_ad, &old_hw_idx);

            p_e->group->group_info.block_id = block_id;
            p_e->fpae.offset_a = new_idx;
            p_tmp_entries[new_idx] = &(p_e->fpae);

            if(new_idx != p_node->old_index)
            {
                tbl_entry_t         tcam_key;
                sys_acl_buffer_t  tmp_buffer;
                sys_acl_buffer_t*   p_new_buffer = NULL;
                uint8 tmp_block_id = 0;

                /*1. if the entry already exists at the location to be written, the entry needs to be read into buffer first*/
                for(tmp_idx=new_idx/8*8; tmp_idx < new_idx/8*8+8; tmp_idx++)
                {
                    if(node_array[tmp_idx] && (node_array[tmp_idx]->p_buffer == NULL) && node_array[tmp_idx]->status == 0 &&
                        ((tmp_idx <= new_idx && new_idx < tmp_idx+node_array[tmp_idx]->step) || (tmp_idx > new_idx && tmp_idx < new_idx+step)))
                    {
                        _sys_usw_acl_get_entry_by_eid(lchip, node_array[tmp_idx]->entry_id, &p_tmp_entry);
                        /*temp change block id for get table used*/
                        tmp_block_id = p_tmp_entry->group->group_info.block_id;
                        p_tmp_entry->group->group_info.block_id = node_array[tmp_idx]->old_block_id;
                        _sys_usw_acl_get_table_id(lchip, p_tmp_entry, &new_key, &new_ad, &new_hw_idx);
                        p_tmp_entry->group->group_info.block_id = tmp_block_id;

                        sal_memset(&tcam_key, 0, sizeof(tcam_key));
                        MALLOC_ZERO(MEM_ACL_MODULE, p_new_buffer, sizeof(sys_acl_buffer_t));
                        if(!p_new_buffer)
                        {
                            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                    		return CTC_E_NO_RESOURCE;
                        }

                        {
                            cmd = DRV_IOR(new_key, DRV_ENTRY_FLAG);
                            tcam_key.data_entry = p_new_buffer->key;
                            tcam_key.mask_entry = p_new_buffer->mask;
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(new_hw_idx, p_tmp_entry->fpae.real_step), cmd, &tcam_key));

                            cmd = DRV_IOR(new_ad, DRV_ENTRY_FLAG);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(new_hw_idx, p_tmp_entry->fpae.real_step), cmd, p_new_buffer->action));
                        }
                        node_array[tmp_idx]->p_buffer = p_new_buffer;
                    }
                }

                /*2. if old entry in buffer, recovery from buffer*/
                _sys_usw_acl_get_table_id(lchip, p_e, &new_key, &new_ad, &new_hw_idx);
                if(node_array[p_node->old_index]->p_buffer)
                {
                    cmd = DRV_IOW(new_ad, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(new_hw_idx, p_e->fpae.real_step), cmd, &(node_array[p_node->old_index]->p_buffer->action)));

                    cmd = DRV_IOW(new_key, DRV_ENTRY_FLAG);
                    tcam_key.data_entry = node_array[p_node->old_index]->p_buffer->key;
                    tcam_key.mask_entry = node_array[p_node->old_index]->p_buffer->mask;
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(new_hw_idx, p_e->fpae.real_step), cmd, &tcam_key));
                    mem_free(node_array[p_node->old_index]->p_buffer);

                }
                else
                {
                    cmd = DRV_IOR(old_key, DRV_ENTRY_FLAG);
                    tcam_key.data_entry = tmp_buffer.key;
                    tcam_key.mask_entry = tmp_buffer.mask;
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(old_hw_idx, p_e->fpae.real_step), cmd, &tcam_key));
                    cmd = DRV_IOR(old_ad, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(old_hw_idx, p_e->fpae.real_step), cmd, &tmp_buffer.action));

                    cmd = DRV_IOW(new_ad, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_AD_INDEX(new_hw_idx, p_e->fpae.real_step), cmd, &tmp_buffer.action));
                    cmd = DRV_IOW(new_key, DRV_ENTRY_FLAG);
                    tcam_key.data_entry = tmp_buffer.key;
                    tcam_key.mask_entry = tmp_buffer.mask;
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(new_hw_idx, p_e->fpae.real_step), cmd, &tcam_key));

                    cmd = DRV_IOD(old_key, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(old_hw_idx, p_e->fpae.real_step), cmd, &tcam_key));
                }
                node_array[p_node->old_index]->status = 1;
            }
        }
        base += sub_entry_num[key_size]*step;
        step *= 2;
    }
    return CTC_E_NONE;
}
/*
 league1: working level
 league2: merged level
*/

int32
_sys_usw_acl_set_league_merge(uint8 lchip, sys_acl_league_t* p_league1, sys_acl_league_t* p_league2, uint8 auto_en)
{
    uint8 lkup_level1= 0;
    uint8 lkup_level2 = 0;
    uint32 entry_enum = 0;


    lkup_level1 = ((p_league1->block_id >= ACL_IGS_BLOCK_MAX_NUM) ? p_league1->block_id-ACL_IGS_BLOCK_MAX_NUM: p_league1->block_id);
    lkup_level2 = ((p_league2->block_id >= ACL_IGS_BLOCK_MAX_NUM) ? p_league2->block_id-ACL_IGS_BLOCK_MAX_NUM: p_league2->block_id);

    CTC_BIT_SET(p_league1->lkup_level_bitmap,lkup_level2);
    CTC_BIT_UNSET(p_league2->lkup_level_bitmap,lkup_level2);
    p_league2->merged_to = lkup_level1;

    entry_enum = p_usw_acl_master[lchip]->block[p_league2->block_id].entry_num;
    p_league1->lkup_level_start[lkup_level2] = p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.entry_count;
    p_league1->lkup_level_count[lkup_level2] = entry_enum;
    p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.entry_count += entry_enum;

    /*The following soft table does not need to be modifed when auto mode is enabled*/
    if(!auto_en)
    {
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.free_count  += entry_enum;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] += entry_enum/8;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] += entry_enum/8;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;
    }
    /*free merged block*/
    sal_memset(&(p_usw_acl_master[lchip]->block[p_league2->block_id].fpab),0, sizeof(ctc_fpa_block_t)-sizeof(sys_acl_entry_t*)) ;
    mem_free(p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.entries);
    return CTC_E_NONE;
}

/*
 league1: working level
 league2: canceled level
*/
int32
_sys_usw_acl_set_league_cancel(uint8 lchip, sys_acl_league_t* p_league1, sys_acl_league_t* p_league2)
{
    uint8 lkup_level2 = 0;
    uint32 entry_num = 0;
    ctc_fpa_block_t* pb = NULL;
 /*    uint32 sub_entry_count = 0;*/
   /*  uint32 sub_free_count = 0;*/

    uint16 tmp_count = 0;
    uint16 start_offset = 0;
    uint16 entry_size;
    /* uint16 per_entry_num = 0;*/
    /* uint16 key_size = 0;*/

   lkup_level2 = ((p_league2->block_id >= ACL_IGS_BLOCK_MAX_NUM) ? p_league2->block_id-ACL_IGS_BLOCK_MAX_NUM: p_league2->block_id);

   entry_num = p_usw_acl_master[lchip]->block[p_league2->block_id].entry_num;
   pb = &p_usw_acl_master[lchip]->block[p_league1->block_id].fpab;
   /*free cancelled block resources*/
   if( p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.free_count < entry_num)
   {
      SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "free cancelled block resources:src block :%d dst block:%d\n",
        p_league1->block_id,p_league2->block_id);

      SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "free cancelled block resources:entry_num :%d free_count:%d\n",
       entry_num,p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.free_count);

      return CTC_E_INVALID_CONFIG;
   }
   else
   {
       p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.entry_count -= entry_num;
       p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.free_count -= entry_num;
       CTC_BIT_UNSET(p_league1->lkup_level_bitmap,lkup_level2);
   }

    if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] && p_league1->lkup_level_start[lkup_level2] >= p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_640])
    {
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] -= entry_num/8;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] -= entry_num/8;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;
    }
    else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_320] && p_league1->lkup_level_start[lkup_level2] >= p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_320])
    {
        tmp_count = p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] * 8;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_640] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] -= (entry_num-tmp_count)/4;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320] -= (entry_num-tmp_count)/4;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;
    }
    else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_160] && p_league1->lkup_level_start[lkup_level2] >= p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_160])
    {
        tmp_count = p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] * 8 + \
                    p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] * 4;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_640] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_320] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] -= (entry_num-tmp_count)/2;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160] -= (entry_num-tmp_count)/2;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160] = 0;
    }
    else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_80] && p_league1->lkup_level_start[lkup_level2] >= p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_80])
    {
        tmp_count = p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] * 8 + \
                    p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] * 4 + \
                    p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] * 2;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_640] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_320] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_160] = p_league1->lkup_level_start[lkup_level2];
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160] = 0;
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160] = 0;

        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] -= (entry_num-tmp_count);
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_80] -= (entry_num-tmp_count);
        p_usw_acl_master[lchip]->block[p_league1->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_80] = 0;
    }

   /*init new block */
   entry_size = sizeof(sys_acl_entry_t*) * entry_num;
   MALLOC_ZERO(MEM_ACL_MODULE, p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.entries, entry_size);
   if (NULL == p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.entries && entry_size)
   {
         return CTC_E_NO_MEMORY;
   }

   p_league2->lkup_level_start[lkup_level2] = 0;
   p_league2->lkup_level_count[lkup_level2] = entry_num;
   p_league2->lkup_level_bitmap = 0;
   CTC_BIT_SET(p_league2->lkup_level_bitmap, lkup_level2);
   p_league2->merged_to = lkup_level2;

   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.entry_count = entry_num;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.free_count  = entry_num;
   start_offset = 0;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_80]    = start_offset;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]  = 0;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_80]   = 0;
   start_offset = 0;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_160]    = start_offset;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] = entry_num/4;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160]  = entry_num/4;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160]  = 0;
   start_offset += entry_num/2;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_320]    = start_offset;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = entry_num*3/32;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320]  = entry_num*3/32;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;
   start_offset += entry_num*3/8;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.start_offset[CTC_FPA_KEY_SIZE_640]    = start_offset;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = entry_num/64;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640]  = entry_num/64;
   p_usw_acl_master[lchip]->block[p_league2->block_id].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;
    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_entry_count_on_lkup_level(uint8 lchip, uint8 block_id, uint8 level, uint32* count)
{
    uint8 step = 0;
    uint16 block_idx = 0;
    uint16 start_idx = 0;
    uint16 end_idx = 0;
    uint32 tmp_count = 0;
    sys_acl_block_t* pb = NULL;
    sys_acl_league_t* p_sys_league = NULL;

    CTC_PTR_VALID_CHECK(count);

    pb = &p_usw_acl_master[lchip]->block[block_id];
    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);
    start_idx = p_sys_league->lkup_level_start[level];
    end_idx = p_sys_league->lkup_level_start[level] + p_sys_league->lkup_level_count[level];
    *count = 0;

    step  = 1;
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

        if (pb->fpab.entries[block_idx])
        {
            tmp_count++;
        }
    }

    *count = tmp_count;

    return CTC_E_NONE;
}

#define _ACL_REORDER_
int32
_sys_usw_acl_reorder_entry_scatter(uint8 lchip, sys_acl_league_t* p_sys_league)
{
    uint16 start_offset_160 = 0;
    uint16 start_offset_320 = 0;
    uint16 start_offset_640 = 0;
    uint16 cur_width_160 = 0;
    uint16 cur_width_320 = 0;
    uint16 cur_width_640 = 0;
    int32 shift_160 = 0;
    int32 shift_320 = 0;
    int32 shift_640 = 0;
    int32 vary = 0;
    int32 vary_160 = 0;
    int32 vary_320 = 0;
    int32 vary_640 = 0;
    ctc_fpa_block_t tmp_fpab;
    ctc_fpa_block_t* old_fpab = NULL;
    sys_acl_block_t * pb = NULL;

    SYS_ACL_DBG_FUNC();

    pb = &(p_usw_acl_master[lchip]->block[p_sys_league->block_id]);
    sal_memset(&tmp_fpab, 0, sizeof(ctc_fpa_block_t));
    sal_memcpy(&tmp_fpab, &pb->fpab, sizeof(ctc_fpa_block_t));

    old_fpab = &pb->fpab;

    if (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] != old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80])
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_160]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_160]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_320]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_640]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;

        vary = (int32)old_fpab->entry_count - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80];

        fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_80, 0, vary);
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] += vary;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] += vary;
    }
    else
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_80] = 0;

        start_offset_160 = 0;
        start_offset_320 = (old_fpab->entry_count)/2;
        start_offset_640 = start_offset_320 + (old_fpab->entry_count)*3/8;

        cur_width_160 = (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160])*2;
        cur_width_320 = (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320])*4;
        cur_width_640 = (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640])*8;

        /*scatter default 160 size not enough, move 320 down*/
        if ((start_offset_320 - start_offset_160) < cur_width_160)
        {
            start_offset_320 = (start_offset_160 + cur_width_160);
            if (start_offset_320 % 4)
            {
                old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += (4 - (start_offset_320 % 4))/2;
                old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += (4 - (start_offset_320 % 4))/2;
                start_offset_320 += (4 - (start_offset_320 % 4));
            }
        }
        /*scatter default 320 size not enough, move 640 down*/
        if ((start_offset_640 - start_offset_320) < cur_width_320)
        {
            start_offset_640 = (start_offset_320 + cur_width_320);
            if (start_offset_640 % 8)
            {
                old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += (4 - (start_offset_640 % 8))/4;
                old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += (4 - (start_offset_640 % 8))/4;
                start_offset_640 += (8 - (start_offset_640 % 8));
            }
        }
        /*scatter default 640 size not enough, move up*/
        if ((old_fpab->entry_count - start_offset_640) < cur_width_640)
        {
            start_offset_640 = (old_fpab->entry_count - cur_width_640);
        }
        /*scatter default 320 size not enough, move up*/
        if ((start_offset_640 - start_offset_320) < cur_width_320)
        {
            start_offset_320 = (start_offset_640 - cur_width_320);
        }

        vary_160 = (int32)(start_offset_320 - start_offset_160)/2 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160];
        vary_320 = (int32)(start_offset_640 - start_offset_320)/4 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320];
        vary_640 = (int32)(old_fpab->entry_count - start_offset_640)/8 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640];

        shift_160 = (int32)start_offset_160 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_160];
        shift_320 = (int32)start_offset_320 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_320];
        shift_640 = (int32)start_offset_640 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_640];

        if ((shift_320 < 0) && (shift_640 >= 0))
        {
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift_160, vary_160);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift_640, vary_640);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift_320, vary_320);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;
        }
        else if ((shift_320 < 0) && (shift_640 < 0))
        {
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift_160, vary_160);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift_320, vary_320);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift_640, vary_640);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;
        }
        else if ((shift_320 >= 0) && (shift_640 >= 0))
        {
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift_640, vary_640);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift_320, vary_320);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift_160, vary_160);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;
        }
        else if ((shift_320 >= 0) && (shift_640 < 0))
        {
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift_320, vary_320);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift_160, vary_160);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;
            fpa_usw_scatter(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift_640, vary_640);
            old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
            old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
            old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_reorder_entry_down_to_up(uint8 lchip, sys_acl_league_t* p_sys_league)
{
    uint16 start_offset_160 = 0;
    uint16 start_offset_320 = 0;
    uint16 start_offset_640 = 0;
    int32 shift = 0;
    int32 vary = 0;
    int32 vary_160 = 0;
    int32 vary_320 = 0;
    int32 vary_640 = 0;
    ctc_fpa_block_t* old_fpab = NULL;
    sys_acl_block_t * pb = NULL;

    SYS_ACL_DBG_FUNC();

    pb = &(p_usw_acl_master[lchip]->block[p_sys_league->block_id]);
    old_fpab = &pb->fpab;

    if (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] != old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80])
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_160]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_160]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_320]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_640]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;

        vary = (int32)old_fpab->entry_count - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80];

        fpa_usw_increase(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_80, 0, vary);
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] += vary;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] += vary;
    }
    else
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_80] = 0;

        start_offset_640 = old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] + (old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640]*8);
        shift = (int32)start_offset_640 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_640];
        vary_640 = (0 - (int32)old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640]);
        fpa_usw_increase(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift, vary_640);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;

        start_offset_320 = start_offset_640 - (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320])*4;
        shift = (int32)start_offset_320 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_320];
        vary_320 = (0 - (int32)old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320]);
        fpa_usw_increase(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift, vary_320);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;

        start_offset_160 = 0;
        shift = start_offset_160 - old_fpab->start_offset[CTC_FPA_KEY_SIZE_160];
        vary_160 = ((int32)start_offset_320/2 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160]);
        fpa_usw_increase(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift, vary_160);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_reorder_entry_up_to_down(uint8 lchip, sys_acl_league_t* p_sys_league)
{
    uint16 start_offset_160 = 0;
    uint16 start_offset_320 = 0;
    uint16 start_offset_640 = 0;
    int32 shift_160 = 0;
    int32 shift_320 = 0;
    int32 shift_640 = 0;
    int32 vary = 0;
    int32 vary_160 = 0;
    int32 vary_320 = 0;
    int32 vary_640 = 0;
    ctc_fpa_block_t* old_fpab = NULL;
    sys_acl_block_t * pb = NULL;

    SYS_ACL_DBG_FUNC();

    pb = &(p_usw_acl_master[lchip]->block[p_sys_league->block_id]);
    old_fpab = &pb->fpab;

    if (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] != old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80])
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_160]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_160]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_320]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;

        old_fpab->start_offset[CTC_FPA_KEY_SIZE_640]    = old_fpab->entry_count;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640]  = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;

        vary = (int32)old_fpab->entry_count - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80];

        fpa_usw_decrease(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_80, 0, vary);
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] += vary;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] += vary;
    }
    else
    {
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_80] = 0;
        old_fpab->sub_rsv_count[CTC_FPA_KEY_SIZE_80] = 0;

        start_offset_160 = 0;
        start_offset_320 = start_offset_160 + (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160])*2;
        if (start_offset_320 % 4)
        {
            start_offset_320 += (4 - (start_offset_320 % 4));
        }

        start_offset_640 = start_offset_320 + (old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] - old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320])*4;

        if (start_offset_640 % 8)
        {
            start_offset_640 += (8 - (start_offset_640 % 8));
        }

        vary_160 = (int32)(start_offset_320 - start_offset_160)/2 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160];
        vary_320 = (int32)(start_offset_640 - start_offset_320)/4 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320];
        vary_640 = (int32)(old_fpab->entry_count - start_offset_640)/8 - (int32)old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640];

        shift_160 = (int32)start_offset_160 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_160];
        shift_320 = (int32)start_offset_320 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_320];
        shift_640 = (int32)start_offset_640 - (int32)old_fpab->start_offset[CTC_FPA_KEY_SIZE_640];

        fpa_usw_decrease(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_160, shift_160, vary_160);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_160] = start_offset_160;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_160] += vary_160;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_160] += vary_160;

        fpa_usw_decrease(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_320, shift_320, vary_320);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_320] = start_offset_320;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_320] += vary_320;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_320] += vary_320;

        fpa_usw_decrease(p_usw_acl_master[lchip]->fpa, &pb->fpab, CTC_FPA_KEY_SIZE_640, shift_640, vary_640);
        old_fpab->start_offset[CTC_FPA_KEY_SIZE_640] = start_offset_640;
        old_fpab->sub_entry_count[CTC_FPA_KEY_SIZE_640] += vary_640;
        old_fpab->sub_free_count[CTC_FPA_KEY_SIZE_640] += vary_640;
    }

    return CTC_E_NONE;
}

#define _ACL_WARMBOOT_
int32
_sys_usw_acl_wb_mapping_group(sys_wb_acl_group_t* p_wb_acl_group, sys_acl_group_t* p_acl_group, uint8 is_sync)
{
    if (is_sync)
    {
        p_wb_acl_group->group_id = p_acl_group->group_id;

        p_wb_acl_group->type = p_acl_group->group_info.type;
        p_wb_acl_group->block_id = p_acl_group->group_info.block_id;

        p_wb_acl_group->dir = (p_acl_group->group_info.dir == CTC_INGRESS)? 0:1;
        p_wb_acl_group->bitmap_status = p_acl_group->group_info.bitmap_status;

        sal_memcpy(&p_wb_acl_group->un, &p_acl_group->group_info.un, sizeof(p_acl_group->group_info.un));
    }
    else
    {
        p_acl_group->group_id = p_wb_acl_group->group_id;
        p_acl_group->group_info.type = p_wb_acl_group->type;
        p_acl_group->group_info.block_id = p_wb_acl_group->block_id;
        p_acl_group->group_info.dir = p_wb_acl_group->dir ? CTC_EGRESS : CTC_INGRESS;
        p_acl_group->group_info.bitmap_status = p_wb_acl_group->bitmap_status;
        sal_memcpy(&p_acl_group->group_info.un, &p_wb_acl_group->un, sizeof(ctc_port_bitmap_t));

        p_acl_group->entry_list = ctc_slist_new();
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_wb_sync_group(void* bucket_data, void* user_data)
{
    uint32 max_entry_cnt = 0;
    sys_acl_group_t* p_acl_group = NULL;
    ctc_wb_data_t* p_wb_data = NULL;
    sys_wb_acl_group_t* p_wb_acl_group = NULL;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_acl_group_t);
    p_acl_group = (sys_acl_group_t*)bucket_data;
    p_wb_data = (ctc_wb_data_t*)user_data;
    p_wb_acl_group = (sys_wb_acl_group_t*)p_wb_data->buffer + p_wb_data->valid_cnt;

    _sys_usw_acl_wb_mapping_group(p_wb_acl_group, p_acl_group, 1);

    if(++p_wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_acl_wb_vlan_build_tag_op(uint8 lchip,  uint8 in_op, uint8 in_mode, uint8* o_op)
{
    switch(in_op)
    {
        case 0:
            *o_op = CTC_ACL_VLAN_TAG_OP_NONE;
            break;
        case 1:
            if(in_mode == 1)
            {
                *o_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
            }
            else
            {
                *o_op = CTC_ACL_VLAN_TAG_OP_REP;
            }

            break;
        case 2:
            *o_op = CTC_ACL_VLAN_TAG_OP_ADD;
            break;
        case 3:
            *o_op = CTC_ACL_VLAN_TAG_OP_DEL;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_wb_unbuild_vlan_edit_info(uint8 lchip, void* p_ds_edit, sys_acl_vlan_edit_t* p_vlan_edit)
{
    uint8 op = 0;
    uint8 mo;

    op = GetDsAclVlanActionProfile(V, sTagAction_f, p_ds_edit);
    mo = GetDsAclVlanActionProfile(V, stagModifyMode_f, p_ds_edit);
    _sys_usw_acl_wb_vlan_build_tag_op(lchip, op, mo, &p_vlan_edit->stag_op);

    op = GetDsAclVlanActionProfile(V, cTagAction_f, p_ds_edit);
    mo = GetDsAclVlanActionProfile(V, ctagModifyMode_f, p_ds_edit);
    _sys_usw_acl_wb_vlan_build_tag_op(lchip, op, mo, &p_vlan_edit->ctag_op);

    p_vlan_edit->svid_sl = GetDsAclVlanActionProfile(V, sVlanIdAction_f, p_ds_edit);
    p_vlan_edit->scos_sl = GetDsAclVlanActionProfile(V, sCosAction_f, p_ds_edit);
    p_vlan_edit->scfi_sl = GetDsAclVlanActionProfile(V, sCfiAction_f, p_ds_edit);

    p_vlan_edit->cvid_sl = GetDsAclVlanActionProfile(V, cVlanIdAction_f, p_ds_edit);
    p_vlan_edit->ccos_sl = GetDsAclVlanActionProfile(V, cCosAction_f, p_ds_edit);
    p_vlan_edit->ccfi_sl = GetDsAclVlanActionProfile(V, cCfiAction_f, p_ds_edit);
    p_vlan_edit->tpid_index = GetDsAclVlanActionProfile(V, svlanTpidIndex_f, p_ds_edit);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_wb_mapping_entry(sys_wb_acl_entry_t* p_wb_acl_entry, sys_acl_entry_t* p_acl_entry, uint8 is_sync)
{
    uint8 lchip = 0;
    uint8 block_id = 0;
    uint32 hw_index = 0;
    sys_acl_group_t* p_acl_group = NULL;

    if(is_sync)
    {
        p_wb_acl_entry->entry_id = p_acl_entry->fpae.entry_id;
        p_wb_acl_entry->priority = p_acl_entry->fpae.priority;
        p_wb_acl_entry->group_id = p_acl_entry->group->group_id;
        p_wb_acl_entry->key_type = p_acl_entry->key_type;
        p_wb_acl_entry->flag = p_acl_entry->fpae.flag;
        p_wb_acl_entry->step = p_acl_entry->fpae.step;
        p_wb_acl_entry->offset_a = p_acl_entry->fpae.offset_a;
        p_wb_acl_entry->nexthop_id = p_acl_entry->nexthop_id;
        p_wb_acl_entry->copp_rate = p_acl_entry->copp_rate;
        p_wb_acl_entry->copp_ptr = p_acl_entry->copp_ptr;
        p_wb_acl_entry->ad_index = p_acl_entry->ad_index;
        p_wb_acl_entry->mode = p_acl_entry->mode;
        p_wb_acl_entry->rg_info.flag = p_acl_entry->rg_info.flag;
        sal_memcpy(p_wb_acl_entry->rg_info.range_id, p_acl_entry->rg_info.range_id, sizeof(uint8)*ACL_RANGE_TYPE_NUM);
        p_wb_acl_entry->rg_info.range_bitmap = p_acl_entry->rg_info.range_bitmap;
        p_wb_acl_entry->rg_info.range_bitmap_mask = p_acl_entry->rg_info.range_bitmap_mask;
        p_wb_acl_entry->action_flag = p_acl_entry->action_flag;

        p_wb_acl_entry->hash_valid = p_acl_entry->hash_valid;
        p_wb_acl_entry->key_exist = p_acl_entry->key_exist;
        p_wb_acl_entry->key_conflict = p_acl_entry->key_conflict;
        p_wb_acl_entry->u1_type = p_acl_entry->u1_type;
        p_wb_acl_entry->u2_type = p_acl_entry->u2_type;
        p_wb_acl_entry->u3_type = p_acl_entry->u3_type;
        p_wb_acl_entry->u4_type = p_acl_entry->u4_type;
        p_wb_acl_entry->u5_type = p_acl_entry->u5_type;
        p_wb_acl_entry->wlan_bmp = p_acl_entry->wlan_bmp;
        p_wb_acl_entry->macl3basic_key_cvlan_mode = p_acl_entry->macl3basic_key_cvlan_mode;
        p_wb_acl_entry->macl3basic_key_macda_mode = p_acl_entry->macl3basic_key_macda_mode;
        p_wb_acl_entry->igmp_snooping = p_acl_entry->igmp_snooping;
        p_wb_acl_entry->hash_sel_tcp_flags = p_acl_entry->hash_sel_tcp_flags;

        p_wb_acl_entry->l2_type = p_acl_entry->l2_type;
        p_wb_acl_entry->l3_type = p_acl_entry->l3_type;
        p_wb_acl_entry->l4_type = p_acl_entry->l4_type;
        p_wb_acl_entry->l4_user_type = p_acl_entry->l4_user_type;
        p_wb_acl_entry->key_port_mode = p_acl_entry->key_port_mode;
        p_wb_acl_entry->fwd_key_nsh_mode = p_acl_entry->fwd_key_nsh_mode;
        p_wb_acl_entry->key_l4_port_mode = p_acl_entry->key_l4_port_mode;
        p_wb_acl_entry->mac_key_vlan_mode = p_acl_entry->mac_key_vlan_mode;
        p_wb_acl_entry->ip_dscp_mode = p_acl_entry->ip_dscp_mode;
        p_wb_acl_entry->key_mergedata_mode = p_acl_entry->key_mergedata_mode;
        p_wb_acl_entry->copp_ext_key_ip_mode = p_acl_entry->copp_ext_key_ip_mode;

        p_wb_acl_entry->u1_bitmap = p_acl_entry->u1_bitmap & 0xFFFF;
        p_wb_acl_entry->u1_bitmap_high = p_acl_entry->u1_bitmap >> 16;
        p_wb_acl_entry->u2_bitmap = p_acl_entry->u2_bitmap & 0xFF;
        p_wb_acl_entry->u2_bitmap_high = p_acl_entry->u2_bitmap >> 8;
        p_wb_acl_entry->u3_bitmap = p_acl_entry->u3_bitmap;
        p_wb_acl_entry->u4_bitmap = p_acl_entry->u4_bitmap;
        p_wb_acl_entry->u5_bitmap = p_acl_entry->u5_bitmap & 0xFF;
        p_wb_acl_entry->u5_bitmap_high = p_acl_entry->u5_bitmap >> 8;

        p_wb_acl_entry->stats_id = p_acl_entry->stats_id;
        p_wb_acl_entry->policer_id = p_acl_entry->policer_id;
        p_wb_acl_entry->cos_index = p_acl_entry->cos_index;
        p_wb_acl_entry->action_type = p_acl_entry->action_type;
        sal_memcpy(p_wb_acl_entry->action_bmp, p_acl_entry->action_bmp, sizeof(uint32)*((CTC_ACL_FIELD_ACTION_NUM-1)/32+1));
        p_wb_acl_entry->ether_type= p_acl_entry->ether_type;
        p_wb_acl_entry->ether_type_index= p_acl_entry->ether_type_index;

        sal_memcpy(p_wb_acl_entry->key_bmp, p_acl_entry->key_bmp, sizeof(uint32)*((CTC_FIELD_KEY_NUM-1)/32+1));
        p_wb_acl_entry->minvid= p_acl_entry->minvid;
        p_wb_acl_entry->key_reason_id= p_acl_entry->key_reason_id;
        p_wb_acl_entry->key_dst_nhid= p_acl_entry->key_dst_nhid;
        p_wb_acl_entry->udf_id= p_acl_entry->udf_id & 0xFFFF;
        p_wb_acl_entry->udf_id_high= p_acl_entry->udf_id >> 16;
        p_wb_acl_entry->limit_rate= p_acl_entry->limit_rate;
        p_wb_acl_entry->hash_sel_id = p_acl_entry->hash_field_sel_id;
        p_wb_acl_entry->cpu_reason_id = p_acl_entry->cpu_reason_id;
        p_wb_acl_entry->real_step = p_acl_entry->fpae.real_step;
    }
    else
    {
        lchip = p_acl_entry->fpae.lchip;
        CTC_ERROR_RETURN(_sys_usw_acl_get_group_by_gid(lchip, p_wb_acl_entry->group_id, &p_acl_group));
        p_acl_entry->group = p_acl_group;
        p_acl_entry->head.next = NULL;

        p_acl_entry->buffer = NULL;
        /*fpa*/
        p_acl_entry->fpae.entry_id = p_wb_acl_entry->entry_id;
        p_acl_entry->fpae.priority = p_wb_acl_entry->priority;
        p_acl_entry->fpae.offset_a = p_wb_acl_entry->offset_a;
        p_acl_entry->fpae.flag = p_wb_acl_entry->flag;
        p_acl_entry->fpae.step = p_wb_acl_entry->step;
        p_acl_entry->fpae.real_step = p_wb_acl_entry->real_step;

        p_acl_entry->key_type = p_wb_acl_entry->key_type;
        p_acl_entry->mode = p_wb_acl_entry->mode;
        p_acl_entry->action_flag = p_wb_acl_entry->action_flag;
        p_acl_entry->ad_index = p_wb_acl_entry->ad_index;
        p_acl_entry->nexthop_id = p_wb_acl_entry->nexthop_id;
        p_acl_entry->copp_rate = p_wb_acl_entry->copp_rate;
        p_acl_entry->copp_ptr = p_wb_acl_entry->copp_ptr;
        p_acl_entry->rg_info.flag = p_wb_acl_entry->rg_info.flag;
        sal_memcpy(p_acl_entry->rg_info.range_id, p_wb_acl_entry->rg_info.range_id, sizeof(uint8)*ACL_RANGE_TYPE_NUM);
        p_acl_entry->rg_info.range_bitmap = p_wb_acl_entry->rg_info.range_bitmap;
        p_acl_entry->rg_info.range_bitmap_mask = p_wb_acl_entry->rg_info.range_bitmap_mask;

        p_acl_entry->hash_valid = p_wb_acl_entry->hash_valid;
        p_acl_entry->key_exist = p_wb_acl_entry->key_exist;
        p_acl_entry->key_conflict = p_wb_acl_entry->key_conflict;
        p_acl_entry->u1_type = p_wb_acl_entry->u1_type;
        p_acl_entry->u2_type = p_wb_acl_entry->u2_type;
        p_acl_entry->u3_type = p_wb_acl_entry->u3_type;
        p_acl_entry->u4_type = p_wb_acl_entry->u4_type;
        p_acl_entry->u5_type = p_wb_acl_entry->u5_type;
        p_acl_entry->wlan_bmp = p_wb_acl_entry->wlan_bmp;

        p_acl_entry->l2_type = p_wb_acl_entry->l2_type;
        p_acl_entry->l3_type = p_wb_acl_entry->l3_type;
        p_acl_entry->l4_type = p_wb_acl_entry->l4_type;
        p_acl_entry->l4_user_type = p_wb_acl_entry->l4_user_type;
        p_acl_entry->key_port_mode = p_wb_acl_entry->key_port_mode;
        p_acl_entry->fwd_key_nsh_mode = p_wb_acl_entry->fwd_key_nsh_mode;
        p_acl_entry->key_l4_port_mode = p_wb_acl_entry->key_l4_port_mode;
        p_acl_entry->mac_key_vlan_mode = p_wb_acl_entry->mac_key_vlan_mode;
        p_acl_entry->ip_dscp_mode = p_wb_acl_entry->ip_dscp_mode;
        p_acl_entry->key_mergedata_mode = p_wb_acl_entry->key_mergedata_mode;
        p_acl_entry->copp_ext_key_ip_mode = p_wb_acl_entry->copp_ext_key_ip_mode;

        p_acl_entry->u1_bitmap = p_wb_acl_entry->u1_bitmap | (p_wb_acl_entry->u1_bitmap_high << 16);
        p_acl_entry->u2_bitmap = p_wb_acl_entry->u2_bitmap | (p_wb_acl_entry->u2_bitmap_high << 8);
        p_acl_entry->u3_bitmap = p_wb_acl_entry->u3_bitmap;
        p_acl_entry->u4_bitmap = p_wb_acl_entry->u4_bitmap;
        p_acl_entry->u5_bitmap = p_wb_acl_entry->u5_bitmap | (p_wb_acl_entry->u5_bitmap_high << 8);

        p_acl_entry->stats_id = p_wb_acl_entry->stats_id;
        p_acl_entry->policer_id = p_wb_acl_entry->policer_id;
        p_acl_entry->cos_index = p_wb_acl_entry->cos_index;
        p_acl_entry->action_type = p_wb_acl_entry->action_type;
        sal_memcpy(p_acl_entry->action_bmp, p_wb_acl_entry->action_bmp, sizeof(uint32)*((CTC_ACL_FIELD_ACTION_NUM-1)/32+1));
        p_acl_entry->ether_type = p_wb_acl_entry->ether_type;
        p_acl_entry->ether_type_index = p_wb_acl_entry->ether_type_index;

        sal_memcpy(p_acl_entry->key_bmp, p_wb_acl_entry->key_bmp, sizeof(uint32)*((CTC_FIELD_KEY_NUM-1)/32+1));
        p_acl_entry->minvid= p_wb_acl_entry->minvid;
        p_acl_entry->key_reason_id= p_wb_acl_entry->key_reason_id;
        p_acl_entry->key_dst_nhid= p_wb_acl_entry->key_dst_nhid;
        p_acl_entry->udf_id= p_wb_acl_entry->udf_id_high << 16 | p_wb_acl_entry->udf_id;
        p_acl_entry->limit_rate= p_wb_acl_entry->limit_rate;
        p_acl_entry->igmp_snooping = p_wb_acl_entry->igmp_snooping;
        p_acl_entry->hash_sel_tcp_flags = p_wb_acl_entry->hash_sel_tcp_flags;
        p_acl_entry->hash_field_sel_id = p_wb_acl_entry->hash_sel_id;
        p_acl_entry->cpu_reason_id = p_wb_acl_entry->cpu_reason_id;
        p_acl_entry->macl3basic_key_cvlan_mode = p_wb_acl_entry->macl3basic_key_cvlan_mode;
        p_acl_entry->macl3basic_key_macda_mode = p_wb_acl_entry->macl3basic_key_macda_mode;

        /* hash spool*/
    /*     if(!ACL_ENTRY_IS_TCAM(p_acl_entry->key_type) && p_acl_entry->ad_index)*/
        if(!ACL_ENTRY_IS_TCAM(p_acl_entry->key_type))
        {
            uint32 cmd = 0;
            ds_t action;
            uint32* p_hash_action = NULL;
            uint8   sys_key_type = 0;

            sal_memset(&action, 0, sizeof(action));
            cmd = DRV_IOR(DsFlow_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_wb_acl_entry->ad_index, cmd, &action));

            action[ACL_HASH_AD_INDEX_OFFSET] = p_acl_entry->ad_index;
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->ad_spool, &action, NULL, &p_hash_action));
            p_acl_entry->hash_action = p_hash_action;

            CTC_ERROR_RETURN(sys_usw_acl_map_ctc_to_sys_hash_key_type(p_acl_entry->key_type,&sys_key_type));
            p_usw_acl_master[lchip]->hash_sel_profile_count[sys_key_type][p_acl_entry->hash_field_sel_id]++;
        }
        else
        {
            uint32   cmd = 0;
            uint32   key_id    = 0;
            uint32   act_id    = 0;
            uint32   ad_index  = 0;
            uint8    step      = 0;
            uint32   vlan_edit_profile_id = 0;
            ds_t     vlan_action;
            sys_acl_vlan_edit_t   sys_vlan_edit;
            sys_acl_vlan_edit_t*   p_sys_vlan_edit_get = NULL;
            _sys_usw_acl_get_table_id(lchip, p_acl_entry, &key_id, &act_id, &hw_index);
            if (p_acl_entry->key_type != CTC_ACL_KEY_INTERFACE)
            {
                cmd = DRV_IOR(act_id, DsAcl0Ingress_vlanActionProfileId_f);
                _sys_usw_acl_get_key_size(lchip, 1, p_acl_entry, &step);
                ad_index = SYS_ACL_MAP_DRV_AD_INDEX(hw_index, step);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &vlan_edit_profile_id));
            }

            if(vlan_edit_profile_id != 0)
            {
                sal_memset(&sys_vlan_edit, 0, sizeof(sys_vlan_edit));
                sal_memset(&vlan_action, 0, sizeof(vlan_action));

                cmd = DRV_IOR(DsAclVlanActionProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_edit_profile_id, cmd, &vlan_action));
                _sys_usw_acl_wb_unbuild_vlan_edit_info(lchip, &vlan_action, &sys_vlan_edit);
                sys_vlan_edit.profile_id = vlan_edit_profile_id;
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->vlan_edit_spool,
                                               &sys_vlan_edit, NULL, &p_sys_vlan_edit_get));
                p_acl_entry->vlan_edit_valid = 1;
                p_acl_entry->vlan_edit = p_sys_vlan_edit_get;
            }

            if(p_acl_entry->ether_type != 0)
            {
                CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, p_acl_entry->ether_type, 0, NULL, &p_acl_entry->ether_type_index));
            }

            block_id = p_acl_group->group_info.block_id;
            if(CTC_EGRESS == p_acl_group->group_info.dir)
            {
                block_id += p_usw_acl_master[lchip]->igs_block_num;
            }
            p_usw_acl_master[lchip]->block[block_id].fpab.entries[p_wb_acl_entry->offset_a] = &(p_acl_entry->fpae);

        }

        /* add to group */
        ctc_slist_add_head(p_acl_group->entry_list, &(p_acl_entry->head));
        (p_acl_group->entry_count)++;

        _sys_usw_acl_update_key_count(lchip, p_acl_entry, 1);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_wb_sync_key_field_range(void* bucket_data, void* user_data)
{
    sys_acl_entry_t* p_acl_entry = (sys_acl_entry_t*)bucket_data;
    sys_acl_field_range_wb_data_t* p_range_wb_data = (sys_acl_field_range_wb_data_t*)(((sys_acl_traverse_data_t*)user_data)->data2);
    uint8   range_id = 0;
    uint8   range_type = 0;
    uint8   range_type_idx = 0;

    if (NULL == p_acl_entry->buffer)
    {
        for (range_type_idx=ACL_RANGE_TYPE_NONE+1; range_type_idx<ACL_RANGE_TYPE_NUM; range_type_idx+=1)
        {
            if (!CTC_IS_BIT_SET(p_acl_entry->rg_info.flag, range_type_idx))
            {
                continue;
            }
            range_type = range_type_idx;
            range_id = p_acl_entry->rg_info.range_id[range_type];
            p_range_wb_data->range_id_ref[range_id]++;
            if (!CTC_IS_BIT_SET(p_range_wb_data->range_id_bitmap, range_id))
            {
                CTC_BIT_SET(p_range_wb_data->range_id_bitmap, range_id);
                p_range_wb_data->range_id_valid_cnt++;
            }
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "rang id:%u, ref:%u, valid cnt:%u, bitmap:0x%X\n", \
            range_id, p_range_wb_data->range_id_ref[range_id], p_range_wb_data->range_id_valid_cnt, p_range_wb_data->range_id_bitmap);
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_wb_sync_entry(void* bucket_data, void* user_data)
{
    uint32 max_entry_cnt = 0;
    ctc_wb_data_t* p_wb_data = NULL;
    sys_acl_entry_t* p_acl_entry = NULL;
    sys_wb_acl_entry_t* p_wb_acl_entry = NULL;
    sys_acl_block_t*     p_block = NULL;
    uint8               lchip = 0;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_acl_entry_t);
    p_wb_data = (ctc_wb_data_t*)(((sys_acl_traverse_data_t*)user_data)->data0);
    p_block = (sys_acl_block_t*)(((sys_acl_traverse_data_t*)user_data)->data1);
    lchip   = ((sys_acl_traverse_data_t*)user_data)->value0;

    p_acl_entry = (sys_acl_entry_t*)bucket_data;
    p_wb_acl_entry = (sys_wb_acl_entry_t*)p_wb_data->buffer + p_wb_data->valid_cnt;

    if (p_acl_entry->buffer)
    {
        /* uninstall entry not sync, but need to update fpa count for tcam entry*/
        if(ACL_ENTRY_IS_TCAM(p_acl_entry->key_type))
        {
            uint8 key_size = 0;
            uint8 step = 0;
            uint8 block_id = (CTC_EGRESS == p_acl_entry->group->group_info.dir) ? \
                (p_acl_entry->group->group_info.block_id+p_usw_acl_master[lchip]->igs_block_num):(p_acl_entry->group->group_info.block_id);

            key_size = _sys_usw_acl_get_key_size(lchip, 0, p_acl_entry, &step);

            (p_block[block_id].fpab.free_count) += step;
            (p_block[block_id].fpab.sub_free_count[key_size])++;
        }
        return CTC_E_NONE;
    }

    _sys_usw_acl_wb_mapping_entry(p_wb_acl_entry, p_acl_entry, 1);

    if (++p_wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }
    _sys_usw_acl_wb_sync_key_field_range(bucket_data, user_data);
    return CTC_E_NONE;
}

#define _ACL_INIT_
/* 10 combinations */
int32
_sys_usw_acl_create_rsv_group(uint8 lchip)
{
    ctc_acl_group_info_t hash_group;
    sal_memset(&hash_group, 0, sizeof(hash_group));

    hash_group.type = CTC_ACL_GROUP_TYPE_HASH;

    CTC_ERROR_RETURN(_sys_usw_acl_create_group(lchip, CTC_ACL_GROUP_ID_HASH_MAC, &hash_group));
    CTC_ERROR_RETURN(_sys_usw_acl_create_group(lchip, CTC_ACL_GROUP_ID_HASH_IPV4, &hash_group));
    CTC_ERROR_RETURN(_sys_usw_acl_create_group(lchip, CTC_ACL_GROUP_ID_HASH_IPV6, &hash_group));
    CTC_ERROR_RETURN(_sys_usw_acl_create_group(lchip, CTC_ACL_GROUP_ID_HASH_MPLS, &hash_group));
    CTC_ERROR_RETURN(_sys_usw_acl_create_group(lchip, CTC_ACL_GROUP_ID_HASH_L2_L3, &hash_group));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_init_vlan_edit_stag(uint8 lchip, sys_acl_vlan_edit_t* p_vlan_edit, uint16* p_prof_idx)
{
    uint8 stag_op = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for (stag_op = CTC_ACL_VLAN_TAG_OP_NONE; stag_op < CTC_ACL_VLAN_TAG_OP_MAX; stag_op++)
    {
        p_vlan_edit->stag_op = stag_op;

        switch (stag_op)
        {
        case CTC_ACL_VLAN_TAG_OP_REP:         /* template has no replace */
            continue;
        case CTC_ACL_VLAN_TAG_OP_REP_OR_ADD:  /* 4*/
        case CTC_ACL_VLAN_TAG_OP_ADD:         /* 4*/
        {
            p_vlan_edit->svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            p_vlan_edit->scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            p_vlan_edit->scos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            p_vlan_edit->scos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            p_vlan_edit->scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, p_vlan_edit, p_prof_idx));
        }
        break;
        case CTC_ACL_VLAN_TAG_OP_NONE:      /*1*/
        case CTC_ACL_VLAN_TAG_OP_DEL:       /*1*/
            p_vlan_edit->svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            p_vlan_edit->scos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, p_vlan_edit, p_prof_idx));
            break;
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_acl_init_vlan_edit(uint8 lchip)
{
    uint16              prof_idx = 0;
    uint8               ctag_op  = 0;

    sys_acl_vlan_edit_t vlan_edit;
    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));

    for (ctag_op = CTC_ACL_VLAN_TAG_OP_NONE; ctag_op < CTC_ACL_VLAN_TAG_OP_MAX; ctag_op++)
    {
        vlan_edit.ctag_op = ctag_op;

        switch (ctag_op)
        {
        case CTC_ACL_VLAN_TAG_OP_ADD:        /* template has no append ctag */
        case CTC_ACL_VLAN_TAG_OP_REP:        /* template has no replace */
            continue;
        case CTC_ACL_VLAN_TAG_OP_REP_OR_ADD: /* 4 */
        {
            vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_acl_init_vlan_edit_stag(lchip, &vlan_edit, &prof_idx));

            vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_init_vlan_edit_stag(lchip, &vlan_edit, &prof_idx));

            vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_init_vlan_edit_stag(lchip, &vlan_edit, &prof_idx));

            vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_acl_init_vlan_edit_stag(lchip, &vlan_edit, &prof_idx));
        }
        break;
        case CTC_ACL_VLAN_TAG_OP_NONE:    /* 1 */
        case CTC_ACL_VLAN_TAG_OP_DEL:     /* 1 */
            vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_init_vlan_edit_stag(lchip, &vlan_edit, &prof_idx));
            break;
        }
    }
    /* 6 * 10 = 60 */

    /*for swap*/
    vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
    vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;

    vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, &vlan_edit, &prof_idx));

    vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
    vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_NONE;
    CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, &vlan_edit, &prof_idx));

    vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
    vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
    vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_ALTERNATIVE;
    CTC_ERROR_RETURN(_sys_usw_acl_install_vlan_edit(lchip, &vlan_edit, &prof_idx));

    /* 60+3 = 63 */
    return CTC_E_NONE;
}

/*
 * init acl register
 */
int32
_sys_usw_acl_init_global_cfg(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg)
{
    sys_acl_register_t acl_register;

    CTC_PTR_VALID_CHECK(acl_global_cfg);

    sal_memset(&acl_register, 0, sizeof(acl_register));

    acl_register.ingress_service_acl_en = 0xff;
    acl_register.egress_service_acl_en  = 0x7;
    acl_register.brg_pkt_vlan_cid_high_prio = 1;

    CTC_ERROR_RETURN(_sys_usw_acl_set_register(lchip, &acl_register));

    /* save it. not just for internal usage. like merge */
    sal_memcpy(&(p_usw_acl_master[lchip]->acl_register), &acl_register, sizeof(acl_register));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_ofb_init(uint8 lchip)
{
    uint32               entry_num    = 0;

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsCategoryIdPairTcamKey_t, &entry_num));
    if (entry_num)
    {
        sys_usw_ofb_param_t ofb;

        /*last index is used for a special entry*/
        entry_num = MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) - 1;    /*mark: !!!!!!!!*/

        sal_memset(&ofb, 0, sizeof(ofb));
        ofb.size = entry_num/2;
        ofb.ofb_cb = _sys_usw_acl_tcam_cid_move_cb;
        ofb.multiple = 1;

        CTC_ERROR_RETURN(sys_usw_ofb_init(lchip, 2, entry_num, &p_usw_acl_master[lchip]->ofb_type_cid));

        CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 0, &ofb));
        ofb.size = entry_num - entry_num/2;
        CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 1, &ofb));
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_opf_init(uint8 lchip)
{
    uint32               entry_num    = 0;
    uint32               start_offset = 0;
    sys_usw_opf_t opf;

    /*vlan-edit opf*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsAclVlanActionProfile_t, &entry_num));
    start_offset = SYS_ACL_VLAN_ACTION_RESERVE_NUM;
    entry_num   -= SYS_ACL_VLAN_ACTION_RESERVE_NUM;

    if (entry_num)
    {
        CTC_ERROR_RETURN(
            sys_usw_opf_init(lchip, &p_usw_acl_master[lchip]->opf_type_vlan_edit, 1, "opf-acl-vlan-action"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_acl_master[lchip]->opf_type_vlan_edit;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));
    }

    /*hash-ad opf*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    entry_num = 0;
    start_offset = 0;

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFlow_t, &entry_num));
    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_acl_master[lchip]->opf_type_ad, 1, "opf-acl-ad"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_acl_master[lchip]->opf_type_ad;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));
    }

    /*category-id hash left/right ad opf*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    entry_num = 0;
    start_offset = 0;

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsCategoryIdPairHashLeftAd_t, &entry_num));
    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_acl_master[lchip]->opf_type_cid_ad, 2, "opf-acl-cid-ad"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_acl_master[lchip]->opf_type_cid_ad;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));
        opf.pool_index = 1;
        opf.pool_type  = p_usw_acl_master[lchip]->opf_type_cid_ad;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));
    }

    /*udf cam opf*/
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_acl_master[lchip]->opf_type_udf, 1, "opf-acl-type-udf"));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_acl_master[lchip]->opf_type_udf;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_ACL_UDF_ENTRY_NUM)));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_spool_init(uint8 lchip)
{
    uint32 ad_entry_size = 0;
    uint32 vedit_entry_size = 0;
    uint32 cid_ad_num = 0;
    ctc_spool_t spool;

    sal_memset(&spool, 0, sizeof(spool));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFlow_t, &ad_entry_size));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsAclVlanActionProfile_t, &vedit_entry_size));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsCategoryIdPairHashLeftAd_t, &cid_ad_num));

    /*spool for flow hash action*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_ACL_USW_HASH_ACTION_BLOCK_SIZE;
    spool.max_count = ad_entry_size;
    spool.user_data_size = sizeof(ds_t);
    spool.spool_key = (hash_key_fn) _sys_usw_acl_hash_make_action;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_acl_hash_compare_action;
    spool.spool_alloc = (spool_alloc_fn) _sys_usw_acl_build_hash_ad_index;
    spool.spool_free = (spool_free_fn) _sys_usw_acl_free_hash_ad_index;
    p_usw_acl_master[lchip]->ad_spool = ctc_spool_create(&spool);

    /*spool for vlan edit profile*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = vedit_entry_size / MCHIP_CAP(SYS_CAP_ACL_VLAN_ACTION_SIZE_PER_BUCKET);
    spool.max_count = vedit_entry_size;
    spool.user_data_size = sizeof(sys_acl_vlan_edit_t);
    spool.spool_key = (hash_key_fn) _sys_usw_acl_hash_make_vlan_edit;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_acl_hash_compare_vlan_edit;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_acl_build_vlan_edit_index;
    spool.spool_free = (spool_free_fn)_sys_usw_acl_free_vlan_edit_index;
    p_usw_acl_master[lchip]->vlan_edit_spool = ctc_spool_create(&spool);

    /*spool for cid hash action*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = cid_ad_num;
    spool.max_count = cid_ad_num * 2;
    spool.user_data_size = sizeof(sys_acl_cid_action_t);
    spool.spool_key = (hash_key_fn) _sys_usw_acl_hash_make_cid_ad;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_acl_hash_compare_cid_ad;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_acl_build_cid_ad_index;
    spool.spool_free = (spool_free_fn)_sys_usw_acl_free_cid_ad_index;
    p_usw_acl_master[lchip]->cid_spool = ctc_spool_create(&spool);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_init_league_bitmap(uint8 lchip)
{
    uint8 block1 = 0;
    uint8 block2 = 0;
    uint8 prio = 0;
    uint32 old_size = 0;
    uint32 new_size = 0;
    uint32               cmd = 0;
    uint32               bmp[ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM] = {0};
    sys_acl_league_t*    p_sys_league = NULL;
    sys_acl_league_t*    p_tmp_league = NULL;
    ctc_fpa_entry_t**    p_tmp_entries = NULL;

    p_sys_league = &(p_usw_acl_master[lchip]->league[0]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl0AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[0])));
    bmp[0] = bmp[0]<<1;
    p_sys_league->lkup_level_bitmap |= bmp[0];

    p_sys_league = &(p_usw_acl_master[lchip]->league[1]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl1AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[1])));
    bmp[1] = bmp[1]<<2;
    p_sys_league->lkup_level_bitmap |= bmp[1];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[2]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl2AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[2])));
    bmp[2] = bmp[2]<<3;
    p_sys_league->lkup_level_bitmap |= bmp[2];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[3]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl3AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[3])));
    bmp[3] = bmp[3]<<4;
    p_sys_league->lkup_level_bitmap |= bmp[3];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]|bmp[2]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[4]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl4AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[4])));
    bmp[4] = bmp[4]<<5;
    p_sys_league->lkup_level_bitmap |= bmp[4];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]|bmp[2]|bmp[3]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[5]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl5AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[5])));
    bmp[5] = bmp[5]<<6;
    p_sys_league->lkup_level_bitmap |= bmp[5];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]|bmp[2]|bmp[3]|bmp[4]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[6]);
    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_acl6AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[6])));
    bmp[6] = bmp[6]<<7;
    p_sys_league->lkup_level_bitmap |= bmp[6];
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]|bmp[2]|bmp[3]|bmp[4]|bmp[5]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[7]);
    p_sys_league->lkup_level_bitmap &= ~(bmp[0]|bmp[1]|bmp[2]|bmp[3]|bmp[4]|bmp[5]|bmp[6]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[8]);
    cmd = DRV_IOR(EpeAclQosCtl_t, EpeAclQosCtl_acl0AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[8])));
    bmp[8] = bmp[8]<<1;
    p_sys_league->lkup_level_bitmap |= bmp[8];

    p_sys_league = &(p_usw_acl_master[lchip]->league[9]);
    cmd = DRV_IOR(EpeAclQosCtl_t, EpeAclQosCtl_acl1AdMergeBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(bmp[9])));
    bmp[9] = bmp[9]<<2;
    p_sys_league->lkup_level_bitmap |= bmp[9];
    p_sys_league->lkup_level_bitmap &= ~(bmp[8]);

    p_sys_league = &(p_usw_acl_master[lchip]->league[10]);
    p_sys_league->lkup_level_bitmap &= ~(bmp[8]|bmp[9]);

    for (block1=0; block1<ACL_IGS_BLOCK_MAX_NUM+ACL_EGS_BLOCK_MAX_NUM; block1++)
    {
        p_sys_league = &(p_usw_acl_master[lchip]->league[block1]);

        for (prio=0; prio<ACL_IGS_BLOCK_MAX_NUM; prio++)
        {
            block2 = ((block1 < ACL_IGS_BLOCK_MAX_NUM) ? prio: prio + ACL_IGS_BLOCK_MAX_NUM);
            if (block2 == block1)
            {
                continue;
            }

            if (!CTC_IS_BIT_SET(p_sys_league->lkup_level_bitmap, prio))
            {
                continue;
            }

            p_tmp_league = &(p_usw_acl_master[lchip]->league[block2]);
            p_tmp_league->merged_to = ((block1 < ACL_IGS_BLOCK_MAX_NUM) ? block1: block1 - ACL_IGS_BLOCK_MAX_NUM);

            p_sys_league->lkup_level_start[prio] = p_usw_acl_master[lchip]->block[block1].fpab.entry_count;
            p_sys_league->lkup_level_count[prio] = p_usw_acl_master[lchip]->block[block2].fpab.entry_count;

            old_size = sizeof(sys_acl_entry_t*) * p_usw_acl_master[lchip]->block[block1].fpab.entry_count;
            p_usw_acl_master[lchip]->block[block1].fpab.entry_count += p_usw_acl_master[lchip]->block[block2].fpab.entry_count;
            p_usw_acl_master[lchip]->block[block1].fpab.free_count  += p_usw_acl_master[lchip]->block[block2].fpab.free_count;
            p_usw_acl_master[lchip]->block[block1].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] += p_usw_acl_master[lchip]->block[block2].fpab.entry_count/8;
            p_usw_acl_master[lchip]->block[block1].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640]  += p_usw_acl_master[lchip]->block[block2].fpab.entry_count/8;
            p_usw_acl_master[lchip]->block[block1].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;

            new_size = sizeof(sys_acl_entry_t*) * p_usw_acl_master[lchip]->block[block1].fpab.entry_count;
            MALLOC_ZERO(MEM_ACL_MODULE, p_tmp_entries, new_size);
            if (NULL == p_tmp_entries)
            {
                return CTC_E_NO_MEMORY;
            }

            sal_memcpy(p_tmp_entries, p_usw_acl_master[lchip]->block[block1].fpab.entries, old_size);
            mem_free(p_usw_acl_master[lchip]->block[block1].fpab.entries);
            p_usw_acl_master[lchip]->block[block1].fpab.entries = p_tmp_entries;

            sal_memset(&(p_usw_acl_master[lchip]->block[block2].fpab),0, sizeof(ctc_fpa_block_t)-sizeof(sys_acl_entry_t*));
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_install_build_key_fn(uint8 lchip)
{
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_MAC] = _sys_usw_acl_add_mackey160_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_IPV4] =  _sys_usw_acl_add_l3key160_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_MPLS] =    NULL;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_IPV6] =    _sys_usw_acl_add_ipv6key320_field;

    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_HASH_MAC] =   _sys_usw_acl_add_hash_mac_key_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_HASH_IPV4] =   _sys_usw_acl_add_hash_ipv4_key_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_HASH_L2_L3] =   _sys_usw_acl_add_hash_l2l3_key_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_HASH_MPLS] =   _sys_usw_acl_add_hash_mpls_key_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_HASH_IPV6] =  _sys_usw_acl_add_hash_ipv6_key_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_PBR_IPV4] =  NULL;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_PBR_IPV6] =  NULL;

    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_IPV4_EXT] = _sys_usw_acl_add_l3key320_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_MAC_IPV4] =   _sys_usw_acl_add_macl3key320_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_MAC_IPV4_EXT] =_sys_usw_acl_add_macl3key640_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_IPV6_EXT] =   _sys_usw_acl_add_ipv6key640_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_MAC_IPV6] =    _sys_usw_acl_add_macipv6key640_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_CID] =      _sys_usw_acl_add_cidkey160_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_INTERFACE] =    _sys_usw_acl_add_shortkey80_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_FWD] =      _sys_usw_acl_add_forwardkey320_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_FWD_EXT] =     _sys_usw_acl_add_forwardkey640_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_COPP] =      _sys_usw_acl_add_coppkey320_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_COPP_EXT] =   _sys_usw_acl_add_coppkey640_field;
    p_usw_acl_master[lchip]->build_key_func[CTC_ACL_KEY_UDF] =   _sys_usw_acl_add_udfkey320_field;
    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_action_fn(uint8 lchip)
{
    p_usw_acl_master[lchip]->get_ad_func[SYS_ACL_ACTION_TCAM_INGRESS] = _sys_usw_acl_get_field_action_tcam_igs;
    p_usw_acl_master[lchip]->get_ad_func[SYS_ACL_ACTION_TCAM_EGRESS] = _sys_usw_acl_get_field_action_tcam_igs;
    p_usw_acl_master[lchip]->get_ad_func[SYS_ACL_ACTION_HASH_INGRESS] = _sys_usw_acl_get_field_action_hash_igs;

    return CTC_E_NONE;
}
int32 sys_usw_acl_set_sort_mode(uint8 lchip)
{
    uint8 block_id = 0;
    LCHIP_CHECK(lchip);
    SYS_ACL_INIT_CHECK();
    if(p_usw_acl_master[lchip]->sort_mode)
    {
        return CTC_E_NONE;
    }
    p_usw_acl_master[lchip]->sort_mode = 1;
    for (block_id = 0; block_id < p_usw_acl_master[lchip]->igs_block_num+p_usw_acl_master[lchip]->egs_block_num; block_id++)
    {
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] = p_usw_acl_master[lchip]->block[block_id].fpab.entry_count;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_80] = p_usw_acl_master[lchip]->block[block_id].fpab.entry_count;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160] = 0;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320] = 0;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
        p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
    }
    return CTC_E_NONE;
}
#define _ACL_DUMP_INFO_
int32
sys_usw_acl_show_field_range(uint8 lchip)
{
    uint8 show_num = 1;
    uint8 index = 0;
    char* str_type[] = {"None", "Pkt-len", "L4-src-port", "L4-dst-port"};
    sys_acl_field_range_t* p_range = NULL;

    LCHIP_CHECK(lchip);
    SYS_ACL_INIT_CHECK();
    SYS_USW_ACL_LOCK(lchip);

    p_range = &(p_usw_acl_master[lchip]->field_range);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Field-Range   Total: %-4u Used: %-4u\n", SYS_ACL_FIELD_RANGE_NUM,
                                                        SYS_ACL_FIELD_RANGE_NUM - p_range->free_num);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s%-12s%-10s%-8s%-8s%s\n", "No.", "TYPE", "RANGE_ID", "MIN", "MAX", "REF_CNT");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------\n");

    for (index = 0; index < SYS_ACL_FIELD_RANGE_NUM; index++)
    {
        if (p_range->range[index].range_type != ACL_RANGE_TYPE_NONE)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10u%-12s%-10u%-8u%-8u%u\n",
                    show_num,
                    str_type[p_range->range[index].range_type],
                    index,
                    p_range->range[index].min, p_range->range[index].max, p_range->range[index].ref);
            show_num++;
        }
    }
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_acl_hash_traverse_cb_get_entry_cnt(sys_acl_entry_t* pe, uint16* p_cb_para)
{
    if(pe)
    {
        p_cb_para[pe->key_type]++;
    }
    return CTC_E_NONE;
}

int32
sys_usw_acl_show_status(uint8 lchip)
{
    uint8          idx;
    uint8          block_id  = 0;
    uint8          key_used_bmp = 0;
    uint16         need_continue = 0;

    uint16        per_type_cnt[CTC_ACL_KEY_NUM] = {0};
    char*  ing_acl_type[8] = {"80 bit(IGS)", "160bit(IGS)", "320bit(IGS)", "640bit(IGS)"};
    char*  egr_acl_type[8] = {"80 bit(EGS)", "160bit(EGS)", "320bit(EGS)", "640bit(EGS)"};
    char*  tcam_prio[11] = {"IGS 0","IGS 1","IGS 2", "IGS 3", "IGS 4", "IGS 5", "IGS 6", "IGS 7", "EGS 0","EGS 1","EGS 2"};
    _acl_cb_para_t para;
    sys_acl_block_t* pb = NULL;

    SYS_ACL_INIT_CHECK();

    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n================= ACL Overall Status ==================\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#0 Group Status\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------****Inner Acl Group****--------------- \n");
    sal_memset(&para, 0, sizeof(_acl_cb_para_t));
    para.count = 0;
    ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                              (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_inner_gid, &para);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Total group count :%u \n", 5);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------****Outer Acl Group****--------------- \n");
    sal_memset(&para, 0, sizeof(_acl_cb_para_t));
    para.count = 0;
    ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                              (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_outer_gid, &para);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Total group count :%u \n", para.count);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#1 Priority Status \n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------------------------------------------------------------------------------------------------------------\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s\n",
                        "Type\\Prio","Prio 0","Prio 1","Prio 2","Prio 3","Prio 4","Prio 5","Prio 6","Prio 7");


    for (idx = 0; idx < CTC_FPA_KEY_SIZE_NUM; idx++)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s", ing_acl_type[idx]);
        for (block_id = 0; block_id < p_usw_acl_master[lchip]->igs_block_num; block_id++)
        {
            if (CTC_IS_BIT_SET(need_continue, block_id))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d", 0, 0);
                continue;
            }
            pb = &p_usw_acl_master[lchip]->block[block_id];
            _sys_usw_acl_get_key_size_bitmap(lchip, pb, &key_used_bmp);
            if (CTC_IS_BIT_SET(key_used_bmp, CTC_FPA_KEY_SIZE_80))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             (pb->fpab.entry_count - pb->fpab.free_count),
                             pb->fpab.entry_count);
                CTC_BIT_SET(need_continue, block_id);
            }
            else if(p_usw_acl_master[lchip]->sort_mode)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             idx? (pb->fpab.entry_count - pb->fpab.free_count)>>1: 0,
                             idx? pb->fpab.entry_count>>1: 0);
                idx? CTC_BIT_SET(need_continue, block_id): CTC_BIT_UNSET(need_continue, block_id);
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             (pb->fpab.sub_entry_count[idx] - pb->fpab.sub_free_count[idx]),
                             pb->fpab.sub_entry_count[idx]);

            }
        }
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    }
    for (idx = 0; idx < CTC_FPA_KEY_SIZE_NUM; idx++)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s", egr_acl_type[idx]);
        for (block_id = p_usw_acl_master[lchip]->igs_block_num; block_id < p_usw_acl_master[lchip]->igs_block_num+p_usw_acl_master[lchip]->egs_block_num; block_id++)
        {
            if (CTC_IS_BIT_SET(need_continue, block_id))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d", 0, 0);
                continue;
            }
            pb = &p_usw_acl_master[lchip]->block[block_id];
            _sys_usw_acl_get_key_size_bitmap(lchip, pb, &key_used_bmp);
            if (CTC_IS_BIT_SET(key_used_bmp, CTC_FPA_KEY_SIZE_80))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             (pb->fpab.entry_count - pb->fpab.free_count),
                             pb->fpab.entry_count);
                CTC_BIT_SET(need_continue, block_id);
            }
            else if(p_usw_acl_master[lchip]->sort_mode)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             idx? (pb->fpab.entry_count - pb->fpab.free_count)>>1: 0,
                             idx? pb->fpab.entry_count>>1: 0);
                idx? CTC_BIT_SET(need_continue, block_id): CTC_BIT_UNSET(need_continue, block_id);
            }
            else
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%5d of %-6d",
                             (p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[idx] - p_usw_acl_master[lchip]->block[block_id].fpab.sub_free_count[idx]),
                             p_usw_acl_master[lchip]->block[block_id].fpab.sub_entry_count[idx]);
            }
        }
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    }

    ctc_hash_traverse_through(p_usw_acl_master[lchip]->entry,
                                  (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_get_entry_cnt, per_type_cnt);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#2 Tcam Entry Status :\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Mac",  per_type_cnt[CTC_ACL_KEY_MAC]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Ipv4", per_type_cnt[CTC_ACL_KEY_IPV4]+per_type_cnt[CTC_ACL_KEY_IPV4_EXT]+per_type_cnt[CTC_ACL_KEY_MAC_IPV4]+per_type_cnt[CTC_ACL_KEY_MAC_IPV4_EXT]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Ipv6", per_type_cnt[CTC_ACL_KEY_IPV6]+per_type_cnt[CTC_ACL_KEY_IPV6_EXT]+per_type_cnt[CTC_ACL_KEY_MAC_IPV6]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Cid",  per_type_cnt[CTC_ACL_KEY_CID]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Intf", per_type_cnt[CTC_ACL_KEY_INTERFACE]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Fwd",  per_type_cnt[CTC_ACL_KEY_FWD]+per_type_cnt[CTC_ACL_KEY_FWD_EXT]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Copp", per_type_cnt[CTC_ACL_KEY_COPP]+per_type_cnt[CTC_ACL_KEY_COPP_EXT]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Udf", per_type_cnt[CTC_ACL_KEY_UDF]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#3 Hash Entry Status :\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Hash-Mac",  per_type_cnt[CTC_ACL_KEY_HASH_MAC]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Hash-Ipv4", per_type_cnt[CTC_ACL_KEY_HASH_IPV4]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Hash-Ipv6", per_type_cnt[CTC_ACL_KEY_HASH_IPV6]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Hash-MPLS", per_type_cnt[CTC_ACL_KEY_HASH_MPLS]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s entries: %u \n", "Hash-L2L3", per_type_cnt[CTC_ACL_KEY_HASH_L2_L3]);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#4 Spool Status :\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------\n");
    SYS_USW_ACL_ERROR_RETURN_UNLOCK( _sys_usw_acl_show_spool_status(lchip));
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#5 Tcam Resource Status :\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s  %-10s  %-10s  %-12s\n","Tcam Prio", "used_count", "free_count", "total_count");
    for (block_id = 0; block_id < p_usw_acl_master[lchip]->igs_block_num+p_usw_acl_master[lchip]->egs_block_num; block_id++)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s  %-10u  %-10u  %-12u\n",tcam_prio[block_id], \
                            p_usw_acl_master[lchip]->block[block_id].fpab.entry_count-p_usw_acl_master[lchip]->block[block_id].fpab.free_count,
            p_usw_acl_master[lchip]->block[block_id].fpab.free_count, p_usw_acl_master[lchip]->block[block_id].fpab.entry_count);
    }
    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_show_tcam_alloc_status(uint8 lchip, uint8 dir, uint8 block_id)
{
    char* dir_str[2] = {"Ingress", "Egress"};
    char* key_str[4] = {"80 bit", "160bit", "320bit", "640bit"};
    sys_acl_block_t *pb = NULL;
    uint8 fpa_block_id = 0;
    uint8 idx = 0;
    uint8 step = 1;

    LCHIP_CHECK(lchip);
    SYS_ACL_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(block_id, p_usw_acl_master[lchip]->igs_block_num - 1);

    fpa_block_id = block_id;
    if(CTC_EGRESS == dir)
    {
        CTC_MAX_VALUE_CHECK(block_id, p_usw_acl_master[lchip]->egs_block_num - 1);
        fpa_block_id += p_usw_acl_master[lchip]->igs_block_num;
    }

    SYS_USW_ACL_LOCK(lchip);

    pb = &(p_usw_acl_master[lchip]->block[fpa_block_id]);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\nblock id: %d, dir: %s\n", block_id, dir_str[dir]);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n(unit 80bit)  entry count: %d, used count: %d", pb->fpab.entry_count, pb->fpab.entry_count - pb->fpab.free_count);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n------------------------------------------------------------------\n");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-15s%-15s%-15s%-15s\n", "key size", "range", "entry count", "used count", "rsv count");

    if (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] != pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80])
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s[%4d,%4d]    %-15u%-15u%-15u\n", "80 bit", 0,
                                        pb->fpab.entry_count - 1,
                                        pb->fpab.entry_count, pb->fpab.entry_count - pb->fpab.free_count,
                                        pb->fpab.free_count);
    }
    else if (p_usw_acl_master[lchip]->sort_mode)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s[%4d,%4d]    %-15u%-15u%-15u\n", "160 bit", 0,
                                        pb->fpab.entry_count - 1,
                                        pb->fpab.entry_count>>1, (pb->fpab.entry_count - pb->fpab.free_count)>>1,
                                        pb->fpab.free_count>>1);
    }
    else
    {
        for(idx = 0; idx < CTC_FPA_KEY_SIZE_NUM; idx++)
        {
            if(pb->fpab.sub_entry_count[idx] > 0)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s[%4d,%4d]    %-15u%-15u%-15u\n", key_str[idx], pb->fpab.start_offset[idx],
                                                pb->fpab.start_offset[idx] + pb->fpab.sub_entry_count[idx] * step - 1,
                                                pb->fpab.sub_entry_count[idx], pb->fpab.sub_entry_count[idx] - pb->fpab.sub_free_count[idx],
                                                pb->fpab.sub_free_count[idx]);
            }
            step = step * 2;
        }
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"------------------------------------------------------------------\n");

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/** show acl entry **
 * type = 0 :by all
 * type = 1 :by entry
 * type = 2 :by group
 * type = 3 :by priority
 */
int32
sys_usw_acl_show_entry(uint8 lchip, uint8 type, uint32 param, uint8 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint8 grep_i = 0;
    SYS_ACL_INIT_CHECK();
    LCHIP_CHECK(lchip);
    /* CTC_ACL_KEY_NUM represents all type*/
    CTC_MAX_VALUE_CHECK(type, CTC_ACL_KEY_NUM);
    CTC_PTR_VALID_CHECK(p_grep);
    CTC_MAX_VALUE_CHECK(grep_cnt, 8);

    for (grep_i = 0; grep_i < grep_cnt; grep_i++)
    {
        if (CTC_FIELD_KEY_PORT == p_grep[grep_i].type || CTC_FIELD_KEY_AWARE_TUNNEL_INFO == p_grep[grep_i].type
            || CTC_FIELD_KEY_SVLAN_RANGE == p_grep[grep_i].type || CTC_FIELD_KEY_CVLAN_RANGE == p_grep[grep_i].type
            || CTC_FIELD_KEY_IP_PKT_LEN_RANGE == p_grep[grep_i].type || CTC_FIELD_KEY_L4_SRC_PORT_RANGE == p_grep[grep_i].type || CTC_FIELD_KEY_L4_DST_PORT_RANGE == p_grep[grep_i].type)
        {
            return CTC_E_NOT_SUPPORT;    /* not support grep CTC_FIELD_KEY_PORT or CTC_FIELD_KEY_AWARE_TUNNEL_INFO */
        }
    }

    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s%-12s%-12s%-4s%-12s%-8s%-18s%-8s%s\n", "No.", "ENTRY_ID", "GROUP_ID", "HW", "E_PRI", "G_PRI", "TYPE", "INDEX","ACTION");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------------------------------------------------------------------\n");

    switch (type)
    {
    case 0:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_show_entry_all(lchip, param, key_type, detail,type, p_grep, grep_cnt));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        break;

    case 1:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_show_entry_by_entry_id(lchip, param, key_type, detail,type, p_grep, grep_cnt));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        break;

    case 2:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_show_entry_by_group_id(lchip, param, key_type, detail,type, p_grep, grep_cnt));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        break;

    case 3:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_show_entry_by_priority(lchip, param, key_type, detail,type, p_grep, grep_cnt));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        break;

    case 4:
        SYS_USW_ACL_ERROR_RETURN_UNLOCK(_sys_usw_acl_show_entry_by_lkup_level(lchip, param, key_type, detail,type, p_grep, grep_cnt));
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        break;

    default:
        SYS_USW_ACL_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_show_entry_distribution(uint8 lchip, uint8 dir, uint8 priority)
{
    uint8 block_id = 0;
    uint8 step = 0;
    uint32 idx = 0;
    uint8 idx2 = 0;
    uint8 idx3 = 0;
    uint8 cur_key = 0;
    uint8 next_key = 0;
    uint8 cur_level = priority;
    uint8 next_level = priority;
    uint8 count = 0;
    uint8 print_head = FALSE;
    sys_acl_block_t* pb = NULL;
    sys_acl_league_t* p_sys_league = NULL;
    char* key_desc[CTC_FPA_KEY_SIZE_NUM] = {"K80 ","K160","K320","K640"};
    uint8 key_step[CTC_FPA_KEY_SIZE_NUM] = {1,2,4,8};
    char buf[3][64];

    SYS_ACL_INIT_CHECK();
    LCHIP_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(priority, (CTC_INGRESS == dir ? ACL_IGS_BLOCK_MAX_NUM-1: ACL_EGS_BLOCK_MAX_NUM-1));

    SYS_USW_ACL_LOCK(lchip);

    sal_memset(buf, 0, sizeof(buf));
    block_id = ((dir == CTC_INGRESS) ? priority: priority + ACL_IGS_BLOCK_MAX_NUM);
    pb = &p_usw_acl_master[lchip]->block[block_id];
    p_sys_league = &(p_usw_acl_master[lchip]->league[block_id]);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-14s%14s%14s%14s%14s%14s\n", "Key", "Total(I/C)", "Used(I/C)", "Free(I/C)", "StartIdx", "Step");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------------\n");
    if (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] != pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80])
    {
        idx = CTC_FPA_KEY_SIZE_80;
        sal_sprintf(buf[0], "%u/%u", pb->fpab.entry_count, pb->fpab.entry_count);
        sal_sprintf(buf[1], "%u/%u", (pb->fpab.entry_count - pb->fpab.free_count), (pb->fpab.entry_count - pb->fpab.free_count));
        sal_sprintf(buf[2], "%u/%u", pb->fpab.free_count, pb->fpab.free_count);

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-14s%14s%14s%14s%14u%14u\n",
            key_desc[idx], buf[0], buf[1], buf[2], 0, 1);
    }
    else if (p_usw_acl_master[lchip]->sort_mode)
    {
        idx = CTC_FPA_KEY_SIZE_160;
        sal_sprintf(buf[0], "%u/%u", pb->fpab.entry_count, pb->fpab.entry_count>>1);
        sal_sprintf(buf[1], "%u/%u", (pb->fpab.entry_count - pb->fpab.free_count), (pb->fpab.entry_count - pb->fpab.free_count)>>1);
        sal_sprintf(buf[2], "%u/%u", pb->fpab.free_count, pb->fpab.free_count>>1);

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-14s%14s%14s%14s%14u%14u\n",
            key_desc[idx], buf[0], buf[1], buf[2], 0, 2);
    }
    else
    {
        for (idx=CTC_FPA_KEY_SIZE_160; idx<CTC_FPA_KEY_SIZE_NUM; idx++)
        {
            sal_sprintf(buf[0], "%u/%u", pb->fpab.sub_entry_count[idx]*key_step[idx], pb->fpab.sub_entry_count[idx]);
            sal_sprintf(buf[1], "%u/%u", (pb->fpab.sub_entry_count[idx] - pb->fpab.sub_free_count[idx])*key_step[idx], pb->fpab.sub_entry_count[idx] - pb->fpab.sub_free_count[idx]);
            sal_sprintf(buf[2], "%u/%u", pb->fpab.sub_free_count[idx]*key_step[idx], pb->fpab.sub_free_count[idx]);

            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-14s%14s%14s%14s%14u%14u\n",
                key_desc[idx], buf[0], buf[1], buf[2], pb->fpab.start_offset[idx], key_step[idx]);
        }
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------------------------------------\n");

    step = 1;
    for (idx = 0; idx < pb->fpab.entry_count; idx = idx + step)
    {
        print_head = FALSE;

        if (idx == 0)
        {
            print_head = TRUE;
        }

        if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_640] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640]))
        {
            next_key = CTC_FPA_KEY_SIZE_640;
        }
        else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_320] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320]))
        {
            next_key = CTC_FPA_KEY_SIZE_320;
        }
        else if(idx >= pb->fpab.start_offset[CTC_FPA_KEY_SIZE_160] && (pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160]))
        {
            next_key = CTC_FPA_KEY_SIZE_160;
        }
        else /*80 bit key*/
        {
            next_key = CTC_FPA_KEY_SIZE_80;
        }

        if ((pb->fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] != pb->fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]))
        {
            next_key = CTC_FPA_KEY_SIZE_80;
        }
        else if (p_usw_acl_master[lchip]->sort_mode)
        {
            next_key = CTC_FPA_KEY_SIZE_160;
        }

        step = key_step[next_key];

        if (idx > 0)
        {
            for (idx2 = ACL_IGS_BLOCK_MAX_NUM-1; idx2 > cur_level; idx2--)
            {
                if ((p_sys_league->lkup_level_start[idx2] > 0) && (idx >= p_sys_league->lkup_level_start[idx2]))
                {
                    next_level = idx2;
                    break;
                }
            }
        }

        if ((cur_key != next_key) || (cur_level != next_level))
        {
            if (idx != 0)
            {
                for (idx3=count; idx3 < 64; idx3++)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," [%-2u]", count);
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
            }
            print_head = TRUE;
            count = 0;
            cur_key = next_key;
            cur_level = next_level;
        }
        else if (count >= 64)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," [%-2u]", count);
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
            print_head = TRUE;
            count = 0;
        }

        if (print_head)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"[%-4s lv%u %5u] ", key_desc[next_key], next_level, idx);
        }

        if (idx == 0)
        {
            cur_key = next_key;
            cur_level = next_level;
        }

        if (pb->fpab.entries[idx])
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"1");
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"0");
        }

        count++;

        if (idx + step >= pb->fpab.entry_count)
        {
            for (idx3=count; idx3 < 64; idx3++)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," ");
            }
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," [%-2u]", count);
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        }
    }

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/** show acl entry **
 * type = 0 :by all
 */
int32
sys_usw_acl_show_group(uint8 lchip, uint8 type)
{
    _acl_cb_para_t para;
    SYS_ACL_INIT_CHECK();

    SYS_USW_ACL_LOCK(lchip);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s%-12s%-6s%-6s%-14s%-10s%s\n", "No.", "GROUP_ID", "DIR", "PRI", "TYPE", "ENTRY_NUM", "KEY");
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------------------------------------------------\n");

    sal_memset(&para, 0, sizeof(_acl_cb_para_t));
    para.count = 0;
    ctc_hash_traverse_through(p_usw_acl_master[lchip]->group,
                              (hash_traversal_fn) _sys_usw_acl_hash_traverse_cb_show_group, &para);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Total group count :%u \n", para.count);

    SYS_USW_ACL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_acl_show_cid_pair(uint8 lchip, uint8 type)
{
    uint16  index = 0;
    uint16  num   = 0;
    uint32 cmd   = 0;
    uint32 ad_index = 0;
    uint8  dst_cid = 0;
    uint8  dst_cid_valid = 0;
    uint16 src_cid = 0;
    uint8  src_cid_valid = 0;
    ds_t  ds_key;
    ds_t  ds_mask;
    ds_t  ds_act;
    tbl_entry_t tcam_key;
    uint8 act_mode = 0;   /*0->over, 1->permit, 2->deny*/
    char* act_name[3] = {"Over Acl", "Permit", "Deny"};

    SYS_ACL_INIT_CHECK();

    SYS_USW_ACL_LOCK(lchip);

    sal_memset(&ds_key,   0, sizeof(ds_t));
    sal_memset(&ds_act,   0, sizeof(ds_t));
    tcam_key.data_entry = (uint32*)(&ds_key);
    tcam_key.mask_entry = (uint32*)(&ds_mask);

    if(0 == type || 2 == type)
    {
        num = 1;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\nHash Key1: DsCategoryIdPairHashLeftKey, Ad1: DsCategoryIdPairHashLeftAd\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.   src-cid     dst-cid     action      key-index   offset    act-index\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------------------\n");

        for(index = 0; index < MCHIP_CAP(SYS_CAP_ACL_HASH_CID_KEY); index++)
        {
            /*get key*/
            cmd = DRV_IOR(DsCategoryIdPairHashLeftKey_t, DRV_ENTRY_FLAG);
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_key));
            if(GetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, &ds_key))
            {
                src_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, &ds_key);
                dst_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, &ds_key);
                src_cid = GetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryId_f, &ds_key);
                dst_cid = GetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryId_f, &ds_key);
                ad_index = GetDsCategoryIdPairHashLeftKey(V, array_0_adIndex_f, &ds_key);
                /*get ad*/
                cmd = DRV_IOR(DsCategoryIdPairHashLeftAd_t, DRV_ENTRY_FLAG);
                SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, ad_index, cmd, &ds_act));
                act_mode = GetDsCategoryIdPairHashLeftAd(V, operationMode_f, &ds_act);
                if(act_mode)
                {
                    if(GetDsCategoryIdPairHashLeftAd(V, u1_g2_permit_f, &ds_act))
                    {
                        act_mode = 1;
                    }
                    else
                    {
                        act_mode = 2;
                    }
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", num);
                if(src_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", src_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                if(dst_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", dst_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-12u%-10u%-12u\n", act_name[act_mode], index, 0,ad_index);
                num++;
            }
            if(GetDsCategoryIdPairHashLeftKey(V, array_1_valid_f, &ds_key))
            {
                src_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_1_srcCategoryIdClassfied_f, &ds_key);
                dst_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_1_destCategoryIdClassfied_f, &ds_key);
                src_cid = GetDsCategoryIdPairHashLeftKey(V, array_1_srcCategoryId_f, &ds_key);
                dst_cid = GetDsCategoryIdPairHashLeftKey(V, array_1_destCategoryId_f, &ds_key);
                ad_index = GetDsCategoryIdPairHashLeftKey(V, array_1_adIndex_f, &ds_key);
                /*get ad*/
                cmd = DRV_IOR(DsCategoryIdPairHashLeftAd_t, DRV_ENTRY_FLAG);
                SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, ad_index, cmd, &ds_act));
                act_mode = GetDsCategoryIdPairHashLeftAd(V, operationMode_f, &ds_act);
                if(act_mode)
                {
                    if(GetDsCategoryIdPairHashLeftAd(V, u1_g2_permit_f, &ds_act))
                    {
                        act_mode = 1;
                    }
                    else
                    {
                        act_mode = 2;
                    }
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", num);
                if(src_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", src_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                if(dst_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", dst_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-12u%-10u%-12u\n", act_name[act_mode], index, 1, ad_index);
                num++;
            }
        }

        num = 1;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\nHash Key2: DsCategoryIdPairHashRightKey, Ad2: DsCategoryIdPairHashRightAd\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.   src-cid     dst-cid     action      key-index   offset    act-index\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------------------\n");
        for(index = 0; index < MCHIP_CAP(SYS_CAP_ACL_HASH_CID_KEY); index++)
        {
            /*get key*/
            cmd = DRV_IOR(DsCategoryIdPairHashRightKey_t, DRV_ENTRY_FLAG);
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_key));
            if(GetDsCategoryIdPairHashRightKey(V, array_0_valid_f, &ds_key))
            {
                src_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, &ds_key);
                dst_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, &ds_key);
                src_cid = GetDsCategoryIdPairHashRightKey(V, array_0_srcCategoryId_f, &ds_key);
                dst_cid = GetDsCategoryIdPairHashRightKey(V, array_0_destCategoryId_f, &ds_key);
                ad_index = GetDsCategoryIdPairHashRightKey(V, array_0_adIndex_f, &ds_key);
                /*get ad*/
                cmd = DRV_IOR(DsCategoryIdPairHashRightAd_t, DRV_ENTRY_FLAG);
                SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, ad_index, cmd, &ds_act));
                act_mode = GetDsCategoryIdPairHashRightAd(V, operationMode_f, &ds_act);
                if(act_mode)
                {
                    if(GetDsCategoryIdPairHashRightAd(V, u1_g2_permit_f, &ds_act))
                    {
                        act_mode = 1;
                    }
                    else
                    {
                        act_mode = 2;
                    }
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", num);
                if(src_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", src_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                if(dst_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", dst_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-12u%-10u%-12u\n", act_name[act_mode], index, 0, ad_index);
                num++;
            }
            if(GetDsCategoryIdPairHashRightKey(V, array_1_valid_f, &ds_key))
            {
                src_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_1_srcCategoryIdClassfied_f, &ds_key);
                dst_cid_valid = GetDsCategoryIdPairHashLeftKey(V, array_1_destCategoryIdClassfied_f, &ds_key);
                src_cid = GetDsCategoryIdPairHashRightKey(V, array_1_srcCategoryId_f, &ds_key);
                dst_cid = GetDsCategoryIdPairHashRightKey(V, array_1_destCategoryId_f, &ds_key);
                ad_index = GetDsCategoryIdPairHashRightKey(V, array_1_adIndex_f, &ds_key);
                /*get ad*/
                cmd = DRV_IOR(DsCategoryIdPairHashRightAd_t, DRV_ENTRY_FLAG);
                SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, ad_index, cmd, &ds_act));
                act_mode = GetDsCategoryIdPairHashRightAd(V, operationMode_f, &ds_act);
                if(act_mode)
                {
                    if(GetDsCategoryIdPairHashRightAd(V, u1_g2_permit_f, &ds_act))
                    {
                        act_mode = 1;
                    }
                    else
                    {
                        act_mode = 2;
                    }
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", num);
                if(src_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", src_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                if(dst_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", dst_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-12u%-10u%-12u\n", act_name[act_mode], index, 1, ad_index);
                num++;
            }
        }

    }
    if(1 == type || 2 == type)
    {
        num = 1;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\nTcam Key: DsCategoryIdPairTcamKey, Ad: DsCategoryIdPairTcamAd\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"No.   src-cid     dst-cid     action      key-index   act-index\n");
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"---------------------------------------------------------------\n");
        sal_memset(&ds_key, 0, sizeof(ds_t));
        sal_memset(&ds_act, 0, sizeof(ds_t));
        for(index = 0; index < MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR); index++)
        {
            /*get key*/
            cmd = DRV_IOR(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
            SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &tcam_key));
            src_cid_valid = GetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f, ds_key);
            dst_cid_valid = GetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, ds_key);
            src_cid = GetDsCategoryIdPairTcamKey(V, srcCategoryId_f, ds_key);
            dst_cid = GetDsCategoryIdPairTcamKey(V, destCategoryId_f, ds_key);
            if((src_cid_valid != 0) || (dst_cid_valid != 0))
            {
                /*get ad*/
                cmd = DRV_IOR(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
                SYS_USW_ACL_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_act));
                act_mode = GetDsCategoryIdPairTcamAd(V, operationMode_f, ds_act);
                if(act_mode)
                {
                    if(GetDsCategoryIdPairTcamAd(V, u1_g2_permit_f, &ds_act))
                    {
                        act_mode = 1;
                    }
                    else
                    {
                        act_mode = 2;
                    }
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u", num);
                if(src_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", src_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                if(dst_cid_valid)
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12u", dst_cid);
                }
                else
                {
                    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"*           ");
                }
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s%-12u%-12u\n", act_name[act_mode], index, index);
                num++;
            }
        }
    }
    SYS_USW_ACL_UNLOCK(lchip);
    return CTC_E_NONE;
}
#define _ACL_EXPAND_TCAM_

/*only used for ingress acl
level status is an array
0 : not use ext tcam
1 : use ext-tcam but there is no entry on ext-tcam
2 : use ext-tcam and there are entries on ext-tcam, but this entry can be move to other tcam
3 : can not move to other tcam

NOTE:
*/
#define SYS_USW_PRIVATE_TCAM_SIZE 1024
#define SYS_USW_EXT_TCAM_SIZE   2048
int32 _sys_usw_acl_get_ext_tcam_status(uint8 lchip, uint8* level_status)
{
    uint8 idx = 0;
    int8 key_size = 0;
    uint8 can_be_compress = 0;
    uint8 exist_entry_in_ext_tcam = 0;
    uint8 step = 0;
    uint16 tmp_offset = 0;
    uint32 size = 0;
    ctc_fpa_block_t* pb = NULL;

    sal_memset(level_status, 0, sizeof(uint8)*p_usw_acl_master[lchip]->igs_block_num);
    for(idx=0; idx < p_usw_acl_master[lchip]->igs_block_num; idx++)
    {
        pb = &(p_usw_acl_master[lchip]->block[idx].fpab);
        if(idx==0 || idx==1 ||idx==4 || idx==5)
        {
            /*special process for scl and acl share one tcam*/
            CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsScl2MacKey160_t+(idx%2), &size));
            size *= 2;

            if(size > SYS_USW_PRIVATE_TCAM_SIZE)
            {
                level_status[idx] = 3;
                continue;
            }
        }
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsAclQosKey80Ing0_t+idx, &size));
        if(size > SYS_USW_PRIVATE_TCAM_SIZE)
        {
            if(pb->entry_count == pb->free_count)
            {
                level_status[idx] = 1;
                continue;
            }

            can_be_compress = ((pb->entry_count-pb->free_count) <= SYS_USW_PRIVATE_TCAM_SIZE) ? 1:0;

            exist_entry_in_ext_tcam = 0;
            step = 8;
            for(key_size=CTC_FPA_KEY_SIZE_640; key_size >=0; key_size--)
            {
                tmp_offset = (pb->start_offset[key_size] > SYS_USW_PRIVATE_TCAM_SIZE) ? pb->start_offset[key_size] : SYS_USW_PRIVATE_TCAM_SIZE;

                for(;tmp_offset<(pb->sub_entry_count[key_size]*step+pb->start_offset[key_size]); tmp_offset+=step)
                {
                    if(pb->entries[tmp_offset])
                    {
                        exist_entry_in_ext_tcam = 1;
                        break;
                    }
                }
                if(exist_entry_in_ext_tcam || (pb->start_offset[key_size] <= SYS_USW_PRIVATE_TCAM_SIZE))
                {
                    break;
                }
                step = step/2;
            }
            level_status[idx] = (exist_entry_in_ext_tcam)?(can_be_compress?2:3):1;
        }
    }
    return CTC_E_NONE;
}

STATIC int32 _sys_usw_acl_do_adjust_ext_tcam(uint8 lchip, uint8 expand_blocks, uint8 compress_blocks, uint8* level_status)
{
    uint8 idx = 0;
    sys_acl_league_t* p_sys_league = NULL;
    ctc_fpa_block_t*  pb = NULL;
    uint16 tmp_count = 0;
    uint32 tmp_size = 0;
    ctc_fpa_entry_t** p_entries = NULL;

    for(idx=0; idx < p_usw_acl_master[lchip]->igs_block_num; idx++)
    {
        pb = &(p_usw_acl_master[lchip]->block[idx].fpab);
        p_sys_league = &(p_usw_acl_master[lchip]->league[idx]);
        tmp_size = pb->entry_count*sizeof(ctc_fpa_entry_t*);
        if(CTC_IS_BIT_SET(compress_blocks, idx))
        {
            if(level_status[idx] == 2)
            {
                _sys_usw_acl_reorder_entry_up_to_down(lchip, p_sys_league);
            }
            p_usw_acl_master[lchip]->block[idx].entry_num -= SYS_USW_EXT_TCAM_SIZE;
            p_sys_league->lkup_level_count[idx] -= SYS_USW_EXT_TCAM_SIZE;
            pb->entry_count -= SYS_USW_EXT_TCAM_SIZE;
            pb->free_count -= SYS_USW_EXT_TCAM_SIZE;

            if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] && pb->start_offset[CTC_FPA_KEY_SIZE_640] < SYS_USW_PRIVATE_TCAM_SIZE)
            {
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] -= SYS_USW_EXT_TCAM_SIZE/8;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_640] -= SYS_USW_EXT_TCAM_SIZE/8;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;
            }
            else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_320] && pb->start_offset[CTC_FPA_KEY_SIZE_320] < SYS_USW_PRIVATE_TCAM_SIZE)
            {
                pb->start_offset[CTC_FPA_KEY_SIZE_640] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

                tmp_count = pb->sub_entry_count[CTC_FPA_KEY_SIZE_320]-pb->sub_free_count[CTC_FPA_KEY_SIZE_320];
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_320] = (SYS_USW_PRIVATE_TCAM_SIZE-pb->start_offset[CTC_FPA_KEY_SIZE_320])/4;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_320] = pb->sub_entry_count[CTC_FPA_KEY_SIZE_320]-tmp_count;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;
            }
            else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_160] && pb->start_offset[CTC_FPA_KEY_SIZE_160] < SYS_USW_PRIVATE_TCAM_SIZE)
            {
                pb->start_offset[CTC_FPA_KEY_SIZE_640] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

                pb->start_offset[CTC_FPA_KEY_SIZE_320] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_320] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;

                tmp_count = pb->sub_entry_count[CTC_FPA_KEY_SIZE_160]-pb->sub_free_count[CTC_FPA_KEY_SIZE_160];
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_160] = (SYS_USW_PRIVATE_TCAM_SIZE-tmp_count)/2;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_160] = pb->sub_entry_count[CTC_FPA_KEY_SIZE_160]-tmp_count;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_160] = 0;
            }
            else if (pb->sub_entry_count[CTC_FPA_KEY_SIZE_80] && pb->start_offset[CTC_FPA_KEY_SIZE_80] < SYS_USW_PRIVATE_TCAM_SIZE)
            {
                pb->start_offset[CTC_FPA_KEY_SIZE_640] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_640] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;

                pb->start_offset[CTC_FPA_KEY_SIZE_320] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_320] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_320] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_320] = 0;

                pb->start_offset[CTC_FPA_KEY_SIZE_160] = SYS_USW_PRIVATE_TCAM_SIZE;
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_160] = 0;
                pb->sub_free_count[CTC_FPA_KEY_SIZE_160] = 0;
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_160] = 0;

                tmp_count = pb->sub_entry_count[CTC_FPA_KEY_SIZE_160]-pb->sub_free_count[CTC_FPA_KEY_SIZE_160];
                pb->sub_entry_count[CTC_FPA_KEY_SIZE_80] = (SYS_USW_PRIVATE_TCAM_SIZE-tmp_count);
                pb->sub_free_count[CTC_FPA_KEY_SIZE_80] = (pb->sub_entry_count[CTC_FPA_KEY_SIZE_80]-tmp_count);
                pb->sub_rsv_count[CTC_FPA_KEY_SIZE_80] = 0;
            }
        }
        else if(CTC_IS_BIT_SET(expand_blocks, idx))
        {
            p_usw_acl_master[lchip]->block[idx].entry_num += SYS_USW_EXT_TCAM_SIZE;
            p_sys_league->lkup_level_count[idx] += SYS_USW_EXT_TCAM_SIZE;
            pb->entry_count += SYS_USW_EXT_TCAM_SIZE;
            pb->free_count += SYS_USW_EXT_TCAM_SIZE;
            pb->sub_entry_count[CTC_FPA_KEY_SIZE_640] += SYS_USW_EXT_TCAM_SIZE/8;
            pb->sub_free_count[CTC_FPA_KEY_SIZE_640] += SYS_USW_EXT_TCAM_SIZE/8;
            pb->sub_rsv_count[CTC_FPA_KEY_SIZE_640] = 0;
        }

        if(CTC_IS_BIT_SET(compress_blocks, idx) || CTC_IS_BIT_SET(expand_blocks, idx))
        {
            p_entries = (ctc_fpa_entry_t**)mem_malloc(MEM_ACL_MODULE, pb->entry_count*sizeof(ctc_fpa_entry_t*));
            if(p_entries == NULL)
            {
                CTC_ERROR_RETURN(CTC_E_NO_MEMORY);
            }
            sal_memcpy(p_entries, pb->entries, (tmp_size>pb->entry_count*sizeof(ctc_fpa_entry_t*)) ? (pb->entry_count*sizeof(ctc_fpa_entry_t*)):tmp_size);
            mem_free(pb->entries);
            pb->entries = p_entries;
        }
    }
    return CTC_E_NONE;
}
int32 _sys_usw_acl_adjust_ext_tcam(uint8 lchip, uint16 ext_tcam_bitmap)
{
    uint8 idx;
    uint8 field_step = FlowTcamTcamSizeCfg_ipeAcl1Cfg80BitsLkupEn_f-FlowTcamTcamSizeCfg_ipeAcl0Cfg80BitsLkupEn_f;
    FlowTcamTcamSizeCfg_m flow_tcam_cfg;
    uint32 cmd = DRV_IOR(FlowTcamTcamSizeCfg_t, DRV_ENTRY_FLAG);
    uint8 level_status[8] = {0};
    uint8 share_level_map[8] = {4,5,6,7,0,1,2,3};
    uint8 compress_blocks = 0;
    uint8 expand_blocks = 0;
    int32  ret = 0;
    sys_acl_league_t* p_tmp_league;

    CTC_ERROR_RETURN(_sys_usw_acl_get_ext_tcam_status(lchip, (uint8*)level_status));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_tcam_cfg));

    for(idx=0; idx < p_usw_acl_master[lchip]->igs_block_num; idx++)
    {
        if(!CTC_IS_BIT_SET(ext_tcam_bitmap, idx))
        {
            continue;
        }
        p_tmp_league = &(p_usw_acl_master[lchip]->league[idx]);
        if (p_tmp_league->lkup_level_bitmap != (1<<idx))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"This block %u has been merged or be merged to others\n", idx);
            CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, error_end);
        }

        if(level_status[idx])
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR," This block %u has been expanded or the ext tcam used by scl\n", idx);
            CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, error_end);
        }

        if(CTC_IS_BIT_SET(ext_tcam_bitmap, share_level_map[idx]) || 3==level_status[share_level_map[idx]])
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR," This block associated ext tcam have been used by other block or expand the shared one tcam\n");
            CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, error_end);
        }

        CTC_BIT_SET(expand_blocks, idx);
        SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg80BitsLkupEn_f+field_step*idx, &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg160BitsLkupEn_f+field_step*idx, &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg320BitsLkupEn_f+field_step*idx, &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg640BitsLkupEn_f+field_step*idx, &flow_tcam_cfg, 0);

        SetFlowTcamTcamSizeCfg(V, share0Cfg80BitsLkupEn_f+field_step*(idx%4), &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, share0Cfg160BitsLkupEn_f+field_step*(idx%4), &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, share0Cfg320BitsLkupEn_f+field_step*(idx%4), &flow_tcam_cfg, 0);
        SetFlowTcamTcamSizeCfg(V, share0Cfg640BitsLkupEn_f+field_step*(idx%4), &flow_tcam_cfg, 0);

        if(level_status[share_level_map[idx]] > 0)
        {
            p_tmp_league = &(p_usw_acl_master[lchip]->league[share_level_map[idx]]);
            if (p_tmp_league->lkup_level_bitmap != (1<<share_level_map[idx]))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"The compressed block %u has been merged or be merged to others\n", share_level_map[idx]);
                CTC_ERROR_GOTO(CTC_E_INVALID_PARAM, ret, error_end);
            }
            CTC_BIT_SET(compress_blocks, share_level_map[idx]);
            SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg80BitsLkupEn_f+field_step*share_level_map[idx], &flow_tcam_cfg, 0);
            SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg160BitsLkupEn_f+field_step*share_level_map[idx], &flow_tcam_cfg, 0);
            SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg320BitsLkupEn_f+field_step*share_level_map[idx], &flow_tcam_cfg, 0);
            SetFlowTcamTcamSizeCfg(V, ipeAcl0Cfg640BitsLkupEn_f+field_step*share_level_map[idx], &flow_tcam_cfg, 0);
        }
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &flow_tcam_cfg), ret, error_end);

    }
    CTC_ERROR_GOTO(_sys_usw_acl_do_adjust_ext_tcam(lchip, expand_blocks, compress_blocks, (uint8*)level_status), ret, error_end);
    CTC_ERROR_GOTO(sys_usw_ftm_adjust_flow_tcam(lchip, expand_blocks, compress_blocks), ret, error_end);

error_end:
    cmd = DRV_IOW(FlowTcamTcamSizeCfg_t, DRV_ENTRY_FLAG);
    sal_memset(&flow_tcam_cfg, 0xFF, sizeof(flow_tcam_cfg));
    DRV_IOCTL(lchip, 0, cmd, &flow_tcam_cfg);
    return ret;
}
