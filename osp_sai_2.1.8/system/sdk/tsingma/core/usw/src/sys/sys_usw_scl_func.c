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
#include "ctc_field.h"
#include "ctc_acl.h"
#include "ctc_security.h"

#include "sys_usw_fpa.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_register.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_l3if.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_scl.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_aps.h"
#include "sys_usw_acl_api.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"


#define SYS_USW_SCL_BITS_SET(flag, bits, shift)      ((flag) = (flag) | ((bits) << (shift)))
#define SYS_USW_SCL_BITS_UNSET(flag, bits, shift)    ((flag) = (flag) & (~((bits) << (shift))))

#define SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, old_type, new_type) \
    if(!is_add && CTC_BMP_ISSET(pe->action_bmp, old_type) && (pAction->type == new_type))\
    {\
        CTC_ERROR_RETURN(CTC_E_INVALID_CONFIG);\
    }

/**< [TM] Config action type in scl instead of port */
enum sys_scl_action_type_e
{
    SYS_SCL_AD_TYPE_NONE = 0,
    SYS_SCL_AD_TYPE_SCL,
    SYS_SCL_AD_TYPE_FLOW,
    SYS_SCL_AD_TYPE_TUNNEL,
    SYS_SCL_AD_TYPE_MAX
};

/**
  flag: 0x4-DenyBridge, 0x2-ForceBridge, 0x1-StatsPtr, 0x6-LogicPort
*/
enum sys_scl_flow_union_type_e
{
    SYS_SCL_UNION_TYPE_STATS,
    SYS_SCL_UNION_TYPE_FORCEBRIDGE,
    SYS_SCL_UNION_TYPE_DENYBRIDGE,
    SYS_SCL_UNION_TYPE_LOGICPORT,
    SYS_SCL_UNION_TYPE_MAX
};
typedef enum sys_scl_flow_union_type_e sys_scl_flow_union_type_t;
STATIC int32
_sys_usw_scl_chk_logic_port_union(uint8 lchip, void* ds_sclflow, sys_scl_flow_union_type_t un_type)
{
    uint8 op_valid = 0;
    uint8 flag = 0;

    GetDsSclFlow(V, denyBridge_f, ds_sclflow)? CTC_BIT_SET(flag, 2): CTC_BIT_UNSET(flag, 2);
    GetDsSclFlow(V, forceBridge_f, ds_sclflow)? CTC_BIT_SET(flag, 1): CTC_BIT_UNSET(flag, 1);
    GetDsSclFlow(V, statsPtr_f, ds_sclflow)? CTC_BIT_SET(flag, 0): CTC_BIT_UNSET(flag, 0);

    switch(flag)
    {
        case 0: /*support all*/
            op_valid = TRUE;
            break;
        case 1: /*support: add deny bridge, add force bridge, update stats*/
            op_valid = (un_type != SYS_SCL_UNION_TYPE_LOGICPORT)? TRUE: FALSE;
            break;
        case 2: /*support: update force bridge, add stats*/
        case 3: /*support: update force bridge, update stats*/
            op_valid = (un_type == SYS_SCL_UNION_TYPE_FORCEBRIDGE || un_type == SYS_SCL_UNION_TYPE_STATS)? TRUE: FALSE;
            break;
        case 4: /*support: update deny bridge, add stats*/
        case 5: /*support: update deny bridge, update stats*/
            op_valid = (un_type == SYS_SCL_UNION_TYPE_DENYBRIDGE || un_type == SYS_SCL_UNION_TYPE_STATS)? TRUE: FALSE;
            break;
        case 6: /*support: add logic port*/
        case 7: /*support: update logic port*/
            op_valid = (un_type == SYS_SCL_UNION_TYPE_LOGICPORT)? TRUE: FALSE;
            break;
        default:
            op_valid = FALSE;
            break;
    }

    return op_valid;
}

STATIC int32
_sys_usw_scl_set_udf_mpls_key_field(uint8 lchip, ctc_field_key_t* pKey, uint8 key_share_mode, void* p_data, void* p_mask, uint8 is_add)
{
    uint32 field_id = 0;
    uint8  data_offset = 0;
    uint32 tmp_data[2] = {0};
    uint32 tmp_mask[2] = {0};
    uint32 field_max_value = 0;
    uint8  field_shift = 0;
    uint16 key_field = 0;

    CTC_PTR_VALID_CHECK(pKey);
    key_field = pKey->type;

    /*Select key field*/
    switch(key_field)
    {
        case CTC_FIELD_KEY_MPLS_LABEL2:
        case CTC_FIELD_KEY_MPLS_EXP2:
        case CTC_FIELD_KEY_MPLS_TTL2:
        case CTC_FIELD_KEY_MPLS_SBIT2:
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            field_id = DsScl0UdfKey320_u_g1_ipDa_f;
            data_offset = 0;
            break;
        case CTC_FIELD_KEY_MPLS_LABEL0:
        case CTC_FIELD_KEY_MPLS_EXP0:
        case CTC_FIELD_KEY_MPLS_TTL0:
        case CTC_FIELD_KEY_MPLS_SBIT0:
            if (key_share_mode < 2)
            {
                field_id = DsScl0UdfKey320_u_g1_ipSa_f;
            }
            else if (key_share_mode == 3)
            {
                field_id = DsScl0UdfKey320_u_g3_ipSa_f;
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode mustn't be 2! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            data_offset = 0;
            break;
        case CTC_FIELD_KEY_MPLS_LABEL1:
        case CTC_FIELD_KEY_MPLS_EXP1:
        case CTC_FIELD_KEY_MPLS_TTL1:
        case CTC_FIELD_KEY_MPLS_SBIT1:
            if (key_share_mode < 2)
            {
                field_id = DsScl0UdfKey320_u_g1_ipSa_f;
            }
            else if (key_share_mode == 3)
            {
                field_id = DsScl0UdfKey320_u_g3_ipSa_f;
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode mustn't be 2! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            data_offset = 1;
            break;
    }

    /*Get field max value and field shift*/
    switch(key_field)
    {
        case CTC_FIELD_KEY_MPLS_LABEL2:
        case CTC_FIELD_KEY_MPLS_LABEL1:
        case CTC_FIELD_KEY_MPLS_LABEL0:
            field_max_value = 0xFFFFF;
            field_shift = 12;
            break;
        case CTC_FIELD_KEY_MPLS_EXP2:
        case CTC_FIELD_KEY_MPLS_EXP1:
        case CTC_FIELD_KEY_MPLS_EXP0:
            field_max_value = 0x7;
            field_shift = 9;
            break;
        case CTC_FIELD_KEY_MPLS_SBIT2:
        case CTC_FIELD_KEY_MPLS_SBIT1:
        case CTC_FIELD_KEY_MPLS_SBIT0:
            field_max_value = 0x1;
            field_shift = 8;
            break;
        case CTC_FIELD_KEY_MPLS_TTL2:
        case CTC_FIELD_KEY_MPLS_TTL1:
        case CTC_FIELD_KEY_MPLS_TTL0:
            field_max_value = 0xFF;
            field_shift = 0;
            break;
        default :
            field_max_value = 0;
            field_shift = 0;
            break;
    }

    CTC_MAX_VALUE_CHECK(pKey->data, field_max_value);

    CTC_ERROR_RETURN(drv_get_field(lchip, DsScl0UdfKey320_t, field_id, p_data, tmp_data));
    CTC_ERROR_RETURN(drv_get_field(lchip, DsScl0UdfKey320_t, field_id, p_mask, tmp_mask));

    SYS_USW_SCL_BITS_UNSET(tmp_data[data_offset], field_max_value, field_shift);
    SYS_USW_SCL_BITS_UNSET(tmp_mask[data_offset], field_max_value, field_shift);
    if (is_add)
    {
        SYS_USW_SCL_BITS_SET(tmp_data[data_offset], pKey->data, field_shift);
        SYS_USW_SCL_BITS_SET(tmp_mask[data_offset], pKey->mask, field_shift);
    }

    CTC_ERROR_RETURN(drv_set_field(lchip, DsScl0UdfKey320_t, field_id, p_data, tmp_data));
    CTC_ERROR_RETURN(drv_set_field(lchip, DsScl0UdfKey320_t, field_id, p_mask, tmp_mask));

    return CTC_E_NONE;
}

STATIC uint8
_sys_usw_scl_map_hw_ad_type(uint8 ad_typee)
{

    if(ad_typee == SYS_SCL_ACTION_INGRESS) return 0;
    else if(ad_typee == SYS_SCL_ACTION_TUNNEL) return 1;
    else if(ad_typee == SYS_SCL_ACTION_FLOW) return 2;

return 0xF;
}

STATIC int32
_sys_usw_scl_get_ip_frag(uint8 lchip, uint8 ctc_ip_frag, uint32* frag_info, uint32* frag_info_mask)
{
    switch (ctc_ip_frag)
    {
    case CTC_IP_FRAG_NON:
        /* Non fragmented */
        *frag_info      = 0;
        *frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_FIRST:
        /* Fragmented, and first fragment */
        *frag_info      = 1;
        *frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_NON_OR_FIRST:
        /* Non fragmented OR Fragmented and first fragment */
        *frag_info      = 0;
        *frag_info_mask = 2;     /* mask frag_info as 0x */
        break;

    case CTC_IP_FRAG_SMALL:
        /* Small fragment */
        *frag_info      = 2;
        *frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_NOT_FIRST:
        /* Not first fragment (Fragmented of cause) */
        *frag_info      = 3;
        *frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_YES:
        /* Any Fragmented */
        *frag_info      = 1;
        *frag_info_mask = 1;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
extern int32
sys_usw_vlan_overlay_get_fid (uint8 lchip,  uint32 vn_id, uint16* p_fid );

extern int32
_sys_usw_scl_recovery_db_from_hw(uint8 lchip, sys_scl_sw_entry_t* pe, sys_scl_sw_temp_entry_t* p_temp_entry);

int32
sys_usw_scl_unmapping_vlan_edit(uint8 lchip, ctc_scl_vlan_edit_t* p_vlan_edit,void* p_ds_edit)
{
    uint8 op = 0;
    uint8 mo = 0;

    CTC_PTR_VALID_CHECK(p_vlan_edit);
    CTC_PTR_VALID_CHECK(p_ds_edit);

    op = GetDsVlanActionProfile(V, sTagAction_f, p_ds_edit);
    mo = GetDsVlanActionProfile(V, stagModifyMode_f, p_ds_edit);
    CTC_ERROR_RETURN(sys_usw_scl_vlan_tag_op_untranslate(lchip, op, mo, &(p_vlan_edit->stag_op)));

    op = GetDsVlanActionProfile(V, cTagAction_f, p_ds_edit);
    mo = GetDsVlanActionProfile(V, ctagModifyMode_f, p_ds_edit);
    CTC_ERROR_RETURN(sys_usw_scl_vlan_tag_op_untranslate(lchip, op, mo, &(p_vlan_edit->ctag_op)))

    p_vlan_edit->svid_sl = GetDsVlanActionProfile(V, sVlanIdAction_f, p_ds_edit);
    p_vlan_edit->scos_sl = GetDsVlanActionProfile(V, sCosAction_f, p_ds_edit);
    p_vlan_edit->scfi_sl = GetDsVlanActionProfile(V, sCfiAction_f, p_ds_edit);

    p_vlan_edit->cvid_sl = GetDsVlanActionProfile(V, cVlanIdAction_f, p_ds_edit);
    p_vlan_edit->ccos_sl = GetDsVlanActionProfile(V, cCosAction_f, p_ds_edit);
    p_vlan_edit->ccfi_sl = GetDsVlanActionProfile(V, cCfiAction_f, p_ds_edit);
    p_vlan_edit->tpid_index = GetDsVlanActionProfile(V, svlanTpidIndex_f, p_ds_edit);

    switch(GetDsVlanActionProfile(V, outerVlanStatus_f, p_ds_edit))
    {
        case 0:
            p_vlan_edit->vlan_domain = CTC_SCL_VLAN_DOMAIN_UNCHANGE;
            break;
        case 1:
            p_vlan_edit->vlan_domain = CTC_SCL_VLAN_DOMAIN_CVLAN;
            break;
        case 2:
            p_vlan_edit->vlan_domain = CTC_SCL_VLAN_DOMAIN_SVLAN;
            break;
        default:
            return CTC_E_INVALID_PARAM;

    }

    return CTC_E_NONE;

}

#define _____build_key_____

/* Hash Entry */

STATIC int32
_sys_usw_scl_build_hash_key_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data = NULL;
    void*         p_mask = NULL;
    ctc_field_port_t* p_port = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserIdPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdPortHashKey(V, isLabel_f, p_data, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                			return CTC_E_NOT_SUPPORT;
                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_globalSrcPort_f);
                    }
                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLabel_f, p_mask, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_mask, 0xFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLabel_f, p_mask, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_globalSrcPort_f, p_mask, 0xFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserPort_isLogicPort_f, p_mask, 0);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                			return CTC_E_NOT_SUPPORT;
                        }
                    }

                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_PORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_PORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
    			return CTC_E_NOT_SUPPORT;

        }
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            			return CTC_E_NOT_SUPPORT;
                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_globalDestPort_f);
                }
                break;

            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_PORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }

    return CTC_E_NONE;
}

#define SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, is_stag) \
do\
{\
    tag_valid = GetDsScl0MacKey160(V, vlanId_f, p_mask) || GetDsScl0MacKey160(V, cos_f, p_mask) || \
                    GetDsScl0MacKey160(V, cfi_f, p_mask) || GetDsScl0MacKey160(V, vlanIdValid_f, p_mask);\
    SetDsScl0MacKey160(V, isStag_f, p_data, tag_valid? is_stag: 0);\
    SetDsScl0MacKey160(V, isStag_f, p_mask, tag_valid? 1 : 0);\
}while(0)

