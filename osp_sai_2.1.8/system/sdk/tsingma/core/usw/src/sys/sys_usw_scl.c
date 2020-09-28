/**
   @file sys_usw_scl.c


   @date 2017-01-24

   @version v5.0

   The file contains all scl APIs of sys layer

 */
/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"

#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_spool.h"
#include "ctc_const.h"
#include "ctc_vlan.h"
#include "ctc_qos.h"
#include "ctc_linklist.h"
#include "ctc_debug.h"


#include "sys_usw_fpa.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_scl.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"
/****************************************************************************
 *
 * Defines and Macros
 *
 *****************************************************************************/


/****************************************************************
*
* Global and Declaration
*
****************************************************************/


/*software hash size, determined by scl owner.*/
#define SYS_SCL_GROUP_HASH_SIZE                 1024
#define SYS_SCL_GROUP_HASH_BLOCK_SIZE           32
#define SYS_SCL_ENTRY_HASH_SIZE                 32768
#define SYS_SCL_ENTRY_HASH_BLOCK_SIZE           1024
#define SYS_SCL_ENTRY_BY_KEY_HASH_SIZE          32768
#define SYS_SCL_ENTRY_BY_KEY_HASH_BLOCK_SIZE    1024
#define SYS_SCL_TCAM_ENTRY_BY_KEY_HASH_SIZE     1024
#define SYS_SCL_VLAN_EDIT_SPOOL_BLOCK_SIZE      64
#define SYS_SCL_AD_SPOOL_BLOCK_SIZE             1024

typedef struct sys_scl_traverse_data_s
{
    void* data0;
    void* data1;
    int32 value0;
}sys_scl_traverse_data_t;

/****************************************************************
*
* Functions
*
****************************************************************/
extern int32 _sys_usw_scl_bind_nexthop(uint8 lchip, sys_scl_sw_entry_t* pe,uint32 nh_id);

sys_scl_master_t* p_usw_scl_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define __hash_and_spool_register_function__
STATIC uint32
_sys_usw_scl_group_hash_make(sys_scl_sw_group_t* pg)
{
    return pg->group_id;
}

STATIC bool
_sys_usw_scl_group_hash_compare(sys_scl_sw_group_t* pg0, sys_scl_sw_group_t* pg1)
{
    return(pg0->group_id == pg1->group_id);
}

STATIC uint32
_sys_usw_scl_entry_hash_make(sys_scl_sw_entry_t* pe)
{
    return pe->entry_id;
}

STATIC bool
_sys_usw_scl_entry_hash_compare(sys_scl_sw_entry_t* pe0, sys_scl_sw_entry_t* pe1)
{
    return(pe0->entry_id == pe1->entry_id);
}

STATIC uint32
_sys_usw_scl_tcam_entry_by_key_hash_make(sys_scl_tcam_entry_key_t* p_tcam_entry)
{
    uint32 size = 0;
    uint8  * k = NULL;

    CTC_PTR_VALID_CHECK(p_tcam_entry);

    size = sizeof(sys_scl_tcam_entry_key_t) - sizeof(sys_scl_sw_entry_t*);
    k    = (uint8 *) p_tcam_entry;

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_scl_tcam_entry_by_key_hash_compare(sys_scl_tcam_entry_key_t* p_tcam_entry0, sys_scl_tcam_entry_key_t* p_tcam_entry1)
{
    if (!p_tcam_entry0 || !p_tcam_entry1)
    {
        return FALSE;
    }

    if (sal_memcmp(p_tcam_entry0->key, p_tcam_entry1->key, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4)\
        || sal_memcmp(p_tcam_entry0->mask, p_tcam_entry1->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4) || p_tcam_entry0->scl_id != p_tcam_entry1->scl_id)
    {
        return FALSE;
    }

    return TRUE;
}


STATIC uint32
_sys_usw_scl_hash_key_entry_make(sys_scl_hash_key_entry_t* pe)
{
    uint8  hash_seeds[20] = {0};
    uint8  hash_seeds_size = 0 ;
    uint32 value = 0;
    sal_memcpy((uint8*)&hash_seeds[0],(uint8*)&pe->key_index,sizeof(uint32));
    hash_seeds[4] = pe->action_type;
    hash_seeds_size = 5;
    value = ctc_hash_caculate(hash_seeds_size, hash_seeds);
    return value;
}

STATIC bool
_sys_usw_scl_hash_key_entry_compare(sys_scl_hash_key_entry_t* pe0, sys_scl_hash_key_entry_t* pe1)
{
    if (!pe0 || !pe1)
    {
        return FALSE;
    }

    if ((pe0->action_type!= pe1->action_type)  || (pe0->key_index != pe1->key_index) || (pe0->scl_id != pe1->scl_id))
    {
        return FALSE;
    }

    return TRUE;
}

STATIC uint32
_sys_usw_scl_vlan_edit_spool_make(sys_scl_sp_vlan_edit_t* pv)
{
    uint32 size = 0;
    uint8* k = NULL;

    CTC_PTR_VALID_CHECK(pv);
    size = sizeof(pv->action_profile);
    k = (uint8*) pv;
    return ctc_hash_caculate(size, k);
}

/* if vlan edit in bucket equals */
STATIC bool
_sys_usw_scl_vlan_edit_spool_compare(sys_scl_sp_vlan_edit_t* pv0,
                                           sys_scl_sp_vlan_edit_t* pv1)
{
    uint32 size = 0;
    if (!pv0 || !pv1)
    {
        return FALSE;
    }
    size = sizeof(pv0->action_profile);
    if (!sal_memcmp(pv0, pv1, size))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_scl_vlan_edit_spool_alloc_index(sys_scl_sp_vlan_edit_t* pv, uint8* lchip)
{
    sys_usw_opf_t opf;
    uint32 value_32 = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_vlan_edit;

    if(CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*lchip, &opf, 1, pv->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*lchip, &opf, 1, &value_32));
        pv->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_vlan_edit_spool_free_index(sys_scl_sp_vlan_edit_t* pv, uint8* lchip)
{
    sys_usw_opf_t opf;
    uint32 index = pv->profile_id;
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_vlan_edit;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, index));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_scl_ad_spool_make(sys_scl_sw_hash_ad_t* pa)
{
    uint32 size = 0;
    uint8* k = NULL;

    CTC_PTR_VALID_CHECK(pa);
    size = CTC_OFFSET_OF(sys_scl_sw_hash_ad_t, calc_key_len);
    k = (uint8*) pa;
    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_scl_ad_spool_compare(sys_scl_sw_hash_ad_t* pAction0, sys_scl_sw_hash_ad_t* pAction1)
{
    uint32 size = 0;
    if (!pAction0 || !pAction1)
    {
        return FALSE;
    }

    size = CTC_OFFSET_OF(sys_scl_sw_hash_ad_t, calc_key_len);
    if (!sal_memcmp(pAction0, pAction1, size))
    {
        return TRUE;
    }

    return FALSE;
}

/* build index of HASH ad */
STATIC int32
_sys_usw_scl_ad_spool_alloc_index(sys_scl_sw_hash_ad_t* pAction, uint8* lchip)
{
    uint32  value_32 = 0;
    uint32  block_size = 0;
    uint8   dir = 0;
    uint8   multi = 0;
    uint32 tbl_id = 0;


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    block_size = pAction->is_half ? 1 : 2;            /* warmboot will not affect this value, because it only decide distribute from which position */
    multi = pAction->is_half ? 1 : 2;
    tbl_id = pAction->priority == 1? DsUserId1_t:DsUserId_t;

    if(!(CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(*lchip, tbl_id, dir, block_size, multi, &value_32));
        pAction->ad_index = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}


/* free index of HASH ad */
STATIC int32
_sys_usw_scl_ad_spool_free_index(sys_scl_sw_hash_ad_t* pAction, uint8* lchip)
{
    uint32  block_size = 0;
    uint8   dir = 0;
    uint32  index = pAction->ad_index;
    uint32 tbl_id = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    block_size = pAction->is_half ? 1 : 2;
    tbl_id = pAction->priority== 1? DsUserId1_t:DsUserId_t;

    CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(*lchip, tbl_id, dir, block_size, index));
    return CTC_E_NONE;
}


STATIC uint32
_sys_usw_scl_acl_profile_spool_make(sys_scl_action_acl_t* pv)
{
    uint32 size  = 0;
    uint8* k = NULL;

    CTC_PTR_VALID_CHECK(pv);
    size = CTC_OFFSET_OF(sys_scl_action_acl_t, calc_key_len);
    k = (uint8*) pv;

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_scl_acl_profile_spool_compare(sys_scl_action_acl_t* pv0,
                                           sys_scl_action_acl_t* pv1)
{
    uint32 size = 0;

    if (!pv0 || !pv1)
    {
        return FALSE;
    }

    size = CTC_OFFSET_OF(sys_scl_action_acl_t, calc_key_len);

    if (!sal_memcmp(pv0, pv1, size))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_scl_acl_profile_spool_alloc_index(sys_scl_action_acl_t* pv, uint8* lchip)
{
    sys_usw_opf_t opf;
    uint32 value_32 = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    if (pv->is_scl)
    {
        opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_flow_acl_profile;
    }
    else
    {
        opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_tunnel_acl_profile;
    }

    if(CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*lchip, &opf, 1, pv->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*lchip, &opf, 1, &value_32));
        pv->profile_id = value_32 & 0xFF;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_acl_profile_spool_free_index(sys_scl_action_acl_t* pv, uint8* lchip)
{
    sys_usw_opf_t opf;
    uint32 index = pv->profile_id;
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    if (pv->is_scl)
    {
        opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_flow_acl_profile;
    }
    else
    {
        opf.pool_type  = p_usw_scl_master[*lchip]->opf_type_tunnel_acl_profile;
    }

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, index));

    return CTC_E_NONE;
}


int32
_sys_usw_scl_free_entry_node_data(void* node_data, void* user_data)
{
    uint8 lchip = 0;
    sys_scl_sw_entry_t* pe = NULL;
    sys_scl_sw_group_t* pg = NULL;
    sys_scl_sw_block_t* pb = NULL;
    sys_scl_sw_tcam_entry_t* pe_tcam = NULL;

    if (user_data)
    {
        lchip= *(uint8*)user_data;
        pe = (sys_scl_sw_entry_t*)node_data;
        if (pe->temp_entry)
        {
            mem_free(pe->temp_entry);
        }
        if (SYS_SCL_ACTION_EGRESS == pe->action_type)
        {
            if (pe->temp_entry)
            {
                mem_free(pe->hash_ad);
            }
        }

        if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
        {
            /* remove from block */
            pg = pe->group;
            pb = &p_usw_scl_master[lchip]->block[pg->priority];
            pe_tcam = (sys_scl_sw_tcam_entry_t*) pb->fpab.entries[pe->key_index];
            if (pe_tcam)
            {
                mem_free(pe_tcam);
            }
        }
    }

    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_scl_free_group_node_data(void* node_data, void* user_data)
{
    sys_scl_sw_group_t* pg = NULL;

    pg = (sys_scl_sw_group_t*)node_data;
    if (pg->entry_list)
    {
        ctc_slist_free(pg->entry_list);
    }

    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_scl_free_tcam_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

#define __scl_fpa_init_function__

int32
_sys_usw_scl_init_fpa(uint8 lchip)
{
    uint8 loop = 0;
    uint32 offset = 0;
    uint32 entry_size = 0;
    uint32 scl_tcam[SCL_TCAM_NUM] = {0};

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsUserId0TcamKey80_t, &scl_tcam[0]));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsUserId1TcamKey80_t, &scl_tcam[1]));

    if (4 == MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM))
    {
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsScl2MacKey160_t, &scl_tcam[2]));
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsScl3MacKey160_t, &scl_tcam[3]));
        scl_tcam[2] = scl_tcam[2] * 2;
        scl_tcam[3] = scl_tcam[3] * 2;
    }

    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_VLAN]        = CTC_FPA_KEY_SIZE_320;

    /* default mac tcam key use 160 bit  */

    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_MAC]                     = CTC_FPA_KEY_SIZE_160;

    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV4_SINGLE]             = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV4]                    = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV6_SINGLE]             = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV6]                    = CTC_FPA_KEY_SIZE_640;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT]                    = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_CVLAN]              = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_SVLAN]              = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_2VLAN]              = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP]         = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_CVLAN_COS]          = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_SVLAN_COS]          = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_MAC]                     = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_MAC]                = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_IPV4]                    = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_SVLAN_MAC]               = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_SVLAN]                   = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_IPV4]               = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_IPV6]                    = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_PORT_IPV6]               = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_L2]                      = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4]             = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE]         = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF]         = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA]          = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV6]             = CTC_FPA_KEY_SIZE_640;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY]      = CTC_FPA_KEY_SIZE_640;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP]         = CTC_FPA_KEY_SIZE_640;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA]          = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_V4_MODE1]          = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_V4_MODE1]          = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1]       = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_IPV4]               = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_IPV6]               = CTC_FPA_KEY_SIZE_320;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_RMAC]               = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_RMAC_RID]           = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_STA_STATUS]         = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS]      = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD]       = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD]      = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TUNNEL_MPLS]             = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_ECID]                    = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_ING_ECID]                = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TRILL_UC]                = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TRILL_UC_RPF]            = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TRILL_MC]                = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TRILL_MC_RPF]            = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_HASH_TRILL_MC_ADJ]            = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_UDF]            = CTC_FPA_KEY_SIZE_160;
    p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_UDF_EXT]            = CTC_FPA_KEY_SIZE_320;

    p_usw_scl_master[lchip]->fpa = fpa_usw_create(_sys_usw_scl_get_block_by_pe_fpa,
                                                      _sys_usw_scl_entry_move_hw_fpa,
                                                      sizeof(ctc_slistnode_t));

    /* init each tcam block */
    for (loop = 0; loop < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); loop++)
    {
        p_usw_scl_master[lchip]->block[loop].fpab.entry_count = scl_tcam[loop];
        p_usw_scl_master[lchip]->block[loop].fpab.free_count  = scl_tcam[loop];
        offset = 0;
        p_usw_scl_master[lchip]->block[loop].fpab.start_offset[CTC_FPA_KEY_SIZE_80]    = offset;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_80] = 0;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_free_count[CTC_FPA_KEY_SIZE_80]  = 0;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_80]   = 0;
        offset = 0;
        p_usw_scl_master[lchip]->block[loop].fpab.start_offset[CTC_FPA_KEY_SIZE_160]    = offset;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_160] = scl_tcam[loop] / 4;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_free_count[CTC_FPA_KEY_SIZE_160]  = scl_tcam[loop] / 4;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_160]   = 0;
        offset += scl_tcam[loop] / 2;
        p_usw_scl_master[lchip]->block[loop].fpab.start_offset[CTC_FPA_KEY_SIZE_320]    = offset;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_320] = scl_tcam[loop]*3 / 32;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_free_count[CTC_FPA_KEY_SIZE_320]  = scl_tcam[loop]*3 / 32;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_320]  = 0;
        offset += scl_tcam[loop]*3 / 8;
        p_usw_scl_master[lchip]->block[loop].fpab.start_offset[CTC_FPA_KEY_SIZE_640]    = offset;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_entry_count[CTC_FPA_KEY_SIZE_640] = scl_tcam[loop] / 64;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_free_count[CTC_FPA_KEY_SIZE_640]  = scl_tcam[loop] / 64;
        p_usw_scl_master[lchip]->block[loop].fpab.sub_rsv_count[CTC_FPA_KEY_SIZE_640]  = 0;
        entry_size = sizeof(sys_scl_sw_entry_t*) * scl_tcam[loop];
        MALLOC_ZERO(MEM_SCL_MODULE, p_usw_scl_master[lchip]->block[loop].fpab.entries, entry_size);
        if (NULL == p_usw_scl_master[lchip]->block[loop].fpab.entries && entry_size)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

    }

    return CTC_E_NONE;
}

