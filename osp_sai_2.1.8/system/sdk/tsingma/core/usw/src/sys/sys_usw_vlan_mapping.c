/**
 @file sys_usw_vlan_mapping.c

 @date 2010-1-4

 @version v2.0

*/

#include "sal.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_register.h"

#include "sys_usw_register.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_chip.h"
#include "sys_usw_vlan.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_aps.h"
#include "sys_usw_ftm.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_register.h"
#include "sys_usw_l3if.h"
#include "sys_usw_stats_api.h"

#include "drv_api.h"

/******************************************************************************
*
*   Macros and Defines
*
*******************************************************************************/

static sys_vlan_mapping_master_t* p_usw_vlan_mapping_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM (DRV_IS_DUET2(lchip)? 0x7FFF: 0xFFFF)

#define SYS_VLAN_RANGE_SW_EACH_GROUP() p_usw_vlan_mapping_master[lchip]->vrange[vrange_info->direction * VLAN_RANGE_ENTRY_NUM + vrange_info->vrange_grpid]
#define SYS_VLAN_RANGE_SW_EACH_MEMBER(mem_idx) p_usw_vlan_mapping_master[lchip]->vrange_mem_use_count[vrange_info->direction * VLAN_RANGE_ENTRY_NUM + vrange_info->vrange_grpid][mem_idx]

#define SYS_VLAN_RANGE_INFO_CHECK(vrange_info) \
    { \
        CTC_BOTH_DIRECTION_CHECK(vrange_info->direction); \
        if (VLAN_RANGE_ENTRY_NUM <= vrange_info->vrange_grpid) \
        {    return CTC_E_INVALID_PARAM; } \
    }

#define SYS_VLAN_ID_VALID_CHECK(vlan_id) \
    { \
        if ((vlan_id) > CTC_MAX_VLAN_ID){ \
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");\
			return CTC_E_BADID;\
 } \
    }

#define SYS_VLAN_MAPPING_INIT_CHECK() \
    { \
        LCHIP_CHECK(lchip); \
        if (p_usw_vlan_mapping_master[lchip] == NULL){ \
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    }

#define SYS_VLAN_MAPPING_CREATE_LOCK(lchip) \
    do \
    { \
        sal_mutex_create(&p_usw_vlan_mapping_master[lchip]->mutex); \
        if (NULL == p_usw_vlan_mapping_master[lchip]->mutex) \
        { \
            return CTC_E_NO_MEMORY; \
        } \
    }while (0)

#define SYS_VLAN_MAPPING_LOCK(lchip) \
    sal_mutex_lock(p_usw_vlan_mapping_master[lchip]->mutex)

#define SYS_VLAN_MAPPING_UNLOCK(lchip) \
    sal_mutex_unlock(p_usw_vlan_mapping_master[lchip]->mutex)

#define CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_vlan_mapping_master[lchip]->mutex); \
            return rv; \
        } \
    }while (0)


#define SYS_VLAN_MAPPING_DBG_OUT(level, FMT, ...) \
    { \
        CTC_DEBUG_OUT(vlan, vlan_mapping, VLAN_MAPPING_SYS, level, FMT, ##__VA_ARGS__); \
    }

#define TAG_CHK_OP(op) \
    { \
        if (op < CTC_VLAN_TAG_OP_NONE || op > CTC_VLAN_TAG_OP_DEL){ return CTC_E_INVALID_PARAM; } \
    }

#define VID_CHK_SELECT(vid_sl) \
    { \
        if (vid_sl < CTC_VLAN_TAG_SL_AS_PARSE || vid_sl > CTC_VLAN_TAG_SL_NEW){ return CTC_E_INVALID_PARAM; } \
    }

#define COS_CHK_SELECT(cos_sl) \
    { \
        if (cos_sl < CTC_VLAN_TAG_SL_AS_PARSE || cos_sl > CTC_VLAN_TAG_SL_NEW){ return CTC_E_INVALID_PARAM; } \
    }

enum sys_vlan_mapping_range_type_e
{
    RANGE_TYPE_IS_SVID  = 0,
    RANGE_TYPE_IS_CVID  = 1
};
typedef enum sys_vlan_mapping_range_type_e sys_vlan_mapping_range_type_t;

struct sys_vlan_mapping_tag_op_s
{
    ctc_vlan_tag_op_t      stag_op;
    ctc_vlan_tag_sl_t      svid_sl;
    ctc_vlan_tag_sl_t      scos_sl;

    ctc_vlan_tag_op_t      ctag_op;
    ctc_vlan_tag_sl_t      cvid_sl;
    ctc_vlan_tag_sl_t      ccos_sl;
};
typedef struct sys_vlan_mapping_tag_op_s sys_vlan_mapping_tag_op_t;

enum sys_vlan_mapping_group_sub_type_e
{
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_IGS,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH1_IGS,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_EGS,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM0,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM1,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM2,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM3,
    SYS_VLAN_MAPPING_GROUP_SUB_TYPE_NUM
};
typedef enum sys_vlan_mapping_group_sub_type_e sys_vlan_mapping_group_sub_type_t;

#define SYS_VLAN_MAPPING_MAP_SCL_KEY_TYPE_BY_KEY(vlan_mapping_key, scl_key_type) \
do{\
    switch (vlan_mapping_key  \
        & (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID \
        | CTC_VLAN_MAPPING_KEY_CTAG_COS | CTC_VLAN_MAPPING_KEY_STAG_COS \
        | CTC_VLAN_MAPPING_KEY_MAC_SA | CTC_VLAN_MAPPING_KEY_IPV4_SA \
        | CTC_VLAN_MAPPING_KEY_IPV6_SA)) \
    {\
        case (CTC_VLAN_MAPPING_KEY_CVID):\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_CVLAN;\
            break;\
        case (CTC_VLAN_MAPPING_KEY_SVID):\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_SVLAN;\
            break;\
        case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID):\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_2VLAN;\
            break;\
        case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_CTAG_COS):\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_CVLAN_COS;\
            break;\
        case (CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_STAG_COS):\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_SVLAN_COS;\
            break;\
        case CTC_VLAN_MAPPING_KEY_MAC_SA:\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_MAC;\
            break;\
        case CTC_VLAN_MAPPING_KEY_IPV4_SA:\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_IPV4;\
            break;\
        case CTC_VLAN_MAPPING_KEY_IPV6_SA:\
            scl_key_type = SYS_SCL_KEY_HASH_PORT_IPV6;\
            break;\
        case CTC_VLAN_MAPPING_KEY_NONE:\
            scl_key_type = SYS_SCL_KEY_HASH_PORT;\
            break;\
        default:\
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");\
			return CTC_E_NOT_SUPPORT;\
\
            break;\
    }\
}while(0)
/******************************************************************************
*
*   Functions
*
*******************************************************************************/
#define __VLAN_RANGE__

STATIC int32
_sys_usw_vlan_get_vlan_range_mode(uint8 lchip, ctc_direction_t dir, uint8* range_mode)
{
    uint32 cmd = 0;
    uint32 vlan_range_mode = 0;

    if (CTC_INGRESS == dir)
    {
        cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_vlanRangeMode_f);
    }
    else
    {
        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_vlanRangeMode_f);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &vlan_range_mode));
    *range_mode = vlan_range_mode;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_get_vlan_range_type(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool* is_svlan)
{

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(vrange_info);
    CTC_PTR_VALID_CHECK(is_svlan);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    if (!CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range not exist\n");
		return CTC_E_NOT_EXIST;

    }

    *is_svlan = (CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_TYPE_BIT_POS))) ? TRUE : FALSE;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_vlan_get_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count)
{
    uint32 cmd = 0;
    uint8 idx = 0;
    uint8 cnt = 0;
    uint32 max = 0;
    uint32 min = 0;
    DsVlanRangeProfile_m         igs_ds;
    DsEgressVlanRangeProfile_m  egs_ds;
    ctc_vlan_range_group_t _vrange_group;
    uint8 direction;
    int32 vrange_grpid;
    uint8  vlan_range_mode = 0;

    CTC_PTR_VALID_CHECK(count);
    CTC_PTR_VALID_CHECK(vrange_info);
    CTC_PTR_VALID_CHECK(vrange_group);

    sal_memset(&igs_ds, 0, sizeof(igs_ds));
    sal_memset(&egs_ds, 0, sizeof(egs_ds));
    sal_memset(&_vrange_group, 0, sizeof(ctc_vlan_range_group_t));

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    if (!CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range not exist\n");
		return CTC_E_NOT_EXIST;

    }

    direction = vrange_info->direction;
    vrange_grpid = vrange_info->vrange_grpid;

    CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_range_mode(lchip, direction, &vlan_range_mode));

    if (CTC_INGRESS == direction)
    {
        cmd = DRV_IOR(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));

        for (idx = 0; idx <8 ;idx++)
        {
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMax0_f + (idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMin0_f + (idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &min);
            _vrange_group.vlan_range[idx].vlan_start = min;
            _vrange_group.vlan_range[idx].vlan_end   = max;
        }
    }
    else if (CTC_EGRESS == direction)
    {
        cmd = DRV_IOR(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));

        for (idx = 0; idx <8 ;idx++)
        {
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMax0_f + (idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMin0_f + (idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &min);
            _vrange_group.vlan_range[idx].vlan_start = min;
            _vrange_group.vlan_range[idx].vlan_end   = max;
        }
    }
    for (idx = 0; idx < 8; idx++)
    {
        if (_vrange_group.vlan_range[idx].vlan_start < _vrange_group.vlan_range[idx].vlan_end)
        {
            if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == vlan_range_mode)
            {
                vrange_group->vlan_range[cnt].is_acl = (idx > 3) ? 1 : 0;
            }
            else if (CTC_GLOBAL_VLAN_RANGE_MODE_ACL == vlan_range_mode)
            {
                vrange_group->vlan_range[cnt].is_acl = 1;
            }
            else
            {
                vrange_group->vlan_range[cnt].is_acl = 0;
            }

            vrange_group->vlan_range[cnt].vlan_start = _vrange_group.vlan_range[idx].vlan_start;
            vrange_group->vlan_range[cnt].vlan_end   = _vrange_group.vlan_range[idx].vlan_end;
            cnt++;
        }
    }

    *count = cnt;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_vlan_get_maxvid_from_vrange(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range, bool is_svlan, uint16* p_max_vid)
{
    uint32 cmd = 0;
    uint32 max  = 0;
    uint32 min  = 0;
    uint8  idx = 0;
    uint8  idx_start =0;
    uint8  idx_end = 0;
    uint8  vlan_range_mode = 0;
    bool is_svlan_range = FALSE;
    DsVlanRangeProfile_m        igs_ds;
    DsEgressVlanRangeProfile_m  egs_ds;

    sal_memset(&igs_ds, 0, sizeof(igs_ds));
    sal_memset(&egs_ds, 0, sizeof(egs_ds));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(vrange_info);
    CTC_PTR_VALID_CHECK(vlan_range);
    CTC_PTR_VALID_CHECK(p_max_vid);

    if (0 == vlan_range->vlan_end)
    {
        return CTC_E_NONE;
    }

    CTC_VLAN_RANGE_CHECK(vlan_range->vlan_start);
    CTC_VLAN_RANGE_CHECK(vlan_range->vlan_end);
    CTC_BOTH_DIRECTION_CHECK(vrange_info->direction);

    if (vlan_range->vlan_start >= vlan_range->vlan_end)
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] End vlan id should greater than start vlan id \n");
			return CTC_E_INVALID_CONFIG;

    }

    CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_range_type(lchip, vrange_info, &is_svlan_range));
    if (is_svlan != is_svlan_range)
    {
        return CTC_E_INVALID_PARAM;
    }

    /* get vlan range mode */
    CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_range_mode(lchip, vrange_info->direction, &vlan_range_mode));

    if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == vlan_range_mode)
    {
        idx_start = vlan_range->is_acl ? 4 : 0;
        idx_end = vlan_range->is_acl ? 8 : 4;
    }
    else
    {
        idx_start = 0;
        idx_end = 8;
    }

    if (CTC_INGRESS == vrange_info->direction)
    {
        cmd = DRV_IOR(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vrange_info->vrange_grpid, cmd, &igs_ds));

        for (idx = idx_start; idx < idx_end; idx++)
        {
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMax0_f + (idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMin0_f + (idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &min);
            if ((vlan_range->vlan_end == max) && (vlan_range->vlan_start == min)
                && (max != min))
            {
                break;
            }
        }

        if (idx >= idx_end)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping get range failed\n");
			return CTC_E_INVALID_CONFIG;

        }

    }
    else
    {
        cmd = DRV_IOR(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vrange_info->vrange_grpid, cmd, &egs_ds));

        for (idx = idx_start; idx < idx_end; idx++)
        {
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMax0_f + (idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMin0_f + (idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &min);
            if ((vlan_range->vlan_end == max) && (vlan_range->vlan_start == min)
                && (max != min))
            {
                break;
            }
        }

        if (idx >= idx_end)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping get range failed\n");
			return CTC_E_INVALID_CONFIG;

        }
    }

    *p_max_vid = max;

    return CTC_E_NONE;
}