STATIC int32
_sys_usw_scl_build_hash_key_cvlan_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data      = NULL;
    void*         p_mask      = NULL;
    ctc_field_port_t* p_port  = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdCvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdCvlanPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdCvlanPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdCvlanPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdCvlanPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserIdCvlanPortHashKey(V, isLabel_f, p_data, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdCvlanPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdCvlanPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserIdCvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                			return CTC_E_NOT_SUPPORT;
                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_globalSrcPort_f);
                    }
                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLabel_f, p_mask, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                			return CTC_E_NOT_SUPPORT;
                        }
                    }

                }
                break;
            case CTC_FIELD_KEY_CVLAN_ID:
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdCvlanPortHashKey(V, cvlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_cvlanId_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_cvlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserCvlanPort_cvlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCvlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdCvlanPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_CVLANPORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CVLANPORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCvlanPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        switch (pKey->type)
        {
             case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamCvlanPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamCvlanPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamCvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamCvlanPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            			return CTC_E_NOT_SUPPORT;
                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_globalDestPort_f);
                }
                break;
            case CTC_FIELD_KEY_CVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamCvlanPortHashKey(V, cvlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_cvlanId_f);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamCvlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamCvlanPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_CVLANPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
    			return CTC_E_NOT_SUPPORT;
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_svlan_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data = NULL;
    void*         p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdSvlanPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserIdSvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdSvlanPortHashKey(V, isLabel_f, p_data, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdSvlanPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdSvlanPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdSvlanPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdSvlanPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdSvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdSvlanPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                            return CTC_E_NOT_SUPPORT;
                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_globalSrcPort_f);
                    }

                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f , p_mask, 0xFFFFFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLabel_f, p_mask, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                            return CTC_E_NOT_SUPPORT;
                        }
                    }

                }
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdSvlanPortHashKey(V, svlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_svlanId_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_svlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanPort_svlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdSvlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdSvlanPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_SVLANPORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_SVLANPORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdSvlanPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamSvlanPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamSvlanPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamSvlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamSvlanPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_globalDestPort_f);
                }
                break;

            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamSvlanPortHashKey(V, svlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_svlanId_f);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamSvlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamSvlanPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_SVLANPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_2vlan_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdDoubleVlanPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdDoubleVlanPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdDoubleVlanPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_globalSrcPort_f);
                    }
                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLabel_f, p_mask, 0);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                    }
                }
                break;

            case CTC_FIELD_KEY_CVLAN_ID:
                CTC_ERROR_RETURN(SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP != pe->key_type? CTC_E_NONE: CTC_E_NOT_SUPPORT);
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdDoubleVlanPortHashKey(V, cvlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_cvlanId_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_cvlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_cvlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_IP_DSCP:
                CTC_ERROR_RETURN(!DRV_IS_DUET2(lchip) && SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP == pe->key_type? CTC_E_NONE: CTC_E_NOT_SUPPORT);
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsUserIdDoubleVlanPortHashKey(V, cvlanId_f, p_data, pKey->data|0x40);     /*bit6:dscp valid, bit5-0:dscp*/
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_cvlanId_f);
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdDoubleVlanPortHashKey(V, svlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_svlanId_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_svlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserDoubleVlan_svlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdDoubleVlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdDoubleVlanPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_DOUBLEVLANPORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdDoubleVlanPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_DOUBLEVLANPORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdDoubleVlanPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamDoubleVlanPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                        return CTC_E_NOT_SUPPORT;
                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_globalDestPort_f);
                }
                break;

            case CTC_FIELD_KEY_CVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamDoubleVlanPortHashKey(V, cvlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_cvlanId_f);
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamDoubleVlanPortHashKey(V, svlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_svlanId_f);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamDoubleVlanPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamDoubleVlanPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_DOUBLEVLANPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                return CTC_E_NOT_SUPPORT;
                break;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_build_hash_key_cvlan_cos_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdCvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdCvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdCvlanCosPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdCvlanCosPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdCvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdCvlanCosPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdCvlanCosPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdCvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdCvlanCosPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_globalSrcPort_f);
                    }

                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_mask, 0xFFFFFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                    }
                }
                break;

            case CTC_FIELD_KEY_CVLAN_ID:
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdCvlanCosPortHashKey(V, cvlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_cvlanId_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey80(V, cvlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, cvlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_CTAG_COS:
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserIdCvlanCosPortHashKey(V, ctagCos_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_ctagCos_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserId0TcamKey80(V, ctagCos_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, ctagCos_f, p_mask, 0x7);
                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCvlanCosPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdCvlanCosPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_CVLANCOSPORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdCvlanCosPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CVLANCOSPORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCvlanCosPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamCvlanCosPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_globalDestPort_f);
                }
                break;

            case CTC_FIELD_KEY_CVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamCvlanCosPortHashKey(V, cvlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_cvlanId_f);
                break;
            case CTC_FIELD_KEY_CTAG_COS:
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsEgressXcOamCvlanCosPortHashKey(V, ctagCos_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_ctagCos_f);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamCvlanCosPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamCvlanCosPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_CVLANCOSPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_build_hash_key_svlan_cos_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    if (CTC_INGRESS == pe->direction)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdSvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdSvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdSvlanCosPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdSvlanCosPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdSvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdSvlanCosPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdSvlanCosPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdSvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdSvlanCosPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_globalSrcPort_f);
                    }
                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                    }
                }
                break;

            case CTC_FIELD_KEY_SVLAN_ID:
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdSvlanCosPortHashKey(V, svlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_svlanId_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_svlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanCosPort_svlanId_f, p_mask, 0xFFF);
                }
                break;
            case CTC_FIELD_KEY_STAG_COS:
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserIdSvlanCosPortHashKey(V, stagCos_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_stagCos_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserId0TcamKey80(V, stagCos_f, p_data, pKey->data);
                    SetDsUserId0TcamKey80(V, stagCos_f, p_mask, 0x7);
                }
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdSvlanCosPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdSvlanCosPortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_SVLANCOSPORT);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_hashType_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanCosPortHashKey_dsAdIndex_f);
                }
                else
                {
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                    SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                    SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_SVLANCOSPORT);
                    SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                }
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdSvlanCosPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_DST_GPORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLabel_f, p_data, 0);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLabel_f, p_data, 1);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsEgressXcOamSvlanCosPortHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_globalDestPort_f);
                }
                break;

            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsEgressXcOamSvlanCosPortHashKey(V, svlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_svlanId_f);
                break;
            case CTC_FIELD_KEY_STAG_COS:
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsEgressXcOamSvlanCosPortHashKey(V, stagCos_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_stagCos_f);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamSvlanCosPortHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamSvlanCosPortHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_SVLANCOSPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_mac(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void*      p_data = NULL;
    void*      p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_MAC_SA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdMacHashKey(V, isMacDa_f, p_data, 0);
                SetDsUserIdMacHashKey(A, macSa_f, p_data, hw_mac);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_isMacDa_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_macSa_f);
            }
            else
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, isMacDa_f, p_data, 0);
                    SetDsUserId0TcamKey80(V, isMacDa_f, p_mask, 1);
                }

                SetDsUserId0TcamKey80(A, macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, macSa_f, p_mask, hw_mac);
            }
            CTC_BIT_UNSET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_MAC_DA);
            break;
        case CTC_FIELD_KEY_MAC_DA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserIdMacHashKey(V, isMacDa_f, p_data, 1);
                }
                SetDsUserIdMacHashKey(A, macSa_f, p_data, hw_mac);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_isMacDa_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_macSa_f);
            }
            else
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, isMacDa_f, p_data, 1);
                    SetDsUserId0TcamKey80(V, isMacDa_f, p_mask, 1);
                }

                SetDsUserId0TcamKey80(A, macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, macSa_f, p_mask, hw_mac);
            }
            CTC_BIT_UNSET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_MAC_SA);
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdMacHashKey(V, valid_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_valid_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdMacHashKey(V, hashType_f, p_data, USERIDHASHTYPE_MAC);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_hashType_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_MAC);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdMacHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_mac_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t                hw_mac = { 0 };
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;
    uint32 cmd = 0;
    uint32 field_val = 0;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                if (!pe->resolve_conflict)
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserIdMacPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdMacPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdMacPortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserIdMacPortHashKey(V, isLogicPort_f, p_data, 1);
                            SetDsUserIdMacPortHashKey(V, isLabel_f, p_data, 0);
                            SetDsUserIdMacPortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserIdMacPortHashKey(V, isLabel_f, p_data, 1);
                            SetDsUserIdMacPortHashKey(V, isLogicPort_f, p_data, 0);
                            SetDsUserIdMacPortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_isLogicPort_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_isLabel_f);
                        CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_globalSrcPort_f);
                    }

                }
                else
                {
                    if (is_add)
                    {
                        if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                        {
                            SYS_GLOBAL_PORT_CHECK(p_port->gport);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                        {
                            CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLogicPort_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLogicPort_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLabel_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_data, p_port->logic_port);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                        {
                            SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLabel_f, p_data, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLabel_f, p_mask, 1);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_isLogicPort_f, p_mask, 0);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_data, p_port->port_class_id);
                            SetDsUserId0TcamKey80(V, u1_gUserMacPort_globalSrcPort_f, p_mask, 0xFFFF);
                        }
                        else
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                        }

                    }
                }
                break;

            case CTC_FIELD_KEY_MAC_SA:
                cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_macPortTypeMacSaFieldShareType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                if (1 == (field_val&0x1))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port mac current mode, not support to macsa \n");
                    return CTC_E_NOT_SUPPORT;
                }
                if (!pe->resolve_conflict)
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    SetDsUserIdMacPortHashKey(V, isMacDa_f, p_data, 0);
                    SetDsUserIdMacPortHashKey(A, macSa_f, p_data, hw_mac);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_isMacDa_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSa_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSaFieldShareType_f);
                }
                else
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    if (is_add)
                    {
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_data, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_mask, 1);
                    }

                    SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                    SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_mask, hw_mac);
                }
                CTC_BIT_UNSET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_MAC_DA);
            break;
        case CTC_FIELD_KEY_MAC_DA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserIdMacPortHashKey(V, isMacDa_f, p_data, 1);
                }

                SetDsUserIdMacPortHashKey(A, macSa_f, p_data, hw_mac);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_isMacDa_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSa_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSaFieldShareType_f);
            }
            else
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_data, 1);
                    SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_mask, 1);
                }

                SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_mask, hw_mac);
            }
            CTC_BIT_UNSET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_MAC_SA);
            break;
        case CTC_FIELD_KEY_CUSTOMER_ID:
            {
                cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_macPortTypeMacSaFieldShareType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                if (0 == (field_val&0x1))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port mac current mode, not support to customer id \n");
                    return CTC_E_NOT_SUPPORT;
                }
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                hw_mac[0] = pKey->data;
                if (!pe->resolve_conflict)
                {
                    SetDsUserIdMacPortHashKey(V, isMacDa_f, p_data, 0);
                    SetDsUserIdMacPortHashKey(V, macSaFieldShareType_f, p_data, 1);
                    SetDsUserIdMacPortHashKey(A, macSa_f, p_data, hw_mac);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_isMacDa_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSa_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_macSaFieldShareType_f);
                }
                else
                {
                    if (is_add)
                    {
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_data, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_mask, 1);
                        SetDsUserId0TcamKey80(V, macSaFieldShareType_f, p_data, 1);
                        SetDsUserId0TcamKey80(V, macSaFieldShareType_f, p_mask, 1);
                    }
                    else
                    {
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_data, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserMacPort_isMacDa_f, p_mask, 0);
                        SetDsUserId0TcamKey80(V, macSaFieldShareType_f, p_data, 0);
                        SetDsUserId0TcamKey80(V, macSaFieldShareType_f, p_mask, 0);
                    }
                    SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                    SetDsUserId0TcamKey80(A, u1_gUserMacPort_macSa_f, p_mask, hw_mac);
                }
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdMacPortHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdMacPortHashKey(V, valid1_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_valid0_f);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_valid1_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdMacPortHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_MACPORT);
                SetDsUserIdMacPortHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_MACPORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_hashType0_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_hashType1_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdMacPortHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_MACPORT);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdMacPortHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_build_hash_key_svlan(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data      = NULL;
    void*         p_mask      = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch(pKey->type)
    {
        case CTC_FIELD_KEY_SVLAN_ID:
            SYS_SCL_VLAN_ID_CHECK(pKey->data);
            if(!pe->resolve_conflict)
            {
                SetDsUserIdSvlanHashKey(V, svlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanHashKey_svlanId_f);
            }
            else
            {
                SetDsUserIdTcamKey80(V, u1_gUserSvlan_svlanId_f, p_data, pKey->data);
                SetDsUserIdTcamKey80(V, u1_gUserSvlan_svlanId_f, p_mask, 0xFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdSvlanHashKey(V, valid_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanHashKey_valid_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdSvlanHashKey(V, hashType_f, p_data, USERIDHASHTYPE_SVLAN);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanHashKey_hashType_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_SVLAN);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdSvlanHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_build_hash_key_svlan_mac(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data      = NULL;
    void*         p_mask      = NULL;
    hw_mac_addr_t  hw_mac;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch(pKey->type)
    {
        case CTC_FIELD_KEY_SVLAN_ID:
            SYS_SCL_VLAN_ID_CHECK(pKey->data);
            if(!pe->resolve_conflict)
            {
                SetDsUserIdSvlanMacSaHashKey(V, svlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_svlanId_f);
            }
            else
            {
                SetDsUserIdTcamKey80(V, u1_gUserSvlanMacSa_svlanId_f, p_data, pKey->data);
                SetDsUserIdTcamKey80(V, u1_gUserSvlanMacSa_svlanId_f, p_mask, 0xFFF);
            }
            break;
        case CTC_FIELD_KEY_MAC_SA:
        case CTC_FIELD_KEY_MAC_DA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdSvlanMacSaHashKey(V, isMacDa_f, p_data, (pKey->type==CTC_FIELD_KEY_MAC_DA)?1:0);
                SetDsUserIdSvlanMacSaHashKey(A, macSa_f, p_data, hw_mac);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_isMacDa_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_macSa_f);
            }
            else
            {
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanMacSa_isMacDa_f, p_data, 0);
                    SetDsUserId0TcamKey80(V, u1_gUserSvlanMacSa_isMacDa_f, p_mask, 1);
                }

                SetDsUserId0TcamKey80(A, u1_gUserSvlanMacSa_macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, u1_gUserSvlanMacSa_macSa_f, p_mask, hw_mac);
            }
            CTC_BIT_UNSET(pe->key_bmp, (pKey->type==CTC_FIELD_KEY_MAC_SA)?SYS_SCL_SHOW_FIELD_KEY_MAC_DA:SYS_SCL_SHOW_FIELD_KEY_MAC_SA);
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdSvlanMacSaHashKey(V, valid_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_valid_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdSvlanMacSaHashKey(V, hashType_f, p_data, USERIDHASHTYPE_SVLANMACSA);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_hashType_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdSvlanMacSaHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_SVLANMACSA);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdSvlanMacSaHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_build_hash_key_ipv4_sa(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*      p_data = NULL;
    void*      p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv4SaHashKey(V, ipSa_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4SaHashKey_ipSa_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gUserIpv4Sa_ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gUserIpv4Sa_ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdIpv4SaHashKey(V, valid_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4SaHashKey_valid_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv4SaHashKey(V, hashType_f, p_data, USERIDHASHTYPE_IPV4SA);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4SaHashKey_hashType_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4SaHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_IPV4SA);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdIpv4SaHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_ipv4_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            p_port = (ctc_field_port_t*)(pKey->ext_data);
            if(!pe->resolve_conflict)
            {
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsUserIdIpv4PortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsUserIdIpv4PortHashKey(V, isLabel_f, p_data, 0);
                        SetDsUserIdIpv4PortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsUserIdIpv4PortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsUserIdIpv4PortHashKey(V, isLabel_f, p_data, 0);
                        SetDsUserIdIpv4PortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsUserIdIpv4PortHashKey(V, isLabel_f, p_data, 1);
                        SetDsUserIdIpv4PortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsUserIdIpv4PortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_globalSrcPort_f);
                }

            }
            else
            {
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLogicPort_f, p_mask, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLabel_f, p_mask, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_mask, 0xFFFFFFFF);
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLogicPort_f, p_data, 1);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLogicPort_f, p_mask, 1);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLabel_f, p_mask, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_data, p_port->logic_port);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_mask, 0xFFFF);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLabel_f, p_data, 1);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLabel_f, p_mask, 1);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_isLogicPort_f, p_mask, 0);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_data, p_port->port_class_id);
                        SetDsUserId0TcamKey80(V, u1_gUserIpv4Port_globalSrcPort_f, p_mask, 0xFFFF);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                }
            }
            break;

        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv4PortHashKey(V, ipSa_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_ipSa_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdIpv4PortHashKey(V, valid_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_valid_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv4PortHashKey(V, hashType_f, p_data, USERIDHASHTYPE_IPV4PORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_hashType_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv4PortHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_IPV4PORT);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdIpv4PortHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_ipv6_sa(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void*      p_data = NULL;
    void*      p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_IPV6_SA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserIdIpv6SaHashKey(A, ipSa_f, p_data, hw_ip6);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_ipSa_f);
            }
            else
            {
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserId0TcamKey160(A, u1_gUserIpv6Sa_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey160(A, u1_gUserIpv6Sa_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdIpv6SaHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdIpv6SaHashKey(V, valid1_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_valid0_f);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_valid1_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv6SaHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_IPV6SA);
                SetDsUserIdIpv6SaHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_IPV6SA);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_hashType0_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_hashType1_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6SaHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, hashType0_f, p_data, USERIDHASHTYPE_IPV6SA);
                SetDsUserId0TcamKey160(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, hashType1_f, p_data, USERIDHASHTYPE_IPV6SA);
                SetDsUserId0TcamKey160(V, hashType1_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdIpv6SaHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_ipv6_port(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void*      p_data = NULL;
    void*      p_mask = NULL;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            p_port = (ctc_field_port_t*)(pKey->ext_data);
            if (!pe->resolve_conflict)
            {
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsUserIdIpv6PortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsUserIdIpv6PortHashKey(V, isLabel_f, p_data, 0);
                        SetDsUserIdIpv6PortHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsUserIdIpv6PortHashKey(V, isLogicPort_f, p_data, 1);
                        SetDsUserIdIpv6PortHashKey(V, isLabel_f, p_data, 0);
                        SetDsUserIdIpv6PortHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsUserIdIpv6PortHashKey(V, isLabel_f, p_data, 1);
                        SetDsUserIdIpv6PortHashKey(V, isLogicPort_f, p_data, 0);
                        SetDsUserIdIpv6PortHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_isLogicPort_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_isLabel_f);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_globalSrcPort_f);
                }

            }
            else
            {
                if (is_add)
                {
                    if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                    {
                        SYS_GLOBAL_PORT_CHECK(p_port->gport);
                        SetDsUserId0TcamKey320(V, isLogicPort_f, p_mask, 0);
                        SetDsUserId0TcamKey320(V, isLabel_f, p_mask, 0);
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_mask, 0xFFFFFFFF);
                    }
                    else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                    {
                        CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                        SetDsUserId0TcamKey320(V, isLogicPort_f, p_data, 1);
                        SetDsUserId0TcamKey320(V, isLogicPort_f, p_mask, 1);
                        SetDsUserId0TcamKey320(V, isLabel_f, p_mask, 0);
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_data, p_port->logic_port);
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_mask, 0xFFFF);
                    }
                    else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                    {
                        SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                        SetDsUserId0TcamKey320(V, isLabel_f, p_data, 1);
                        SetDsUserId0TcamKey320(V, isLabel_f, p_mask, 1);
                        SetDsUserId0TcamKey320(V, isLogicPort_f, p_mask, 0);
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_data, p_port->port_class_id);
                        SetDsUserId0TcamKey320(V, globalSrcPort_f, p_mask, 0xFFFF);
                    }
                    else
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                    }
                }
            }
            break;

        case CTC_FIELD_KEY_IPV6_SA:
            if (!pe->resolve_conflict)
            {
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserIdIpv6PortHashKey(A, ipSa_f, p_data, hw_ip6);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_ipSa_f);
            }
            else
            {
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserId0TcamKey320(A, u1_gUserIpv6Port_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gUserIpv6Port_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdIpv6PortHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdIpv6PortHashKey(V, valid1_f, p_data, 1);
            SetDsUserIdIpv6PortHashKey(V, valid2_f, p_data, 1);
            SetDsUserIdIpv6PortHashKey(V, valid3_f, p_data, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_valid0_f);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_valid1_f);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_valid2_f);
            CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_valid3_f);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdIpv6PortHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserIdIpv6PortHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserIdIpv6PortHashKey(V, hashType2_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserIdIpv6PortHashKey(V, hashType3_f, p_data, USERIDHASHTYPE_IPV6PORT);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_hashType0_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_hashType1_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_hashType2_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_hashType3_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsUserIdIpv6PortHashKey_dsAdIndex_f);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));

                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_IPV6PORT);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdIpv6PortHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_scl_flow_l2(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    SclFlowHashFieldSelect_m   ds_sel;
    hw_mac_addr_t               hw_mac = { 0 };
    void*      p_data = NULL;
    void*      p_mask = NULL;
    uint32 cmd = 0;
    uint8 field_sel_id = 0;
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;


    field_sel_id = pe->hash_field_sel_id;
    cmd = DRV_IOR(SclFlowHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            p_port = (ctc_field_port_t*)(pKey->ext_data);
            if (GetSclFlowHashFieldSelect(V, userIdLabelEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsUserIdSclFlowL2HashKey(V, userIdLabel_f, p_data, p_port->port_class_id);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_userIdLabelEn_f);
                }
                else
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsUserId0TcamKey160(V, userIdLabel_f, p_data, p_port->port_class_id);
                    SetDsUserId0TcamKey160(V, userIdLabel_f, p_mask, 0xFFFF);
                }
            }
            break;

        case CTC_FIELD_KEY_ETHER_TYPE:
            if (GetSclFlowHashFieldSelect(V, etherTypeEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                    SetDsUserIdSclFlowL2HashKey(V, etherType_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_etherTypeEn_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                    SetDsUserId0TcamKey160(V, etherType_f, p_data, pKey->data);
                    SetDsUserId0TcamKey160(V, etherType_f, p_mask, 0xFFFFFFFF);
                }
            }
            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            if (GetSclFlowHashFieldSelect(V, svlanIdEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserIdSclFlowL2HashKey(V, svlanId_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_svlanIdEn_f);
                }
                else
                {
                    SYS_SCL_VLAN_ID_CHECK(pKey->data);
                    SetDsUserId0TcamKey160(V, svlanId_f, p_data, pKey->data);
                    SetDsUserId0TcamKey160(V, svlanId_f, p_mask, 0xFFF);
                }
            }
            break;
        case CTC_FIELD_KEY_STAG_COS:
            if (GetSclFlowHashFieldSelect(V, stagCosEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserIdSclFlowL2HashKey(V, stagCos_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_stagCosEn_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                    SetDsUserId0TcamKey160(V, stagCos_f, p_data, pKey->data);
                    SetDsUserId0TcamKey160(V, stagCos_f, p_mask, 0x7);
                }
            }
            break;
        case CTC_FIELD_KEY_STAG_CFI:
            if (GetSclFlowHashFieldSelect(V, stagCfiEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                    SetDsUserIdSclFlowL2HashKey(V, stagCfi_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_stagCfiEn_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 1);
                    SetDsUserId0TcamKey160(V, stagCfi_f, p_data, pKey->data);
                    SetDsUserId0TcamKey160(V, stagCfi_f, p_mask, 0x1);
                }
            }
            break;
        case CTC_FIELD_KEY_STAG_VALID:
            if (GetSclFlowHashFieldSelect(V, svlanIdValidEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                    SetDsUserIdSclFlowL2HashKey(V, svlanIdValid_f, p_data, pKey->data);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_svlanIdValidEn_f);
                }
                else
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                    SetDsUserId0TcamKey160(V, svlanIdValid_f, p_data, pKey->data);
                    SetDsUserId0TcamKey160(V, svlanIdValid_f, p_mask, 0x1);
                }
            }
            break;
        case CTC_FIELD_KEY_MAC_DA:
            if (GetSclFlowHashFieldSelect(V, macDaEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    SetDsUserIdSclFlowL2HashKey(A, macDa_f, p_data, hw_mac);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_macDaEn_f);
                }
                else
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    SetDsUserId0TcamKey160(A, macDa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                    SetDsUserId0TcamKey160(A, macDa_f, p_mask, hw_mac);
                }
            }
            break;
        case CTC_FIELD_KEY_MAC_SA:
            if (GetSclFlowHashFieldSelect(V, macSaEn_f, &ds_sel))
            {
                if (!pe->resolve_conflict)
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    SetDsUserIdSclFlowL2HashKey(A, macSa_f, p_data, hw_mac);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, SclFlowHashFieldSelect_macSaEn_f);
                }
                else
                {
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    if (pKey->ext_data)
                    {
                        SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    }
                    SetDsUserId0TcamKey160(A, macSa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                    SetDsUserId0TcamKey160(A, macSa_f, p_mask, hw_mac);
                }
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdSclFlowL2HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdSclFlowL2HashKey(V, valid1_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsUserIdSclFlowL2HashKey(V, flowFieldSel_f, p_data, pKey->data);

                SetDsUserIdSclFlowL2HashKey(V, hashType0_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2));
                SetDsUserIdSclFlowL2HashKey(V, hashType1_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2));
            }
            else
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsUserId0TcamKey160(V, flowFieldSel_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, flowFieldSel_f, p_mask, 0x3);
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, hashType0_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2));
                SetDsUserId0TcamKey160(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, hashType1_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2));
                SetDsUserId0TcamKey160(V, hashType1_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdSclFlowL2HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
            SetDsUserIdTunnelIpv4HashKey(V, layer4Type_f, pu32, pKey->data);
            break;
        #if 0
        case CTC_FIELD_KEY_L4_SRC_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            SetDsUserIdTunnelIpv4HashKey(V, udpSrcPort_f, pu32, pKey->data);
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            SetDsUserIdTunnelIpv4HashKey(V, udpDestPort_f, pu32, pKey->data);
            break;
        #endif
        case CTC_FIELD_KEY_IP_SA:
            SetDsUserIdTunnelIpv4HashKey(V, ipSa_f, pu32, pKey->data);
            break;
        case CTC_FIELD_KEY_IP_DA:
            SetDsUserIdTunnelIpv4HashKey(V, ipDa_f, pu32, pKey->data);
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4HashKey(V, valid1_f, pu32, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            SetDsUserIdTunnelIpv4HashKey(V, hashType0_f, pu32, USERIDHASHTYPE_TUNNELIPV4);
            SetDsUserIdTunnelIpv4HashKey(V, hashType1_f, pu32, USERIDHASHTYPE_TUNNELIPV4);
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4HashKey(V, dsAdIndex_f, pu32, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_gre(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;
    switch (pKey->type)
    {
        case CTC_FIELD_KEY_GRE_KEY:
            if(pe->resolve_conflict)
            {
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_greKey_f, pu32, pKey->data);
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_greKey_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserIdTunnelIpv4GreKeyHashKey(V, greKey_f, pu32, pKey->data);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
            if(pe->resolve_conflict)
            {
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_layer4Type_f, pu32, pKey->data);
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_layer4Type_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserIdTunnelIpv4GreKeyHashKey(V, layer4Type_f, pu32, pKey->data);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if(pe->resolve_conflict)
            {
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_ipSa_f, pu32, pKey->data);
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_ipSa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserIdTunnelIpv4GreKeyHashKey(V, ipSa_f, pu32, pKey->data);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if(pe->resolve_conflict)
            {
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_ipDa_f, pu32, pKey->data);
                SetDsUserIdTcamKey160(V, u1_gTunnelIpv4GreKey_ipDa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserIdTunnelIpv4GreKeyHashKey(V, ipDa_f, pu32, pKey->data);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, valid1_f, pu32, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, hashType0_f, pu32, USERIDHASHTYPE_TUNNELIPV4GREKEY);
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, hashType1_f, pu32, USERIDHASHTYPE_TUNNELIPV4GREKEY);
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, dsAdIndex_f, pu32, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_rpf(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4RpfHashKey(V, ipSa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Rpf_ipSa_f, p_data, pKey->data);
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Rpf_ipSa_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4RpfHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4RpfHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4RPF);
            }
            else
            {
                SetDsUserIdTcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4RPF);
                SetDsUserIdTcamKey80(V, hashType_f, p_mask, 0x3F);
                SetDsUserIdTcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4RpfHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_da(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4DaHashKey(V, layer4Type_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Da_layer4Type_f, p_data, pKey->data);
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Da_layer4Type_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4DaHashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Da_ipDa_f, p_data, pKey->data);
                SetDsUserIdTcamKey80(V, u1_gTunnelIpv4Da_ipDa_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4DaHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4DaHashKey(V, hashType_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA));
            }
            else
            {
                SetDsUserIdTcamKey80(V, hashType_f, p_data, DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA));
                SetDsUserIdTcamKey80(V, hashType_f, p_mask, 0x3F);
                SetDsUserIdTcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4DaHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_da(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;
    ipv6_addr_t               hw_ip6;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6DaHashKey(V, layer4Type_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey320(V, u1_gTunnelIpv6Da_layer4Type_f, p_data, pKey->data);
                SetDsUserIdTcamKey320(V, u1_gTunnelIpv6Da_layer4Type_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6DaHashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey320(A, u1_gTunnelIpv6Da_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserIdTcamKey320(A, u1_gTunnelIpv6Da_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6DaHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6DaHashKey(V, valid1_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6DaHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6DA);
                SetDsUserIdTunnelIpv6DaHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6DA);
            }
            else
            {
                SetDsUserIdTcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6DA);
                SetDsUserIdTcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6DA);
                SetDsUserIdTcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserIdTcamKey320(V, hashType1_f, p_mask, 0x3F);

                SetDsUserIdTcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6DaHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;
    ipv6_addr_t               hw_ip6;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(V, layer4Type_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_layer4Type_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_layer4Type_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(V, udpSrcPort_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_udpSrcPort_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_udpSrcPort_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(V, udpDestPort_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_udpDestPort_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6_udpDestPort_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTunnelIpv6HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTunnelIpv6HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTunnelIpv6HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
            }
            else
            {
                SetDsUserIdTcamKey640(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType0_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType1_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType2_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType3_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType4_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType4_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType5_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType5_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType6_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType6_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType7_f, p_data, USERIDHASHTYPE_TUNNELIPV6);
                SetDsUserIdTcamKey640(V, hashType7_f, p_mask, 0x3F);

                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}
#if 0
STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_udp(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;
    ipv6_addr_t               hw_ip6;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(V, layer4Type_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_layer4Type_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_layer4Type_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(V, udpSrcPort_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_udpSrcPort_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_udpSrcPort_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(V, udpDestPort_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_udpDestPort_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_udpDestPort_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_ipSa_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_ipSa_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_ipDa_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6Udp_ipDa_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UdpHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTunnelIpv6UdpHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTunnelIpv6UdpHashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTunnelIpv6UdpHashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
            }
            else
            {
                SetDsUserIdTcamKey640(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType0_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType1_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType2_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType3_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType4_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType5_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType6_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType7_f, p_data, USERIDHASHTYPE_TUNNELIPV6UDP);
                SetDsUserIdTcamKey640(V, hashType4_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType5_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType6_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType7_f, p_mask, 0x3F);

                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6UdpHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}
#endif

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_grekey(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;
    ipv6_addr_t               hw_ip6;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_GRE_KEY:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, greKey_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6GreKey_greKey_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6GreKey_greKey_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, layer4Type_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6GreKey_layer4Type_f, p_data, pKey->data);
                SetDsUserIdTcamKey640(V, u1_gTunnelIpv6GreKey_layer4Type_f, p_mask, pKey->mask);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6GreKeyHashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6GreKey_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6GreKey_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6GreKeyHashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6GreKey_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserIdTcamKey640(A, u1_gTunnelIpv6GreKey_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTunnelIpv6GreKeyHashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
            }
            else
            {
                SetDsUserIdTcamKey640(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType0_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType1_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType2_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType3_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType4_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType4_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType5_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType5_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType6_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType6_f, p_mask, 0x3F);
                SetDsUserIdTcamKey640(V, hashType7_f, p_data, USERIDHASHTYPE_TUNNELIPV6GREKEY);
                SetDsUserIdTcamKey640(V, hashType7_f, p_mask, 0x3F);

                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType4_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType5_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType6_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserIdTcamKey640(V, sclKeyType7_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_uc_nvgre_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode0_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode0_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0);
            }
            else
            {
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_uc_nvgre_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, ipDaIndex_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_ipDaIndex_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_ipDaIndex_f, p_mask, 0x3);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, ipSa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcNvgreMode1_ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1);
            }
            else
            {
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_mc_nvgre_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McNvgreMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McNvgreMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McNvgreMode0_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McNvgreMode0_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0);
            }
            else
            {
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_nvgre_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if(!pe->resolve_conflict)
            {
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, ipSa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4NvgreMode1_ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, valid1_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV4NVGREMODE1);
                SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV4NVGREMODE1);
            }
            else
            {
                SetDsUserId0TcamKey160(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV4NVGREMODE1);
                SetDsUserId0TcamKey160(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV4NVGREMODE1);
                SetDsUserId0TcamKey160(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_uc_nvgre_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6UcNvgreMode0_vni_f, p_data, pKey->data);
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6UcNvgreMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode0_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode0_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_uc_nvgre_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6UcNvgreMode1_vni_f, p_data, pKey->data);
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6UcNvgreMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode1_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode1_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode1_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6UcNvgreMode1_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_mc_nvgre_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6McNvgreMode0_vni_f, p_data, pKey->data);
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6McNvgreMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode0_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode0_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_mc_nvgre_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6McNvgreMode1_vni_f, p_data, pKey->data);
                SetDsUserId1TcamKey320(V, u1_gTunnelIpv6McNvgreMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode1_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode1_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode1_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId1TcamKey320(A, u1_gTunnelIpv6McNvgreMode1_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_uc_vxlan_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode0_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode0_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_uc_vxlan_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, ipDaIndex_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_ipDaIndex_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_ipDaIndex_f, p_mask, 0x3);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, ipSa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4UcVxlanMode1_ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
        if(!pe->resolve_conflict)
        {
            SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_mc_vxlan_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McVxlanMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McVxlanMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McVxlanMode0_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelIpv4McVxlanMode0_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0);
            }
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_vxlan_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, ipDa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_ipDa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, ipSa_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, u1_gTunnelIpv4VxlanMode1_ipSa_f, p_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, valid1_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
        if (!pe->resolve_conflict)
            {
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV4VXLANMODE1);
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV4VXLANMODE1);
            }
            else
            {
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV4VXLANMODE1);
                SetDsUserId0TcamKey160(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV4VXLANMODE1);
                SetDsUserId0TcamKey160(V, hashType1_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_uc_vxlan_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6UcVxlanMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6UcVxlanMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode0_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode0_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_uc_vxlan_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6UcVxlanMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6UcVxlanMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode1_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode1_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode1_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6UcVxlanMode1_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_mc_vxlan_mode0(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if(!pe->resolve_conflict)
            {
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6McVxlanMode0_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6McVxlanMode0_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McVxlanMode0HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode0_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode0_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_mc_vxlan_mode1(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* p_data = NULL;
    void* p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    p_mask = (void*)pe->temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_VN_ID:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            if(!pe->resolve_conflict)
            {
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, vni_f, p_data, pKey->data);
            }
            else
            {
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6McVxlanMode1_vni_f, p_data, pKey->data);
                SetDsUserId0TcamKey320(V, u1_gTunnelIpv6McVxlanMode1_vni_f, p_mask, 0xFFFFFF);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McVxlanMode1HashKey(A, ipDa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode1_ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode1_ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(pKey->ext_data)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
            }
            if (!pe->resolve_conflict)
            {
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(A, ipSa_f, p_data, hw_ip6);
            }
            else
            {
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode1_ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, u1_gTunnelIpv6McVxlanMode1_ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid0_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid1_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid2_f, p_data, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid3_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
            }
            else
            {
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
            }
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv4_capwap(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    if (!pe->resolve_conflict)
    {
        pu32 = (void*)pe->temp_entry->key;

        switch (pKey->type)
        {
            case CTC_FIELD_KEY_L4_DST_PORT:
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsUserIdTunnelIpv4CapwapHashKey(V, udpDestPort_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_IP_DA:
                SetDsUserIdTunnelIpv4CapwapHashKey(V, ipDa_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_IP_SA:
                SetDsUserIdTunnelIpv4CapwapHashKey(V, ipSa_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdTunnelIpv4CapwapHashKey(V, valid0_f, pu32, 1);
                SetDsUserIdTunnelIpv4CapwapHashKey(V, valid1_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdTunnelIpv4CapwapHashKey(V, hashType0_f, pu32, USERIDHASHTYPE_TUNNELIPV4CAPWAP);
                SetDsUserIdTunnelIpv4CapwapHashKey(V, hashType1_f, pu32, USERIDHASHTYPE_TUNNELIPV4CAPWAP);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdTunnelIpv4CapwapHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
         switch (pKey->type)
        {
            case CTC_FIELD_KEY_L4_DST_PORT:
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsUserId0TcamKey160(V, udpDestPort_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, udpDestPort_f, p_mask, 0xFFFF);
                break;
            case CTC_FIELD_KEY_IP_DA:
                SetDsUserId0TcamKey160(V, ipDa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, ipDa_f, p_mask, 0xFFFFFFFF);
                break;
            case CTC_FIELD_KEY_IP_SA:
                SetDsUserId0TcamKey160(V, ipSa_f, p_data, pKey->data);
                SetDsUserId0TcamKey160(V, ipSa_f, p_mask, 0xFFFFFFFF);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey160(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV4CAPWAP);
                SetDsUserId0TcamKey160(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey160(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV4CAPWAP);
                SetDsUserId0TcamKey160(V, hashType1_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_ipv6_capwap(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);
    if (!pe->resolve_conflict)
    {
        pu32 = (void*)pe->temp_entry->key;

        switch (pKey->type)
        {
            case CTC_FIELD_KEY_L4_DST_PORT:
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, udpDestPort_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_IPV6_DA:
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }

                SetDsUserIdTunnelIpv6CapwapHashKey(A, ipDa_f, pu32, hw_ip6);

                break;
            case CTC_FIELD_KEY_IPV6_SA:
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserIdTunnelIpv6CapwapHashKey(A, ipSa_f, pu32, hw_ip6);

                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdTunnelIpv6CapwapHashKey(V, valid0_f, pu32, 1);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, valid1_f, pu32, 1);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, valid2_f, pu32, 1);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, valid3_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdTunnelIpv6CapwapHashKey(V, hashType0_f, pu32, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, hashType1_f, pu32, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, hashType2_f, pu32, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserIdTunnelIpv6CapwapHashKey(V, hashType3_f, pu32, USERIDHASHTYPE_TUNNELIPV6CAPWAP);

                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdTunnelIpv6CapwapHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_L4_DST_PORT:
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsUserId0TcamKey320(V, udpDestPort_f, p_data, pKey->data);
                SetDsUserId0TcamKey320(V, udpDestPort_f, p_mask, 0xFFFF);
                break;
            case CTC_FIELD_KEY_IPV6_DA:
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserId0TcamKey320(A, ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, ipDa_f, p_mask, hw_ip6);
                break;
            case CTC_FIELD_KEY_IPV6_SA:
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsUserId0TcamKey320(A, ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0xFF, sizeof(ipv6_addr_t));
                SetDsUserId0TcamKey320(A, ipSa_f, p_mask, hw_ip6);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey320(V, hashType0_f, p_data, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserId0TcamKey320(V, hashType0_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType1_f, p_data, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserId0TcamKey320(V, hashType1_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType2_f, p_data, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserId0TcamKey320(V, hashType2_f, p_mask, 0x3F);
                SetDsUserId0TcamKey320(V, hashType3_f, p_data, USERIDHASHTYPE_TUNNELIPV6CAPWAP);
                SetDsUserId0TcamKey320(V, hashType3_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_capwap_rmac(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;
    if (!pe->resolve_conflict)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_WLAN_RADIO_MAC:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdTunnelCapwapRmacHashKey(A, radioMac_f, pu32, hw_mac);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdTunnelCapwapRmacHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdTunnelCapwapRmacHashKey(V, hashType_f, pu32, USERIDHASHTYPE_TUNNELCAPWAPRMAC);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdTunnelCapwapRmacHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_WLAN_RADIO_MAC:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, radioMac_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, radioMac_f, p_mask, hw_mac);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELCAPWAPRMAC);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_tunnel_capwap_rmac_rid(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;
    if (!pe->resolve_conflict)
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_WLAN_RADIO_ID:
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1F);
                SetDsUserIdTunnelCapwapRmacRidHashKey(V, rid_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_WLAN_RADIO_MAC:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdTunnelCapwapRmacRidHashKey(A, radioMac_f, pu32, hw_mac);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdTunnelCapwapRmacRidHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdTunnelCapwapRmacRidHashKey(V, hashType_f, pu32, USERIDHASHTYPE_TUNNELCAPWAPRMACRID);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdTunnelCapwapRmacRidHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_WLAN_RADIO_ID:
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1F);
                SetDsUserId0TcamKey80(V, rid_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, rid_f, p_mask, 0x1F);
                break;
            case CTC_FIELD_KEY_WLAN_RADIO_MAC:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, u1_gCapwapRmacRid_radioMac_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, u1_gCapwapRmacRid_radioMac_f, p_mask, hw_mac);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELCAPWAPRMACRID);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_capwap_sta_status(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;
    if (!pe->resolve_conflict)
    {
         switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_SA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdCapwapStaStatusHashKey(A, mac_f, pu32, hw_mac);
                if (is_add)
                {
                    SetDsUserIdCapwapStaStatusHashKey(V, isMacSa_f, pu32, 1);
                }
                break;
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdCapwapStaStatusHashKey(A, mac_f, pu32, hw_mac);
                SetDsUserIdCapwapStaStatusHashKey(V, isMacSa_f, pu32, 0);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCapwapStaStatusHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdCapwapStaStatusHashKey(V, hashType_f, pu32, USERIDHASHTYPE_CAPWAPSTASTATUS);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCapwapStaStatusHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_SA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, u1_gStaStatus_mac_f, p_data, hw_mac);
                if(0 != hw_mac[0] || 0 != hw_mac[1])
                {
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                }
                SetDsUserId0TcamKey80(A, u1_gStaStatus_mac_f, p_mask, hw_mac);
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_data, 1);
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_mask, 1);
                }
                break;
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, u1_gStaStatus_mac_f, p_data, hw_mac);
                if(0 != hw_mac[0] || 0 != hw_mac[1])
                {
                    sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                }
                SetDsUserId0TcamKey80(A, u1_gStaStatus_mac_f, p_mask, hw_mac);
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_data, 0);
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_mask, 1);
                }
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CAPWAPSTASTATUS);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_capwap_mc_sta_status(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    pu32 = (void*)pe->temp_entry->key;

    if (!pe->resolve_conflict)
    {
         switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdCapwapStaStatusMcHashKey(A, mac_f, pu32, hw_mac);
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsUserIdCapwapStaStatusMcHashKey(V, svlanId_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCapwapStaStatusMcHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdCapwapStaStatusMcHashKey(V, hashType_f, pu32, USERIDHASHTYPE_CAPWAPSTASTATUSMC);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCapwapStaStatusMcHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, u1_gStaStatusMc_mac_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, u1_gStaStatusMc_mac_f, p_mask, hw_mac);
                if (is_add)
                {
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_data, 0);
                    SetDsUserId0TcamKey80(V, isMacSa_f, p_mask, 1);
                }
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsUserId0TcamKey80(V, svlanId_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, svlanId_f, p_mask, 0xFFF);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CAPWAPSTASTATUSMC);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}

#if 0
STATIC int32
_sys_usw_scl_build_hash_key_capwap_macda_forward(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    if (!pe->resolve_conflict)
    {
        pu32 = (void*)pe->temp_entry->key;

        switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserIdCapwapMacDaForwardHashKey(A, mac_f, pu32, hw_mac);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCapwapMacDaForwardHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdCapwapMacDaForwardHashKey(V, hashType_f, pu32, USERIDHASHTYPE_CAPWAPMACDAFORWARD);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCapwapStaStatusHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_MAC_DA:
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if(pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsUserId0TcamKey80(A, mac_f, p_data, hw_mac);
                sal_memset(hw_mac, 0xFF, sizeof(hw_mac_addr_t));
                SetDsUserId0TcamKey80(A, mac_f, p_mask, hw_mac);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CAPWAPMACDAFORWARD);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_capwap_vlan_forward(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;
    void*         p_data = NULL;
    void*         p_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    if (!pe->resolve_conflict)
    {
        pu32 = (void*)pe->temp_entry->key;

        switch (pKey->type)
        {
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsUserIdCapwapVlanForwardHashKey(V, vlanId_f, pu32, pKey->data);
                break;
            case CTC_FIELD_KEY_HASH_VALID:
                SetDsUserIdCapwapVlanForwardHashKey(V, valid_f, pu32, 1);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserIdCapwapVlanForwardHashKey(V, hashType_f, pu32, USERIDHASHTYPE_CAPWAPVLANFORWARD);
                break;
            case SYS_SCL_FIELD_KEY_AD_INDEX:
                SetDsUserIdCapwapVlanForwardHashKey(V, dsAdIndex_f, pu32, pKey->data);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }
    }
    else
    {
        p_data = (void*)pe->temp_entry->key;
        p_mask = (void*)pe->temp_entry->mask;
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsUserId0TcamKey80(V, vlanId_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, vlanId_f, p_mask, 0xFFF);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_CAPWAPVLANFORWARD);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

                break;
        }

    }


    return CTC_E_NONE;
}
#endif

STATIC int32
_sys_usw_scl_build_hash_key_ecid(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    if (pe->resolve_conflict)
    {
	    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }


    pu32 = (void*)pe->temp_entry->key;

    switch (pKey->type)
    {
        case SYS_SCL_FIELD_KEY_NAMESPACE:
            SetDsUserIdEcidNameSpaceHashKey(V, nameSpace_f, pu32, pKey->data);
            break;
        case SYS_SCL_FIELD_KEY_ECID:
            SetDsUserIdEcidNameSpaceHashKey(V, ecid_f, pu32, pKey->data);
            break;

        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdEcidNameSpaceHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            SetDsUserIdEcidNameSpaceHashKey(V, hashType_f, pu32, USERIDHASHTYPE_ECIDNAMESPACE);
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdEcidNameSpaceHashKey(V, dsAdIndex_f, pu32, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }


    return CTC_E_NONE;

}


STATIC int32
_sys_usw_scl_build_hash_key_ing_ecid(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* pu32 = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    if (pe->resolve_conflict)
    {
	    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    pu32 = (void*)pe->temp_entry->key;

    switch (pKey->type)
    {
        case SYS_SCL_FIELD_KEY_NAMESPACE:
            SetDsUserIdEcidNameSpaceHashKey(V, nameSpace_f, pu32, pKey->data);
            break;
        case SYS_SCL_FIELD_KEY_ECID:
            SetDsUserIdEcidNameSpaceHashKey(V, ecid_f, pu32, pKey->data);
            break;

        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdEcidNameSpaceHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            SetDsUserIdEcidNameSpaceHashKey(V, hashType_f, pu32, USERIDHASHTYPE_INGECIDNAMESPACE);
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdEcidNameSpaceHashKey(V, dsAdIndex_f, pu32, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }


    return CTC_E_NONE;

}

STATIC int32
_sys_usw_scl_build_hash_key_port_cross(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*         p_data = NULL;
    ctc_field_port_t* p_port = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;

    if (CTC_INGRESS == pe->direction || pe->resolve_conflict)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
            /* global src port */
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*) (pKey->ext_data);

                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsEgressXcOamPortCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsEgressXcOamPortCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, isLogicPort_f, p_data, 1);
                    SetDsEgressXcOamPortCrossHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLogicPort_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLabel_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_globalSrcPort_f);
                break;

            case CTC_FIELD_KEY_DST_GPORT:
            /* global dest port */
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*) (pKey->ext_data);

                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsEgressXcOamPortCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsEgressXcOamPortCrossHashKey(V, isLogicPort_f, p_data, 1);
                    SetDsEgressXcOamPortCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsEgressXcOamPortCrossHashKey(V, isLabel_f, p_data, 1);
                    SetDsEgressXcOamPortCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortCrossHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);
                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_globalSrcPort_f);
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLogicPort_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLabel_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_globalDestPort_f);
                break;

            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamPortCrossHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamPortCrossHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_PORTCROSS);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                return CTC_E_NOT_SUPPORT;
                break;
        }

    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_scl_build_hash_key_port_vlan_cross(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void *p_data = NULL;
    ctc_field_port_t *p_port = NULL;
    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;

    if (CTC_INGRESS == pe->direction || pe->resolve_conflict)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    else
    {
        switch (pKey->type)
        {
            case CTC_FIELD_KEY_PORT:
                /* global src port */
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*) (pKey->ext_data);

                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, 0X7FFF);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLogicPort_f, p_data, 1);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLogicPort_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLabel_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_globalSrcPort_f);
                break;

            case CTC_FIELD_KEY_DST_GPORT:
                /* global dest port */
                CTC_PTR_VALID_CHECK(pKey->ext_data);
                p_port = (ctc_field_port_t*) (pKey->ext_data);

                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, globalDestPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLogicPort_f, p_data, 1);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLabel_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, globalDestPort_f, p_data, p_port->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLabel_f, p_data, 1);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsEgressXcOamPortVlanCrossHashKey(V, globalDestPort_f, p_data, p_port->port_class_id);

                    CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_globalSrcPort_f);
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLogicPort_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLabel_f);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_globalDestPort_f);
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                if (CTC_IS_BIT_SET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_CVLAN_ID))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Already configure cvlan before configuring svlan \n");
                    return CTC_E_PARAM_CONFLICT;
                }
                SetDsEgressXcOamPortVlanCrossHashKey(V, vlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_vlanId_f);
                break;
            case CTC_FIELD_KEY_CVLAN_ID:
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                if (CTC_IS_BIT_SET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_SVLAN_ID))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Already configure svlan before configuring cvlan \n");
                    return CTC_E_PARAM_CONFLICT;
                }
                SetDsEgressXcOamPortVlanCrossHashKey(V, vlanId_f, p_data, pKey->data);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_vlanId_f);
                break;

            case CTC_FIELD_KEY_HASH_VALID:
                SetDsEgressXcOamPortVlanCrossHashKey(V, valid_f, p_data, 1);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_valid_f);
                break;
            case SYS_SCL_FIELD_KEY_COMMON:
                SetDsEgressXcOamPortVlanCrossHashKey(V, hashType_f, p_data, EGRESSXCOAMHASHTYPE_PORTVLANCROSS);
                CTC_BMP_UNSET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_hashType_f);
                break;
            default:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                return CTC_E_NOT_SUPPORT;
                break;
        }

    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_scl_build_hash_key_trill_uc(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    /*void* p_mask = NULL;*/

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    /*p_mask = (void*)pe->temp_entry->mask;*/

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillUcDecapHashKey(V, ingressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcDecap_ingressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcDecap_ingressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillUcDecapHashKey(V, egressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcDecap_egressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcDecap_egressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelTrillUcDecapHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillUcDecapHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLUCDECAP);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLUCDECAP);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            */
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelTrillUcDecapHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_trill_uc_rpf(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    /*void* p_mask = NULL;*/

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    /*p_mask = (void*)pe->temp_entry->mask;*/

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillUcRpfHashKey(V, ingressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcRpf_ingressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillUcRpf_ingressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelTrillUcRpfHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillUcRpfHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLUCRPF);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLUCRPF);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            */
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelTrillUcRpfHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_trill_mc(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    /*void* p_mask = NULL;*/

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    /*p_mask = (void*)pe->temp_entry->mask;*/

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcDecapHashKey(V, ingressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcDecap_ingressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcDecap_ingressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcDecapHashKey(V, egressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcDecap_egressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcDecap_egressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelTrillMcDecapHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcDecapHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCDECAP);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCDECAP);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            */
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelTrillMcDecapHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_trill_mc_rpf(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    /*void* p_mask = NULL;*/

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    /*p_mask = (void*)pe->temp_entry->mask;*/

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcRpfHashKey(V, ingressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcRpf_ingressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcRpf_ingressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcRpfHashKey(V, egressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcRpf_egressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcRpf_egressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelTrillMcRpfHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcRpfHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCRPF);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCRPF);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            */
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelTrillMcRpfHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_hash_key_trill_mc_adj(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* p_data = NULL;
    /*void* p_mask = NULL;*/
    ctc_field_port_t* p_port = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_data = (void*)pe->temp_entry->key;
    /*p_mask = (void*)pe->temp_entry->mask;*/

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            p_port = (ctc_field_port_t*)(pKey->ext_data);
            if (!pe->resolve_conflict)
            {
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLabel_f, p_data, 0);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                }
                #if 0
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLogicPort_f, p_data, 1);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLabel_f, p_data, 0);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, globalSrcPort_f, p_data, p_port->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLabel_f, p_data, 1);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, isLogicPort_f, p_data, 0);
                    SetDsUserIdTunnelTrillMcAdjHashKey(V, globalSrcPort_f, p_data, p_port->port_class_id);
                }
                #endif
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
            }
            /*
            else
            {
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    if (!SYS_USW_SCL_GPORT_IS_LOCAL(lchip, p_port->gport))
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
                        return CTC_E_INVALID_CHIP_ID;
                    }
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLogicPort_f, p_mask, 0);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLabel_f, p_mask, 0);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_mask, 0xFFFF);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLogicPort_f, p_data, 1);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLogicPort_f, p_mask, 1);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLabel_f, p_mask, 0);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_data, p_port->logic_port);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_mask, 0xFFFF);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLabel_f, p_data, 1);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLabel_f, p_mask, 1);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_isLogicPort_f, p_mask, 0);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_data, p_port->port_class_id);
                    SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_globalSrcPort_f, p_mask, 0xFFFF);
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
            }
            */
            break;
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcAdjHashKey(V, egressNickname_f, p_data, pKey->data);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_egressNickname_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, u1_gTunnelTrillMcAdj_egressNickname_f, p_mask, 0xFFFF);
            }
            */
            break;
        case CTC_FIELD_KEY_HASH_VALID:
            SetDsUserIdTunnelTrillMcAdjHashKey(V, valid_f, p_data, 1);
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (!pe->resolve_conflict)
            {
                SetDsUserIdTunnelTrillMcAdjHashKey(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCADJ);
            }
            /*
            else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELTRILLMCADJ);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            */
            break;
        case SYS_SCL_FIELD_KEY_AD_INDEX:
            SetDsUserIdTunnelTrillMcAdjHashKey(V, dsAdIndex_f, p_data, pKey->data);
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}


/* Tcam Entry */