#define __scl_warmboot_function__

int32
_sys_usw_scl_wb_mapping_group(sys_wb_scl_group_t* p_wb_scl_group, sys_scl_sw_group_t* p_scl_group, uint8 is_sync)
{
    if (is_sync)
    {
        p_wb_scl_group->group_id = p_scl_group->group_id;
        p_wb_scl_group->priority = p_scl_group->priority;
        p_wb_scl_group->type     = p_scl_group->type;
        p_wb_scl_group->gport    = p_scl_group->gport;
    }
    else
    {
        p_scl_group->group_id = p_wb_scl_group->group_id;
        p_scl_group->type     = p_wb_scl_group->type;
        p_scl_group->priority = p_wb_scl_group->priority;
        p_scl_group->gport    = p_wb_scl_group->gport;
        p_scl_group->entry_list = ctc_slist_new();
        CTC_PTR_VALID_CHECK(p_scl_group->entry_list);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_scl_wb_sync_group_func(void* bucket_data, void* user_data)
{
    uint32 max_entry_num = 0;
    ctc_wb_data_t* p_wb_data = NULL;
    sys_scl_sw_group_t* p_scl_group = NULL;
    sys_wb_scl_group_t* p_wb_scl_group = NULL;

    max_entry_num = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_scl_group_t);
    p_scl_group = (sys_scl_sw_group_t*)bucket_data;
    p_wb_data = (ctc_wb_data_t*)user_data;

    p_wb_scl_group = (sys_wb_scl_group_t*)p_wb_data->buffer + p_wb_data->valid_cnt;
    sal_memset(p_wb_scl_group, 0, sizeof(sys_wb_scl_group_t));

    CTC_ERROR_RETURN(_sys_usw_scl_wb_mapping_group(p_wb_scl_group, p_scl_group, 1));
    if (++p_wb_data->valid_cnt == max_entry_num)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_wb_mapping_entry(sys_wb_scl_entry_t* p_wb_scl_entry, sys_scl_sw_entry_t* p_scl_entry, uint8 is_sync)
{
    uint8  lchip = 0;
    if (is_sync)
    {
        p_wb_scl_entry->entry_id             = p_scl_entry->entry_id;
        p_wb_scl_entry->lchip                = p_scl_entry->lchip;
        lchip                                = p_scl_entry->lchip;
        p_wb_scl_entry->direction            = p_scl_entry->direction;
        p_wb_scl_entry->key_type             = p_scl_entry->key_type;
        p_wb_scl_entry->action_type          = p_scl_entry->action_type;
        p_wb_scl_entry->resolve_conflict     = p_scl_entry->resolve_conflict;
        p_wb_scl_entry->is_half              = p_scl_entry->is_half;
        p_wb_scl_entry->rpf_check_en         = p_scl_entry->rpf_check_en;
        p_wb_scl_entry->uninstall            = p_scl_entry->uninstall;
        p_wb_scl_entry->hash_valid           = p_scl_entry->hash_valid;
        p_wb_scl_entry->key_exist            = p_scl_entry->key_exist;
        p_wb_scl_entry->key_conflict         = p_scl_entry->key_conflict;
        p_wb_scl_entry->u1_type              = p_scl_entry->u1_type;
        p_wb_scl_entry->u2_type              = p_scl_entry->u2_type;
        p_wb_scl_entry->u3_type              = p_scl_entry->u3_type;
        p_wb_scl_entry->u4_type              = p_scl_entry->u4_type;
        p_wb_scl_entry->u5_type              = p_scl_entry->u5_type;
        p_wb_scl_entry->is_service_policer   = p_scl_entry->is_service_policer;
        p_wb_scl_entry->bind_nh              = p_scl_entry->bind_nh;
        p_wb_scl_entry->vpws_oam_en          = p_scl_entry->vpws_oam_en;

        p_wb_scl_entry->l2_type              = p_scl_entry->l2_type;
        p_wb_scl_entry->l3_type              = p_scl_entry->l3_type;
        p_wb_scl_entry->l4_type              = p_scl_entry->l4_type;
        p_wb_scl_entry->l4_user_type         = p_scl_entry->l4_user_type;
        p_wb_scl_entry->mac_key_vlan_mode    = p_scl_entry->mac_key_vlan_mode;
        p_wb_scl_entry->mac_key_macda_mode   = p_scl_entry->mac_key_macda_mode;
        p_wb_scl_entry->key_l4_port_mode     = p_scl_entry->key_l4_port_mode;

        p_wb_scl_entry->u1_bitmap            = p_scl_entry->u1_bitmap & 0xFFFF;
        p_wb_scl_entry->u1_bitmap_high     = p_scl_entry->u1_bitmap >> 16;
        p_wb_scl_entry->u3_bitmap            = p_scl_entry->u3_bitmap;
        p_wb_scl_entry->u2_bitmap            = p_scl_entry->u2_bitmap & 0xFF;
        p_wb_scl_entry->u2_bitmap_high     = p_scl_entry->u2_bitmap >> 8;
        p_wb_scl_entry->u4_bitmap            = p_scl_entry->u4_bitmap;
        p_wb_scl_entry->u5_bitmap            = p_scl_entry->u5_bitmap & 0xFF;
        p_wb_scl_entry->u5_bitmap_high     = p_scl_entry->u5_bitmap >> 8;
        p_wb_scl_entry->hash_field_sel_id    = p_scl_entry->hash_field_sel_id;
        sal_memcpy(p_wb_scl_entry->hash_sel_bmp, p_scl_entry->hash_sel_bmp, sizeof(p_scl_entry->hash_sel_bmp));

        p_wb_scl_entry->nexthop_id           = p_scl_entry->nexthop_id;
        p_wb_scl_entry->stats_id             = p_scl_entry->stats_id;
        p_wb_scl_entry->key_index            = p_scl_entry->key_index;
        p_wb_scl_entry->ad_index             = p_scl_entry->ad_index;
        p_wb_scl_entry->policer_id           = p_scl_entry->policer_id;
        p_wb_scl_entry->vn_id                = p_scl_entry->vn_id;
        sal_memcpy(p_wb_scl_entry->action_bmp, p_scl_entry->action_bmp, sizeof(p_scl_entry->action_bmp));

        p_wb_scl_entry->group_id             = p_scl_entry->group->group_id;

        p_wb_scl_entry->vlan_edit_profile_id = p_scl_entry->vlan_edit ? p_scl_entry->vlan_edit->profile_id : 0xFFFF;

        p_wb_scl_entry->acl_profile_id = p_scl_entry->acl_profile ? p_scl_entry->acl_profile->profile_id : 0xFF;
        p_wb_scl_entry->is_scl         = p_scl_entry->acl_profile ? p_scl_entry->acl_profile->is_scl : 0;

        p_wb_scl_entry->ether_type             = p_scl_entry->ether_type;
        p_wb_scl_entry->ether_type_index             = p_scl_entry->ether_type_index;
        p_wb_scl_entry->service_id           = p_scl_entry->service_id;
        p_wb_scl_entry->cos_idx           = p_scl_entry->cos_idx;

        /* for tcam entry */
        if (SCL_ENTRY_IS_TCAM(p_scl_entry->key_type) || p_scl_entry->resolve_conflict)
        {
            p_wb_scl_entry->priority = p_usw_scl_master[lchip]->block[p_scl_entry->group->priority].fpab.entries[p_scl_entry->key_index]->priority;
        }

        if(p_scl_entry->acl_profile)
        {
            p_wb_scl_entry->acl_profile_valid = 1;
        }

        p_wb_scl_entry->key_bmp = p_scl_entry->key_bmp;
    }
    else
    {
        uint8  count_size = 0;
        uint32 cmd   = 0;
        uint32 key_id = 0;
        uint32 act_id = 0;
        uint32 entry_size = 0;
        sys_scl_sw_group_t* pg = NULL;
        sys_scl_sp_vlan_edit_t sp_vlan_edit;
        sys_scl_sp_vlan_edit_t* p_sp_vlan_edit = NULL;
        sys_scl_action_acl_t acl;
        sys_scl_action_acl_t* p_acl = NULL;
        sys_scl_sw_hash_ad_t hash_ad;
        sys_scl_sw_hash_ad_t* p_hash_ad = NULL;

        sal_memset(&sp_vlan_edit, 0, sizeof(sys_scl_sp_vlan_edit_t));
        sal_memset(&acl, 0, sizeof(sys_scl_action_acl_t));
        sal_memset(&hash_ad, 0, sizeof(sys_scl_sw_hash_ad_t));
        p_scl_entry->entry_id                = p_wb_scl_entry->entry_id;
        p_scl_entry->lchip                   = p_wb_scl_entry->lchip;
        lchip                                = p_wb_scl_entry->lchip;
        p_scl_entry->direction               = p_wb_scl_entry->direction;
        p_scl_entry->key_type                = p_wb_scl_entry->key_type;
        p_scl_entry->action_type             = p_wb_scl_entry->action_type;
        p_scl_entry->resolve_conflict        = p_wb_scl_entry->resolve_conflict;
        p_scl_entry->is_half                 = p_wb_scl_entry->is_half;
        p_scl_entry->rpf_check_en            = p_wb_scl_entry->rpf_check_en;
        p_scl_entry->uninstall               = p_wb_scl_entry->uninstall;
        p_scl_entry->hash_valid              = p_wb_scl_entry->hash_valid;
        p_scl_entry->key_exist               = p_wb_scl_entry->key_exist;
        p_scl_entry->key_conflict            = p_wb_scl_entry->key_conflict;
        p_scl_entry->u1_type                 = p_wb_scl_entry->u1_type;
        p_scl_entry->u2_type                 = p_wb_scl_entry->u2_type;
        p_scl_entry->u3_type                 = p_wb_scl_entry->u3_type;
        p_scl_entry->u4_type                 = p_wb_scl_entry->u4_type;
        p_scl_entry->u5_type                 = p_wb_scl_entry->u5_type;
        p_scl_entry->is_service_policer      = p_wb_scl_entry->is_service_policer;
        p_scl_entry->bind_nh                 = p_wb_scl_entry->bind_nh;
        p_scl_entry->vpws_oam_en             = p_wb_scl_entry->vpws_oam_en;

        p_scl_entry->l2_type                 = p_wb_scl_entry->l2_type;
        p_scl_entry->l3_type                 = p_wb_scl_entry->l3_type;
        p_scl_entry->l4_type                 = p_wb_scl_entry->l4_type;
        p_scl_entry->l4_user_type            = p_wb_scl_entry->l4_user_type;
        p_scl_entry->mac_key_vlan_mode       = p_wb_scl_entry->mac_key_vlan_mode;
        p_scl_entry->mac_key_macda_mode      = p_wb_scl_entry->mac_key_macda_mode;
        p_scl_entry->key_l4_port_mode        = p_wb_scl_entry->key_l4_port_mode;

        p_scl_entry->u1_bitmap               = p_wb_scl_entry->u1_bitmap | (p_wb_scl_entry->u1_bitmap_high << 16);
        p_scl_entry->u3_bitmap               = p_wb_scl_entry->u3_bitmap;
        p_scl_entry->u2_bitmap               = p_wb_scl_entry->u2_bitmap | (p_wb_scl_entry->u2_bitmap_high << 8);
        p_scl_entry->u4_bitmap               = p_wb_scl_entry->u4_bitmap;
        p_scl_entry->u5_bitmap               = p_wb_scl_entry->u5_bitmap | (p_wb_scl_entry->u5_bitmap_high << 8);
        p_scl_entry->hash_field_sel_id       = p_wb_scl_entry->hash_field_sel_id;
        sal_memcpy(p_scl_entry->hash_sel_bmp, p_wb_scl_entry->hash_sel_bmp, sizeof(p_scl_entry->hash_sel_bmp));

        p_scl_entry->ether_type             = p_wb_scl_entry->ether_type;
        p_scl_entry->ether_type_index             = p_wb_scl_entry->ether_type_index;

        p_scl_entry->nexthop_id              = p_wb_scl_entry->nexthop_id;
        p_scl_entry->stats_id                = p_wb_scl_entry->stats_id;
        p_scl_entry->key_index               = p_wb_scl_entry->key_index;
        p_scl_entry->ad_index                = p_wb_scl_entry->ad_index;
        p_scl_entry->policer_id              = p_wb_scl_entry->policer_id;
        p_scl_entry->vn_id                   = p_wb_scl_entry->vn_id;
        sal_memcpy(p_scl_entry->action_bmp, p_wb_scl_entry->action_bmp, sizeof(p_scl_entry->action_bmp));        /* this is array not pointer so do not need alloc memory */

        p_scl_entry->key_bmp = p_wb_scl_entry->key_bmp;

        p_scl_entry->temp_entry              = NULL;                            /* all entries must have been installed before quiting sdk, that is temp entry hash been mem_free */

        _sys_usw_scl_get_group_by_gid(lchip, p_wb_scl_entry->group_id, &pg);

        p_scl_entry->group                   = pg;
        p_scl_entry->service_id              = p_wb_scl_entry->service_id;
        p_scl_entry->cos_idx                 = p_wb_scl_entry->cos_idx;

        /* restore entry count by different action type, this count is used for spec limit */
        _sys_usw_scl_get_table_id(lchip, pg->priority, p_scl_entry, &key_id, &act_id);
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_size(lchip, key_id, &entry_size));
        count_size = entry_size / 12;
        if (!SCL_ENTRY_IS_TCAM(p_scl_entry->key_type) && !p_scl_entry->resolve_conflict)
        {
            p_usw_scl_master[lchip]->entry_count[p_scl_entry->action_type] += count_size;
        }

        /* restore hash field select count */
        if (SYS_SCL_KEY_HASH_L2 == p_scl_entry->key_type)
        {
            p_usw_scl_master[lchip]->hash_sel_profile_count[p_scl_entry->hash_field_sel_id] += 1;
        }

        if (p_wb_scl_entry->vlan_edit_profile_id != 0xFFFF)
        {
            cmd = DRV_IOR(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_wb_scl_entry->vlan_edit_profile_id, cmd, &sp_vlan_edit.action_profile));
            sp_vlan_edit.profile_id = p_wb_scl_entry->vlan_edit_profile_id;
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->vlan_edit_spool, &sp_vlan_edit, NULL, &p_sp_vlan_edit));
            p_scl_entry->vlan_edit = p_sp_vlan_edit;
        }

        /* 0 is reasonable */
        if (p_wb_scl_entry->acl_profile_valid)
        {
            if (p_wb_scl_entry->is_scl)
            {
                cmd = DRV_IOR(DsSclAclControlProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_wb_scl_entry->acl_profile_id, cmd, &acl.acl_control_profile));
                acl.is_scl = 1;
                acl.profile_id = p_wb_scl_entry->acl_profile_id;
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &acl, NULL, &p_acl));
                p_scl_entry->acl_profile = p_acl;
            }
            else
            {
                cmd = DRV_IOR(DsTunnelAclControlProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_wb_scl_entry->acl_profile_id, cmd, &acl.acl_control_profile));
                acl.is_scl = 0;
                acl.profile_id = p_wb_scl_entry->acl_profile_id;
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &acl, NULL, &p_acl));
                p_scl_entry->acl_profile = p_acl;
            }
        }

        if(p_wb_scl_entry->ether_type != 0)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, p_scl_entry->ether_type, 0, NULL, &p_scl_entry->ether_type_index));
        }

        /* for hash entry */
        if (!SCL_ENTRY_IS_TCAM(p_scl_entry->key_type) && !p_scl_entry->resolve_conflict)
        {
            if (SYS_SCL_ACTION_EGRESS != p_wb_scl_entry->action_type)
            {
                cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_wb_scl_entry->ad_index, cmd, &hash_ad.action));
                hash_ad.action_type = p_scl_entry->action_type;
                hash_ad.is_half = p_scl_entry->is_half;
                hash_ad.ad_index = p_wb_scl_entry->ad_index;
                hash_ad.priority = (1 == p_usw_scl_master[lchip]->hash_mode ? p_scl_entry->group->priority : 0);

                CTC_MAX_VALUE_CHECK(hash_ad.priority, 1);
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->ad_spool[hash_ad.priority], &hash_ad, NULL, &p_hash_ad));
                p_scl_entry->hash_ad = p_hash_ad;
            }
        }

        /* for tcam entry */
        if (SCL_ENTRY_IS_TCAM(p_scl_entry->key_type) || p_scl_entry->resolve_conflict)
        {
            sys_scl_sw_tcam_entry_t* pe_tcam = NULL;
            MALLOC_ZERO(MEM_SCL_MODULE, pe_tcam, sizeof(sys_scl_sw_tcam_entry_t));
            if (NULL == pe_tcam)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;

            }
            pe_tcam->fpae.entry_id = p_wb_scl_entry->entry_id;
            pe_tcam->fpae.lchip = lchip;
            pe_tcam->fpae.offset_a = p_wb_scl_entry->key_index;
            pe_tcam->fpae.priority = p_wb_scl_entry->priority;
            pe_tcam->fpae.step = SYS_SCL_GET_STEP(p_usw_scl_master[lchip]->fpa_size[p_wb_scl_entry->key_type]);
            pe_tcam->fpae.flag = !(p_wb_scl_entry->uninstall);
            pe_tcam->entry = p_scl_entry;

            p_usw_scl_master[lchip]->block[pg->priority].fpab.entries[p_wb_scl_entry->key_index] = &pe_tcam->fpae;
        }

        /* for inner entry need to restore entry-id-opf */
        if(SCL_INNER_ENTRY_ID(p_scl_entry->entry_id))
        {
            sys_usw_opf_t opf;
            sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
            opf.pool_index = 0;
            opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;

            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, (p_scl_entry->entry_id - SYS_SCL_MIN_INNER_ENTRY_ID));
        }

        /* add to group entry list's head, error just return */
        if(NULL == ctc_slist_add_head(pg->entry_list, &(p_scl_entry->head)))
        {
            return CTC_E_INVALID_PARAM;
        }

    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_wb_sync_entry_func(void* bucket_data, void* user_data)
{
    uint32 max_entry_num = 0;
    sys_scl_sw_entry_t* p_scl_entry    = NULL;
    sys_wb_scl_entry_t*   p_wb_scl_entry = NULL;
    ctc_wb_data_t*           p_wb_data      = NULL;
    sys_scl_sw_block_t*      p_block = NULL;
    uint8                    lchip = 0;

    p_scl_entry = (sys_scl_sw_entry_t*)bucket_data;
    p_wb_data   = (ctc_wb_data_t*)(((sys_scl_traverse_data_t*)user_data)->data0);
    p_block   = (sys_scl_sw_block_t*)(((sys_scl_traverse_data_t*)user_data)->data1);
    lchip     = ((sys_scl_traverse_data_t*)user_data)->value0;

    p_wb_scl_entry = (sys_wb_scl_entry_t*)p_wb_data->buffer + p_wb_data->valid_cnt;
    sal_memset(p_wb_scl_entry, 0, sizeof(sys_wb_scl_entry_t));

    max_entry_num = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_scl_entry_t);

    if (p_scl_entry->temp_entry)
    {
         /* uninstall entry not sync, but need to update fpa count for tcam entry*/
        if (SCL_ENTRY_IS_TCAM(p_scl_entry->key_type) || (p_scl_entry->resolve_conflict))
        {
            uint8 fpa_size = p_usw_scl_master[lchip]->fpa_size[p_scl_entry->key_type];
            (p_block[p_scl_entry->group->priority].fpab.free_count) += (SYS_SCL_GET_STEP(fpa_size));
            (p_block[p_scl_entry->group->priority].fpab.sub_free_count[fpa_size])++;
        }
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_scl_wb_mapping_entry(p_wb_scl_entry, p_scl_entry, 1));
    if (++p_wb_data->valid_cnt == max_entry_num)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;

}