#define __INGRESS_VLAN_MAPPING__

STATIC int32
_sys_usw_vlan_check_vlan_mapping_action(uint8 lchip, ctc_direction_t dir, sys_vlan_mapping_tag_op_t* p_vlan_mapping)
{
    switch (p_vlan_mapping->stag_op)
    {
    case CTC_VLAN_TAG_OP_NONE:
    case CTC_VLAN_TAG_OP_DEL:
        if ((CTC_VLAN_TAG_SL_AS_PARSE != p_vlan_mapping->svid_sl) || (CTC_VLAN_TAG_SL_AS_PARSE != p_vlan_mapping->scos_sl))
        {
            return CTC_E_NOT_SUPPORT;
        }

        break;

    default:
        break;
    }

    switch (p_vlan_mapping->ctag_op)
    {
    case CTC_VLAN_TAG_OP_NONE:
    case CTC_VLAN_TAG_OP_DEL:
        if ((CTC_VLAN_TAG_SL_AS_PARSE != p_vlan_mapping->cvid_sl) || (CTC_VLAN_TAG_SL_AS_PARSE != p_vlan_mapping->ccos_sl))
        {
            return CTC_E_NOT_SUPPORT;
        }

        break;

    default:
        break;
    }

    if (CTC_EGRESS == dir)
    {
        if ((CTC_VLAN_TAG_SL_DEFAULT == p_vlan_mapping->cvid_sl) || (CTC_VLAN_TAG_SL_DEFAULT == p_vlan_mapping->svid_sl))
        {
            return CTC_E_NOT_SUPPORT;
        }
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_vlan_mapping_get_scl_gid(uint8 lchip, uint8 dir, uint16 lport, uint8 is_logic_port, uint8 resolve_conflict, uint8 block_id)
{
    uint32 gid = 0;

    if (resolve_conflict)
    {
        if (!is_logic_port)
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, (SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM0 + block_id), CTC_FIELD_PORT_TYPE_GPORT, 0, lport);
        }
        else
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, (SYS_VLAN_MAPPING_GROUP_SUB_TYPE_TCAM0 + block_id), CTC_FIELD_PORT_TYPE_LOGIC_PORT, 0, lport);
        }
    }
    else
    {

        if (CTC_INGRESS == dir)
        {
            if (!is_logic_port)
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, (SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_IGS + block_id), CTC_FIELD_PORT_TYPE_GPORT, 0, lport);
            }
            else
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, (SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_IGS + block_id), CTC_FIELD_PORT_TYPE_LOGIC_PORT, 0, lport);
            }
        }
        else
        {
            if (!is_logic_port)
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_EGS, CTC_FIELD_PORT_TYPE_GPORT, 0, lport);
            }
            else
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN_MAPPING, SYS_VLAN_MAPPING_GROUP_SUB_TYPE_HASH_EGS, CTC_FIELD_PORT_TYPE_LOGIC_PORT, 0, lport);
            }
        }

    }

    return gid;
}

STATIC int32
_sys_usw_vlan_mapping_add_action_field(uint8 lchip, uint8 scl_id, uint32 entry_id, ctc_scl_field_action_t* p_field_action, uint8 is_default, uint32 gport, uint8 action_type)
{
    sys_scl_default_action_t default_action;
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));
    default_action.scl_id = scl_id;

    if (is_default)
    {
        default_action.lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
        default_action.action_type = action_type;
        default_action.field_action = p_field_action;
        CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, p_field_action));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_mapping_build_igs_key(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping_in, ctc_scl_entry_t* scl_entry, sys_scl_lkup_key_t* lkup_key, uint8 is_add)
{
    uint16 old_cvid = 0;
    uint16 old_svid = 0;

    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t vlan_range;

    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;
    ctc_field_key_t  field_key;

    mac_addr_t mac_mask;
    ipv6_addr_t ipv6_sa_mask;

    sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    sal_memset(&vrange_info, 0, sizeof(ctc_vlan_range_info_t));
    sal_memset(&vlan_range, 0, sizeof(ctc_vlan_range_t));
    sal_memset(&mac_mask, 0xFF, sizeof(mac_addr_t));
    sal_memset(&ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if((p_vlan_mapping_in->key > CTC_VLAN_MAPPING_KEY_MAC_SA) && (p_vlan_mapping_in->key & 0xF))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (p_vlan_mapping_in->key
            & (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID
               | CTC_VLAN_MAPPING_KEY_CTAG_COS | CTC_VLAN_MAPPING_KEY_STAG_COS
               | CTC_VLAN_MAPPING_KEY_MAC_SA | CTC_VLAN_MAPPING_KEY_IPV4_SA
               | CTC_VLAN_MAPPING_KEY_IPV6_SA))
    {
    case (CTC_VLAN_MAPPING_KEY_CVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        old_cvid = p_vlan_mapping_in->old_cvid;

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port= 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport= 0xFFFFFFFF;
        }

        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        break;

    case (CTC_VLAN_MAPPING_KEY_SVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        break;

    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        old_cvid = p_vlan_mapping_in->old_cvid;
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_CTAG_COS):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->old_ccos);
        old_cvid = p_vlan_mapping_in->old_cvid;

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CTAG_COS;
        field_key.data = p_vlan_mapping_in->old_ccos;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_STAG_COS):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->old_scos);
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_INGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_STAG_COS;
        field_key.data = p_vlan_mapping_in->old_scos;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case CTC_VLAN_MAPPING_KEY_MAC_SA:
        /* only support use macsa as key */
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_vlan_mapping_in->macsa;
        field_key.ext_mask = mac_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case CTC_VLAN_MAPPING_KEY_IPV4_SA:
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_IP_SA;
        field_key.data = p_vlan_mapping_in->ipv4_sa;
        field_key.mask = 0xFFFFFFFF;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case CTC_VLAN_MAPPING_KEY_IPV6_SA:
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_IPV6_SA;
        field_key.ext_data = p_vlan_mapping_in->ipv6_sa;
        field_key.ext_mask = ipv6_sa_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;
    case CTC_VLAN_MAPPING_KEY_NONE:
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = gport;
            port_mask.logic_port = 0xFFFF;
        }
        else
        {
            port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_data.gport = gport;
            port_mask.gport = 0xFFFFFFFF;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &port_data;
        field_key.ext_mask = &port_mask;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

/*
    if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX))
    {
        pc->key.resolve_conflict = 1;
    }
*/
    return CTC_E_NONE;

}