STATIC int32
_sys_usw_scl_build_tcam_key_mac(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint8      ceth_type = 0;
    uint8      tag_valid = 0;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_ETHER_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, pKey->data, pe->ether_type, &ceth_type,&pe->ether_type_index));
                SetDsScl0MacKey160(V, cEthType_f, p_data, ceth_type);
                SetDsScl0MacKey160(V, cEthType_f, p_mask, 0x7F);
                pe->ether_type = pKey->data;
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe->ether_type));
                pe->ether_type = 0;
                SetDsScl0MacKey160(V, cEthType_f, p_data, 0);
                SetDsScl0MacKey160(V, cEthType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CUSTOMER_ID:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if (is_add)
            {
                SetDsScl0MacKey160(V, customerIdValid_f, p_data, 1);
                SetDsScl0MacKey160(V, customerIdValid_f, p_mask, 1);
                hw_mac[0] = pKey->data;
                SetDsScl0MacKey160(A, macDa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                hw_mac[0] = pKey->mask;
                SetDsScl0MacKey160(A, macDa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacKey160(V, customerIdValid_f, p_data, 0);
                SetDsScl0MacKey160(V, customerIdValid_f, p_mask, 0);
                SetDsScl0MacKey160(A, macDa_f, p_data, hw_mac);
                SetDsScl0MacKey160(A, macDa_f, p_mask, hw_mac);
            }

            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            if (is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacKey160(V, isStag_f, p_data, 1);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
                SetDsScl0MacKey160(V, vlanId_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, vlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacKey160(V, vlanId_f, p_data, 0);
                SetDsScl0MacKey160(V, vlanId_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 1);
            }
            break;
        case CTC_FIELD_KEY_STAG_COS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacKey160(V, cos_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, cos_f, p_mask, pKey->mask & 0x7);
                SetDsScl0MacKey160(V, isStag_f, p_data, 1);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
            }
            else
            {
                SetDsScl0MacKey160(V, cos_f, p_data, 0);
                SetDsScl0MacKey160(V, cos_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 1);
            }
            break;
        case CTC_FIELD_KEY_STAG_CFI:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacKey160(V, cfi_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, cfi_f, p_mask, pKey->mask & 0x1);
                SetDsScl0MacKey160(V, isStag_f, p_data, 1);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
            }
            else
            {
                SetDsScl0MacKey160(V, cfi_f, p_data, 0);
                SetDsScl0MacKey160(V, cfi_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 1);
            }
            break;
        case CTC_FIELD_KEY_CVLAN_ID:
            if (is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacKey160(V, vlanId_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, vlanId_f, p_mask, pKey->mask & 0xFFF);
                SetDsScl0MacKey160(V, isStag_f, p_data, 0);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
            }
            else
            {
                SetDsScl0MacKey160(V, vlanId_f, p_data, 0);
                SetDsScl0MacKey160(V, vlanId_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_COS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacKey160(V, cos_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, cos_f, p_mask, pKey->mask & 0x7);
                SetDsScl0MacKey160(V, isStag_f, p_data, 0);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
            }
            else
            {
                SetDsScl0MacKey160(V, cos_f, p_data, 0);
                SetDsScl0MacKey160(V, cos_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_CFI:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacKey160(V, cfi_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, cfi_f, p_mask, pKey->mask & 0x1);
                SetDsScl0MacKey160(V, isStag_f, p_data, 0);
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
            }
            else
            {
                SetDsScl0MacKey160(V, cfi_f, p_data, 0);
                SetDsScl0MacKey160(V, cfi_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, 0);
            }
            break;
        /**< [TM] Append Parser Result SvlanId Valid */
        case CTC_FIELD_KEY_PKT_STAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacKey160_t, DsScl0MacKey160_prSvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacKey160(V, prSvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, prSvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacKey160(V, prSvlanIdValid_f, p_data, 0);
                SetDsScl0MacKey160(V, prSvlanIdValid_f, p_mask, 0);
            }
            break;
        /**< [TM] Append Parser Result CvlanId Valid */
        case CTC_FIELD_KEY_PKT_CTAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacKey160_t, DsScl0MacKey160_prCvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacKey160(V, prCvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, prCvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacKey160(V, prCvlanIdValid_f, p_data, 0);
                SetDsScl0MacKey160(V, prCvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_VALID:
        case CTC_FIELD_KEY_CTAG_VALID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacKey160(V, isStag_f, p_data, (CTC_FIELD_KEY_STAG_VALID == pKey->type));
                SetDsScl0MacKey160(V, isStag_f, p_mask, 1);
                SetDsScl0MacKey160(V, vlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacKey160(V, vlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacKey160(V, vlanIdValid_f, p_data, 0);
                SetDsScl0MacKey160(V, vlanIdValid_f, p_mask, 0);
                SYS_USW_SCL_TCAM_KEY_MAC_CLEAR_TAG_FLAG(tag_valid, p_data, p_mask, (CTC_FIELD_KEY_STAG_VALID == pKey->type));
            }
            break;
        case CTC_FIELD_KEY_VLAN_NUM:
        case CTC_FIELD_KEY_L2_TYPE:
            SetDsScl0MacKey160(V, layer2Type_f, p_data, is_add? pKey->data: 0);
            SetDsScl0MacKey160(V, layer2Type_f, p_mask, is_add? pKey->mask & 0x3: 0);
            break;
        case CTC_FIELD_KEY_MAC_SA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if(is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacKey160(A, macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacKey160(A, macSa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacKey160(A, macSa_f, p_data, hw_mac);
                SetDsScl0MacKey160(A, macSa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_MAC_DA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if(is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacKey160(A, macDa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacKey160(A, macDa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacKey160(A, macDa_f, p_data, hw_mac);
                SetDsScl0MacKey160(A, macDa_f, p_mask, hw_mac);
            }
            break;

        case CTC_FIELD_KEY_PORT:
            if(is_add)
            {
                SetDsScl0MacKey160(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacKey160(V, globalPort_f, p_mask, 0);
                SetDsScl0MacKey160(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacKey160(V, sclLabel1To0_f, p_mask, 0);
                p_port = (ctc_field_port_t*) (pKey->ext_data);
                p_port_mask = (ctc_field_port_t*) (pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SetDsScl0MacKey160(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0MacKey160(V, globalPort_f, p_mask, p_port_mask->gport);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    SetDsScl0MacKey160(V, isLogicPort_f, p_data, 1);
                    SetDsScl0MacKey160(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0MacKey160(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0MacKey160(V, globalPort_f, p_mask, p_port_mask->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SetDsScl0MacKey160(V, sclLabel7To2_f, p_data, (p_port->port_class_id) >> 2 & 0x3F);
                    SetDsScl0MacKey160(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x03);
                    SetDsScl0MacKey160(V, sclLabel7To2_f, p_mask, (p_port_mask->port_class_id) >> 2 & 0x3F);
                    SetDsScl0MacKey160(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x03);
                }
            }
            else
            {
                SetDsScl0MacKey160(V, isLogicPort_f, p_data, 0);
                SetDsScl0MacKey160(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacKey160(V, globalPort_f, p_data, 0);
                SetDsScl0MacKey160(V, globalPort_f, p_mask, 0);
                SetDsScl0MacKey160(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0MacKey160(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0MacKey160(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacKey160(V, sclLabel1To0_f, p_mask, 0);
            }
            break;

        case SYS_SCL_FIELD_KEY_COMMON:
            if(is_add)
            {
                SetDsScl0MacKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACKEY160));
                SetDsScl0MacKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACKEY160));
                SetDsScl0MacKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacKey160(V, _type_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacKey160(V, _type_f, p_mask, 0x3);
            }
            /*
            else
            {
                SetDsScl0MacKey160(V, sclKeyType0_f, p_data, 0);
                SetDsScl0MacKey160(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0MacKey160(V, sclKeyType1_f, p_data, 0);
                SetDsScl0MacKey160(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0MacKey160(V, _type_f, p_data, 0);
                SetDsScl0MacKey160(V, _type_f, p_mask, 0);
            }
            */
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_ipv4_double(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t     hw_mac = { 0 };
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_RARP:
            SetDsScl0MacL3Key320(V, isRarp_f, p_data, (is_add? pKey->data: 0));
            SetDsScl0MacL3Key320(V, isRarp_f, p_mask, (is_add? pKey->mask & 0x1: 0));
            break;
        case CTC_FIELD_KEY_PORT:
            if (is_add)
            {
                SetDsScl0MacL3Key320(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, globalPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclLabel1To0_f, p_mask, 0);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                p_port_mask = (ctc_field_port_t*)(pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SetDsScl0MacL3Key320(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0MacL3Key320(V, globalPort_f, p_mask, p_port_mask->gport);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    SetDsScl0MacL3Key320(V, isLogicPort_f, p_data, 1);
                    SetDsScl0MacL3Key320(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0MacL3Key320(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0MacL3Key320(V, globalPort_f, p_mask, p_port_mask->logic_port);

                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SetDsScl0MacL3Key320(V, sclLabel7To2_f, p_data, ((p_port->port_class_id) >> 2) & 0x3F);
                    SetDsScl0MacL3Key320(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x3);
                    SetDsScl0MacL3Key320(V, sclLabel7To2_f, p_mask, ((p_port_mask->port_class_id) >> 2) & 0x3F);
                    SetDsScl0MacL3Key320(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x3);
                }
            }
            else
            {
                SetDsScl0MacL3Key320(V, isLogicPort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, globalPort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, globalPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclLabel1To0_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_MAC_DA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if(is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacL3Key320(A, macDa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacL3Key320(A, macDa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacL3Key320(A, macDa_f, p_data, hw_mac);
                SetDsScl0MacL3Key320(A, macDa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_MAC_SA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if(is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacL3Key320(A, macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacL3Key320(A, macSa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacL3Key320(A, macSa_f, p_data, hw_mac);
                SetDsScl0MacL3Key320(A, macSa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_CVLAN_ID:
            if(is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacL3Key320(V, cvlanId_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, cvlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, cvlanId_f, p_data, 0);
                SetDsScl0MacL3Key320(V, cvlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_COS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacL3Key320(V, ctagCos_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ctagCos_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ctagCos_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ctagCos_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_CFI:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, ctagCfi_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ctagCfi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ctagCfi_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ctagCfi_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            if(is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacL3Key320(V, svlanId_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, svlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, svlanId_f, p_data, 0);
                SetDsScl0MacL3Key320(V, svlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_COS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacL3Key320(V, stagCos_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, stagCos_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacL3Key320(V, stagCos_f, p_data, 0);
                SetDsScl0MacL3Key320(V, stagCos_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_CFI:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, stagCfi_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, stagCfi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, stagCfi_f, p_data, 0);
                SetDsScl0MacL3Key320(V, stagCfi_f, p_mask, 0);
            }
            break;
        /**< [TM] Append Parser Result stag valid */
        case CTC_FIELD_KEY_PKT_STAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacL3Key320_t, DsScl0MacL3Key320_prSvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, prSvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, prSvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, prSvlanIdValid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, prSvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_VALID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, svlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, svlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, svlanIdValid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, svlanIdValid_f, p_mask, 0);
            }
            break;
        /**< [TM] Append Packet ctag valid */
        case CTC_FIELD_KEY_PKT_CTAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacL3Key320_t, DsScl0MacL3Key320_prCvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, prCvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, prCvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, prCvlanIdValid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, prCvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_VALID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, cvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, cvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, cvlanIdValid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, cvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VLAN_NUM:
        case CTC_FIELD_KEY_L2_TYPE:
            SetDsScl0MacL3Key320(V, layer2Type_f, p_data, is_add? pKey->data: 0);
            SetDsScl0MacL3Key320(V, layer2Type_f, p_mask, is_add? pKey->mask & 0x3: 0);
            break;
        case CTC_FIELD_KEY_L3_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0MacL3Key320(V, layer3Type_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, layer3Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, layer3Type_f, p_data, 0);
                SetDsScl0MacL3Key320(V, layer3Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
                SetDsScl0MacL3Key320(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, layer4Type_f, p_data, 0);
                SetDsScl0MacL3Key320(V, layer4Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_USER_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xB);
                SetDsScl0MacL3Key320(V, layer4UserType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, layer4UserType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, layer4UserType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, layer4UserType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ETHER_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                /*CTC_ERROR_RETURN(sys_usw_common_get_compress_ether_type(lchip, pKey->data, &ceth_type));*/
                SetDsScl0MacL3Key320(V, etherType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, etherType_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, etherType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, etherType_f, p_mask, 0);
            }
            break;

        /* ip */
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ipHeaderError_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_TTL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0MacL3Key320(V, ttl_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ttl_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ttl_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ttl_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_DA:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, ipDa_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ipDa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ipDa_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ipDa_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, ipSa_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ipSa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ipSa_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ipSa_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_DSCP:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0MacL3Key320(V, dscp_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, dscp_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0MacL3Key320(V, dscp_f, p_data, 0);
                SetDsScl0MacL3Key320(V, dscp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_ECN:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0MacL3Key320(V, ecn_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ecn_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ecn_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ecn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0MacL3Key320(V, fragInfo_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, fragInfo_f, p_mask, tmp_mask);
                SetDsScl0MacL3Key320(V, isFragmentPacket_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, isFragmentPacket_f, p_mask, tmp_mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, fragInfo_f, p_data, 0);
                SetDsScl0MacL3Key320(V, fragInfo_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, isFragmentPacket_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isFragmentPacket_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, ipOptions_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ipOptions_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ipOptions_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ipOptions_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_FLAGS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0MacL3Key320(V, tcpFlags_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, tcpFlags_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0MacL3Key320(V, tcpFlags_f, p_data, 0);
                SetDsScl0MacL3Key320(V, tcpFlags_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_ECN:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacL3Key320(V, tcpEcn_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, tcpEcn_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacL3Key320(V, tcpEcn_f, p_data, 0);
                SetDsScl0MacL3Key320(V, tcpEcn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_NVGRE_KEY:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, is_add? (pKey->data << 8) & 0xFF00: 0);
            SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, is_add? (pKey->mask << 8) & 0xFF00: 0);
            SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, is_add? (pKey->data >> 8) & 0xFFFF: 0);
            SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, is_add? (pKey->mask >> 8) & 0xFFFF: 0);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFFFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VN_ID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4DestPort_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ICMP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0xFF00;
                tmp_mask &= 0xFF00;
                tmp_data |= (pKey->data & 0xFF);
                tmp_mask |= (pKey->mask & 0xFF);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0xFF00;
                tmp_mask = tmp_mask & 0xFF00;
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_ICMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IGMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;

        /*arp*/
        case CTC_FIELD_KEY_ARP_OP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, arpOpCode_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, arpOpCode_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, arpOpCode_f, p_data, 0);
                SetDsScl0MacL3Key320(V, arpOpCode_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, protocolType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, protocolType_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, protocolType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, protocolType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_TARGET_IP:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, targetIp_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, targetIp_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, targetIp_f, p_data, 0);
                SetDsScl0MacL3Key320(V, targetIp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_SENDER_IP:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, senderIp_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, senderIp_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacL3Key320(V, senderIp_f, p_data, 0);
                SetDsScl0MacL3Key320(V, senderIp_f, p_mask, 0);
            }
            break;

        /*mpls*/
        case CTC_FIELD_KEY_LABEL_NUM:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x9);
                SetDsScl0MacL3Key320(V, labelNum_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, labelNum_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, labelNum_f, p_data, 0);
                SetDsScl0MacL3Key320(V, labelNum_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_MPLS_LABEL0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_LABEL1:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_LABEL2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0MacL3Key320(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IS_Y1731_OAM:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, isY1731Oam_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, isY1731Oam_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, isY1731Oam_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isY1731Oam_f, p_mask, 0);
            }
            break;
        /*fcoe*/
        case CTC_FIELD_KEY_FCOE_DST_FCID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0MacL3Key320(V, fcoeDid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, fcoeDid_f, p_mask, pKey->mask & 0xFFFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, fcoeDid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, fcoeDid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_FCOE_SRC_FCID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0MacL3Key320(V, fcoeSid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, fcoeSid_f, p_mask, pKey->mask & 0xFFFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, fcoeSid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, fcoeSid_f, p_mask, 0);
            }
            break;

        /*trill*/
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, egressNickname_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, egressNickname_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, egressNickname_f, p_data, 0);
                SetDsScl0MacL3Key320(V, egressNickname_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, ingressNickname_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ingressNickname_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ingressNickname_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ingressNickname_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_MULTIHOP:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, trillMultiHop_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillMultiHop_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillMultiHop_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillMultiHop_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_LENGTH:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3FFF);
                SetDsScl0MacL3Key320(V, trillLength_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillLength_f, p_mask, pKey->mask & 0x3FFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillLength_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillLength_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0MacL3Key320(V, trillVersion_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillVersion_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillVersion_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillVersion_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, isTrillChannel_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, isTrillChannel_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, isTrillChannel_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isTrillChannel_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IS_ESADI:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, isEsadi_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, isEsadi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, isEsadi_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isEsadi_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID:
            if(is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacL3Key320(V, trillInnerVlanId_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillInnerVlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillInnerVlanId_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillInnerVlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, trillInnerVlanValid_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillInnerVlanValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillInnerVlanValid_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillInnerVlanValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_MULTICAST:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacL3Key320(V, trillMulticast_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillMulticast_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillMulticast_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillMulticast_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_TTL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0MacL3Key320(V, u1_g5_ttl_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, u1_g5_ttl_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {

                SetDsScl0MacL3Key320(V, u1_g5_ttl_f, p_data, 0);
                SetDsScl0MacL3Key320(V, u1_g5_ttl_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_KEY_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacL3Key320(V, trillKeyType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, trillKeyType_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacL3Key320(V, trillKeyType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, trillKeyType_f, p_mask, 0);
            }
            break;

        /*ether oam*/
        case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_mask, pKey->mask & 0x7);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_mask, pKey->mask & 0x7);
                }
            }
            else
            {
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_mask, 0);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamLevel_f, p_mask, 0);
                }
            }
            break;
        case CTC_FIELD_KEY_ETHER_OAM_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1F);
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_mask, pKey->mask & 0x1F);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_mask, pKey->mask & 0x1F);
                }
            }
            else
            {
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_mask, 0);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamVersion_f, p_mask, 0);
                }
            }
            break;
        case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_mask, pKey->mask & 0xFF);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_data, pKey->data);
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_mask, pKey->mask & 0xFF);
                }
            }
            else
            {
                if (pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST)
                {
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_mask, 0);
                }
                else if (pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_data, 0);
                    SetDsScl0MacL3Key320(V, etherOamOpCode_f, p_mask, 0);
                }
            }
            break;

        /*slow protocol*/
        case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0MacL3Key320(V, slowProtocolSubType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, slowProtocolSubType_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, slowProtocolSubType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, slowProtocolSubType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacL3Key320(V, slowProtocolFlags_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, slowProtocolFlags_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, slowProtocolFlags_f, p_data, 0);
                SetDsScl0MacL3Key320(V, slowProtocolFlags_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0MacL3Key320(V, slowProtocolCode_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, slowProtocolCode_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, slowProtocolCode_f, p_data, 0);
                SetDsScl0MacL3Key320(V, slowProtocolCode_f, p_mask, 0);
            }
            break;

        /*ptp*/
        case CTC_FIELD_KEY_PTP_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0MacL3Key320(V, ptpVersion_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ptpVersion_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ptpVersion_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ptpVersion_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0MacL3Key320(V, ptpMessageType_f, p_data, pKey->data);
                SetDsScl0MacL3Key320(V, ptpMessageType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacL3Key320(V, ptpMessageType_f, p_data, 0);
                SetDsScl0MacL3Key320(V, ptpMessageType_f, p_mask, 0);
            }
            break;

        case SYS_SCL_FIELD_KEY_COMMON:
            if(is_add)
            {
                SetDsScl0MacL3Key320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320));
                SetDsScl0MacL3Key320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacL3Key320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320));
                SetDsScl0MacL3Key320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacL3Key320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320));
                SetDsScl0MacL3Key320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacL3Key320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320));
                SetDsScl0MacL3Key320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));

                SetDsScl0MacL3Key320(V, isIpv6Key_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isIpv6Key_f, p_mask, 1);

                SetDsScl0MacL3Key320(V, _type0_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacL3Key320(V, _type0_f, p_mask, 0x3);
                SetDsScl0MacL3Key320(V, _type1_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacL3Key320(V, _type1_f, p_mask, 0x3);
            }
            /*
            else
            {
                SetDsScl0MacL3Key320(V, sclKeyType0_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclKeyType1_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclKeyType2_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclKeyType2_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, sclKeyType3_f, p_data, 0);
                SetDsScl0MacL3Key320(V, sclKeyType3_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, isIpv6Key_f, p_data, 0);
                SetDsScl0MacL3Key320(V, isIpv6Key_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, _type0_f, p_data, 0);
                SetDsScl0MacL3Key320(V, _type0_f, p_mask, 0);
                SetDsScl0MacL3Key320(V, _type1_f, p_data, 0);
                SetDsScl0MacL3Key320(V, _type1_f, p_mask, 0);
            }
            */
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_ipv4_single(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    sys_scl_sw_temp_entry_t* p_temp_entry_t = NULL;
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_temp_entry_t = pe->temp_entry;
    p_data = (void*)p_temp_entry_t->key;
    p_mask = (void*)p_temp_entry_t->mask;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            if(is_add)
            {
                SetDsScl0L3Key160(V, isLogicPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, globalPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0L3Key160(V, sclLabel1To0_f, p_mask, 0);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                p_port_mask = (ctc_field_port_t*)(pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SetDsScl0L3Key160(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0L3Key160(V, globalPort_f, p_mask, p_port_mask->gport);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    SetDsScl0L3Key160(V, isLogicPort_f, p_data, 1);
                    SetDsScl0L3Key160(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0L3Key160(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0L3Key160(V, globalPort_f, p_mask, p_port_mask->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SetDsScl0L3Key160(V, sclLabel7To2_f, p_data, ((p_port->port_class_id) >> 2) & 0x3F);
                    SetDsScl0L3Key160(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x3);
                    SetDsScl0L3Key160(V, sclLabel7To2_f, p_mask, ((p_port_mask->port_class_id) >> 2) & 0x3F);
                    SetDsScl0L3Key160(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x3);

                }
            }
            else
            {
                SetDsScl0L3Key160(V, isLogicPort_f, p_data, 0);
                SetDsScl0L3Key160(V, isLogicPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, globalPort_f, p_data, 0);
                SetDsScl0L3Key160(V, globalPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0L3Key160(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0L3Key160(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0L3Key160(V, sclLabel1To0_f, p_mask, 0);
            }
            break;

        case CTC_FIELD_KEY_L3_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0L3Key160(V, layer3Type_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, layer3Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0L3Key160(V, layer3Type_f, p_data, 0);
                SetDsScl0L3Key160(V, layer3Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
                SetDsScl0L3Key160(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0L3Key160(V, layer4Type_f, p_data, 0);
                SetDsScl0L3Key160(V, layer4Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_USER_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xB);
                SetDsScl0L3Key160(V, layer4UserType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, layer4UserType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0L3Key160(V, layer4UserType_f, p_data, 0);
                SetDsScl0L3Key160(V, layer4UserType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ETHER_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                /*CTC_ERROR_RETURN(sys_usw_common_get_compress_ether_type(lchip, pKey->data, &ceth_type));*/
                SetDsScl0L3Key160(V, etherType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, etherType_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0L3Key160(V, etherType_f, p_data, 0);
                SetDsScl0L3Key160(V, etherType_f, p_mask, 0);
            }
            break;

        /* ip */
        case CTC_FIELD_KEY_IP_DA:
            if(is_add)
            {
                SetDsScl0L3Key160(V, ipDa_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ipDa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0L3Key160(V, ipDa_f, p_data, 0);
                SetDsScl0L3Key160(V, ipDa_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            if(is_add)
            {
                SetDsScl0L3Key160(V, ipSa_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ipSa_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0L3Key160(V, ipSa_f, p_data, 0);
                SetDsScl0L3Key160(V, ipSa_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_DSCP:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0L3Key160(V, dscp_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, dscp_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0L3Key160(V, dscp_f, p_data, 0);
                SetDsScl0L3Key160(V, dscp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_ECN:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0L3Key160(V, ecn_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ecn_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0L3Key160(V, ecn_f, p_data, 0);
                SetDsScl0L3Key160(V, ecn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, ipHeaderError_f, p_data, 0);
                SetDsScl0L3Key160(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, ipOptions_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ipOptions_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, ipOptions_f, p_data, 0);
                SetDsScl0L3Key160(V, ipOptions_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0L3Key160(V, fragInfo_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, fragInfo_f, p_mask, tmp_mask);
            }
            else
            {
                SetDsScl0L3Key160(V, fragInfo_f, p_data, 0);
                SetDsScl0L3Key160(V, fragInfo_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_FLAGS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                tmp_data = GetDsScl0L3Key160(V, shareFields_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, shareFields_f, p_mask);
                tmp_data = (tmp_data & 0xC0) | pKey->data;
                tmp_mask = (tmp_mask & 0xC0) | (pKey->mask & 0x3F);
                SetDsScl0L3Key160(V, shareFields_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, shareFields_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, shareFields_f, p_mask);
                tmp_data = tmp_data & 0xC0;
                tmp_mask = tmp_mask & 0xC0;
                SetDsScl0L3Key160(V, shareFields_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_TCP_ECN:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                tmp_data = GetDsScl0L3Key160(V, shareFields_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, shareFields_f, p_mask);
                tmp_data = (tmp_data&0x3F) | (pKey->data << 6);
                tmp_mask = (tmp_mask&0x3F) | ((pKey->mask & 0x7) << 6);
                SetDsScl0L3Key160(V, shareFields_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, shareFields_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, shareFields_f, p_mask);
                tmp_data = tmp_data & 0x3F;
                tmp_mask = tmp_mask & 0x3F;
                SetDsScl0L3Key160(V, shareFields_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IP_TTL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0L3Key160(V, shareFields_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0L3Key160(V, shareFields_f, p_data, 0);
                SetDsScl0L3Key160(V, shareFields_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_NVGRE_KEY:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            SetDsScl0L3Key160(V, l4DestPort_f, p_data, is_add? (pKey->data << 8) & 0xFF00: 0);
            SetDsScl0L3Key160(V, l4DestPort_f, p_mask, is_add? (pKey->mask << 8) & 0xFF00: 0);
            SetDsScl0L3Key160(V, l4SourcePort_f, p_data, is_add? (pKey->data >> 8) & 0xFFFF: 0);
            SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, is_add? (pKey->mask >> 8) & 0xFFFF: 0);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            if(is_add)
            {
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFFFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VN_ID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFF);
            }
            else
            {
                SetDsScl0L3Key160(V, l4DestPort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4DestPort_f, p_mask, 0);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, 0);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ICMP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data &= 0xFF00;
                tmp_mask &= 0xFF00;
                tmp_data |= (pKey->data & 0xFF);
                tmp_mask |= (pKey->mask & 0xFF);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0xFF00;
                tmp_mask = tmp_mask & 0xFF00;
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_ICMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IGMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0L3Key160(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;

        /*arp*/
        case CTC_FIELD_KEY_ARP_OP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, arpOpCode_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, arpOpCode_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, arpOpCode_f, p_data, 0);
                SetDsScl0L3Key160(V, arpOpCode_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, protocolType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, protocolType_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, protocolType_f, p_data, 0);
                SetDsScl0L3Key160(V, protocolType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_TARGET_IP:
            if(is_add)
            {
                SetDsScl0L3Key160(V, targetIp_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, targetIp_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0L3Key160(V, targetIp_f, p_data, 0);
                SetDsScl0L3Key160(V, targetIp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ARP_SENDER_IP:
            if(is_add)
            {
                SetDsScl0L3Key160(V, senderIp_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, senderIp_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0L3Key160(V, senderIp_f, p_data, 0);
                SetDsScl0L3Key160(V, senderIp_f, p_mask, 0);
            }
            break;

        case CTC_FIELD_KEY_RARP:
            SetDsScl0L3Key160(V, isRarp_f, p_data, (is_add? pKey->data: 0));
            SetDsScl0L3Key160(V, isRarp_f, p_mask, (is_add? pKey->mask & 0x1: 0));
            break;

        /*mpls*/
        case CTC_FIELD_KEY_LABEL_NUM:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x9);
                SetDsScl0L3Key160(V, labelNum_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, labelNum_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0L3Key160(V, labelNum_f, p_data, 0);
                SetDsScl0L3Key160(V, labelNum_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_MPLS_LABEL0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL0:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel0_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel0_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0L3Key160(V, mplsLabel0_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel0_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_LABEL1:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL1:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel1_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel1_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0L3Key160(V, mplsLabel1_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel1_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_LABEL2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                tmp_data = tmp_data | (pKey->data << 12);
                tmp_mask = tmp_mask | (pKey->mask << 12);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0x00000FFF;
                tmp_mask = tmp_mask & 0x00000FFF;
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_EXP2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                tmp_data = tmp_data | (pKey->data << 9);
                tmp_mask = tmp_mask | (pKey->mask << 9);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFF1FF;
                tmp_mask = tmp_mask & 0xFFFFF1FF;
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_SBIT2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                tmp_data = tmp_data | (pKey->data << 8);
                tmp_mask = tmp_mask | (pKey->mask << 8);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFEFF;
                tmp_mask = tmp_mask & 0xFFFFFEFF;
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        case CTC_FIELD_KEY_MPLS_TTL2:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                tmp_data = tmp_data | (pKey->data);
                tmp_mask = tmp_mask | (pKey->mask);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0L3Key160(V, mplsLabel2_f, p_data);
                tmp_mask = GetDsScl0L3Key160(V, mplsLabel2_f, p_mask);
                tmp_data = tmp_data & 0xFFFFFF00;
                tmp_mask = tmp_mask & 0xFFFFFF00;
                SetDsScl0L3Key160(V, mplsLabel2_f, p_data, tmp_data);
                SetDsScl0L3Key160(V, mplsLabel2_f, p_mask, tmp_mask);
            }
            break;

        /*fcoe*/
        case CTC_FIELD_KEY_FCOE_DST_FCID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0L3Key160(V, fcoeDid_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, fcoeDid_f, p_mask, pKey->mask & 0xFFFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, fcoeDid_f, p_data, 0);
                SetDsScl0L3Key160(V, fcoeDid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_FCOE_SRC_FCID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0L3Key160(V, fcoeSid_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, fcoeSid_f, p_mask, pKey->mask & 0xFFFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, fcoeSid_f, p_data, 0);
                SetDsScl0L3Key160(V, fcoeSid_f, p_mask, 0);
            }
            break;

        /*trill*/
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, egressNickname_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, egressNickname_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, egressNickname_f, p_data, 0);
                SetDsScl0L3Key160(V, egressNickname_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_INGRESS_NICKNAME:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, ingressNickname_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ingressNickname_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, ingressNickname_f, p_data, 0);
                SetDsScl0L3Key160(V, ingressNickname_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_MULTIHOP:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, trillMultiHop_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillMultiHop_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, trillMultiHop_f, p_data, 0);
                SetDsScl0L3Key160(V, trillMultiHop_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_LENGTH:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3FFF);
                SetDsScl0L3Key160(V, trillLength_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillLength_f, p_mask, pKey->mask & 0x3FFF);
            }
            else
            {
                SetDsScl0L3Key160(V, trillLength_f, p_data, 0);
                SetDsScl0L3Key160(V, trillLength_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0L3Key160(V, trillVersion_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillVersion_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0L3Key160(V, trillVersion_f, p_data, 0);
                SetDsScl0L3Key160(V, trillVersion_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, isTrillChannel_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, isTrillChannel_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, isTrillChannel_f, p_data, 0);
                SetDsScl0L3Key160(V, isTrillChannel_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IS_ESADI:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, isEsadi_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, isEsadi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, isEsadi_f, p_data, 0);
                SetDsScl0L3Key160(V, isEsadi_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID:
            if(is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0L3Key160(V, trillInnerVlanId_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillInnerVlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, trillInnerVlanId_f, p_data, 0);
                SetDsScl0L3Key160(V, trillInnerVlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, trillInnerVlanValid_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillInnerVlanValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, trillInnerVlanValid_f, p_data, 0);
                SetDsScl0L3Key160(V, trillInnerVlanValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_MULTICAST:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0L3Key160(V, trillMulticast_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillMulticast_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0L3Key160(V, trillMulticast_f, p_data, 0);
                SetDsScl0L3Key160(V, trillMulticast_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_TTL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0L3Key160(V, ttl_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ttl_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0L3Key160(V, ttl_f, p_data, 0);
                SetDsScl0L3Key160(V, ttl_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TRILL_KEY_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0L3Key160(V, trillKeyType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, trillKeyType_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0L3Key160(V, trillKeyType_f, p_data, 0);
                SetDsScl0L3Key160(V, trillKeyType_f, p_mask, 0);
            }
            break;

        /*ether oam*/
        case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0L3Key160(V, etherOamLevel_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, etherOamLevel_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0L3Key160(V, etherOamLevel_f, p_data, 0);
                SetDsScl0L3Key160(V, etherOamLevel_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ETHER_OAM_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1F);
                SetDsScl0L3Key160(V, etherOamVersion_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, etherOamVersion_f, p_mask, pKey->mask & 0x1F);
            }
            else
            {
                SetDsScl0L3Key160(V, etherOamVersion_f, p_data, 0);
                SetDsScl0L3Key160(V, etherOamVersion_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0L3Key160(V, etherOamOpCode_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, etherOamOpCode_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0L3Key160(V, etherOamOpCode_f, p_data, 0);
                SetDsScl0L3Key160(V, etherOamOpCode_f, p_mask, 0);
            }
            break;

        /*slow protocol*/
        case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0L3Key160(V, slowProtocolSubType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, slowProtocolSubType_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0L3Key160(V, slowProtocolSubType_f, p_data, 0);
                SetDsScl0L3Key160(V, slowProtocolSubType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0L3Key160(V, slowProtocolFlags_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, slowProtocolFlags_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0L3Key160(V, slowProtocolFlags_f, p_data, 0);
                SetDsScl0L3Key160(V, slowProtocolFlags_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0L3Key160(V, slowProtocolCode_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, slowProtocolCode_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0L3Key160(V, slowProtocolCode_f, p_data, 0);
                SetDsScl0L3Key160(V, slowProtocolCode_f, p_mask, 0);
            }
            break;

        /*ptp*/
        case CTC_FIELD_KEY_PTP_VERSION:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0L3Key160(V, ptpVersion_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ptpVersion_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0L3Key160(V, ptpVersion_f, p_data, 0);
                SetDsScl0L3Key160(V, ptpVersion_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0L3Key160(V, ptpMessageType_f, p_data, pKey->data);
                SetDsScl0L3Key160(V, ptpMessageType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0L3Key160(V, ptpMessageType_f, p_data, 0);
                SetDsScl0L3Key160(V, ptpMessageType_f, p_mask, 0);
            }
            break;

        case SYS_SCL_FIELD_KEY_COMMON:
            if(is_add)
            {
                SetDsScl0L3Key160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_L3KEY160));
                SetDsScl0L3Key160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));

                SetDsScl0L3Key160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_L3KEY160));
                SetDsScl0L3Key160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));

                SetDsScl0L3Key160(V, isIpv6Key_f, p_data, 0);
                SetDsScl0L3Key160(V, isIpv6Key_f, p_mask, 1);

                SetDsScl0L3Key160(V, _type_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0L3Key160(V, _type_f, p_mask, 0x3);
            }
            /*
            else
            {
                SetDsScl0L3Key160(V, sclKeyType0_f, p_data, 0);
                SetDsScl0L3Key160(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0L3Key160(V, sclKeyType1_f, p_data, 0);
                SetDsScl0L3Key160(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0L3Key160(V, isIpv6Key_f, p_data, 0);
                SetDsScl0L3Key160(V, isIpv6Key_f, p_mask, 0);
                SetDsScl0L3Key160(V, _type_f, p_data, 0);
                SetDsScl0L3Key160(V, _type_f, p_mask, 0);
            }*/
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_ipv6_double(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    hw_mac_addr_t               hw_mac = { 0 };
    ipv6_addr_t               hw_ip6;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    uint8       key_type_mask = 0;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_L3_TYPE:
            CTC_ERROR_RETURN(DRV_IS_DUET2(lchip)? CTC_E_NOT_SUPPORT: CTC_E_NONE);
            key_type_mask = CTC_PARSER_L3_TYPE_NONE == pKey->data && is_add? 0x3: DRV_ENUM(DRV_SCL_KEY_TYPE_MASK);
            SetDsScl0MacIpv6Key640(V, sclKeyType0_f, p_mask, key_type_mask);
            SetDsScl0MacIpv6Key640(V, sclKeyType1_f, p_mask, key_type_mask);
            SetDsScl0MacIpv6Key640(V, sclKeyType2_f, p_mask, key_type_mask);
            SetDsScl0MacIpv6Key640(V, sclKeyType3_f, p_mask, key_type_mask);
            break;
        case CTC_FIELD_KEY_PORT:
            if(is_add)
            {
                SetDsScl0MacIpv6Key640(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, globalPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel1To0_f, p_mask, 0);
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                p_port_mask = (ctc_field_port_t*)(pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SetDsScl0MacIpv6Key640(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0MacIpv6Key640(V, globalPort_f, p_mask, p_port_mask->gport);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    SetDsScl0MacIpv6Key640(V, isLogicPort_f, p_data, 1);
                    SetDsScl0MacIpv6Key640(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0MacIpv6Key640(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0MacIpv6Key640(V, globalPort_f, p_mask, p_port_mask->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SetDsScl0MacIpv6Key640(V, sclLabel7To2_f, p_data, ((p_port->port_class_id) >> 2) & 0x3F);
                    SetDsScl0MacIpv6Key640(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x3);
                    SetDsScl0MacIpv6Key640(V, sclLabel7To2_f, p_mask, ((p_port_mask->port_class_id) >> 2) & 0x3F);
                    SetDsScl0MacIpv6Key640(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x3);
                }
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, isLogicPort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isLogicPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, globalPort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, globalPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclLabel1To0_f, p_mask, 0);
            }
            break;

        case CTC_FIELD_KEY_MAC_DA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if (is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacIpv6Key640(A, macDa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacIpv6Key640(A, macDa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacIpv6Key640(A, macDa_f, p_data, hw_mac);
                SetDsScl0MacIpv6Key640(A, macDa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_MAC_SA:
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if (is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                }
                SetDsScl0MacIpv6Key640(A, macSa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                }
                SetDsScl0MacIpv6Key640(A, macSa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0MacIpv6Key640(A, macSa_f, p_data, hw_mac);
                SetDsScl0MacIpv6Key640(A, macSa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_CVLAN_ID:
            if (is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacIpv6Key640(V, cvlanId_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, cvlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, cvlanId_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, cvlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_COS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacIpv6Key640(V, ctagCos_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ctagCos_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ctagCos_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ctagCos_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_CFI:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, ctagCfi_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ctagCfi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ctagCfi_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ctagCfi_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            if (is_add)
            {
                SYS_SCL_VLAN_ID_CHECK(pKey->data);
                SetDsScl0MacIpv6Key640(V, svlanId_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, svlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, svlanId_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, svlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_COS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacIpv6Key640(V, stagCos_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, stagCos_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, stagCos_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, stagCos_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_CFI:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 1);
                SetDsScl0MacIpv6Key640(V, stagCfi_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, stagCfi_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, stagCfi_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, stagCfi_f, p_mask, 0);
            }
            break;
        /**< [TM] Append Packet stag valid */
        case CTC_FIELD_KEY_PKT_STAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacIpv6Key640_t, DsScl0MacIpv6Key640_prSvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, prSvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, prSvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, prSvlanIdValid_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, prSvlanIdValid_f, p_mask, 0);
            }
            break;
        /**< [TM] Append Packet ctag valid */
        case CTC_FIELD_KEY_PKT_CTAG_VALID:
            if (!DRV_FLD_IS_EXISIT(DsScl0MacIpv6Key640_t, DsScl0MacIpv6Key640_prCvlanIdValid_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, prCvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, prCvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, prCvlanIdValid_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, prCvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_STAG_VALID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, svlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, svlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, svlanIdValid_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, svlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CTAG_VALID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, cvlanIdValid_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, cvlanIdValid_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, cvlanIdValid_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, cvlanIdValid_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VLAN_NUM:
        case CTC_FIELD_KEY_L2_TYPE:
            SetDsScl0MacIpv6Key640(V, layer2Type_f, p_data, is_add? pKey->data: 0);
            SetDsScl0MacIpv6Key640(V, layer2Type_f, p_mask, is_add? pKey->mask & 0x3: 0);
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
                SetDsScl0MacIpv6Key640(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, layer4Type_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, layer4Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_USER_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xB);
                SetDsScl0MacIpv6Key640(V, layer4UserType_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, layer4UserType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, layer4UserType_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, layer4UserType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ETHER_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                /*CTC_ERROR_RETURN(sys_usw_common_get_compress_ether_type(lchip, pKey->data, &ceth_type));*/
                SetDsScl0MacIpv6Key640(V, etherType_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, etherType_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, etherType_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, etherType_f, p_mask, 0);
            }
            break;

        /* ip */
        case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                SetDsScl0MacIpv6Key640(V, ipv6FlowLabel_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ipv6FlowLabel_f, p_mask, pKey->mask & 0xFFFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ipv6FlowLabel_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ipv6FlowLabel_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ipHeaderError_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_TTL:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0MacIpv6Key640(V, ttl_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ttl_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ttl_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ttl_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if(is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsScl0MacIpv6Key640(A, ipDa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                }
                SetDsScl0MacIpv6Key640(A, ipDa_f, p_mask, hw_ip6);
            }
            else
            {
                SetDsScl0MacIpv6Key640(A, ipDa_f, p_data, hw_ip6);
                SetDsScl0MacIpv6Key640(A, ipDa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsScl0MacIpv6Key640(A, ipSa_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                }
                SetDsScl0MacIpv6Key640(A, ipSa_f, p_mask, hw_ip6);
            }
            else
            {

                SetDsScl0MacIpv6Key640(A, ipSa_f, p_data, hw_ip6);
                SetDsScl0MacIpv6Key640(A, ipSa_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IP_DSCP:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0MacIpv6Key640(V, dscp_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, dscp_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, dscp_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, dscp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_ECN:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0MacIpv6Key640(V, ecn_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ecn_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ecn_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ecn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0MacIpv6Key640(V, fragInfo_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, fragInfo_f, p_mask, tmp_mask);
                SetDsScl0MacIpv6Key640(V, isFragmentPacket_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, isFragmentPacket_f, p_mask, tmp_mask);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, fragInfo_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, fragInfo_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, isFragmentPacket_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isFragmentPacket_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0MacIpv6Key640(V, ipOptions_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, ipOptions_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, ipOptions_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, ipOptions_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_FLAGS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0MacIpv6Key640(V, tcpFlags_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, tcpFlags_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, tcpFlags_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, tcpFlags_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_ECN:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0MacIpv6Key640(V, tcpEcn_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, tcpEcn_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, tcpEcn_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, tcpEcn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, pKey->data);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_NVGRE_KEY:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, is_add? (pKey->data << 8) & 0xFF00: 0);
            SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, is_add? (pKey->mask << 8) & 0xFF00: 0);
            SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, is_add? (pKey->data >> 8) & 0xFFFF: 0);
            SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, is_add? (pKey->mask >> 8) & 0xFFFF: 0);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            if (is_add)
            {
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFFFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VN_ID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFF);
            }
            else
            {
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4DestPort_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ICMP_CODE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data &= 0xFF00;
                tmp_mask &= 0xFF00;
                tmp_data |= (pKey->data & 0xFF);
                tmp_mask |= (pKey->mask & 0xFF);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0xFF00;
                tmp_mask = tmp_mask & 0xFF00;
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_ICMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IGMP_TYPE:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0MacIpv6Key640(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;

        case SYS_SCL_FIELD_KEY_COMMON:
            if(is_add)
            {
                SetDsScl0MacIpv6Key640(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType4_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType4_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType5_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType5_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType6_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType6_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, sclKeyType7_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640));
                SetDsScl0MacIpv6Key640(V, sclKeyType7_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0MacIpv6Key640(V, isIpv6Key0_f, p_data, 1);
                SetDsScl0MacIpv6Key640(V, isIpv6Key0_f, p_mask, 1);
                SetDsScl0MacIpv6Key640(V, isIpv6Key1_f, p_data, 1);
                SetDsScl0MacIpv6Key640(V, isIpv6Key1_f, p_mask, 1);
                SetDsScl0MacIpv6Key640(V, isUpper0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isUpper0_f, p_mask, 1);
                SetDsScl0MacIpv6Key640(V, isUpper1_f, p_data, 1);
                SetDsScl0MacIpv6Key640(V, isUpper1_f, p_mask, 1);
                SetDsScl0MacIpv6Key640(V, _type0_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacIpv6Key640(V, _type0_f, p_mask, 0x3);
                SetDsScl0MacIpv6Key640(V, _type1_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacIpv6Key640(V, _type1_f, p_mask, 0x3);
                SetDsScl0MacIpv6Key640(V, _type2_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacIpv6Key640(V, _type2_f, p_mask, 0x3);
                SetDsScl0MacIpv6Key640(V, _type3_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0MacIpv6Key640(V, _type3_f, p_mask, 0x3);
                SetDsScl0MacIpv6Key640(V, paddingField_f, p_data, CTC_PARSER_L3_TYPE_IPV4);
                SetDsScl0MacIpv6Key640(V, paddingField_f, p_mask, 0xF);
            }
            /*else
            {
                SetDsScl0MacIpv6Key640(V, sclKeyType0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType1_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType2_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType2_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType3_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType3_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType4_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType4_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType5_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType5_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType6_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType6_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType7_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, sclKeyType7_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, isIpv6Key0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isIpv6Key0_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, isIpv6Key1_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isIpv6Key1_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, isUpper0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isUpper0_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, isUpper1_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, isUpper1_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, _type0_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, _type0_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, _type1_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, _type1_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, _type2_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, _type2_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, _type3_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, _type3_f, p_mask, 0);
                SetDsScl0MacIpv6Key640(V, paddingField_f, p_data, 0);
                SetDsScl0MacIpv6Key640(V, paddingField_f, p_mask, 0);
            }*/
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_ipv6_single(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    ipv6_addr_t               hw_ip6;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;

    CTC_PTR_VALID_CHECK(pKey);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_PORT:
            if(is_add)
            {
                p_port = (ctc_field_port_t*)(pKey->ext_data);
                p_port_mask = (ctc_field_port_t*)(pKey->ext_mask);
                SetDsScl0Ipv6Key320(V, isLogicPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, globalPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclLabel1To0_f, p_mask, 0);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SetDsScl0Ipv6Key320(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0Ipv6Key320(V, globalPort_f, p_mask, p_port_mask->gport);
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    SetDsScl0Ipv6Key320(V, isLogicPort_f, p_data, 1);
                    SetDsScl0Ipv6Key320(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0Ipv6Key320(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0Ipv6Key320(V, globalPort_f, p_mask, p_port_mask->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SetDsScl0Ipv6Key320(V, sclLabel7To2_f, p_data, ((p_port->port_class_id) >> 2) & 0x3F);
                    SetDsScl0Ipv6Key320(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x3);
                    SetDsScl0Ipv6Key320(V, sclLabel7To2_f, p_mask, ((p_port_mask->port_class_id) >> 2) & 0x3F);
                    SetDsScl0Ipv6Key320(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x3);
                }
            }
            else
            {
                SetDsScl0Ipv6Key320(V, isLogicPort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, isLogicPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, globalPort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, globalPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclLabel1To0_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VLAN_NUM:
        case CTC_FIELD_KEY_L2_TYPE:
            SetDsScl0Ipv6Key320(V, layer2Type_f, p_data, is_add? pKey->data: 0);
            SetDsScl0Ipv6Key320(V, layer2Type_f, p_mask, is_add? pKey->mask & 0x3: 0);
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xE);
                SetDsScl0Ipv6Key320(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, layer4Type_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, layer4Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_USER_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xB);
                SetDsScl0Ipv6Key320(V, layer4UserType_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, layer4UserType_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, layer4UserType_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, layer4UserType_f, p_mask, 0);
            }
            break;

        /* ip */
        case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFF);
                SetDsScl0Ipv6Key320(V, ipv6FlowLabel_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, ipv6FlowLabel_f, p_mask, pKey->mask & 0xFFFFF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, ipv6FlowLabel_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, ipv6FlowLabel_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0Ipv6Key320(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, ipHeaderError_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_TTL:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0Ipv6Key320(V, ttl_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, ttl_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, ttl_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, ttl_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsScl0Ipv6Key320(A, ipAddr1_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                }
                SetDsScl0Ipv6Key320(A, ipAddr1_f, p_mask, hw_ip6);
            }
            else
            {
                SetDsScl0Ipv6Key320(A, ipAddr1_f, p_data, hw_ip6);
                SetDsScl0Ipv6Key320(A, ipAddr1_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (is_add)
            {
                if (pKey->ext_data)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                }
                SetDsScl0Ipv6Key320(A, ipAddr2_f, p_data, hw_ip6);
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                if (pKey->ext_mask)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                }
                SetDsScl0Ipv6Key320(A, ipAddr2_f, p_mask, hw_ip6);
            }
            else
            {
                SetDsScl0Ipv6Key320(A, ipAddr2_f, p_data, hw_ip6);
                SetDsScl0Ipv6Key320(A, ipAddr2_f, p_mask, hw_ip6);
            }
            break;
        case CTC_FIELD_KEY_IP_DSCP:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0Ipv6Key320(V, dscp_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, dscp_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, dscp_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, dscp_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_ECN:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3);
                SetDsScl0Ipv6Key320(V, ecn_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, ecn_f, p_mask, pKey->mask & 0x3);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, ecn_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, ecn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0Ipv6Key320(V, fragInfo_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, fragInfo_f, p_mask, tmp_mask & 0x1);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, fragInfo_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, fragInfo_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0Ipv6Key320(V, ipOptions_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, ipOptions_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, ipOptions_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, ipOptions_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_FLAGS:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x3F);
                SetDsScl0Ipv6Key320(V, tcpFlags_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, tcpFlags_f, p_mask, pKey->mask & 0x3F);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, tcpFlags_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, tcpFlags_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_TCP_ECN:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x7);
                SetDsScl0Ipv6Key320(V, tcpEcn_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, tcpEcn_f, p_mask, pKey->mask & 0x7);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, tcpEcn_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, tcpEcn_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, pKey->data);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_NVGRE_KEY:
            CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
            SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, is_add? (pKey->data << 8) & 0xFF00: 0);
            SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, is_add? (pKey->mask << 8) & 0xFF00: 0);
            SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, is_add? (pKey->data >> 8) & 0xFFFF: 0);
            SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, is_add? (pKey->mask >> 8) & 0xFFFF: 0);
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            if (is_add)
            {
            SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
            SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFFFF);
            SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFFFF);
                }
            else
            {
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_VN_ID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFFFF);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, pKey->data & 0xFFFF);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, (pKey->data >> 16) & 0xFF);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, (pKey->mask >> 16) & 0xFF);
            }
            else
            {
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4DestPort_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_ICMP_CODE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0xFF00;
                tmp_mask &= 0xFF00;
                tmp_data |= (pKey->data & 0xFF);
                tmp_mask |= (pKey->mask & 0xFF);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0xFF00;
                tmp_mask = tmp_mask & 0xFF00;
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_ICMP_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;
        case CTC_FIELD_KEY_IGMP_TYPE:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data &= 0x00FF;
                tmp_mask &= 0x00FF;
                tmp_data |= pKey->data << 8;
                tmp_mask |= pKey->mask << 8;
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            else
            {
                tmp_data = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data);
                tmp_mask = GetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask);
                tmp_data = tmp_data & 0x00FF;
                tmp_mask = tmp_mask & 0x00FF;
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_data, tmp_data);
                SetDsScl0Ipv6Key320(V, l4SourcePort_f, p_mask, tmp_mask);
            }
            break;

        case SYS_SCL_FIELD_KEY_COMMON:
            if (is_add)
            {
                SetDsScl0Ipv6Key320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320));
                SetDsScl0Ipv6Key320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0Ipv6Key320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320));
                SetDsScl0Ipv6Key320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0Ipv6Key320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320));
                SetDsScl0Ipv6Key320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0Ipv6Key320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320));
                SetDsScl0Ipv6Key320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0Ipv6Key320(V, isIpv6Key0_f, p_data, 1);
                SetDsScl0Ipv6Key320(V, isIpv6Key0_f, p_mask, 1);
                SetDsScl0Ipv6Key320(V, isIpv6Key1_f, p_data, 1);
                SetDsScl0Ipv6Key320(V, isIpv6Key1_f, p_mask, 1);

                SetDsScl0Ipv6Key320(V, _type0_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0Ipv6Key320(V, _type0_f, p_mask, 0x3);
                SetDsScl0Ipv6Key320(V, _type1_f, p_data, _sys_usw_scl_map_hw_ad_type(pe->action_type));
                SetDsScl0Ipv6Key320(V, _type1_f, p_mask, 0x3);
            }
            /*else
            {
                SetDsScl0Ipv6Key320(V, sclKeyType0_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType1_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType2_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType2_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType3_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, sclKeyType3_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, isIpv6Key0_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, isIpv6Key0_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, isIpv6Key1_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, isIpv6Key1_f, p_mask, 0);

                SetDsScl0Ipv6Key320(V, _type0_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, _type0_f, p_mask, 0);
                SetDsScl0Ipv6Key320(V, _type1_f, p_data, 0);
                SetDsScl0Ipv6Key320(V, _type1_f, p_mask, 0);
            }*/
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_udf(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_IP_HDR_ERROR :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0UdfKey160(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0UdfKey160(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0UdfKey160(V, ipHeaderError_f, p_data, 0);
                SetDsScl0UdfKey160(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0UdfKey160(V, isFragPkt_f, p_data, tmp_data);
                SetDsScl0UdfKey160(V, isFragPkt_f, p_mask, tmp_mask & 0x1);
            }
            else
            {
                SetDsScl0UdfKey160(V, isFragPkt_f, p_data, 0);
                SetDsScl0UdfKey160(V, isFragPkt_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L2_TYPE :
        case CTC_FIELD_KEY_VLAN_NUM:
            SetDsScl0UdfKey160(V, layer2Type_f, p_data, is_add? pKey->data: 0);
            SetDsScl0UdfKey160(V, layer2Type_f, p_mask, is_add? pKey->mask & 0x7: 0);
            break;
        case CTC_FIELD_KEY_L3_TYPE :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0UdfKey160(V, layer3Type_f, p_data, pKey->data);
                SetDsScl0UdfKey160(V, layer3Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0UdfKey160(V, layer3Type_f, p_data, 0);
                SetDsScl0UdfKey160(V, layer3Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0UdfKey160(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0UdfKey160(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0UdfKey160(V, layer4Type_f, p_data, 0);
                SetDsScl0UdfKey160(V, layer4Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_PORT :
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            CTC_PTR_VALID_CHECK(pKey->ext_mask);
            if(is_add)
            {
                ctc_field_port_t* p_port = (ctc_field_port_t*) (pKey->ext_data);
                ctc_field_port_t* p_port_mask = (ctc_field_port_t*) (pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsScl0UdfKey160(V, sclLabel7To2_f, p_data, (p_port->port_class_id) >> 2 & 0x3F);
                    SetDsScl0UdfKey160(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x03);
                    SetDsScl0UdfKey160(V, sclLabel7To2_f, p_mask, (p_port_mask->port_class_id) >> 2 & 0x3F);
                    SetDsScl0UdfKey160(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x03);
                }
                else if (CTC_FIELD_PORT_TYPE_NONE == p_port->type)
                {
                    /* do nothing */
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
            }
            else
            {
                SetDsScl0UdfKey160(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0UdfKey160(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0UdfKey160(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0UdfKey160(V, sclLabel1To0_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_UDF :
            if (is_add)
            {
                ctc_acl_udf_t* p_udf_data = (ctc_acl_udf_t*)pKey->ext_data;
                ctc_acl_udf_t* p_udf_mask = (ctc_acl_udf_t*)pKey->ext_mask;
                sys_acl_udf_entry_t* sys_udf_entry = NULL;
                uint32    hw_udf[4]= {0};

                CTC_PTR_VALID_CHECK(p_udf_data);
                CTC_PTR_VALID_CHECK(p_udf_mask);
                sys_usw_acl_get_udf_info(lchip, p_udf_data->udf_id, &sys_udf_entry);
                if (!sys_udf_entry || !sys_udf_entry->key_index_used)
                {
                    return CTC_E_NOT_EXIST;
                }
                SetDsScl0UdfKey160(V, udfHitIndex_f, p_data, sys_udf_entry->udf_hit_index);
                SetDsScl0UdfKey160(V, udfHitIndex_f, p_mask, 0xF);
                SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_data->udf);
                SetDsScl0UdfKey160(A, udf_f, p_data, hw_udf);
                SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_mask->udf);
                SetDsScl0UdfKey160(A, udf_f, p_mask, hw_udf);
            }
            else
            {
                uint8 zeros[CTC_ACL_UDF_BYTE_NUM] = {0};
                SetDsScl0UdfKey160(V, udfHitIndex_f, p_data, 0);
                SetDsScl0UdfKey160(V, udfHitIndex_f, p_mask, 0);
                SetDsScl0UdfKey160(A, udf_f, p_data, zeros);
                SetDsScl0UdfKey160(A, udf_f, p_mask, zeros);
            }
            SetDsScl0UdfKey160(V, udfValid_f, p_data, is_add? 0x1: 0);
            SetDsScl0UdfKey160(V, udfValid_f, p_mask, is_add? 0x1: 0);
            break;
        case SYS_SCL_FIELD_KEY_COMMON :
            if (is_add)
            {
                SetDsScl0UdfKey160(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY160));
                SetDsScl0UdfKey160(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0UdfKey160(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY160));
                SetDsScl0UdfKey160(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            /*else
            {
                SetDsScl0UdfKey160(V, sclKeyType0_f, p_data, 0);
                SetDsScl0UdfKey160(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0UdfKey160(V, sclKeyType1_f, p_data, 0);
                SetDsScl0UdfKey160(V, sclKeyType1_f, p_mask, 0);
            }*/
            break;
        default :
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_udf_ext(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void       * p_data = NULL;
    void       * p_mask = NULL;
    uint32     tmp_data = 0;
    uint32     tmp_mask = 0;
    uint8        key_share_mode = 0;
    uint32      cmd = 0;
    IpeUserIdTcamCtl_m      ipe_userid_tcam_ctl;
    ipv6_addr_t                  hw_ip6 = {0};
    hw_mac_addr_t           hw_mac = {0};
    uint32      tmp_array[2] = {0};
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    ctc_field_port_t* p_port = NULL;
    ctc_field_port_t* p_port_mask = NULL;
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    cmd = DRV_IOR(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_userid_tcam_ctl));
     switch (pe->group->priority)
     {
         case 0 :
             key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_0_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
             break;
         case 1 :
             key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_1_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
             break;
         case 2 :
             key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_2_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
             break;
         case 3 :
             key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_3_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
             break;
         default :
             break;
     }

    switch (pKey->type)
    {
        case CTC_FIELD_KEY_IP_HDR_ERROR :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0UdfKey320(V, ipHeaderError_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, ipHeaderError_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0UdfKey320(V, ipHeaderError_f, p_data, 0);
                SetDsScl0UdfKey320(V, ipHeaderError_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_FRAG :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, CTC_IP_FRAG_MAX - 1);
                CTC_ERROR_RETURN(_sys_usw_scl_get_ip_frag(lchip, pKey->data, &tmp_data, &tmp_mask));
                SetDsScl0UdfKey320(V, isFragPkt_f, p_data, tmp_data);
                SetDsScl0UdfKey320(V, isFragPkt_f, p_mask, tmp_mask & 0x1);
            }
            else
            {
                SetDsScl0UdfKey320(V, isFragPkt_f, p_data, 0);
                SetDsScl0UdfKey320(V, isFragPkt_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L2_TYPE :
        case CTC_FIELD_KEY_VLAN_NUM:
            SetDsScl0UdfKey320(V, layer2Type_f, p_data, is_add ? pKey->data: 0);
            SetDsScl0UdfKey320(V, layer2Type_f, p_mask, is_add ? pKey->mask & 0x7: 0);
            break;
        case CTC_FIELD_KEY_L3_TYPE :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0UdfKey320(V, layer3Type_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, layer3Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0UdfKey320(V, layer3Type_f, p_data, 0);
                SetDsScl0UdfKey320(V, layer3Type_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_TYPE :
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xF);
                SetDsScl0UdfKey320(V, layer4Type_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, layer4Type_f, p_mask, pKey->mask & 0xF);
            }
            else
            {
                SetDsScl0UdfKey320(V, layer4Type_f, p_data, 0);
                SetDsScl0UdfKey320(V, layer4Type_f, p_mask, 0);
            }
            break;

          case CTC_FIELD_KEY_IPV6_DA :
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            CTC_PTR_VALID_CHECK(pKey->ext_mask);
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (is_add)
            {
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_data, hw_ip6[3]);
                sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_mask, hw_ip6[3]);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_DA :
            if (key_share_mode < 2)
            {
                if (is_add)
                {
                    SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_data, pKey->data);
                    SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_mask, pKey->mask);
                }
                else
                {
                    SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_data, 0);
                    SetDsScl0UdfKey320(V, u_g1_ipDa_f, p_mask, 0);
                }
            }
            else if (3 == key_share_mode)
            {
                sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                if (is_add)
                {
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    tmp_array[1] = pKey->data;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                    tmp_array[1] = pKey->mask;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                }
                else
                {
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    tmp_array[1] = 0;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                    tmp_array[1] = 0;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                }
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 1 or 3! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            break;
        case CTC_FIELD_KEY_MPLS_LABEL2 :
        case CTC_FIELD_KEY_MPLS_LABEL1 :
        case CTC_FIELD_KEY_MPLS_LABEL0 :
        case CTC_FIELD_KEY_MPLS_EXP2 :
        case CTC_FIELD_KEY_MPLS_EXP1 :
        case CTC_FIELD_KEY_MPLS_EXP0 :
        case CTC_FIELD_KEY_MPLS_SBIT2 :
        case CTC_FIELD_KEY_MPLS_SBIT1 :
        case CTC_FIELD_KEY_MPLS_SBIT0 :
        case CTC_FIELD_KEY_MPLS_TTL2 :
        case CTC_FIELD_KEY_MPLS_TTL1 :
        case CTC_FIELD_KEY_MPLS_TTL0 :
            CTC_ERROR_RETURN(_sys_usw_scl_set_udf_mpls_key_field(lchip, pKey, key_share_mode, p_data, p_mask, is_add));
            break;
        case CTC_FIELD_KEY_IP_OPTIONS :
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0x1);
                SetDsScl0UdfKey320(V, u_g1_ipOptions_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g1_ipOptions_f, p_mask, pKey->mask & 0x1);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g1_ipOptions_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g1_ipOptions_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IPV6_SA :
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            CTC_PTR_VALID_CHECK(pKey->ext_mask);
            sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
            if (key_share_mode < 2)
            {
                if (is_add)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_data, &hw_ip6[2]);
                    sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_mask, &hw_ip6[2]);
                }
                else
                {
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_data, &hw_ip6[2]);
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_mask, &hw_ip6[2]);
                }
            }
            else if (3 == key_share_mode)
            {
                if (is_add)
                {
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_data);
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, &hw_ip6[2]);
                    sal_memset(hw_ip6, 0, sizeof(ipv6_addr_t));
                    SYS_USW_REVERT_IP6(hw_ip6, (uint32*)pKey->ext_mask);
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, &hw_ip6[2]);
                }
                else
                {
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, &hw_ip6[2]);
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, &hw_ip6[2]);
                }
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 1 or 3! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            break;
        case CTC_FIELD_KEY_IP_SA :
            if (key_share_mode < 2)
            {
                sal_memset(tmp_array, 0, sizeof(tmp_array));
                if (is_add)
                {
                    tmp_array[0] = pKey->data;
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_data, tmp_array);
                    sal_memset(tmp_array, 0, sizeof(tmp_array));
                    tmp_array[0] = pKey->mask;
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_mask, tmp_array);
                }
                else
                {
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_data, tmp_array);
                    SetDsScl0UdfKey320(A, u_g1_ipSa_f, p_mask, tmp_array);
                }
            }
            else if (3 == key_share_mode)
            {
                sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                if (is_add)
                {
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    tmp_array[0] = pKey->data;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                    tmp_array[0] = pKey->mask;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                }
                else
                {
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    tmp_array[0] = 0;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_data, tmp_array);
                    sal_memset(tmp_array, 0, 2 * sizeof(uint32));
                    GetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                    tmp_array[0] = 0;
                    SetDsScl0UdfKey320(A, u_g3_ipSa_f, p_mask, tmp_array);
                }
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 1 or 3! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            break;

        case CTC_FIELD_KEY_L4_DST_PORT :
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0UdfKey320(V, u_g1_l4DestPort_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g1_l4DestPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g1_l4DestPort_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g1_l4DestPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT :
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0UdfKey320(V, u_g1_l4SrcPort_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g1_l4SrcPort_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g1_l4SrcPort_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g1_l4SrcPort_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_IP_PROTOCOL :
            if (key_share_mode > 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 0 or 1! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFF);
                SetDsScl0UdfKey320(V, u_g1_layer3HeaderProtocol_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g1_layer3HeaderProtocol_f, p_mask, pKey->mask & 0xFF);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g1_layer3HeaderProtocol_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g1_layer3HeaderProtocol_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_CVLAN_ID :
            if (2 != key_share_mode)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 2! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFF);
                SetDsScl0UdfKey320(V, u_g2_cvlanId_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g2_cvlanId_f, p_mask, pKey->mask & 0xFFF);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g2_cvlanId_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g2_cvlanId_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_SVLAN_ID :
            if (2 == key_share_mode)
            {
                if (is_add)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0xFFF);
                    SetDsScl0UdfKey320(V, u_g2_svlanId_f, p_data, pKey->data);
                    SetDsScl0UdfKey320(V, u_g2_svlanId_f, p_mask, pKey->mask & 0xFFF);
                }
                else
                {
                    SetDsScl0UdfKey320(V, u_g2_svlanId_f, p_data, 0);
                    SetDsScl0UdfKey320(V, u_g2_svlanId_f, p_mask, 0);
                }
            }
            else if (3 == key_share_mode)
            {
                if (is_add)
                {
                    CTC_MAX_VALUE_CHECK(pKey->data, 0xFFF);
                    SetDsScl0UdfKey320(V, u_g3_svlanId_f, p_data, pKey->data);
                    SetDsScl0UdfKey320(V, u_g3_svlanId_f, p_mask, pKey->mask & 0xFFF);
                }
                else
                {
                    SetDsScl0UdfKey320(V, u_g3_svlanId_f, p_data, 0);
                    SetDsScl0UdfKey320(V, u_g3_svlanId_f, p_mask, 0);
                }
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 2 or 3! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            break;
        case CTC_FIELD_KEY_ETHER_TYPE :
            if (2 != key_share_mode)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 2! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pKey->data, 0xFFFF);
                SetDsScl0UdfKey320(V, u_g2_etherType_f, p_data, pKey->data);
                SetDsScl0UdfKey320(V, u_g2_etherType_f, p_mask, pKey->mask & 0xFFFF);
            }
            else
            {
                SetDsScl0UdfKey320(V, u_g2_etherType_f, p_data, 0);
                SetDsScl0UdfKey320(V, u_g2_etherType_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_MAC_SA :
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            CTC_PTR_VALID_CHECK(pKey->ext_mask);
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if (2 == key_share_mode)
            {
                if (is_add)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    SetDsScl0UdfKey320(A, u_g2_macSa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                    SetDsScl0UdfKey320(A, u_g2_macSa_f, p_mask, hw_mac);
                }
                else
                {
                    SetDsScl0UdfKey320(A, u_g2_macSa_f, p_data, hw_mac);
                    SetDsScl0UdfKey320(A, u_g2_macSa_f, p_mask, hw_mac);
                }
            }
            else if (3 == key_share_mode)
            {
                if (is_add)
                {
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                    SetDsScl0UdfKey320(A, u_g3_macSa_f, p_data, hw_mac);
                    sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                    SetDsScl0UdfKey320(A, u_g3_macSa_f, p_mask, hw_mac);
                }
                else
                {
                    SetDsScl0UdfKey320(A, u_g3_macSa_f, p_data, hw_mac);
                    SetDsScl0UdfKey320(A, u_g3_macSa_f, p_mask, hw_mac);
                }
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 2 or 3! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            break;
        case CTC_FIELD_KEY_MAC_DA :
            if (2 != key_share_mode)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Key share mode must be 2! \n");
                return CTC_E_SCL_INVALID_KEY;
            }
            CTC_PTR_VALID_CHECK(pKey->ext_data);
            CTC_PTR_VALID_CHECK(pKey->ext_mask);
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            if (is_add)
            {
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_data);
                SetDsScl0UdfKey320(A, u_g2_macDa_f, p_data, hw_mac);
                sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)pKey->ext_mask);
                SetDsScl0UdfKey320(A, u_g2_macDa_f, p_mask, hw_mac);
            }
            else
            {
                SetDsScl0UdfKey320(A, u_g2_macDa_f, p_data, hw_mac);
                SetDsScl0UdfKey320(A, u_g2_macDa_f, p_mask, hw_mac);
            }
            break;
        case CTC_FIELD_KEY_PORT :
            if(is_add)
            {
                p_port = (ctc_field_port_t*) (pKey->ext_data);
                p_port_mask = (ctc_field_port_t*) (pKey->ext_mask);
                if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
                {
                    SYS_GLOBAL_PORT_CHECK(p_port->gport);
                    SetDsScl0UdfKey320(V, globalPort_f, p_data, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port->gport));
                    SetDsScl0UdfKey320(V, globalPort_f, p_mask, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port_mask->gport));
                }
                else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
                {
                    CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
                    SetDsScl0UdfKey320(V, isLogicPort_f, p_data, 1);
                    SetDsScl0UdfKey320(V, isLogicPort_f, p_mask, 1);
                    SetDsScl0UdfKey320(V, globalPort_f, p_data, p_port->logic_port);
                    SetDsScl0UdfKey320(V, globalPort_f, p_mask, p_port_mask->logic_port);
                }
                else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
                {
                    SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
                    SetDsScl0UdfKey320(V, sclLabel7To2_f, p_data, (p_port->port_class_id) >> 2 & 0x3F);
                    SetDsScl0UdfKey320(V, sclLabel1To0_f, p_data, (p_port->port_class_id) & 0x03);
                    SetDsScl0UdfKey320(V, sclLabel7To2_f, p_mask, (p_port_mask->port_class_id) >> 2 & 0x3F);
                    SetDsScl0UdfKey320(V, sclLabel1To0_f, p_mask, (p_port_mask->port_class_id) & 0x03);
                }
                else if (CTC_FIELD_PORT_TYPE_NONE == p_port->type)
                {
                    /* do nothing */
                }
                else
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }
            }
            else
            {
                SetDsScl0UdfKey320(V, isLogicPort_f, p_data, 0);
                SetDsScl0UdfKey320(V, isLogicPort_f, p_mask, 0);
                SetDsScl0UdfKey320(V, globalPort_f, p_data, 0);
                SetDsScl0UdfKey320(V, globalPort_f, p_mask, 0);
                SetDsScl0UdfKey320(V, sclLabel7To2_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclLabel1To0_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclLabel7To2_f, p_mask, 0);
                SetDsScl0UdfKey320(V, sclLabel1To0_f, p_mask, 0);
            }
            break;
        case CTC_FIELD_KEY_UDF :
            if (is_add)
            {
                ctc_acl_udf_t* p_udf_data = (ctc_acl_udf_t*)pKey->ext_data;
                ctc_acl_udf_t* p_udf_mask = (ctc_acl_udf_t*)pKey->ext_mask;
                sys_acl_udf_entry_t* sys_udf_entry = NULL;
                uint32          hw_udf[4] = {0};

                CTC_PTR_VALID_CHECK(p_udf_data);
                CTC_PTR_VALID_CHECK(p_udf_mask);
                sys_usw_acl_get_udf_info(lchip, p_udf_data->udf_id, &sys_udf_entry);
                if (!sys_udf_entry || !sys_udf_entry->key_index_used)
                {
                    return CTC_E_NOT_EXIST;
                }
                SetDsScl0UdfKey320(V, udfHitIndex_f, p_data, sys_udf_entry->udf_hit_index);
                SetDsScl0UdfKey320(V, udfHitIndex_f, p_mask, 0xF);
                SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_data->udf);
                SetDsScl0UdfKey320(A, udf_f, p_data, hw_udf);
                SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_mask->udf);
                SetDsScl0UdfKey320(A, udf_f, p_mask, hw_udf);
            }
            else
            {
                uint8 zeros[CTC_ACL_UDF_BYTE_NUM] = {0};
                SetDsScl0UdfKey320(V, udfHitIndex_f, p_data, 0);
                SetDsScl0UdfKey320(V, udfHitIndex_f, p_mask, 0);
                SetDsScl0UdfKey320(A, udf_f, p_data, zeros);
                SetDsScl0UdfKey320(A, udf_f, p_mask, zeros);
            }
            SetDsScl0UdfKey320(V, udfValid_f, p_data, is_add? 0x1: 0x0);
            SetDsScl0UdfKey320(V, udfValid_f, p_mask, is_add? 0x1: 0x0);
            break;
        case SYS_SCL_FIELD_KEY_COMMON :
            if (is_add)
            {
                SetDsScl0UdfKey320(V, sclKeyType0_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320));
                SetDsScl0UdfKey320(V, sclKeyType0_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0UdfKey320(V, sclKeyType1_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320));
                SetDsScl0UdfKey320(V, sclKeyType1_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0UdfKey320(V, sclKeyType2_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320));
                SetDsScl0UdfKey320(V, sclKeyType2_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsScl0UdfKey320(V, sclKeyType3_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320));
                SetDsScl0UdfKey320(V, sclKeyType3_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
            }
            /*else
            {
                SetDsScl0UdfKey320(V, sclKeyType0_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclKeyType0_f, p_mask, 0);
                SetDsScl0UdfKey320(V, sclKeyType1_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclKeyType1_f, p_mask, 0);
                SetDsScl0UdfKey320(V, sclKeyType2_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclKeyType2_f, p_mask, 0);
                SetDsScl0UdfKey320(V, sclKeyType3_f, p_data, 0);
                SetDsScl0UdfKey320(V, sclKeyType3_f, p_mask, 0);
            }*/
            break;
        default :
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_tcam_key_mpls(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void       * p_data = NULL;
    void       * p_mask = NULL;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    p_temp_entry = pe->temp_entry;
    p_data = (void*)p_temp_entry->key;
    p_mask = (void*)p_temp_entry->mask;
    switch (pKey->type)
    {
        case SYS_SCL_FIELD_KEY_MPLS_LABEL:
            if (is_add)
            {
                SetDsUserId0TcamKey80(V, mplsLabel_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, mplsLabel_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserId0TcamKey80(V, mplsLabel_f, p_data, 0);
                SetDsUserId0TcamKey80(V, mplsLabel_f, p_mask, 0);
            }
            break;
        case SYS_SCL_FIELD_KEY_MPLS_LABEL_SPACE:
            if (is_add)
            {
            SetDsUserId0TcamKey80(V, labelSpace_f, p_data, pKey->data);
            SetDsUserId0TcamKey80(V, labelSpace_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserId0TcamKey80(V, labelSpace_f, p_data, 0);
                SetDsUserId0TcamKey80(V, labelSpace_f, p_mask, 0);
            }
            break;
        case SYS_SCL_FIELD_KEY_MPLS_IS_INTERFACEID:
            if (is_add)
            {
                SetDsUserId0TcamKey80(V, isInterfaceId_f, p_data, pKey->data);
                SetDsUserId0TcamKey80(V, isInterfaceId_f, p_mask, pKey->mask);
            }
            else
            {
                SetDsUserId0TcamKey80(V, isInterfaceId_f, p_data, 0);
                SetDsUserId0TcamKey80(V, isInterfaceId_f, p_mask, 0);
            }
            break;
        case SYS_SCL_FIELD_KEY_COMMON:
            if (is_add)
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT));
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, DRV_ENUM(DRV_SCL_KEY_TYPE_MASK));
                SetDsUserId0TcamKey80(V, hashType_f, p_data, USERIDHASHTYPE_TUNNELMPLS);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0x3F);
            }
            /*else
            {
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_data, 0);
                SetDsUserId0TcamKey80(V, sclKeyType_f, p_mask, 0);
                SetDsUserId0TcamKey80(V, hashType_f, p_data, 0);
                SetDsUserId0TcamKey80(V, hashType_f, p_mask, 0);
            }*/
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
    }
    return CTC_E_NONE;
}


#define _____build_action_____
#define SYS_USW_SCL_SET_BMP(bmp, bit, is_set) \
    do{\
        if(is_set)\
            CTC_BMP_SET(bmp, bit);\
        else\
            CTC_BMP_UNSET(bmp, bit);\
    }while(0)

STATIC int32
_sys_usw_scl_update_action(uint8 lchip, void* data, void* change_nh_param)
{
    sys_scl_sw_entry_t* pe = (sys_scl_sw_entry_t*)data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    ds_t   action;
    uint32 cmdr  = 0;
    uint32 cmdw  = 0;
    uint32 fwd_offset = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_key_index = 0;
    uint32 hw_ad_index = 0;
    uint8  igs_hash_key = 0;
    uint8 scl_id = 0;
    sys_scl_sw_hash_ad_t  new_profile;
    sys_scl_sw_hash_ad_t  old_profile;
    sys_scl_sw_hash_ad_t* out_profile = NULL;
    ctc_field_key_t      field_key;
    int32 ret = 0;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    uint32 old_ad_index = 0;
    sys_nh_info_dsnh_t nh_info;
    uint8 need_dsfwd = 0;
    uint32 key_id1 = 0;
    uint32 act_id1 = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&action, 0, sizeof(ds_t));
    sal_memset(&new_profile, 0, sizeof(sys_scl_sw_hash_ad_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&old_profile, 0, sizeof(sys_scl_sw_hash_ad_t));

	igs_hash_key = !(SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict) && (SYS_SCL_ACTION_EGRESS != pe->action_type);

    CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, (SYS_SCL_IS_HASH_COM_MODE(pe)? 0: pe->group->priority), pe, &key_id, &act_id));
    if (SYS_SCL_IS_HASH_COM_MODE(pe))
    {
        _sys_usw_scl_get_table_id(lchip, 1, pe, &key_id1, &act_id1);
    }
    /* get key_index and ad_index */
    CTC_ERROR_RETURN(_sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, &hw_ad_index));
    old_ad_index = hw_ad_index;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "update bind scl action, key_tbl_id:[%u]  ad_tbl_id:[%u]  ad_index:[%u]\n", key_id, act_id, hw_ad_index);

    cmdr = DRV_IOR(act_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(act_id, DRV_ENTRY_FLAG);

    if (!pe->temp_entry)
    {
        DRV_IOCTL(lchip, hw_ad_index, cmdr, &action);
    }
    else
    {
        sal_memcpy(&action, &pe->temp_entry->action, sizeof(action));
    }

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pe->nexthop_id, &nh_info, 1));
    if (nh_info.aps_en && !nh_info.dsfwd_valid)
    {
        /*nexthop using merge dsfwd, and update from not aps to aps, in this case should use dsfwd*/
        need_dsfwd = 1;
    }

    if (!change_nh_param || need_dsfwd || p_dsnh_info->cloud_sec_en)
    {
        if (GetDsUserId(V, dsFwdPtrValid_f, &action))
        {
            return CTC_E_NONE;
        }

        if (pe->u2_type != SYS_AD_UNION_G_4)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "nh already in use, cannot update to dsfwd! \n");
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Already in used \n");
			return CTC_E_IN_USE;

        }

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwd_offset, 1));

        if (pe->u2_type == SYS_AD_UNION_G_1)
        {
            /*scl already change to using dsfwd, to avoid cb function entry twice*/
            return CTC_E_NONE;
        }

        /*clear u4*/
        SetDsUserId(V, adDestMap_f, &action, 0);
        SetDsUserId(V, adNextHopPtrHighBits_f, &action, 0);
        SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_3,DsUserId,4,&action, 0);
        /*clear u2*/
        SetDsUserId(V, adNextHopPtrLowBits_f, &action, 0);
        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_4,DsUserId,2,&action, 0);
        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsUserId,2,&action, 1);
        /*2. update ad*/
        SetDsUserId(V, dsFwdPtrValid_f, &action, 1);
        SetDsUserId(V, dsFwdPtr_f, &action, fwd_offset);
        SetDsUserId(V, forwardOnStatus_f, &action, 0);

        pe->u2_bitmap = 0;
        pe->u4_bitmap = 0;
        pe->bind_nh = 0;
    }
    else
    {
        uint8 adjust_len_idx = 0;
        if(GetDsUserId(V, dsFwdPtrValid_f, &action) == 1)
        {
             return CTC_E_NONE;
        }

        SetDsUserId(V, dsFwdPtrValid_f, &action, 0);
        SetDsUserId(V, adDestMap_f, &action, p_dsnh_info->dest_map);
        SetDsUserId(V, adNextHopPtrHighBits_f, &action, (p_dsnh_info->dsnh_offset>>16)&0x3);
        SetDsUserId(V, adNextHopPtrLowBits_f, &action, (p_dsnh_info->dsnh_offset)&0xFFFF);
        SetDsUserId(V, adNextHopExt_f, &action, p_dsnh_info->nexthop_ext);
        if(0 != p_dsnh_info->adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
            SetDsUserId(V, adLengthAdjustType_f, &action, adjust_len_idx);
        }
        else
        {
            SetDsUserId(V, adLengthAdjustType_f, &action, 0);
        }
        /**<  [TM] Enable aps bridge */
        SetDsUserId(V, adApsBridgeEn_f, &action, p_dsnh_info->aps_en);
    }

    /*Update spool */
    if ((igs_hash_key) && (!pe->temp_entry))
    {
        new_profile.action_type = pe->action_type;
        sal_memcpy(&new_profile.action, &action, sizeof(DsUserId_m));
        sal_memcpy(&old_profile, pe->hash_ad, sizeof(sys_scl_sw_hash_ad_t));
        scl_id = ((1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM)||SYS_SCL_IS_HASH_COM_MODE(pe))? 0: pe->group->priority);
        CTC_MAX_VALUE_CHECK(scl_id, 1);
        ctc_spool_add(p_usw_scl_master[lchip]->ad_spool[scl_id], &new_profile, pe->hash_ad, &out_profile);
        if (out_profile)
        {
            hw_ad_index = out_profile->ad_index;
            pe->hash_ad = out_profile;
        }
    }

    /*write ad*/
    if (!pe->temp_entry)
    {
        uint32 cmd_ds = 0;

        DRV_IOCTL(lchip, hw_ad_index, cmdw, &action);
        if (SYS_SCL_IS_HASH_COM_MODE(pe))
        {
            cmd_ds = DRV_IOW(act_id1, DRV_ENTRY_FLAG);
            ret = ret ? ret: DRV_IOCTL(lchip, hw_ad_index, cmd_ds, &action);
        }
    }
    else
    {
        sal_memcpy(&pe->temp_entry->action, &action, sizeof(action));
    }

    /*update key for ad index not match*/
    if ((hw_ad_index != pe->ad_index) && igs_hash_key && (!pe->temp_entry))
    {
        drv_acc_in_t in;
        drv_acc_out_t out;
        uint32 key_tbl[2] = {key_id, key_id1};
        uint8 key_num = SYS_SCL_IS_HASH_COM_MODE(pe)?2:1;
        uint8 index = 0;

        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));

        pe->ad_index = hw_ad_index;
        p_temp_entry = mem_malloc(MEM_SCL_MODULE, sizeof(sys_scl_sw_temp_entry_t));
        if (!p_temp_entry)
        {
            ret = CTC_E_NO_MEMORY;
            goto error0;
        }

        sal_memset(p_temp_entry, 0, sizeof(sys_scl_sw_temp_entry_t));

        for (index = 0; index < key_num; index++)
        {
            in.type = DRV_ACC_TYPE_LOOKUP;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            in.tbl_id = key_tbl[index];
            in.data   = (void*)p_temp_entry->key;
            in.index  = hw_key_index;
            CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, error1);

	        sal_memcpy(p_temp_entry->key, out.data, sizeof(uint32) * SYS_SCL_MAX_KEY_SIZE_IN_WORD);

            pe->temp_entry = p_temp_entry;
            field_key.type = SYS_SCL_FIELD_KEY_AD_INDEX;
            field_key.data = hw_ad_index;
            CTC_ERROR_GOTO(p_usw_scl_master[lchip]->build_key_func[pe->key_type](lchip, &field_key, pe, TRUE), ret, error1);

            in.type = DRV_ACC_TYPE_ADD;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            in.tbl_id = key_tbl[index];
            in.data   = (void*)p_temp_entry->key;
            in.index  = hw_key_index;
            ret = drv_acc_api(lchip, &in, &out);
            if (ret)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Scl update ad process, write key failed, cannot rollback! \n");
            }
        }
        mem_free(pe->temp_entry);
    }

    return CTC_E_NONE;

error1:
    if (p_temp_entry)
    {
        mem_free(pe->temp_entry);
    }

error0:
    if ((igs_hash_key)&&(!pe->temp_entry))
    {
        ctc_spool_add(p_usw_scl_master[lchip]->ad_spool[scl_id], &old_profile, &new_profile, &out_profile);
        pe->ad_index = old_ad_index;
    }

    return ret;
}

int32
_sys_usw_scl_bind_nexthop(uint8 lchip, sys_scl_sw_entry_t* pe,uint32 nh_id)
{
    sys_nh_update_dsnh_param_t update_dsnh;
	int32 ret = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    /* nh don't use dsfwd, need bind nh and dsacl */
    update_dsnh.data = pe;
    update_dsnh.updateAd = _sys_usw_scl_update_action;
    ret = sys_usw_nh_bind_dsfwd_cb(lchip, nh_id, &update_dsnh);
    if(CTC_E_NONE == ret)
    {
       /*MergeDsFwd Mode need bind ad and nh*/
       pe->bind_nh = 1;
    }
    return ret;
}


int32
_sys_usw_scl_unbind_nexthop(uint8 lchip, sys_scl_sw_entry_t* pe, uint32 nh_id)
{
    int32 ret = 0;

    if (pe->bind_nh && nh_id)
    {
        sys_nh_update_dsnh_param_t update_dsnh;
        update_dsnh.data = NULL;
        update_dsnh.updateAd = NULL;
        ret = sys_usw_nh_bind_dsfwd_cb(lchip, nh_id, &update_dsnh);
        pe->bind_nh = 0;
    }

    return ret;
}


int32
_sys_usw_scl_build_field_action_igs_vlan_edit(uint8 lchip, sys_scl_sw_entry_t* pe, ctc_scl_field_action_t* pAction, uint8 is_add)
{
    ctc_scl_vlan_edit_t* vlan_edit = NULL;
    uint8  vlan_edit_valid = 0;
    uint16 profile_id = 0;
    void* ds_userid = (void*)(&(pe->temp_entry->action));
    ctc_scl_vlan_edit_t vlan_edit_new;

    SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);

    /*---------------------------------------------------------------*/
    /*update DB (vlan profile)*/

    if (is_add) /*Add or update vlan edit */
    {
        CTC_PTR_VALID_CHECK(pAction->ext_data);
        vlan_edit = (ctc_scl_vlan_edit_t*)(pAction->ext_data);

        if (vlan_edit->ctag_op
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
        {
            /*only in specific stuation need check u3_g2 */
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
        }
        CTC_MAX_VALUE_CHECK(vlan_edit->tpid_index, 3)
        if (pe->vlan_edit)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove vlan profile :%d\n", pe->vlan_edit->profile_id);
        }

       /*Add/Update new vlan edit*/
        pe->vlan_edit = sys_usw_scl_add_vlan_edit_action_profile(lchip, vlan_edit, (void*)pe->vlan_edit);

        if (NULL == pe->vlan_edit)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
            return CTC_E_NO_RESOURCE;
        }

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add new vlan profile :%d\n", pe->vlan_edit->profile_id);

        /*vlan edit valid*/
        profile_id = pe->vlan_edit->profile_id;
        vlan_edit_valid = 1;

    }
    else   /*Remove or Recovery vlan edit */
    {

        /*Remove vlan edit*/
        if (NULL == pe->vlan_edit)
        {
            return CTC_E_NOT_EXIST;
        }

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove vlan profile :%d\n", pe->vlan_edit->profile_id);

        sal_memset(&vlan_edit_new, 0, sizeof(ctc_scl_vlan_edit_t));

        sys_usw_scl_unmapping_vlan_edit(lchip, &vlan_edit_new, &(pe->vlan_edit->action_profile));
        vlan_edit = &vlan_edit_new;

        sys_usw_scl_remove_vlan_edit_action_profile(lchip, pe->vlan_edit);

        pe->vlan_edit = NULL;

        /*vlan edit valid*/
        vlan_edit_valid = 0;


    }

    /*---------------------------------------------------------------*/
    /*update DB (temp buffer)*/
    if (vlan_edit_valid)
    {
        /* refer to ctc_vlan_tag_sl_t, only CTC_VLAN_TAG_SL_NEW need to write hw table */
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl)
        {
            SetDsUserId(V, userCcfi_f, ds_userid, vlan_edit->ccfi_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl)
        {
            SetDsUserId(V, userCcos_f, ds_userid, vlan_edit->ccos_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
        {
            SetDsUserId(V, userCvlanId_f, ds_userid, vlan_edit->cvid_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scfi_sl)
        {
            SetDsUserId(V, userScfi_f, ds_userid, vlan_edit->scfi_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scos_sl)
        {
            SetDsUserId(V, userScos_f, ds_userid, vlan_edit->scos_new);
        }
        if (CTC_VLAN_TAG_SL_NEW == vlan_edit->svid_sl)
        {
            SetDsUserId(V, userSvlanId_f, ds_userid, vlan_edit->svid_new);
        }
        SetDsUserId(V, vlanActionProfileId_f, ds_userid, profile_id);
        SetDsUserId(V, svlanTagOperationValid_f, ds_userid, vlan_edit->stag_op ? 1 : 0);
        if (vlan_edit->ctag_op)
        {
            /* if do not do this judge, we will write u3_g2_cvlanTagOperationValid_f even if we do not config ctag vlan edit */
            SetDsUserId(V, cvlanTagOperationValid_f, ds_userid, 1);
        }
        if (vlan_edit->ctag_op)
        {
            SetDsUserId(V, u3Type_f, ds_userid, 1);
        }
        SetDsUserId(V, vlanActionProfileValid_f, ds_userid, 1);
    }
    else
    {
        if (vlan_edit->ctag_op
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl
            || CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
        {
            /*only in specific stuation need check u3_g2 */
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, userCcfi_f, ds_userid, 0);
            SetDsUserId(V, userCcos_f, ds_userid, 0);
            SetDsUserId(V, userCvlanId_f, ds_userid, 0);
            SetDsUserId(V, cvlanTagOperationValid_f, ds_userid, 0);
        }

        SetDsUserId(V, userScfi_f, ds_userid, 0);
        SetDsUserId(V, userScos_f, ds_userid, 0);
        SetDsUserId(V, userSvlanId_f, ds_userid, 0);
        SetDsUserId(V, vlanActionProfileId_f, ds_userid, 0);
        SetDsUserId(V, svlanTagOperationValid_f, ds_userid, 0);
        SetDsUserId(V, vlanActionProfileValid_f, ds_userid, 0);
    }


    /*---------------------------------------------------------------*/
    /*update DB (union bitmap)*/
    if (vlan_edit->ctag_op
        || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl
        || CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl
        || CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
    {
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsUserId,3,ds_userid, is_add);
    }

    SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);


    return CTC_E_NONE;

}

int32
_sys_usw_scl_build_field_action_igs_wlan_client(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{

    ctc_scl_vlan_edit_t*      vlan_edit;
    uint16 profile_id = 0;
    sys_scl_wlan_client_t* p_wlan_client;
    uint32 temp_data = 0;
    void* ds_userid = (void*)(&(pe->temp_entry->action));

    /* new add code :vlan_edit & acl lookup */
    if (is_add)
    {
        CTC_PTR_VALID_CHECK(pAction->ext_data);
        p_wlan_client = (sys_scl_wlan_client_t*)(pAction->ext_data);
        vlan_edit = &p_wlan_client->vlan_edit;
        if (p_wlan_client->is_vlan_edit)
        {

            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);

            pe->vlan_edit = sys_usw_scl_add_vlan_edit_action_profile(lchip, vlan_edit, (void*)pe->vlan_edit);
            if (!pe->vlan_edit)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			    return CTC_E_NO_RESOURCE;
            }

            SetDsUserId(V, userSvlanId_f, ds_userid, vlan_edit->svid_new);
            SetDsUserId(V, userScos_f, ds_userid, vlan_edit->scos_new);
            SetDsUserId(V, userScfi_f, ds_userid, vlan_edit->scfi_new);
            SetDsUserId(V, vlanActionProfileId_f, ds_userid, pe->vlan_edit->profile_id);
            SetDsUserId(V, vlanActionProfileValid_f, ds_userid, 1);
            SetDsUserId(V, svlanTagOperationValid_f, ds_userid, 1);

            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);
        }

        if(p_wlan_client->acl_lkup_num != 0)
        {
            if (pe->acl_profile)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
			    return CTC_E_EXIST;
            }

            SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_2);

            pe->acl_profile = sys_usw_scl_add_acl_control_profile( lchip, 0, p_wlan_client->acl_lkup_num, p_wlan_client->acl_prop, &profile_id);
            if (!pe->acl_profile)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			    return CTC_E_NO_RESOURCE;
            }

            SetDsUserId(V, capwapVlanActionProfileId_f, ds_userid, 0);
            SetDsUserId(V, capwapUserScfi_f, ds_userid, 1);
            temp_data = (pe->acl_profile->profile_id&0xF) << 8 | p_wlan_client->acl_prop[0].class_id;
            SetDsUserId(V, capwapUserSvlanId_f, ds_userid, temp_data & 0xFFF);

            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_2,DsUserId,4,ds_userid, is_add);
        }

        if(p_wlan_client->vrfid)
        {
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, fidSelect_f, ds_userid, 0x3);
            SetDsUserId(V, fid_f, ds_userid, p_wlan_client->vrfid);

            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsUserId,2,ds_userid, is_add);
        }

        SetDsUserId(V, forwardOnStatus_f, ds_userid, p_wlan_client->forward_on_status);
        SetDsUserId(V, staRoamStatus_f, ds_userid, p_wlan_client->sta_roam_status);
        SetDsUserId(V, staStatusCheckEn_f, ds_userid, p_wlan_client->sta_status_chk_en);
        SetDsUserId(V, notLocalSta_f, ds_userid, p_wlan_client->not_local_sta);

    }
    else
    {
        sys_usw_scl_remove_vlan_edit_action_profile(lchip, pe->vlan_edit);
        pe->vlan_edit = NULL;
        sys_usw_scl_remove_acl_control_profile(lchip, pe->acl_profile);
        pe->acl_profile = NULL;

        SetDsUserId(V, userSvlanId_f, ds_userid, 0);
        SetDsUserId(V, userScos_f, ds_userid, 0);
        SetDsUserId(V, userScfi_f, ds_userid, 0);
        SetDsUserId(V, vlanActionProfileId_f, ds_userid, 0);
        SetDsUserId(V, vlanActionProfileValid_f, ds_userid, 0);
        SetDsUserId(V, svlanTagOperationValid_f, ds_userid, 0);

        SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
        SetDsUserId(V, staRoamStatus_f, ds_userid, 0);
        SetDsUserId(V, staStatusCheckEn_f, ds_userid, 0);
        SetDsUserId(V, notLocalSta_f, ds_userid,0);

        SetDsUserId(V, fidSelect_f, ds_userid, 0);
        SetDsUserId(V, fid_f, ds_userid, 0);

        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);
        SetDsUserId(V, capwapVlanActionProfileId_f, ds_userid, 0);
        SetDsUserId(V, capwapUserScfi_f, ds_userid, 0);
        SetDsUserId(V, capwapUserSvlanId_f, ds_userid, 0);
        SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_2,DsUserId,4,ds_userid, is_add);

        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsUserId,2,ds_userid, is_add);
    }

    return CTC_E_NONE;
}


/*Used for cancel all and pending entry*/
int32
_sys_usw_scl_build_field_action_igs_cancel_all(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* ds_userid = NULL;
    uint16 old_logic_port = 0;
    uint32 cmd   = 0;
    sys_scl_sp_vlan_edit_t* p_vlan_edit = NULL;
    sys_scl_action_acl_t* p_acl_profile = NULL;
    sys_scl_sw_temp_entry_t temp_entry;

    sal_memset(&temp_entry, 0, sizeof(sys_scl_sw_temp_entry_t));

    /* add cancel all is support and remove cancel all is not defined */
    if (pAction->type == CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL
        && !is_add)
    {
        return CTC_E_NONE;
    }

    if (is_add)
    {
        if (pAction->type == CTC_SCL_FIELD_ACTION_TYPE_PENDING)
        {
            if (pe->p_pe_backup)
            {
                return CTC_E_NONE;
            }

            /* malloc an empty group */
            MALLOC_ZERO(MEM_SCL_MODULE, pe->p_pe_backup, sizeof(sys_scl_sw_entry_t));
            if (!pe->p_pe_backup)
            {
                return  CTC_E_NO_MEMORY;
            }
            sal_memcpy(pe->p_pe_backup, pe, sizeof(sys_scl_sw_entry_t));
        }

        old_logic_port = GetDsUserId(V, logicSrcPort_f, &pe->temp_entry->action);
        if (old_logic_port && pe->service_id)
        {
            /* make sure that this operation do not lead to packet loss */
            sys_usw_qos_unbind_service_logic_srcport(lchip, pe->service_id, old_logic_port);
        }
        pe->service_id = 0;

        /*If intstalled entry with un-install action, update to db*/
        ds_userid = (void*)(&(pe->temp_entry->action));
        sal_memset(ds_userid, 0, sizeof(pe->temp_entry->action));

        SetDsUserId(V, isHalf_f, ds_userid, pe->is_half);
        SetDsUserId(V, dsType_f, ds_userid, SYS_SCL_AD_TYPE_SCL);

        /*remove spool node*/
        if (pe->vlan_edit)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove vlan profile :%d\n", pe->vlan_edit->profile_id);
            ctc_spool_remove(p_usw_scl_master[lchip]->vlan_edit_spool, pe->vlan_edit, NULL);
            pe->vlan_edit = NULL;
            pe->vlan_profile_id = 0xFFFF;
        }

        if (pe->acl_profile)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove acl profile :%d\n", pe->acl_profile->profile_id);
            ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, pe->acl_profile, NULL);
            pe->acl_profile = NULL;
            pe->acl_profile_id = 0xFFFF;
        }

        _sys_usw_scl_unbind_nexthop(lchip, pe, pe->nexthop_id);
        pe->nexthop_id = 0;

        pe->u1_bitmap = 0;
        pe->u2_bitmap = 0;
        pe->u3_bitmap = 0;
        pe->u4_bitmap = 0;
        pe->u5_bitmap = 0;
        pe->u1_type = 0;
        pe->u2_type = 0;
        pe->u3_type = 0;
        pe->u4_type = 0;
        pe->u5_type = 0;

        sal_memset((uint8*)(pe->action_bmp), 0, sizeof(pe->action_bmp));
        pe->vpws_oam_en = 0;
    }
    else
    {
        if (!pe->p_pe_backup)
        {
            return CTC_E_NONE;
        }
        /* new added profile */
        p_vlan_edit = pe->vlan_edit;
        p_acl_profile = pe->acl_profile;

        sal_memcpy(pe, pe->p_pe_backup, sizeof(sys_scl_sw_entry_t));
        mem_free(pe->p_pe_backup);
        pe->p_pe_backup = NULL;

        pe->vlan_edit = p_vlan_edit;
        pe->acl_profile = p_acl_profile;

        _sys_usw_scl_bind_nexthop(lchip, pe, pe->nexthop_id);

        /* pe has already recovered from p_pe_backup, sw table and hw table are all original status */
        if (pe->is_default)
        {
            /* default entry can not use _sys_usw_scl_recovery_db_from_hw */
            cmd = DRV_IOR(DsUserId_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, pe->ad_index, cmd, &temp_entry.action);
        }
        else
        {
            _sys_usw_scl_recovery_db_from_hw(lchip, pe, &temp_entry);
        }

        old_logic_port = GetDsUserId(V, logicSrcPort_f, &temp_entry.action);
        if (old_logic_port && pe->service_id)
        {
            sys_usw_qos_bind_service_logic_srcport(lchip, pe->service_id, old_logic_port);
        }

        /*Recovery vlan edit, 0 is also reasonable */
        if (0xFFFF != pe->vlan_profile_id)
        {
            sys_scl_sp_vlan_edit_t hw_vlan_edit;
            sys_scl_sp_vlan_edit_t* p_out_vlan_edit = NULL;

            sal_memset(&hw_vlan_edit, 0, sizeof(hw_vlan_edit));


            /*Recovery vlan edit from hardware */
            cmd = DRV_IOR(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->vlan_profile_id, cmd, &hw_vlan_edit.action_profile));
            hw_vlan_edit.profile_id = pe->vlan_profile_id;

            /*Add hardward vlan profile*/
            ctc_spool_add(p_usw_scl_master[lchip]->vlan_edit_spool, &hw_vlan_edit, pe->vlan_edit, &p_out_vlan_edit);

            if (NULL == p_out_vlan_edit)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                return CTC_E_NO_RESOURCE;
            }

            pe->vlan_edit = p_out_vlan_edit;
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Recovery old vlan profile :%d\n", pe->vlan_edit->profile_id);

        }
        else if (pe->vlan_edit)
        {
            /* old do not have vlan_edit and new have vlan_edit */
            ctc_spool_remove(p_usw_scl_master[lchip]->vlan_edit_spool, pe->vlan_edit, NULL);
            pe->vlan_edit = NULL;
        }


        /*Recovery acl profile*/
        if (0xFFFF != pe->acl_profile_id)
        {
            sys_scl_action_acl_t hw_acl;
            sys_scl_action_acl_t* p_out_acl_profile = NULL;
            sal_memset(&hw_acl, 0, sizeof(hw_acl));

            /*Recovery vlan edit from hardware */
            cmd = DRV_IOR(DsSclAclControlProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->acl_profile_id, cmd, &hw_acl.acl_control_profile));
            hw_acl.profile_id = pe->acl_profile_id;
            hw_acl.is_scl = 1;

            /*Add hardward vlan profile*/
            ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &hw_acl, pe->acl_profile, &p_out_acl_profile);

            if (NULL == p_out_acl_profile)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                return CTC_E_NO_RESOURCE;
            }

            pe->acl_profile = p_out_acl_profile;

            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Recovery old vlan profile :%d\n", pe->acl_profile->profile_id);

        }
        else if (pe->acl_profile)
        {
            /* old do not have acl_profile and new have acl_profile */
            ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, pe->acl_profile, NULL);
            pe->acl_profile = NULL;
        }

        mem_free(pe->temp_entry);

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_remove_old_action(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint32 old_type, uint32 new_type)
{
    ctc_scl_field_action_t scl_action;
    sal_memset(&scl_action, 0, sizeof(ctc_scl_field_action_t));
    if (CTC_BMP_ISSET(pe->action_bmp, old_type) && (new_type == pAction->type))
    {
        scl_action.type = old_type;
        CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_igs(lchip, &scl_action, pe, FALSE));
    }
    return CTC_E_NONE;
}

int32
_sys_usw_scl_build_field_action_igs(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    sys_nh_info_dsnh_t nhinfo;
    uint32 temp_data = 0;
    uint8 type = 0;
    uint32 nexthop_id  = 0;
    sys_qos_policer_param_t policer_param;
    void* ds_userid = NULL;
    uint8   have_dsfwd = 0;
    uint8  service_queue_mode = 0;
    ctc_scl_field_action_t  field_action;
    ctc_scl_logic_port_t* p_logic_port = NULL;
    uint16 old_logic_port = 0;

    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    ds_userid = (void*)(&(pe->temp_entry->action));

    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_ERROR_RETURN(sys_usw_queue_get_service_queue_mode(lchip, &service_queue_mode));
    old_logic_port = GetDsUserId(V, logicSrcPort_f, &pe->temp_entry->action);

    if (is_add)     /*Add new action, must remove old action first, when old action is conflict with new action*/
    {
        CTC_ERROR_RETURN(_sys_usw_scl_remove_old_action(lchip, pAction, pe, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT, CTC_SCL_FIELD_ACTION_TYPE_DISCARD));
        CTC_ERROR_RETURN(_sys_usw_scl_remove_old_action(lchip, pAction, pe, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT, CTC_SCL_FIELD_ACTION_TYPE_DISCARD));
        CTC_ERROR_RETURN(_sys_usw_scl_remove_old_action(lchip, pAction, pe, CTC_SCL_FIELD_ACTION_TYPE_DISCARD, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT));
        CTC_ERROR_RETURN(_sys_usw_scl_remove_old_action(lchip, pAction, pe, CTC_SCL_FIELD_ACTION_TYPE_DISCARD, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT));
    }

    switch (pAction->type)
    {
        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_TYPE:
            CTC_ERROR_RETURN(DRV_IS_DUET2(lchip) ? CTC_E_NOT_SUPPORT : CTC_E_NONE);
            SetDsUserId(V, logicPortType_f, ds_userid, is_add? 1: 0);
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON:
            SetDsUserId(V, dsType_f, ds_userid, is_add? SYS_SCL_AD_TYPE_SCL: 0);
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_IS_HALF:
            if (is_add)
            {
                SetDsUserId(V, isHalf_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, isHalf_f, ds_userid, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_PENDING:
        case CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL:
             return _sys_usw_scl_build_field_action_igs_cancel_all(lchip, pAction, pe, is_add);
             break;

         case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT, CTC_SCL_FIELD_ACTION_TYPE_DISCARD);
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT, CTC_SCL_FIELD_ACTION_TYPE_DISCARD);
             SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            if (is_add)
            {
                if (pe->bind_nh)
                {
                    /*already use merge dsfwd*/
                    pe->u4_bitmap = 0;
                    pe->u2_bitmap = 0;
                    pe->u2_type    = SYS_AD_UNION_G_NA;
                    _sys_usw_scl_unbind_nexthop(lchip, pe, pe->nexthop_id);
                }

                SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_1);
                /*using dsfwd*/
                SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 1);
			    SetDsUserId(V, dsFwdPtr_f, ds_userid, 0xFFFF);
            }
            else
            {
                SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
			    SetDsUserId(V, dsFwdPtr_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsUserId,2,ds_userid, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN:
            if (is_add)
            {
                SetDsUserId(V, serviceAclQosEn_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, serviceAclQosEn_f, ds_userid, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT:
            if(is_add)
            {
                p_logic_port =  (ctc_scl_logic_port_t*) pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_logic_port->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
                CTC_MAX_VALUE_CHECK(p_logic_port->logic_port_type, 1);
            }
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            if(CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID))
            {
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_qos_bind_service_logic_srcport(lchip, pe->service_id,p_logic_port->logic_port));
                    if(old_logic_port && old_logic_port != p_logic_port->logic_port )
                    {
                        sys_usw_qos_unbind_service_logic_srcport(lchip, pe->service_id,old_logic_port);
                    }
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_srcport(lchip, pe->service_id,old_logic_port));
                }
            }
            if (is_add)
            {
                SetDsUserId(V, bindingEn_f, ds_userid, 0);
			    SetDsUserId(V, logicSrcPort_f, ds_userid, p_logic_port->logic_port);
                SetDsUserId(V, logicPortType_f, ds_userid, p_logic_port->logic_port_type);
            }
            else
            {
            	SetDsUserId(V, logicSrcPort_f, ds_userid, 0);
                SetDsUserId(V, logicPortType_f, ds_userid, 0);
            }

            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STP_ID:
            if (is_add)
            {
                int32 ret = 0;

                CTC_MAX_VALUE_CHECK(pAction->data0, 127);
                CTC_MIN_VALUE_CHECK(pAction->data0, DRV_IS_DUET2(lchip)? 1: 0);
                if (pe->nexthop_id)
                {
                    /*change to use dsfwd*/
                    ret = _sys_usw_scl_update_action(lchip, pe, NULL);
                    if (ret)
                    {
                        return CTC_E_PARAM_CONFLICT;
                    }
                    pe->bind_nh = 0;
                }
                SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_1);
                SetDsUserId(V, staStatusCheckEn_f, ds_userid, 0);
                SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                SetDsUserId(V, stpId_f, ds_userid, pAction->data0);
                SetDsUserId(V, stpIdValid_f, ds_userid, (128 > pAction->data0 ? 1: 0));
            }
            else
            {
                SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_1);
                SetDsUserId(V, staStatusCheckEn_f, ds_userid, 0);
                SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                SetDsUserId(V, stpId_f, ds_userid, 0);
                SetDsUserId(V, stpIdValid_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_1,DsUserId,4,ds_userid, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_APS:
            SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_1);
            if (is_add)
            {
                ctc_scl_aps_t* aps = pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);

                if (aps->protected_vlan_valid)
                {
                    CTC_VLAN_RANGE_CHECK(aps->protected_vlan);
                }
                SYS_APS_GROUP_ID_CHECK(aps->aps_select_group_id);
                CTC_MAX_VALUE_CHECK(aps->is_working_path, 1);

                SetDsUserId(V, staStatusCheckEn_f, ds_userid, 0);

                SetDsUserId(V, apsSelectValid_f, ds_userid, 1);
                if (aps->protected_vlan_valid)
                {
                    SetDsUserId(V, userVlanPtr_f, ds_userid, aps->protected_vlan);
                }
                SetDsUserId(V, apsSelectProtectingPath_f, ds_userid, !aps->is_working_path);
                SetDsUserId(V, apsSelectGroupId_f, ds_userid, aps->aps_select_group_id);
            }
            else
            {
                SetDsUserId(V, apsSelectValid_f, ds_userid, 0);
                SetDsUserId(V, userVlanPtr_f, ds_userid, 0);
                SetDsUserId(V, apsSelectProtectingPath_f, ds_userid, 0);
                SetDsUserId(V, apsSelectGroupId_f, ds_userid, 0);

            }
           SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_1,DsUserId,4,ds_userid, is_add);
           break;
        case SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT:
            CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_igs_wlan_client(lchip, pAction, pe,is_add));
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT:
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_DISCARD, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT)
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT)
            if (is_add)
            {
                SYS_GLOBAL_PORT_CHECK(pAction->data0);
                CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, pAction->data0, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nexthop_id));
            }
            pAction->data0 = nexthop_id;
             /*do not need break, continue to CTC_ACL_FIELD_ACTION_REDIRECT*/
        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT:
            /* data0: nexthopid */
             /*u2:g1,g3,g4 can overwrire, so process g3 and g4 as g1*/
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT)
            SYS_USW_SCL_REMOVE_ACTION_CHK(is_add, pe, pAction, CTC_SCL_FIELD_ACTION_TYPE_DISCARD, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT)

            pAction->data0 = is_add ? pAction->data0 : pe->nexthop_id;

            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pAction->data0, &nhinfo, 0));
            if (is_add)
            {
                if (pe->nexthop_id)
                {
                    switch(pe->u2_type)
                    {
                        case SYS_AD_UNION_G_3:
                            SetDsUserId(V, ecmpSelect_f, ds_userid, 0);
                            SetDsUserId(V, ecmpGroupId_f, ds_userid, 0);
                            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_3,DsUserId,2,ds_userid, 0);
                            break;
                        case SYS_AD_UNION_G_1:
                            SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                            SetDsUserId(V, dsFwdPtr_f, ds_userid, 0);
                            SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsUserId,2,ds_userid, 0);
                            break;
                        case SYS_AD_UNION_G_4:
                            SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                            SetDsUserId(V, adDestMap_f, ds_userid, 0);
                            SetDsUserId(V, adApsBridgeEn_f, ds_userid, 0);
                            SetDsUserId(V, adNextHopPtrHighBits_f, ds_userid, 0);
                            SetDsUserId(V, adNextHopPtrLowBits_f, ds_userid, 0);
                            SetDsUserId(V, adNextHopExt_f, ds_userid, 0);
                            SetDsUserId(V, adLengthAdjustType_f, ds_userid, 0);
                            SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_3,DsUserId,4,ds_userid, 0);
                            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_4,DsUserId,2,ds_userid, 0);
                            break;
                        default:
                            break;
                    }
                    CTC_BMP_UNSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT);
                    CTC_BMP_UNSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_REDIRECT);
                }
                _sys_usw_scl_unbind_nexthop(lchip, pe, pe->nexthop_id);

                have_dsfwd = (pe->u4_type == SYS_AD_UNION_G_1) || (pe->u4_type == SYS_AD_UNION_G_2)
                    || nhinfo.dsfwd_valid || pe->is_default || nhinfo.cloud_sec_en;
                if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_adApsBridgeEn_f))
                {
                    have_dsfwd |= nhinfo.aps_en;
                }

                if(!have_dsfwd && !nhinfo.ecmp_valid)
                {
                    have_dsfwd |= (CTC_E_NONE != _sys_usw_scl_bind_nexthop(lchip, pe, pAction->data0));
                }

	             /* for scl ecmp */
                if (nhinfo.ecmp_valid)
                {
                    SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_3);
                    SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                    SetDsUserId(V, ecmpSelect_f, ds_userid, 1);
                    SetDsUserId(V, ecmpGroupId_f, ds_userid, nhinfo.ecmp_group_id);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_3,DsUserId,2,ds_userid, is_add);
                }
                else if (have_dsfwd)
                {
                    SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_1);
                    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pAction->data0, &temp_data, 0));
                    SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 1);
                    SetDsUserId(V, dsFwdPtr_f, ds_userid, temp_data);
                    SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsUserId,2,ds_userid, is_add);
                }
                else
                {
                    uint8 adjust_len_idx = 0;
                    /*merge dsfwd*/
                    SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_3);
                    SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_4);
                    SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                    SetDsUserId(V, adDestMap_f, ds_userid, nhinfo.dest_map);
                    /**<  [TM] Enable aps bridge */
                    SetDsUserId(V, adApsBridgeEn_f, ds_userid, nhinfo.aps_en);
                    SetDsUserId(V, adNextHopExt_f, ds_userid, nhinfo.nexthop_ext);
                    if(0 != nhinfo.adjust_len)
                    {
                        sys_usw_lkup_adjust_len_index(lchip, nhinfo.adjust_len, &adjust_len_idx);
                        SetDsUserId(V, adLengthAdjustType_f, ds_userid, adjust_len_idx);
                    }
                    else
                    {
                        SetDsUserId(V, adLengthAdjustType_f, ds_userid, 0);
                    }
                    SetDsUserId(V, adNextHopPtrHighBits_f, ds_userid, ((nhinfo.dsnh_offset>>16)&0x3));
                    SetDsUserId(V, adNextHopPtrLowBits_f, ds_userid, (nhinfo.dsnh_offset&0xffff));
                    SetDsUserId(V, forwardOnStatus_f, ds_userid, 1);
                    SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_3,DsUserId,4,ds_userid, is_add);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_4,DsUserId,2,ds_userid, is_add);
                }

			    SetDsUserId(V, denyBridge_f, ds_userid, 1);

                if (0 == GetDsUserId(V, userVlanPtr_f, ds_userid))
                {
                    SetDsUserId(V, userVlanPtr_f, ds_userid, MCHIP_CAP(SYS_CAP_SCL_BY_PASS_VLAN_PTR));
                }
                pe->nexthop_id = pAction->data0;

            }
            else
            {
                if (nhinfo.ecmp_valid)
                {
                    SetDsUserId(V, ecmpSelect_f, ds_userid, 0);
                    SetDsUserId(V, ecmpGroupId_f, ds_userid, 0);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_3,DsUserId,2,ds_userid, is_add);
                }
                else if (pe->bind_nh)
                {
                    /*using merge dsfwd*/
                    SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                    SetDsUserId(V, adDestMap_f, ds_userid, 0);
                    SetDsUserId(V, adApsBridgeEn_f, ds_userid, 0);
                    SetDsUserId(V, adNextHopPtrHighBits_f, ds_userid, 0);
                    SetDsUserId(V, adNextHopPtrLowBits_f, ds_userid, 0);
                    SetDsUserId(V, adNextHopExt_f, ds_userid, 0);
                    SetDsUserId(V, adLengthAdjustType_f, ds_userid, 0);
                    SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                    SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_3,DsUserId,4,ds_userid, is_add);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_4,DsUserId,2,ds_userid, is_add);
                    _sys_usw_scl_unbind_nexthop(lchip, pe, pe->nexthop_id);
                }
                else
                {
                    SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                    SetDsUserId(V, dsFwdPtr_f, ds_userid, 0);
                    SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
                    SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsUserId,2,ds_userid, is_add);
                }
                SetDsUserId(V, denyBridge_f, ds_userid, 0);

                if (MCHIP_CAP(SYS_CAP_SCL_BY_PASS_VLAN_PTR) == GetDsUserId(V, userVlanPtr_f, ds_userid))
                {
                    SetDsUserId(V, userVlanPtr_f, ds_userid, 0);
                }
                pe->nexthop_id = 0;
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data1, 1);
                CTC_NOT_ZERO_CHECK(pAction->data0);

                type = SYS_QOS_POLICER_TYPE_SERVICE;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, pAction->data0,
                                                                      type,
                                                                      &policer_param));
                if (policer_param.is_bwp)
                {
                    SetDsUserId(V, policerPhbEn_f, ds_userid, 1);
                }
                SetDsUserId(V, policerLvlSel_f, ds_userid, policer_param.level);
                SetDsUserId(V, policerPtr_f, ds_userid, policer_param.policer_ptr);
                pe->policer_id = pAction->data0;
                pe->is_service_policer = pAction->data1;
            }
            else
            {
                SetDsUserId(V, policerPhbEn_f, ds_userid, 0);
                SetDsUserId(V, policerLvlSel_f, ds_userid, 0);
                SetDsUserId(V, policerPtr_f, ds_userid, 0);
                pe->policer_id = 0;
                pe->is_service_policer = 0;
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data1, 3);
                CTC_NOT_ZERO_CHECK(pAction->data0);

                type = SYS_QOS_POLICER_TYPE_FLOW;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, pAction->data0,
                                                                      type,
                                                                      &policer_param));

                SetDsUserId(V, policerPhbEn_f, ds_userid, 0);
                SetDsUserId(V, policerLvlSel_f, ds_userid, policer_param.level);
                SetDsUserId(V, policerPtr_f, ds_userid, policer_param.policer_ptr+pAction->data1);
                pe->policer_id = pAction->data0;
                pe->cos_idx = pAction->data1;
            }
            else
            {
                SetDsUserId(V, policerPhbEn_f, ds_userid, 0);
                SetDsUserId(V, policerLvlSel_f, ds_userid, 0);
                SetDsUserId(V, policerPtr_f, ds_userid, 0);
                pe->policer_id = 0;
                pe->cos_idx = 0;
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID:
            if( service_queue_mode != 1 && CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT))
            {
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_qos_bind_service_logic_srcport(lchip, pAction->data0,old_logic_port));
                    pe->service_id = pAction->data0;
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_srcport(lchip, pe->service_id,old_logic_port));
                    pe->service_id = 0;
                }
            }
            else if(service_queue_mode != 1)
            {
                pe->service_id = is_add ? pAction->data0 : 0;
            }
            else
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, (MCHIP_CAP(SYS_CAP_SCL_SERVICE_ID_NUM)-1));
                SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_0);
                if (is_add)
                {
                    SetDsUserId(V, serviceId_f, ds_userid, pAction->data0);
                    SetDsUserId(V, u5Type_f, ds_userid, 0);
                }
                else
                {
                    SetDsUserId(V, serviceId_f, ds_userid, 0);
                }
                SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_0,DsUserId,5,ds_userid, is_add);
            }

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_1);
            if (is_add)
            {
                uint32 stats_ptr = 0;
                sys_stats_statsid_t statsid;
                sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
                statsid.dir = CTC_INGRESS;
                statsid.stats_id_type = SYS_STATS_TYPE_SCL;
                statsid.stats_id = pAction->data0;
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr_with_check(lchip, &statsid, &stats_ptr));
                SetDsUserId(V, statsPtr_f, ds_userid, stats_ptr & 0xFFFF);
                SetDsUserId(V, u5Type_f, ds_userid, 1);

            }
            else
            {
                SetDsUserId(V, statsPtr_f, ds_userid, 0);
                SetDsUserId(V, u5Type_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_1,DsUserId,5,ds_userid, is_add);
		    pe->stats_id = pAction->data0;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_1);
            SetDsUserId(V, u3Type_f, ds_userid, 0);
            if (is_add)
            {
                SetDsUserId(V, isSpanPkt_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, isSpanPkt_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_1,DsUserId,3,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP:
            /* the param in the Ds to be checked */
            /* when user priority is not equal to zeros, but we just do not need it , this situaition is thought to be reasonable */
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_1);
			 if (is_add)
            {
                ctc_scl_qos_map_t* p_qos = (ctc_scl_qos_map_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_qos->priority, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->color, 0x3);
                CTC_MAX_VALUE_CHECK(p_qos->trust_dscp, 0x1);
                CTC_MAX_VALUE_CHECK(p_qos->dscp_domain, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->cos_domain, 0x7);
                CTC_MAX_VALUE_CHECK(p_qos->dscp, 0x3F);
                CTC_MAX_VALUE_CHECK(p_qos->trust_cos, 0x1);

                SetDsUserId(V, u3Type_f, ds_userid, 0);
                SetDsUserId(V, trustDscp_f, ds_userid, p_qos->trust_dscp);
                SetDsUserId(V, dscpPhbPtr_f, ds_userid, p_qos->dscp_domain);
                SetDsUserId(V, cosPhbPtr_f, ds_userid, p_qos->cos_domain);
                SetDsUserId(V, userColor_f, ds_userid, p_qos->color);
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID))
                {
                    SetDsUserId(V, cosPhbUseInnerValid_f, ds_userid, 1);
                    SetDsUserId(V, cosPhbUseInner_f, ds_userid, p_qos->trust_cos);
                }
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID))
                {
                    SetDsUserId(V, newDscpValid_f, ds_userid, 1);
                    SetDsUserId(V, newDscp_f, ds_userid, p_qos->dscp);
                }
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID))
                {
                    SetDsUserId(V, userPrioValid_f, ds_userid, 1);
                    SetDsUserId(V, userPrio_f, ds_userid, p_qos->priority);
                }
            }
            else
            {
                SetDsUserId(V, u3Type_f, ds_userid, 0);
                SetDsUserId(V, trustDscp_f, ds_userid, 0);
                SetDsUserId(V, dscpPhbPtr_f, ds_userid, 0);
                SetDsUserId(V, cosPhbPtr_f, ds_userid, 0);
                SetDsUserId(V, userColor_f, ds_userid, 0);
                SetDsUserId(V, cosPhbUseInnerValid_f, ds_userid, 0);
                SetDsUserId(V, cosPhbUseInner_f, ds_userid, 0);
                SetDsUserId(V, newDscpValid_f, ds_userid, 0);
                SetDsUserId(V, newDscp_f, ds_userid, 0);
                SetDsUserId(V, userPrioValid_f, ds_userid, 0);
                SetDsUserId(V, userPrio_f, ds_userid, 0);

            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_1,DsUserId,3,ds_userid, is_add);
		    break;
        case CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_1);
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x03);
                SetDsUserId(V, u3Type_f, ds_userid, 0);
                SetDsUserId(V, ptpIndex_f, ds_userid, pAction->data0);
            }
            else
            {
                SetDsUserId(V, u3Type_f, ds_userid, 0);
                SetDsUserId(V, ptpIndex_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_1,DsUserId,3,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SRC_CID:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);

			 if(GetDsUserId(V, categoryIdValid_f, ds_userid)
				&& !GetDsUserId(V, categoryType_f, ds_userid))
			{/*dst cid*/
				return CTC_E_PARAM_CONFLICT;
			}

			if(is_add)
            {
                SYS_USW_CID_CHECK(lchip,pAction->data0);
     			is_add = pAction->data0 ? 1:0;	/*disable cid*/
            }
			SetDsUserId(V, u3Type_f, ds_userid, 1);
            SetDsUserId(V, categoryIdValid_f, ds_userid, is_add?1:0);
            SetDsUserId(V, categoryType_f, ds_userid, is_add?1:0);
            SetDsUserId(V, categoryId_f, ds_userid, is_add?pAction->data0:0);

            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsUserId,3,ds_userid, is_add);
		    break;
        case CTC_SCL_FIELD_ACTION_TYPE_DST_CID:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
		    if(GetDsUserId(V, categoryIdValid_f, ds_userid)
				&& GetDsUserId(V, categoryType_f, ds_userid))
			{
				return CTC_E_PARAM_CONFLICT;
			}

			if(is_add)
            {
                SYS_USW_CID_CHECK(lchip,pAction->data0);
     			is_add = pAction->data0 ? 1:0;	/*disable cid*/
            }
			SetDsUserId(V, u3Type_f, ds_userid, 1);
            SetDsUserId(V, categoryIdValid_f, ds_userid, is_add?1:0);
            SetDsUserId(V, categoryType_f, ds_userid, 0);
            SetDsUserId(V, categoryId_f, ds_userid, is_add?pAction->data0:0);
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsUserId,3,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT:
            CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_igs_vlan_edit(lchip, pe, pAction, is_add));

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FID:
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_2);
            if(pe->vpws_oam_en == 1)
            {
                /* for vpws, can not config fid */
                return CTC_E_INVALID_CONFIG;
            }
            /*data0:fid*/
            SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x3FFF);
                SetDsUserId(V, fidSelect_f, ds_userid, 2);         /*2:vssid*/
                SetDsUserId(V, fid_f, ds_userid, pAction->data0);
            }
            else
            {
                SetDsUserId(V, fidSelect_f, ds_userid, 0);
                SetDsUserId(V, fid_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsUserId,2,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_VRFID:
            /*data0:vrfid*/
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID));

                SetDsUserId(V, fidSelect_f, ds_userid, 3);     /*3:vrfid*/
                SetDsUserId(V, fid_f, ds_userid, pAction->data0);
            }
            else
            {
                SetDsUserId(V, fidSelect_f, ds_userid, 0);
                SetDsUserId(V, fid_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsUserId,2,ds_userid, is_add);

            break;
        case SYS_SCL_FIELD_ACTION_TYPE_VN_ID:
            {
                uint16 fid = 0;
                SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_2);

                /*data0:fid*/
                SetDsUserId(V, dsFwdPtrValid_f, ds_userid, 0);
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_vlan_overlay_get_fid(lchip, pAction->data0, &fid));
                    CTC_MAX_VALUE_CHECK(fid, 0x3FFF);
                    pe->vn_id = pAction->data0;
                    SetDsUserId(V, fidSelect_f, ds_userid, 2);         /*2:vssid*/
                    SetDsUserId(V, fid_f, ds_userid, fid);
                }
                else
                {
                    pe->vn_id = 0;
                    SetDsUserId(V, fidSelect_f, ds_userid, 0);
                    SetDsUserId(V, fid_f, ds_userid, 0);
                }
                SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsUserId,2,ds_userid, is_add);

            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN:
            {
                uint32 bind_data[2] = { 0 };
                SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_1);

                SetDsUserId(V, bindingMacSa_f, ds_userid, 0);
                if (is_add)
                {
                    ctc_scl_bind_t* bind = pAction->ext_data;
                    hw_mac_addr_t mac = { 0 };

                    CTC_PTR_VALID_CHECK(pAction->ext_data);
                    switch (bind->type)
                    {
                        case CTC_SCL_BIND_TYPE_PORT:
                            SYS_GLOBAL_PORT_CHECK(bind->gport);
                            bind_data[1] = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(bind->gport);
                            bind_data[0] = 0;
                            break;
                        case CTC_SCL_BIND_TYPE_VLAN:
                            CTC_VLAN_RANGE_CHECK(bind->vlan_id);
                            bind_data[1] = bind->vlan_id << 4;
                            bind_data[0] = 1;
                            break;
                        case CTC_SCL_BIND_TYPE_IPV4SA:
                            bind_data[1] = (bind->ipv4_sa >> 28);
                            bind_data[0] = ((bind->ipv4_sa & 0xFFFFFFF) << 4) | 2;
                            break;
                        case CTC_SCL_BIND_TYPE_IPV4SA_VLAN:
                            CTC_VLAN_RANGE_CHECK(bind->vlan_id);
                            bind_data[1] = (bind->ipv4_sa >> 28) | (bind->vlan_id << 4);
                            bind_data[0] = ((bind->ipv4_sa & 0xFFFFFFF) << 4) | 3;
                            break;
                        case CTC_SCL_BIND_TYPE_MACSA:
                            SYS_USW_SET_HW_MAC(mac, (uint8*)bind->mac_sa);
                            bind_data[0] = mac[0];
                            bind_data[1] = mac[1];
                            SetDsUserId(V, bindingMacSa_f, ds_userid, 1);
                            break;
                        default:
                            return CTC_E_INVALID_PARAM;
                    }
                    SetDsUserId(A, bindingData_f, ds_userid, bind_data);
                    SetDsUserId(V, bindingEn_f, ds_userid, 1);
                }
                else
                {
                    SetDsUserId(V, bindingMacSa_f, ds_userid, 0);
                    SetDsUserId(A, bindingData_f, ds_userid, bind_data);
                    SetDsUserId(V, bindingEn_f, ds_userid, 0);
                }
                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsUserId,1,ds_userid, is_add);

                break;
            }
        case CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, bindingEn_f, ds_userid, 0);
            if (is_add)
            {
                SetDsUserId(V, userIdExceptionEn_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, userIdExceptionEn_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, bindingEn_f, ds_userid, 0);
            if (is_add)
            {
                SetDsUserId(V, denyBridge_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, denyBridge_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_OAM:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            if((pAction->data0 == 0 && pe->vpws_oam_en) || (pAction->data0 == 1 && !pe->vpws_oam_en && CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_OAM)))
            {
                return CTC_E_INVALID_CONFIG;
            }
            /* for vpws, check uservlanptr is set */
            if (pAction->data0 == 1 && (CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR) || CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FID)))
            {
                return CTC_E_INVALID_CONFIG;
            }
            if (is_add)
            {
                if (pAction->data0 == 1 && pAction->data1 > 0xFFF)
                {
                    return CTC_E_INVALID_PARAM;
                }
                SetDsUserId(V, vplsOamEn_f, ds_userid, 1);

                if (pAction->data0 == 1)
                {
                    SetDsUserId(V, userVlanPtr_f, ds_userid, pAction->data1);
                    pe->vpws_oam_en = 1;
                }
            }
            else
            {
                SetDsUserId(V, vplsOamEn_f, ds_userid, 0);
                if (pAction->data0 == 1)
                {
                    SetDsUserId(V, userVlanPtr_f, ds_userid, 0);
                }
                pe->vpws_oam_en = 0;
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);
            break;


        case CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, bindingEn_f, ds_userid, 0);
            if (is_add)
            {
                SetDsUserId(V, isLeaf_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, isLeaf_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
            SetDsUserId(V, bindingEn_f, ds_userid, 0);
            if (is_add)
            {
                SetDsUserId(V, logicPortSecurityEn_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, logicPortSecurityEn_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);

            break;
        /**< [TM] Mac limit discard or to cpu */
         case CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT :
            {
                uint8 is_to_cpu = (CTC_MACLIMIT_ACTION_TOCPU == pAction->data0);
                uint8 is_discard = (CTC_MACLIMIT_ACTION_DISCARD == pAction->data0) || is_to_cpu;

                if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_logicPortMacSecurityDiscard_f) && \
                    !DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_logicPortSecurityExceptionEn_f))
                {
                    return CTC_E_NOT_SUPPORT;
                }

                SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
                SetDsUserId(V, logicPortMacSecurityDiscard_f, ds_userid, (is_add? is_discard : 0));
                SetDsUserId(V, logicPortSecurityExceptionEn_f, ds_userid, (is_add? is_to_cpu : 0));
                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsUserId,1,ds_userid, is_add);
            }
            break;
        /**< [TM] Set router mac match */
         case CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH:
            if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_tcamHitRouterMac_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            SetDsUserId(V, tcamHitRouterMac_f, ds_userid, (is_add? 1: 0));
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_DOT1BR_PE:
            SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_3);
            if (is_add)
            {
                sys_scl_dot1br_t* p_dot1br = (sys_scl_dot1br_t*) pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                SetDsUserId(V, bindingEn_f, ds_userid, 1);
                SetDsUserId(V, localPhyPortValid_f, ds_userid, p_dot1br->lport_valid);
                SetDsUserId(V, localPhyPort_f, ds_userid, p_dot1br->lport);
                SetDsUserId(V, globalSrcPortValid_f, ds_userid, p_dot1br->src_gport_valid);
                SetDsUserId(V, globalSrcPort_f, ds_userid, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_dot1br->src_gport));
                SetDsUserId(V, discard_f, ds_userid, p_dot1br->src_discard);
                SetDsUserId(V, userIdExceptionEn_f, ds_userid, p_dot1br->exception_en);
                SetDsUserId(V, bypassAll_f, ds_userid, p_dot1br->bypass_all);
            }
            else
            {
                SetDsUserId(V, bindingEn_f, ds_userid, 0);
                SetDsUserId(V, localPhyPortValid_f, ds_userid, 0);
                SetDsUserId(V, localPhyPort_f, ds_userid, 0);
                SetDsUserId(V, globalSrcPortValid_f, ds_userid, 0);
                SetDsUserId(V, globalSrcPort_f, ds_userid, 0);
                SetDsUserId(V, discard_f, ds_userid, 0);
                SetDsUserId(V, userIdExceptionEn_f, ds_userid, 0);
                SetDsUserId(V, bypassAll_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_3,DsUserId,1,ds_userid, is_add);

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR:
            if (GetDsUserId(V, vplsOamEn_f, ds_userid) && pe->vpws_oam_en == 1)
            {
                return CTC_E_INVALID_CONFIG;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x1FFF);
                SetDsUserId(V, userVlanPtr_f, ds_userid, pAction->data0);
            }
            else
            {
                SetDsUserId(V, userVlanPtr_f, ds_userid, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN:
            if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_macSecurityDiscard_f))
            {
                return CTC_E_NOT_SUPPORT;
            }
            SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_1);
            SetDsUserId(V, staStatusCheckEn_f, ds_userid, 0);
            SetDsUserId(V, forwardOnStatus_f, ds_userid, 0);
            if(is_add)
            {
                SetDsUserId(V, macSecurityDiscard_f, ds_userid, 1);
            }
            else
            {
                SetDsUserId(V, macSecurityDiscard_f, ds_userid, 0);
            }
            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_1,DsUserId,4,ds_userid, is_add);
            break;
        /*just for get field action use, do nothing*/
        case SYS_SCL_FIELD_ACTION_TYPE_VPLS:
        case SYS_SCL_FIELD_ACTION_TYPE_VPWS:
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }
    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);
    /* the following situation is update action */
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_FID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_VRFID, 0);
    }
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_VRFID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FID, 0);
    }

    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_SRC_CID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_DST_CID, 0);
    }
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_DST_CID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_SRC_CID, 0);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_field_action_egs_cancel_all(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*  ds_egress  = NULL;
    ds_egress  = (void*)(&(pe->temp_entry->action));

    if (is_add)
    {
        sal_memset(ds_egress, 0, sizeof(pe->temp_entry->action));
        sal_memset((uint8*)(pe->action_bmp), 0, sizeof(pe->action_bmp));
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_build_field_action_egs(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    uint8  op         = 0;
    uint8  mo         = 0;
    uint8  do_nothing = 0;
    void*  ds_egress  = NULL;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);
    CTC_PTR_VALID_CHECK(pAction);

    ds_egress  = (void*)(&(pe->temp_entry->action));
    switch (pAction->type)
    {
      case CTC_SCL_FIELD_ACTION_TYPE_PENDING:
      case CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL:
             return _sys_usw_scl_build_field_action_egs_cancel_all(lchip, pAction, pe, is_add);
             break;
        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            if (is_add)
            {
                SetDsVlanXlate(V, discard_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, discard_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            /*data0: stats_id*/
            if (is_add)
            {
                uint32 stats_ptr = 0;
                sys_stats_statsid_t statsid;
                sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
                statsid.dir = CTC_EGRESS;
                statsid.stats_id_type = SYS_STATS_TYPE_SCL;
                statsid.stats_id = pAction->data0;
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr_with_check(lchip, &statsid, &stats_ptr));
                SetDsVlanXlate(V, statsPtr_f, ds_egress, stats_ptr & 0xFFFF);
            }
            else
            {
                SetDsVlanXlate(V, statsPtr_f, ds_egress, 0);
            }
            pe->stats_id = pAction->data0;
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT:
            if (is_add)
            {
                ctc_scl_vlan_edit_t* vlan_edit = (ctc_scl_vlan_edit_t*) pAction->ext_data;

                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_ERROR_RETURN(sys_usw_scl_check_vlan_edit(lchip, vlan_edit, &do_nothing));
                if(vlan_edit->vlan_domain)
                {
                    return CTC_E_NOT_SUPPORT;
                }
                if (do_nothing)
                {
                    return CTC_E_NONE;
                }
                /* refer to ctc_vlan_tag_sl_t, only CTC_VLAN_TAG_SL_NEW need to write hw table */
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl)
                {
                    SetDsVlanXlate(V, userCCfi_f, ds_egress, vlan_edit->ccfi_new);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl)
                {
                    SetDsVlanXlate(V, userCCos_f, ds_egress, vlan_edit->ccos_new);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
                {
                    SetDsVlanXlate(V, userCVlanId_f, ds_egress, vlan_edit->cvid_new);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scfi_sl)
                {
                    SetDsVlanXlate(V, userSCfi_f, ds_egress, vlan_edit->scfi_new);
                }
                /*SYSTEM MODIFIED by yangbs for SDK bug103073, 2017-01-15 */
                if (vlan_edit->stag_op == CTC_VLAN_TAG_OP_DEL)
                {
                    SetDsVlanXlate(V, userSCfi_f, ds_egress, 1);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scos_sl)
                {
                    SetDsVlanXlate(V, userSCos_f, ds_egress, vlan_edit->scos_new);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->svid_sl)
                {
                    SetDsVlanXlate(V, userSVlanId_f, ds_egress, vlan_edit->svid_new);
                }

                SetDsVlanXlate(V, sVlanIdAction_f, ds_egress, vlan_edit->svid_sl);
                SetDsVlanXlate(V, cVlanIdAction_f, ds_egress, vlan_edit->cvid_sl);
                SetDsVlanXlate(V, sCosAction_f, ds_egress, vlan_edit->scos_sl);
                SetDsVlanXlate(V, cCosAction_f, ds_egress, vlan_edit->ccos_sl);

                sys_usw_scl_vlan_tag_op_translate(lchip, vlan_edit->stag_op, &op, &mo);
                SetDsVlanXlate(V, sTagAction_f, ds_egress, op);
                SetDsVlanXlate(V, stagModifyMode_f, ds_egress, mo);

                sys_usw_scl_vlan_tag_op_translate(lchip, vlan_edit->ctag_op, &op, &mo);
                SetDsVlanXlate(V, cTagAction_f, ds_egress, op);
                SetDsVlanXlate(V, ctagModifyMode_f, ds_egress, mo);

                SetDsVlanXlate(V, sCfiAction_f, ds_egress, vlan_edit->scfi_sl);
                SetDsVlanXlate(V, cCfiAction_f, ds_egress, vlan_edit->ccfi_sl);
                SetDsVlanXlate(V, svlanXlateValid_f, ds_egress, vlan_edit->stag_op ? 1 : 0);
                SetDsVlanXlate(V, cvlanXlateValid_f, ds_egress, vlan_edit->ctag_op ? 1 : 0);
                /* SYSTEM MODIFIED for CNOS vlan mapping using port ethertype to update the packet tpid */
                #if 0
                SetDsVlanXlate(V, svlanTpidIndexEn_f, ds_egress, 1);
                SetDsVlanXlate(V, svlanTpidIndex_f, ds_egress, vlan_edit->tpid_index);
                #endif
            }
            else
            {
                SetDsVlanXlate(V, userSVlanId_f, ds_egress, 0);
                SetDsVlanXlate(V, userSCos_f, ds_egress, 0);
                SetDsVlanXlate(V, userSCfi_f, ds_egress, 0);
                SetDsVlanXlate(V, userCVlanId_f, ds_egress, 0);
                SetDsVlanXlate(V, userCCos_f, ds_egress, 0);
                SetDsVlanXlate(V, userCCfi_f, ds_egress, 0);
                SetDsVlanXlate(V, sVlanIdAction_f, ds_egress, 0);
                SetDsVlanXlate(V, cVlanIdAction_f, ds_egress, 0);
                SetDsVlanXlate(V, sCosAction_f, ds_egress, 0);
                SetDsVlanXlate(V, cCosAction_f, ds_egress, 0);

                SetDsVlanXlate(V, sTagAction_f, ds_egress, 0);
                SetDsVlanXlate(V, stagModifyMode_f, ds_egress, 0);

                SetDsVlanXlate(V, cTagAction_f, ds_egress, 0);
                SetDsVlanXlate(V, ctagModifyMode_f, ds_egress, 0);

                SetDsVlanXlate(V, sCfiAction_f, ds_egress, 0);
                SetDsVlanXlate(V, cCfiAction_f, ds_egress, 0);
                SetDsVlanXlate(V, svlanXlateValid_f, ds_egress, 0);
                SetDsVlanXlate(V, cvlanXlateValid_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST:
            if (is_add)
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 1);
                SetDsVlanXlate(V, discardUnknownUcast_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 0);
                SetDsVlanXlate(V, discardUnknownUcast_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST:
            if (is_add)
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 1);
                SetDsVlanXlate(V, discardUnknownMcast_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 0);
                SetDsVlanXlate(V, discardUnknownMcast_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST:
            if (is_add)
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 1);
                SetDsVlanXlate(V, discardBcast_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 0);
                SetDsVlanXlate(V, discardBcast_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST:
            if (is_add)
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 1);
                SetDsVlanXlate(V, discardKnownUcast_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 0);
                SetDsVlanXlate(V, discardKnownUcast_f, ds_egress, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST:
            if (is_add)
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 1);
                SetDsVlanXlate(V, discardKnownMcast_f, ds_egress, 1);
            }
            else
            {
                SetDsVlanXlate(V, vlanXlatePortIsolateEn_f, ds_egress, 0);
                SetDsVlanXlate(V, discardKnownMcast_f, ds_egress, 0);
            }
            break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;
    }
    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);
    return CTC_E_NONE;
}

#if 0
STATIC int32
_sys_usw_scl_build_field_action_flow_cancel_all(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void* ds_sclflow = NULL;

    ds_sclflow = (void*)(&pe->temp_entry->action);

    sal_memset(ds_sclflow, 0, sizeof(pe->temp_entry->action));
    sys_usw_scl_remove_acl_control_profile (lchip, 0, pe->acl_profile->profile_id);
    pe->acl_profile = NULL;


    return CTC_E_NONE;
}
#endif

STATIC int32
_sys_usw_scl_build_field_action_flow(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    uint32 temp_data = 0;
    sys_nh_info_dsnh_t nhinfo;
    ctc_acl_property_t* pAcl = NULL;
    uint8 type = 0;
    uint8 gint_en = 0;
    sys_qos_policer_param_t policer_param;
    void* ds_sclflow = NULL;
    ctc_scl_logic_port_t* logic_port = NULL;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    ds_sclflow = (void*)(&pe->temp_entry->action);
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));

    switch (pAction->type)
    {
        case SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON:
            SetDsSclFlow(V, dsType_f, ds_sclflow, is_add? SYS_SCL_AD_TYPE_FLOW: 0);
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID:
            CTC_ERROR_RETURN(DRV_IS_DUET2(lchip)? CTC_E_NOT_SUPPORT: CTC_E_NONE);
            temp_data = pAction->data0;
            CTC_MAX_VALUE_CHECK(temp_data, 0x7);
            SetDsSclFlow(V, hashBulkBitmapProfile_f, ds_sclflow, is_add? temp_data: 0);
            break;
         case CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW:
            if (is_add)
            {
                SetDsSclFlow(V, isSpanPkt_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, isSpanPkt_f, ds_sclflow, 0);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_1);
            if (is_add)
            {
                ctc_scl_qos_map_t* p_qos = (ctc_scl_qos_map_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_qos->priority, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->color, 0x3);
                CTC_MAX_VALUE_CHECK(p_qos->trust_dscp, 0x1);
                CTC_MAX_VALUE_CHECK(p_qos->dscp_domain, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->cos_domain, 0x7);
                CTC_MAX_VALUE_CHECK(p_qos->dscp, 0x3F);
                if(CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID))
                {
                    return CTC_E_NOT_SUPPORT;
                }
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 1);
                SetDsSclFlow(V, trustDscp_f, ds_sclflow, p_qos->trust_dscp);
                SetDsSclFlow(V, dscpPhbPtr_f, ds_sclflow, p_qos->dscp_domain);
                SetDsSclFlow(V, cosPhbPtr_f, ds_sclflow, p_qos->cos_domain);
                SetDsSclFlow(V, u2Type_f, ds_sclflow, 0);
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID))
                {
                    SetDsSclFlow(V, newDscpValid_f, ds_sclflow, 1);
                    SetDsSclFlow(V, newDscp_f, ds_sclflow, p_qos->dscp);
                }
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID))
                {
                    SetDsSclFlow(V, sclPrioValid_f, ds_sclflow, 1);
                    SetDsSclFlow(V, sclPrio_f, ds_sclflow, p_qos->priority);
                }
                SetDsSclFlow(V, sclColor_f, ds_sclflow, p_qos->color);
            }
            else
            {
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 0);
                SetDsSclFlow(V, trustDscp_f, ds_sclflow, 0);
                SetDsSclFlow(V, dscpPhbPtr_f, ds_sclflow, 0);
                SetDsSclFlow(V, cosPhbPtr_f, ds_sclflow, 0);
                SetDsSclFlow(V, u2Type_f, ds_sclflow, 0);
                SetDsSclFlow(V, newDscpValid_f, ds_sclflow, 0);
                SetDsSclFlow(V, newDscp_f, ds_sclflow, 0);
                SetDsSclFlow(V, sclPrioValid_f, ds_sclflow, 0);
                SetDsSclFlow(V, sclPrio_f, ds_sclflow, 0);
                SetDsSclFlow(V, sclColor_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsSclFlow,3,ds_sclflow, is_add);
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsSclFlow,2,ds_sclflow, is_add);
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN:
            SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_1);
            if (is_add)
            {
                ctc_acl_property_t acl_profile[CTC_MAX_ACL_LKUP_NUM];
                uint16 profile_id = 0;
                sal_memcpy(&acl_profile[0], pAction->ext_data, sizeof(ctc_acl_property_t));
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                pAcl = pAction->ext_data;
                CTC_MAX_VALUE_CHECK(pAcl->tcam_lkup_type, CTC_ACL_TCAM_LKUP_TYPE_MAX - 1);
                CTC_MAX_VALUE_CHECK(pAcl->acl_priority, CTC_MAX_ACL_LKUP_NUM - 1);

                if (pe->acl_profile)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
                    return CTC_E_EXIST;

                }

                profile_id = pe->acl_profile ? pe->acl_profile->profile_id : 0;
                pe->acl_profile = sys_usw_scl_add_acl_control_profile(lchip, 0, 1, acl_profile, &profile_id);
                if (!pe->acl_profile)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                    return CTC_E_NO_RESOURCE;
                }

                SetDsSclFlow(V, isAclFlowTcamOperation_f, ds_sclflow, 1);
                SetDsSclFlow(V, aclControlProfileId_f, ds_sclflow, pe->acl_profile->profile_id);
                SetDsSclFlow(V, aclControlProfileIdValid_f, ds_sclflow, 1);
                SetDsSclFlow(V, overwriteAclLabel0_f, ds_sclflow, pAcl->class_id);
            }
            else
            {
                if (pe->acl_profile)
                {
                    sys_usw_scl_remove_acl_control_profile (lchip, pe->acl_profile);
                    pe->acl_profile = NULL;
                    SetDsSclFlow(V, isAclFlowTcamOperation_f, ds_sclflow, 0);
                    SetDsSclFlow(V, aclControlProfileId_f, ds_sclflow, 0);
                    SetDsSclFlow(V, overwriteAclLabel0_f, ds_sclflow, 0);
                    SetDsSclFlow(V, aclControlProfileIdValid_f, ds_sclflow, 0);
                }
            }

            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_1,DsSclFlow,4,ds_sclflow, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN:
            SYS_CHECK_UNION_BITMAP(pe->u4_type, SYS_AD_UNION_G_2);
            if (is_add)
            {
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                pAcl = pAction->ext_data;
                CTC_MAX_VALUE_CHECK(pAcl->hash_lkup_type, CTC_ACL_HASH_LKUP_TYPE_MAX - 1);
                CTC_MAX_VALUE_CHECK(pAcl->hash_field_sel_id, 15);
                SetDsSclFlow(V, isAclFlowTcamOperation_f, ds_sclflow, 0);
                SetDsSclFlow(V, aclFlowHashType_f, ds_sclflow, pAcl->hash_lkup_type);
                SetDsSclFlow(V, aclFlowHashFieldSel_f, ds_sclflow, pAcl->hash_field_sel_id);
                SetDsSclFlow(V, overwriteAclFlowHash_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, isAclFlowTcamOperation_f, ds_sclflow, 0);
                SetDsSclFlow(V, aclFlowHashType_f, ds_sclflow, 0);
                SetDsSclFlow(V, aclFlowHashFieldSel_f, ds_sclflow, 0);
                SetDsSclFlow(V, overwriteAclFlowHash_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_2,DsSclFlow,4,ds_sclflow, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD:
            CTC_MAX_VALUE_CHECK(pAction->data0, 0x01);
            if (is_add)
            {
                SetDsSclFlow(V, aclQosUseOuterInfoValid_f, ds_sclflow, 1);
                SetDsSclFlow(V, aclQosUseOuterInfo_f, ds_sclflow, pAction->data0);
            }
            else
            {
                SetDsSclFlow(V, aclQosUseOuterInfoValid_f, ds_sclflow, 0);
                SetDsSclFlow(V, aclQosUseOuterInfo_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP:
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_1);
            if (GetDsSclFlow(V, forceSecondParser_f, ds_sclflow))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                uint32 cmd = 0;
                uint32 move_bit = 0;
                uint32 value = 0;
                ctc_scl_force_decap_t* p_decap = (ctc_scl_force_decap_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_decap->offset_base_type, CTC_SCL_OFFSET_BASE_TYPE_MAX - 1);
                /* the max extend length is 30 bytes */
                cmd = DRV_IOR(IpeSclFlowCtl_t, IpeSclFlowCtl_removeByteShift_f);
                DRV_IOCTL(lchip, 0, cmd, &move_bit);
                value = 1 << move_bit;
                CTC_MAX_VALUE_CHECK(p_decap->ext_offset, 15 * value);
                if(p_decap->ext_offset % 2)
                {
                    return CTC_E_INVALID_PARAM;
                }
                CTC_MAX_VALUE_CHECK(p_decap->payload_type, 7);
                SetDsSclFlow(V, innerPacketLookup_f, ds_sclflow, p_decap->force_decap_en);
                SetDsSclFlow(V, payloadOffsetStartPoint_f, ds_sclflow, p_decap->offset_base_type + 1);
                SetDsSclFlow(V, payloadOffset_f, ds_sclflow, (p_decap->ext_offset)/value);
                SetDsSclFlow(V, packetType_f, ds_sclflow, p_decap->payload_type);
                SetDsSclFlow(V, ttlUpdate_f, ds_sclflow, p_decap->use_outer_ttl? 0: 1);
                /* force decap do not need macth packet outer router mac or enable force route */
                SetDsSclFlow(V, isTunnel_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, innerPacketLookup_f, ds_sclflow, 0);
                SetDsSclFlow(V, payloadOffsetStartPoint_f, ds_sclflow, 0);
                SetDsSclFlow(V, payloadOffset_f, ds_sclflow, 0);
                SetDsSclFlow(V, packetType_f, ds_sclflow, 0);
                SetDsSclFlow(V, ttlUpdate_f, ds_sclflow, 0);
                SetDsSclFlow(V, isTunnel_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsSclFlow,2,ds_sclflow, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER:
            if (DRV_IS_DUET2(lchip))
            {
                return CTC_E_NOT_SUPPORT;
            }
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_1);
            if (GetDsSclFlow(V, isTunnel_f, ds_sclflow))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            {
                uint32 cmd = 0;
                uint32 move_bit = 0;
                uint32 value = 0;
                ctc_scl_snooping_parser_t* p_snooping_parser = (ctc_scl_snooping_parser_t*) (pAction->ext_data);
                if (is_add)
                {
                    CTC_PTR_VALID_CHECK(pAction->ext_data);
                    CTC_MAX_VALUE_CHECK(p_snooping_parser->start_offset_type, CTC_SCL_OFFSET_BASE_TYPE_MAX - 1);
                    /* the max extend length is 30 bytes */
                    cmd = DRV_IOR(IpeSclFlowCtl_t, IpeSclFlowCtl_removeByteShift_f);
                    DRV_IOCTL(lchip, 0, cmd, &move_bit);
                    value = 1 << move_bit;
                    CTC_MAX_VALUE_CHECK(p_snooping_parser->ext_offset, 15 * value);
                    if(p_snooping_parser->ext_offset % 2)
                    {
                        return CTC_E_INVALID_PARAM;
                    }
                    CTC_MAX_VALUE_CHECK(p_snooping_parser->payload_type, 7);
                }
                SetDsSclFlow(V, forceSecondParser_f, ds_sclflow, (is_add && p_snooping_parser->enable)? 1: 0);
                SetDsSclFlow(V, innerPacketLookup_f, ds_sclflow, 0);
                SetDsSclFlow(V, payloadOffsetStartPoint_f, ds_sclflow, is_add? p_snooping_parser->start_offset_type + 1: 0);
                SetDsSclFlow(V, payloadOffset_f, ds_sclflow, is_add? (p_snooping_parser->ext_offset)/value: 0);
                SetDsSclFlow(V, packetType_f, ds_sclflow, is_add? p_snooping_parser->payload_type: 0);
                SetDsSclFlow(V, forceUseInnerDoHash_f, ds_sclflow, (is_add && p_snooping_parser->use_inner_hash_en)? 1: 0);
                /* snooping parser do not need macth packet outer router mac or enable force route */
                SetDsSclFlow(V, isTunnel_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsSclFlow,2,ds_sclflow, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            if (!_sys_usw_scl_chk_logic_port_union(lchip, ds_sclflow, SYS_SCL_UNION_TYPE_STATS))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Stats invalid! \n");
                return CTC_E_INVALID_CONFIG;
            }
            /*data0: stats_id*/
            if (is_add)
            {
                uint32 stats_ptr = 0;
                sys_stats_statsid_t statsid;
                sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
                statsid.dir = CTC_INGRESS;
                statsid.stats_id_type = SYS_STATS_TYPE_SCL;
                statsid.stats_id = pAction->data0;
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr_with_check(lchip, &statsid, &stats_ptr));
                SetDsSclFlow(V, statsPtr_f, ds_sclflow, stats_ptr & 0xFFFF);
            }
            else
            {
                SetDsSclFlow(V, statsPtr_f, ds_sclflow, 0);
            }
            pe->stats_id = pAction->data0;
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT:
            sys_usw_global_get_gint_en(lchip, &gint_en);
            if (gint_en)
            {
                logic_port = (ctc_scl_logic_port_t*) pAction->ext_data;
                SetDsSclFlow(V, metadataType_f, ds_sclflow, is_add? (logic_port->logic_port >> 14): 0);
                SetDsSclFlow(V, metadata_f, ds_sclflow, is_add? (logic_port->logic_port & 0x3FFF): 0);
                SetDsSclFlow(V, forceRoute_f, ds_sclflow, is_add? 1: 0);
                SetDsSclFlow(V, denyRoute_f, ds_sclflow, is_add? 1: 0);
                break;
            }
            if (!_sys_usw_scl_chk_logic_port_union(lchip, ds_sclflow, SYS_SCL_UNION_TYPE_LOGICPORT))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Logic port invalid! \n");
                return CTC_E_INVALID_CONFIG;
            }
            if (DRV_IS_DUET2(lchip))
            {
                return CTC_E_NOT_SUPPORT;
            }
            {
                logic_port =  (ctc_scl_logic_port_t*) pAction->ext_data;
                if (is_add)
                {
                    CTC_PTR_VALID_CHECK(pAction->ext_data);
                    CTC_MAX_VALUE_CHECK(logic_port->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
                    CTC_MAX_VALUE_CHECK(logic_port->logic_port_type, 1);
                }
                SetDsSclFlow(V, statsPtr_f, ds_sclflow, is_add? logic_port->logic_port: 0);
                SetDsSclFlow(V, denyBridge_f, ds_sclflow, is_add? 1: 0);
                SetDsSclFlow(V, forceBridge_f, ds_sclflow, is_add? 1: 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE:
            if (!_sys_usw_scl_chk_logic_port_union(lchip, ds_sclflow, SYS_SCL_UNION_TYPE_FORCEBRIDGE))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Force bridge invalid! \n");
                return CTC_E_INVALID_CONFIG;
            }
            if (is_add)
            {
                SetDsSclFlow(V, forceBridge_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, forceBridge_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING:
            if (is_add)
            {
                SetDsSclFlow(V, forceLearning_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, forceLearning_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE:
            if (is_add)
            {
                SetDsSclFlow(V, forceRoute_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, forceRoute_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE:
            if (!_sys_usw_scl_chk_logic_port_union(lchip, ds_sclflow, SYS_SCL_UNION_TYPE_DENYBRIDGE))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Deny bridge invalid! \n");
                return CTC_E_INVALID_CONFIG;
            }
            if (is_add)
            {
                SetDsSclFlow(V, denyBridge_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, denyBridge_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING:
            if (is_add)
            {
                SetDsSclFlow(V, denyLearning_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, denyLearning_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE:
            if (is_add)
            {
                SetDsSclFlow(V, denyRoute_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, denyRoute_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            if (is_add)
            {
                SetDsSclFlow(V, discardPacket_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, discardPacket_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU:
            if (is_add)
            {
                SetDsSclFlow(V, exceptionToCpu_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, exceptionToCpu_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data1, 1);
                CTC_NOT_ZERO_CHECK(pAction->data0);

                type = SYS_QOS_POLICER_TYPE_SERVICE;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, pAction->data0,
                                                                      type,
                                                                      &policer_param));
                if (policer_param.is_bwp)
                {
                    SetDsSclFlow(V, policerPhbEn_f, ds_sclflow, 1);
                }
                SetDsSclFlow(V, policerLvlSel_f, ds_sclflow, policer_param.level);
                SetDsSclFlow(V, policerPtr_f, ds_sclflow, policer_param.policer_ptr);
                pe->policer_id = pAction->data0;
                pe->is_service_policer = pAction->data1;
            }
            else
            {
                SetDsSclFlow(V, policerPhbEn_f, ds_sclflow, 0);
                SetDsSclFlow(V, policerLvlSel_f, ds_sclflow, 0);
                SetDsSclFlow(V, policerPtr_f, ds_sclflow, 0);
                pe->policer_id = 0;
                pe->is_service_policer = 0;
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data1, 3);
                CTC_NOT_ZERO_CHECK(pAction->data0);

                type = SYS_QOS_POLICER_TYPE_FLOW;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, pAction->data0,
                                                                      type,
                                                                      &policer_param));

                SetDsSclFlow(V, policerPhbEn_f, ds_sclflow, 0);
                SetDsSclFlow(V, policerLvlSel_f, ds_sclflow, policer_param.level);
                SetDsSclFlow(V, policerPtr_f, ds_sclflow, policer_param.policer_ptr+pAction->data1);
                pe->policer_id = pAction->data0;
                pe->cos_idx = pAction->data1;
            }
            else
            {
                SetDsSclFlow(V, policerPhbEn_f, ds_sclflow, 0);
                SetDsSclFlow(V, policerLvlSel_f, ds_sclflow, 0);
                SetDsSclFlow(V, policerPtr_f, ds_sclflow, 0);
                pe->policer_id = 0;
                pe->cos_idx = 0;
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT:
            if (!_sys_usw_scl_chk_logic_port_union(lchip, ds_sclflow, SYS_SCL_UNION_TYPE_DENYBRIDGE))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Redirect invalid! \n");
                return CTC_E_INVALID_CONFIG;
            }
            /*data0: nexthopid*/
             /*u1:g1,g2 can overwrire, so process u1 just like u1 only has member g1*/

            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pAction->data0, &nhinfo, 0));

                /* for scl ecmp */
                if (nhinfo.ecmp_valid)
                {
                    SetDsSclFlow(V, ecmpEn_f, ds_sclflow, 1);
                    SetDsSclFlow(V, u1Type_f, ds_sclflow, SYS_SCL_FWD_TYPE_ECMP);
                    SetDsSclFlow(V, ecmpGroupId_f, ds_sclflow, nhinfo.ecmp_group_id);
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pAction->data0, &temp_data, 0));
                    SetDsSclFlow(V, dsFwdPtr_f, ds_sclflow, temp_data);
                    SetDsSclFlow(V, u1Type_f, ds_sclflow, SYS_SCL_FWD_TYPE_DSFWD);
                }

                SetDsSclFlow(V, denyBridge_f, ds_sclflow, 1);
                SetDsSclFlow(V, denyRoute_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, ecmpEn_f, ds_sclflow, 0);
                SetDsSclFlow(V, u1Type_f, ds_sclflow, 0);
                SetDsSclFlow(V, ecmpGroupId_f, ds_sclflow, 0);
                SetDsSclFlow(V, dsFwdPtr_f, ds_sclflow, 0);
                SetDsSclFlow(V, denyBridge_f, ds_sclflow, 0);
                SetDsSclFlow(V, denyRoute_f, ds_sclflow, 0);
            }
            pe->nexthop_id = is_add ? pAction->data0 : 0;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_METADATA:
            /* do not need consider pending status */
            temp_data = GetDsSclFlow(V, metadataType_f, ds_sclflow);
            if (CTC_METADATA_TYPE_METADATA != temp_data && CTC_METADATA_TYPE_INVALID != temp_data)
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x3FFF);
                SetDsSclFlow(V, metadata_f, ds_sclflow, pAction->data0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, CTC_METADATA_TYPE_METADATA);
            }
            else
            {
                SetDsSclFlow(V, metadata_f, ds_sclflow, 0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FID:
            /* do not need consider pending status */
            temp_data = GetDsSclFlow(V, metadataType_f, ds_sclflow);
            if (CTC_METADATA_TYPE_FID != temp_data && CTC_METADATA_TYPE_INVALID != temp_data)
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                /*data0: fid*/
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x3FFF);
                SetDsSclFlow(V, metadata_f, ds_sclflow, pAction->data0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, CTC_METADATA_TYPE_FID);
            }
            else
            {
                SetDsSclFlow(V, metadata_f, ds_sclflow, 0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_VRFID:
            /* do not need consider pending status */
            temp_data = GetDsSclFlow(V, metadataType_f, ds_sclflow);
            if (CTC_METADATA_TYPE_VRFID != temp_data && CTC_METADATA_TYPE_INVALID != temp_data)
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                /*data0: vrfid*/
                CTC_MAX_VALUE_CHECK(pAction->data0, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID));
                SetDsSclFlow(V, metadata_f, ds_sclflow, pAction->data0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, CTC_METADATA_TYPE_VRFID);
            }
            else
            {
                SetDsSclFlow(V, metadata_f, ds_sclflow, 0);
                SetDsSclFlow(V, metadataType_f, ds_sclflow, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SRC_CID:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
            if(GetDsSclFlow(V, categoryIdValid_f, ds_sclflow) && !GetDsSclFlow(V, categoryType_f, ds_sclflow))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                /*data0: src cid*/
                SYS_USW_CID_CHECK(lchip, pAction->data0);
                SetDsSclFlow(V, categoryIdValid_f, ds_sclflow, 1);
                SetDsSclFlow(V, categoryId_f, ds_sclflow, pAction->data0);
                SetDsSclFlow(V, categoryType_f, ds_sclflow, 1);
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, categoryIdValid_f, ds_sclflow, 0);
                SetDsSclFlow(V, categoryId_f, ds_sclflow, 0);
                SetDsSclFlow(V, categoryType_f, ds_sclflow, 0);
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsSclFlow,3,ds_sclflow, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DST_CID:
            SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_2);
            if(GetDsSclFlow(V, categoryIdValid_f, ds_sclflow) && GetDsSclFlow(V, categoryType_f, ds_sclflow))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            if (is_add)
            {
                /*data0: dst cid*/
                SYS_USW_CID_CHECK(lchip, pAction->data0);
                SetDsSclFlow(V, categoryIdValid_f, ds_sclflow, 1);
                SetDsSclFlow(V, categoryId_f, ds_sclflow, pAction->data0);
                SetDsSclFlow(V, categoryType_f, ds_sclflow, 0);
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 1);
            }
            else
            {
                SetDsSclFlow(V, categoryIdValid_f, ds_sclflow, 0);
                SetDsSclFlow(V, categoryId_f, ds_sclflow, 0);
                SetDsSclFlow(V, categoryType_f, ds_sclflow, 0);
                SetDsSclFlow(V, u3Type_f, ds_sclflow, 0);
            }
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsSclFlow,3,ds_sclflow, is_add);
            break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;


    }
    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);

    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_METADATA)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FID, 0);
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_VRFID, 0);
    }
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_FID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_METADATA, 0);
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_VRFID, 0);
    }
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_VRFID)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_METADATA, 0);
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FID, 0);
    }
    if(pAction->type == CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT)
    {
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE, 0);
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE, 0);
        SYS_USW_SCL_SET_BMP(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_STATS, 0);
    }
    return CTC_E_NONE;
}