int32
_sys_usw_scl_wb_mapping_entry_by_key(sys_wb_scl_hash_key_entry_t* p_wb_scl_entry_by_key, sys_scl_hash_key_entry_t* p_scl_entry_by_key, uint8 is_sync)
{
    if(is_sync)
    {
        p_wb_scl_entry_by_key->key_index = p_scl_entry_by_key->key_index;
        p_wb_scl_entry_by_key->action_type = p_scl_entry_by_key->action_type;
        p_wb_scl_entry_by_key->entry_id = p_scl_entry_by_key->entry_id;
        p_wb_scl_entry_by_key->scl_id = p_scl_entry_by_key->scl_id;
    }
    else
    {
        p_scl_entry_by_key->key_index = p_wb_scl_entry_by_key->key_index;
        p_scl_entry_by_key->action_type = p_wb_scl_entry_by_key->action_type;
        p_scl_entry_by_key->entry_id = p_wb_scl_entry_by_key->entry_id;
        p_scl_entry_by_key->scl_id = p_wb_scl_entry_by_key->scl_id;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_scl_wb_sync_entry_by_key_func(void* bucket_data, void* user_data)
{
    uint32 max_entry_num = 0;
    sys_scl_hash_key_entry_t* p_scl_entry_by_key    = NULL;
    sys_wb_scl_hash_key_entry_t*   p_wb_scl_entry_by_key = NULL;
    ctc_wb_data_t*                 p_wb_data      = NULL;

    p_scl_entry_by_key = (sys_scl_hash_key_entry_t*)bucket_data;
    p_wb_data   = (ctc_wb_data_t*)user_data;

    p_wb_scl_entry_by_key = (sys_wb_scl_hash_key_entry_t*)p_wb_data->buffer + p_wb_data->valid_cnt;
    sal_memset(p_wb_scl_entry_by_key, 0, sizeof(sys_wb_scl_hash_key_entry_t));

    max_entry_num = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_scl_hash_key_entry_t);

    CTC_ERROR_RETURN(_sys_usw_scl_wb_mapping_entry_by_key(p_wb_scl_entry_by_key, p_scl_entry_by_key, 1));
    if (++p_wb_data->valid_cnt == max_entry_num)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;

}

int32
_sys_usw_scl_wb_mapping_tcam_entry_by_key(sys_wb_scl_tcam_key_entry_t* p_wb_scl_tcam_entry_by_key, sys_scl_tcam_entry_key_t* p_scl_tcam_entry_by_key, uint8 is_sync)
{
    if(is_sync)
    {
        p_wb_scl_tcam_entry_by_key->lchip = p_scl_tcam_entry_by_key->p_entry->lchip;
        p_wb_scl_tcam_entry_by_key->scl_id = p_scl_tcam_entry_by_key->scl_id;
        p_wb_scl_tcam_entry_by_key->entry_id = p_scl_tcam_entry_by_key->p_entry->entry_id;
        sal_memcpy(p_wb_scl_tcam_entry_by_key->key, p_scl_tcam_entry_by_key->key, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
        sal_memcpy(p_wb_scl_tcam_entry_by_key->mask, p_scl_tcam_entry_by_key->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
    }
    else
    {
        uint8 lchip;
        sys_scl_sw_entry_t pe;
        sys_scl_sw_entry_t* pe_lkup = NULL;
        sal_memset(&pe, 0, sizeof(sys_scl_sw_entry_t));
        lchip = p_wb_scl_tcam_entry_by_key->lchip;
        p_scl_tcam_entry_by_key->scl_id = p_wb_scl_tcam_entry_by_key->scl_id;
        sal_memcpy(p_scl_tcam_entry_by_key->key, p_wb_scl_tcam_entry_by_key->key, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
        sal_memcpy(p_scl_tcam_entry_by_key->mask, p_wb_scl_tcam_entry_by_key->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
        pe.entry_id = p_wb_scl_tcam_entry_by_key->entry_id;

        pe_lkup = ctc_hash_lookup(p_usw_scl_master[lchip]->entry, &pe);
        if(!pe_lkup)
        {
            return CTC_E_NOT_EXIST;
        }
        p_scl_tcam_entry_by_key->p_entry = pe_lkup;

    }
    return CTC_E_NONE;
}


int32
_sys_usw_scl_wb_sync_tcam_entry_by_key_func(void* bucket_data, void* user_data)
{
    uint32 max_entry_num = 0;
    sys_scl_tcam_entry_key_t* p_scl_tcam_entry_by_key    = NULL;
    sys_wb_scl_tcam_key_entry_t*   p_wb_scl_tcam_entry_by_key = NULL;
    ctc_wb_data_t*                 p_wb_data      = NULL;

    p_scl_tcam_entry_by_key = (sys_scl_tcam_entry_key_t*)bucket_data;
    p_wb_data   = (ctc_wb_data_t*)user_data;

    p_wb_scl_tcam_entry_by_key = (sys_wb_scl_tcam_key_entry_t*)p_wb_data->buffer + p_wb_data->valid_cnt;
    sal_memset(p_wb_scl_tcam_entry_by_key, 0, sizeof(sys_wb_scl_tcam_key_entry_t));

    max_entry_num = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_scl_tcam_key_entry_t);

    CTC_ERROR_RETURN(_sys_usw_scl_wb_mapping_tcam_entry_by_key(p_wb_scl_tcam_entry_by_key, p_scl_tcam_entry_by_key, 1));
    if (++p_wb_data->valid_cnt == max_entry_num)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_wb_restore_default_acl_profile(uint8 lchip)
{
    uint8   hash_index = 0;
    uint32 tunnel_type = 0;
    uint32 index = 0;
    uint32 cmd = 0;
    uint32 acl_profile_id = 0;
    uint32 acl_profile_id_valid = 0;
    uint32 tbl_id[2] = {DsTunnelId_t, DsTunnelId1_t};
    uint32 fld_id1[2] = {DsTunnelId_aclControlProfileIdValid_f, DsTunnelId1_u3_g4_aclControlProfileIdValid_f};
    uint32 fld_id2[2] = {DsTunnelId_aclControlProfileId_f, DsTunnelId1_u3_g4_aclControlProfileId_f};
    sys_scl_action_acl_t acl_profile;
    sal_memset(&acl_profile, 0, sizeof(sys_scl_action_acl_t));

    for (tunnel_type=0; tunnel_type<(DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA)-USERIDHASHTYPE_TUNNELIPV4+1); tunnel_type+=1)  /*if new tunnel hash type add, please reconsider the loop condition*/
    {
        for (hash_index=0; hash_index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); hash_index+=1)
        {
            if (hash_index && (0 == p_usw_scl_master[lchip]->hash_mode))
            {
                break;      /*hash compatible mode, ad1 and ad2 is the same*/
            }
            index = p_usw_scl_master[lchip]->tunnel_default_base + 2*tunnel_type;
            cmd = DRV_IOR(tbl_id[hash_index], fld_id1[hash_index]);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &acl_profile_id_valid));
            if (acl_profile_id_valid)
            {
                cmd = DRV_IOR(tbl_id[hash_index], fld_id2[hash_index]);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &acl_profile_id));
                cmd = DRV_IOR(DsTunnelAclControlProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, acl_profile_id, cmd, &acl_profile.acl_control_profile));
                acl_profile.profile_id = acl_profile_id;
                acl_profile.is_scl = 0;     /*tunnel use default, scl flow don't use default*/
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &acl_profile, NULL, NULL));
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO," default action acl profile id:%u, ad index:%u!\n", acl_profile_id, index);
            }
        }
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_wb_restore_default_vlan_edit(uint8 lchip)
{
    uint8   hash_index = 0;
    uint32 lport = 0;
    uint32 index = 0;
    uint32 cmd = 0;
    uint32 vlan_edit_profile_id = 0;
    uint32 tbl_id[2] = {DsUserId_t, DsUserId1_t};
    uint32 fld_id[2] = {DsUserId_vlanActionProfileId_f, DsUserId1_u1_g2_vlanActionProfileId_f};
    sys_scl_sp_vlan_edit_t vlan_edit;
    sal_memset(&vlan_edit, 0, sizeof(sys_scl_sp_vlan_edit_t));

    for (lport=0; lport<SYS_USW_MAX_PORT_NUM_PER_CHIP; lport+=1)
    {
        for (hash_index=0; hash_index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); hash_index+=1)
        {
            if (hash_index && (0 == p_usw_scl_master[lchip]->hash_mode))
            {
                break;      /*hash compatible mode, ad1 and ad2 is the same*/
            }
            index = p_usw_scl_master[lchip]->igs_default_base[hash_index] + 2*lport;
            cmd = DRV_IOR(tbl_id[hash_index], fld_id[hash_index]);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &vlan_edit_profile_id));
            if (vlan_edit_profile_id)
            {
                cmd = DRV_IOR(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_edit_profile_id, cmd, &vlan_edit.action_profile));
                vlan_edit.profile_id = vlan_edit_profile_id;
                CTC_ERROR_RETURN(ctc_spool_add(p_usw_scl_master[lchip]->vlan_edit_spool, &vlan_edit, NULL, NULL));
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO," default action vlan edit profile id:%u, ad index:%u!\n", vlan_edit_profile_id, index);
            }
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_scl_wb_sync(uint8 lchip)
{
    uint16 loop1 = 0;
    uint16 loop2 = 0;
    int32  ret   = CTC_E_NONE;
    uint32 max_entry_num = 0;
    ctc_wb_data_t wb_data;
    sys_wb_scl_master_t* p_wb_scl_master = NULL;
    sys_scl_sw_block_t block[SCL_TCAM_NUM];
    sys_scl_traverse_data_t    user_data;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);
	if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		return CTC_E_NONE;
	}
    /* sync up scl_matser */
    wb_data.buffer = mem_malloc(MEM_SCL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memcpy(block, p_usw_scl_master[lchip]->block, sizeof(p_usw_scl_master[lchip]->block));

    /* sync entry info */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_scl_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_ENTRY);

    user_data.data0 = (void*)&wb_data;
    user_data.data1 = (void*)block;
    user_data.value0 = lchip;

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->entry, _sys_usw_scl_wb_sync_entry_func, (void*)(&user_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }


    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_scl_master_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_MASTER);

    max_entry_num = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_scl_master_t);

    for (loop1 = 0; loop1 < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); loop1++)
    {
        for (loop2 = 0; loop2 < CTC_FPA_KEY_SIZE_NUM; loop2++)
        {
            p_wb_scl_master = (sys_wb_scl_master_t*)wb_data.buffer + wb_data.valid_cnt;
            sal_memset(p_wb_scl_master, 0, sizeof(sys_wb_scl_master_t));
            p_wb_scl_master->lchip = lchip;
            p_wb_scl_master->block_id = loop1;
            p_wb_scl_master->key_size_type = loop2;
            p_wb_scl_master->start_offset = block[loop1].fpab.start_offset[loop2];
            p_wb_scl_master->sub_entry_count = block[loop1].fpab.sub_entry_count[loop2];
            p_wb_scl_master->sub_free_count = block[loop1].fpab.sub_free_count[loop2];
            p_wb_scl_master->version = SYS_WB_VERSION_SCL;

            if (++wb_data.valid_cnt == max_entry_num)
            {
                CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
                wb_data.valid_cnt = 0;
            }
        }
    }
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /* sync group info */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_scl_group_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_GROUP);

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->group, _sys_usw_scl_wb_sync_group_func, (void*)(&wb_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /* sync hash entry by key info */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_scl_hash_key_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_HASH_KEY_ENTRY);
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->hash_key_entry, _sys_usw_scl_wb_sync_entry_by_key_func, (void*)(&wb_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /* sync tcam entry by key */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_scl_tcam_key_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_TCAM_KEY_ENTRY);
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->tcam_entry_by_key, _sys_usw_scl_wb_sync_tcam_entry_by_key_func, (void*)(&wb_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}