/*some props are not supported in ingress vlan, should return error.*/
STATIC int32
_sys_usw_vlan_mapping_build_igs_ad(uint8 lchip, ctc_vlan_mapping_t* p_vlan_mapping_in, uint32 entry_id, uint32 gport)
{
    uint8  is_logic_port_en     = 0;
    uint8  is_aps_en      = 0;
    uint8  is_fid_en      = 0;
    uint8  is_nh_en       = 0;
    uint8  is_service_plc_en  = 0;
    uint8  is_service_acl_en  = 0;
    uint8  is_vn_id_en        = 0;
    uint8  is_stats_en        = 0;
    uint8  is_stp_id_en        = 0;
    uint8 is_l2vpn_oam = 0;
    uint8 is_vlan_domain_en = 0;
    uint8  is_srvc_en     = 0;
    uint8  is_vrfid_en        = 0;
    uint8  mode = 0;
    uint8  is_default = 0;
    uint8  action_type = SYS_SCL_ACTION_INGRESS;
    uint8 xgpon_en = 0;
    ctc_scl_field_action_t field_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((p_vlan_mapping_in->stag_op >= CTC_VLAN_TAG_OP_MAX)
        || (p_vlan_mapping_in->svid_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->scos_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->ctag_op >= CTC_VLAN_TAG_OP_MAX)
        || (p_vlan_mapping_in->cvid_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->ccos_sl >= CTC_VLAN_TAG_SL_MAX))
    {
        return CTC_E_INVALID_PARAM;
    }

    is_logic_port_en    = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
    is_aps_en     = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_APS_SELECT);
    is_fid_en     = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_FID);
    is_nh_en      = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_NHID);
    is_service_plc_en = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN);
    is_service_acl_en = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN);
    is_vn_id_en   = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_VN_ID);
    is_stats_en       = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_STATS_EN);
    is_stp_id_en   = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_STP_ID);
    is_l2vpn_oam = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_L2VPN_OAM);
    is_vlan_domain_en = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN);
    is_srvc_en    = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID);
    is_vrfid_en       = CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_VRFID);
    is_default    = CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY);

    if ((is_fid_en + is_nh_en + is_vn_id_en + is_vrfid_en) > 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    sys_usw_vlan_overlay_get_vni_fid_mapping_mode(lchip, &mode);
    if (is_vn_id_en && mode)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPWS | CTC_VLAN_MAPPING_FLAG_VPLS))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (is_l2vpn_oam)
    {
        if ((!CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPWS))
            && (!CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPLS)))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    if ((p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_MACLIMIT_EN) ||
        (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_IGMP_SNOOPING_EN))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    if(is_aps_en)
    {
        if (0 != p_vlan_mapping_in->protected_vlan)
        {
            CTC_VLAN_RANGE_CHECK(p_vlan_mapping_in->protected_vlan);
        }
        SYS_APS_GROUP_ID_CHECK(p_vlan_mapping_in->aps_select_group_id);
    }
    if(is_fid_en)
    {
        if (p_vlan_mapping_in->u3.fid == 0 || p_vlan_mapping_in->u3.fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
        {
            return CTC_E_BADID;
        }
    }
    if(is_stp_id_en)
    {
        CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->stp_id, 127);
    }
    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->user_vlanptr,0x1fff);
    }
    if (is_l2vpn_oam)
    {
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPWS))
        {
            if (p_vlan_mapping_in->l2vpn_oam_id > 0xFFF)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        if (p_vlan_mapping_in->stag_op || p_vlan_mapping_in->ctag_op || is_vlan_domain_en ||
           (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPLS) ||
           (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPWS))
        {
            SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->new_svid);
            SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->new_cvid);
            CTC_COS_RANGE_CHECK(p_vlan_mapping_in->new_scos);
            CTC_COS_RANGE_CHECK(p_vlan_mapping_in->new_ccos);
            CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->vlan_domain, CTC_SCL_VLAN_DOMAIN_MAX - 1);
        }
    }
    if (is_vrfid_en)
    {
        SYS_VRFID_CHECK(p_vlan_mapping_in->u3.vrf_id);
    }
    if (is_stats_en)
    {
        uint32 cache_ptr = 0;
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_vlan_mapping_in->stats_id, &cache_ptr));
    }
    if (is_srvc_en)
    {
        CTC_MIN_VALUE_CHECK(p_vlan_mapping_in->service_id, 1);
    }
    if (is_nh_en)
    {
        sys_nh_info_com_t *p_nhinfo = NULL;
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_vlan_mapping_in->u3.nh_id, &p_nhinfo));
    }
    if (p_vlan_mapping_in->policer_id > 0)
    {
        uint8 type = 0;
        sys_qos_policer_param_t policer_param;
        sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
        type = SYS_QOS_POLICER_TYPE_SERVICE;
        CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, p_vlan_mapping_in->policer_id,
                                                       type,
                                                       &policer_param));
    }
    if (is_vn_id_en)
    {
        uint16 fid = 0;
        CTC_ERROR_RETURN(sys_usw_vlan_overlay_get_fid(lchip, p_vlan_mapping_in->u3.vn_id, &fid));
    }
    CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->color, CTC_QOS_COLOR_GREEN);
    CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));


    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);

    if (is_logic_port_en)
    {
        ctc_scl_logic_port_t logic_port;
        if (!DRV_IS_DUET2(lchip) && xgpon_en)
        {
            field_action.data0 = CTC_IS_BIT_SET(p_vlan_mapping_in->logic_src_port, 15) << 3;
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
            CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->logic_src_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
        }
        sal_memset(&logic_port, 0, sizeof(ctc_scl_logic_port_t));
        logic_port.logic_port = p_vlan_mapping_in->logic_src_port & MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT);
        logic_port.logic_port_type = p_vlan_mapping_in->logic_port_type;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
        field_action.ext_data = &logic_port;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }
    else
    {
        if (!DRV_IS_DUET2(lchip) && xgpon_en)
        {
            field_action.data0 = 1<<4;
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
            CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        }
    }

    if (is_aps_en)
    {
        ctc_scl_aps_t aps;
        sal_memset(&aps, 0, sizeof(ctc_scl_aps_t));
        if (0 != p_vlan_mapping_in->protected_vlan)
        {
            aps.protected_vlan_valid = 1;
            aps.protected_vlan = p_vlan_mapping_in->protected_vlan;
        }
        SYS_APS_GROUP_ID_CHECK(p_vlan_mapping_in->aps_select_group_id);
        aps.aps_select_group_id = p_vlan_mapping_in->aps_select_group_id;
        aps.is_working_path = p_vlan_mapping_in->is_working_path;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_APS;
        field_action.ext_data = &aps;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        /* need to check logic port */
    }

    if (p_vlan_mapping_in->policer_id > 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_vlan_mapping_in->policer_id;
        if (is_service_plc_en)
        {
            field_action.data1 = 1;
        }
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_srvc_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;
        field_action.data0 = p_vlan_mapping_in->service_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        /*
        if CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_SERVICE_QUEUE_EN)
        {
            CTC_SET_FLAG(pia->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_QUEUE_EN);
        }
        */
    }

    if (is_service_acl_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_fid_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0 = p_vlan_mapping_in->u3.fid;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_stp_id_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STP_ID;
        field_action.data0 = p_vlan_mapping_in->stp_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
        field_action.data0 = p_vlan_mapping_in->user_vlanptr;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_vn_id_en)
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VN_ID;
        field_action.data0 = p_vlan_mapping_in->u3.vn_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_vrfid_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
        field_action.data0 = p_vlan_mapping_in->u3.vrf_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (CTC_QOS_COLOR_NONE != p_vlan_mapping_in->color)
    {
        ctc_scl_qos_map_t qos;
        sal_memset(&qos, 0, sizeof(ctc_scl_qos_map_t));
        qos.color = p_vlan_mapping_in->color;
        if (p_vlan_mapping_in->priority)
        {
            CTC_SET_FLAG(qos.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
            qos.priority = p_vlan_mapping_in->priority;
        }

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPLS)
    {
        ctc_scl_logic_port_t  scl_logic_port;

        /*Need Logic port*/
        sal_memset(&scl_logic_port, 0, sizeof(ctc_scl_logic_port_t));
        scl_logic_port.logic_port = p_vlan_mapping_in->logic_src_port;
        scl_logic_port.logic_port_type = p_vlan_mapping_in->logic_port_type;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
        field_action.ext_data = (void*)&scl_logic_port;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));

        if(0 != p_vlan_mapping_in->user_vlanptr)
        {
            /*Need Vlan Ptr*/
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
            field_action.data0 = p_vlan_mapping_in->user_vlanptr;
            CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        }

        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VPLS;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if(p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPWS)
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VPWS;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    /*for vpws*/
    if (is_nh_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_vlan_mapping_in->u3.nh_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_stats_en)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_vlan_mapping_in->stats_id;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if (is_l2vpn_oam)
    {
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPWS))
        {
            /* vpws */
            field_action.data0 = 1;
        }
        else
        {
            field_action.data0 = 0;
        }
        field_action.data1 = p_vlan_mapping_in->l2vpn_oam_id;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if(CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS))
    {
        sys_scl_wlan_client_t wlan_client;
        sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
        field_action.ext_data = &wlan_client;
        wlan_client.sta_roam_status = 1;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
    }

    if(CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN))
    {
        if (xgpon_en)
        {
            sys_scl_wlan_client_t wlan_client;
            sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));
            if (1 == p_vlan_mapping_in->l2vpn_oam_id)
            {
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN;
                CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
            }
            else if (2 == p_vlan_mapping_in->l2vpn_oam_id)
            {
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
                CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
            }
            else if (3 == p_vlan_mapping_in->l2vpn_oam_id)
            {
                field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
                field_action.ext_data = &wlan_client;
                wlan_client.sta_roam_status = 1;
                CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
            }
        }
        else
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN;
            CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));
        }
    }

    if (p_vlan_mapping_in->stag_op || p_vlan_mapping_in->ctag_op || is_vlan_domain_en ||
        (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPLS) ||
        (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPWS))
    {
        ctc_scl_vlan_edit_t vlan_edit;
        sal_memset(&vlan_edit, 0 ,sizeof(ctc_scl_vlan_edit_t));

        /* stag op is a global concept, that is to say vid/cos/cfi has the same op type, while choose which value to finish this op is differently selectable */
        vlan_edit.stag_op = p_vlan_mapping_in->stag_op;
        vlan_edit.ctag_op = p_vlan_mapping_in->ctag_op;
        vlan_edit.svid_sl = p_vlan_mapping_in->svid_sl;
        vlan_edit.cvid_sl = p_vlan_mapping_in->cvid_sl;
        vlan_edit.scos_sl = p_vlan_mapping_in->scos_sl;
        vlan_edit.ccos_sl = p_vlan_mapping_in->ccos_sl;
        vlan_edit.svid_new = p_vlan_mapping_in->new_svid;
        vlan_edit.cvid_new = p_vlan_mapping_in->new_cvid;
        vlan_edit.scos_new = p_vlan_mapping_in->new_scos;
        vlan_edit.ccos_new = p_vlan_mapping_in->new_ccos;
        vlan_edit.tpid_index = p_vlan_mapping_in->tpid_index;

        if ((p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPLS) ||
            (p_vlan_mapping_in->action & CTC_VLAN_MAPPING_FLAG_VPWS))
        {
             vlan_edit.vlan_domain = CTC_SCL_VLAN_DOMAIN_UNCHANGE;
            if (CTC_VLAN_TAG_OP_NONE == vlan_edit.stag_op)
            {
                vlan_edit.stag_op     = CTC_VLAN_TAG_OP_VALID;
            }
        }

        if (is_vlan_domain_en)
        {
            vlan_edit.vlan_domain = p_vlan_mapping_in->vlan_domain;
        }

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, p_vlan_mapping_in->scl_id, entry_id, &field_action, is_default, gport, action_type));

    }

    return CTC_E_NONE;

}