int32
_sys_usw_scl_build_field_action_tunnel_wlan_decap(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{

    sys_scl_wlan_decap_t* p_wlan_decap;
    uint16 profile_id = 0;
    void* ds_tunnel = (void*)(&(pe->temp_entry->action));
    uint16 acl_label = 0;

    CTC_PTR_VALID_CHECK(pAction->ext_data);
    p_wlan_decap = (sys_scl_wlan_decap_t*) (pAction->ext_data);
    if (is_add)
    {
        if (p_wlan_decap->acl_lkup_num != 0)
        {
          if(pe->acl_profile)
          {
             SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
			return CTC_E_EXIST;

          }

          profile_id = pe->acl_profile?pe->acl_profile->profile_id:0;
          pe->acl_profile = sys_usw_scl_add_acl_control_profile( lchip,1,p_wlan_decap->acl_lkup_num,p_wlan_decap->acl_prop, &profile_id);
          if (!pe->acl_profile)
          {
                 SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

          }
          SetDsTunnelId(V, bindingEn_f, ds_tunnel, 0);
          SetDsTunnelId(V, apsSelectValid_f, ds_tunnel, 0);
          SetDsTunnelId(V, apsSelectProtectingPath_f, ds_tunnel, 1);
          acl_label = (p_wlan_decap->acl_prop[0].class_id<<4) | (pe->acl_profile->profile_id);
          SetDsTunnelId(V, apsSelectGroupId_f, ds_tunnel,acl_label);
       }
       SetDsTunnelId(V, useDefaultVlanTag_f, ds_tunnel, p_wlan_decap->user_default_vlan_tag_valid);
       SetDsTunnelId(V, userDefaultVlanValid_f, ds_tunnel, p_wlan_decap->user_default_vlan_valid);
       SetDsTunnelId(V, userDefaultVlanId_f, ds_tunnel, p_wlan_decap->user_default_vid);
       SetDsTunnelId(V, userDefaultCos_f, ds_tunnel, p_wlan_decap->user_default_cos);
       SetDsTunnelId(V, userDefaultCfi_f, ds_tunnel, p_wlan_decap->user_default_cfi);
       SetDsTunnelId(V, isAc2AcTunnel_f, ds_tunnel, p_wlan_decap->is_actoac_tunnel);

       SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, p_wlan_decap->ttl_check_en);
       SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, p_wlan_decap->payload_pktType);
       SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel, p_wlan_decap->payloadOffset);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapReassembleEn_f, ds_tunnel, p_wlan_decap->wlan_reassemble_en);
       SetDsTunnelId(V, uTunnelType_gCapwap_decryptKeyIndex_f, ds_tunnel, p_wlan_decap->decrypt_key_index);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapCipherEn_f, ds_tunnel, p_wlan_decap->wlan_cipher_en);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapTunnelId_f, ds_tunnel, p_wlan_decap->wlan_tunnel_id);
       SetDsTunnelId(V, isTunnel_f, ds_tunnel, p_wlan_decap->is_tunnel);
       SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, p_wlan_decap->inner_packet_lkup_en);
       SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, p_wlan_decap->ds_fwd_ptr);
   }
   /*else
   {
       sys_usw_scl_remove_acl_control_profile(lchip, pe->acl_profile);
       SetDsTunnelId(V, useDefaultVlanTag_f, ds_tunnel, 0);
       SetDsTunnelId(V, userDefaultVlanValid_f, ds_tunnel, 0);
       SetDsTunnelId(V, userDefaultVlanId_f, ds_tunnel, 0);
       SetDsTunnelId(V, userDefaultCos_f, ds_tunnel, 0);
       SetDsTunnelId(V, userDefaultCfi_f, ds_tunnel, 0);
       SetDsTunnelId(V, isAc2AcTunnel_f, ds_tunnel, 00);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapReassembleEn_f, ds_tunnel, 0);
       SetDsTunnelId(V, uTunnelType_gCapwap_decryptKeyIndex_f, ds_tunnel, 0);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapCipherEn_f, ds_tunnel, 0);
       SetDsTunnelId(V, uTunnelType_gCapwap_capwapTunnelId_f, ds_tunnel, 0);
       SetDsTunnelId(V, isTunnel_f, ds_tunnel, 0);
       SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, 0);
       SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, 0);
       SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, 0);
       SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel,0);
       SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, 0);
   }*/
	return CTC_E_NONE;
}