int32
sys_usw_scl_wb_restore(uint8 lchip)
{
    int32  ret       = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint16 entry_cnt = 0;
    ctc_wb_query_t wb_query;
    sys_wb_scl_master_t wb_scl_master;
    sys_wb_scl_group_t wb_scl_group;
    sys_scl_sw_group_t* p_scl_group = NULL;
    sys_wb_scl_entry_t wb_scl_entry;
    sys_scl_sw_entry_t* p_scl_entry = NULL;
    sys_wb_scl_hash_key_entry_t wb_scl_entry_by_key;
    sys_scl_hash_key_entry_t* p_scl_entry_by_key = NULL;
    sys_scl_tcam_entry_key_t* p_scl_tcam_entry_by_key = NULL;
    sys_wb_scl_tcam_key_entry_t wb_scl_tcam_entry_by_key;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);
	if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		return CTC_E_NONE;
	}
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_SCL_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    if (DRV_FLD_IS_EXISIT(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdDefaultEntryBase_f))
    {
        cmd   = DRV_IOR(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdDefaultEntryBase_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        p_usw_scl_master[lchip]->igs_default_base[0] = value;
    }
    else
    {
        cmd   = DRV_IOR(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdHashLookup0DefaultEntryBase_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        p_usw_scl_master[lchip]->igs_default_base[0] = value;
        cmd   = DRV_IOR(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdHashLookup1DefaultEntryBase_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        p_usw_scl_master[lchip]->igs_default_base[1] = value;
    }

    cmd   = DRV_IOR(UserIdHashLookupCtl_t, UserIdHashLookupCtl_tunnelIdDefaultEntryBase_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    p_usw_scl_master[lchip]->tunnel_default_base = value;

    /* restore scl master */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_scl_master_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_MASTER);
    /* set default value for new added fields, the default vlaue may not be zeros */
    sal_memset(&wb_scl_master, 0, sizeof(sys_wb_scl_master_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_scl_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_SCL, wb_scl_master.version))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto done;
        }
        p_usw_scl_master[wb_scl_master.lchip]->block[wb_scl_master.block_id].fpab.start_offset[wb_scl_master.key_size_type] = wb_scl_master.start_offset;
        p_usw_scl_master[wb_scl_master.lchip]->block[wb_scl_master.block_id].fpab.sub_entry_count[wb_scl_master.key_size_type] = wb_scl_master.sub_entry_count;
        p_usw_scl_master[wb_scl_master.lchip]->block[wb_scl_master.block_id].fpab.sub_free_count[wb_scl_master.key_size_type] = wb_scl_master.sub_free_count;
        p_usw_scl_master[wb_scl_master.lchip]->block[wb_scl_master.block_id].fpab.entry_count += wb_scl_master.sub_entry_count;
        p_usw_scl_master[wb_scl_master.lchip]->block[wb_scl_master.block_id].fpab.free_count += wb_scl_master.sub_free_count;
        entry_cnt++;
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore scl group */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_scl_group_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_GROUP);

    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_scl_group, 0, sizeof(sys_wb_scl_group_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_scl_group, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_scl_group = mem_malloc(MEM_SCL_MODULE, sizeof(sys_scl_sw_group_t));
        if (NULL == p_scl_group)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_scl_group, 0, sizeof(sys_scl_sw_group_t));
        CTC_ERROR_GOTO(_sys_usw_scl_wb_mapping_group(&wb_scl_group, p_scl_group, 0), ret, done);

        if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->group, (void*)p_scl_group))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore scl entry */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_scl_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_ENTRY);

    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_scl_entry, 0, sizeof(sys_wb_scl_entry_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_scl_entry, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_scl_entry = mem_malloc(MEM_SCL_MODULE, sizeof(sys_scl_sw_entry_t));
        if (NULL == p_scl_entry)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_scl_entry, 0, sizeof(sys_scl_sw_entry_t));
        CTC_ERROR_GOTO(_sys_usw_scl_wb_mapping_entry(&wb_scl_entry, p_scl_entry, 0), ret, done);

        if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->entry, (void*)p_scl_entry))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }

        if (p_scl_entry->bind_nh)
        {
            _sys_usw_scl_bind_nexthop(lchip, p_scl_entry, p_scl_entry->nexthop_id);
        }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore scl entry by key */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_scl_hash_key_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_HASH_KEY_ENTRY);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_scl_entry_by_key, 0, sizeof(sys_wb_scl_hash_key_entry_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_scl_entry_by_key, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_scl_entry_by_key = mem_malloc(MEM_SCL_MODULE, sizeof(sys_scl_hash_key_entry_t));
        if (NULL == p_scl_entry_by_key)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_scl_entry_by_key, 0, sizeof(sys_scl_hash_key_entry_t));
        CTC_ERROR_GOTO(_sys_usw_scl_wb_mapping_entry_by_key(&wb_scl_entry_by_key, p_scl_entry_by_key, 0), ret, done);

        if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->hash_key_entry, (void*)p_scl_entry_by_key))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore scl tcam entry by key */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_scl_tcam_key_entry_t, CTC_FEATURE_SCL, SYS_WB_APPID_SCL_SUBID_TCAM_KEY_ENTRY);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_scl_tcam_entry_by_key, 0, sizeof(sys_wb_scl_tcam_key_entry_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_scl_tcam_entry_by_key, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_scl_tcam_entry_by_key = mem_malloc(MEM_SCL_MODULE, sizeof(sys_scl_tcam_entry_key_t));
        if (NULL == p_scl_tcam_entry_by_key)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_scl_tcam_entry_by_key, 0, sizeof(sys_scl_tcam_entry_key_t));
        CTC_ERROR_GOTO(_sys_usw_scl_wb_mapping_tcam_entry_by_key(&wb_scl_tcam_entry_by_key, p_scl_tcam_entry_by_key, 0), ret, done);

        if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->tcam_entry_by_key, (void*)p_scl_tcam_entry_by_key))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));
    ret = ret? ret: _sys_usw_scl_wb_restore_default_vlan_edit(lchip);
    ret = ret? ret: _sys_usw_scl_wb_restore_default_acl_profile(lchip);

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