STATIC int32 _sys_usw_vlan_mapping_unbuild_ad(uint8 lchip, uint32 entry_id, ctc_vlan_mapping_t* p_vlan_mapping)
{
    ctc_scl_field_action_t   field_action;
    ctc_scl_logic_port_t logic_port;
    ctc_scl_aps_t aps;
    ctc_scl_qos_map_t qos;
    ctc_scl_vlan_edit_t vlan_edit;
    uint8 xgpon_en = 0;
    int32 ret;

    sal_memset(&vlan_edit, 0 ,sizeof(ctc_scl_vlan_edit_t));
    sal_memset(&qos, 0, sizeof(ctc_scl_qos_map_t));
    sal_memset(&aps, 0, sizeof(ctc_scl_aps_t));
    sal_memset(&logic_port, 0, sizeof(ctc_scl_logic_port_t));
    sal_memset(&field_action, 0 , sizeof(ctc_scl_field_action_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    field_action.ext_data = &logic_port;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->logic_src_port = logic_port.logic_port;
        p_vlan_mapping->logic_port_type = logic_port.logic_port_type;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_APS;
    field_action.ext_data = &aps;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->is_working_path = aps.is_working_path;
        p_vlan_mapping->aps_select_group_id = aps.aps_select_group_id;
        if(aps.protected_vlan_valid)
        {
            p_vlan_mapping->protected_vlan = aps.protected_vlan;
        }
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_APS_SELECT);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        if(field_action.data1)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN);
        }
        p_vlan_mapping->policer_id = field_action.data0;
    }


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID);
        p_vlan_mapping->service_id = field_action.data0;
    }


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN);
    }


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_FID);
        p_vlan_mapping->u3.fid = field_action.data0;
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STP_ID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_STP_ID);
        p_vlan_mapping->stp_id = field_action.data0;
    }


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR);
        p_vlan_mapping->user_vlanptr = field_action.data0;
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->u3.vrf_id = field_action.data0;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VRFID);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
    field_action.ext_data = &qos;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->priority = qos.priority;
        p_vlan_mapping->color = qos.color;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VRFID);
    }

    /*for vpws*/
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->u3.nh_id = field_action.data0;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_NHID);
    }


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->stats_id = field_action.data0;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_STATS_EN);

    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->l2vpn_oam_id = field_action.data1;
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_L2VPN_OAM);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    field_action.ext_data = &vlan_edit;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        p_vlan_mapping->stag_op = vlan_edit.stag_op;
        p_vlan_mapping->ctag_op = vlan_edit.ctag_op;
        p_vlan_mapping->svid_sl = vlan_edit.svid_sl;
        p_vlan_mapping->cvid_sl = vlan_edit.cvid_sl;
        p_vlan_mapping->scos_sl = vlan_edit.scos_sl;
        p_vlan_mapping->ccos_sl = vlan_edit.ccos_sl;
        p_vlan_mapping->new_svid = vlan_edit.svid_new;
        p_vlan_mapping->new_cvid = vlan_edit.cvid_new;
        p_vlan_mapping->new_scos = vlan_edit.scos_new;
        p_vlan_mapping->new_ccos = vlan_edit.ccos_new;
        p_vlan_mapping->vlan_domain = vlan_edit.vlan_domain;
        p_vlan_mapping->tpid_index = vlan_edit.tpid_index;
        if(vlan_edit.vlan_domain)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN);
        }

        if(p_vlan_mapping->svid_sl == CTC_VLAN_TAG_SL_NEW)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
        }
        if(p_vlan_mapping->scos_sl == CTC_VLAN_TAG_SL_NEW)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
        }
        if(p_vlan_mapping->cvid_sl == CTC_VLAN_TAG_SL_NEW)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
        }
        if(p_vlan_mapping->ccos_sl == CTC_VLAN_TAG_SL_NEW)
        {
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
        }
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VN_ID;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VN_ID);
        p_vlan_mapping->u3.vn_id = field_action.data0;
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF);
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VPLS;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_VPLS);
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VPWS;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_VPWS);
    }

    /*special process for tm xgpon*/
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if (!DRV_IS_DUET2(lchip) && xgpon_en && CTC_IS_BIT_SET(p_vlan_mapping->user_vlanptr, 3))
    {
        CTC_BIT_UNSET(p_vlan_mapping->user_vlanptr, 3);
        CTC_UNSET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR);
        CTC_BIT_SET(p_vlan_mapping->logic_src_port, 15);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_mapping_set_igs_default_action(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping, uint8 is_add)
{
    uint16 lport = 0;
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_default_action_t default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT) || CTC_IS_LINKAGG_PORT(gport))
    {
        /* default entry do not support logic port and linkagg port */
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    default_action.lport = lport;
    default_action.action_type = SYS_SCL_ACTION_INGRESS;
    default_action.field_action = &field_action;
    default_action.scl_id = p_vlan_mapping->scl_id;

    /* set pending status */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    if (is_add)
    {
        /* add action field, entry_id is meaningless */
        CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_igs_ad(lchip, p_vlan_mapping, 0, gport), ret, error0);
    }

    /* clear pending status and install to hw */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_usw_scl_set_default_action(lchip, &default_action);

    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    default_action.is_rollback = 1;
    sys_usw_scl_set_default_action(lchip, &default_action);
    return ret;
}

STATIC int32
_sys_usw_vlan_add_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping, bool overwrite)
{

    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t   scl;
    uint32  gid  = 0;
    sys_vlan_mapping_tag_op_t vlan_mapping_op;
    ctc_field_key_t field_key;
    uint8 is_logic_port = 0;
    uint16 lport = 0;
    ctc_scl_group_info_t group;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);
    CTC_MAX_VALUE_CHECK(p_vlan_mapping->scl_id, 1);
    CTC_MAX_VALUE_CHECK(p_vlan_mapping->tpid_index, 3);

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                     :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                 :%d\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " action                    :0x%X\n", p_vlan_mapping->action);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                      :0x%X\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                      :0x%X\n", p_vlan_mapping->old_cvid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svlan end                 :0x%X\n", p_vlan_mapping->svlan_end);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvlan end                 :0x%X\n", p_vlan_mapping->cvlan_end);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "-------mapping to-----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " new svid                  :%d\n", p_vlan_mapping->new_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " new cvid                  :%d\n", p_vlan_mapping->new_cvid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " fid                       :%d\n", p_vlan_mapping->u3.fid);
    if (CTC_FLAG_ISSET(p_vlan_mapping->action, CTC_VLAN_MAPPING_FLAG_L2VPN_OAM))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " l2vpn oam id              :%d\n", p_vlan_mapping->l2vpn_oam_id);
    }
    vlan_mapping_op.ccos_sl = p_vlan_mapping->ccos_sl;
    vlan_mapping_op.cvid_sl = p_vlan_mapping->cvid_sl;
    vlan_mapping_op.ctag_op = p_vlan_mapping->ctag_op;
    vlan_mapping_op.scos_sl = p_vlan_mapping->scos_sl;
    vlan_mapping_op.svid_sl = p_vlan_mapping->svid_sl;
    vlan_mapping_op.stag_op = p_vlan_mapping->stag_op;
    CTC_ERROR_RETURN(_sys_usw_vlan_check_vlan_mapping_action(lchip, CTC_INGRESS, &vlan_mapping_op));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY))
    {
        return _sys_usw_vlan_mapping_set_igs_default_action(lchip, gport, p_vlan_mapping, 1);
    }

    sal_memset(&scl, 0, sizeof(ctc_scl_entry_t));

    /* do not care about whether is use flex */
    SYS_VLAN_MAPPING_MAP_SCL_KEY_TYPE_BY_KEY(p_vlan_mapping->key, scl.key_type);
    scl.action_type = SYS_SCL_ACTION_INGRESS;
    scl.resolve_conflict = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);

    /* for getting group id use */
    is_logic_port = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_INGRESS, lport, is_logic_port, scl.resolve_conflict, p_vlan_mapping->scl_id);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);

    if (!overwrite) /*add*/
    {
        group.type = CTC_SCL_GROUP_TYPE_NONE;
        /* use default priority for inter module */
        group.priority = p_vlan_mapping->scl_id;
        ret = sys_usw_scl_create_group(lchip, gid, &group, 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            return ret;
        }

        /* before adding a entry, the target group must have been created already */
        CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, gid, &scl, 1));

        CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_igs_key(lchip, gport, p_vlan_mapping, &scl, NULL, 1), ret, error0);

        /* if is hash entry, do not forget to valid this key */
        if (!scl.resolve_conflict)
        {
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl.entry_id, &field_key), ret, error0);
        }


        CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_igs_ad(lchip, p_vlan_mapping, scl.entry_id, gport), ret, error0);

        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl.entry_id, 1), ret, error0);
    }
    else  /*update*/
    {
        ctc_scl_field_action_t field_action;
        sys_scl_lkup_key_t lkup_key;
        sal_memset(&field_action, 0 , sizeof(ctc_scl_field_action_t));
        sal_memset(&lkup_key, 0 , sizeof(sys_scl_lkup_key_t));

        lkup_key.key_type = scl.key_type;
        lkup_key.action_type = scl.action_type;
        lkup_key.group_priority = p_vlan_mapping->scl_id;
        lkup_key.resolve_conflict = scl.resolve_conflict;
        lkup_key.group_id = gid;

        CTC_ERROR_RETURN(_sys_usw_vlan_mapping_build_igs_key(lchip, gport, p_vlan_mapping, NULL, &lkup_key, 0));

        /* to sepcify, add entry when calculate key index will make valid bit and key common field included, so when remove as well */
        if (!scl.resolve_conflict)
        {
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
        }

        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));


        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, lkup_key.entry_id, &field_action));

        ret = _sys_usw_vlan_mapping_build_igs_ad(lchip, p_vlan_mapping, lkup_key.entry_id, gport);
        if (CTC_E_NONE != ret)
        {
            /* remove pending only used for roll back, which must be used together with add pending */
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
            sys_usw_scl_remove_action_field(lchip, lkup_key.entry_id, &field_action);
            return ret;
        }

        CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, lkup_key.entry_id, 1));

    }

    return CTC_E_NONE;