int32
_sys_usw_scl_build_field_action_tunnel_overlay(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    uint16 profile_id = 0;
    void*  ds_tunnel = NULL;
    sys_scl_overlay_t* p_overlay = NULL;

    CTC_PTR_VALID_CHECK(pAction->ext_data);
    ds_tunnel = (void*)(&pe->temp_entry->action);
    p_overlay = (sys_scl_overlay_t*)(pAction->ext_data);

    if (is_add)
    {
        CTC_PTR_VALID_CHECK(pAction->ext_data);
        p_overlay = (sys_scl_overlay_t*)pAction->ext_data;

        if (p_overlay->acl_lkup_num)
        {
            if (pe->acl_profile)
            {
                return CTC_E_EXIST;
            }

            pe->acl_profile = sys_usw_scl_add_acl_control_profile(lchip, 1, p_overlay->acl_lkup_num, p_overlay->acl_pro, &profile_id);
            if (!pe->acl_profile)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                return CTC_E_NO_RESOURCE;
            }
            SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x3);
            SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 1);
            SetDsTunnelId(V, aclControlProfileIdValid_f, ds_tunnel, 1);
            SetDsTunnelId(V, aclControlProfileId_f, ds_tunnel, pe->acl_profile->profile_id);
            SetDsTunnelId(V, aclLabel_f, ds_tunnel, p_overlay->acl_pro[0].class_id);
        }

        SetDsTunnelId(V, isTunnel_f, ds_tunnel, p_overlay->is_tunnel);
        SetDsTunnelId(V, ttlUpdate_f, ds_tunnel, !p_overlay->use_outer_ttl);
        SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, p_overlay->ttl_check_en);
        if (p_overlay->is_vxlan)
        {
            /* overlay only include vxlan and nvgre */
            SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_6);
            SetDsTunnelId(V, uTunnelType_gVxlan_vxlanInnerVtagCheckMode_f, ds_tunnel, p_overlay->inner_taged_chk_mode);
        }
        else
        {
            SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_3);
            SetDsTunnelId(V, uTunnelType_gNvgre_nvgreInnerVtagCheckMode_f, ds_tunnel, p_overlay->inner_taged_chk_mode);
            if (DRV_IS_DUET2(lchip))
            {
                SetDsTunnelId(V, uTunnelType_gGre_tunnelGreOptions_f, ds_tunnel, p_overlay->gre_option);
            }
        }
        SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel, p_overlay->payloadOffset);
        SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, p_overlay->inner_packet_lookup);
        SetDsTunnelId(V, logicPortType_f, ds_tunnel, p_overlay->logic_port_type);
        SetDsTunnelId(V, logicSrcPort_f, ds_tunnel, p_overlay->logic_src_port);
        SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, p_overlay->acl_qos_use_outer_info);
        SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, p_overlay->acl_qos_use_outer_info_valid);
        SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdr_f, ds_tunnel, p_overlay->acl_key_merge_inner_and_outer_hdr);
        SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdrValid_f, ds_tunnel, p_overlay->acl_key_merge_inner_and_outer_hdr_valid);
        SetDsTunnelId(V, ipfixAndMicroflowUseOuterInfo_f, ds_tunnel, p_overlay->ipfix_and_microflow_use_outer_info);
        SetDsTunnelId(V, ipfixAndMicroflowUseOuterInfoValid_f, ds_tunnel, p_overlay->ipfix_and_microflow_use_outer_info_valid);
        SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, p_overlay->classify_use_outer_info);
        SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, p_overlay->classify_use_outer_info_valid);
        if (p_overlay->router_mac_profile_en)
        {
            SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, p_overlay->router_mac_profile);
            pe->inner_rmac_index = p_overlay->router_mac_profile;
        }
        SetDsTunnelId(V, arpCtlEn_f, ds_tunnel, p_overlay->arp_ctl_en);
        SetDsTunnelId(V, arpExceptionType_f, ds_tunnel, p_overlay->arp_exception_type);

        SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, p_overlay->payload_pktType);
        SetDsTunnelId(V, outerVlanIsCVlan_f, ds_tunnel, p_overlay->vlan_domain);
        SetDsTunnelId(V, svlanTpidIndex_f, ds_tunnel, p_overlay->svlan_tpid);

    }
    #if 0
    else
    {
        CTC_PTR_VALID_CHECK(pAction->ext_data);
        p_overlay = (sys_scl_overlay_t*)pAction->ext_data;
        SetDsTunnelId(V, isTunnel_f, ds_tunnel, 0);
        SetDsTunnelId(V, ttlUpdate_f, ds_tunnel, 0);
        SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, 0);
        if (p_overlay->is_vxlan)
        {
            /* overlay only include vxlan and nvgre */
            SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_6);
            SetDsTunnelId(V, uTunnelType_gVxlan_vxlanInnerVtagCheckMode_f, ds_tunnel, p_overlay->inner_taged_chk_mode);
        }
        else
        {
            SYS_CHECK_UNION_BITMAP(pe->u5_type, SYS_AD_UNION_G_3);
            SetDsTunnelId(V, uTunnelType_gNvgre_nvgreInnerVtagCheckMode_f, ds_tunnel, p_overlay->inner_taged_chk_mode);
        }
        SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel, 0);
        SetDsTunnelId(V, uTunnelType_gGre_tunnelGreOptions_f, ds_tunnel, 0);
        SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, 0);
        SetDsTunnelId(V, logicPortType_f, ds_tunnel, 0);
        SetDsTunnelId(V, logicSrcPort_f, ds_tunnel, 0);
        SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, 0);
        SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, 0);
        SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdr_f, ds_tunnel, 0);
        SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdrValid_f, ds_tunnel, 0);
        SetDsTunnelId(V, ipfixAndMicroflowUseOuterInfo_f, ds_tunnel, 0);
        SetDsTunnelId(V, ipfixAndMicroflowUseOuterInfoValid_f, ds_tunnel, 0);
        SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, 0);
        SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, 0);
        SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, 0);
        SetDsTunnelId(V, arpCtlEn_f, ds_tunnel, 0);
        SetDsTunnelId(V, arpExceptionType_f, ds_tunnel, 0);

        SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, 0);
        SetDsTunnelId(V, outerVlanIsCVlan_f, ds_tunnel, 0);
        SetDsTunnelId(V, svlanTpidIndex_f, ds_tunnel, 0);
    }
    #endif
    if (p_overlay->is_vxlan)
    {
        SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_6,DsTunnelId,TunnelType,ds_tunnel, is_add);
    }
    else
    {
        SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_3,DsTunnelId,TunnelType,ds_tunnel, is_add);
    }

    return CTC_E_NONE;
}
int32
_sys_usw_scl_build_field_action_tunnel_trill(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*  ds_tunnel = NULL;
    sys_scl_trill_t* p_trill = NULL;
    uint32 bind_data[2] = { 0 };
    hw_mac_addr_t mac = { 0 };

    CTC_PTR_VALID_CHECK(pAction->ext_data);
    ds_tunnel = (void*)(&pe->temp_entry->action);
    p_trill = (sys_scl_trill_t*)(pAction->ext_data);

    if (is_add)
    {
        SetDsTunnelId(V, isTunnel_f, ds_tunnel, p_trill->is_tunnel);
        SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, p_trill->inner_packet_lookup);
        SetDsTunnelId(V, bindingEn_f, ds_tunnel, p_trill->bind_check_en);
        SetDsTunnelId(V, uTunnelType_gTrill_trillTtlCheckEn_f, ds_tunnel, 1);

        if (p_trill->bind_mac_sa)
        {
            SetDsTunnelId(V, u3Type_f, ds_tunnel, 0);
            SetDsTunnelId(V, bindingMacSa_f, ds_tunnel, p_trill->bind_mac_sa);
            SYS_USW_SET_HW_MAC(mac, (uint8*)p_trill->mac_sa);
            bind_data[0] = mac[0];
            bind_data[1] = mac[1];
            SetDsTunnelId(A, bindingData_f, ds_tunnel, bind_data);
        }
        else if (p_trill->multi_rpf_check)
        {
            SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x01);
            SetDsTunnelId(V, uTunnelType_gTrill_trillMultiRpfCheck_f, ds_tunnel, p_trill->multi_rpf_check);
            bind_data[0] = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_trill->src_gport) & 0x3FFF;
            bind_data[0] |= 0x4000;
            SetDsTunnelId(A, bindingData_f, ds_tunnel, bind_data);
            SetDsTunnelId(V, uTunnelType_gTrill_trillRpfCheckPortHighBit_f, ds_tunnel, (SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_trill->src_gport) >> 14) & 0x3);
        }

        if (p_trill->router_mac_profile_en)
        {
            SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, p_trill->router_mac_profile);
        }

        SetDsTunnelId(V, arpCtlEn_f, ds_tunnel, p_trill->arp_ctl_en);
        SetDsTunnelId(V, arpExceptionType_f, ds_tunnel, p_trill->arp_exception_type);
    }
    #if 0
    else
    {
        SetDsTunnelId(V, u3Type_f, ds_tunnel, 0);
        SetDsTunnelId(V, isTunnel_f, ds_tunnel, 0);
        SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, 0);
        SetDsTunnelId(V, bindingEn_f, ds_tunnel, 0);
        SetDsTunnelId(V, bindingMacSa_f, ds_tunnel, 0);
        SetDsTunnelId(A, bindingData_f, ds_tunnel, bind_data);
        SetDsTunnelId(V, uTunnelType_gTrill_trillMultiRpfCheck_f, ds_tunnel, 0);
        SetDsTunnelId(V, uTunnelType_gTrill_trillRpfCheckPortHighBit_f, ds_tunnel, 0);
        SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, 0);
        SetDsTunnelId(V, arpCtlEn_f, ds_tunnel, 0);
        SetDsTunnelId(V, arpExceptionType_f, ds_tunnel, 0);
        SetDsTunnelId(V, uTunnelType_gTrill_trillTtlCheckEn_f, ds_tunnel, 0);
    }
    #endif
    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_field_action_tunnel_cancel_all(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    void*  ds_tunnel = NULL;
    uint8 route_mac_index = 0;
    uint8 old_route_mac_index = 0;
    uint32 cmd = 0;
    uint32 mac[2] = {0};
    sys_scl_action_acl_t* p_out_acl_profile = NULL;

    ds_tunnel = (void*)(&pe->temp_entry->action);

    /* add cancel all is support and remove cancel all is not defined */
    if (pAction->type == CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL && !is_add)
    {
        return CTC_E_NONE;
    }

    if (is_add)
    {
        if (CTC_SCL_FIELD_ACTION_TYPE_PENDING == pAction->type)
        {
            if (pe->p_pe_backup)
            {
                return CTC_E_NONE;
            }
            MALLOC_ZERO(MEM_SCL_MODULE, pe->p_pe_backup, sizeof(sys_scl_sw_entry_t));
            if (!pe->p_pe_backup)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memcpy(pe->p_pe_backup, pe, sizeof(sys_scl_sw_entry_t));
        }

        /* must get this value from hw, default tunnel will free pe after pending 0, so next time this value will be zeros */
        route_mac_index = GetDsTunnelId(V, routerMacProfile_f, &pe->temp_entry->action);


        if (route_mac_index)
        {
            /* 0 is invalid */
            cmd = DRV_IOR(DsRouterMacInner_t, DsRouterMacInner_routerMac_f);
            DRV_IOCTL(lchip, route_mac_index, cmd, mac);

            SYS_USW_SET_USER_MAC(pe->old_inner_rmac, mac);

            sys_usw_l3if_unbinding_inner_router_mac(lchip, route_mac_index);
            pe->inner_rmac_index = 0;
        }
        sal_memset(ds_tunnel, 0, sizeof(pe->temp_entry->action));
        SetDsTunnelId(V, isHalf_f, ds_tunnel, pe->is_half);
        SetDsTunnelId(V, dsType_f, ds_tunnel, SYS_SCL_AD_TYPE_TUNNEL);

        if (pe->acl_profile)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove acl profile :%d\n", pe->acl_profile->profile_id);
            ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, pe->acl_profile, NULL);
            pe->acl_profile = NULL;
        }
        sal_memset((uint8*)(pe->action_bmp), 0, sizeof(pe->action_bmp));
    }
    else
    {
        if(!pe->p_pe_backup)
        {
            return CTC_E_NONE;
        }

        /* recover l3if inner router */
        cmd = DRV_IOR(DsTunnelId_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, pe->ad_index, cmd, &pe->temp_entry->action);
        old_route_mac_index = GetDsTunnelId(V, routerMacProfile_f, &pe->temp_entry->action);

        if (pe->inner_rmac_index)
        {
            /* 0 is invalid, free new */
            sys_usw_l3if_unbinding_inner_router_mac(lchip, pe->inner_rmac_index);
        }

        /* keep unbinding before binding */
        if(old_route_mac_index)
        {
            /* 0 is invalid and recover old */
            sys_usw_l3if_binding_inner_router_mac(lchip, &pe->old_inner_rmac, &old_route_mac_index);
            pe->inner_rmac_index = old_route_mac_index;
        }

        /*Recovery acl profile*/
        if (0xFFFF != pe->acl_profile_id)
        {
            uint32 cmd = 0;
            sys_scl_action_acl_t hw_acl;
            sal_memset(&hw_acl, 0, sizeof(hw_acl));

            /*Recovery vlan edit from hardware */
            cmd = DRV_IOR(DsTunnelAclControlProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->acl_profile_id, cmd, &hw_acl.acl_control_profile));
            hw_acl.profile_id = pe->acl_profile_id;
            hw_acl.is_scl = 0;

            /*Add hardward vlan profile*/
            ctc_spool_add(p_usw_scl_master[lchip]->acl_spool, &hw_acl, pe->acl_profile, &p_out_acl_profile);

            if (NULL == p_out_acl_profile)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                return CTC_E_NO_RESOURCE;
            }

            pe->acl_profile = p_out_acl_profile;

            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Recovery old vlan profile :%d\n", pe->acl_profile->profile_id);

        }
        else if (pe->acl_profile)
        {
            ctc_spool_remove(p_usw_scl_master[lchip]->acl_spool, pe->acl_profile, NULL);
            p_out_acl_profile = NULL;
            pe->acl_profile = p_out_acl_profile;
        }

        sal_memcpy(pe, pe->p_pe_backup, sizeof(sys_scl_sw_entry_t));
        mem_free(pe->p_pe_backup);
    }


    return CTC_E_NONE;
}