#define __scl_internal_function__
STATIC int32
_sys_usw_scl_build_vlan_edit(uint8 lchip, void* p_ds_edit, sys_scl_sw_vlan_edit_t* p_vlan_edit)
{
    uint8 op = 0;
    uint8 mo = 0;
    uint8 st = 0;

    sys_usw_scl_vlan_tag_op_translate(lchip, p_vlan_edit->stag_op, &op, &mo);
    SetDsVlanActionProfile(V, sTagAction_f, p_ds_edit, op);
    SetDsVlanActionProfile(V, stagModifyMode_f, p_ds_edit, mo);
    sys_usw_scl_vlan_tag_op_translate(lchip, p_vlan_edit->ctag_op, &op, &mo);
    SetDsVlanActionProfile(V, cTagAction_f, p_ds_edit, op);
    SetDsVlanActionProfile(V, ctagModifyMode_f, p_ds_edit, mo);

    SetDsVlanActionProfile(V, sVlanIdAction_f, p_ds_edit, p_vlan_edit->svid_sl);
    SetDsVlanActionProfile(V, sCosAction_f, p_ds_edit, p_vlan_edit->scos_sl);
    SetDsVlanActionProfile(V, sCfiAction_f, p_ds_edit, p_vlan_edit->scfi_sl);

    SetDsVlanActionProfile(V, cVlanIdAction_f, p_ds_edit, p_vlan_edit->cvid_sl);
    SetDsVlanActionProfile(V, cCosAction_f, p_ds_edit, p_vlan_edit->ccos_sl);
    SetDsVlanActionProfile(V, cCfiAction_f, p_ds_edit, p_vlan_edit->ccfi_sl);
    SetDsVlanActionProfile(V, svlanTpidIndexEn_f, p_ds_edit, 1);
    SetDsVlanActionProfile(V, svlanTpidIndex_f, p_ds_edit, p_vlan_edit->tpid_index);

    switch (p_vlan_edit->vlan_domain)
    {
        case CTC_SCL_VLAN_DOMAIN_SVLAN:
            st = 2;
            break;
        case CTC_SCL_VLAN_DOMAIN_CVLAN:
            st = 1;
            break;
        case CTC_SCL_VLAN_DOMAIN_UNCHANGE:
            st = 0;
            break;
        default:
            break;
    }
    SetDsVlanActionProfile(V, outerVlanStatus_f, p_ds_edit, st);
/*
   ctagAddMode_f
   svlanTpidIndexEn_f
   svlanTpidIndex_f
   ds_vlan_action_profile_t
 */
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_vlan_edit_static_add(uint8 lchip, sys_scl_sw_vlan_edit_t* p_vlan_edit, uint16* p_prof_idx)
{
    uint32 cmd = 0;
    int32  ret = CTC_E_NONE ;
    sys_scl_sp_vlan_edit_t *p_sp_vlan_edit;

    MALLOC_ZERO(MEM_SCL_MODULE, p_sp_vlan_edit, sizeof(sys_scl_sp_vlan_edit_t));
    if (NULL == p_sp_vlan_edit)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    p_sp_vlan_edit->profile_id = *p_prof_idx;

    _sys_usw_scl_build_vlan_edit(lchip, &p_sp_vlan_edit->action_profile, p_vlan_edit);
    if (ctc_spool_static_add(p_usw_scl_master[lchip]->vlan_edit_spool, p_sp_vlan_edit) < 0)
    {
        ret = CTC_E_NO_MEMORY;
        goto cleanup;
    }

    cmd = DRV_IOW(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
    if ((DRV_IOCTL(lchip, p_sp_vlan_edit->profile_id, cmd, &p_sp_vlan_edit->action_profile)) < 0 )
    {
        ret = CTC_E_HW_FAIL;
        goto cleanup;
    }

    *p_prof_idx = *p_prof_idx + 1;

    mem_free(p_sp_vlan_edit);
    return CTC_E_NONE;

cleanup:
    mem_free(p_sp_vlan_edit);
    return ret;
}

STATIC int32
_sys_usw_scl_stag_edit_template_build(uint8 lchip, sys_scl_sw_vlan_edit_t* p_vlan_edit, uint16* p_prof_idx)
{
    uint8 stag_op = 0;


    for (stag_op = CTC_VLAN_TAG_OP_NONE; stag_op < CTC_VLAN_TAG_OP_MAX; stag_op++)
    {
        p_vlan_edit->stag_op = stag_op;

        switch (stag_op)
        {
        case CTC_VLAN_TAG_OP_REP:         /* template has no replace */
        case CTC_VLAN_TAG_OP_VALID:       /* template has no valid */
            break;
        case CTC_VLAN_TAG_OP_REP_OR_ADD:
        case CTC_VLAN_TAG_OP_ADD:
        {
            p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, p_vlan_edit, p_prof_idx));

            p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_NEW;
            CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, p_vlan_edit, p_prof_idx));
        }
        break;
        case CTC_VLAN_TAG_OP_NONE:
        case CTC_VLAN_TAG_OP_DEL:
            p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, p_vlan_edit, p_prof_idx));
            break;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_rsv_vlan_edit(uint8 lchip)
{
    uint16                 prof_idx = 1;
    uint8                  ctag_op  = 0;

    sys_scl_sw_vlan_edit_t vlan_edit;
    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));

    vlan_edit.vlan_domain = CTC_SCL_VLAN_DOMAIN_UNCHANGE;
    for (ctag_op = CTC_VLAN_TAG_OP_NONE; ctag_op < CTC_VLAN_TAG_OP_MAX; ctag_op++)
    {
        vlan_edit.ctag_op = ctag_op;

        switch (ctag_op)
        {
            case CTC_VLAN_TAG_OP_ADD :           /* template has no append ctag */
            case CTC_VLAN_TAG_OP_REP :           /* template has no replace */
            case CTC_VLAN_TAG_OP_VALID :         /* template has no valid */
                break;
            case CTC_VLAN_TAG_OP_REP_OR_ADD:
                {
                    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_NEW;
                    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_NEW;
                    CTC_ERROR_RETURN(_sys_usw_scl_stag_edit_template_build(lchip, &vlan_edit, &prof_idx));

                    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_NEW;
                    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                    CTC_ERROR_RETURN(_sys_usw_scl_stag_edit_template_build(lchip, &vlan_edit, &prof_idx));

                    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                    CTC_ERROR_RETURN(_sys_usw_scl_stag_edit_template_build(lchip, &vlan_edit, &prof_idx));

                    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_NEW;
                    CTC_ERROR_RETURN(_sys_usw_scl_stag_edit_template_build(lchip, &vlan_edit, &prof_idx));
                }
                break;
            case CTC_VLAN_TAG_OP_NONE:
            case CTC_VLAN_TAG_OP_DEL:
                vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
                CTC_ERROR_RETURN(_sys_usw_scl_stag_edit_template_build(lchip, &vlan_edit, &prof_idx));
                break;
        }
    }

    /*for swap*/
    vlan_edit.ctag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
    vlan_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;

    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.svid_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.scos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, &vlan_edit, &prof_idx));

    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
    vlan_edit.svid_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
    CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, &vlan_edit, &prof_idx));

    vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
    vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    vlan_edit.svid_sl = CTC_VLAN_TAG_SL_AS_PARSE;
    vlan_edit.scos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    CTC_ERROR_RETURN(_sys_usw_scl_vlan_edit_static_add(lchip, &vlan_edit, &prof_idx));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_init_default_entry(uint8 lchip)
{

    uint16          lport;
    uint32          cmd      = 0;
    uint8           index    = 0;
    DsTunnelId_m    tunnelid;
    DsUserId_m      userid;
    DsVlanXlateDefault_m   vlanXlate;

    /* init userid default entry */
    sal_memset(&userid, 0, sizeof(DsUserId_m));
    /* ingress hash default entry */
    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        cmd = DRV_IOW(DsUserId_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_usw_scl_master[lchip]->igs_default_base[0] + 2*lport, cmd, &userid));
        if (2 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM))
        {
            cmd = DRV_IOW(DsUserId1_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_usw_scl_master[lchip]->igs_default_base[1] + 2*lport, cmd, &userid));
        }
    }

    sal_memset(&vlanXlate, 0, sizeof(DsVlanXlateDefault_m));
    /* egress hash default entry */
    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        cmd = DRV_IOW(DsVlanXlateDefault_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &vlanXlate));
    }

    sal_memset(&tunnelid, 0, sizeof(DsTunnelId_m));
    /* init ip-tunnel default entry */
    for (index = 0; index <= (DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) - USERIDHASHTYPE_TUNNELIPV4); index++)
    {
        cmd = DRV_IOW(DsTunnelId_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_usw_scl_master[lchip]->tunnel_default_base + 2*index, cmd, &tunnelid));
    }

    return CTC_E_NONE;
}

/*
 * init scl register
 */