error0:
    sys_usw_scl_remove_entry(lchip, scl.entry_id, 1);
    return ret;
}

#define __EGRESS_VLAN_MAPPING__

STATIC int32
_sys_usw_vlan_mapping_build_egs_key(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping_in, ctc_scl_entry_t* scl_entry, sys_scl_lkup_key_t* lkup_key, uint8 is_add)
{
    uint16 old_cvid = 0;
    uint16 old_svid = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t vlan_range;
    ctc_field_port_t port;
    ctc_field_key_t field_key;

    sal_memset(&vrange_info, 0, sizeof(ctc_vlan_range_info_t));
    sal_memset(&vlan_range, 0, sizeof(ctc_vlan_range_t));
    sal_memset(&port, 0, sizeof(ctc_field_port_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (p_vlan_mapping_in->key
            & (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID
               | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS))
    {
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        old_cvid = p_vlan_mapping_in->old_cvid;

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        old_cvid = p_vlan_mapping_in->old_cvid;
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_cvid);
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->old_ccos);
        old_cvid = p_vlan_mapping_in->old_cvid;

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_cvid;
        vlan_range.vlan_end = p_vlan_mapping_in->cvlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, FALSE, &old_cvid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = old_cvid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_CTAG_COS;
        field_key.data = p_vlan_mapping_in->old_ccos;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->old_svid);
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->old_scos);
        old_svid = p_vlan_mapping_in->old_svid;

        vrange_info.direction = CTC_EGRESS;
        vrange_info.vrange_grpid = p_vlan_mapping_in->vrange_grpid;
        vlan_range.is_acl = 0;
        vlan_range.vlan_start = old_svid;
        vlan_range.vlan_end = p_vlan_mapping_in->svlan_end;
        CTC_ERROR_RETURN(_sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, TRUE, &old_svid));

        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = old_svid;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }

        field_key.type = CTC_FIELD_KEY_STAG_COS;
        field_key.data = p_vlan_mapping_in->old_scos;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    case CTC_EGRESS_VLAN_MAPPING_KEY_NONE:
        if (CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
        {
            port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port.logic_port = gport;
        }
        else
        {
            port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port.gport = gport;
        }
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
        field_key.ext_data = &port;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
        break;

    default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_vlan_mapping_build_egs_ad(uint8 lchip, ctc_egress_vlan_mapping_t* p_vlan_mapping_in, uint32 entry_id, uint32 gport)
{
    uint8 is_default = 0;
    uint8 action_type = SYS_SCL_ACTION_EGRESS;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;

    sal_memset(&field_action, 0 ,sizeof(ctc_scl_field_action_t));
    sal_memset(&vlan_edit, 0 ,sizeof(ctc_scl_vlan_edit_t));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((p_vlan_mapping_in->stag_op >= CTC_VLAN_TAG_OP_MAX)
        || (p_vlan_mapping_in->svid_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->scos_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->scfi_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->ctag_op >= CTC_VLAN_TAG_OP_MAX)
        || (p_vlan_mapping_in->cvid_sl >= CTC_VLAN_TAG_SL_MAX)
        || (p_vlan_mapping_in->ccos_sl >= CTC_VLAN_TAG_SL_MAX))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_vlan_mapping_in->color != 0 ) || (p_vlan_mapping_in->priority != 0))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    is_default = CTC_FLAG_ISSET(p_vlan_mapping_in->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY);

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID))
    {
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->new_svid);
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS))
    {
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->new_scos);
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCFI))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_mapping_in->new_scfi, 1);
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID))
    {
        SYS_VLAN_ID_VALID_CHECK(p_vlan_mapping_in->new_cvid);
    }

    if (CTC_FLAG_ISSET(p_vlan_mapping_in->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS))
    {
        CTC_COS_RANGE_CHECK(p_vlan_mapping_in->new_ccos);
    }
    vlan_edit.stag_op = p_vlan_mapping_in->stag_op;
    vlan_edit.svid_sl = p_vlan_mapping_in->svid_sl;
    vlan_edit.scos_sl = p_vlan_mapping_in->scos_sl;
    vlan_edit.scfi_sl = p_vlan_mapping_in->scfi_sl;
    vlan_edit.svid_new = p_vlan_mapping_in->new_svid;
    vlan_edit.scos_new = p_vlan_mapping_in->new_scos;
    vlan_edit.scfi_new = p_vlan_mapping_in->new_scfi;
    vlan_edit.ctag_op = p_vlan_mapping_in->ctag_op;
    vlan_edit.cvid_sl = p_vlan_mapping_in->cvid_sl;
    vlan_edit.ccos_sl = p_vlan_mapping_in->ccos_sl;
    vlan_edit.cvid_new = p_vlan_mapping_in->new_cvid;
    vlan_edit.ccos_new = p_vlan_mapping_in->new_ccos;
    vlan_edit.tpid_index = p_vlan_mapping_in->tpid_index;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    field_action.ext_data = &vlan_edit;
    CTC_ERROR_RETURN(_sys_usw_vlan_mapping_add_action_field(lchip, 0, entry_id, &field_action, is_default, gport, action_type));
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_vlan_mapping_unbuild_egs_ad(uint8 lchip, ctc_egress_vlan_mapping_t* p_vlan_mapping, uint32 entry_id)
{
    int32 ret = 0;
    ctc_scl_field_action_t   field_action;
    ctc_scl_vlan_edit_t      vlan_edit;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&field_action, 0, sizeof(field_action));
    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    field_action.ext_data = &vlan_edit;
    ret = sys_usw_scl_get_action_field(lchip, entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
    p_vlan_mapping->stag_op = vlan_edit.stag_op;
    p_vlan_mapping->ctag_op = vlan_edit.ctag_op;
    p_vlan_mapping->svid_sl = vlan_edit.svid_sl;
    p_vlan_mapping->cvid_sl = vlan_edit.cvid_sl;
    p_vlan_mapping->scos_sl = vlan_edit.scos_sl;
    p_vlan_mapping->ccos_sl = vlan_edit.ccos_sl;
    p_vlan_mapping->new_svid = vlan_edit.svid_new;
    p_vlan_mapping->new_cvid = vlan_edit.cvid_new;
    p_vlan_mapping->new_scos = vlan_edit.scos_new;
    p_vlan_mapping->new_ccos = vlan_edit.ccos_new;
    p_vlan_mapping->tpid_index = vlan_edit.tpid_index;
    /*
    p_vlan_mapping->vlan_domain = vlan_edit.vlan_domain;
    if(vlan_edit.vlan_domain)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN);
    }
    */
    if(p_vlan_mapping->svid_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID);
    }
    if(p_vlan_mapping->scos_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS);
    }
    if(p_vlan_mapping->cvid_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID);
    }
    if(p_vlan_mapping->ccos_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS);
    }
    if(p_vlan_mapping->scfi_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_mapping->action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCFI);
    }
    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_vlan_mapping_set_egs_default_action(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping, uint8 is_add)
{
    uint16 lport = 0;
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_default_action_t default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT) || CTC_IS_LINKAGG_PORT(gport))
    {
        /* default entry do not support logic port and linkagg port */
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    default_action.lport = lport;
    default_action.action_type = SYS_SCL_ACTION_EGRESS;
    default_action.field_action = &field_action;

    /* set pending status */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    if (is_add)
    {
        /* add action field */
        CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_egs_ad(lchip, p_vlan_mapping, 0, gport), ret, error0);
    }

    /* clear pending status and install to hw */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_usw_scl_set_default_action(lchip, &default_action);

    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    default_action.is_rollback = 1;
    sys_usw_scl_set_default_action(lchip, &default_action);
    return ret;
}


#define __VLAN_MAPPING_API__