/**< [TM] dot1ae ingress process */
STATIC int32
_sys_usw_scl_build_field_action_tunnel_dot1ae(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    sys_scl_dot1ae_ingress_t* p_dot1ae_ingress;
    void* ds_tunnelid = (void*)(&(pe->temp_entry->action));

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    CTC_PTR_VALID_CHECK(pAction->ext_data);
    p_dot1ae_ingress = (sys_scl_dot1ae_ingress_t*)(pAction->ext_data);

    if (is_add)
    {
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode0En_f, ds_tunnelid, p_dot1ae_ingress->dot1ae_p2p_mode0_en);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode1En_f, ds_tunnelid, p_dot1ae_ingress->dot1ae_p2p_mode1_en);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode2En_f, ds_tunnelid, p_dot1ae_ingress->dot1ae_p2p_mode2_en);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeSaIndex_f, ds_tunnelid, p_dot1ae_ingress->dot1ae_sa_index);
        SetDsTunnelId(V, uTunnelType_gVxlan_encryptVxlanDisableAcl_f, ds_tunnelid, p_dot1ae_ingress->encrypt_disable_acl);
        SetDsTunnelId(V, uTunnelType_gVxlan_needDot1AeDecrypt_f, ds_tunnelid, p_dot1ae_ingress->need_dot1ae_decrypt);
        SetDsTunnelId(V, dsFwdPtr_f, ds_tunnelid, p_dot1ae_ingress->dsfwdptr);
    }
    #if 0
    else
    {
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode0En_f, ds_tunnelid, 0);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode1En_f, ds_tunnelid, 0);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode2En_f, ds_tunnelid, 0);
        SetDsTunnelId(V, uTunnelType_gVxlan_dot1AeSaIndex_f, ds_tunnelid, 0);
        SetDsTunnelId(V, uTunnelType_gVxlan_encryptVxlanDisableAcl_f, ds_tunnelid, 0);
        SetDsTunnelId(V, uTunnelType_gVxlan_needDot1AeDecrypt_f, ds_tunnelid, 0);
        SetDsTunnelId(V, dsFwdPtr_f, ds_tunnelid, 0);
    }
    #endif
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_field_action_tunnel(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    uint8  type      = 0;
    uint32 temp_data = 0 ;
    void*  ds_tunnel = NULL;
    sys_qos_policer_param_t policer_param;
    sys_nh_info_dsnh_t      nhinfo;
    sys_scl_iptunnel_t* p_ip_tunnel = (sys_scl_iptunnel_t*)(pAction->ext_data);

    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->temp_entry);

    ds_tunnel = (void*)(&pe->temp_entry->action);
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));

    switch (pAction->type)
    {
        case SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON:
            SetDsTunnelId(V, dsType_f, ds_tunnel, is_add? SYS_SCL_AD_TYPE_TUNNEL: 0);
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_PENDING:
        case CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL:
            return _sys_usw_scl_build_field_action_tunnel_cancel_all(lchip, pAction, pe, is_add);
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_IS_HALF:
            if (is_add)
            {
                SetDsTunnelId(V, isHalf_f, ds_tunnel, 1);
            }
            else
            {
                SetDsTunnelId(V, isHalf_f, ds_tunnel, 0);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SRC_CID:
            SetDsTunnelId(V, u4Type_f, ds_tunnel, 0);
            if(is_add)
            {
			    SYS_USW_CID_CHECK(lchip,pAction->data0);
                SetDsTunnelId(V, categoryIdValid_f, ds_tunnel, 1);
                SetDsTunnelId(V, categoryId_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, categoryIdValid_f, ds_tunnel, 0);
                SetDsTunnelId(V, categoryId_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP:
            if(is_add)
            {
                ctc_scl_qos_map_t* p_qos = (ctc_scl_qos_map_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_qos->priority, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->color, 0);
                CTC_MAX_VALUE_CHECK(p_qos->trust_dscp, 0x1);
                CTC_MAX_VALUE_CHECK(p_qos->dscp_domain, 0xF);
                CTC_MAX_VALUE_CHECK(p_qos->cos_domain, 0);
                CTC_MAX_VALUE_CHECK(p_qos->dscp, 0x3F);
                if (CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID) ||
                    CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID) ||
                    CTC_FLAG_ISSET(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Tunnel cannot support qos map priority or dscp directly\n");
                    return CTC_E_NOT_SUPPORT;
                }

                SetDsTunnelId(V, u4Type_f, ds_tunnel, 1);
                SetDsTunnelId(V, trustDscpValid_f, ds_tunnel, p_qos->trust_dscp?1:0);
                SetDsTunnelId(V, trustDscp_f, ds_tunnel, p_qos->trust_dscp);
                SetDsTunnelId(V, dscpPhbPtr_f, ds_tunnel, p_qos->dscp_domain);
            }
            /*else
            {
                SetDsTunnelId(V, u4Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, trustDscpValid_f, ds_tunnel, 0);
                SetDsTunnelId(V, trustDscp_f, ds_tunnel, 0);
                SetDsTunnelId(V, dscpPhbPtr_f, ds_tunnel, 0);
            }*/
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_AWARE_TUNNEL_INFO_EN:
            SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdrValid_f, ds_tunnel, is_add? 1: 0);
            SetDsTunnelId(V, aclKeyMergeInnerAndOuterHdr_f, ds_tunnel, (is_add && pAction->data0)? 1: 0);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN:
            SetDsTunnelId(V, bindingEn_f, ds_tunnel, 0);
            SetDsTunnelId(V, logicPortSecurityEn_f, ds_tunnel, is_add? 1: 0);
            break;

        /**< [TM] logic port mac limit */
         case CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT:
            {
                uint8 is_to_cpu = (CTC_MACLIMIT_ACTION_TOCPU == pAction->data0);
                uint8 is_discard = (CTC_MACLIMIT_ACTION_DISCARD == pAction->data0) || is_to_cpu;
                uint8 is_fwd = (CTC_MACLIMIT_ACTION_FWD == pAction->data0);

                if (!DRV_FLD_IS_EXISIT(DsTunnelId_t, DsTunnelId_logicPortMacSecurityDiscard_f) && \
                    !DRV_FLD_IS_EXISIT(DsTunnelId_t, DsTunnelId_logicPortSecurityExceptionEn_f))
                {
                    return CTC_E_NOT_SUPPORT;
                }

                SetDsTunnelId(V, logicPortMacSecurityDiscard_f, ds_tunnel, (is_add? is_discard : 0));
                SetDsTunnelId(V, logicPortSecurityExceptionEn_f, ds_tunnel, (is_add? is_to_cpu: 0));
                SetDsTunnelId(V, denyLearning_f, ds_tunnel, (is_add? is_fwd: 0));
            }
            break;

        /**< [TM] dot1ae ingress process */
        case SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS:
            _sys_usw_scl_build_field_action_tunnel_dot1ae(lchip, pAction, pe, is_add);
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_QOS_USE_OUTER_INFO:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x01);
                SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, 1);
                SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, 0);
                SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, 0);
            }*/
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_ROUTER_MAC:
            if(is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x3F);
                SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, routerMacProfile_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING:
            if (is_add)
            {
                SetDsTunnelId(V, denyLearning_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, denyLearning_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE:
            if (is_add)
            {
                SetDsTunnelId(V, denyBridge_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, denyBridge_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE:
            if (is_add)
            {
                SetDsTunnelId(V, routeDisable_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, routeDisable_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU:
            if (is_add)
            {
                SetDsTunnelId(V, bindingEn_f, ds_tunnel, 0);
                SetDsTunnelId(V, tunnelIdExceptionEn_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, bindingEn_f, ds_tunnel, 0);
                SetDsTunnelId(V, tunnelIdExceptionEn_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            /*data0: stats_id*/
            if (is_add)
            {
                uint32 stats_ptr = 0;
                sys_stats_statsid_t statsid;
                sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
                statsid.dir = CTC_INGRESS;
                statsid.stats_id_type = SYS_STATS_TYPE_TUNNEL;
                statsid.stats_id = pAction->data0;
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr_with_check(lchip, &statsid, &stats_ptr));
                SetDsTunnelId(V, statsPtr_f, ds_tunnel, stats_ptr & 0xFFFF);
                SetDsTunnelId(V, metadataType_f, ds_tunnel, CTC_METADATA_TYPE_INVALID);
            }
            /*else
            {
                SetDsTunnelId(V, statsPtr_f, ds_tunnel, 0);
                SetDsTunnelId(V, metadataType_f, ds_tunnel, 0);
            }*/
            pe->stats_id = pAction->data0;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data1, 1);
                CTC_NOT_ZERO_CHECK(pAction->data0);

                type = SYS_QOS_POLICER_TYPE_SERVICE;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, pAction->data0,
                                                                      type,
                                                                      &policer_param));
                if (policer_param.is_bwp)
                {
                    SetDsTunnelId(V, policerPhbEn_f, ds_tunnel, 1);
                }
                SetDsTunnelId(V, policerLvlSel_f, ds_tunnel, policer_param.level);
                SetDsTunnelId(V, policerPtr_f, ds_tunnel, policer_param.policer_ptr);
                pe->policer_id = pAction->data0;
                pe->is_service_policer = pAction->data1;
            }
            /*else
            {
                SetDsTunnelId(V, policerPhbEn_f, ds_tunnel, 0);
                SetDsTunnelId(V, policerLvlSel_f, ds_tunnel, 0);
                SetDsTunnelId(V, policerPtr_f, ds_tunnel, 0);
                pe->policer_id = 0;
                pe->is_service_policer = 0;
            }*/
            break;

/*        case CTC_OVERLAY_TUNNEL_FLAG_TTL_CHECK:
            if (is_add)
            {
                SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, 1);
            }
            else
            {
                SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, 0);
            }
            break;
*/
        case SYS_SCL_FIELD_ACTION_TYPE_IP_TUNNEL_DECAP:
            CTC_PTR_VALID_CHECK(pAction->ext_data);
            /* the param in the Ds to be checked */
            if(p_ip_tunnel->rpf_check_en)
            {
                uint8  loop = 0;
                uint32 step = DsTunnelIdRpf_array_1_rpfCheckIdEn_f - DsTunnelIdRpf_array_0_rpfCheckIdEn_f;
                pe->rpf_check_en = 1;
                if (is_add)
                {
                    for(loop=0; loop < CTC_IPUC_IP_TUNNEL_MAX_IF_NUM; loop++)
                    {
                        SetDsTunnelIdRpf(V, array_0_rpfCheckIdEn_f+loop*step, ds_tunnel, (p_ip_tunnel->l3_if[loop] ? 1 : 0));
                        SetDsTunnelIdRpf(V, array_0_rpfCheckId_f+loop*step, ds_tunnel, (p_ip_tunnel->l3_if[loop] & 0x3FFF));
                    }

                    SetDsTunnelIdRpf(V, tunnelMoreRpfIf_f, ds_tunnel, p_ip_tunnel->rpf_check_fail_tocpu);
                    SetDsTunnelIdRpf(V, rpfCheckEn_f, ds_tunnel, 1);
                    SetDsTunnelIdRpf(V, tunnelRpfCheckEn_f, ds_tunnel, 1);
                }
                /*else
                {
                    for(loop=0; loop < CTC_IPUC_IP_TUNNEL_MAX_IF_NUM; loop++)
                    {
                        SetDsTunnelIdRpf(V, array_0_rpfCheckIdEn_f+loop*step, ds_tunnel, 0);
                        SetDsTunnelIdRpf(V, array_0_rpfCheckId_f+loop*step, ds_tunnel, 0);
                    }
                    SetDsTunnelIdRpf(V, tunnelMoreRpfIf_f, ds_tunnel, 0);
                    SetDsTunnelIdRpf(V, rpfCheckEn_f, ds_tunnel, 0);
                    SetDsTunnelIdRpf(V, tunnelRpfCheckEn_f, ds_tunnel, 0);
                }*/
            }
            else
            {
                pe->rpf_check_en = 0;
                if(is_add)
                {
                    SetDsTunnelId(V, isTunnel_f, ds_tunnel, 1);
                    SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, p_ip_tunnel->inner_lkup_en);
                    SetDsTunnelId(V, innerIpPublicLookupEn_f, ds_tunnel, p_ip_tunnel->inner_pub_lkup_en);
                    SetDsTunnelId(V, ttlUpdate_f, ds_tunnel, (p_ip_tunnel->use_outer_ttl ? 0 : 1));
                    SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, p_ip_tunnel->ttl_check_en);
                    SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, p_ip_tunnel->payload_pktType);
                    SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel, p_ip_tunnel->payload_offset);
                    SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, p_ip_tunnel->classify_use_outer_info_valid);
                    SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, p_ip_tunnel->classify_use_outer_info);
                    SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, p_ip_tunnel->acl_use_outer_info_valid);
                    SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, p_ip_tunnel->acl_use_outer_info);

                    if(p_ip_tunnel->nexthop_id_valid)
                    {
                        uint32 fwd_offset = 0;
                        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_ip_tunnel->nexthop_id, &fwd_offset, 0));
                        SetDsTunnelId(V, u2Type_f, ds_tunnel, 0);
                        SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, fwd_offset);
                    }

                    if(p_ip_tunnel->meta_data_valid)
                    {
                        SetDsTunnelId(V, statsPtr_f, ds_tunnel, p_ip_tunnel->metadata);
                        SetDsTunnelId(V, metadataType_f, ds_tunnel, CTC_METADATA_TYPE_METADATA);
                    }

                    if(p_ip_tunnel->tunnel_type == SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_IPV6_IN_IP)
                    {
                        SetDsTunnelId(V, uTunnelType_gIpv6InIp_isatapCheckEn_f, ds_tunnel, p_ip_tunnel->isatap_check_en);
                    }
                    else if(p_ip_tunnel->tunnel_type == SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_GRE)
                    {
                        SetDsTunnelId(V, uTunnelType_gGre_tunnelGreOptions_f, ds_tunnel, p_ip_tunnel->gre_options);
                    }

                    if(p_ip_tunnel->use_outer_ttl)
                    {
                        SetDsTunnelId(V, u4Type_f, ds_tunnel, 1);
                        SetDsTunnelId(V, outerTtlCheck_f, ds_tunnel, 1);
                    }

                    if(p_ip_tunnel->rpf_check_request)
                    {
                        SetDsTunnelId(V, u4Type_f, ds_tunnel, 1);
                        SetDsTunnelId(V, tunnelRpfCheckRequest_f, ds_tunnel, 1);
                    }

                    if(p_ip_tunnel->acl_lkup_num != 0)
                    {
                        uint16 profile_id = 0;
                        uint16 acl_label = 0;
                        if(pe->acl_profile)
                        {
                            return CTC_E_EXIST;
                        }

                        profile_id = pe->acl_profile ? pe->acl_profile->profile_id:0;
                        pe->acl_profile = sys_usw_scl_add_acl_control_profile(lchip,1,p_ip_tunnel->acl_lkup_num,p_ip_tunnel->acl_prop, &profile_id);
                        if (!pe->acl_profile)
                        {
                            return CTC_E_NO_RESOURCE;
                        }

                        SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x03);
                        SetDsTunnelId(V, aclControlProfileIdValid_f, ds_tunnel, 1);
                        SetDsTunnelId(V, aclControlProfileId_f, ds_tunnel, pe->acl_profile->profile_id);
                        acl_label = p_ip_tunnel->acl_prop[0].class_id;
                        SetDsTunnelId(V, aclLabel_f, ds_tunnel, acl_label);
                        SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 1);
                   }
                }
                /*else
                {
                    SetDsTunnelId(V, isTunnel_f, ds_tunnel, 0);
                    SetDsTunnelId(V, innerPacketLookup_f, ds_tunnel, 0);
                    SetDsTunnelId(V, innerIpPublicLookupEn_f, ds_tunnel, 0);
                    SetDsTunnelId(V, ttlCheckEn_f, ds_tunnel, 0);
                    SetDsTunnelId(V, tunnelPacketType_f, ds_tunnel, 0);
                    SetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel, 0);
                    SetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel, 0);
                    SetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel, 0);
                    SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, 0);
                    SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, 0);
                    SetDsTunnelId(V, uTunnelType_gIpv6InIp_isatapCheckEn_f, ds_tunnel, 0);
                    SetDsTunnelId(V, uTunnelType_gGre_tunnelGreOptions_f, ds_tunnel, 0);
                    SetDsTunnelId(V, statsPtr_f, ds_tunnel, 0);
                    SetDsTunnelId(V, metadataType_f, ds_tunnel, 0);
                    SetDsTunnelId(V, u2Type_f, ds_tunnel, 0);
                    SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, 0);
                    SetDsTunnelId(V, u4Type_f, ds_tunnel, 0);
                    SetDsTunnelId(V, outerTtlCheck_f, ds_tunnel, 0);
                    SetDsTunnelId(V, tunnelRpfCheckRequest_f, ds_tunnel, 0);

                    sys_usw_scl_remove_acl_control_profile(lchip, pe->acl_profile);
                }*/
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT:
            if (is_add)
            {
                ctc_scl_logic_port_t* logic_port = (ctc_scl_logic_port_t*)pAction->ext_data;

                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(logic_port->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
                CTC_MAX_VALUE_CHECK(logic_port->logic_port_type, 1);
                SetDsTunnelId(V, logicSrcPort_f, ds_tunnel, logic_port->logic_port);
                SetDsTunnelId(V, logicPortType_f, ds_tunnel, logic_port->logic_port_type);
            }
            /*else
            {
                SetDsTunnelId(V, logicSrcPort_f, ds_tunnel, 0);
                SetDsTunnelId(V, logicPortType_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x01);
                SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, 1);
                SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel, 0);
                SetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN:
            if (is_add)
            {
                SetDsTunnelId(V, serviceAclQosEn_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, serviceAclQosEn_f, ds_tunnel, 0);
            }*/
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_WLAN_DECAP:
            CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_tunnel_wlan_decap(lchip, pAction, pe,is_add));
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_OVERLAY:
            CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_tunnel_overlay(lchip,pAction, pe, is_add));
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_VRFID:
            if (is_add)
            {
                SYS_VRFID_CHECK(pAction->data0);
                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x02);
                SetDsTunnelId(V, secondFidEn_f, ds_tunnel, 1);
                SetDsTunnelId(V, secondFid_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, secondFidEn_f, ds_tunnel, 0);
                SetDsTunnelId(V, secondFid_f, ds_tunnel, 0);
            }*/
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_FID:
            if (is_add)
            {
                SetDsTunnelId(V, u2Type_f, ds_tunnel, 0x03);
                SetDsTunnelId(V, fid_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, u2Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, fid_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT:
            /*data0: nexthopid*/
             /*u2:g2,g3 can overwrire, so process u2 just like u2 only has member g2*/
            pe->nexthop_id = pAction->data0;
            SYS_CHECK_UNION_BITMAP(pe->u2_type, SYS_AD_UNION_G_2);
            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pe->nexthop_id, &nhinfo, 0));

                /* for scl ecmp */
                if (nhinfo.ecmp_valid)
                {
                    SetDsTunnelId(V, ecmpGroupId_f, ds_tunnel, nhinfo.ecmp_group_id);

                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &temp_data, 0));
                    SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, temp_data);
                }

                SetDsTunnelId(V, denyBridge_f, ds_tunnel, 1);
                SetDsTunnelId(V, routeDisable_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, ecmpGroupId_f, ds_tunnel, 0);
                SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, 0);
                SetDsTunnelId(V, denyBridge_f, ds_tunnel, 0);
                SetDsTunnelId(V, routeDisable_f, ds_tunnel, 0);
            }*/
            SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsTunnelId,2,ds_tunnel, is_add);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_METADATA:
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(pAction->data0, 0x3FFF);
                SetDsTunnelId(V, metadataType_f, ds_tunnel, CTC_METADATA_TYPE_METADATA);
                SetDsTunnelId(V, statsPtr_f, ds_tunnel, pAction->data0);
            }
            /*else
            {
                SetDsTunnelId(V, metadataType_f, ds_tunnel, 0);
                SetDsTunnelId(V, statsPtr_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            if (is_add)
            {
                SetDsTunnelId(V, u2Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, 0xFFFF);
            }
            /*else
            {
                SetDsTunnelId(V, u2Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, dsFwdPtr_f, ds_tunnel, 0);
            }*/
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_RPF_CHECK:
            if (is_add)
            {
                SetDsTunnelId(V, rpfCheckEn_f, ds_tunnel, 1);
            }
            /*else
            {
                SetDsTunnelId(V, rpfCheckEn_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN:
            if(is_add)
            {
                uint32 temp_data = 0;
                ctc_acl_property_t* p_acl = (ctc_acl_property_t*)pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                CTC_MAX_VALUE_CHECK(p_acl->hash_lkup_type, CTC_ACL_HASH_LKUP_TYPE_MAX - 1);
                CTC_MAX_VALUE_CHECK(p_acl->hash_field_sel_id, 15);
                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x3);
                SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 0);
                temp_data = (1 << 6) | ((p_acl->hash_lkup_type & 0x3) << 4) | (p_acl->hash_field_sel_id & 0xF);
                SetDsTunnelId(V, aclLabel_f, ds_tunnel, temp_data);

            }
            /*else
            {
                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 0);
                SetDsTunnelId(V, aclLabel_f, ds_tunnel, 0);
            }*/
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN:
            if (is_add)
            {
                uint16 profile_id = 0;
                ctc_acl_property_t* pAcl = NULL;
                ctc_acl_property_t acl_profile[CTC_MAX_ACL_LKUP_NUM];
                sal_memset(&acl_profile[0], 0, sizeof(ctc_acl_property_t) * CTC_MAX_ACL_LKUP_NUM);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                sal_memcpy(&acl_profile[0], pAction->ext_data, sizeof(ctc_acl_property_t));
                pAcl = pAction->ext_data;
                CTC_MAX_VALUE_CHECK(pAcl->tcam_lkup_type, CTC_ACL_TCAM_LKUP_TYPE_MAX - 1);
                CTC_MAX_VALUE_CHECK(pAcl->acl_priority, CTC_MAX_ACL_LKUP_NUM - 1);

                if (pe->acl_profile)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
                    return CTC_E_EXIST;

                }

                profile_id = pe->acl_profile ? pe->acl_profile->profile_id : 0;
                pe->acl_profile = sys_usw_scl_add_acl_control_profile(lchip, 1, 1, acl_profile, &profile_id);
                if (!pe->acl_profile)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
                    return CTC_E_NO_RESOURCE;
                }

                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0x3);
                SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 1);
                SetDsTunnelId(V, aclControlProfileIdValid_f, ds_tunnel, 1);
                SetDsTunnelId(V, aclControlProfileId_f, ds_tunnel, pe->acl_profile->profile_id);
                SetDsTunnelId(V, aclLabel_f, ds_tunnel, pAcl->class_id);
            }
            /*else
            {
                if (pe->acl_profile)
                {
                    sys_usw_scl_remove_acl_control_profile (lchip, pe->acl_profile);
                    pe->acl_profile = NULL;
                }
                SetDsTunnelId(V, u3Type_f, ds_tunnel, 0);
                SetDsTunnelId(V, isAclFlowTcamOperation_f, ds_tunnel, 0);
                SetDsTunnelId(V, aclControlProfileIdValid_f, ds_tunnel, 0);
                SetDsTunnelId(V, aclControlProfileId_f, ds_tunnel, 0);
                SetDsTunnelId(V, aclLabel_f, ds_tunnel, 0);
            }*/
            break;

        /**< [TM] force mac da lookup to ipv4 lookup */
        case SYS_SCL_FIELD_ACTION_TYPE_IPV4_BASED_L2MC :
            if (!DRV_FLD_IS_EXISIT(DsTunnelId_t, DsTunnelId_forceIpv4Lookup_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            SetDsTunnelId(V, forceIpv4Lookup_f, ds_tunnel, (is_add? 1: 0));
            break;

        case SYS_SCL_FIELD_ACTION_TYPE_TRILL:
            CTC_ERROR_RETURN(_sys_usw_scl_build_field_action_tunnel_trill(lchip,pAction, pe, is_add));
            break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;


    }

    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_build_field_action_mpls(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(pe);

    switch (pAction->type)
    {
        case SYS_SCL_FIELD_ACTION_TYPE_MPLS:
            CTC_PTR_VALID_CHECK(pAction->ext_data);
            if (is_add)
            {
                sal_memcpy(&pe->temp_entry->action, pAction->ext_data, sizeof(DsMpls_m));
            }
            else
            {
                sal_memset(&pe->temp_entry->action, 0, sizeof(DsMpls_m));
            }
            break;
        default:
            return CTC_E_NOT_SUPPORT;
            break;
    }
    SYS_USW_SCL_SET_BMP(pe->action_bmp, pAction->type, is_add);
    return CTC_E_NONE;
}


#define ____get_action_____
#if 0
STATIC int32
_sys_usw_scl_get_field_action_igs_wlan_client(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe)
{

    ctc_scl_vlan_edit_t*      vlan_edit;
    sys_scl_wlan_client_t* p_wlan_client;
    uint32 temp_data = 0;
    void* ds_userid = (void*)(&(pe->temp_entry->action));

    CTC_PTR_VALID_CHECK(pAction->ext_data);
    p_wlan_client = (sys_scl_wlan_client_t*)(pAction->ext_data);

    vlan_edit = &p_wlan_client->vlan_edit;
    if (GetDsUserId(V, capwapVlanActionProfileId_f, ds_userid))
    {
        CTC_ERROR_RETURN(sys_usw_scl_unmapping_vlan_edit(lchip, vlan_edit, &(pe->vlan_edit->action_profile)));

        vlan_edit->svid_new = GetDsUserId(V, capwapUserSvlanId_f, ds_userid);
        vlan_edit->scos_new = GetDsUserId(V, capwapUserScos_f, ds_userid);
        vlan_edit->scfi_new = GetDsUserId(V, capwapUserScfi_f, ds_userid);
        /*recovery CTC_VLAN_TAG_OP_VALID*/
        if(GetDsUserId(V, svlanTagOperationValid_f, ds_userid) &&
            0 == vlan_edit->stag_op)
        {
            vlan_edit->stag_op = CTC_VLAN_TAG_OP_VALID;
        }
    }
    else if(GetDsUserId(V, capwapUserScfi_f, ds_userid) && !GetDsUserId(V, capwapVlanActionProfileId_f, ds_userid))
    {
        if (!pe->acl_profile)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

        }
        CTC_ERROR_RETURN(sys_usw_scl_get_acl_control_profile(lchip, 0, &(p_wlan_client->acl_lkup_num), \
                                                p_wlan_client->acl_prop, &(pe->acl_profile->acl_control_profile)));
        temp_data = GetDsUserId(V, capwapUserSvlanId_f, ds_userid);
        p_wlan_client->acl_prop[0].class_id = temp_data&0xFF;
    }

    if(GetDsUserId(V, fidSelect_f, ds_userid)  == 0x3)
    {
        p_wlan_client->vrfid = GetDsUserId(V, fid_f, ds_userid);
    }

    p_wlan_client->forward_on_status = GetDsUserId(V, forwardOnStatus_f, ds_userid);
    p_wlan_client->sta_roam_status = GetDsUserId(V, staRoamStatus_f, ds_userid);
    p_wlan_client->sta_status_chk_en = GetDsUserId(V, staStatusCheckEn_f, ds_userid);
    p_wlan_client->not_local_sta = GetDsUserId(V, notLocalSta_f, ds_userid);

    return CTC_E_NONE;
}
#endif