STATIC int32
_sys_usw_scl_init_register(uint8 lchip, void* scl_global_cfg)
{
    uint32 cmd   = 0;
    uint32 value = 0;
    IpeUserIdTcamCtl_m userid_tcam_ctl;

    sal_memset(&userid_tcam_ctl, 0, sizeof(userid_tcam_ctl));

    value = p_usw_scl_master[lchip]->igs_default_base[0];
    cmd   = DRV_IOW(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdDefaultEntryBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd   = DRV_IOW(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdHashLookup0DefaultEntryBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    value = p_usw_scl_master[lchip]->igs_default_base[1];
    cmd   = DRV_IOW(UserIdHashLookupCtl_t, UserIdHashLookupCtl_userIdHashLookup1DefaultEntryBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd   = DRV_IOW(UserIdHashLookupCtl_t, UserIdHashLookupCtl_tunnelIdDefaultEntryBase_f);
    value = p_usw_scl_master[lchip]->tunnel_default_base;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
    SetIpeUserIdTcamCtl(V, array_0_v6BasicKeyIpAddressMode_f, &userid_tcam_ctl, 0);
    SetIpeUserIdTcamCtl(V, array_1_v6BasicKeyIpAddressMode_f, &userid_tcam_ctl, 0);
    SetIpeUserIdTcamCtl(V, array_0_v6BasicKeyMode0IpAddr1Encode_f, &userid_tcam_ctl, 0x1F);
    SetIpeUserIdTcamCtl(V, array_0_v6BasicKeyMode0IpAddr2Encode_f, &userid_tcam_ctl, 0x1F);
    SetIpeUserIdTcamCtl(V, array_1_v6BasicKeyMode0IpAddr1Encode_f, &userid_tcam_ctl, 0x1F);
    SetIpeUserIdTcamCtl(V, array_1_v6BasicKeyMode0IpAddr2Encode_f, &userid_tcam_ctl, 0x1F);

    if (!DRV_FLD_IS_EXISIT(IpeUserIdTcamCtl_t, IpeUserIdTcamCtl_lookupLevel_0_l3Key160ShareFieldMode_f))
    {
        SetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForUserId_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForUserId_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForScl_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForScl_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForTunnelId_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForTunnelId_f, &userid_tcam_ctl, 0);
    }
    else
    {
        /**< [TM] Keep compatibile with D2 */
        SetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_0_changeFlowL2KeyUseCvlanMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_changeFlowL2KeyUseCvlanMode_f, &userid_tcam_ctl, 0);
        /**< [TM] Append UDF key 320 Share type: 1-l3l4, 2-l2, 3-l2l3 and default 0-l3l4*/
        SetIpeUserIdTcamCtl(V, lookupLevel_0_udf320KeyShareType_f, &userid_tcam_ctl, 2);
        SetIpeUserIdTcamCtl(V, lookupLevel_1_udf320KeyShareType_f, &userid_tcam_ctl, 2);
        /**< [TM] Append tcam2 tcam 3 lookup */
        SetIpeUserIdTcamCtl(V, array_2_v6BasicKeyIpAddressMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, array_3_v6BasicKeyIpAddressMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, array_2_v6BasicKeyMode0IpAddr1Encode_f, &userid_tcam_ctl, 0x1F);
        SetIpeUserIdTcamCtl(V, array_2_v6BasicKeyMode0IpAddr2Encode_f, &userid_tcam_ctl, 0x1F);
        SetIpeUserIdTcamCtl(V, array_3_v6BasicKeyMode0IpAddr1Encode_f, &userid_tcam_ctl, 0x1F);
        SetIpeUserIdTcamCtl(V, array_3_v6BasicKeyMode0IpAddr2Encode_f, &userid_tcam_ctl, 0x1F);
        SetIpeUserIdTcamCtl(V, lookupLevel_2_l3Key160ShareFieldMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_3_l3Key160ShareFieldMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_2_changeFlowL2KeyUseCvlanMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_3_changeFlowL2KeyUseCvlanMode_f, &userid_tcam_ctl, 0);
        SetIpeUserIdTcamCtl(V, lookupLevel_2_udf320KeyShareType_f, &userid_tcam_ctl, 2);
        SetIpeUserIdTcamCtl(V, lookupLevel_3_udf320KeyShareType_f, &userid_tcam_ctl, 2);
        /**< [TM] Append prCvlanIdValid, here set "1" to keep compatible with D2
            0:DsSclMacL3Key320.cvlanIdValid = cvlanIdValid
            1:DsSclMacL3Key320.cvlanIdValid = prCvlanIdValid */
        SetIpeUserIdTcamCtl(V, l2L3KeyUsePrCVlanValidInfo_f, &userid_tcam_ctl, 1);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &userid_tcam_ctl));

    value = 1;
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_layer2TypeUsedAsVlanNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_create_rsv_group(uint8 lchip)
{

    uint32               index = 0;
    ctc_scl_group_info_t hash_group;
    ctc_scl_group_info_t tcam_group;
    sal_memset(&hash_group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&tcam_group, 0, sizeof(ctc_scl_group_info_t));
    hash_group.type = CTC_SCL_GROUP_TYPE_HASH;
    tcam_group.type = CTC_SCL_GROUP_TYPE_NONE;
    hash_group.lchip = lchip;
    tcam_group.lchip = lchip;
    /*reserved hash Group*/
    for (index = CTC_SCL_GROUP_ID_HASH_PORT; index < CTC_SCL_GROUP_ID_HASH_MAX; index++)
    {
         CTC_ERROR_RETURN(_sys_usw_scl_create_group(lchip, index, &hash_group, 1));
    }
    /*reserved tcam Group*/
    for (index = 0; index < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index += 1)
    {
        tcam_group.priority = index;
        CTC_ERROR_RETURN(_sys_usw_scl_create_group(lchip, CTC_SCL_GROUP_ID_TCAM0 + index, &tcam_group, 1));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_init_opf(uint8 lchip)
{
    uint32               entry_num    = 0;
    uint32               start_offset = 0;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsVlanActionProfile_t, &entry_num));
    start_offset = MCHIP_CAP(SYS_CAP_SCL_VLAN_ACTION_RESERVE_NUM);
    entry_num   -= MCHIP_CAP(SYS_CAP_SCL_VLAN_ACTION_RESERVE_NUM);

    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_scl_master[lchip]->opf_type_vlan_edit, 1, "opf-type-scl-vlan-edit"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_vlan_edit;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));

    }

    start_offset = 0;
    entry_num    = SYS_SCL_MAX_INNER_ENTRY_ID - SYS_SCL_MIN_INNER_ENTRY_ID + 1;

    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_scl_master[lchip]->opf_type_entry_id, 1, "opf-type-scl-entry-id"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, entry_num));
    }

    start_offset = 0;
    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_scl_master[lchip]->opf_type_flow_acl_profile, 1, "opf-type-scl-flow-acl-profile"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_flow_acl_profile;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, MCHIP_CAP(SYS_CAP_SCL_ACL_CONTROL_PROFILE)));
    }

    start_offset = 0;
    if (entry_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile, 1, "opf-type-scl-tunnel-acl-profile"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, MCHIP_CAP(SYS_CAP_SCL_ACL_CONTROL_PROFILE)));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_mapping_vlan_edit(uint8 lchip,  ctc_scl_vlan_edit_t* p_vlan_edit,void* p_ds_edit)
{
    uint8 op = 0;
    uint8 mo = 0;
    uint8 st = 0;

    sys_usw_scl_vlan_tag_op_translate(lchip, p_vlan_edit->stag_op, &op, &mo);
    SetDsVlanActionProfile(V, sTagAction_f, p_ds_edit, op);
    SetDsVlanActionProfile(V, stagModifyMode_f, p_ds_edit, mo);
    sys_usw_scl_vlan_tag_op_translate(lchip, p_vlan_edit->ctag_op, &op, &mo);
    SetDsVlanActionProfile(V, cTagAction_f, p_ds_edit, op);
    SetDsVlanActionProfile(V, ctagModifyMode_f, p_ds_edit, mo);

    SetDsVlanActionProfile(V, sVlanIdAction_f, p_ds_edit, p_vlan_edit->svid_sl);
    SetDsVlanActionProfile(V, sCosAction_f, p_ds_edit, p_vlan_edit->scos_sl);
    SetDsVlanActionProfile(V, sCfiAction_f, p_ds_edit, p_vlan_edit->scfi_sl);

    SetDsVlanActionProfile(V, cVlanIdAction_f, p_ds_edit, p_vlan_edit->cvid_sl);
    SetDsVlanActionProfile(V, cCosAction_f, p_ds_edit, p_vlan_edit->ccos_sl);
    SetDsVlanActionProfile(V, cCfiAction_f, p_ds_edit, p_vlan_edit->ccfi_sl);
    SetDsVlanActionProfile(V, svlanTpidIndexEn_f, p_ds_edit, 1);
    SetDsVlanActionProfile(V, svlanTpidIndex_f, p_ds_edit, p_vlan_edit->tpid_index);

    switch (p_vlan_edit->vlan_domain)
    {
        case CTC_SCL_VLAN_DOMAIN_SVLAN:
            st = 2;
            break;
        case CTC_SCL_VLAN_DOMAIN_CVLAN:
            st = 1;
            break;
        case CTC_SCL_VLAN_DOMAIN_UNCHANGE:
            st = 0;
            break;
        default:
            break;
    }
    SetDsVlanActionProfile(V, outerVlanStatus_f, p_ds_edit, st);
/*
   ctagAddMode_f
   svlanTpidIndexEn_f
   svlanTpidIndex_f
   ds_vlan_action_profile_t
 */
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_mapping_acl_profile(uint8 lchip, uint8 is_tunnel, ctc_acl_property_t* p_acl_profile, void* p_ds_edit)
{
    uint8 offset          = 0;
    uint8 step            = 0;
    uint8 gport_type      = 0;
    uint8 sys_lkup_type   = 0;
    uint8 use_mapped_vlan = 0;


    sys_lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, p_acl_profile->tcam_lkup_type);

    if (CTC_FLAG_ISSET(p_acl_profile->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
        gport_type = DRV_FLOWPORTTYPE_LPORT;
    }
    else if (CTC_FLAG_ISSET(p_acl_profile->flag, CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        gport_type = DRV_FLOWPORTTYPE_METADATA;
    }
    else if (CTC_FLAG_ISSET(p_acl_profile->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
        gport_type = DRV_FLOWPORTTYPE_BITMAP;
    }
    else
    {
        gport_type = DRV_FLOWPORTTYPE_GPORT;
    }

    use_mapped_vlan = CTC_FLAG_ISSET(p_acl_profile->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);

    if (!is_tunnel)
    {
        step = DsSclAclControlProfile_gAcl_1_aclLookupType_f - DsSclAclControlProfile_gAcl_0_aclLookupType_f;
        offset = step * p_acl_profile->acl_priority;
        SetDsSclAclControlProfile(V, gAcl_0_overwriteIntfAcl_f + offset, p_ds_edit, 1);
        SetDsSclAclControlProfile(V, gAcl_0_aclLookupType_f + offset, p_ds_edit, sys_lkup_type);
        SetDsSclAclControlProfile(V, gAcl_0_aclUseGlobalPortType_f + offset, p_ds_edit, gport_type);
        SetDsSclAclControlProfile(V, gAcl_0_aclUsePIVlan_f + offset, p_ds_edit, use_mapped_vlan);
        SetDsSclAclControlProfile(V, gAcl_0_trustOverwriteLabel_f + offset, p_ds_edit, 1);
        SetDsSclAclControlProfile(V, gAcl_0_overwriteLabelType_f + offset, p_ds_edit, 1);
    }
    else
    {
        step = DsTunnelAclControlProfile_gAcl_1_aclLookupType_f - DsTunnelAclControlProfile_gAcl_0_aclLookupType_f;
        offset = step * p_acl_profile->acl_priority;
        SetDsTunnelAclControlProfile(V, gAcl_0_overwriteIntfAcl_f + offset, p_ds_edit, 1);
        SetDsTunnelAclControlProfile(V, gAcl_0_aclLookupType_f + offset, p_ds_edit, sys_lkup_type);
        SetDsTunnelAclControlProfile(V, gAcl_0_aclUseGlobalPortType_f + offset, p_ds_edit, gport_type);
        SetDsTunnelAclControlProfile(V, gAcl_0_aclUsePIVlan_f + offset, p_ds_edit, use_mapped_vlan);
        SetDsTunnelAclControlProfile(V, gAcl_0_trustDsTunnelLabel_f + offset, p_ds_edit, 1);
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_dump_vlan_edit_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_scl_sp_vlan_edit_t* p_ad = (sys_scl_sp_vlan_edit_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u: %-20s\n", (*p_cnt),p_ad->profile_id, "Static");
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u: %-20u\n", (*p_cnt),p_ad->profile_id, p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_dump_acl_profile_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_scl_action_acl_t* p_ad = (sys_scl_action_acl_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u%-20uStatic\n", (*p_cnt),p_ad->profile_id, p_ad->is_scl);
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-20u%-20u%-20u\n", (*p_cnt),p_ad->profile_id, p_ad->is_scl, p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_dump_hash_ad_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32* p_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_scl_sw_hash_ad_t* p_ad = (sys_scl_sw_hash_ad_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-24u%-20u%-20uStatic\n", (*p_cnt), p_ad->action_type, p_ad->ad_index, p_ad->is_half);
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-24u%-20u%-20u%-20u\n", (*p_cnt), p_ad->action_type, p_ad->ad_index, p_ad->is_half, p_node->ref_cnt);
    }
    (*p_cnt)++;
    return CTC_E_NONE;
}
#define __scl_internal_api_function__
int32
sys_usw_scl_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint32 loop = 0;
    char*   action_type_str[SYS_SCL_ACTION_NUM] = {"Ingress", "Egress", "Tunnel", "Flow","Mpls"};
    sys_scl_sw_block_t* pb = NULL;
    sys_traverse_t  param;
    uint32  spool_cnt = 1;
    ctc_spool_t* p_ad = NULL;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# SCL");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of vlan edit", p_usw_scl_master[lchip]->opf_type_vlan_edit);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of entry id", p_usw_scl_master[lchip]->opf_type_entry_id);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of flow acl profile", p_usw_scl_master[lchip]->opf_type_flow_acl_profile);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Opf type of tunnel acl profile", p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile);

    SYS_DUMP_DB_LOG(p_f, "%-36s: %u %u\n","Ingress default entry base", p_usw_scl_master[lchip]->igs_default_base[0], p_usw_scl_master[lchip]->igs_default_base[1]);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Hash Mode", p_usw_scl_master[lchip]->hash_mode);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Tunnel default entry base", p_usw_scl_master[lchip]->tunnel_default_base);

    SYS_DUMP_DB_LOG(p_f, "Entry Count\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-24s%-24s\n","Action Type", "Entry count");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
    for(loop=0; loop < SYS_SCL_ACTION_NUM; loop++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-24s%-24u\n",action_type_str[loop], p_usw_scl_master[lchip]->entry_count[loop]);
    }

    SYS_DUMP_DB_LOG(p_f, "\n%-36s\n","Block information:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16s%-16s%-16s\n","Block ID","Entry numer", "Entry count","Free count","Move number");
    SYS_DUMP_DB_LOG(p_f, "%-16s%-16s%-16s%-16s%-16s%-16s\n","","Key size","Start offset", "Sub cnt","Sub free cnt","Reserve cnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------------------------------");
    for(loop=0; loop < SCL_TCAM_NUM; loop++)
    {
        pb = &p_usw_scl_master[lchip]->block[loop];
        SYS_DUMP_DB_LOG(p_f, "\n%-16u%-16u%-16u%-16u\n", loop, pb->fpab.entry_count, pb->fpab.free_count, pb->fpab.move_num);

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

    SYS_DUMP_DB_LOG(p_f, "%-36s\n","Hash select information:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-12s%-14s\n","Hash sel id", "Profile count");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------");

    for(loop=0; loop < SYS_SCL_HASH_SEL_ID_MAX; loop++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-12u%-14u\n", loop, p_usw_scl_master[lchip]->hash_sel_profile_count[loop]);
    }

    SYS_DUMP_DB_LOG(p_f, "\n");

    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_scl_master[lchip]->opf_type_vlan_edit, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_scl_master[lchip]->opf_type_entry_id, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_scl_master[lchip]->opf_type_flow_acl_profile, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");

    if(p_usw_scl_master[lchip]->vlan_edit_spool->count)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;

        SYS_DUMP_DB_LOG(p_f, "\nSpool type: %s\n","Vlan edit spool (Table Name: DsVlanActionProfile)");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-12s%-20s: %-20s\n", "Node","Index", "Ref count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-------------------------------------------------------------");
        ctc_spool_traverse(p_usw_scl_master[lchip]->vlan_edit_spool, (spool_traversal_fn)_sys_usw_scl_dump_vlan_edit_spool_db , (void*)(&param));
    }

    if(p_usw_scl_master[lchip]->acl_spool->count)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;

        SYS_DUMP_DB_LOG(p_f, "\nSpool type: %s\n","Acl profile spool (Table Name: DsSclAclControlProfile)");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-12s%-20s%-20s%-20s\n", "Node","Table index", "Is Scl", "Ref count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "--------------------------------------------------------------------");
        ctc_spool_traverse(p_usw_scl_master[lchip]->acl_spool, (spool_traversal_fn)_sys_usw_scl_dump_acl_profile_spool_db , (void*)(&param));
    }
    for(loop=0; loop < 2; loop++)
    {
        spool_cnt = 1;
        sal_memset(&param, 0, sizeof(param));
        param.data = (void*)p_f;
        param.data1 = (void*)&spool_cnt;
        p_ad = p_usw_scl_master[lchip]->ad_spool[loop];
        if(p_ad && p_ad->count)
        {
            SYS_DUMP_DB_LOG(p_f, "\nSpool type: Ad Spool %u\n", loop);
            SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------------------------");
            SYS_DUMP_DB_LOG(p_f, "%-12s%-20s%-20s%-20s%-20s\n", "Node","Action Type","Profile ID","Is Half","Ref count");
            SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------------------------");
            ctc_spool_traverse(p_ad, (spool_traversal_fn)_sys_usw_scl_dump_hash_ad_spool_db , (void*)(&param));
        }
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}
int32
sys_usw_scl_ftm_scl_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_LOCK(lchip);
    specs_info->used_size = p_usw_scl_master[lchip]->entry_count[SYS_SCL_ACTION_INGRESS];
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_ftm_tunnel_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_LOCK(lchip);
    specs_info->used_size = p_usw_scl_master[lchip]->entry_count[SYS_SCL_ACTION_TUNNEL];
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_ftm_flow_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_LOCK(lchip);
    specs_info->used_size = p_usw_scl_master[lchip]->entry_count[SYS_SCL_ACTION_FLOW];
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_init(uint8 lchip, void* scl_global_cfg)
{
    uint32      ad_num[2] = {0};
    uint32      ad_tbl[2] = {DsUserId_t, DsUserId1_t};
    uint32      vedit_entry_size        = 0;
    int32       ret    = 0;
    uint32      cmd    = 0;
    uint32      value  = 0;
    uint32      offset = 0;
    uint32      loop   = 0;
    uint32      entry_num = 0;
    uint32      spec_type = 0;
    uint32      spec_value = 0;
    ctc_spool_t spool;
    uint8       index = 0;
    uint32      entry_num1 = 0;
    drv_ftm_info_detail_t p_ftm_info;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
	}
    LCHIP_CHECK(lchip);
    /* check init */
    if (NULL != p_usw_scl_master[lchip])
    {
        return CTC_E_NONE;
    }
    /* malloc master */
    MALLOC_ZERO(MEM_SCL_MODULE, p_usw_scl_master[lchip], sizeof(sys_scl_master_t));
    if (NULL == p_usw_scl_master[lchip])
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(&p_ftm_info, 0, sizeof(drv_ftm_info_detail_t));
    p_ftm_info.info_type = DRV_FTM_INFO_TYPE_SCL_MODE;
    CTC_ERROR_RETURN(drv_usw_ftm_get_info_detail(lchip, &p_ftm_info));
    if (1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM))       /*only one hash*/
    {
        p_usw_scl_master[lchip]->hash_mode = 2;
    }
    else if (2 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM) && !p_ftm_info.scl_mode)       /*two hash, incompatible mode*/
    {
        p_usw_scl_master[lchip]->hash_mode = 1;
    }
    else
    {
        /*default: two hash, compatible mode*/
    }

    /* get Hw table size */
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsEgressXcOamPortHashKey_t, &(p_usw_scl_master[lchip]->egs_entry_num)), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, ad_tbl[0], &ad_num[0]), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, ad_tbl[1], &ad_num[1]), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsVlanActionProfile_t, &vedit_entry_size), ret, error0);

    /*init DB*/
    p_usw_scl_master[lchip]->group = ctc_hash_create(SYS_SCL_GROUP_HASH_SIZE/SYS_SCL_GROUP_HASH_BLOCK_SIZE,
                                        SYS_SCL_GROUP_HASH_BLOCK_SIZE,
                                        (hash_key_fn) _sys_usw_scl_group_hash_make,
                                        (hash_cmp_fn) _sys_usw_scl_group_hash_compare);
    if (NULL == p_usw_scl_master[lchip]->group)
    {
        ret = CTC_E_INIT_FAIL;
        goto error0;
    }

    p_usw_scl_master[lchip]->entry = ctc_hash_create(SYS_SCL_ENTRY_HASH_SIZE/SYS_SCL_ENTRY_HASH_BLOCK_SIZE,
                                            SYS_SCL_ENTRY_HASH_BLOCK_SIZE,
                                            (hash_key_fn) _sys_usw_scl_entry_hash_make,
                                            (hash_cmp_fn) _sys_usw_scl_entry_hash_compare);
    if (NULL == p_usw_scl_master[lchip]->entry)
    {
        ret = CTC_E_INIT_FAIL;
        goto error1;
    }

    p_usw_scl_master[lchip]->hash_key_entry = ctc_hash_create(SYS_SCL_ENTRY_BY_KEY_HASH_SIZE/SYS_SCL_ENTRY_BY_KEY_HASH_BLOCK_SIZE,
                                                   SYS_SCL_ENTRY_BY_KEY_HASH_BLOCK_SIZE,
                                                   (hash_key_fn) _sys_usw_scl_hash_key_entry_make,
                                                   (hash_cmp_fn) _sys_usw_scl_hash_key_entry_compare);
    if (NULL == p_usw_scl_master[lchip]->hash_key_entry)
    {
        ret = CTC_E_INIT_FAIL;
        goto error2;
    }

    p_usw_scl_master[lchip]->tcam_entry_by_key = ctc_hash_create(SYS_SCL_TCAM_ENTRY_BY_KEY_HASH_SIZE, 1,
                                                   (hash_key_fn) _sys_usw_scl_tcam_entry_by_key_hash_make,
                                                   (hash_cmp_fn) _sys_usw_scl_tcam_entry_by_key_hash_compare);
    if (NULL == p_usw_scl_master[lchip]->tcam_entry_by_key)
    {
        ret = CTC_E_INIT_FAIL;
        goto error3;
    }
    /*vlan edit spool*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = vedit_entry_size / SYS_SCL_VLAN_EDIT_SPOOL_BLOCK_SIZE;
    spool.block_size = SYS_SCL_VLAN_EDIT_SPOOL_BLOCK_SIZE;
    spool.max_count = vedit_entry_size;
    spool.user_data_size = sizeof(sys_scl_sp_vlan_edit_t);
    spool.spool_key = (hash_key_fn) _sys_usw_scl_vlan_edit_spool_make;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_scl_vlan_edit_spool_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_scl_vlan_edit_spool_alloc_index;
    spool.spool_free= (spool_free_fn)_sys_usw_scl_vlan_edit_spool_free_index;
    p_usw_scl_master[lchip]->vlan_edit_spool = ctc_spool_create(&spool);
    if (NULL == p_usw_scl_master[lchip]->vlan_edit_spool)
    {
        ret = CTC_E_INIT_FAIL;
        goto error3;
    }

    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        if(ad_num[index] == 0)
        {
            continue;
        }
        /*Ad spool*/
        spool.lchip = lchip;
        spool.block_num = ad_num[index] / SYS_SCL_AD_SPOOL_BLOCK_SIZE;
        spool.block_size = SYS_SCL_AD_SPOOL_BLOCK_SIZE;
        spool.max_count = ad_num[index];
        spool.user_data_size = sizeof(sys_scl_sw_hash_ad_t);
        spool.spool_key = (hash_key_fn) _sys_usw_scl_ad_spool_make;
        spool.spool_cmp = (hash_cmp_fn) _sys_usw_scl_ad_spool_compare;
        spool.spool_alloc = (spool_alloc_fn)_sys_usw_scl_ad_spool_alloc_index;
        spool.spool_free= (spool_free_fn)_sys_usw_scl_ad_spool_free_index;
        p_usw_scl_master[lchip]->ad_spool[index] = ctc_spool_create(&spool);
        if (NULL == p_usw_scl_master[lchip]->ad_spool[index])
        {
            ret = CTC_E_INIT_FAIL;
            goto error4;
        }
    }


    /*acl profile spool*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 2 * MCHIP_CAP(SYS_CAP_SCL_ACL_CONTROL_PROFILE);
    spool.max_count = 2 * MCHIP_CAP(SYS_CAP_SCL_ACL_CONTROL_PROFILE);
    spool.user_data_size = sizeof(sys_scl_action_acl_t);
    spool.spool_key = (hash_key_fn) _sys_usw_scl_acl_profile_spool_make;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_scl_acl_profile_spool_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_scl_acl_profile_spool_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_scl_acl_profile_spool_free_index;
    p_usw_scl_master[lchip]->acl_spool = ctc_spool_create(&spool);
    if (NULL == p_usw_scl_master[lchip]->acl_spool)
    {
        ret = CTC_E_INIT_FAIL;
        goto error5;
    }

    /* init count */
    sal_memset(p_usw_scl_master[lchip]->entry_count, 0, sizeof(p_usw_scl_master[lchip]->entry_count));

    CTC_ERROR_GOTO(_sys_usw_scl_init_opf(lchip), ret, error6);
    CTC_ERROR_GOTO(_sys_usw_scl_init_fpa(lchip), ret, error7);
    CTC_ERROR_GOTO(_sys_usw_scl_init_build_key_and_action_cb(lchip), ret, error8);

    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        if(ad_num[index] == 0)
        {
            continue;
        }
        CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, ad_tbl[index], 1, 2*SYS_SCL_DEFAULT_ENTRY_BASE, 1,  &offset), ret, error8);
        p_usw_scl_master[lchip]->igs_default_base[index] = offset;
    }

    /*MUST alloc tunnel ad index from forward for tunnle defualt entry base is same*/
    CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsTunnelId_t, 0, 2*(DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA)- USERIDHASHTYPE_TUNNELIPV4 + 1), 2, &offset), ret, error8);
    if(!DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsTunnelId1_t, 0, 2*(DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) - USERIDHASHTYPE_TUNNELIPV4 + 1), 2, &offset), ret, error8);
    }
    p_usw_scl_master[lchip]->tunnel_default_base = offset;
    CTC_ERROR_GOTO(_sys_usw_scl_rsv_vlan_edit(lchip), ret, error8);

    CTC_ERROR_GOTO(_sys_usw_scl_init_default_entry(lchip), ret, error8);
    CTC_ERROR_GOTO(_sys_usw_scl_init_register(lchip, NULL), ret, error8); /* after init default entry */

    sal_mutex_create(&p_usw_scl_master[lchip]->mutex);
    if (NULL == p_usw_scl_master[lchip]->mutex)
    {
        ret = CTC_E_NO_MEMORY;
        goto error8;
    }

    if (!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER))
    {
        CTC_ERROR_GOTO(_sys_usw_scl_create_rsv_group(lchip), ret, error9);
    }

    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_SCL, sys_usw_scl_ftm_scl_cb);
    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_TUNNEL, sys_usw_scl_ftm_tunnel_cb);
    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_SCL_FLOW, sys_usw_scl_ftm_flow_cb);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_SCL, sys_usw_scl_wb_sync), ret, error9);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_scl_wb_restore(lchip), ret, error9);
    }

    /* for DsSclFlow force decap use */
    cmd = DRV_IOW(IpeSclFlowCtl_t, IpeSclFlowCtl_removeByteShift_f);
    value = 1;
    DRV_IOCTL(lchip, 0, cmd, &value);

    /* set chip_capability */
    spec_type = CTC_FTM_SPEC_SCL;
    sys_usw_ftm_get_profile_specs(lchip, spec_type, &spec_value);
    sys_usw_ftm_query_table_entry_num(lchip, DsUserIdPortHashKey_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_SCL_HASH_ENTRY_NUM) = (entry_num < spec_value) ? entry_num : spec_value;
    if (1 == p_usw_scl_master[lchip]->hash_mode)
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsUserId1PortHashKey_t, &entry_num);
        MCHIP_CAP(SYS_CAP_SPEC_SCL1_HASH_ENTRY_NUM) = (entry_num < spec_value) ? entry_num : spec_value;
    }
    else
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsUserId1PortHashKey_t, &entry_num1);
        if ((entry_num != entry_num1) && !p_usw_scl_master[lchip]->hash_mode)
        {
            ret = CTC_E_NOT_INIT;
            goto error9;
        }
        MCHIP_CAP(SYS_CAP_SPEC_SCL1_HASH_ENTRY_NUM) = 0;
    }
    sys_usw_ftm_query_table_entry_num(lchip, DsScl0MacKey160_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_SCL0_IGS_TCAM_ENTRY_NUM) = entry_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsScl1MacKey160_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_SCL1_IGS_TCAM_ENTRY_NUM) = entry_num;
    if (4 == MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM))
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsScl2MacKey160_t, &entry_num);
        MCHIP_CAP(SYS_CAP_SPEC_SCL2_IGS_TCAM_ENTRY_NUM) = entry_num;
        sys_usw_ftm_query_table_entry_num(lchip, DsScl3MacKey160_t, &entry_num);
        MCHIP_CAP(SYS_CAP_SPEC_SCL3_IGS_TCAM_ENTRY_NUM) = entry_num;
    }
    else
    {
        MCHIP_CAP(SYS_CAP_SPEC_SCL2_IGS_TCAM_ENTRY_NUM) = 0;
        MCHIP_CAP(SYS_CAP_SPEC_SCL3_IGS_TCAM_ENTRY_NUM) = 0;
    }

    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_SCL, sys_usw_scl_dump_db), ret, error9);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
        extern int32 sys_usw_l3if_wb_restore_sub_if(uint8 lchip);
        CTC_ERROR_GOTO(sys_usw_l3if_wb_restore_sub_if(lchip), ret, error9);
	}

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error9:
    sal_mutex_destroy(p_usw_scl_master[lchip]->mutex);