/*lchip's configs are the same
0~0 means disable
vlan range cannot overlap*/
int32
sys_usw_vlan_create_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool is_svlan)
{
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(vrange_info);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    SYS_VLAN_MAPPING_LOCK(lchip);

    if (CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    { /*group created*/
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range already exist\n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_EXIST;

    }

    CTC_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS));

    if (is_svlan)
    {
        CTC_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_TYPE_BIT_POS));
    }
    else
    {
        CTC_BIT_UNSET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_TYPE_BIT_POS));
    }

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_destroy_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info)
{
    uint32 cmd = 0;

    uint8 direction;
    int32 vrange_grpid;

    DsVlanRangeProfile_m igs_ds;
    DsEgressVlanRangeProfile_m egs_ds;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(vrange_info);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    SYS_VLAN_MAPPING_LOCK(lchip);

    if (!CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range not exist\n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    direction = vrange_info->direction;
    vrange_grpid = vrange_info->vrange_grpid;

    sal_memset(&igs_ds, 0, sizeof(igs_ds));
    sal_memset(&egs_ds, 0, sizeof(egs_ds));


    if (CTC_INGRESS == direction)
    {
        cmd = DRV_IOW(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));

    }
    else
    {
        cmd = DRV_IOW(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));

    }

    SYS_VLAN_RANGE_SW_EACH_GROUP() = 0;

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*lchip's configs are the same
0~0 means disable
vlan range cannot overlap*/
int32
sys_usw_vlan_get_vlan_range_type(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool* is_svlan)
{
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_vlan_range_type(lchip, vrange_info, is_svlan));
    SYS_VLAN_MAPPING_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_vlan_add_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{
    uint32 cmd = 0;
    uint8  vlan_range_mode = 0;
    uint8  free_idx = 0;
    uint8  idx_start =0;
    uint8  idx_end = 0;
    uint8  xgpon_en = 0;
    DsVlanRangeProfile_m igs_ds;
    DsEgressVlanRangeProfile_m egs_ds;

    uint8 idx = 0;
    uint8 count = 0;
    uint8 direction;
    uint8 vrange_grpid;
    uint16 vlan_end;
    uint16 vlan_start;
    ctc_vlan_range_group_t vrange_group;

    CTC_PTR_VALID_CHECK(vrange_info);
    CTC_PTR_VALID_CHECK(vlan_range);

    vlan_start = vlan_range->vlan_start;
    vlan_end = vlan_range->vlan_end;
    direction = vrange_info->direction;
    vrange_grpid = vrange_info->vrange_grpid;

    sal_memset(&igs_ds, 0, sizeof(igs_ds));
    sal_memset(&egs_ds, 0, sizeof(egs_ds));
    sal_memset(&vrange_group, 0, sizeof(ctc_vlan_range_group_t));

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_VLAN_RANGE_CHECK(vlan_start);
    CTC_VLAN_RANGE_CHECK(vlan_end);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    SYS_VLAN_MAPPING_LOCK(lchip);
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);

    if (vlan_start >= vlan_end)
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] End vlan id should greater than start vlan id \n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_INVALID_CONFIG;

    }

    if (!CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range not exist\n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    /* get vlan range mode */
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_vlan_range_mode(lchip, direction, &vlan_range_mode));

    if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == vlan_range_mode)
    {
        idx_start = vlan_range->is_acl ? 4 : 0;
        idx_end = vlan_range->is_acl ? 8 : 4;
    }
    else
    {
        idx_start = 0;
        idx_end = 8;
    }

    for (free_idx = idx_start; free_idx < idx_end; free_idx++)
    {
        if (!IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), free_idx))
        {
            break;
        }
    }

    if (free_idx >= idx_end)
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping vlan range full\n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NO_RESOURCE;

    }

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_vlan_range_member(lchip, vrange_info, &vrange_group, &count));

    for (idx = 0; idx < count; idx++)
    {
        if ((vrange_group.vlan_range[idx].vlan_start == vlan_start) && (vlan_end == vrange_group.vlan_range[idx].vlan_end))
        {
            if (((CTC_GLOBAL_VLAN_RANGE_MODE_SHARE != vlan_range_mode) ||
                ((CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == vlan_range_mode) && (vrange_group.vlan_range[idx].is_acl == vlan_range->is_acl))) &&
                xgpon_en)
            {
                SYS_VLAN_RANGE_SW_EACH_MEMBER(idx)++;
                SYS_VLAN_MAPPING_UNLOCK(lchip);
                return CTC_E_NONE;
            }

        }

        if ((((vrange_group.vlan_range[idx].vlan_start <= vlan_start) && (vlan_start <= vrange_group.vlan_range[idx].vlan_end)) \
            || ((vrange_group.vlan_range[idx].vlan_start <= vlan_end) && (vlan_end <= vrange_group.vlan_range[idx].vlan_end))) \
            && (vrange_group.vlan_range[idx].is_acl == vlan_range->is_acl))
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping vlan range overlapped\n");
            SYS_VLAN_MAPPING_UNLOCK(lchip);
			return CTC_E_INVALID_CONFIG;

        }
    }

    if (CTC_INGRESS == direction)
    {
        cmd = DRV_IOR(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));

        DRV_SET_FIELD_V(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMin0_f + (free_idx* p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, vlan_start);
        DRV_SET_FIELD_V(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMax0_f + (free_idx* p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, vlan_end);

        cmd = DRV_IOW(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));
    }
    else
    {
        cmd = DRV_IOR(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));

        DRV_SET_FIELD_V(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMin0_f + (free_idx*p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, vlan_start);
        DRV_SET_FIELD_V(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMax0_f + (free_idx*p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, vlan_end);

        cmd = DRV_IOW(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));
    }

    CTC_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), free_idx);

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_vlan_remove_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{

    uint32 cmd = 0;
    uint8  del_idx = 0;
    uint8  idx_start =0;
    uint8  idx_end = 0;
    uint8 direction;
    uint8  vlan_range_mode = 0;
    uint8 xgpon_en = 0;
    int32 vrange_grpid;
    uint16 vlan_start;
    uint16 vlan_end;

    DsVlanRangeProfile_m         igs_ds;
    DsEgressVlanRangeProfile_m  egs_ds;

    sal_memset(&igs_ds, 0, sizeof(igs_ds));
    sal_memset(&egs_ds, 0, sizeof(egs_ds));

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(vrange_info);
    CTC_PTR_VALID_CHECK(vlan_range);
    vlan_start = vlan_range->vlan_start;
    vlan_end = vlan_range->vlan_end;
    CTC_VLAN_RANGE_CHECK(vlan_start);
    CTC_VLAN_RANGE_CHECK(vlan_end);
    SYS_VLAN_RANGE_INFO_CHECK(vrange_info);

    direction = vrange_info->direction;
    vrange_grpid = vrange_info->vrange_grpid;

    SYS_VLAN_MAPPING_LOCK(lchip);
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if (!CTC_IS_BIT_SET(SYS_VLAN_RANGE_SW_EACH_GROUP(), MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)))
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan range not exist\n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    if (vlan_start >= vlan_end)
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] End vlan id should greater than start vlan id \n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_INVALID_CONFIG;

    }

    /* get vlan range mode */
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_vlan_range_mode(lchip, direction, &vlan_range_mode));

    if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == vlan_range_mode)
    {
        idx_start = vlan_range->is_acl ? 4 : 0;
        idx_end = vlan_range->is_acl ? 8 : 4;
    }
    else
    {
        idx_start = 0;
        idx_end = 8;
    }

    if (CTC_INGRESS == direction)
    {
        cmd = DRV_IOR(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));

        for (del_idx = idx_start; del_idx < idx_end; del_idx++)
        {
            uint32 max;
            uint32 min;
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMax0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMin0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, &min);
            if ((vlan_end == max) && (vlan_start == min)
                && (max != min))
            {
                break;
            }
        }

        if (del_idx >= idx_end)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping get range failed\n");
            SYS_VLAN_MAPPING_UNLOCK(lchip);
			return CTC_E_INVALID_CONFIG;

        }

        if (SYS_VLAN_RANGE_SW_EACH_MEMBER(del_idx) && xgpon_en)
        {
            SYS_VLAN_RANGE_SW_EACH_MEMBER(del_idx)--;
            SYS_VLAN_MAPPING_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        DRV_SET_FIELD_V(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMax0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, 0);
        DRV_SET_FIELD_V(lchip, DsVlanRangeProfile_t, DsVlanRangeProfile_vlanMin0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->igs_step), &igs_ds, 0);

        cmd = DRV_IOW(DsVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &igs_ds));

    }
    else
    {
        cmd = DRV_IOR(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));

        for (del_idx = idx_start; del_idx < idx_end; del_idx++)
        {
            uint32 max;
            uint32 min;
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMax0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &max);
            DRV_GET_FIELD_A(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMin0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, &min);
            if ((vlan_end == max) && (vlan_start == min)
                && (max != min))
            {
                break;
            }
        }

        if (del_idx >= idx_end)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan mapping get range failed\n");
            SYS_VLAN_MAPPING_UNLOCK(lchip);
			return CTC_E_INVALID_CONFIG;

        }

        if (SYS_VLAN_RANGE_SW_EACH_MEMBER(del_idx) && xgpon_en)
        {
            SYS_VLAN_RANGE_SW_EACH_MEMBER(del_idx)--;
            SYS_VLAN_MAPPING_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        DRV_SET_FIELD_V(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMax0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, 0);
        DRV_SET_FIELD_V(lchip, DsEgressVlanRangeProfile_t, DsEgressVlanRangeProfile_vlanMin0_f + (del_idx * p_usw_vlan_mapping_master[lchip]->egs_step), &egs_ds, 0);

        cmd = DRV_IOW(DsEgressVlanRangeProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(DRV_IOCTL(lchip, vrange_grpid, cmd, &egs_ds));

    }

    CTC_BIT_UNSET(SYS_VLAN_RANGE_SW_EACH_GROUP(), del_idx);

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count)
{
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_vlan_range_member(lchip, vrange_info, vrange_group, count));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_maxvid_from_vrange(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range, bool is_svlan, uint16* p_max_vid)
{
    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_get_maxvid_from_vrange(lchip, vrange_info, vlan_range, is_svlan, p_max_vid));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_add_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_add_vlan_mapping(lchip, gport, p_vlan_mapping, FALSE));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_remove_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    ctc_scl_entry_t   scl;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t  field_key;
    int32             ret = CTC_E_NONE;
    uint8 is_logic_port = 0;
    uint16 lport = 0;
    uint32 gid = 0;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			    return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                      :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                  :%d\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                       :%X\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                       :%X\n", p_vlan_mapping->old_cvid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svlan end                  :%X\n", p_vlan_mapping->svlan_end);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvlan end                  :%X\n", p_vlan_mapping->cvlan_end);

    sal_memset(&scl, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY))
    {
        return _sys_usw_vlan_mapping_set_igs_default_action(lchip, gport, p_vlan_mapping, 0);
    }

    SYS_VLAN_MAPPING_MAP_SCL_KEY_TYPE_BY_KEY(p_vlan_mapping->key, scl.key_type);
    /* keep lock behide map key type */
    SYS_VLAN_MAPPING_LOCK(lchip);

    scl.action_type = SYS_SCL_ACTION_INGRESS;
    scl.resolve_conflict = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);

    /* for getting group id use */
    is_logic_port = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_INGRESS, lport, is_logic_port, scl.resolve_conflict, p_vlan_mapping->scl_id);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);

    lkup_key.key_type = scl.key_type;
    lkup_key.action_type = scl.action_type;
    lkup_key.group_priority = p_vlan_mapping->scl_id;
    lkup_key.resolve_conflict = scl.resolve_conflict;
    lkup_key.group_id = gid;

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_build_igs_key(lchip, gport, p_vlan_mapping, NULL, &lkup_key, 0));

    /* to sepcify, add entry when calculate key index will make valid bit and key common field included, so when remove as well */
    if (!scl.resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    scl.entry_id = lkup_key.entry_id;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_uninstall_entry(lchip, scl.entry_id, 1));
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_remove_entry(lchip, scl.entry_id, 1));

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_vlan_update_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_add_vlan_mapping(lchip, gport, p_vlan_mapping, TRUE));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    int32 ret = CTC_E_NONE;
    uint32  gid  = 0;
    uint16 lport = 0;

    sys_scl_lkup_key_t    lkup_key;
    ctc_field_key_t       field_key;


    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			    return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                     :0x%X\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                :0x%X\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " action                    :0x%X\n", p_vlan_mapping->action);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " flag                       :0x%X\n", p_vlan_mapping->flag);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                       :%d\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                       :%d\n", p_vlan_mapping->old_cvid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svlan end                :%d\n", p_vlan_mapping->svlan_end);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvlan end                :%d\n", p_vlan_mapping->cvlan_end);


    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_INGRESS, lport, \
                            CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT),\
                            CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX),p_vlan_mapping->scl_id);


    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);


    sal_memset(&lkup_key, 0 , sizeof(sys_scl_lkup_key_t));

    SYS_VLAN_MAPPING_MAP_SCL_KEY_TYPE_BY_KEY(p_vlan_mapping->key, lkup_key.key_type);

    SYS_VLAN_MAPPING_LOCK(lchip);
    lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
    lkup_key.group_priority = p_vlan_mapping->scl_id;
    lkup_key.resolve_conflict = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    lkup_key.group_id = gid;

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_build_igs_key(lchip, gport, p_vlan_mapping, NULL, &lkup_key, 0));

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Entry id by lookup key, entry id is 0x%x\n", lkup_key.entry_id);

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_unbuild_ad(lchip, lkup_key.entry_id, p_vlan_mapping));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_vlan_add_default_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_def_action)
{
    uint16 lport = 0;
    int32 ret = 0;
    sys_scl_default_action_t default_action;
    ctc_scl_field_action_t field_action;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                      :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " flag value                 :%d\n", p_def_action->flag);

    SYS_VLAN_MAPPING_INIT_CHECK();

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_PTR_VALID_CHECK(p_def_action);
    CTC_MAX_VALUE_CHECK(p_def_action->scl_id, 1);

    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    /* keep all the parameter check in the front and never move it */
    if (CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR == p_def_action->flag)
    {
        CTC_MAX_VALUE_CHECK(p_def_action->user_vlanptr, 0x1fff);
    }

    if (p_def_action->new_svid > CTC_MAX_VLAN_ID)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_def_action->new_scos >= CTC_MAX_COS)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_def_action->scos_sl >= CTC_VLAN_TAG_SL_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_VLAN_MAPPING_LOCK(lchip);

    default_action.scl_id = p_def_action->scl_id;
    default_action.action_type = SYS_SCL_ACTION_INGRESS;
    default_action.lport = lport;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    default_action.field_action = &field_action;

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &default_action));

    if(CTC_VLAN_MISS_ACTION_DO_NOTHING == p_def_action->flag)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret , error0);
    }
    else if(CTC_VLAN_MISS_ACTION_DISCARD == p_def_action->flag)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }
    else if(CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR == p_def_action->flag)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
        field_action.data0 = p_def_action->user_vlanptr;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }
    else if(CTC_VLAN_MISS_ACTION_TO_CPU == p_def_action->flag)
    {
        /* copy to cpu and discard */
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }
    else if(CTC_VLAN_MISS_ACTION_APPEND_STAG == p_def_action->flag)
    {
        ctc_scl_vlan_edit_t vlan_edit;
        sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));
        vlan_edit.svid_new = p_def_action->new_svid;
        vlan_edit.stag_op = CTC_VLAN_TAG_OP_ADD;
        vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.scos_new =  p_def_action->new_scos;
        vlan_edit.scos_sl =  p_def_action->scos_sl;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_usw_scl_set_default_action(lchip, &default_action);

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    default_action.is_rollback = 1;
    sys_usw_scl_set_default_action(lchip, &default_action);
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_vlan_remove_default_vlan_mapping(uint8 lchip, uint32 gport)
{

    uint16 lport = 0;
    uint8 index = 0;
    sys_scl_default_action_t def_action;
    ctc_scl_field_action_t   field_action;

    sal_memset(&def_action, 0, sizeof(sys_scl_default_action_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_VLAN_MAPPING_LOCK(lchip);
    def_action.action_type = SYS_SCL_ACTION_INGRESS;
    def_action.lport = lport;
    def_action.field_action = &field_action;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        def_action.scl_id = index;
        field_action.data0 = 1;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &def_action));

        field_action.data0 = 0;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &def_action));
    }
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_vlan_remove_all_vlan_mapping_by_port(uint8 lchip, uint32 gport)
{
    uint32 gid = 0;
    int32 ret = 0;
    uint16 lport = 0;
    uint8 index = 0;

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* check chip is local */
    SYS_GLOBAL_PORT_CHECK(gport);
    if (!CTC_IS_LINKAGG_PORT(gport))
    {
        ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
        if (FALSE == ret)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

        }
    }
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    SYS_VLAN_MAPPING_LOCK(lchip);
    /* ingress hash group */
    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_INGRESS, lport, 0, 0, index);
        ret = sys_usw_scl_uninstall_group(lchip, gid, 1);
        if (CTC_E_NONE == ret)
        {
            CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_remove_all_entry(lchip, gid, 1));
        }
        else if (CTC_E_NOT_EXIST == ret)
        {

        }
        else
        {
            SYS_VLAN_MAPPING_UNLOCK(lchip);
            return ret;
        }
    }

    /* tcam group for resolve conflict */
    for (index=0; index<2; index+=1)
    {
        gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_INGRESS, lport, 0, 1, index);
        ret = sys_usw_scl_uninstall_group(lchip, gid, 1);
        if (CTC_E_NONE == ret)
        {
            CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_remove_all_entry(lchip, gid, 1));
        }
        else if (CTC_E_NOT_EXIST == ret)
        {

        }
        else
        {
            SYS_VLAN_MAPPING_UNLOCK(lchip);
            return ret;
        }
    }

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_add_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    int32             ret = CTC_E_NONE;
    uint32            gid = 0;
    uint16          lport = 0;
    uint8   is_logic_port = 0;
    ctc_scl_entry_t   scl;
    ctc_field_key_t field_key;
    sys_vlan_mapping_tag_op_t vlan_mapping_op;
    ctc_scl_group_info_t group;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);
    CTC_MAX_VALUE_CHECK(p_vlan_mapping->tpid_index, 3);
    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			    return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                     :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                 :%d\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " action                    :0x%X\n", p_vlan_mapping->action);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                      :0x%X\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                      :0x%X\n", p_vlan_mapping->old_cvid);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "-------mapping to-----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " new svid                  :%d\n", p_vlan_mapping->new_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " new cvid                  :%d\n", p_vlan_mapping->new_cvid);

    /*egress action check missed..*/
    sal_memset(&scl, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&vlan_mapping_op, 0, sizeof(sys_vlan_mapping_tag_op_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));

    vlan_mapping_op.ccos_sl = p_vlan_mapping->ccos_sl;
    vlan_mapping_op.cvid_sl = p_vlan_mapping->cvid_sl;
    vlan_mapping_op.ctag_op = p_vlan_mapping->ctag_op;
    vlan_mapping_op.scos_sl = p_vlan_mapping->scos_sl;
    vlan_mapping_op.svid_sl = p_vlan_mapping->svid_sl;
    vlan_mapping_op.stag_op = p_vlan_mapping->stag_op;
    CTC_ERROR_RETURN(_sys_usw_vlan_check_vlan_mapping_action(lchip, CTC_EGRESS, &vlan_mapping_op));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY))
    {
        return _sys_usw_vlan_mapping_set_egs_default_action(lchip, gport, p_vlan_mapping, 1);
    }

    SYS_VLAN_MAPPING_LOCK(lchip);

    /* first step, add a entry */
    switch (p_vlan_mapping->key
            & (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID
               | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS))
    {
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_CVLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_SVLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_2VLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_CVLAN_COS;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_SVLAN_COS;
        break;
    case CTC_EGRESS_VLAN_MAPPING_KEY_NONE:
        scl.key_type = SYS_SCL_KEY_HASH_PORT;
        break;
    default:
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NOT_SUPPORT;

        break;
    }
    scl.action_type = SYS_SCL_ACTION_EGRESS;
    scl.resolve_conflict = 0;

    /* for getting group id use */
    is_logic_port = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_EGRESS, lport, is_logic_port, scl.resolve_conflict, 0);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);

    group.type = CTC_SCL_GROUP_TYPE_NONE;
    /* use default priority for inter module */
    ret = sys_usw_scl_create_group(lchip, gid, &group, 1);
    if ((ret < 0 ) && (ret != CTC_E_EXIST))
    {
        SYS_VLAN_MAPPING_UNLOCK(lchip);
        return ret;
    }

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_add_entry(lchip, gid, &scl, 1));

    CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_egs_key(lchip, gport, p_vlan_mapping, &scl, NULL, 1), ret, error0);

    if (!scl.resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl.entry_id, &field_key), ret, error0);
    }

    CTC_ERROR_GOTO(_sys_usw_vlan_mapping_build_egs_ad(lchip, p_vlan_mapping, scl.entry_id, gport), ret, error0);

    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl.entry_id, 1), ret, error0);

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    sys_usw_scl_remove_entry(lchip, scl.entry_id, 1);
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_vlan_remove_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    int32             ret = CTC_E_NONE;
    ctc_scl_entry_t   scl;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    uint8 is_logic_port = 0;
    uint16 lport = 0;
    uint32 gid = 0;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			    return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                      :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                  :%d\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                       :%X\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                       :%X\n", p_vlan_mapping->old_cvid);

    sal_memset(&scl, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY))
    {
        return _sys_usw_vlan_mapping_set_egs_default_action(lchip, gport, p_vlan_mapping, 0);
    }

    SYS_VLAN_MAPPING_LOCK(lchip);

    switch (p_vlan_mapping->key
            & (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID
               | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS))
    {
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_CVLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_SVLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_2VLAN;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_CVLAN_COS;
        break;
    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
        scl.key_type = SYS_SCL_KEY_HASH_PORT_SVLAN_COS;
        break;
    case CTC_EGRESS_VLAN_MAPPING_KEY_NONE:
        scl.key_type = SYS_SCL_KEY_HASH_PORT;
        break;
    default:
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        SYS_VLAN_MAPPING_UNLOCK(lchip);
		return CTC_E_NOT_SUPPORT;

        break;
    }
    scl.action_type = SYS_SCL_ACTION_EGRESS;
    scl.resolve_conflict = 0;

    /* for getting group id use */
    is_logic_port = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_EGRESS, lport, is_logic_port, scl.resolve_conflict, 0);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);

    lkup_key.key_type = scl.key_type;
    lkup_key.action_type = scl.action_type;
    lkup_key.group_priority = 0;
    lkup_key.resolve_conflict = 0;
    lkup_key.group_id = gid;

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_build_egs_key(lchip, gport, p_vlan_mapping, NULL, &lkup_key, 0));

    if (!scl.resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

    scl.entry_id = lkup_key.entry_id;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_uninstall_entry(lchip, scl.entry_id, 1));
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_remove_entry(lchip, scl.entry_id, 1));

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_vlan_get_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t*  p_vlan_mapping)
{
    int32 ret = CTC_E_NONE;
    uint32  gid  = 0;
    uint16 lport = 0;

    sys_scl_lkup_key_t    lkup_key;
    ctc_field_key_t       field_key;


    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_MAPPING_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT))
    {
        SYS_GLOBAL_PORT_CHECK(gport);
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
            if (FALSE == ret)
            {
                SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			    return CTC_E_INVALID_CHIP_ID;

            }
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(gport, SYS_VLAN_MAPPING_MAX_LOGIC_PORT_NUM);
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                     :0x%X\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " key value                :0x%X\n", p_vlan_mapping->key);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " action                    :0x%X\n", p_vlan_mapping->action);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " flag                       :0x%X\n", p_vlan_mapping->flag);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svid                       :%d\n", p_vlan_mapping->old_svid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvid                       :%d\n", p_vlan_mapping->old_cvid);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " svlan end                :%d\n", p_vlan_mapping->svlan_end);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " cvlan end                :%d\n", p_vlan_mapping->cvlan_end);


    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_EGRESS, lport, \
                            CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT),\
                            CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX),0);


    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " group id :%u\n", gid);


    sal_memset(&lkup_key, 0 , sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(field_key));
    SYS_VLAN_MAPPING_MAP_SCL_KEY_TYPE_BY_KEY(p_vlan_mapping->key, lkup_key.key_type);
    SYS_VLAN_MAPPING_LOCK(lchip);
    lkup_key.action_type = SYS_SCL_ACTION_EGRESS;
    lkup_key.group_priority = 0;
    lkup_key.resolve_conflict = CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    lkup_key.group_id = gid;

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_build_egs_key(lchip, gport, p_vlan_mapping, NULL, &lkup_key, 0));

    if (!CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Entry id by lookup key, entry id is 0x%x\n", lkup_key.entry_id);

    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(_sys_usw_vlan_mapping_unbuild_egs_ad(lchip, p_vlan_mapping, lkup_key.entry_id));
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return ret;
}