int32 _sys_usw_scl_get_field_action_igs(uint8 lchip, ctc_scl_field_action_t* p_action, sys_scl_sw_entry_t* pe)
{
    void* p_action_buffer = pe->temp_entry->action;
    uint8  service_queue_mode = 0;

    switch(p_action->type)
    {
        case SYS_SCL_FIELD_ACTION_TYPE_IS_HALF:
        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
        case CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN:
        case CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW:
        case CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU:
        case CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE:
        case CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF:
        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN:
        case SYS_SCL_FIELD_ACTION_TYPE_VPLS:
        case SYS_SCL_FIELD_ACTION_TYPE_VPWS:
             break;
        case CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT :
            {
                uint8 is_discard = GetDsUserId(V, logicPortMacSecurityDiscard_f, p_action_buffer);
                uint8 is_to_cpu = is_discard && GetDsUserId(V, logicPortSecurityExceptionEn_f, p_action_buffer);

                if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_logicPortMacSecurityDiscard_f) && \
                    !DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_logicPortSecurityExceptionEn_f))
                {
                    return CTC_E_NOT_SUPPORT;
                }

                p_action->data0= (is_to_cpu? CTC_MACLIMIT_ACTION_TOCPU: (is_discard? CTC_MACLIMIT_ACTION_DISCARD: 0));
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH :
            if (!DRV_FLD_IS_EXISIT(DsUserId_t, DsUserId_tcamHitRouterMac_f))
            {
                return CTC_E_NOT_SUPPORT;
            }

            p_action->data0 = GetDsUserId(V, tcamHitRouterMac_f, p_action_buffer);
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT:
            {
                ctc_scl_logic_port_t* p_logic_port = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_logic_port);

                p_logic_port->logic_port = GetDsUserId(V, logicSrcPort_f, p_action_buffer);
                p_logic_port->logic_port_type = GetDsUserId(V, logicPortType_f, p_action_buffer);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STP_ID:
            p_action->data0 = GetDsUserId(V, stpId_f, p_action_buffer);
            p_action->data1 = GetDsUserId(V, stpIdValid_f, p_action_buffer);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_APS:
            {
                ctc_scl_aps_t* aps = p_action->ext_data;
                CTC_PTR_VALID_CHECK(aps);
                aps->protected_vlan = GetDsUserId(V, userVlanPtr_f, p_action_buffer);
                aps->is_working_path = !GetDsUserId(V, apsSelectProtectingPath_f, p_action_buffer);
                aps->protected_vlan_valid = !!(aps->protected_vlan);
                aps->aps_select_group_id = GetDsUserId(V, apsSelectGroupId_f, p_action_buffer);
            }
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT:
            /*_sys_usw_scl_get_field_action_igs_wlan_client(lchip, p_action, pe);*/
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT:
            p_action->data0 = pe->nexthop_id;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT:
            {
                uint16 destmap = GetDsUserId(V, adDestMap_f, p_action_buffer);
                uint16 gchip = SYS_DECODE_DESTMAP_GCHIP(destmap);
                uint16 lport = destmap & 0x1FF;

                p_action->data0 = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID:
            p_action->data0 = pe->policer_id;
            p_action->data1 = pe->is_service_policer;

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER:
            p_action->data0 = pe->policer_id;
            p_action->data1 = pe->cos_idx;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID:
            CTC_ERROR_RETURN(sys_usw_queue_get_service_queue_mode(lchip, &service_queue_mode));
            p_action->data0 = (service_queue_mode == 1) ? GetDsUserId(V, serviceId_f, p_action_buffer) : pe->service_id;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            p_action->data0 = pe->stats_id;
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP:
            {
                ctc_scl_qos_map_t* p_qos = (ctc_scl_qos_map_t*) (p_action->ext_data);
                CTC_PTR_VALID_CHECK(p_qos);
                p_qos->trust_dscp = GetDsUserId(V, trustDscp_f, p_action_buffer);
                p_qos->dscp_domain = GetDsUserId(V, dscpPhbPtr_f, p_action_buffer);
                p_qos->cos_domain = GetDsUserId(V, cosPhbPtr_f, p_action_buffer);
                p_qos->color = GetDsUserId(V, userColor_f, p_action_buffer);

                if(GetDsUserId(V, cosPhbUseInnerValid_f, p_action_buffer))
                {
                    p_qos->trust_cos = GetDsUserId(V, cosPhbUseInner_f, p_action_buffer);
                    CTC_SET_FLAG(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID);
                }

                if(GetDsUserId(V, newDscpValid_f, p_action_buffer))
                {
                    p_qos->dscp = GetDsUserId(V, newDscp_f, p_action_buffer);
                    CTC_SET_FLAG(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID);
                }
                if(GetDsUserId(V, userPrioValid_f, p_action_buffer))
                {
                    p_qos->priority = GetDsUserId(V, userPrio_f, p_action_buffer);
                    CTC_SET_FLAG(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
                }
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID:
            p_action->data0 = GetDsUserId(V, ptpIndex_f, p_action_buffer);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SRC_CID:
        case CTC_SCL_FIELD_ACTION_TYPE_DST_CID:
            p_action->data0 = GetDsUserId(V, categoryId_f, p_action_buffer);
		    break;

        case CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT:
            {
                ctc_scl_vlan_edit_t*         vlan_edit = (ctc_scl_vlan_edit_t*)(p_action->ext_data);
                CTC_PTR_VALID_CHECK(p_action->ext_data);

                if(!pe->vlan_edit)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

                }

                CTC_ERROR_RETURN(sys_usw_scl_unmapping_vlan_edit(lchip, vlan_edit, &(pe->vlan_edit->action_profile)));

                /* refer to ctc_vlan_tag_sl_t, only CTC_VLAN_TAG_SL_NEW need to write hw table */
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl)
                {
                    vlan_edit->ccfi_new = GetDsUserId(V, userCcfi_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl)
                {
                    vlan_edit->ccos_new = GetDsUserId(V, userCcos_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
                {
                    vlan_edit->cvid_new = GetDsUserId(V, userCvlanId_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scfi_sl)
                {
                    vlan_edit->scfi_new = GetDsUserId(V, userScfi_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scos_sl)
                {
                    vlan_edit->scos_new = GetDsUserId(V, userScos_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->svid_sl)
                {
                    vlan_edit->svid_new = GetDsUserId(V, userSvlanId_f, p_action_buffer);
                }

                /*recovery CTC_VLAN_TAG_OP_VALID*/
                if(0 == GetDsUserId(V, bindingEn_f, p_action_buffer) &&
                    GetDsUserId(V, vlanActionProfileValid_f, p_action_buffer) &&
                    GetDsUserId(V, svlanTagOperationValid_f, p_action_buffer) &&
                    0 == vlan_edit->stag_op)
                {
                    vlan_edit->stag_op = CTC_VLAN_TAG_OP_VALID;
                }
                if(0 == GetDsUserId(V, bindingEn_f, p_action_buffer) &&
                    GetDsUserId(V, vlanActionProfileValid_f, p_action_buffer) &&
                    GetDsUserId(V, cvlanTagOperationValid_f, p_action_buffer) &&
                    GetDsUserId(V, u3Type_f, p_action_buffer) &&
                    0 == vlan_edit->ctag_op)
                {
                    vlan_edit->ctag_op = CTC_VLAN_TAG_OP_VALID;
                }
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FID:
        case CTC_SCL_FIELD_ACTION_TYPE_VRFID:
            p_action->data0 = GetDsUserId(V, fid_f, p_action_buffer);
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_VN_ID:
            p_action->data0 = pe->vn_id;
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN:
            {
                ctc_scl_bind_t* bind = p_action->ext_data;
                uint32 bind_data[2] = { 0 };

                CTC_PTR_VALID_CHECK(bind);
                GetDsUserId(A, bindingData_f, p_action_buffer, bind_data);
                if(GetDsUserId(V, bindingMacSa_f, p_action_buffer))
                {
                    bind->type = CTC_SCL_BIND_TYPE_MACSA;
                    SYS_USW_SET_USER_MAC(bind->mac_sa, bind_data);
                }
                else if(bind_data[0] == 0)
                {
                    bind->type = CTC_SCL_BIND_TYPE_PORT;
                    bind->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(bind_data[1]);
                }
                else if(bind_data[0] == 1)
                {
                    bind->type = CTC_SCL_BIND_TYPE_VLAN;
                    bind->vlan_id = (bind_data[1]>>4)&0xFFFF;
                }
                else if((bind_data[0]&0xF) == 2)
                {
                    bind->type = CTC_SCL_BIND_TYPE_IPV4SA;
                    bind->ipv4_sa = ((bind_data[1]&0xF) << 28) | (bind_data[0]>>4);
                }
                else if((bind_data[0]&0xF) == 3)
                {
                    bind->type = CTC_SCL_BIND_TYPE_IPV4SA_VLAN;
                    bind->vlan_id = bind_data[1]>>4;
                    bind->ipv4_sa = ((bind_data[1]&0xF) << 28) | (bind_data[0]>>4);
                }
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_OAM:
            {
                if(CTC_BMP_ISSET(pe->action_bmp, CTC_SCL_FIELD_ACTION_TYPE_FID))
                {
                    /* vpls */
                    p_action->data1 = GetDsUserId(V, fid_f, p_action_buffer);
                }
                else
                {
                    /* vpws */
                    p_action->data1 = GetDsUserId(V, userVlanPtr_f, p_action_buffer);
                }
            }
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_DOT1BR_PE:
            {
                sys_scl_dot1br_t* p_dot1br = (sys_scl_dot1br_t*) p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_action->ext_data);

                p_dot1br->lport_valid =GetDsUserId(V, localPhyPortValid_f, p_action_buffer);
                p_dot1br->lport = GetDsUserId(V, localPhyPort_f, p_action_buffer);
                p_dot1br->src_gport_valid = GetDsUserId(V, globalSrcPortValid_f, p_action_buffer);
                p_dot1br->src_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsUserId(V, globalSrcPort_f, p_action_buffer));
                p_dot1br->src_discard = GetDsUserId(V, discard_f, p_action_buffer);
                p_dot1br->exception_en = GetDsUserId(V, userIdExceptionEn_f, p_action_buffer);
                p_dot1br->bypass_all = GetDsUserId(V, bypassAll_f, p_action_buffer);
            }

            break;

        case CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR:
            p_action->data0 = GetDsUserId(V, userVlanPtr_f, p_action_buffer);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN:
            p_action->data0 = GetDsUserId(V, macSecurityDiscard_f, p_action_buffer);
            break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}


int32 _sys_usw_scl_get_field_action_egs(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe)
{
    void* ds_egress = pe->temp_entry->action;
    switch (pAction->type)
    {
        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            pAction->data0 = pe->stats_id;
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT:
            {
                uint8 op;
                uint8 mo;
                ctc_scl_vlan_edit_t* vlan_edit = (ctc_scl_vlan_edit_t*) pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);

                if(GetDsVlanXlate(V, svlanXlateValid_f, ds_egress))
                {
                    op = GetDsVlanXlate(V, sTagAction_f, ds_egress);
                    mo = GetDsVlanXlate(V, stagModifyMode_f, ds_egress);
                    sys_usw_scl_vlan_tag_op_untranslate(lchip, op, mo, &(vlan_edit->stag_op));

                }

                if(GetDsVlanXlate(V, cvlanXlateValid_f, ds_egress))
                {
                    op = GetDsVlanXlate(V, cTagAction_f, ds_egress);
                    mo = GetDsVlanXlate(V, ctagModifyMode_f, ds_egress);
                    sys_usw_scl_vlan_tag_op_untranslate(lchip, op, mo, &(vlan_edit->ctag_op));
                }

                vlan_edit->svid_sl = GetDsVlanXlate(V, sVlanIdAction_f, ds_egress);
                vlan_edit->cvid_sl = GetDsVlanXlate(V, cVlanIdAction_f, ds_egress);

                vlan_edit->scos_sl = GetDsVlanXlate(V, sCosAction_f, ds_egress);
                vlan_edit->ccos_sl = GetDsVlanXlate(V, cCosAction_f, ds_egress);
                vlan_edit->scfi_sl = GetDsVlanXlate(V, sCfiAction_f, ds_egress);
                vlan_edit->ccfi_sl = GetDsVlanXlate(V, cCfiAction_f, ds_egress);
                vlan_edit->tpid_index = GetDsVlanXlate(V, svlanTpidIndex_f, ds_egress);

                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccfi_sl)
                {
                    vlan_edit->ccfi_new = GetDsVlanXlate(V, userCCfi_f, ds_egress);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->ccos_sl)
                {
                    vlan_edit->ccos_new = GetDsVlanXlate(V, userCCos_f, ds_egress);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->cvid_sl)
                {
                    vlan_edit->cvid_new = GetDsVlanXlate(V, userCVlanId_f, ds_egress);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scfi_sl)
                {
                    vlan_edit->scfi_new = GetDsVlanXlate(V, userSCfi_f, ds_egress);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->scos_sl)
                {
                    vlan_edit->scos_new = GetDsVlanXlate(V, userSCos_f, ds_egress);
                }
                if (CTC_VLAN_TAG_SL_NEW == vlan_edit->svid_sl)
                {
                    vlan_edit->svid_new = GetDsVlanXlate(V, userSVlanId_f, ds_egress);
                }

            }
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_scl_get_field_action_flow(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe)
{
    void* ds_sclflow = NULL;
    uint8 gint_en = 0;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    ds_sclflow = (void*)(&pe->temp_entry->action);
    switch (pAction->type)
    {
        case CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW:
        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID:
            pAction->data0 = GetDsSclFlow(V, hashBulkBitmapProfile_f, ds_sclflow);
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT:
            {
                ctc_scl_logic_port_t* logic_port =  (ctc_scl_logic_port_t*) pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                sys_usw_global_get_gint_en(lchip, &gint_en);
                if (gint_en)
                {
                    logic_port->logic_port = GetDsSclFlow(V, metadata_f, ds_sclflow);
                    logic_port->logic_port |= GetDsSclFlow(V, metadataType_f, ds_sclflow) << 14;
                }
                else
                {
                    logic_port->logic_port = GetDsSclFlow(V, statsPtr_f, ds_sclflow);
                }
                logic_port->logic_port_type = 0;
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP:
            {
                uint32 cmd = 0;
                uint32 move_bit = 0;
                uint32 value = 0;
                ctc_scl_force_decap_t* p_decap = (ctc_scl_force_decap_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                cmd = DRV_IOR(IpeSclFlowCtl_t, IpeSclFlowCtl_removeByteShift_f);
                DRV_IOCTL(lchip, 0, cmd, &move_bit);
                value = 1 << move_bit;
                p_decap->force_decap_en = GetDsSclFlow(V, innerPacketLookup_f, ds_sclflow);
                p_decap->offset_base_type = GetDsSclFlow(V, payloadOffsetStartPoint_f, ds_sclflow) - 1;
                p_decap->ext_offset = GetDsSclFlow(V, payloadOffset_f, ds_sclflow) * value;
                p_decap->payload_type = GetDsSclFlow(V, packetType_f, ds_sclflow);
                p_decap->use_outer_ttl = GetDsSclFlow(V, ttlUpdate_f, ds_sclflow)? 0: 1;
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER:
            {
                uint32 cmd = 0;
                uint32 move_bit = 0;
                uint32 value = 0;
                ctc_scl_snooping_parser_t* p_snooping_parser = (ctc_scl_snooping_parser_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                cmd = DRV_IOR(IpeSclFlowCtl_t, IpeSclFlowCtl_removeByteShift_f);
                DRV_IOCTL(lchip, 0, cmd, &move_bit);
                value = 1 << move_bit;
                p_snooping_parser->enable = GetDsSclFlow(V, forceSecondParser_f, ds_sclflow);
                p_snooping_parser->start_offset_type = GetDsSclFlow(V, payloadOffsetStartPoint_f, ds_sclflow) - 1;
                p_snooping_parser->ext_offset = GetDsSclFlow(V, payloadOffset_f, ds_sclflow) * value;
                p_snooping_parser->payload_type = GetDsSclFlow(V, packetType_f, ds_sclflow);
                p_snooping_parser->use_inner_hash_en = GetDsSclFlow(V, forceUseInnerDoHash_f, ds_sclflow);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP:
            {
                ctc_scl_qos_map_t* p_qos = (ctc_scl_qos_map_t*) (pAction->ext_data);
                CTC_PTR_VALID_CHECK(pAction->ext_data);

                p_qos->trust_dscp = GetDsSclFlow(V, trustDscp_f, ds_sclflow);
                p_qos->dscp_domain = GetDsSclFlow(V, dscpPhbPtr_f, ds_sclflow);
                p_qos->cos_domain = GetDsSclFlow(V, cosPhbPtr_f, ds_sclflow);

                if(GetDsSclFlow(V, newDscpValid_f, ds_sclflow))
                {
                    p_qos->dscp = GetDsSclFlow(V, newDscp_f, ds_sclflow);
                    CTC_SET_FLAG(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID);
                }

                if (GetDsSclFlow(V, sclPrioValid_f, ds_sclflow))
                {
                    p_qos->priority = GetDsSclFlow(V, sclPrio_f, ds_sclflow);
                    CTC_SET_FLAG(p_qos->flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
                }

                p_qos->color = GetDsSclFlow(V, sclColor_f, ds_sclflow);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN:
            {
                ctc_acl_property_t acl_profile[CTC_MAX_ACL_LKUP_NUM];
                ctc_acl_property_t* p_acl = pAction->ext_data;
                uint8 lkup_num = 0;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                if (!pe->acl_profile)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
                    return CTC_E_NOT_EXIST;

                }

                sal_memset(&acl_profile, 0, sizeof(acl_profile));
                sys_usw_scl_get_acl_control_profile(lchip, 0, &lkup_num, acl_profile, &pe->acl_profile->acl_control_profile);
                sal_memcpy(p_acl, acl_profile, sizeof(ctc_acl_property_t));
                p_acl->class_id = GetDsSclFlow(V, overwriteAclLabel0_f, ds_sclflow);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN:
            {
                ctc_acl_property_t* p_acl = pAction->ext_data;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                p_acl = pAction->ext_data;

                p_acl->hash_lkup_type = GetDsSclFlow(V, aclFlowHashType_f, ds_sclflow);
                p_acl->hash_field_sel_id = GetDsSclFlow(V, aclFlowHashFieldSel_f, ds_sclflow);
                CTC_SET_FLAG(p_acl->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);
            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD:
            if(GetDsSclFlow(V, aclQosUseOuterInfoValid_f, ds_sclflow))
            {
                pAction->data0 = GetDsSclFlow(V, aclQosUseOuterInfo_f, ds_sclflow);
            }
            break;
        case CTC_SCL_FIELD_ACTION_TYPE_STATS:
            pAction->data0 = pe->stats_id;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_DISCARD:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU:
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID:
            pAction->data0 = pe->policer_id;
            pAction->data1 = pe->is_service_policer;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER:
            pAction->data0 = pe->policer_id;
            pAction->data1 = pe->cos_idx;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_REDIRECT:
            pAction->data0 = pe->nexthop_id;
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_METADATA:
            if (GetDsSclFlow(V, metadataType_f, ds_sclflow) == CTC_METADATA_TYPE_METADATA)
            {
                pAction->data0 = GetDsSclFlow(V, metadata_f, ds_sclflow);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_FID:
            if (GetDsSclFlow(V, metadataType_f, ds_sclflow) == CTC_METADATA_TYPE_FID)
            {
                pAction->data0 = GetDsSclFlow(V, metadata_f, ds_sclflow);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_VRFID:
            if (GetDsSclFlow(V, metadataType_f, ds_sclflow) == CTC_METADATA_TYPE_VRFID)
            {
                pAction->data0 = GetDsSclFlow(V, metadata_f, ds_sclflow);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

            }
            break;

        case CTC_SCL_FIELD_ACTION_TYPE_SRC_CID:
        case CTC_SCL_FIELD_ACTION_TYPE_DST_CID:
            pAction->data0 = GetDsSclFlow(V, categoryId_f, ds_sclflow);
		    break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;


    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_get_field_action_tunnel(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe)
{
    void* ds_tunnel = NULL;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);

    ds_tunnel = (void*)(&pe->temp_entry->action);

    switch (pAction->type)
    {
                /**< [TM] dot1ae ingress process */
        case SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS:
            {
                sys_scl_dot1ae_ingress_t* p_dot1ae_ingress = NULL;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                p_dot1ae_ingress = (sys_scl_dot1ae_ingress_t*)pAction->ext_data;

                p_dot1ae_ingress->dot1ae_p2p_mode0_en = GetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode0En_f, ds_tunnel);
                p_dot1ae_ingress->dot1ae_p2p_mode1_en = GetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode1En_f, ds_tunnel);
                p_dot1ae_ingress->dot1ae_p2p_mode2_en = GetDsTunnelId(V, uTunnelType_gVxlan_dot1AeP2pMode2En_f, ds_tunnel);
                p_dot1ae_ingress->dot1ae_sa_index = GetDsTunnelId(V, uTunnelType_gVxlan_dot1AeSaIndex_f, ds_tunnel);
                p_dot1ae_ingress->encrypt_disable_acl = GetDsTunnelId(V, uTunnelType_gVxlan_encryptVxlanDisableAcl_f, ds_tunnel);
                p_dot1ae_ingress->need_dot1ae_decrypt = GetDsTunnelId(V, uTunnelType_gVxlan_needDot1AeDecrypt_f, ds_tunnel);
                p_dot1ae_ingress->logic_src_port = GetDsTunnelId(V, logicSrcPort_f, ds_tunnel);
            }
            break;
        case SYS_SCL_FIELD_ACTION_TYPE_OVERLAY:
            {
                sys_scl_overlay_t* p_overlay = NULL;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                p_overlay = (sys_scl_overlay_t*)pAction->ext_data;

                p_overlay->is_tunnel = GetDsTunnelId(V, isTunnel_f, ds_tunnel);
                p_overlay->use_outer_ttl = !GetDsTunnelId(V, ttlUpdate_f, ds_tunnel);
                p_overlay->ttl_check_en = GetDsTunnelId(V, ttlCheckEn_f, ds_tunnel);
                if (p_overlay->is_vxlan)
                {
                    /* overlay only include vxlan and nvgre */
                    p_overlay->inner_taged_chk_mode = GetDsTunnelId(V, uTunnelType_gVxlan_vxlanInnerVtagCheckMode_f, ds_tunnel);
                }
                else
                {
                    p_overlay->inner_taged_chk_mode = GetDsTunnelId(V, uTunnelType_gNvgre_nvgreInnerVtagCheckMode_f, ds_tunnel);
                }
                p_overlay->payloadOffset = GetDsTunnelId(V, tunnelPayloadOffset_f, ds_tunnel);
                p_overlay->inner_packet_lookup = GetDsTunnelId(V, innerPacketLookup_f, ds_tunnel);
                p_overlay->logic_port_type = GetDsTunnelId(V, logicPortType_f, ds_tunnel);
                p_overlay->logic_src_port = GetDsTunnelId(V, logicSrcPort_f, ds_tunnel);
                p_overlay->acl_qos_use_outer_info = GetDsTunnelId(V, aclQosUseOuterInfo_f, ds_tunnel);
                p_overlay->acl_qos_use_outer_info_valid = GetDsTunnelId(V, aclQosUseOuterInfoValid_f, ds_tunnel);
                p_overlay->ipfix_and_microflow_use_outer_info = GetDsTunnelId(V, ipfixAndMicroflowUseOuterInfo_f, ds_tunnel);
                p_overlay->ipfix_and_microflow_use_outer_info_valid = GetDsTunnelId(V, ipfixAndMicroflowUseOuterInfoValid_f, ds_tunnel);
                p_overlay->classify_use_outer_info = GetDsTunnelId(V, phbUseOuterInfo_f, ds_tunnel);
                p_overlay->classify_use_outer_info_valid = GetDsTunnelId(V, phbUseOuterInfoValid_f, ds_tunnel);
                p_overlay->router_mac_profile = GetDsTunnelId(V, routerMacProfile_f, ds_tunnel);
                p_overlay->arp_ctl_en = GetDsTunnelId(V, arpCtlEn_f, ds_tunnel);
                p_overlay->arp_exception_type = GetDsTunnelId(V, arpExceptionType_f, ds_tunnel);
                p_overlay->payload_pktType = GetDsTunnelId(V, tunnelPacketType_f, ds_tunnel);
                p_overlay->vlan_domain = GetDsTunnelId(V, outerVlanIsCVlan_f, ds_tunnel);
                p_overlay->svlan_tpid = GetDsTunnelId(V, svlanTpidIndex_f, ds_tunnel);
                break;
            }
        case SYS_SCL_FIELD_ACTION_TYPE_TRILL:
            {
                sys_scl_trill_t* p_trill = NULL;
                CTC_PTR_VALID_CHECK(pAction->ext_data);
                p_trill = (sys_scl_trill_t*)pAction->ext_data;

                p_trill->is_tunnel = GetDsTunnelId(V, isTunnel_f, ds_tunnel);
                p_trill->inner_packet_lookup = GetDsTunnelId(V, innerPacketLookup_f, ds_tunnel);
                p_trill->router_mac_profile = GetDsTunnelId(V, routerMacProfile_f, ds_tunnel);
                p_trill->bind_check_en = GetDsTunnelId(V, bindingEn_f, ds_tunnel);
                p_trill->multi_rpf_check = GetDsTunnelId(V, uTunnelType_gTrill_trillMultiRpfCheck_f, ds_tunnel);

                break;
            }
        default:

            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_get_field_action_mpls(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe)
{
    void* ds_mpls = NULL;
    CTC_PTR_VALID_CHECK(pAction);
    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(&pe->temp_entry->action);
    ds_mpls = (void*)(pe->temp_entry->action);

    switch (pAction->type)
    {
        case SYS_SCL_FIELD_ACTION_TYPE_MPLS:
            sal_memcpy(pAction->ext_data, ds_mpls, sizeof(DsMpls_m));
            break;
        default:
            return CTC_E_NOT_SUPPORT;
            break;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_scl_init_build_key_and_action_cb(uint8 lchip)
{
    /*build key*/
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_VLAN             ]    = NULL;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_MAC              ]    = _sys_usw_scl_build_tcam_key_mac;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_IPV4             ]    = _sys_usw_scl_build_tcam_key_ipv4_double;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_IPV4_SINGLE      ]    = _sys_usw_scl_build_tcam_key_ipv4_single;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_IPV6             ]    = _sys_usw_scl_build_tcam_key_ipv6_double;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_IPV6_SINGLE      ]    = _sys_usw_scl_build_tcam_key_ipv6_single;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_UDF      ]    = _sys_usw_scl_build_tcam_key_udf;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_TCAM_UDF_EXT      ]    = _sys_usw_scl_build_tcam_key_udf_ext;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT             ]    = _sys_usw_scl_build_hash_key_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_CVLAN       ]    = _sys_usw_scl_build_hash_key_cvlan_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_SVLAN       ]    = _sys_usw_scl_build_hash_key_svlan_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_2VLAN       ]    = _sys_usw_scl_build_hash_key_2vlan_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP  ]    = _sys_usw_scl_build_hash_key_2vlan_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_CVLAN_COS   ]    = _sys_usw_scl_build_hash_key_cvlan_cos_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_SVLAN_COS   ]    = _sys_usw_scl_build_hash_key_svlan_cos_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_MAC              ]    = _sys_usw_scl_build_hash_key_mac;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_MAC         ]    = _sys_usw_scl_build_hash_key_mac_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_IPV4             ]    = _sys_usw_scl_build_hash_key_ipv4_sa;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_SVLAN             ]    = _sys_usw_scl_build_hash_key_svlan;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_SVLAN_MAC             ]    = _sys_usw_scl_build_hash_key_svlan_mac;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_IPV4        ]    = _sys_usw_scl_build_hash_key_ipv4_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_IPV6             ]    = _sys_usw_scl_build_hash_key_ipv6_sa;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_IPV6        ]    = _sys_usw_scl_build_hash_key_ipv6_port;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_L2               ]    = _sys_usw_scl_build_hash_key_scl_flow_l2;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV4      ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA   ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_da;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE  ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_gre;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF  ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_rpf;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA  ]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_da;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV6  ]    = _sys_usw_scl_build_hash_key_tunnel_ipv6;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP  ]    = NULL;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY  ]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_grekey;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_uc_nvgre_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_uc_nvgre_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_mc_nvgre_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_V4_MODE1   ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_nvgre_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_uc_nvgre_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_uc_nvgre_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_mc_nvgre_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_mc_nvgre_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_uc_vxlan_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_uc_vxlan_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_mc_vxlan_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_V4_MODE1   ]    = _sys_usw_scl_build_hash_key_tunnel_ipv4_vxlan_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_uc_vxlan_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_uc_vxlan_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_mc_vxlan_mode0;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1]    = _sys_usw_scl_build_hash_key_tunnel_ipv6_mc_vxlan_mode1;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_IPV4      ]      = _sys_usw_scl_build_hash_key_tunnel_ipv4_capwap;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_IPV6      ]      = _sys_usw_scl_build_hash_key_tunnel_ipv6_capwap;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_RMAC      ]      = _sys_usw_scl_build_hash_key_tunnel_capwap_rmac;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_RMAC_RID  ]      = _sys_usw_scl_build_hash_key_tunnel_capwap_rmac_rid;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_STA_STATUS]      = _sys_usw_scl_build_hash_key_capwap_sta_status;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS]   = _sys_usw_scl_build_hash_key_capwap_mc_sta_status;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD ]   = NULL;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD]   = NULL;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_ECID]                 = _sys_usw_scl_build_hash_key_ecid;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_ING_ECID]             = _sys_usw_scl_build_hash_key_ing_ecid;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_CROSS]           = _sys_usw_scl_build_hash_key_port_cross;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_PORT_VLAN_CROSS]      = _sys_usw_scl_build_hash_key_port_vlan_cross;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TUNNEL_MPLS]          = _sys_usw_scl_build_tcam_key_mpls;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TRILL_UC]             = _sys_usw_scl_build_hash_key_trill_uc;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TRILL_UC_RPF]         = _sys_usw_scl_build_hash_key_trill_uc_rpf;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TRILL_MC]             = _sys_usw_scl_build_hash_key_trill_mc;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TRILL_MC_RPF]         = _sys_usw_scl_build_hash_key_trill_mc_rpf;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_HASH_TRILL_MC_ADJ]         = _sys_usw_scl_build_hash_key_trill_mc_adj;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_PORT_DEFAULT_INGRESS  ]    = NULL;
    p_usw_scl_master[lchip]->build_key_func[SYS_SCL_KEY_PORT_DEFAULT_EGRESS   ]    = NULL;

   /* init entry size */
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_VLAN             ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_MAC              ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_IPV4             ] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_IPV4_SINGLE      ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_IPV6             ] = SCL_KEY_USW_SIZE_640;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_TCAM_IPV6_SINGLE      ] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT             ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_CVLAN       ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_SVLAN       ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_2VLAN       ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP  ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_CVLAN_COS   ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_SVLAN_COS   ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_MAC              ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_MAC         ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_IPV4             ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_SVLAN            ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_SVLAN_MAC        ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_IPV4        ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_IPV6             ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_IPV6        ] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_L2               ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4      ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA   ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE  ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF  ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_V4_MODE1   ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_V4_MODE1   ] = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1] = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_IPV4      ]   = SCL_KEY_USW_SIZE_160;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_IPV6      ]   = SCL_KEY_USW_SIZE_320;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_RMAC      ]   = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_RMAC_RID  ]   = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_STA_STATUS]   = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD ] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD] = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TUNNEL_MPLS      ]  = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TRILL_UC]          = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TRILL_UC_RPF]      = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TRILL_MC]          = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TRILL_MC_RPF]      = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_TRILL_MC_ADJ]      = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_ECID]              = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_ING_ECID]          = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_CROSS]        = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_HASH_PORT_VLAN_CROSS]   = SCL_KEY_USW_SIZE_80;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_PORT_DEFAULT_INGRESS  ] = SCL_KEY_USW_SIZE_DEFAULT_PORT;
    p_usw_scl_master[lchip]->key_size[SYS_SCL_KEY_PORT_DEFAULT_EGRESS   ] = SCL_KEY_USW_SIZE_DEFAULT_PORT;

    /*build action*/
    p_usw_scl_master[lchip]->build_ad_func[SYS_SCL_ACTION_INGRESS ] = _sys_usw_scl_build_field_action_igs;
    p_usw_scl_master[lchip]->build_ad_func[SYS_SCL_ACTION_EGRESS  ] = _sys_usw_scl_build_field_action_egs;
    p_usw_scl_master[lchip]->build_ad_func[SYS_SCL_ACTION_TUNNEL  ] = _sys_usw_scl_build_field_action_tunnel;
    p_usw_scl_master[lchip]->build_ad_func[SYS_SCL_ACTION_FLOW    ] = _sys_usw_scl_build_field_action_flow;
    p_usw_scl_master[lchip]->build_ad_func[SYS_SCL_ACTION_MPLS    ] = _sys_usw_scl_build_field_action_mpls;

    /*get action*/
    p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_INGRESS] = _sys_usw_scl_get_field_action_igs;
    p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_EGRESS] = _sys_usw_scl_get_field_action_egs;
    p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_FLOW] = _sys_usw_scl_get_field_action_flow;
    p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_TUNNEL  ] = _sys_usw_scl_get_field_action_tunnel;
    p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_MPLS  ] = _sys_usw_scl_get_field_action_mpls;
    return CTC_E_NONE;
}