error8:
    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        offset = p_usw_scl_master[lchip]->igs_default_base[index];
        if (offset)
        {
            sys_usw_ftm_free_table_offset(lchip, ad_tbl[index], 1, 2*SYS_SCL_DEFAULT_ENTRY_BASE, offset);
        }
    }
    offset = p_usw_scl_master[lchip]->tunnel_default_base;
    sys_usw_ftm_free_table_offset(lchip, DsTunnelId_t, 0, 2*(DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) - USERIDHASHTYPE_TUNNELIPV4 + 1), offset);
    if(!DRV_IS_DUET2(lchip))
    {
        sys_usw_ftm_free_table_offset(lchip, DsTunnelId1_t, 0, 2*(DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) - USERIDHASHTYPE_TUNNELIPV4 + 1), offset);
    }
    mem_free(p_usw_scl_master[lchip]->fpa);

    for(loop = 0; loop < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); loop++)
    {
        mem_free(p_usw_scl_master[lchip]->block[loop].fpab.entries);
    }
error7:
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_vlan_edit);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_entry_id);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_flow_acl_profile);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile);
error6:
    ctc_spool_free(p_usw_scl_master[lchip]->acl_spool);
error5:
    if (NULL != p_usw_scl_master[lchip]->ad_spool[1])
    {
        ctc_spool_free(p_usw_scl_master[lchip]->ad_spool[1]);
    }