int32
sys_usw_vlan_add_default_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_def_action)
{
    uint16 lport = 0;
    int32 ret = 0;
    sys_scl_default_action_t default_action;
    ctc_scl_field_action_t   field_action;

    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "------vlan mapping----------\n");
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " gport                      :%d\n", gport);
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " flag value                 :%d\n", p_def_action->flag);

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_PTR_VALID_CHECK(p_def_action);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_VLAN_MAPPING_LOCK(lchip);
    default_action.action_type = SYS_SCL_ACTION_EGRESS;
    default_action.lport = lport;
    default_action.field_action = &field_action;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &default_action));

    if (CTC_VLAN_MISS_ACTION_DO_NOTHING == p_def_action->flag)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }
    else if (CTC_VLAN_MISS_ACTION_DISCARD == p_def_action->flag)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &default_action), ret, error0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_usw_scl_set_default_action(lchip, &default_action);

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    default_action.is_rollback = 1;
    sys_usw_scl_set_default_action(lchip, &default_action);
    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_vlan_remove_default_egress_vlan_mapping(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    sys_scl_default_action_t def_action;
    ctc_scl_field_action_t   field_action;

    sal_memset(&def_action, 0, sizeof(sys_scl_default_action_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_VLAN_MAPPING_LOCK(lchip);
    def_action.action_type = SYS_SCL_ACTION_EGRESS;
    def_action.lport = lport;
    def_action.field_action = &field_action;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &def_action));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_set_default_action(lchip, &def_action));

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_remove_all_egress_vlan_mapping_by_port(uint8 lchip, uint32 gport)
{
    uint32 gid = 0;
    int32 ret = 0;
    uint16 lport = 0;

    SYS_VLAN_MAPPING_INIT_CHECK();
    SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* check chip is local */
    SYS_GLOBAL_PORT_CHECK(gport);
    if (!CTC_IS_LINKAGG_PORT(gport))
    {
        ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
        if (FALSE == ret)
        {
            SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

        }
    }
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    SYS_VLAN_MAPPING_LOCK(lchip);
    /* egress hash group */
    gid = _sys_usw_vlan_mapping_get_scl_gid(lchip, CTC_EGRESS, lport, 0, 0, 0);
    ret = sys_usw_scl_uninstall_group(lchip, gid, 1);
    if (CTC_E_NONE == ret)
    {
        CTC_ERROR_RETURN_VLAN_MAPPING_UNLOCK(sys_usw_scl_remove_all_entry(lchip, gid, 1));
    }
    else if (CTC_E_NOT_EXIST == ret)
    {

    }
    else
    {
        SYS_VLAN_MAPPING_UNLOCK(lchip);
        return ret;
    }

    SYS_VLAN_MAPPING_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_mapping_get_master(uint8 lchip, sys_vlan_mapping_master_t** p_vlan_mapping_master)
{
    SYS_VLAN_MAPPING_INIT_CHECK();

    *p_vlan_mapping_master = p_usw_vlan_mapping_master[lchip];

    return CTC_E_NONE;
}

int32
sys_usw_vlan_mapping_init(uint8 lchip)
{
    ctc_scl_group_info_t group;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
	}
    LCHIP_CHECK(lchip);

    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));

    if (NULL != p_usw_vlan_mapping_master[lchip])
    {
        return CTC_E_NONE;
    }

    MALLOC_ZERO(MEM_VLAN_MODULE, p_usw_vlan_mapping_master[lchip], sizeof(sys_vlan_mapping_master_t));

    if (NULL == p_usw_vlan_mapping_master[lchip])
    {
        SYS_VLAN_MAPPING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    p_usw_vlan_mapping_master[lchip]->igs_step = DsVlanRangeProfile_vlanMax1_f - DsVlanRangeProfile_vlanMax0_f ;
    p_usw_vlan_mapping_master[lchip]->egs_step = DsEgressVlanRangeProfile_vlanMax1_f - DsEgressVlanRangeProfile_vlanMax0_f ;

    SYS_VLAN_MAPPING_CREATE_LOCK(lchip);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
	}
    return CTC_E_NONE;
}

int32
sys_usw_vlan_mapping_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_vlan_mapping_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(p_usw_vlan_mapping_master[lchip]->mutex);
    mem_free(p_usw_vlan_mapping_master[lchip]);

    return CTC_E_NONE;
}