error4:
    if (NULL != p_usw_scl_master[lchip]->ad_spool[0])
    {
        ctc_spool_free(p_usw_scl_master[lchip]->ad_spool[0]);
    }
    ctc_spool_free(p_usw_scl_master[lchip]->vlan_edit_spool);
error3:
    if (p_usw_scl_master[lchip]->tcam_entry_by_key)
    {
        ctc_hash_free(p_usw_scl_master[lchip]->tcam_entry_by_key);
    }
    ctc_hash_free(p_usw_scl_master[lchip]->hash_key_entry);
error2:
    ctc_hash_free(p_usw_scl_master[lchip]->entry);
error1:
    ctc_hash_traverse(p_usw_scl_master[lchip]->group, (hash_traversal_fn) _sys_usw_scl_free_group_node_data, NULL);
    ctc_hash_free(p_usw_scl_master[lchip]->group);
error0:
    mem_free(p_usw_scl_master[lchip]);
    return ret;
}

int32
sys_usw_scl_deinit(uint8 lchip)
{
    uint8  idx  = 0;

    LCHIP_CHECK(lchip);
    if (NULL == p_usw_scl_master[lchip])
    {
        return CTC_E_NONE;
    }
    /* free opf */
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_vlan_edit);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_entry_id);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_flow_acl_profile);
    sys_usw_opf_deinit(lchip, p_usw_scl_master[lchip]->opf_type_tunnel_acl_profile);

    /*free scl entry */
    ctc_hash_traverse(p_usw_scl_master[lchip]->entry, (hash_traversal_fn)_sys_usw_scl_free_entry_node_data, &lchip);
    ctc_hash_free(p_usw_scl_master[lchip]->entry);
    ctc_hash_free(p_usw_scl_master[lchip]->hash_key_entry);
    ctc_hash_traverse(p_usw_scl_master[lchip]->tcam_entry_by_key, (hash_traversal_fn)_sys_usw_scl_free_tcam_node_data, NULL);
    ctc_hash_free(p_usw_scl_master[lchip]->tcam_entry_by_key);

     /*free ad */
     for (idx = 0; idx < MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); idx += 1)
     {
        if(p_usw_scl_master[lchip]->ad_spool[idx])
        {
            ctc_spool_free(p_usw_scl_master[lchip]->ad_spool[idx]);
        }
     }

    /*free vlan edit spool */
    ctc_spool_free(p_usw_scl_master[lchip]->vlan_edit_spool);

    /*free acl profile spool */
    ctc_spool_free(p_usw_scl_master[lchip]->acl_spool);

    /*free scl group */
    ctc_hash_traverse(p_usw_scl_master[lchip]->group, (hash_traversal_fn)_sys_usw_scl_free_group_node_data, NULL);
    ctc_hash_free(p_usw_scl_master[lchip]->group);

    /*free fpa entries*/
    for (idx = 0; idx < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); idx++)
    {
        mem_free(p_usw_scl_master[lchip]->block[idx].fpab.entries);
    }

    /*free fpa */
    fpa_usw_free(p_usw_scl_master[lchip]->fpa);

    sal_mutex_destroy(p_usw_scl_master[lchip]->mutex);

    mem_free(p_usw_scl_master[lchip]);


    return CTC_E_NONE;
}

int32
sys_usw_scl_vlan_tag_op_translate(uint8 lchip, uint8 op_in, uint8* op_out, uint8* mode_out)
{
    if (CTC_VLAN_TAG_OP_REP_OR_ADD == op_in)
    {
        *mode_out = 1;
    }
    else
    {
        *mode_out = 0;
    }

    switch (op_in)
    {
        case CTC_VLAN_TAG_OP_NONE:
        case CTC_VLAN_TAG_OP_VALID:
            *op_out = 0;
            break;
        case CTC_VLAN_TAG_OP_REP:
        case CTC_VLAN_TAG_OP_REP_OR_ADD:
            *op_out = 1;
            break;
        case CTC_VLAN_TAG_OP_ADD:
            *op_out = 2;
            break;
        case CTC_VLAN_TAG_OP_DEL:
            *op_out = 3;
            break;
    }
    return CTC_E_NONE;
}

int32
sys_usw_scl_vlan_tag_op_untranslate(uint8 lchip, uint8 op_in, uint8 mo_in, uint8* op_out)
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

int32
sys_usw_scl_check_vlan_edit(uint8 lchip, ctc_scl_vlan_edit_t* p_sys, uint8* do_nothing)
{
    /* check tag op */
    CTC_MAX_VALUE_CHECK(p_sys->vlan_domain, CTC_SCL_VLAN_DOMAIN_MAX - 1);
    CTC_MAX_VALUE_CHECK(p_sys->stag_op, CTC_VLAN_TAG_OP_MAX - 1);
    CTC_MAX_VALUE_CHECK(p_sys->ctag_op, CTC_VLAN_TAG_OP_MAX - 1);

    if ((CTC_VLAN_TAG_OP_NONE == p_sys->stag_op)
        && (CTC_VLAN_TAG_OP_NONE == p_sys->ctag_op))
    {
        *do_nothing = 1;
        return CTC_E_NONE;
    }

    *do_nothing = 0;

    if ((CTC_VLAN_TAG_OP_ADD == p_sys->stag_op) \
        || (CTC_VLAN_TAG_OP_REP == p_sys->stag_op)
        || (CTC_VLAN_TAG_OP_REP_OR_ADD == p_sys->stag_op))
    {
        /* check stag select */
        if ((p_sys->svid_sl >= CTC_VLAN_TAG_SL_MAX)    \
            || (p_sys->scos_sl >= CTC_VLAN_TAG_SL_MAX) \
            || (p_sys->scfi_sl >= CTC_VLAN_TAG_SL_MAX))
        {
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->svid_sl)
        {
            if (p_sys->svid_new > CTC_MAX_VLAN_ID)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");
			return CTC_E_BADID;

            }
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->scos_sl)
        {
            CTC_COS_RANGE_CHECK(p_sys->scos_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->scfi_sl)
        {
            CTC_MAX_VALUE_CHECK(p_sys->scfi_new, 1);
        }
    }

    if ((CTC_VLAN_TAG_OP_ADD == p_sys->ctag_op) \
        || (CTC_VLAN_TAG_OP_REP == p_sys->ctag_op)
        || (CTC_VLAN_TAG_OP_REP_OR_ADD == p_sys->ctag_op))
    {
        /* check ctag select */
        if ((p_sys->cvid_sl >= CTC_VLAN_TAG_SL_MAX)    \
            || (p_sys->ccos_sl >= CTC_VLAN_TAG_SL_MAX) \
            || (p_sys->ccfi_sl >= CTC_VLAN_TAG_SL_MAX))
        {
            return CTC_E_INVALID_PARAM;
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->cvid_sl)
        {
            if (p_sys->cvid_new > CTC_MAX_VLAN_ID)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");
			return CTC_E_BADID;

            }
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->ccos_sl)
        {
            CTC_COS_RANGE_CHECK(p_sys->ccos_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == p_sys->ccfi_sl)
        {
            CTC_MAX_VALUE_CHECK(p_sys->ccfi_new, 1);
        }
    }
    CTC_MAX_VALUE_CHECK(p_sys->tpid_index, 3);

    return CTC_E_NONE;
}

int32
sys_usw_scl_get_acl_control_profile(uint8 lchip, uint8 is_tunnel, uint8* lkup_num, ctc_acl_property_t* p_acl_profile, void* p_buffer)
{
    uint8 offset          = 0;
    uint8 step            = 0;
    uint8 gport_type      = 0;
    uint8 sys_lkup_type   = 0;
    uint8 use_mapped_vlan = 0;
    uint8  loop           = 0;

    *lkup_num = 0;

    for(loop=0; loop < CTC_MAX_ACL_LKUP_NUM; loop++)
    {
        if (!is_tunnel)
        {
            step = DsSclAclControlProfile_gAcl_1_aclLookupType_f - DsSclAclControlProfile_gAcl_0_aclLookupType_f;
            offset = step * loop;
            sys_lkup_type = GetDsSclAclControlProfile(V, gAcl_0_aclLookupType_f + offset, p_buffer);
            gport_type = GetDsSclAclControlProfile(V, gAcl_0_aclUseGlobalPortType_f + offset, p_buffer);
            use_mapped_vlan = GetDsSclAclControlProfile(V, gAcl_0_aclUsePIVlan_f + offset, p_buffer);
        }
        else
        {
            step = DsTunnelAclControlProfile_gAcl_1_aclLookupType_f - DsTunnelAclControlProfile_gAcl_0_aclLookupType_f;
            offset = step * loop;
            sys_lkup_type = GetDsTunnelAclControlProfile(V, gAcl_0_aclLookupType_f + offset, p_buffer);
            gport_type = GetDsTunnelAclControlProfile(V, gAcl_0_aclUseGlobalPortType_f + offset, p_buffer);
            use_mapped_vlan = GetDsTunnelAclControlProfile(V, gAcl_0_aclUsePIVlan_f + offset, p_buffer);
        }
        if(sys_lkup_type)
        {
            p_acl_profile[*lkup_num].acl_en = 1;
            p_acl_profile[*lkup_num].acl_priority = loop;
            p_acl_profile[*lkup_num].tcam_lkup_type = sys_usw_unmap_acl_tcam_lkup_type(lchip, sys_lkup_type);
            if(use_mapped_vlan)
            {
                CTC_SET_FLAG(p_acl_profile[*lkup_num].flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
            }

            switch(gport_type)
            {
                case DRV_FLOWPORTTYPE_BITMAP:
                    CTC_SET_FLAG(p_acl_profile[*lkup_num].flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
                    break;
                case DRV_FLOWPORTTYPE_LPORT:
                    CTC_SET_FLAG(p_acl_profile[*lkup_num].flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
                    break;
                case DRV_FLOWPORTTYPE_METADATA:
                    CTC_SET_FLAG(p_acl_profile[*lkup_num].flag, CTC_ACL_PROP_FLAG_USE_METADATA);
                    break;
                default:
                    break;
            }
            (*lkup_num)++;
        }
    }

    return CTC_E_NONE;
}

void*
sys_usw_scl_add_vlan_edit_action_profile(uint8 lchip, ctc_scl_vlan_edit_t* vlan_edit, void* old_vlan_edit_profile)
{
    int32  ret = 0;
    uint8  do_nothing = 0;
    sys_scl_sp_vlan_edit_t  new_vlan_edit;
    sys_scl_sp_vlan_edit_t* out_vlan_edit = NULL;

    sal_memset(&new_vlan_edit, 0, sizeof(sys_scl_sp_vlan_edit_t));

    ret = sys_usw_scl_check_vlan_edit(lchip, vlan_edit, &do_nothing);
    if (ret != CTC_E_NONE)
    {
        return NULL;
    }
    _sys_usw_scl_mapping_vlan_edit(lchip, vlan_edit, &new_vlan_edit.action_profile);

    ret = ctc_spool_add(p_usw_scl_master[lchip]->vlan_edit_spool, &new_vlan_edit, old_vlan_edit_profile, &out_vlan_edit);
    if (ret != CTC_E_NONE)
    {
        ctc_spool_remove(p_usw_scl_master[lchip]->vlan_edit_spool, out_vlan_edit, NULL);
        return NULL;
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add VlanActionProfileId: %d\n", out_vlan_edit->profile_id);

    return out_vlan_edit;
}

int32 sys_usw_scl_remove_vlan_edit_action_profile(uint8 lchip, sys_scl_sp_vlan_edit_t* p_vlan_edit)
{
    int32  ret = 0;

    SYS_SCL_INIT_CHECK();
    if (p_vlan_edit)
    {
        ret = ctc_spool_remove(p_usw_scl_master[lchip]->vlan_edit_spool, p_vlan_edit, NULL);
    }

    return ret ;
}

void*
sys_usw_scl_add_acl_control_profile(uint8 lchip,
                                     uint8 is_tunnel,
                                     uint8 lkup_num,
                                     ctc_acl_property_t acl_profile[CTC_MAX_ACL_LKUP_NUM],
                                     uint16* profile_id)
{
    int32  ret = CTC_E_NONE;
    uint8  loop = 0;

    sys_scl_action_acl_t  new_profile;
    sys_scl_action_acl_t* out_profile = NULL;

    sal_memset(&new_profile, 0, sizeof(sys_scl_action_acl_t));


    for (loop = 0 ; loop < lkup_num; loop++)
    {
        if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == acl_profile[loop].tcam_lkup_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Acl not support tcam vlan key\n");
            return NULL;
        }
        _sys_usw_scl_mapping_acl_profile(lchip, is_tunnel, &acl_profile[loop], &new_profile.acl_control_profile);
    }

    new_profile.is_scl = !is_tunnel;

    ret = ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &new_profile, NULL, &out_profile);
    if (ret != CTC_E_NONE)
    {
        ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, out_profile, NULL);
        return NULL;
    }
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add acl control profile: %d\n", out_profile->profile_id);
    return out_profile ;
}

int32
sys_usw_scl_remove_acl_control_profile(uint8 lchip, sys_scl_action_acl_t* p_acl_profile)
{
    int32  ret = CTC_E_NONE;

    SYS_SCL_INIT_CHECK();
    if (p_acl_profile)
    {
        ret = ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, p_acl_profile, NULL);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove acl_control_profile: %d\n", p_acl_profile->profile_id);
    }

    return ret ;
}

