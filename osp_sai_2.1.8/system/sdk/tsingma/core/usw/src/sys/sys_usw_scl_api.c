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
#include "ctc_acl.h"
#include "ctc_security.h"

#include "sys_usw_fpa.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_register.h"
#include "sys_usw_qos_api.h"

#include "sys_usw_scl.h"
#include "sys_usw_scl_api.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

#define SYS_USW_SCL_MAX_KEY_SIZE  40
#define SYS_SCL_MAP_DRV_AD_INDEX(index, step)  ((1 == (step)) ? (index) : ((index) / 2))

extern int32
_sys_usw_scl_unbind_nexthop(uint8 lchip, sys_scl_sw_entry_t* pe, uint32 nh_id);

#define _________api_internal_function__________

/*
 * get sys group node by group id
 */
int32
_sys_usw_scl_get_group_by_gid(uint8 lchip, uint32 gid, sys_scl_sw_group_t** pg)
{
    sys_scl_sw_group_t* pg_lkup = NULL;
    sys_scl_sw_group_t  sys_group;

    CTC_PTR_VALID_CHECK(pg);

    sal_memset(&sys_group, 0, sizeof(sys_scl_sw_group_t));
    sys_group.group_id = gid;

    pg_lkup = ctc_hash_lookup(p_usw_scl_master[lchip]->group, &sys_group);

    *pg = pg_lkup;

    return CTC_E_NONE;
}

int32
_sys_usw_scl_get_nodes_by_eid(uint8 lchip, uint32 eid, sys_scl_sw_entry_t** pe)
{
    sys_scl_sw_entry_t  sys_entry;
    sys_scl_sw_entry_t* pe_lkup = NULL;

    CTC_PTR_VALID_CHECK(pe);

    sal_memset(&sys_entry, 0, sizeof(sys_scl_sw_entry_t));
    sys_entry.entry_id = eid;
    pe_lkup = ctc_hash_lookup(p_usw_scl_master[lchip]->entry, &sys_entry);

    *pe = pe_lkup;

    return CTC_E_NONE;
}



int32
_sys_usw_scl_get_index(uint8 lchip, uint32 key_id, sys_scl_sw_entry_t* pe, uint32* o_key_index, uint32* o_ad_index)
{
    uint8  step      = 0;
    uint32 key_index = 0;
    uint32 ad_index  = 0;

    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))   /*tcam entry*/
    {
        step = SYS_SCL_GET_STEP(p_usw_scl_master[lchip]->fpa_size[pe->key_type]);
        key_index = pe->key_index / step;
        ad_index  = SYS_SCL_MAP_DRV_AD_INDEX(pe->key_index, step);
        if ((key_id == DsUserId0TcamKey80_t) || (key_id == DsUserId1TcamKey80_t))
        {
            key_index = key_index * 2;
        }
    }
    else    /*hash*/
    {
        key_index = pe->key_index;
        ad_index  = pe->ad_index;
    }

    if (o_key_index)
    {
        *o_key_index = key_index;
    }
    if (o_ad_index)
    {
        *o_ad_index = ad_index;
    }

    return CTC_E_NONE;
}




int32
_sys_usw_scl_get_table_id(uint8 lchip, uint8 block_id, sys_scl_sw_entry_t* pe,
                                 uint32* key_id, uint32* act_id)
{
    uint8 ad_distance = (DsUserId1Tcam_t - DsUserId0Tcam_t) * block_id;
    uint8 dir      = 0;
    uint8 key_type = 0;
    uint8 act_type = 0;
    uint8 is_half = 0;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(key_id);
    CTC_PTR_VALID_CHECK(act_id);

    key_type = pe->key_type;
    act_type = pe->action_type;
    is_half = pe->is_half;

    *key_id = 0;
    *act_id = 0;

    switch (act_type)
    {
    case SYS_SCL_ACTION_INGRESS:
        switch(block_id)
        {
            case 0:
                *act_id = is_half? DsUserIdHalf_t : DsUserId_t;
                break;
            case 1:
                *act_id = is_half? DsUserIdHalf1_t : DsUserId1_t;
                if (1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM))
                {
                    *act_id = is_half? DsUserIdHalf_t : DsUserId_t;
                }
                break;
            case 2:
                *act_id = DsUserId2_t;
                break;
            case 3:
                *act_id = DsUserId3_t;
                break;
            default :
                break;
        }
        break;
    case SYS_SCL_ACTION_EGRESS:
        *act_id = 0xFFFFFFFF; /*invalid*/
        dir     = 1;
        break;
    case SYS_SCL_ACTION_TUNNEL:
        if (0 == block_id)
        {
            *act_id = is_half? DsTunnelIdHalf_t: DsTunnelId_t;
        }
        else if (1 == block_id)
        {
            if (1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM))
            {
                *act_id = is_half? DsTunnelIdHalf_t: DsTunnelId_t;
            }
            else
            {
                *act_id = is_half? DsTunnelIdHalf1_t: DsTunnelId1_t;
            }
        }
        break;
    case SYS_SCL_ACTION_FLOW:
        if (0 == block_id)
        {
            *act_id = DsSclFlow_t;
        }
        else if (1 == block_id)
        {
            if (1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM))
            {
                *act_id = DsSclFlow_t;
            }
            else
            {
                *act_id = DsSclFlow1_t;
            }
        }
        else if (2 == block_id)
        {
            *act_id = DsSclFlow2Tcam_t;
        }
        else if (3 == block_id)
        {
            *act_id = DsSclFlow3Tcam_t;
        }
        break;
    case SYS_SCL_ACTION_MPLS:
        *act_id = DsUserId1Tcam_t;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if (pe->resolve_conflict)
    {
        switch (key_type)
        {
        case SYS_SCL_KEY_HASH_PORT_2VLAN:
        case SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP:
        case SYS_SCL_KEY_HASH_PORT_CVLAN:
        case SYS_SCL_KEY_HASH_PORT_SVLAN:
        case SYS_SCL_KEY_HASH_PORT:
        case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
        case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
        case SYS_SCL_KEY_HASH_MAC:
        case SYS_SCL_KEY_HASH_PORT_MAC:
        case SYS_SCL_KEY_HASH_IPV4:
        case SYS_SCL_KEY_HASH_SVLAN_MAC:
        case SYS_SCL_KEY_HASH_SVLAN:
        case SYS_SCL_KEY_HASH_PORT_IPV4:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
        case SYS_SCL_KEY_HASH_WLAN_RMAC:
        case SYS_SCL_KEY_HASH_WLAN_RMAC_RID:
        case SYS_SCL_KEY_HASH_WLAN_STA_STATUS:
        case SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS:
        case SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD:
        case SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD:
        case SYS_SCL_KEY_HASH_TUNNEL_MPLS:
        case SYS_SCL_KEY_HASH_TRILL_UC:
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
        case SYS_SCL_KEY_HASH_TRILL_MC:
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            *key_id = block_id ? DsUserId1TcamKey80_t : DsUserId0TcamKey80_t;
            *act_id = DsUserId0Tcam_t + ad_distance;
            break;

        /*160*/
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
        case SYS_SCL_KEY_HASH_IPV6:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
        case SYS_SCL_KEY_HASH_L2:
            *key_id = block_id ? DsUserId1TcamKey160_t : DsUserId0TcamKey160_t;
            *act_id = DsUserId0Tcam_t + ad_distance;
            break;

        /*320*/
        case SYS_SCL_KEY_HASH_PORT_IPV6:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
            *key_id = block_id ? DsUserId1TcamKey320_t : DsUserId0TcamKey320_t;
            *act_id = DsUserId0Tcam_t + ad_distance;
            break;

        /*640*/
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
            *key_id = block_id ? DsUserId1TcamKey640_t : DsUserId0TcamKey640_t;
            *act_id = DsUserId0Tcam_t + ad_distance;
            break;
        case SYS_SCL_KEY_TCAM_MAC:
        /*case SYS_SCL_KEY_TCAM_VLAN:*/  /*not support*/
        case SYS_SCL_KEY_TCAM_IPV4:
        case SYS_SCL_KEY_TCAM_IPV4_SINGLE:
        case SYS_SCL_KEY_TCAM_IPV6:
        case SYS_SCL_KEY_TCAM_IPV6_SINGLE:

        /*BPE ecid not support TCAM*/
        case SYS_SCL_KEY_HASH_ECID:
        case SYS_SCL_KEY_HASH_ING_ECID:

        case SYS_SCL_KEY_HASH_PORT_CROSS:
        case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:

        case SYS_SCL_KEY_PORT_DEFAULT_INGRESS:
        case SYS_SCL_KEY_PORT_DEFAULT_EGRESS:
        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        uint8 scl_id = (1 == MCHIP_CAP(SYS_CAP_SCL_HASH_NUM)? 0: block_id);      /* used for hash */
        switch (key_type)
        {
        case SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP:
            *key_id = scl_id ? DsUserId1DoubleVlanPortHashKey_t : DsUserIdDoubleVlanPortHashKey_t;
            break;
        case SYS_SCL_KEY_HASH_PORT_2VLAN:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1DoubleVlanPortHashKey_t : DsUserIdDoubleVlanPortHashKey_t) : DsEgressXcOamDoubleVlanPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_PORT:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1PortHashKey_t : DsUserIdPortHashKey_t) : DsEgressXcOamPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_PORT_CVLAN:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1CvlanPortHashKey_t : DsUserIdCvlanPortHashKey_t) : DsEgressXcOamCvlanPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_PORT_SVLAN:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1SvlanPortHashKey_t : DsUserIdSvlanPortHashKey_t) : DsEgressXcOamSvlanPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1CvlanCosPortHashKey_t : DsUserIdCvlanCosPortHashKey_t) : DsEgressXcOamCvlanCosPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
            *key_id = (0 == dir) ? (scl_id ? DsUserId1SvlanCosPortHashKey_t : DsUserIdSvlanCosPortHashKey_t) : DsEgressXcOamSvlanCosPortHashKey_t;
            break;

        case SYS_SCL_KEY_HASH_MAC:
            *key_id = (scl_id ? DsUserId1MacHashKey_t : DsUserIdMacHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_PORT_MAC:
            *key_id = (scl_id ? DsUserId1MacPortHashKey_t : DsUserIdMacPortHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_IPV4:
            *key_id = (scl_id ? DsUserId1Ipv4SaHashKey_t : DsUserIdIpv4SaHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_SVLAN:
            *key_id = (scl_id ? DsUserId1SvlanHashKey_t : DsUserIdSvlanHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_SVLAN_MAC:
            *key_id = (scl_id ? DsUserId1SvlanMacSaHashKey_t : DsUserIdSvlanMacSaHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_PORT_IPV4:
            *key_id = (scl_id ? DsUserId1Ipv4PortHashKey_t : DsUserIdIpv4PortHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_IPV6:
            *key_id = (scl_id ? DsUserId1Ipv6SaHashKey_t : DsUserIdIpv6SaHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_PORT_IPV6:
            *key_id = (scl_id ? DsUserId1Ipv6PortHashKey_t : DsUserIdIpv6PortHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_L2:
            *key_id = (scl_id ? DsUserId1SclFlowL2HashKey_t : DsUserIdSclFlowL2HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
            *key_id = (scl_id ? DsUserId1TunnelIpv4HashKey_t : DsUserIdTunnelIpv4HashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
            *key_id = (scl_id ? DsUserId1TunnelIpv4DaHashKey_t : DsUserIdTunnelIpv4DaHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
            *key_id = (scl_id ? DsUserId1TunnelIpv4GreKeyHashKey_t : DsUserIdTunnelIpv4GreKeyHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
            *key_id = (scl_id ? DsUserId1TunnelIpv4RpfHashKey_t : DsUserIdTunnelIpv4RpfHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
            *key_id = (scl_id ? DsUserId1TunnelIpv6HashKey_t : DsUserIdTunnelIpv6HashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
            *key_id = (scl_id ? DsUserId1TunnelIpv6DaHashKey_t : DsUserIdTunnelIpv6DaHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
            *key_id = (scl_id ? DsUserId1TunnelIpv6GreKeyHashKey_t : DsUserIdTunnelIpv6GreKeyHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
            *key_id = (scl_id ? DsUserId1TunnelIpv6UdpHashKey_t : DsUserIdTunnelIpv6UdpHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv4UcNvgreMode0HashKey_t : DsUserIdTunnelIpv4UcNvgreMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv4UcNvgreMode1HashKey_t : DsUserIdTunnelIpv4UcNvgreMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv4McNvgreMode0HashKey_t : DsUserIdTunnelIpv4McNvgreMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv4NvgreMode1HashKey_t : DsUserIdTunnelIpv4NvgreMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv6UcNvgreMode0HashKey_t : DsUserIdTunnelIpv6UcNvgreMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv6UcNvgreMode1HashKey_t : DsUserIdTunnelIpv6UcNvgreMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv6McNvgreMode0HashKey_t : DsUserIdTunnelIpv6McNvgreMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv6McNvgreMode1HashKey_t : DsUserIdTunnelIpv6McNvgreMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv4UcVxlanMode0HashKey_t : DsUserIdTunnelIpv4UcVxlanMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv4UcVxlanMode1HashKey_t : DsUserIdTunnelIpv4UcVxlanMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv4McVxlanMode0HashKey_t : DsUserIdTunnelIpv4McVxlanMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv4VxlanMode1HashKey_t : DsUserIdTunnelIpv4VxlanMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv6UcVxlanMode0HashKey_t : DsUserIdTunnelIpv6UcVxlanMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv6UcVxlanMode1HashKey_t : DsUserIdTunnelIpv6UcVxlanMode1HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
            *key_id = (scl_id ? DsUserId1TunnelIpv6McVxlanMode0HashKey_t : DsUserIdTunnelIpv6McVxlanMode0HashKey_t);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
            *key_id = (scl_id ? DsUserId1TunnelIpv6McVxlanMode1HashKey_t : DsUserIdTunnelIpv6McVxlanMode1HashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
            *key_id = (scl_id ? DsUserId1TunnelIpv4CapwapHashKey_t : DsUserIdTunnelIpv4CapwapHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
            *key_id = (scl_id ? DsUserId1TunnelIpv6CapwapHashKey_t : DsUserIdTunnelIpv6CapwapHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_RMAC:
            *key_id = (scl_id ? DsUserId1TunnelCapwapRmacHashKey_t : DsUserIdTunnelCapwapRmacHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_RMAC_RID:
            *key_id = (scl_id ? DsUserId1TunnelCapwapRmacRidHashKey_t : DsUserIdTunnelCapwapRmacRidHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_STA_STATUS:
            *key_id = (scl_id ? DsUserId1CapwapStaStatusHashKey_t : DsUserIdCapwapStaStatusHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS:
            *key_id = (scl_id ? DsUserId1CapwapStaStatusMcHashKey_t : DsUserIdCapwapStaStatusMcHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD:
            *key_id = (scl_id ? DsUserId1CapwapVlanForwardHashKey_t : DsUserIdCapwapVlanForwardHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD:
            *key_id = (scl_id ? DsUserId1CapwapMacDaForwardHashKey_t : DsUserIdCapwapMacDaForwardHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_MPLS:
            *key_id = MplsHashKey_t;
            break;
        case SYS_SCL_KEY_HASH_TRILL_UC:
            *key_id = (scl_id ? DsUserId1TunnelTrillUcDecapHashKey_t : DsUserIdTunnelTrillUcDecapHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
            *key_id = (scl_id ? DsUserId1TunnelTrillUcRpfHashKey_t : DsUserIdTunnelTrillUcRpfHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC:
            *key_id = (scl_id ? DsUserId1TunnelTrillMcDecapHashKey_t : DsUserIdTunnelTrillMcDecapHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
            *key_id = (scl_id ? DsUserId1TunnelTrillMcRpfHashKey_t : DsUserIdTunnelTrillMcRpfHashKey_t);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            *key_id = (scl_id ? DsUserId1TunnelTrillMcAdjHashKey_t : DsUserIdTunnelTrillMcAdjHashKey_t);
            break;
        case SYS_SCL_KEY_TCAM_MAC:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0MacKey160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1MacKey160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2MacKey160_t;
                    break;
                case 3:
                    *key_id = DsScl3MacKey160_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_VLAN:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0MacL3Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1MacL3Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2MacL3Key320_t;
                    break;
                case 3:
                    *key_id = DsScl3MacL3Key320_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_IPV4:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0MacL3Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1MacL3Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2MacL3Key320_t;
                    break;
                case 3:
                    *key_id = DsScl3MacL3Key320_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_IPV4_SINGLE:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0L3Key160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1L3Key160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2L3Key160_t;
                    break;
                case 3:
                    *key_id = DsScl3L3Key160_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_IPV6:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0MacIpv6Key640_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1MacIpv6Key640_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2MacIpv6Key640_t;
                    break;
                case 3:
                    *key_id = DsScl3MacIpv6Key640_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_IPV6_SINGLE:
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0Ipv6Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1Ipv6Key320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2Ipv6Key320_t;
                    break;
                case 3:
                    *key_id = DsScl3Ipv6Key320_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_UDF :
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0UdfKey160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1UdfKey160_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2UdfKey160_t;
                    break;
                case 3:
                    *key_id = DsScl3UdfKey160_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_TCAM_UDF_EXT :
            switch(block_id)
            {
                case 0:
                    *key_id = DsScl0UdfKey320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 1:
                    *key_id = DsScl1UdfKey320_t;
                    *act_id = DsUserId0Tcam_t + ad_distance;
                    break;
                case 2:
                    *key_id = DsScl2UdfKey320_t;
                    break;
                case 3:
                    *key_id = DsScl3UdfKey320_t;
                    break;
                default :
                    break;
            }
            break;

        case SYS_SCL_KEY_HASH_ECID:
            *key_id = (scl_id ? DsUserId1EcidNameSpaceHashKey_t : DsUserIdEcidNameSpaceHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_ING_ECID:
            *key_id = (scl_id ? DsUserId1IngEcidNameSpaceHashKey_t : DsUserIdIngEcidNameSpaceHashKey_t);
            break;

        case SYS_SCL_KEY_HASH_PORT_CROSS:
            *key_id = DsEgressXcOamPortCrossHashKey_t;
            break;
        case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:
            *key_id = DsEgressXcOamPortVlanCrossHashKey_t;
            break;

        case SYS_SCL_KEY_PORT_DEFAULT_INGRESS:
            *act_id = (scl_id ? DsUserId1_t : DsUserId_t);
            break;
        case SYS_SCL_KEY_PORT_DEFAULT_EGRESS:
            *act_id = DsVlanXlateDefault_t;
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_set_hw_hash_valid(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint32               pu32[MAX_ENTRY_WORD]       = { 0 };
    uint32               key_id;
    uint32               act_id;
    void                 * p_key_void = pu32;

   /* -- set hw valid bit  -- */

    switch (pe->key_type)
    {
        case SYS_SCL_KEY_HASH_PORT_2VLAN:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdDoubleVlanPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamDoubleVlanPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP:
            SetDsUserIdDoubleVlanPortHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_PORT:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_PORT_CVLAN:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdCvlanPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamCvlanPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_PORT_SVLAN:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdSvlanPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamSvlanPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdCvlanCosPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamCvlanCosPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
            if(CTC_INGRESS == pe->direction)
            {
                SetDsUserIdSvlanCosPortHashKey(V, valid_f, pu32, 1);
            }
            else
            {
                SetDsEgressXcOamSvlanCosPortHashKey(V, valid_f, pu32, 1);
            }
            break;

        case SYS_SCL_KEY_HASH_MAC:
            SetDsUserIdMacHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_PORT_MAC:
            SetDsUserIdMacPortHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdMacPortHashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_IPV4:
            SetDsUserIdIpv4SaHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_SVLAN:
            SetDsUserIdSvlanHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_SVLAN_MAC:
            SetDsUserIdSvlanMacSaHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_PORT_IPV4:
            SetDsUserIdIpv4PortHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_IPV6:
            SetDsUserIdIpv6SaHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdIpv6SaHashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_PORT_IPV6:
            SetDsUserIdIpv6PortHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdIpv6PortHashKey(V, valid1_f, pu32, 1);
            SetDsUserIdIpv6PortHashKey(V, valid2_f, pu32, 1);
            SetDsUserIdIpv6PortHashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_L2:
            SetDsUserIdSclFlowL2HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdSclFlowL2HashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
            SetDsUserIdTunnelIpv4HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4HashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
            SetDsUserIdTunnelIpv4DaHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4GreKeyHashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
            SetDsUserIdTunnelIpv6HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
            SetDsUserIdTunnelIpv6DaHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6DaHashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6GreKeyHashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6UdpHashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
            SetDsUserIdTunnelIpv4RpfHashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
            SetDsUserIdTunnelIpv4UcNvgreMode0HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
            SetDsUserIdTunnelIpv4UcNvgreMode1HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
            SetDsUserIdTunnelIpv4McNvgreMode0HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4NvgreMode1HashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode0HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcNvgreMode1HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode0HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6McNvgreMode1HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
            SetDsUserIdTunnelIpv4UcVxlanMode0HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
            SetDsUserIdTunnelIpv4UcVxlanMode1HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
            SetDsUserIdTunnelIpv4McVxlanMode0HashKey(V, valid_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4VxlanMode1HashKey(V, valid1_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode0HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6UcVxlanMode1HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode0HashKey(V, valid3_f, pu32, 1);
            break;

        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6McVxlanMode1HashKey(V, valid3_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
            SetDsUserIdTunnelIpv4CapwapHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv4CapwapHashKey(V, valid1_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
            SetDsUserIdTunnelIpv6CapwapHashKey(V, valid0_f, pu32, 1);
            SetDsUserIdTunnelIpv6CapwapHashKey(V, valid1_f, pu32, 1);
            SetDsUserIdTunnelIpv6CapwapHashKey(V, valid2_f, pu32, 1);
            SetDsUserIdTunnelIpv6CapwapHashKey(V, valid3_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_RMAC:
            SetDsUserIdTunnelCapwapRmacHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_RMAC_RID:
            SetDsUserIdTunnelCapwapRmacRidHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_STA_STATUS:
            SetDsUserIdCapwapStaStatusHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS:
            SetDsUserIdCapwapStaStatusMcHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD:
            SetDsUserIdCapwapVlanForwardHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD:
            SetDsUserIdCapwapMacDaForwardHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_ECID:
            SetDsUserIdEcidNameSpaceHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_ING_ECID:
            SetDsUserIdIngEcidNameSpaceHashKey(V, valid_f, pu32, 1);
	     break;
	    case SYS_SCL_KEY_HASH_PORT_CROSS:
            SetDsEgressXcOamPortCrossHashKey(V, valid_f, pu32, 1);
	     break;
	    case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:
            SetDsEgressXcOamPortVlanCrossHashKey(V, valid_f, pu32, 1);
	     break;
        case SYS_SCL_KEY_HASH_TRILL_UC:
            SetDsUserIdTunnelTrillUcDecapHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
            SetDsUserIdTunnelTrillUcRpfHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC:
            SetDsUserIdTunnelTrillMcDecapHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
            SetDsUserIdTunnelTrillMcRpfHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            SetDsUserIdTunnelTrillMcAdjHashKey(V, valid_f, pu32, 1);
            break;
        case SYS_SCL_KEY_PORT_DEFAULT_EGRESS:
        case SYS_SCL_KEY_PORT_DEFAULT_INGRESS:
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, pe->group->priority, pe, &key_id, &act_id));

    if ((SYS_SCL_KEY_PORT_DEFAULT_EGRESS != pe->key_type) &&
        (SYS_SCL_KEY_PORT_DEFAULT_INGRESS != pe->key_type))
    {
        /*cmd_key = DRV_IOW(key_id, DRV_ENTRY_FLAG);*/
        drv_acc_in_t  in;
        drv_acc_out_t  out;

        sal_memset(&in, 0, sizeof(in));

        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id;
        in.data   = p_key_void;
        in.index  = pe->key_index;
        in.level  = pe->group->priority;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    }

    return CTC_E_NONE;
}

sys_scl_sw_hash_ad_t*
_sys_usw_scl_add_hash_ad_spool(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    int32  ret = CTC_E_NONE;
    uint8 is_half = 0;
    uint8 scl_id = 0;
    sys_scl_sw_hash_ad_t  new_profile;
    sys_scl_sw_hash_ad_t  old_profile;
    sys_scl_sw_hash_ad_t* out_profile = NULL;
    ctc_scl_field_action_t   field_action;

    sal_memset(&new_profile, 0, sizeof(sys_scl_sw_hash_ad_t));
    sal_memset(&old_profile, 0, sizeof(sys_scl_sw_hash_ad_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    if ((SYS_SCL_ACTION_INGRESS == pe->action_type) || (SYS_SCL_ACTION_TUNNEL ==  pe->action_type))
    {
        if (0 == *((uint32*)(&pe->temp_entry->action) + 3) + *((uint32*)(&pe->temp_entry->action) + 4) + *((uint32*)(&pe->temp_entry->action) + 5))
        {
            is_half = 1;
        }
        else
        {
            is_half = 0;
        }
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_IS_HALF;
        p_usw_scl_master[lchip]->build_ad_func[pe->action_type](lchip, &field_action, pe, is_half);
    }
    scl_id = (1 == p_usw_scl_master[lchip]->hash_mode ? pe->group->priority : 0);
    new_profile.action_type = pe->action_type;
    new_profile.is_half = is_half;
    new_profile.priority = scl_id;
    sal_memcpy(&new_profile.action, &pe->temp_entry->action, sizeof(DsUserId_m));
    if(p_usw_scl_master[lchip]->ad_spool[scl_id] != NULL)
    {
        ret = ctc_spool_add(p_usw_scl_master[lchip]->ad_spool[scl_id], &new_profile, pe->hash_ad, &out_profile);
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add spool error\n");
        return NULL;
    }
    if (ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add_hash_ad: %d,is_half: %d\n", out_profile->ad_index, out_profile->is_half);
        return out_profile;
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add spool error\n");
        return NULL;
    }

}

STATIC int32
_sys_usw_scl_remove_hash_ad_spool(uint8 lchip, sys_scl_sw_entry_t* pe)

{
    int32 ret = 0;
    uint8 scl_id = 0;

    if (pe->hash_ad)
    {
        CTC_MAX_VALUE_CHECK(pe->group->priority, MCHIP_CAP(SYS_CAP_SCL_HASH_NUM)-1);
        scl_id = (1 == p_usw_scl_master[lchip]->hash_mode ? pe->group->priority : 0);
        ret = ctc_spool_remove(p_usw_scl_master[lchip]->ad_spool[scl_id], pe->hash_ad, NULL);

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove_hash_ad:%d is_half:%d \n", pe->hash_ad->ad_index, pe->hash_ad->is_half);
    }

    if (ret < 0)
    {
        return ret;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_resume_buffer_from_hw(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint32                 key_id        = 0;
    uint32                 act_id        = 0;
    sys_scl_sw_group_t*      pg = NULL;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    tbl_entry_t          entry;
    uint32*               p_key = NULL;
    uint32*               p_mask = NULL;
    uint32 cmd = 0;
    ctc_field_key_t field_key;
    uint32 hw_key_index = 0;
    uint32 hw_ad_index  = 0;
	uint8  egs_hash_key = 0;
    uint8  tcam_key = 0;
    int32 ret = CTC_E_NONE;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    if (!pe)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }
    pg = pe->group;

    if (!pg)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Group not exist \n");
			return CTC_E_NOT_EXIST;

    }

    if (pe->uninstall)
    {
        return CTC_E_NONE;
    }
    tcam_key = SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict;
	egs_hash_key = !tcam_key && ( SYS_SCL_ACTION_EGRESS == pe->action_type);

    /* get group_info */
    CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, pg->priority, pe, &key_id, &act_id));
    /* get key_index and ad_index */
    CTC_ERROR_RETURN(_sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, &hw_ad_index));

    MALLOC_ZERO(MEM_SCL_MODULE, p_temp_entry, sizeof(sys_scl_sw_temp_entry_t));
    if (!p_temp_entry)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    pe->temp_entry = p_temp_entry;

    if(!egs_hash_key)
    {
    	 cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_ad_index, cmd, &p_temp_entry->action));
    }

    p_key = mem_malloc(MEM_SCL_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    p_mask = mem_malloc(MEM_SCL_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    if ((NULL == p_key) || (NULL == p_mask))
    {
        ret = CTC_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_key, 0, sizeof(uint32)*MAX_ENTRY_WORD);
    sal_memset(p_mask, 0, sizeof(uint32)*MAX_ENTRY_WORD);

    if (tcam_key)
    {
        entry.data_entry = p_key;
        entry.mask_entry = p_mask;
        cmd = DRV_IOR(key_id, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, hw_key_index, cmd, &entry), ret, out);
        sal_memcpy(p_temp_entry->key, entry.data_entry, 96);
        sal_memcpy(p_temp_entry->mask, entry.mask_entry, 96);
    }
    else
    {
        drv_acc_in_t in;
        drv_acc_out_t out;
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));
        in.type = DRV_ACC_TYPE_LOOKUP;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id;
        in.index = hw_key_index;
        in.level = pe->group->priority;
        CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, out);

        sal_memcpy(p_temp_entry->key, out.data, sizeof(p_temp_entry->key));

        if(egs_hash_key)
        {
            sal_memcpy((uint8*)(&pe->temp_entry->action), (uint8*)(pe->temp_entry->key + 3), sizeof(DsVlanXlate_m));
        }
    }

out:
    if (p_key)
    {
        mem_free(p_key);
    }
    if (p_mask)
    {
        mem_free(p_mask);
    }

    return ret;
}

STATIC int32
_sys_usw_scl_add_hw(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint32      hw_key_index  = 0;
    uint32      hw_ad_index   = 0;
    uint32      cmd_ds     = 0;
    uint32      cmd_key    = 0;
    uint32      key_id     = 0;
    uint32      act_id     = 0;
    void*       p_key_void = NULL;
	uint8       igs_hash_key = 0;
	uint8       egs_hash_key = 0;
    uint8       is_tcam_key = 0;
    uint8       scl_id                = SYS_SCL_IS_HASH_COM_MODE(pe)? 0: pe->group->priority;
	int32       ret = CTC_E_NONE;
    tbl_entry_t              tcam_key;
    ctc_field_key_t          field_key;
    ctc_scl_field_action_t   field_action;
    sys_scl_sw_group_t* pg = pe->group;
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(pe);
    p_key_void = (void*)pe->temp_entry->key;

    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

	is_tcam_key = SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict;
	egs_hash_key = !is_tcam_key  && ( SYS_SCL_ACTION_EGRESS == pe->action_type);
    igs_hash_key = !is_tcam_key && !egs_hash_key;

    /*add vlan edit spool hardware*/
    if (pe->vlan_edit)
    {
        uint32 cmd = 0;

        cmd = DRV_IOW(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->vlan_edit->profile_id, cmd, (void*)&pe->vlan_edit->action_profile));
        pe->vlan_profile_id = pe->vlan_edit->profile_id;
    }

    /*add acl spool hardware*/
    if (pe->acl_profile)
    {
        uint32 cmd = 0;
        uint32 tbl_id = 0;

        tbl_id = pe->acl_profile->is_scl? DsSclAclControlProfile_t: DsTunnelAclControlProfile_t;

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, pe->acl_profile->profile_id, cmd, &pe->acl_profile->acl_control_profile));
        pe->acl_profile_id = pe->acl_profile->profile_id;
    }



	if(igs_hash_key)
	{
		pe->hash_ad = _sys_usw_scl_add_hash_ad_spool(lchip, pe);
		if(NULL == pe->hash_ad)
		{
		   SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

		}
		pe->ad_index = pe->hash_ad->ad_index;
		pe->is_half = pe->hash_ad->is_half;
	}
	if(egs_hash_key)
	{
	   pe->ad_index = pe->key_index;
	   sal_memcpy(pe->temp_entry->key + 3, (uint32*)(&pe->temp_entry->action) , sizeof(DsVlanXlate_m));
	}

    _sys_usw_scl_get_table_id(lchip, scl_id, pe, &key_id, &act_id);
    _sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, &hw_ad_index);

    if (is_tcam_key)
    {
        tcam_key.data_entry = (uint32 *) (pe->temp_entry->key);
        tcam_key.mask_entry = (uint32 *) (pe->temp_entry->mask);
        p_key_void = &tcam_key;
    }
    else if(igs_hash_key)
 	{
      field_key.type = SYS_SCL_FIELD_KEY_AD_INDEX;
      field_key.data = hw_ad_index;
      CTC_ERROR_RETURN(p_usw_scl_master[lchip]->build_key_func[pe->key_type](lchip, &field_key, pe, TRUE));

 	}

    if (!egs_hash_key) /* igs hash key or tcamkey*/
    {
        cmd_ds = DRV_IOW(act_id, DRV_ENTRY_FLAG);
        ret = ret ? ret: DRV_IOCTL(lchip, hw_ad_index, cmd_ds, &pe->temp_entry->action);

    }

    if (is_tcam_key)
    {
        cmd_key = DRV_IOW(key_id, DRV_ENTRY_FLAG);
        ret = ret ? ret:DRV_IOCTL(lchip, hw_key_index, cmd_key, p_key_void);
    }
    else
    {
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));

        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id;
        in.data   = p_key_void;
        in.index  = hw_key_index;
        in.level  = scl_id;
        ret = ret ? ret: drv_acc_api(lchip, &in, &out);

    }

    if (SYS_SCL_IS_HASH_COM_MODE(pe))
    {
        uint32 key_id1 = 0;
        uint32 act_id1 = 0;
        drv_acc_out_t out1;
        in.level = 1;

        _sys_usw_scl_get_table_id(lchip, in.level, pe, &key_id1, &act_id1);

        cmd_ds = DRV_IOW(act_id1, DRV_ENTRY_FLAG);
        ret = ret ? ret: DRV_IOCTL(lchip, hw_ad_index, cmd_ds, &pe->temp_entry->action);

        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));

        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id1;
        in.data   = p_key_void;
        in.index  = hw_key_index;
        ret = ret ? ret: drv_acc_api(lchip, &in, &out1);

    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add_hw %s: :lkup:%d entry id:%u key_id:%u hw_key_index:%u (%u)\n",(ret?"fail":"pass"),pg->priority, pe->entry_id,key_id,hw_key_index,pe->key_index);

    if (ret != CTC_E_NONE)
    {
        _sys_usw_scl_remove_hash_ad_spool(lchip, pe);
        return ret;     /* Mustn't free temp entry, when error happened */
    }
    else
    {
        uint8 loop = 0;
        /* only success add to hash and update action falg is uninstall flag is equal to 0 */
        if ((SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict) && pe->uninstall)
        {
            sys_scl_tcam_entry_key_t* tcam_entry;               /* when do hash insert, the data to inserted must be a pointer */

            MALLOC_ZERO(MEM_SCL_MODULE, tcam_entry, sizeof(sys_scl_tcam_entry_key_t));
            if (NULL == tcam_entry)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                return CTC_E_NO_MEMORY;

            }
            for(loop = 0; loop < SYS_SCL_MAX_KEY_SIZE_IN_WORD; loop++)
            {
                tcam_entry->key[loop] = pe->temp_entry->key[loop] & pe->temp_entry->mask[loop];
            }
            sal_memcpy(tcam_entry->mask, pe->temp_entry->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
            tcam_entry->scl_id = pe->group->priority;
            tcam_entry->p_entry = pe;

            /* add to hash by key */
            if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->tcam_entry_by_key, tcam_entry))
            {
                mem_free(tcam_entry);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Hash insert fail\n");
                return CTC_E_NO_MEMORY;
            }
        }
        /* keep this opration behind adding to hash always */
        pe->uninstall = 0;
    }

    mem_free(pe->temp_entry);

    pe->temp_entry = NULL;

    return ret;
}

int32
_sys_usw_scl_remove_hw(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint32 cmd         = 0;
    uint32 hw_key_index   = 0;
    uint32 key_id      = 0;
    uint32 act_id      = 0;
    sys_scl_sw_group_t* pg = NULL;

    if (!pe)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    pg = pe->group;
    if (!pg)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Group not exist \n");
			return CTC_E_NOT_EXIST;

    }

    /* get group_info */
    _sys_usw_scl_get_table_id(lchip, pg->priority, pe, &key_id, &act_id);

    _sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, NULL);

    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        cmd = DRV_IOD(key_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, hw_key_index, cmd, &cmd);
    }
    else
    {
        _sys_usw_scl_remove_hash_ad_spool(lchip, pe);
        /* -- set hw valid bit  -- */
        _sys_usw_scl_set_hw_hash_valid(lchip, pe);
        if (SYS_SCL_IS_HASH_COM_MODE(pe))
        {
            uint8 scl_id = pe->group->priority;
            pe->group->priority = scl_id? 0: 1;
            _sys_usw_scl_set_hw_hash_valid(lchip, pe);
            pe->group->priority = scl_id;
        }
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove_hw: priority:%d entry id:%u key_id:%u key_index:%u\n", pg->priority, pe->entry_id, key_id, hw_key_index);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_asic_hash_lkup(uint8 lchip, sys_scl_sw_entry_t* pe, uint8 is_lkup, uint8 install, uint32* o_key_index)
{

    uint32 key_id       = 0;
    uint32 act_id       = 0;
    uint8  key_exist    = 0;
    uint8  key_conflict = 0;
    uint8  scl_id = (1 == p_usw_scl_master[lchip]->hash_mode ? pe->group->priority : 0);
    drv_acc_in_t  in;
    drv_acc_out_t out;

    CTC_PTR_VALID_CHECK(pe);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    if (pe->hash_sel_bmp[0] || pe->hash_sel_bmp[1])
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_scl_asic_hash_lkup : key is incomplete, hash_sel_bmp(0x%.8x,0x%.8x)\n", pe->hash_sel_bmp[0], pe->hash_sel_bmp[1]);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Not ready to configuration \n");
		return CTC_E_NOT_READY;

    }

    CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, scl_id, pe, &key_id, &act_id));

    in.type     = is_lkup ? DRV_ACC_TYPE_LOOKUP : DRV_ACC_TYPE_ALLOC;
    in.op_type  = DRV_ACC_OP_BY_KEY;
    in.tbl_id   = key_id;
    in.data     = pe->temp_entry->key;
    in.level    = scl_id;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    key_exist   =  out.is_hit;
    key_conflict = out.is_conflict;

    if (SYS_SCL_IS_HASH_COM_MODE(pe))
    {
        drv_acc_out_t out1;
        uint32 key_id1 = 0;
        uint32 act_id1 = 0;
        CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, 1, pe, &key_id1, &act_id1));
        in.tbl_id = key_id1;
        in.level = 1;
        drv_acc_api(lchip, &in, &out1);
    }

    if (is_lkup && install && (pe->key_exist || pe->key_conflict) && !out.is_hit)
    {
        in.type     =  DRV_ACC_TYPE_ALLOC;
        in.op_type  = DRV_ACC_OP_BY_KEY;
        in.tbl_id   = key_id;
        in.data     = pe->temp_entry->key;
        in.level    = scl_id;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
        key_exist =  out.is_hit;
        key_conflict =  out.is_conflict;
        is_lkup = 0;
    }


    if (key_conflict)
    {
        if (is_lkup && !pe->key_conflict && install)
        {
            /* do nothing */
        }
        else
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% conflict: %d \n", key_conflict);
            pe->key_conflict  = key_conflict;
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
			return CTC_E_HASH_CONFLICT;

        }
    }
    else if (key_exist)
    {
        ds_t ds_hash_key;
        sal_memset(&ds_hash_key, 0, sizeof(ds_hash_key));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% entry exist: %d \n", key_exist);

        in.type     = DRV_ACC_TYPE_ADD;
        in.op_type  = DRV_ACC_OP_BY_INDEX;
        in.tbl_id   = key_id;
        in.data     = ds_hash_key;
        in.index    = pe->key_index;
        in.level    = scl_id;
        if (pe->hash_valid)
        {
            drv_acc_api(lchip, &in, &out);
            if (SYS_SCL_IS_HASH_COM_MODE(pe))
            {
                CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, 1, pe, &key_id, &act_id));
                in.tbl_id = key_id;
                in.level = 1;
                drv_acc_api(lchip, &in, &out);
            }
            pe->hash_valid = 0;

        }
        pe->key_index = 0xFFFFFFFF;
        pe->key_exist = key_exist;

        if(o_key_index)
        {
            *o_key_index = out.key_index;
        }

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
		return CTC_E_EXIST;

    }

    if (!is_lkup)
    {
        pe->key_index = out.key_index;
        pe->hash_valid = 1;
        pe->key_exist = 0;
        pe->key_conflict = 0;
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% out.index: 0x%x \n", pe->key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_install_entry(uint8 lchip, sys_scl_sw_entry_t* pe, uint8 move_sw)
{
    uint8 hash_key = !SCL_ENTRY_IS_TCAM(pe->key_type) && (!pe->resolve_conflict);
    uint8 loop = 0;
    sys_scl_tcam_entry_key_t* pe_lkup = NULL;
    sys_scl_tcam_entry_key_t pe_in;
    sal_memset(&pe_in, 0, sizeof(sys_scl_tcam_entry_key_t));
    /* check tcam entry key exit */
    if (SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict)
    {
        for (loop = 0; loop < SYS_SCL_MAX_KEY_SIZE_IN_WORD; loop++)
        {
            pe_in.key[loop] = pe->temp_entry->key[loop] & pe->temp_entry->mask[loop];
        }
        sal_memcpy(pe_in.mask, pe->temp_entry->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
        pe_in.scl_id = pe->group->priority;
        pe_lkup = ctc_hash_lookup(p_usw_scl_master[lchip]->tcam_entry_by_key, &pe_in);
        if (pe_lkup) /* must be unique */
        {
            return CTC_E_EXIST;
        }
    }
    /* check hash entry key exit */
    if (!SCL_ENTRY_IS_TCAM(pe->key_type) && !pe->resolve_conflict)
    {
        CTC_ERROR_RETURN(_sys_usw_scl_asic_hash_lkup(lchip, pe, 1, 1, NULL));
    }

    if (hash_key && !pe->hash_valid)
    {
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_scl_install_entry,eid:%u hash_valid:%d\n", pe->entry_id, pe->hash_valid);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Not ready to configuration \n");
			return CTC_E_NOT_READY;

    }

    /* when install group may install all entry include installed entry*/
    CTC_ERROR_RETURN(_sys_usw_scl_add_hw(lchip, pe));

    if (move_sw)
    {
        /* installed entry move to tail,keep uninstalled entry before installed in group entry list */
        ctc_slist_delete_node(pe->group->entry_list, &(pe->head));
        if (NULL == ctc_slist_add_tail(pe->group->entry_list, &(pe->head)))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    if ((!SCL_ENTRY_IS_TCAM(pe->key_type) && (!pe->resolve_conflict)))
    {
        sys_scl_hash_key_entry_t* hash_entry;               /* when do hash insert, the data to inserted must be a pointer */

        MALLOC_ZERO(MEM_SCL_MODULE, hash_entry, sizeof(sys_scl_hash_key_entry_t));
        if (NULL == hash_entry)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        hash_entry->key_index = pe->key_index;
        hash_entry->action_type = pe->action_type;
        hash_entry->entry_id = pe->entry_id;
        hash_entry->scl_id = pe->group->priority;

        /* add to hash by key */
        if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->hash_key_entry, hash_entry))
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Hash insert fail\n");
			return CTC_E_NO_MEMORY;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_uninstall_entry(uint8 lchip, sys_scl_sw_entry_t* pe, uint8 move_sw)
{
    sys_scl_tcam_entry_key_t* pe_lkup = NULL;
    uint8 loop = 0;

    CTC_ERROR_RETURN(_sys_usw_scl_resume_buffer_from_hw(lchip, pe));
    CTC_ERROR_RETURN(_sys_usw_scl_remove_hw(lchip, pe));

    pe->uninstall = 1;

    if (move_sw)
    {
        /* installed entry move to tail,keep uninstalled entry before installed in group entry list */
        /* move to head */
        ctc_slist_delete_node(pe->group->entry_list, &(pe->head));

        if (NULL == ctc_slist_add_head(pe->group->entry_list, &(pe->head)))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

	if ((!SCL_ENTRY_IS_TCAM(pe->key_type) && (!pe->resolve_conflict)))
    {
	     sys_scl_hash_key_entry_t hash_entry;
	     sys_scl_hash_key_entry_t* p_hash_entry = NULL;
		 hash_entry.key_index = pe->key_index;
		 hash_entry.action_type = pe->action_type;
		 hash_entry.entry_id = pe->entry_id;
              hash_entry.scl_id = pe->group->priority;

        /* add to hash by key */
        p_hash_entry = ctc_hash_remove(p_usw_scl_master[lchip]->hash_key_entry, &hash_entry);
        if(!p_hash_entry)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Hash Remove fail\n");
			return CTC_E_NOT_EXIST;

        }
        mem_free(p_hash_entry);
    }
    else
    {
        sys_scl_tcam_entry_key_t tcam_entry;
        sal_memset(&tcam_entry, 0, sizeof(sys_scl_tcam_entry_key_t));
        for (loop = 0; loop < SYS_SCL_MAX_KEY_SIZE_IN_WORD; loop++)
        {
            tcam_entry.key[loop] = pe->temp_entry->key[loop] & pe->temp_entry->mask[loop];
        }
        sal_memcpy(tcam_entry.mask, pe->temp_entry->mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
        tcam_entry.scl_id = pe->group->priority;
        tcam_entry.p_entry = pe;

        /* remove hash node by key */
        pe_lkup = ctc_hash_remove(p_usw_scl_master[lchip]->tcam_entry_by_key, &tcam_entry);
        if(NULL == pe_lkup)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Hash Remove fail\n");
			return CTC_E_NOT_EXIST;
        }
        mem_free(pe_lkup);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_tcam_entry_add(uint8 lchip, ctc_scl_entry_t* scl_entry, sys_scl_sw_tcam_entry_t* pe)
{
    uint8    fpa_size = 0;
    int32    ret = CTC_E_NONE;
    uint32   block_index = 0;
    uint32   priority = 0;
    sys_scl_sw_block_t*  pb = NULL;
    sys_scl_sw_group_t*  pg = pe->entry->group;

    if (scl_entry->priority_valid)
    {
        priority = scl_entry->priority;
    }
    else
    {
        priority = FPA_PRIORITY_DEFAULT;
    }

    /* get block index. */
    fpa_size = p_usw_scl_master[lchip]->fpa_size[pe->entry->key_type];
    pb       = &p_usw_scl_master[lchip]->block[pg->priority];

    CTC_ERROR_GOTO(fpa_usw_alloc_offset(p_usw_scl_master[lchip]->fpa, &pb->fpab, fpa_size,   /* may trigger fpa entry movement */
                                          priority, &block_index), ret, cleanup);
    {
        /* move to fpa */
        /* add to block */
        pb->fpab.entries[block_index] = &(pe->fpae);

        /* set block index */
        pe->entry->key_index = block_index;
        pe->entry->ad_index = block_index;
        pe->fpae.priority = priority;
        pe->fpae.entry_id = scl_entry->entry_id;
        pe->fpae.offset_a = block_index;
        pe->fpae.lchip = lchip;

        fpa_usw_reorder(p_usw_scl_master[lchip]->fpa, &pb->fpab, fpa_size); /* if fpab move_num at threshold will trigger scatter reorder */
    }
     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add_tcam_entry: priority:%d entry id:%u key_type:%d key_index:%u\n",pg->priority, scl_entry->entry_id,pe->entry->key_type,block_index);

    return CTC_E_NONE;

cleanup:
    return ret;
}

STATIC int32
_sys_usw_scl_tcam_entry_remove(uint8 lchip, sys_scl_sw_entry_t* pe_lkup)
{

    uint32 block_index = 0;
    sys_scl_sw_group_t* pg = NULL;
    sys_scl_sw_block_t* pb = NULL;
    sys_scl_sw_tcam_entry_t* pe_tcam = NULL;


    /* remove from block */
    pg          = pe_lkup->group;
    pb          = &p_usw_scl_master[lchip]->block[pg->priority];
    block_index = pe_lkup->key_index;
    pe_tcam     = (sys_scl_sw_tcam_entry_t*) pb->fpab.entries[block_index];

    CTC_ERROR_RETURN(fpa_usw_free_offset(&pb->fpab, block_index));
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add_tcam_entry: priority:%d entry id:%u key_type:%d key_index:%u\n",pg->priority, pe_lkup->entry_id,pe_lkup->key_type,block_index);
    mem_free(pe_tcam);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_add_entry(uint8 lchip, sys_scl_sw_group_t* pg,
                              sys_scl_sw_entry_t* pe)
{
    int32 ret = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* add to hash, error just return */
    if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->entry, pe))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Hash insert fail\n");
        return CTC_E_NO_MEMORY;
    }

    /* add to group entry list's head, error just return */
    if(NULL == ctc_slist_add_head(pg->entry_list, &(pe->head)))
    {
        ret = CTC_E_INVALID_PARAM;
        goto error0;
    }

    /* the tunnel count static data has to be reconsidered */
    if(SYS_SCL_KEY_HASH_L2 == pe->key_type)
    {
        /*flow l2 hash key*/
        p_usw_scl_master[lchip]->hash_sel_profile_count[pe->hash_field_sel_id]++;
    }

    return CTC_E_NONE;

error0:
    /* do not free pe, free pe outside of the function */
    ctc_hash_remove(p_usw_scl_master[lchip]->entry, pe);

    return ret;
}

int32
_sys_usw_scl_remove_entry(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint8  count_size  = 0;
    uint8  scl_id         = (SYS_SCL_IS_HASH_COM_MODE(pe) || 2 == p_usw_scl_master[lchip]->hash_mode ? 0: pe->group->priority);
    uint16 old_logic_port = 0;
    uint32 key_id      = 0;
    uint32 act_id      = 0;
    uint32 entry_size  = 0;
    sys_scl_sw_group_t* pg = pe->group;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* unbind qos */
    old_logic_port = GetDsUserId(V, logicSrcPort_f, &pe->temp_entry->action);
    if (old_logic_port && pe->service_id)
    {
        sys_usw_qos_unbind_service_logic_srcport(lchip, pe->service_id, old_logic_port);
    }

    if (pe->bind_nh)
    {
        _sys_usw_scl_unbind_nexthop(lchip, pe, pe->nexthop_id);
    }
    /*remove spool node*/
    sys_usw_scl_remove_vlan_edit_action_profile(lchip, pe->vlan_edit);
    pe->vlan_edit = NULL;
    sys_usw_scl_remove_acl_control_profile(lchip, pe->acl_profile);
    pe->acl_profile = NULL;

    if(pe->ether_type != 0)
    {
        CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe->ether_type));
    }

    /* remove from group */
    ctc_slist_delete_node(pg->entry_list, &(pe->head));

    /* remove from hash */
    if (NULL == ctc_hash_remove(p_usw_scl_master[lchip]->entry, pe))
    {
        return CTC_E_NOT_EXIST;
    }


    if (!SCL_ENTRY_IS_TCAM(pe->key_type) && (!pe->resolve_conflict))
    {

        CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, scl_id, pe, &key_id, &act_id));

        /* clear hw valid bit regardless the valid bit is set or not */
        /* celar hw must check this entry has hash_valid or not before removing entry, if not, can not use drv acc interface because
        the entry's key_index equals zeros has high possibility of having already becoming other entry's key_index */
        if (pe->hash_valid && pe->key_index != 0xFFFFFFFF)
        {
            drv_acc_in_t  in;
            drv_acc_out_t  out;
            sal_memset(&in, 0, sizeof(in));

            in.type = DRV_ACC_TYPE_DEL;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            in.tbl_id = key_id;
            in.index  = pe->key_index;
            in.level  = scl_id;
            CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
        }

        if (SYS_SCL_IS_HASH_COM_MODE(pe))
        {
            CTC_ERROR_RETURN(_sys_usw_scl_get_table_id(lchip, 1, pe, &key_id, &act_id));

            /* clear hw valid bit regardless the valid bit is set or not */
            /* celar hw must check this entry has hash_valid or not before removing entry, if not, can not use drv acc interface because
            the entry's key_index equals zeros has high possibility of having already becoming other entry's key_index */
            if (pe->hash_valid && pe->key_index != 0xFFFFFFFF)
            {
                drv_acc_in_t  in;
                drv_acc_out_t  out;
                sal_memset(&in, 0, sizeof(in));

                in.type = DRV_ACC_TYPE_DEL;
                in.op_type = DRV_ACC_OP_BY_INDEX;
                in.tbl_id = key_id;
                in.index  = pe->key_index;
                in.level  = 1;
                CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
            }
        }
        /* update statistic data */
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_size(lchip, key_id, &entry_size));
        count_size = entry_size / 12;
        p_usw_scl_master[lchip]->entry_count[pe->action_type] = p_usw_scl_master[lchip]->entry_count[pe->action_type] - count_size;
    }

    if(SYS_SCL_KEY_HASH_L2 == pe->key_type)
    {
        /*flow l2 hash key*/
        if (p_usw_scl_master[lchip]->hash_sel_profile_count[pe->hash_field_sel_id] > 0)
        {
            p_usw_scl_master[lchip]->hash_sel_profile_count[pe->hash_field_sel_id]--;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_map_key_action_type(ctc_scl_entry_t* scl_entry, sys_scl_sw_entry_t* pe)
{

    switch (scl_entry->key_type)
    {
        case CTC_SCL_KEY_TCAM_MAC:
            pe->key_type = SYS_SCL_KEY_TCAM_MAC;
            break;
        case CTC_SCL_KEY_TCAM_IPV4:
            pe->key_type = SYS_SCL_KEY_TCAM_IPV4;
            break;
        case CTC_SCL_KEY_TCAM_IPV4_SINGLE:
            pe->key_type = SYS_SCL_KEY_TCAM_IPV4_SINGLE;
            break;
        case CTC_SCL_KEY_TCAM_IPV6:
            pe->key_type = SYS_SCL_KEY_TCAM_IPV6;
            break;
        case CTC_SCL_KEY_TCAM_IPV6_SINGLE:
            pe->key_type = SYS_SCL_KEY_TCAM_IPV6_SINGLE;
            break;
        case CTC_SCL_KEY_TCAM_UDF:
            pe->key_type = SYS_SCL_KEY_TCAM_UDF;
            break;
        case CTC_SCL_KEY_TCAM_UDF_EXT:
            pe->key_type = SYS_SCL_KEY_TCAM_UDF_EXT;
            break;
        case CTC_SCL_KEY_HASH_PORT:
            pe->key_type = SYS_SCL_KEY_HASH_PORT;
            break;
        case CTC_SCL_KEY_HASH_PORT_CVLAN:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_CVLAN;
            break;
        case CTC_SCL_KEY_HASH_PORT_SVLAN:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_SVLAN;
            break;
        case CTC_SCL_KEY_HASH_PORT_2VLAN:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_2VLAN;
            break;
        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_CVLAN_COS;
            break;
        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_SVLAN_COS;
            break;
        case CTC_SCL_KEY_HASH_MAC:
            pe->key_type = SYS_SCL_KEY_HASH_MAC;
            break;
        case CTC_SCL_KEY_HASH_PORT_MAC:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_MAC;
            break;
        case CTC_SCL_KEY_HASH_IPV4:
            pe->key_type = SYS_SCL_KEY_HASH_IPV4;
            break;
        case CTC_SCL_KEY_HASH_SVLAN:
            pe->key_type = SYS_SCL_KEY_HASH_SVLAN;
            break;
        case CTC_SCL_KEY_HASH_SVLAN_MAC:
            pe->key_type = SYS_SCL_KEY_HASH_SVLAN_MAC;
            break;
        case CTC_SCL_KEY_HASH_PORT_IPV4:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_IPV4;
            break;
        case CTC_SCL_KEY_HASH_IPV6:
            pe->key_type = SYS_SCL_KEY_HASH_IPV6;
            break;
        case CTC_SCL_KEY_HASH_PORT_IPV6:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_IPV6;
            break;
        case CTC_SCL_KEY_HASH_L2:
            pe->key_type = SYS_SCL_KEY_HASH_L2;
            break;
        case CTC_SCL_KEY_HASH_PORT_CROSS:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_CROSS;
            break;
        case CTC_SCL_KEY_HASH_PORT_SVLAN_DSCP:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP;
            break;
        case CTC_SCL_KEY_HASH_PORT_VLAN_CROSS:
            pe->key_type = SYS_SCL_KEY_HASH_PORT_VLAN_CROSS;
            break;

        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    switch (scl_entry->action_type)
    {
        case CTC_SCL_ACTION_INGRESS:
            pe->action_type = SYS_SCL_ACTION_INGRESS;
            break;
        case CTC_SCL_ACTION_EGRESS:
            pe->action_type = SYS_SCL_ACTION_EGRESS;
            break;
        case CTC_SCL_ACTION_FLOW:
            pe->action_type = SYS_SCL_ACTION_FLOW;
            break;
        default:
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_get_hash_sel_bmp(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint32  cmd = 0;
    uint32  field_id = 0;
    uint32  field_id_cnt = 0;
    uint32  tbl_id = 0;
    uint32  key_id = 0;
    uint32  act_id = 0;
    ds_t    ds_sel;

    if (pe->key_type == SYS_SCL_KEY_HASH_L2)
    {
        tbl_id = SclFlowHashFieldSelect_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->hash_field_sel_id, cmd, &ds_sel));

        drv_get_table_property(lchip, DRV_TABLE_PROP_FIELD_NUM, tbl_id, 0, &field_id_cnt);

        for (field_id = 0; field_id < field_id_cnt; field_id++)
        {
            if (drv_get_field_value(lchip, tbl_id,  field_id, &ds_sel) != 0 )
            {
                CTC_BMP_SET(pe->hash_sel_bmp, field_id);
            }
        }
    }
    else
    {
        if (SYS_SCL_ACTION_EGRESS != pe->action_type)
        {
            _sys_usw_scl_get_table_id(lchip, 0, pe, &key_id, &act_id);
            drv_get_table_property(lchip, DRV_TABLE_PROP_FIELD_NUM, key_id, 0, &field_id_cnt);
            if(field_id_cnt > 64)
            {
                return CTC_E_INVALID_PARAM;
            }

            for (field_id = 0; field_id < field_id_cnt; field_id++)
            {
                CTC_BMP_SET(pe->hash_sel_bmp, field_id);
            }
        }
        else
        {
            /* egress action */
            switch (pe->key_type)
            {
                case SYS_SCL_KEY_HASH_PORT:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_CVLAN:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_cvlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_SVLAN:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_svlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_2VLAN:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_cvlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_svlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamDoubleVlanPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_cvlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_ctagCos_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamCvlanCosPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_svlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_stagCos_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamSvlanCosPortHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_CROSS:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_globalSrcPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortCrossHashKey_hashType_f);
                    break;
                case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLogicPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_isLabel_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_globalSrcPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_globalDestPort_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_vlanId_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_valid_f);
                    CTC_BMP_SET(pe->hash_sel_bmp, DsEgressXcOamPortVlanCrossHashKey_hashType_f);
                    break;

                default:
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

            }
        }
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "_sys_usw_scl_get_hash_sel_bmp : bit map is, hash_sel_bmp(0x%.8x,0x%.8x)\n", pe->hash_sel_bmp[0], pe->hash_sel_bmp[1]);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_check_action_type(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    uint8 match     = 0;
    uint8 dir_valid = 0;
    uint8 dir       = 0;
    uint8 action_type = 0;
    action_type = pe->action_type;
    switch (pe->key_type)
    {
        case SYS_SCL_KEY_TCAM_IPV4:
        case SYS_SCL_KEY_TCAM_IPV4_SINGLE:
        case SYS_SCL_KEY_TCAM_IPV6:
        case SYS_SCL_KEY_TCAM_IPV6_SINGLE:
        case SYS_SCL_KEY_TCAM_UDF :
        case SYS_SCL_KEY_TCAM_UDF_EXT :
            match = (SYS_SCL_ACTION_INGRESS == action_type) || (SYS_SCL_ACTION_TUNNEL == action_type) || (action_type == SYS_SCL_ACTION_FLOW);
            break;
        case SYS_SCL_KEY_TCAM_MAC:
        case SYS_SCL_KEY_HASH_MAC:
        case SYS_SCL_KEY_HASH_PORT_MAC:
        case SYS_SCL_KEY_HASH_IPV4:
        case SYS_SCL_KEY_HASH_SVLAN:
        case SYS_SCL_KEY_HASH_SVLAN_MAC:
        case SYS_SCL_KEY_HASH_PORT_IPV4:
        case SYS_SCL_KEY_HASH_IPV6:
        case SYS_SCL_KEY_HASH_PORT_IPV6:
        case SYS_SCL_KEY_PORT_DEFAULT_INGRESS:
            match = (SYS_SCL_ACTION_INGRESS == action_type) || (action_type == SYS_SCL_ACTION_FLOW);
            break;
        case SYS_SCL_KEY_HASH_PORT:
        case SYS_SCL_KEY_HASH_PORT_CVLAN:
        case SYS_SCL_KEY_HASH_PORT_SVLAN:
        case SYS_SCL_KEY_HASH_PORT_2VLAN:
        case SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP:
        case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
        case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
            dir       = pe->direction;
            dir_valid = 1;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
        case SYS_SCL_KEY_HASH_WLAN_RMAC:
        case SYS_SCL_KEY_HASH_WLAN_RMAC_RID:
        case SYS_SCL_KEY_HASH_TRILL_UC:
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
        case SYS_SCL_KEY_HASH_TRILL_MC:
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            match = (SYS_SCL_ACTION_TUNNEL == action_type);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_MPLS:
            match = (SYS_SCL_ACTION_MPLS == action_type);
            break;
        case SYS_SCL_KEY_HASH_WLAN_STA_STATUS:
        case SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS:
        case SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD:
        case SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD:
        case SYS_SCL_KEY_HASH_ECID:
        case SYS_SCL_KEY_HASH_ING_ECID:
            match = (SYS_SCL_ACTION_INGRESS == action_type);
            break;
        case SYS_SCL_KEY_PORT_DEFAULT_EGRESS:
        case SYS_SCL_KEY_HASH_PORT_CROSS:
        case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:
            match = (SYS_SCL_ACTION_EGRESS == action_type);
            break;
        case SYS_SCL_KEY_HASH_L2:
            match = (SYS_SCL_ACTION_FLOW == action_type);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    if (dir_valid)
    {
        CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION - 1);
        if (dir == CTC_INGRESS)
        {
            match = ((action_type == SYS_SCL_ACTION_INGRESS) ||
            (action_type == SYS_SCL_ACTION_FLOW));
        }
        else if (dir == CTC_EGRESS)
        {
            match = (action_type == SYS_SCL_ACTION_EGRESS);
        }
    }

    if (!match)
    {
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}



int32
_sys_usw_scl_recovery_db_from_hw(uint8 lchip, sys_scl_sw_entry_t* pe, sys_scl_sw_temp_entry_t* p_temp_entry)
{
    uint32 cmd         = 0;
    uint8  egs_hash_key = 0;
    uint8  is_tcam_key = 0;
    uint32 key_id      = 0;
    uint32 act_id      = 0;
    uint32 hw_key_index   = 0;
    uint32 hw_ad_index    = 0;
    void*  p_key_void = NULL;
    tbl_entry_t tcam_key;
    drv_acc_in_t in;
    drv_acc_out_t out;

    sal_memset(&tcam_key, 0, sizeof(tbl_entry_t));
    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    is_tcam_key = SCL_ENTRY_IS_TCAM(pe->key_type) || pe->resolve_conflict;
    egs_hash_key = (!SCL_ENTRY_IS_TCAM(pe->key_type)) && !(pe->resolve_conflict) && ( SYS_SCL_ACTION_EGRESS == pe->action_type);

    _sys_usw_scl_get_table_id(lchip, pe->group->priority, pe, &key_id, &act_id);
    _sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, &hw_ad_index);

    if (!egs_hash_key)
    {
        /*tcam key or igs hash key*/
        cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_ad_index, cmd, &p_temp_entry->action));
    }

    if (is_tcam_key)
    {
        tcam_key.data_entry = (uint32 *) (p_temp_entry->key);
        tcam_key.mask_entry = (uint32 *) (p_temp_entry->mask);
        p_key_void = &tcam_key;

        cmd = DRV_IOR(key_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, hw_key_index, cmd, p_key_void);
    }

    if (!is_tcam_key)
    {
        /*igs hash entry and egs hash entry */
        in.type = DRV_ACC_TYPE_LOOKUP;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id;
        in.index = hw_key_index;
        in.level = pe->group->priority;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

        sal_memcpy(p_temp_entry->key, out.data, sizeof(p_temp_entry->key));
    }

    if (egs_hash_key)
    {
        sal_memcpy((uint32*)(&p_temp_entry->action), p_temp_entry->key + 3, sizeof(DsVlanXlate_m));
    }

    return CTC_E_NONE;

}


int32
_sys_usw_scl_update_action_field(uint8 lchip, sys_scl_sw_entry_t* pe, ctc_scl_field_action_t* p_field_action, uint8 is_add)
{
    int32 ret = 0;
    MALLOC_ZERO(MEM_SCL_MODULE, pe->temp_entry, sizeof(sys_scl_sw_temp_entry_t));
    if (!pe->temp_entry)
    {
        return CTC_E_NO_MEMORY;
    }
    CTC_ERROR_GOTO(_sys_usw_scl_recovery_db_from_hw(lchip, pe, pe->temp_entry), ret, error0);


    if (p_usw_scl_master[lchip]->build_ad_func[pe->action_type])
    {
        CTC_ERROR_GOTO(p_usw_scl_master[lchip]->build_ad_func[pe->action_type](lchip, p_field_action, pe, is_add), ret, error0);
    }

    CTC_ERROR_GOTO(_sys_usw_scl_add_hw(lchip, pe), ret, error0);


    return CTC_E_NONE;

error0:
    mem_free(pe->temp_entry);

    return ret;
}

STATIC int32
_sys_usw_scl_check_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    uint8 tcam_mac_key = 0;
    uint8 tcam_ipv4_key = 0;
    uint8 tcam_ipv6_key = 0;
    uint8 tcam_ipv4_single_key = 0;
    uint8 tcam_ipv6_single_key = 0;
    uint8 tcam_udf_key  = 0;
    uint8 tcam_udf_ext_key = 0;
    uint8 ipv6_addr_mode = 0;
    uint8 l3Key160ShareMode = 0;
    uint32 l2_type_as_vlan_num = 0;
    uint32 cmd = 0;
    IpeUserIdTcamCtl_m Ipe_UserId;
    ctc_field_port_t* p_port = NULL;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    tcam_mac_key = pe->key_type == SYS_SCL_KEY_TCAM_MAC;
    tcam_ipv4_key = pe->key_type == SYS_SCL_KEY_TCAM_IPV4;
    tcam_ipv6_key = pe->key_type == SYS_SCL_KEY_TCAM_IPV6;
    tcam_ipv4_single_key = pe->key_type == SYS_SCL_KEY_TCAM_IPV4_SINGLE;
    tcam_ipv6_single_key = pe->key_type == SYS_SCL_KEY_TCAM_IPV6_SINGLE;
    tcam_udf_key = pe->key_type == SYS_SCL_KEY_TCAM_UDF;
    tcam_udf_ext_key = pe->key_type == SYS_SCL_KEY_TCAM_UDF_EXT;

    cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_layer2TypeUsedAsVlanNum_f);
    DRV_IOCTL(lchip, 0, cmd, &l2_type_as_vlan_num);

    /* IPV6 IPSA/IPDA share */
    cmd = DRV_IOR(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &Ipe_UserId);

    if (pe->group->priority == 0)
    {
        ipv6_addr_mode = GetIpeUserIdTcamCtl(V, array_0_v6BasicKeyIpAddressMode_f, &Ipe_UserId);
    }
    else if (1 == pe->group->priority)
    {
        ipv6_addr_mode = GetIpeUserIdTcamCtl(V, array_1_v6BasicKeyIpAddressMode_f, &Ipe_UserId);
    }
    else if (2 == pe->group->priority)
    {
        ipv6_addr_mode = GetIpeUserIdTcamCtl(V, array_2_v6BasicKeyIpAddressMode_f, &Ipe_UserId);
    }
    else if (3 == pe->group->priority)
    {
        ipv6_addr_mode = GetIpeUserIdTcamCtl(V, array_3_v6BasicKeyIpAddressMode_f, &Ipe_UserId);
    }

    if (!DRV_FLD_IS_EXISIT(IpeUserIdTcamCtl_t, IpeUserIdTcamCtl_lookupLevel_0_l3Key160ShareFieldMode_f))
    {
        /*IPV4 TTL/TCPFLAG/... share*/
        if (pe->action_type == SYS_SCL_ACTION_INGRESS)
        {
            if (0 == pe->group->priority)
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForUserId_f, &Ipe_UserId);
            }
            else
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForUserId_f, &Ipe_UserId);
            }
        }
        else if (pe->action_type == SYS_SCL_ACTION_FLOW)
        {
            if (pe->group->priority == 0)
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForScl_f, &Ipe_UserId);
            }
            else
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForScl_f, &Ipe_UserId);
            }
        }
        else if (pe->action_type == SYS_SCL_ACTION_TUNNEL)
        {
            if (pe->group->priority == 0)
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldModeForTunnelId_f, &Ipe_UserId);
            }
            else
            {
                l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldModeForTunnelId_f, &Ipe_UserId);
            }
        }
    }
    else
    {
        switch(pe->group->priority)
        {
        case 0:
            l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_0_l3Key160ShareFieldMode_f, &Ipe_UserId);
            break;
        case 1:
            l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_1_l3Key160ShareFieldMode_f, &Ipe_UserId);
            break;
        case 2 :
            l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_2_l3Key160ShareFieldMode_f, &Ipe_UserId);
            break;
        case 3:
            l3Key160ShareMode = GetIpeUserIdTcamCtl(V, lookupLevel_3_l3Key160ShareFieldMode_f, &Ipe_UserId);
            break;
        default :
            break;
        }
    }


    switch (key_field->type)
    {
        case CTC_FIELD_KEY_VLAN_NUM:
        case CTC_FIELD_KEY_L2_TYPE:
            if (l2_type_as_vlan_num == (CTC_FIELD_KEY_VLAN_NUM != key_field->type))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Layer2 type %s used as vlan num! \n", CTC_FIELD_KEY_VLAN_NUM != key_field->type? "is": "isn't");
                return CTC_E_INVALID_CONFIG;
            }
            CTC_MAX_VALUE_CHECK(key_field->data, (SYS_SCL_KEY_TCAM_UDF == pe->key_type || SYS_SCL_KEY_TCAM_UDF_EXT == pe->key_type)? 0x7: 0x3);
            break;
        case CTC_FIELD_KEY_PORT:
            p_port = (ctc_field_port_t*) (key_field->ext_data);
            CTC_PTR_VALID_CHECK(p_port);
            if (CTC_FIELD_PORT_TYPE_GPORT == p_port->type)
            {
                if (!SYS_USW_SCL_GPORT_IS_LOCAL(lchip, p_port->gport))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
                    return CTC_E_INVALID_CHIP_ID;
                }
                SYS_GLOBAL_PORT_CHECK(p_port->gport);
            }
            else if (CTC_FIELD_PORT_TYPE_LOGIC_PORT == p_port->type)
            {
                CTC_MAX_VALUE_CHECK(p_port->logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
            }
            else if (CTC_FIELD_PORT_TYPE_PORT_CLASS == p_port->type)
            {
                SYS_SCL_PORT_CLASS_ID_CHECK(p_port->port_class_id);
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
            break;
        case CTC_FIELD_KEY_STAG_VALID:
        case CTC_FIELD_KEY_SVLAN_ID:
        case CTC_FIELD_KEY_STAG_COS:
        case CTC_FIELD_KEY_STAG_CFI:
            if (tcam_mac_key && pe->mac_key_vlan_mode == 2)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key vlan_mode:%d \n", pe->mac_key_vlan_mode);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_CTAG_VALID:
        case CTC_FIELD_KEY_CVLAN_ID:
        case CTC_FIELD_KEY_CTAG_COS:
        case CTC_FIELD_KEY_CTAG_CFI:
            if (tcam_mac_key && pe->mac_key_vlan_mode == 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key vlan_mode:%d \n", pe->mac_key_vlan_mode);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_CUSTOMER_ID:
            if (tcam_mac_key && pe->mac_key_macda_mode == 1)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key macda_mode:%d \n", pe->mac_key_macda_mode);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_MAC_DA:
            if (tcam_mac_key && pe->mac_key_macda_mode == 2)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mac key macda_mode:%d \n", pe->mac_key_macda_mode);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_IP_DA:
        case CTC_FIELD_KEY_IP_SA:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_IPV4)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv4, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_IP_DSCP:
        case CTC_FIELD_KEY_IP_ECN:
            if((tcam_ipv4_key || tcam_ipv4_single_key) && (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not Ipv4, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_VN_ID:
            if (tcam_ipv4_key || tcam_ipv4_single_key)
            {
                /* ipv4 key */
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || pe->l4_type != CTC_PARSER_L4_TYPE_UDP || pe->l4_user_type != CTC_PARSER_L4_USER_TYPE_UDP_VXLAN)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not vxlan, L3 type is :%d, L4 type is :%d, L4 user type is :%d\n", pe->l3_type, pe->l4_type, pe->l4_user_type);
                    return CTC_E_INVALID_PARAM;
                }

                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 2)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not vxlan, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                /* ipv6 key */
                if (pe->l4_type != CTC_PARSER_L4_TYPE_UDP || pe->l4_user_type != CTC_PARSER_L4_USER_TYPE_UDP_VXLAN)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not vxlan, L4 type is :%d, L4 user type is :%d\n", pe->l4_type, pe->l4_user_type);
                    return CTC_E_INVALID_PARAM;
                }

                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 2)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not vxlan, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_GRE_KEY:
        case CTC_FIELD_KEY_NVGRE_KEY:
            if(tcam_ipv4_single_key || tcam_ipv4_key)
            {
                /* ipv4 key */
                if(pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || pe->l4_type != CTC_PARSER_L4_TYPE_GRE)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not GRE, L3 type is :%d, L4 type is :%d \n", pe->l3_type, pe->l4_type);
                    return CTC_E_INVALID_PARAM;
                }

                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 3)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not GRE, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                /* ipv6 key */
                if(pe->l4_type != CTC_PARSER_L4_TYPE_GRE)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not GRE, L4 type is :%d \n", pe->l4_type);
                    return CTC_E_INVALID_PARAM;
                }
                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 3)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not GRE, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if(tcam_ipv4_single_key || tcam_ipv4_key)
            {
                /* ipv4 key */
                if(pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || pe->l4_type == CTC_PARSER_L4_TYPE_GRE || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP && pe->l4_user_type == CTC_PARSER_L4_USER_TYPE_UDP_VXLAN))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not l4-port, L3 type is :%d, L4 type is :%d, L4 user type is :%d\n", pe->l3_type, pe->l4_type, pe->l4_user_type);
                    return CTC_E_INVALID_PARAM;
                }
                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not l4-port, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                /* ipv6 key */
                if(pe->l4_type == CTC_PARSER_L4_TYPE_GRE || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP && pe->l4_user_type == CTC_PARSER_L4_USER_TYPE_UDP_VXLAN))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not l4-port, L4 type is :%d, L4 user type is :%d\n", pe->l4_type, pe->l4_user_type);
                    return CTC_E_INVALID_PARAM;
                }
                if (pe->key_l4_port_mode != 0 &&  pe->key_l4_port_mode != 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L4 port mode is not l4-port, L4 port mode is :%d \n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;


        case CTC_FIELD_KEY_ARP_OP_CODE:
        case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        case CTC_FIELD_KEY_ARP_SENDER_IP:
        case CTC_FIELD_KEY_ARP_TARGET_IP:
        case CTC_FIELD_KEY_RARP:
            if (CTC_FIELD_KEY_RARP == key_field->type && DRV_IS_DUET2(lchip))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "RARP is not support\n");
                return CTC_E_NOT_SUPPORT;
            }
            else if (CTC_FIELD_KEY_RARP == key_field->type)
            {
                CTC_MAX_VALUE_CHECK(key_field->data, 1);
            }
            if(pe->l3_type != CTC_PARSER_L3_TYPE_ARP)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not ARP, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_LABEL_NUM:
        case CTC_FIELD_KEY_MPLS_LABEL0:
        case CTC_FIELD_KEY_MPLS_LABEL1:
        case CTC_FIELD_KEY_MPLS_LABEL2:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_MPLS && pe->l3_type != CTC_PARSER_L3_TYPE_MPLS_MCAST)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not MPLS, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_FCOE_DST_FCID:
        case CTC_FIELD_KEY_FCOE_SRC_FCID:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_FCOE)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not FCOE, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_INGRESS_NICKNAME:
        case CTC_FIELD_KEY_EGRESS_NICKNAME:
        case CTC_FIELD_KEY_IS_ESADI:
        case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        case CTC_FIELD_KEY_TRILL_KEY_TYPE:
        case CTC_FIELD_KEY_TRILL_LENGTH:
        case CTC_FIELD_KEY_TRILL_MULTIHOP:
        case CTC_FIELD_KEY_TRILL_MULTICAST:
        case CTC_FIELD_KEY_TRILL_VERSION:
        case CTC_FIELD_KEY_TRILL_TTL:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_TRILL)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not TRILL, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        case CTC_FIELD_KEY_ETHER_OAM_VERSION:
            if (tcam_ipv4_single_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_ETHER_OAM)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not ETHEROAM, L3 Type :%d \n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if (tcam_ipv4_key)
            {
                if(!(pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM) && !((pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST) && pe->l4_type == CTC_PARSER_L4_TYPE_ACH_OAM && pe->l4_user_type == CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHY1731))
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "OAM is conflict, L3 Type is:%d, L4 Type is:%d, L4 user Type is:%d\n", pe->l3_type, pe->l4_type, pe->l4_user_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_SLOW_PROTO)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not SLOWPROTO, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        case CTC_FIELD_KEY_PTP_VERSION:
            if(pe->l3_type != CTC_PARSER_L3_TYPE_PTP)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 Type is not PTP, L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_ETHER_TYPE:
            if(pe->l3_type == CTC_PARSER_L3_TYPE_IPV4 || pe->l3_type == CTC_PARSER_L3_TYPE_ARP || \
               pe->l3_type == CTC_PARSER_L3_TYPE_MPLS || pe->l3_type == CTC_PARSER_L3_TYPE_MPLS_MCAST || \
               pe->l3_type == CTC_PARSER_L3_TYPE_FCOE || pe->l3_type == CTC_PARSER_L3_TYPE_TRILL || \
               pe->l3_type == CTC_PARSER_L3_TYPE_ETHER_OAM || pe->l3_type == CTC_PARSER_L3_TYPE_SLOW_PROTO || \
               pe->l3_type == CTC_PARSER_L3_TYPE_PTP || pe->l3_type == CTC_PARSER_L3_TYPE_CMAC)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ETHER_TYPE is already known , L3 Type :%d \n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        case CTC_FIELD_KEY_IPV6_SA:
            {
                if (tcam_ipv6_single_key && ipv6_addr_mode != 0 && ipv6_addr_mode != 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ipv6 address mode is :%d \n", ipv6_addr_mode);
                    return CTC_E_INVALID_PARAM;
                }
                if (pe->key_type == SYS_SCL_KEY_TCAM_UDF_EXT && pe->l3_type != CTC_PARSER_L3_TYPE_IPV6)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Layer3 type is :%d \n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_IPV6_DA:
            {
                if (tcam_ipv6_single_key && ipv6_addr_mode == 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ipv6 address mode is :%d \n", ipv6_addr_mode);
                    return CTC_E_INVALID_PARAM;
                }
                if (pe->key_type == SYS_SCL_KEY_TCAM_UDF_EXT && pe->l3_type != CTC_PARSER_L3_TYPE_IPV6)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Layer3 type is :%d \n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_TCP_ECN:
        case CTC_FIELD_KEY_TCP_FLAGS:
            if(tcam_ipv6_single_key || tcam_ipv6_key)
            {
                /* ipv6 key */
                if(pe->key_l4_port_mode == 2 || pe->key_l4_port_mode == 3)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, l4 port mode is: %d\n", pe->key_l4_port_mode);
                    return CTC_E_INVALID_PARAM;
                }
                else
                {
                    if(pe->l4_type != CTC_PARSER_L4_TYPE_TCP)
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, l4 port mode is: %d, l4 type is %d\n", pe->key_l4_port_mode, pe->l4_type);
                        return CTC_E_INVALID_PARAM;
                    }
                }
            }

            if (tcam_ipv4_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, L3 type is :%d \n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
                else
                {
                    /* l3_type is IPV4 */
                    if (pe->key_l4_port_mode == 2 || pe->key_l4_port_mode == 3)
                    {
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, l4 port mode is: %d\n", pe->key_l4_port_mode);
                        return CTC_E_INVALID_PARAM;
                    }
                    else
                    {
                        if (pe->l4_type != CTC_PARSER_L4_TYPE_TCP)
                        {
                            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, l4 port mode is: %d, l4 type is %d\n", pe->key_l4_port_mode, pe->l4_type);
                            return CTC_E_INVALID_PARAM;
                        }
                    }
                }
            }

            if (tcam_ipv4_single_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || l3Key160ShareMode != 2 || pe->l4_type != CTC_PARSER_L4_TYPE_TCP)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TCP config is conflict, l3 type is: %d, l3Key160ShareMode is %d, l4 type is %d\n", pe->l3_type, l3Key160ShareMode, pe->l4_type);
                    return CTC_E_INVALID_PARAM;
                }
            }

            break;

        case CTC_FIELD_KEY_IP_HDR_ERROR:
        case CTC_FIELD_KEY_IP_OPTIONS:
        case CTC_FIELD_KEY_IP_FRAG:
            if (CTC_FIELD_KEY_IP_FRAG == key_field->type && \
                !DRV_IS_DUET2(lchip) && \
                (tcam_ipv4_key || tcam_udf_key || tcam_udf_ext_key || tcam_ipv6_key) && \
                !(CTC_IP_FRAG_NON == key_field->data || CTC_IP_FRAG_YES == key_field->data))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip frag only support none and yes\n");
                return CTC_E_NOT_SUPPORT;
            }
            if(tcam_ipv4_single_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || l3Key160ShareMode == 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip header error is conflict, l3 type is: %d, l3Key160ShareMode is %d\n", pe->l3_type, l3Key160ShareMode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if (tcam_ipv4_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip header error is conflict, l3 type is: %d\n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
            if (tcam_ipv4_single_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || l3Key160ShareMode != 1)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip packet length range is conflict, l3 type is: %d, l3Key160ShareMode is %d\n", pe->l3_type, l3Key160ShareMode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if (tcam_ipv4_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ip packet length range is conflict, l3 type is: %d\n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_IP_TTL:
            if(tcam_ipv4_single_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4 || l3Key160ShareMode == 1 || l3Key160ShareMode == 2)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TTL is conflict, l3 type is: %d, l3Key160ShareMode is %d\n", pe->l3_type, l3Key160ShareMode);
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if (tcam_ipv4_key)
            {
                if (pe->l3_type != CTC_PARSER_L3_TYPE_IPV4)
                {
                    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TTL is conflict, l3 type is: %d\n", pe->l3_type);
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;

        case CTC_FIELD_KEY_IS_Y1731_OAM:
            if ((CTC_PARSER_L3_TYPE_MPLS != pe->l3_type) && (CTC_PARSER_L3_TYPE_MPLS_MCAST != pe->l3_type))
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "L3 type is not MPLS l3 type:%d\n", pe->l3_type);
                return CTC_E_INVALID_PARAM;
            }
            break;

        default:
            /* no need to check key conflict */
            break;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_set_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_scl_sw_entry_t* pe, uint8 is_add)
{
    switch (key_field->type)
    {
        case CTC_FIELD_KEY_L2_TYPE:
            pe->l2_type = is_add ? key_field->data : 0;
            break;
        case  CTC_FIELD_KEY_L3_TYPE:
            pe->l3_type = is_add ? key_field->data : 0;
            break;
        case  CTC_FIELD_KEY_L4_TYPE:
            pe->l4_type = is_add ? key_field->data : 0;
            break;
        case CTC_FIELD_KEY_L4_USER_TYPE:
            pe->l4_user_type = is_add ? key_field->data : 0;
/* SYSTEM MODIFIED by yuzg for TAP */            
#ifdef TAPPRODUCT
            if (CTC_PARSER_L4_USER_TYPE_UDP_VXLAN == pe->l4_user_type)
            {
                pe->key_l4_port_mode = 2;
            }
#endif
            break;
        case CTC_FIELD_KEY_STAG_VALID:
        case CTC_FIELD_KEY_SVLAN_ID :
        case CTC_FIELD_KEY_STAG_COS:
        case CTC_FIELD_KEY_STAG_CFI :
            pe->mac_key_vlan_mode = 1;
            break;

        case CTC_FIELD_KEY_CTAG_VALID:
        case CTC_FIELD_KEY_CVLAN_ID:
        case CTC_FIELD_KEY_CTAG_COS:
        case CTC_FIELD_KEY_CTAG_CFI:
            pe->mac_key_vlan_mode = 2;
            break;

        case CTC_FIELD_KEY_MAC_DA:
            pe->mac_key_macda_mode = 1;
            break;

        case CTC_FIELD_KEY_CUSTOMER_ID:
            pe->mac_key_macda_mode = 2;
            break;

        case CTC_FIELD_KEY_VN_ID:
            pe->key_l4_port_mode = 2;
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
        case CTC_FIELD_KEY_L4_SRC_PORT:
            pe->key_l4_port_mode = 1;
            break;

        case CTC_FIELD_KEY_GRE_KEY:
        case CTC_FIELD_KEY_NVGRE_KEY:
            pe->key_l4_port_mode = 3;
            break;

        default:
            /* other situation do not need change sw table */
            break;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner)
{
    sys_scl_sw_group_t* pg_new = NULL;
    sys_scl_sw_group_t* pg     = NULL;
    int32             ret;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: group_id: %u \n", group_id);

    /*check param vaild*/
    if (!inner && group_id > CTC_SCL_GROUP_ID_MAX_NORMAL)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Invalid group id \n");
		return CTC_E_BADID;

    }
    CTC_PTR_VALID_CHECK(group_info);
    CTC_MAX_VALUE_CHECK(group_info->type, CTC_SCL_GROUP_TYPE_MAX - 1);
    SYS_SCL_CHECK_GROUP_PRIO(group_info->priority);

    if (group_info->type == CTC_SCL_GROUP_TYPE_PORT_CLASS)
    {
        SYS_SCL_PORT_CLASS_ID_CHECK(group_info->un.port_class_id);
    }
    else if (group_info->type == CTC_SCL_GROUP_TYPE_PORT)
    {
        SYS_GLOBAL_PORT_CHECK(group_info->un.gport)
    }
    else if (group_info->type == CTC_SCL_GROUP_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(group_info->un.logic_port, SYS_USW_SCL_LOGIC_PORT_MAX_NUM);
    }
    /*
    *  group_id is uint32.
    *  #1 check block_num from p_usw_scl_master. precedence cannot bigger than block_num.
    *  #2 malloc a sys_scl_sw_group_t, add to hash based on group_id.
    */
    /* check if group exist */
    _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
    if (pg)
    {
        ret = CTC_E_EXIST;
        goto error0;
    }

    /* malloc an empty group */
    MALLOC_ZERO(MEM_SCL_MODULE, pg_new, sizeof(sys_scl_sw_group_t));
    if (!pg_new)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }
    pg_new->type = group_info->type;
    pg_new->group_id   = group_id;
    pg_new->priority  = group_info->priority;

    if (CTC_SCL_GROUP_TYPE_PORT == group_info->type)
    {
        pg_new->gport = group_info->un.gport;
    }
    else if (CTC_SCL_GROUP_TYPE_LOGIC_PORT == group_info->type)
    {
        pg_new->gport = group_info->un.logic_port;
    }
    else if (CTC_SCL_GROUP_TYPE_PORT_CLASS == group_info->type)
    {
        pg_new->gport = group_info->un.port_class_id;
    }
    pg_new->entry_list = ctc_slist_new();
    if (!(pg_new->entry_list))
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;

    }
    /* add to hash */
    if (NULL == ctc_hash_insert(p_usw_scl_master[lchip]->group, pg_new))
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


STATIC int32
_sys_usw_scl_map_key_field_type(uint8 lchip, sys_scl_sw_entry_t* pe, uint16 ctc_field_type, uint8* p_sys_field_type)
{
    uint32 cmd = 0;
    SclFlowHashFieldSelect_m   ds_sel;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(p_sys_field_type);

    if (SYS_SCL_KEY_HASH_L2 == pe->key_type)
    {
        sal_memset(&ds_sel, 0, sizeof(SclFlowHashFieldSelect_m));
        cmd = DRV_IOR(SclFlowHashFieldSelect_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->hash_field_sel_id, cmd, &ds_sel));
    }

    switch(ctc_field_type)
    {
        case CTC_FIELD_KEY_PORT:
            if ((SYS_SCL_KEY_HASH_L2 == pe->key_type) && (!GetSclFlowHashFieldSelect(V, userIdLabelEn_f, &ds_sel)))
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
            }
            else
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_PORT;
            }
            break;
        case CTC_FIELD_KEY_DST_GPORT:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_DST_GPORT;
            break;
        case CTC_FIELD_KEY_CVLAN_ID:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_CVLAN_ID;
            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            if ((SYS_SCL_KEY_HASH_L2 == pe->key_type) && (!GetSclFlowHashFieldSelect(V, svlanIdEn_f, &ds_sel)))
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
            }
            else
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_SVLAN_ID;
            }
            break;
        case CTC_FIELD_KEY_MAC_SA:
            if ((SYS_SCL_KEY_HASH_L2 == pe->key_type) && (!GetSclFlowHashFieldSelect(V, macSaEn_f, &ds_sel)))
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
            }
            else
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_MAC_SA;
            }
            break;
        case CTC_FIELD_KEY_MAC_DA:
            if ((SYS_SCL_KEY_HASH_L2 == pe->key_type) && (!GetSclFlowHashFieldSelect(V, macDaEn_f, &ds_sel)))
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
            }
            else
            {
                *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_MAC_DA;
            }
            break;
        case CTC_FIELD_KEY_IP_SA:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_IP_SA;
            break;
        case CTC_FIELD_KEY_IP_DA:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_IP_DA;
            break;
        case CTC_FIELD_KEY_IPV6_SA:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_IPV6_SA;
            break;
        case CTC_FIELD_KEY_IPV6_DA:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_IPV6_DA;
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_L4_SRC_PORT;
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_L4_DST_PORT;
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_GRE_KEY;
            break;
        case CTC_FIELD_KEY_VN_ID:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_VN_ID;
            break;
        case CTC_FIELD_KEY_MPLS_LABEL0:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL0;
            break;
        case CTC_FIELD_KEY_MPLS_LABEL1:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL1;
            break;
        case CTC_FIELD_KEY_MPLS_LABEL2:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL2;
            break;

        default:
            *p_sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;  /* other situation do not need map */
            break;
    }

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_scl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key)
{
    int32 ret = CTC_E_NONE;
    sys_scl_sw_entry_t* pe = NULL;
    uint8 sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
	uint8 hash_key = 0 ;
	ds_t  ext_mask_temp   = {0};

    CTC_PTR_VALID_CHECK(p_field_key);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_key_type %u\n", entry_id, p_field_key->type);

    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if (!pe )
    {
        return CTC_E_NOT_EXIST;
    }

    if (!pe->temp_entry || !pe->uninstall)
    {
        return CTC_E_NOT_READY;
    }

	hash_key = !SCL_ENTRY_IS_TCAM(pe->key_type) && (!pe->resolve_conflict);

    if (hash_key && pe->hash_valid)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_scl_add_key_field, eid:%u hash_valid:%d\n", pe->entry_id, pe->hash_valid);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
		return CTC_E_EXIST;
    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        if (p_field_key->ext_mask == NULL)
        {
            /* tcam entry and ext_mask is NULL */
            sal_memset(ext_mask_temp, 0xFF, sizeof(ds_t));
            p_field_key->ext_mask = ext_mask_temp;
        }
/* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
        if (CTC_FIELD_KEY_VN_ID != p_field_key->type && CTC_FIELD_KEY_GRE_KEY != p_field_key->type)
#endif
        p_field_key->mask = p_field_key->mask ? p_field_key->mask : 0xFFFFFFFF;
    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_ERROR_RETURN(_sys_usw_scl_check_tcam_key_union(lchip, p_field_key, pe, 1));
    }

    if (p_usw_scl_master[lchip]->build_key_func[pe->key_type])
    {
        CTC_ERROR_RETURN(p_usw_scl_master[lchip]->build_key_func[pe->key_type](lchip, p_field_key, pe, TRUE));
    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_ERROR_RETURN(_sys_usw_scl_set_tcam_key_union(lchip, p_field_key, pe, 1));
    }

    if (CTC_FIELD_KEY_HASH_VALID == p_field_key->type && hash_key && !pe->hash_valid)
    {
        ret = _sys_usw_scl_asic_hash_lkup(lchip, pe, 0, 0, NULL);
    }

    _sys_usw_scl_map_key_field_type(lchip, pe, p_field_key->type, &sys_field_type);

    if (SYS_SCL_SHOW_FIELD_KEY_NUM != sys_field_type)
    {
        CTC_BIT_SET(pe->key_bmp, sys_field_type);
    }
    if (CTC_FIELD_KEY_NVGRE_KEY == p_field_key->type)
    {
        CTC_BIT_UNSET(pe->key_bmp, SYS_SCL_SHOW_FIELD_KEY_GRE_KEY);
    }

    return ret;

}

int32
_sys_usw_scl_get_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    sys_scl_sw_entry_t* pe;

    SYS_SCL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_field_action);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_key_type %u\n", entry_id, p_field_action->type);

    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if(pe)
    {
        if(!CTC_BMP_ISSET(pe->action_bmp, p_field_action->type))
        {
            return CTC_E_NOT_EXIST;
        }

        if(pe->uninstall)
        {
            CTC_ERROR_RETURN(p_usw_scl_master[lchip]->get_ad_func[pe->action_type](lchip, p_field_action, pe));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_scl_resume_buffer_from_hw(lchip, pe));
            CTC_ERROR_RETURN(p_usw_scl_master[lchip]->get_ad_func[pe->action_type](lchip, p_field_action, pe));
            mem_free(pe->temp_entry);
            pe->temp_entry = NULL;
        }
    }
    else
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_get_default_action(uint8 lchip, sys_scl_default_action_t* p_default_action)
{
    uint32 cmd   = 0;
    DsUserId_m ds_userid;
    DsVlanXlateDefault_m   vlanXlate;
    uint32 tbl_id = 0;
    uint16 lport = p_default_action->lport;
    sys_scl_sw_entry_t *pe = NULL;
    uint8 step = 0;
    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_default_action);
    CTC_MAX_VALUE_CHECK(p_default_action->scl_id, MCHIP_CAP(SYS_CAP_SCL_HASH_NUM)-1);

    sal_memset(&ds_userid, 0, sizeof(DsUserId_m));
    sal_memset(&vlanXlate, 0, sizeof(DsVlanXlateDefault_m));

    if (SYS_SCL_ACTION_INGRESS == p_default_action->action_type)
    {
        if (p_default_action->is_vlan_class)
        {
            _sys_usw_scl_get_nodes_by_eid(lchip, p_default_action->eid, &pe);

            if (!pe)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
                return CTC_E_NOT_EXIST;
            }

            /* for tacm entry, key index is equal to ad_index in the scale of 80 bit regardless of key type,
            so in our per entry sw table, the key index and ad_index is 80bit not in the form of hw table */
            p_default_action->ad_index[0] = pe->key_index;
            step = SYS_SCL_GET_STEP(p_usw_scl_master[lchip]->fpa_size[pe->key_type]);
            p_default_action->ad_index[0] = SYS_SCL_MAP_DRV_AD_INDEX(p_default_action->ad_index[0], step);

            if (!pe->group)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
                return CTC_E_NOT_EXIST;
            }
            else
            {
                switch(pe->group->priority)
                {
                    case 0 :
                        tbl_id = DsUserId0Tcam_t;
                        break;
                    case 1 :
                        tbl_id = DsUserId1Tcam_t;
                        break;
                    case 2 :
                        tbl_id = DsUserId2_t;
                        break;
                    case 3 :
                        tbl_id = DsUserId3_t;
                        break;
                    default :
                        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid scl id \n");
                        return CTC_E_INTR_INVALID_PARAM;
                }
            }
        }
        else
        {
            if (lport  >= SYS_USW_MAX_PORT_NUM_PER_CHIP)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Invalid default port\n");
                return CTC_E_INVALID_PORT;

            }
            p_default_action->ad_index[p_default_action->scl_id] = p_usw_scl_master[lchip]->igs_default_base[p_default_action->scl_id] + 2 * lport;

            tbl_id = (p_default_action->scl_id == 0? DsUserId_t : DsUserId1_t);
        }
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_default_action->ad_index[p_default_action->scl_id] , cmd, &ds_userid));

        /* the sequence can not be changed */
        if (GetDsUserId(V, userIdExceptionEn_f, &ds_userid))
        {/*To CPU*/
            p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_TO_CPU;
        }
        else if (GetDsUserId(V, dsFwdPtrValid_f, &ds_userid)
                && GetDsUserId(V, dsFwdPtr_f, &ds_userid) == 0xFFFF)
        { /*drop*/
            p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_DISCARD;
        }
        else if (GetDsUserId(V, userVlanPtr_f, &ds_userid) != 0 )
        {/*user vlanptr*/
            p_default_action->vlan_miss->user_vlanptr = GetDsUserId(V, userVlanPtr_f, &ds_userid);
            p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR;
        }

        else if (GetDsUserId(V, vlanActionProfileValid_f, &ds_userid))

        {/*vlan id*/
            p_default_action->vlan_miss->new_svid = GetDsUserId(V, userSvlanId_f, &ds_userid);
            p_default_action->vlan_miss->new_scos =  GetDsUserId(V, userScos_f, &ds_userid);
            p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_APPEND_STAG;
        }
        else
        {
           p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
        }
    }
    else if (SYS_SCL_ACTION_EGRESS == p_default_action->action_type)
    {
        p_default_action->ad_index[0]  = lport;
        cmd = DRV_IOR(DsVlanXlateDefault_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_default_action->ad_index[0], cmd, &vlanXlate));

        if(GetDsVlanXlateDefault(V, discard_f, &vlanXlate))
        {
          p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_DISCARD;
        }
        else
        {
          p_default_action->vlan_miss->flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
        }

    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_show_vlan_profile(void *node, void* user_data)
{
    uint32 *cnt = (uint32 *)user_data;
    uint32 ref_cnt = 0;
    sys_scl_sp_vlan_edit_t *vlan_profile = (sys_scl_sp_vlan_edit_t *)(((ctc_spool_node_t*)node)->data);

    ref_cnt = ((ctc_spool_node_t*)node)->ref_cnt;

    if (ref_cnt == 0xFFFFFFFF)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5d %10d %10s\n", (*cnt)++, vlan_profile->profile_id, "-");
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5d %10d %10u\n", (*cnt)++, vlan_profile->profile_id, ref_cnt);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_scl_map_drv_tunnel_type(uint8 lchip, uint8 key_type, uint8* p_drv_key_type)
{
    uint8 drv_tmp_key_type = 0;

    switch(key_type)
    {
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4GREKEY;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4RPF;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
            drv_tmp_key_type = DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA);
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6GREKEY;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6UDP;
            break;
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6DA;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4NVGREMODE1;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0;
            break;
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4VXLANMODE1;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0;
            break;
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1;
            break;
        case SYS_SCL_KEY_HASH_TRILL_UC:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELTRILLUCDECAP;
            break;
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELTRILLUCRPF;
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELTRILLMCDECAP;
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELTRILLMCRPF;
            break;
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELTRILLMCADJ;
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV4CAPWAP;
            break;
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
            drv_tmp_key_type = USERIDHASHTYPE_TUNNELIPV6CAPWAP;
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }

    *p_drv_key_type = drv_tmp_key_type - USERIDHASHTYPE_TUNNELIPV4;

    return CTC_E_NONE;
}

#define __scl_show_function__
typedef struct
{
    uint32             count;
    uint8              detail;
    uint8              lchip;
    uint8              care_key_type;
    uint8              care_grp_mod_id;
    uint8              grp_mod_id;
    uint8              show_type;
    uint8              grep_cnt;
    sys_scl_key_type_t key_type;
    ctc_linklist_t*    p_linklist;
    ctc_field_key_t*   p_grep;
} _scl_cb_para_t;

STATIC int32 _sys_usw_scl_linklist_entry_cmp(void* bucket_data1, void* bucket_data2)
{
    sys_scl_sw_entry_t* pe1 = (sys_scl_sw_entry_t*) bucket_data1;
    sys_scl_sw_entry_t* pe2 = (sys_scl_sw_entry_t*) bucket_data2;

	if(pe1->entry_id < pe2->entry_id)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

STATIC int32 _sys_usw_scl_linklist_group_cmp(void* bucket_data1, void* bucket_data2)
{
    sys_scl_sw_group_t* pg1 = (sys_scl_sw_group_t*) bucket_data1;
    sys_scl_sw_group_t* pg2 = (sys_scl_sw_group_t*) bucket_data2;

	if(pg1->group_id < pg2->group_id)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

STATIC int32 _sys_usw_scl_store_data_to_list(void* node_data, void* user_data)
{
    ctc_linklist_t* p_list = (ctc_linklist_t*)user_data;
	ctc_listnode_add_sort(p_list, node_data);
	return CTC_E_NONE;
}

STATIC char*
_sys_usw_scl_get_type(uint8 lchip, uint8 key_type)
{
    switch (key_type)
    {
        case SYS_SCL_KEY_TCAM_MAC:
            return "tcam_mac";
        case SYS_SCL_KEY_TCAM_IPV4:
            return "tcam_ipv4";
        case SYS_SCL_KEY_TCAM_IPV4_SINGLE:
            return "tcam_ipv4_single";
        case SYS_SCL_KEY_TCAM_IPV6:
            return "tcam_ipv6";
        case SYS_SCL_KEY_TCAM_IPV6_SINGLE:
            return "tcam_ipv6_single";
        case SYS_SCL_KEY_TCAM_UDF:
            return "tcam_udf";
        case SYS_SCL_KEY_TCAM_UDF_EXT:
            return "tcam_udf_ext";
        case SYS_SCL_KEY_HASH_PORT:
            return "hash_port";
        case SYS_SCL_KEY_HASH_PORT_CVLAN:
            return "hash_port_cvlan";
        case SYS_SCL_KEY_HASH_PORT_SVLAN:
            return "hash_port_svlan";
        case SYS_SCL_KEY_HASH_PORT_2VLAN:
            return "hash_port_2vlan";
        case SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP:
            return "hash_port_svlan_dscp";
        case SYS_SCL_KEY_HASH_PORT_CVLAN_COS:
            return "hash_port_cvlan_cos";
        case SYS_SCL_KEY_HASH_PORT_SVLAN_COS:
            return "hash_port_svlan_cos";
        case SYS_SCL_KEY_HASH_MAC:
            return "hash_mac";
        case SYS_SCL_KEY_HASH_PORT_MAC:
            return "hash_port_mac";
        case SYS_SCL_KEY_HASH_IPV4:
            return "hash_ipv4";
        case SYS_SCL_KEY_HASH_SVLAN:
            return "hash_svlan";
        case SYS_SCL_KEY_HASH_SVLAN_MAC:
            return "hash_svlan_mac";
        case SYS_SCL_KEY_HASH_PORT_IPV4:
            return "hash_port_ipv4";
        case SYS_SCL_KEY_HASH_IPV6:
            return "hash_ipv6";
        case SYS_SCL_KEY_HASH_PORT_IPV6:
            return "hash_port_ipv6";
        case SYS_SCL_KEY_HASH_L2:
            return "hash_l2";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4:
            return "hash_tunnel_ipv4";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA:
            return "hash_tunnel_ipv4_da";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE:
            return "hash_tunnel_ipv4_gre";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF:
            return "hash_tunnel_ipv4_rpf";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6:
            return "hash_tunnel_ipv6";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA:
            return "hash_tunnel_ipv6_da";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY:
            return "hash_tunnel_ipv6_gre";
        case SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP:
            return "hash_tunnel_ipv6_udp";
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0:
            return "hash_nvgre_uc_v4_mode0";
        case SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1:
            return "hash_nvgre_uc_v4_mode1";
        case SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0:
            return "hash_nvgre_mc_v4_mode0";
        case SYS_SCL_KEY_HASH_NVGRE_V4_MODE1:
            return "hash_nvgre_v4_mode";
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0:
            return "hash_nvgre_uc_v6_mode0";
        case SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1:
            return "hash_nvgre_uc_v6_mode1";
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0:
            return "hash_nvgre_mc_v6_mode0";
        case SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1:
            return "hash_nvgre_mc_v6_mode1";
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0:
            return "hash_vxlan_uc_v4_mode0";
        case SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1:
            return "hash_vxlan_uc_v4_mode1";
        case SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0:
            return "hash_vxlan_mc_v4_mode0";
        case SYS_SCL_KEY_HASH_VXLAN_V4_MODE1:
            return "hash_vxlan_v4_mode1";
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0:
            return "hash_vxlan_uc_v6_mode0";
        case SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1:
            return "hash_vxlan_uc_v6_mode1";
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0:
            return "hash_vxlan_mc_v6_mode0";
        case SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1:
            return "hash_vxlan_mc_v6_mode1";
        case SYS_SCL_KEY_HASH_WLAN_IPV4:
            return "hash_capwap_ipv4";
        case SYS_SCL_KEY_HASH_WLAN_IPV6:
            return "hash_capwap_ipv6";
        case SYS_SCL_KEY_HASH_WLAN_RMAC:
            return "hash_capwap_rmac";
        case SYS_SCL_KEY_HASH_WLAN_RMAC_RID:
            return "hash_capwap_rmac_rid";
        case SYS_SCL_KEY_HASH_WLAN_STA_STATUS:
            return "hash_capwap_sta_status";
        case SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS:
            return "hash_capwap_mc_sta_status";
        case SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD:
            return "hash_capwap_vlan_forward";
        case SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD:
            return "hash_capwap_macda_forward";
        case SYS_SCL_KEY_HASH_TUNNEL_MPLS:
            return "hash_tunnel_mpls";
        case SYS_SCL_KEY_HASH_ECID:
            return "hash_ecid";
        case SYS_SCL_KEY_HASH_ING_ECID:
            return "hash_src_ecid";
        case SYS_SCL_KEY_HASH_PORT_CROSS:
            return "hash_port_cross";
        case SYS_SCL_KEY_HASH_PORT_VLAN_CROSS:
            return "hash_port_vlan_cross";
        case SYS_SCL_KEY_HASH_TRILL_UC:
            return "hash_trill_uc";
        case SYS_SCL_KEY_HASH_TRILL_UC_RPF:
            return "hash_trill_uc_rpf";
        case SYS_SCL_KEY_HASH_TRILL_MC:
            return "hash_trill_mc";
        case SYS_SCL_KEY_HASH_TRILL_MC_RPF:
            return "hash_trill_mc_rpf";
        case SYS_SCL_KEY_HASH_TRILL_MC_ADJ:
            return "hash_trill_mc_adj";
        case SYS_SCL_KEY_PORT_DEFAULT_INGRESS:
            return "port_default_ingress";
        case SYS_SCL_KEY_PORT_DEFAULT_EGRESS:
            return "port_default_egress";
        default:
            return "XXXXXX";
    }
}
/*
 * show scl action
 */
STATIC int32
_sys_usw_scl_show_igs_action(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    int32  ret = 0;
    ctc_scl_field_action_t field_action;
    ctc_scl_qos_map_t qos;
    ctc_scl_logic_port_t logic_port;
    ctc_scl_vlan_edit_t vlan_edit;
    ctc_scl_aps_t aps;
    ctc_scl_bind_t bind;

    char  * tag_op[CTC_VLAN_TAG_OP_MAX];
    char  * tag_sl[CTC_VLAN_TAG_SL_MAX];
    char  * domain_sl[CTC_SCL_VLAN_DOMAIN_MAX];

    tag_op[CTC_VLAN_TAG_OP_NONE]       = "None";
    tag_op[CTC_VLAN_TAG_OP_REP_OR_ADD] = "Replace or Add";
    tag_op[CTC_VLAN_TAG_OP_ADD]        = "Add";
    tag_op[CTC_VLAN_TAG_OP_DEL]        = "Delete";
    tag_op[CTC_VLAN_TAG_OP_REP]        = "Replace";
    tag_op[CTC_VLAN_TAG_OP_VALID]      = "Valid";

    tag_sl[CTC_VLAN_TAG_SL_AS_PARSE]    = "As parser";
    tag_sl[CTC_VLAN_TAG_SL_ALTERNATIVE] = "Alternative";
    tag_sl[CTC_VLAN_TAG_SL_NEW]         = "New";
    tag_sl[CTC_VLAN_TAG_SL_DEFAULT]     = "Default";

    domain_sl[CTC_SCL_VLAN_DOMAIN_SVLAN]    = "Svlan domain";
    domain_sl[CTC_SCL_VLAN_DOMAIN_CVLAN]  = "Cvlan domain";
    domain_sl[CTC_SCL_VLAN_DOMAIN_UNCHANGE]  = "Unchange";

    CTC_PTR_VALID_CHECK(pe);
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&qos, 0, sizeof(qos));
    sal_memset(&logic_port, 0, sizeof(logic_port));
    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));
    sal_memset(&aps, 0, sizeof(aps));
    sal_memset(&bind, 0, sizeof(bind));

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL ingress action\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DISCARD\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "STATS", pe->stats_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    field_action.ext_data = &vlan_edit;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if (ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if (ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: \n", "VLAN_EDIT");
        if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%25s - %s\n", "Stag_op", tag_op[vlan_edit.stag_op]);

            if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "svid_sl", tag_sl[vlan_edit.svid_sl], "New_svid", vlan_edit.svid_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "scos_sl", tag_sl[vlan_edit.scos_sl], "New_scos", vlan_edit.scos_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "scfi_sl", tag_sl[vlan_edit.scfi_sl], "New_scfi", vlan_edit.scfi_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12u\n", "tpid_idx", vlan_edit.tpid_index);
            }
        }
        if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%25s - %s\n", "Ctag_op", tag_op[vlan_edit.ctag_op]);
            if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "cvid_sl", tag_sl[vlan_edit.cvid_sl], "New_cvid", vlan_edit.cvid_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "ccos_sl", tag_sl[vlan_edit.ccos_sl], "New_ccos", vlan_edit.ccos_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%27s - %-12s  %s - %u\n", "ccfi_sl", tag_sl[vlan_edit.ccfi_sl], "New_ccfi", vlan_edit.ccfi_new);
            }
        }
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%29s - %s\n", "Vlan domain", domain_sl[vlan_edit.vlan_domain]);
        if (pe->vlan_edit)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%40s - %d\n", "Vlan action profile id", pe->vlan_edit->profile_id);
        }
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  COPY_TO_CPU\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "REDIRECT", pe->nexthop_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "FID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "VRFID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "POLICER_ID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u %-16s: %u\n", "COS_POLICER_ID", field_action.data0, "COS_INDEX",  field_action.data1);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SRC_CID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "SRC_CID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DST_CID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "DST_CID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  SPAN_FLOW\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%04x\n", "REDIRECT_PORT", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
    field_action.ext_data = &qos;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "QOS_MAP");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dscp - %u", qos.dscp);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  priority - %u", qos.priority);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  color - %u\n", qos.color);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    field_action.ext_data = &logic_port;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "LOGIC_PORT");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "logic port - %d  logic port type - %d\n", logic_port.logic_port, logic_port.logic_port_type);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "USER_VLANPTR", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_APS;
    field_action.ext_data = &aps;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if (ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if (ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "APS");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "aps protected vlan - %d  aps select group id - %d  is working path - %d\n",
                             aps.protected_vlan, aps.aps_select_group_id, aps.is_working_path);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if (ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if (ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  ETREE_LEAF\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STP_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "STP_ID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "PTP_CLOCK_ID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "SERVICE_ID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  SERVICE_ACL_EN\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  LOGIC_PORT_SECURITY_EN\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
    field_action.ext_data = &bind;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        uint32 ip_sa = 0;
        uint32 addr;
        char   ip_addr[16];

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: Type - ", "BINDING_EN");

        switch (bind.type)
        {
            case CTC_SCL_BIND_TYPE_PORT:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Port  Value - 0x%X\n", bind.gport);
                break;

            case CTC_SCL_BIND_TYPE_VLAN:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Vlan  Value - %u\n", bind.vlan_id);
                break;

            case CTC_SCL_BIND_TYPE_IPV4SA:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPv4sa  Value - ");
                ip_sa = bind.ipv4_sa;
                addr  = sal_ntohl(ip_sa);
                sal_inet_ntop(AF_INET, &addr, ip_addr, sizeof(ip_addr));
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", ip_addr);
                break;

            case CTC_SCL_BIND_TYPE_IPV4SA_VLAN:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Vlan and IPv4sa  Value - %u", bind.vlan_id);

                ip_sa = bind.ipv4_sa;
                addr  = sal_ntohl(ip_sa);
                sal_inet_ntop(AF_INET, &addr, ip_addr, sizeof(ip_addr));
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %s\n", ip_addr);
                break;

            case CTC_SCL_BIND_TYPE_MACSA:
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Macsa  Value - %.4x.%.4x.%.4x%s ", sal_ntohs(*(unsigned short*)&bind.mac_sa[0]),
                                 sal_ntohs(*(unsigned short*)&bind.mac_sa[2]),
                                 sal_ntohs(*(unsigned short*)&bind.mac_sa[4]), " ");
                break;

            default:
                return CTC_E_NOT_SUPPORT;

        }

    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DENY_BRIDGE\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "L2vpn_oam_id", field_action.data1);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", "Router_Mac_Match", (field_action.data0? "Enable": "Disable"));
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", "Mac_Limit", (CTC_MACLIMIT_ACTION_TOCPU == field_action.data0? "To CPU": "Discard"));
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  MAC_LIMIT_DISCARD_EN\n");
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_VN_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "VN_ID", field_action.data0);
    }

    return CTC_E_NONE;
}


/*
 * show scl action
 */
STATIC int32
_sys_usw_scl_show_egs_action(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;

    char* tag_op[CTC_VLAN_TAG_OP_MAX];
    char* tag_sl[CTC_VLAN_TAG_SL_MAX];
    tag_op[CTC_VLAN_TAG_OP_NONE]       = "None";
    tag_op[CTC_VLAN_TAG_OP_REP_OR_ADD] = "Replace or Add";
    tag_op[CTC_VLAN_TAG_OP_ADD]        = "Add";
    tag_op[CTC_VLAN_TAG_OP_DEL]        = "Delete";
    tag_op[CTC_VLAN_TAG_OP_REP]        = "Replace";
    tag_op[CTC_VLAN_TAG_OP_VALID]      = "Valid";

    tag_sl[CTC_VLAN_TAG_SL_AS_PARSE]    = "As parser   ";
    tag_sl[CTC_VLAN_TAG_SL_ALTERNATIVE] = "Alternative";
    tag_sl[CTC_VLAN_TAG_SL_NEW]         = "New        ";
    tag_sl[CTC_VLAN_TAG_SL_DEFAULT]     = "Default    ";

    CTC_PTR_VALID_CHECK(pe);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL Egress action\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");


    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DISCARD\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-12s: %u\n", "STATS", pe->stats_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    field_action.ext_data = &vlan_edit;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if (ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if (ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-12s: \n", "VLAN_EDIT");
        if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%21s - %s\n", "Stag_op", tag_op[vlan_edit.stag_op]);
            if (vlan_edit.stag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "svid_sl", tag_sl[vlan_edit.svid_sl], "New_svid", vlan_edit.svid_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "scos_sl", tag_sl[vlan_edit.scos_sl], "New_scos", vlan_edit.scos_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "scfi_sl", tag_sl[vlan_edit.scfi_sl], "New_scfi", vlan_edit.scfi_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12u\n", "tpid_idx", vlan_edit.tpid_index);
            }
        }
        if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_NONE)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%21s - %s\n", "Ctag_op", tag_op[vlan_edit.ctag_op]);
            if (vlan_edit.ctag_op != CTC_VLAN_TAG_OP_DEL)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "cvid_sl", tag_sl[vlan_edit.cvid_sl], "New_cvid", vlan_edit.cvid_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "ccos_sl", tag_sl[vlan_edit.ccos_sl], "New_ccos", vlan_edit.ccos_new);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%23s - %-12s  %s - %u\n", "ccfi_sl", tag_sl[vlan_edit.ccfi_sl], "New_ccfi", vlan_edit.ccfi_new);
            }
        }

    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DROP_UNKNOWN_UCAST\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DROP_UNKNOWN_MCAST\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DROP_BCAST\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DROP_KNOWN_UCAST\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DROP_KNOWN_MCAST\n");
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_show_tunnel_action(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    CTC_PTR_VALID_CHECK(pe);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL tunnel action:\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_show_flow_action(uint8 lchip, sys_scl_sw_entry_t* pe)
{
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    ctc_scl_qos_map_t qos;
    ctc_acl_property_t acl;
    ctc_scl_force_decap_t decap;
    ctc_scl_snooping_parser_t snooping_parser;
    CTC_PTR_VALID_CHECK(pe);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&qos, 0, sizeof(ctc_scl_qos_map_t));
    sal_memset(&acl, 0, sizeof(ctc_acl_property_t));
    sal_memset(&decap, 0, sizeof(ctc_scl_force_decap_t));
    sal_memset(&snooping_parser, 0, sizeof(ctc_scl_snooping_parser_t));


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL Flow action:\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "Load balance hash select id", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DISCARD\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "STATS", pe->stats_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  COPY_TO_CPU\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "REDIRECT", pe->nexthop_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "FID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "VRFID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "POLICER_ID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  SPAN_FLOW\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
    field_action.ext_data = &qos;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "QOS_MAP");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dscp - %u", qos.dscp);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  priority - %u", qos.priority);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  color - %u\n", qos.color);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_METADATA;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x\n", "METADATA", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DENY_BRIDGE\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DENY_LEARNING\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  DENY_ROUTE\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  FORCE_BRIDGE\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  FORCE_LEARNING\n");

    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  FORCE_ROUTE\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN;
    field_action.ext_data = &acl;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "ACL_TCAM_EN");
        /* ctc type */
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ACL flow tcam type - %u", acl.tcam_lkup_type);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  ACL flow tcam label - %u", acl.class_id);
        if(pe->acl_profile)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  profile id - %u", pe->acl_profile->profile_id);
        }
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN;
    field_action.ext_data = &acl;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "ACL_HASH_EN");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ACL flow hash type - %u", acl.hash_lkup_type);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  ACL flow hash field sel - %u\n", acl.hash_field_sel_id);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "OUTER_HEAD", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP;
    field_action.ext_data = &decap;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "FORCE_DECAP");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "offset_base_type - ");
        if (CTC_SCL_OFFSET_BASE_TYPE_L2 == decap.offset_base_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2");
        }
        else if (CTC_SCL_OFFSET_BASE_TYPE_L3 == decap.offset_base_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L3");
        }
        else if (CTC_SCL_OFFSET_BASE_TYPE_L4 == decap.offset_base_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L4");
        }
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  ext_offset - %u", decap.ext_offset);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  payload_type - %u", decap.payload_type);
        if (DRV_FLD_IS_EXISIT(DsSclFlow_t, DsSclFlow_ttlUpdate_f))
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  use_outer_ttl - %u\n", decap.use_outer_ttl);
        }
        else
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        }
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER;
    field_action.ext_data = &snooping_parser;
    ret = _sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    else if(ret == CTC_E_NONE)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: ", "SNOOPING_PARSER");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "start_offset_type - ");
        if (CTC_SCL_OFFSET_BASE_TYPE_L2 == snooping_parser.start_offset_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2");
        }
        else if (CTC_SCL_OFFSET_BASE_TYPE_L3 == snooping_parser.start_offset_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L3");
        }
        else if (CTC_SCL_OFFSET_BASE_TYPE_L4 == snooping_parser.start_offset_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L4");
        }
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  ext_offset - %u", snooping_parser.ext_offset);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  payload_type - %u", snooping_parser.payload_type);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  use_inner_hash_en - %u\n", snooping_parser.use_inner_hash_en);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SRC_CID;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "SRC_CID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DST_CID;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        CTC_ERROR_RETURN(_sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "DST_CID", field_action.data0);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    if (CTC_BMP_ISSET(pe->action_bmp, field_action.type))
    {
        ctc_scl_logic_port_t logic_port;
        sal_memset(&logic_port, 0, sizeof(ctc_scl_logic_port_t));
        field_action.ext_data = (void*)&logic_port;
        CTC_ERROR_RETURN(_sys_usw_scl_get_action_field(lchip, pe->entry_id, &field_action));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", "Logic port", logic_port.logic_port);
    }

    return CTC_E_NONE;
}

/* data_type: 1 uint32; 2 ext_data */
STATIC int32
sys_usw_scl_map_key_field_to_field_name(uint8 lchip, sys_scl_sw_entry_t* pe, uint16 key_field_type, uint32 key_id, uint8* data_type, char** field_name, uint8* start_bit, uint8* bit_len, uint8* field_name_cnt, char** print_str, uint32* p_key_size)
{
    uint32 drv_data_temp[4] = {0,0,0,0};
    uint8 is_hash = !SCL_ENTRY_IS_TCAM(pe->key_type);
    fld_id_t field_id[4] = {0,0,0,0};
    char* str = NULL;
    char* field_str0 = NULL;
    char* field_str1 = NULL;
    uint32 key_size = 4;
    SclFlowHashFieldSelect_m   ds_sel;
    uint32 cmd = 0;

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

    cmd = DRV_IOR(SclFlowHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->hash_field_sel_id, cmd, &ds_sel));

    switch(key_field_type)
    {
        case CTC_FIELD_KEY_MAC_SA:
            if (pe->resolve_conflict)
            {
                if (SYS_SCL_KEY_HASH_MAC == pe->key_type)
                {
                    field_name[0] = "u1_gUserMac_macSa";
                }
                else if (SYS_SCL_KEY_HASH_PORT_MAC == pe->key_type)
                {
                    field_name[0] = "u1_gUserMacPort_macSa";
                }
                else if (SYS_SCL_KEY_HASH_WLAN_STA_STATUS == pe->key_type)
                {
                    field_name[0] = "u1_gStaStatus_mac";
                }
                else if (SYS_SCL_KEY_HASH_L2 == pe->key_type)
                {
                    if (GetSclFlowHashFieldSelect(V, macSaEn_f, &ds_sel))
                    {
                        field_name[0] = "u1_gSclFlowL2_macSa";
                    }
                }
                else
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if (SYS_SCL_KEY_HASH_WLAN_STA_STATUS == pe->key_type)
            {
                field_name[0] = "mac";
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
            if (pe->resolve_conflict)
            {
                if (SYS_SCL_KEY_HASH_WLAN_STA_STATUS == pe->key_type)
                {
                    field_name[0] = "u1_gStaStatus_mac";
                }
                else if (SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS == pe->key_type)
                {
                    field_name[0] = "u1_gStaStatusMc_mac";
                }
                else if (SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD == pe->key_type)
                {
                    field_name[0] = "u1_gCapwapMacDaFwd_mac";
                }
                else if (SYS_SCL_KEY_HASH_MAC == pe->key_type)
                {
                    field_name[0] = "u1_gUserMac_macSa";
                }
                else if (SYS_SCL_KEY_HASH_PORT_MAC == pe->key_type)
                {
                    field_name[0] = "u1_gUserMacPort_macSa";
                }
                else if (SYS_SCL_KEY_HASH_L2 == pe->key_type)
                {
                    if (GetSclFlowHashFieldSelect(V, macDaEn_f, &ds_sel))
                    {
                        field_name[0] = "u1_gSclFlowL2_macDa";
                    }
                }
                else
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            else if ((SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD == pe->key_type) || (SYS_SCL_KEY_HASH_WLAN_STA_STATUS == pe->key_type) ||(SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS == pe->key_type))
            {
                field_name[0] = "mac";
            }
            else if ((SYS_SCL_KEY_HASH_L2 == pe->key_type) ||(SYS_SCL_KEY_TCAM_MAC == pe->key_type) ||
                      (SYS_SCL_KEY_TCAM_IPV4 == pe->key_type) ||(SYS_SCL_KEY_TCAM_IPV6 == pe->key_type))
            {
                field_name[0] = "macDa";
            }
            else
            {
                field_name[0] = "macSa";
            }
            str = "Mac Da";
            *data_type = 2;
            key_size = sizeof(mac_addr_t);
            break;
        case CTC_FIELD_KEY_SVLAN_ID:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_PORT_SVLAN == pe->key_type)
                    {
                        field_name[0] = "u1_gUserSvlanPort_svlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_2VLAN == pe->key_type)
                    {
                        field_name[0] = "u1_gUserDoubleVlan_svlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_SVLAN_COS == pe->key_type)
                    {
                        field_name[0] = "u1_gUserSvlanCosPort_svlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD == pe->key_type)
                    {
                        field_name[0] = "u1_gCapwapVlanFwd_vlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS == pe->key_type)
                    {
                        field_name[0] = "u1_gStaStatusMc_svlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_L2 == pe->key_type)
                    {
                        if (GetSclFlowHashFieldSelect(V, svlanIdEn_f, &ds_sel))
                        {
                            field_name[0] = "u1_gSclFlowL2_svlanId";
                        }
                    }
                    else
                    {
                        return CTC_E_INVALID_PARAM;
                    }
                }
                else if ((SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD == pe->key_type) || (SYS_SCL_KEY_TCAM_MAC == pe->key_type) || (SYS_SCL_KEY_HASH_PORT_VLAN_CROSS == pe->key_type))
                {
                    field_name[0] = "vlanId";
                }
                else
                {
                    field_name[0] = "svlanId";
                }
                str = "Svlan ID";
                break;
            }
        case CTC_FIELD_KEY_CVLAN_ID:
            {
                if (pe->resolve_conflict && (CTC_INGRESS == pe->direction))
                {
                    if (SYS_SCL_KEY_HASH_PORT_CVLAN == pe->key_type)
                    {
                        field_name[0] = "u1_gUserCvlanPort_cvlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_2VLAN == pe->key_type)
                    {
                        field_name[0] = "u1_gUserDoubleVlan_cvlanId";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_CVLAN_COS == pe->key_type)
                    {
                        field_name[0] = "u1_gUserCvlanCosPort_cvlanId";
                    }
                }
                else if ((SYS_SCL_KEY_TCAM_MAC == pe->key_type) || (SYS_SCL_KEY_HASH_PORT_VLAN_CROSS == pe->key_type))
                {
                    field_name[0] = "vlanId";
                }
                else
                {
                    field_name[0] = "cvlanId";
                }
                str = "Cvlan ID";
                break;
            }
        case CTC_FIELD_KEY_IP_SA:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_IPV4 == pe->key_type)
                    {
                        field_name[0] = "u1_gUserIpv4Sa_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_IPV4 == pe->key_type)
                    {
                        field_name[0] = "u1_gUserIpv4Port_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcNvgreMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4NvgreMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcVxlanMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4VxlanMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_IPV4 == pe->key_type)
                    {
                        field_name[0] = "u1_gCapwapIpv4_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4Rpf_ipSa";
                    }
                }
                else
                {
                    field_name[0] = "ipSa";
                }
                str = "IP SA";
                break;
            }
        case CTC_FIELD_KEY_IP_DA:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcNvgreMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcNvgreMode1_ipDaIndex";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4McNvgreMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4NvgreMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcVxlanMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcVxlanMode1_ipDaIndex";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4McVxlanMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4VxlanMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_IPV4 == pe->key_type)
                    {
                        field_name[0] = "u1_gCapwapIpv4_ipDa";
                    }
                }
                else
                {
                    field_name[0] = "ipDa";
                }
                str = "IP DA";
                break;
            }
        case CTC_FIELD_KEY_IPV6_SA:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gUserIpv6Sa_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gUserIpv6Port_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcNvgreMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McNvgreMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcVxlanMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McVxlanMode1_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gCapwapIpv6_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6GreKey_ipSa";
                    }
                    else if (SYS_SCL_KEY_HASH_TUNNEL_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6Udp_ipSa";
                    }
                }
                else if (SYS_SCL_KEY_TCAM_IPV6_SINGLE == pe->key_type)
                {
                    field_name[0] = "ipAddr2";
                }
                else
                {
                    field_name[0] = "ipSa";
                }
                str = "IPV6 SA";
                 *data_type = 2;
                 key_size = sizeof(ipv6_addr_t);
                break;
            }
        case CTC_FIELD_KEY_IPV6_DA:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcNvgreMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcNvgreMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McNvgreMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McNvgreMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcVxlanMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcVxlanMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McVxlanMode0_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McVxlanMode1_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_WLAN_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gCapwapIpv6_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6GreKey_ipDa";
                    }
                    else if (SYS_SCL_KEY_HASH_TUNNEL_IPV6 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6Udp_ipDa";
                    }
                }
                else if (SYS_SCL_KEY_TCAM_IPV6_SINGLE == pe->key_type)
                {
                    field_name[0] = "ipAddr1";
                }
                else
                {
                    field_name[0] = "ipDa";
                }
                str = "IPV6 DA";
                 *data_type = 2;
                 key_size = sizeof(ipv6_addr_t);
                break;
            }
        case CTC_FIELD_KEY_L4_DST_PORT:
            if (pe->resolve_conflict && (SYS_SCL_KEY_HASH_WLAN_IPV4 == pe->key_type))
            {
                field_name[0] = "u1_gCapwapIpv4_udpDestPort";
            }
            else if ((SYS_SCL_KEY_TCAM_IPV4 == pe->key_type) || (SYS_SCL_KEY_TCAM_IPV4_SINGLE == pe->key_type) || \
                        (SYS_SCL_KEY_TCAM_IPV6 == pe->key_type) || (SYS_SCL_KEY_TCAM_IPV6_SINGLE == pe->key_type))
            {
                field_name[0] = "l4DestPort";
            }
            else
            {
                field_name[0] = "udpDestPort";
            }
            str = "L4 DST PORT";
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT:
            if (SYS_SCL_KEY_HASH_TUNNEL_IPV4 == pe->key_type)
            {
                field_name[0] = "udpSrcPort";
            }
            else
            {
                field_name[0] = "l4SourcePort";
            }
            str = "L4 SRC PORT";
            break;
        case CTC_FIELD_KEY_GRE_KEY:
            {
                if (SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE == pe->key_type)
                {
                    field_name[0] = "greKey";
                }
                else if (SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY == pe->key_type)
                {
                    if (1 == pe->resolve_conflict)
                    {
                        field_name[0] = "u1_gTunnelIpv6GreKey_greKey";
                    }
                    else
                    {
                        field_name[0] = "greKey";
                    }
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
        case CTC_FIELD_KEY_VN_ID:
            {
                if (pe->resolve_conflict)
                {
                    if (SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcNvgreMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcNvgreMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4McNvgreMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4NvgreMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcNvgreMode0_vni";
                    }
                    if (SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcNvgreMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McNvgreMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McNvgreMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcVxlanMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4UcVxlanMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4McVxlanMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv4VxlanMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcVxlanMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6UcVxlanMode1_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McVxlanMode0_vni";
                    }
                    else if (SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1 == pe->key_type)
                    {
                        field_name[0] = "u1_gTunnelIpv6McVxlanMode1_vni";
                    }
                }
                else if ((SYS_SCL_KEY_TCAM_IPV4 == pe->key_type) || (SYS_SCL_KEY_TCAM_IPV4_SINGLE == pe->key_type) || \
                        (SYS_SCL_KEY_TCAM_IPV6 == pe->key_type) || (SYS_SCL_KEY_TCAM_IPV6_SINGLE == pe->key_type))
                {
                    field_name[0] = "l4DestPort";
                    start_bit[0] = 0;
                    bit_len[0]   = 16;
                    field_name[1] = "l4SourcePort";
                    start_bit[1] = 0;
                    bit_len[1]   = 8;
                    *field_name_cnt = 2;
                }
                else
                {
                    field_name[0] = "vni";
                }
                str = "VN ID";
                break;
            }
        case CTC_FIELD_KEY_MPLS_LABEL0:
            field_name[0] = "mplsLabel0";
            start_bit[0] = 12;
            bit_len[0]   = 20;
            str = "Mpls Label0";
            break;
        case CTC_FIELD_KEY_MPLS_LABEL1:
            field_name[0] = "mplsLabel1";
            start_bit[0] = 12;
            bit_len[0]   = 20;
            str = "Mpls Label1";
            break;
        case CTC_FIELD_KEY_MPLS_LABEL2:
            field_name[0] = "mplsLabel2";
            start_bit[0] = 12;
            bit_len[0]   = 20;
            str = "Mpls Label2";
            break;
        case CTC_FIELD_KEY_PORT:
            field_name[0] = "globalSrcPort";
            if (is_hash)
            {
                if (SYS_SCL_KEY_HASH_L2 == pe->key_type)
                {
                    if (GetSclFlowHashFieldSelect(V, userIdLabelEn_f, &ds_sel))
                    {
                        field_name[0] = "userIdLabel";
                        str = "Class ID";
                        break;
                    }
                }
                else
                {
                    if (!pe->resolve_conflict)
                    {
                        field_str0 = "isLogicPort";
                        field_str1 = "isLabel";
                        field_name[0] = ((CTC_INGRESS == pe->direction) || (SYS_SCL_KEY_HASH_PORT_CROSS == pe->key_type) || (SYS_SCL_KEY_HASH_PORT_VLAN_CROSS == pe->key_type)) ? "globalSrcPort" : "globalDestPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT == pe->key_type)
                    {
                        field_str0 = "u1_gUserPort_isLogicPort";
                        field_str1 = "u1_gUserPort_isLabel";
                        field_name[0] = "u1_gUserPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_CVLAN == pe->key_type)
                    {
                        field_str0 = "u1_gUserCvlanPort_isLogicPort";
                        field_str1 = "u1_gUserCvlanPort_isLabel";
                        field_name[0] = "u1_gUserCvlanPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_SVLAN == pe->key_type)
                    {
                        field_str0 = "u1_gUserSvlanPort_isLogicPort";
                        field_str1 = "u1_gUserSvlanPort_isLabel";
                        field_name[0] = "u1_gUserSvlanPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_2VLAN == pe->key_type)
                    {
                        field_str0 = "u1_gUserDoubleVlan_isLogicPort";
                        field_str1 = "u1_gUserDoubleVlan_isLabel";
                        field_name[0] = "u1_gUserDoubleVlan_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_CVLAN_COS == pe->key_type)
                    {
                        field_str0 = "u1_gUserCvlanCosPort_isLogicPort";
                        field_str1 = "u1_gUserCvlanCosPort_isLabel";
                        field_name[0] = "u1_gUserCvlanCosPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_SVLAN_COS == pe->key_type)
                    {
                        field_str0 = "u1_gUserSvlanCosPort_isLogicPort";
                        field_str1 = "u1_gUserSvlanCosPort_isLabel";
                        field_name[0] = "u1_gUserSvlanCosPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_MAC == pe->key_type)
                    {
                        field_str0 = "u1_gUserMacPort_isLogicPort";
                        field_str1 = "u1_gUserMacPort_isLabel";
                        field_name[0] = "u1_gUserMacPort_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_IPV4 == pe->key_type)
                    {
                        field_str0 = "u1_gUserIpv4Port_isLogicPort";
                        field_str1 = "u1_gUserIpv4Port_isLabel";
                        field_name[0] = "u1_gUserIpv4Port_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_PORT_IPV6 == pe->key_type)
                    {
                        field_str0 = "u1_gUserIpv6Port_isLogicPort";
                        field_str1 = "u1_gUserIpv6Port_isLabel";
                        field_name[0] = "u1_gUserIpv6Port_globalSrcPort";
                    }
                    else if (SYS_SCL_KEY_HASH_TRILL_MC_ADJ == pe->key_type)
                    {
                        field_str0 = "u1_gTunnelTrillMcAdj_isLogicPort";
                        field_str1 = "u1_gTunnelTrillMcAdj_isLabel";
                        field_name[0] = "u1_gTunnelTrillMcAdj_globalSrcPort";
                    }
                    else
                    {
                        return CTC_E_INVALID_PARAM;
                    }

                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[0], field_str0);
                    drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[1], field_str1);
                    DRV_GET_FIELD(lchip, key_id, field_id[0], pe->temp_entry->key, &drv_data_temp[0]);
                    DRV_GET_FIELD(lchip, key_id, field_id[1], pe->temp_entry->key, &drv_data_temp[1]);
                    if (drv_data_temp[1])
                    {
                        str = "Class ID";
                        break;
                    }
                    else if (drv_data_temp[0])
                    {
                        str = "Logical port";
                        break;
                    }
                    else
                    {
                        str = "Gport";
                        break;
                    }
                }
            }
            else
            {
                /*port class can be configured independently*/
                drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[0], "sclLabel1To0");
                drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[1], "sclLabel7To2");
                DRV_GET_FIELD(lchip, key_id, field_id[0], pe->temp_entry->mask, &drv_data_temp[0]);
                DRV_GET_FIELD(lchip, key_id, field_id[1], pe->temp_entry->mask, &drv_data_temp[1]);
                if (drv_data_temp[0] || drv_data_temp[1])
                {
                    field_name[1] = "sclLabel7To2";
                    bit_len[1]   = 6;
                    field_name[0] = "sclLabel1To0";
                    bit_len[0]   = 2;
                    *field_name_cnt = 2;
                    str = "Class ID";
                    break;
                }
                drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[0], "isLogicPort");
                drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[1], "globalPort");
                DRV_GET_FIELD(lchip, key_id, field_id[0], pe->temp_entry->mask, &drv_data_temp[0]);
                DRV_GET_FIELD(lchip, key_id, field_id[1], pe->temp_entry->mask, &drv_data_temp[1]);
                if (0 != drv_data_temp[0])
                {
                    field_name[0] = "globalPort";
                    str = "Logical port";
                    break;
                }
                else if (drv_data_temp[1])
                {
                    field_name[0] = "globalPort";
                    str = "Gport";
                    break;
                }
                else
                {
                    break;
                }
            }
        case CTC_FIELD_KEY_DST_GPORT:
            field_str0 = "isLogicPort";
            field_str1 = "isLabel";
            field_name[0] = "globalDestPort";

            drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[0], field_str0);
            drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id[1], field_str1);
            DRV_GET_FIELD(lchip, key_id, field_id[0], pe->temp_entry->key, &drv_data_temp[0]);
            DRV_GET_FIELD(lchip, key_id, field_id[1], pe->temp_entry->key, &drv_data_temp[1]);
            if (drv_data_temp[1])
            {
                str = "DST Class ID";
                break;
            }
            else if (drv_data_temp[0])
            {
                str = "DST Logical port";
                break;
            }
            else
            {
                str = "DST Gport";
                break;
            }
    }

    if (str == NULL)
    {
        return CTC_E_INVALID_PARAM;
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
_sys_usw_scl_get_key_field_value(uint8 lchip, sys_scl_sw_entry_t* pe, uint16 key_field_type, uint32 key_id, ctc_field_key_t* p_field_key, char** print_str, uint32* key_size)
{
    char* field_name[4] = {NULL};
    fld_id_t field_id = 0;
    uint8 start_bit[4] = {0};
    uint8 bit_len[4] = {0};
    uint8 field_name_cnt = 0;
    uint8 sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;
    uint32 drv_data_temp[4] = {0,0,0,0};
    uint32 drv_mask_temp[4] = {0,0,0,0};
    uint32 shift = 0;
    int field_name_i= 0;
    uint8 data_type = 0;
    ds_t ipe_vxlan_nvgre_ipda_ctl;
    uint32 cmd = 0;
    ctc_field_port_t field_port;
    int32  ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(pe->temp_entry);

    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    _sys_usw_scl_map_key_field_type(lchip, pe, key_field_type, &sys_field_type);
    if(!CTC_IS_BIT_SET(pe->key_bmp, sys_field_type))
    {
        return 1;/*not exit*/
    }

    ret = sys_usw_scl_map_key_field_to_field_name(lchip, pe, key_field_type, key_id,&data_type, field_name, start_bit, bit_len, &field_name_cnt, print_str, key_size);
    if (ret < 0)
    {
        return 1;/*not exit*/
    }

    p_field_key->data = 0;
    p_field_key->mask = 0;

    for (field_name_i = 0; field_name_i < field_name_cnt; field_name_i++)
    {
        sal_memset(drv_data_temp, 0, sizeof(drv_data_temp));
        sal_memset(drv_mask_temp, 0, sizeof(drv_mask_temp));

        if ((NULL == field_name[field_name_i]))
        {
            break;
        }

        drv_usw_get_field_id_by_sub_string(lchip, key_id, &field_id, field_name[field_name_i]);
        DRV_GET_FIELD(lchip, key_id, field_id, pe->temp_entry->key, drv_data_temp);
        DRV_GET_FIELD(lchip, key_id, field_id, pe->temp_entry->mask, drv_mask_temp);

        if (1 == data_type) /* use field_key.data */
        {
            /* field_key.data; first is low bits; later is high bits */
            {
                p_field_key->data |= ((drv_data_temp[0]>>start_bit[field_name_i])&((1LLU<<bit_len[field_name_i])-1))<<shift;
                p_field_key->mask |= ((drv_mask_temp[0]>>start_bit[field_name_i])&((1LLU<<bit_len[field_name_i])-1))<<shift;
                shift += bit_len[field_name_i];
            }

            if ((0 == sal_strcasecmp(*print_str,"DST Gport")) || (0 == sal_strcasecmp(*print_str,"Gport")))
            {
                uint16 lport = 0;
                uint8  gchip = 0;
                lport = p_field_key->data & 0x1FF;
                gchip = (p_field_key->data >> 9)&0x7F;
                p_field_key->data = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
                p_field_key->mask = 0xFFFFFFFF;     /* global port not care mask, set 0xFFFFFFFF */
            }
            else if ((SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1 == pe->key_type) || (SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1 == pe->key_type))
            {
                sal_memset(&ipe_vxlan_nvgre_ipda_ctl, 0, sizeof(ipe_vxlan_nvgre_ipda_ctl));
                cmd = DRV_IOR(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));
                if (0 == p_field_key->data)
                {
                    GetIpeVxlanNvgreIpdaCtl(A, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &p_field_key->data);
                }
                else if(1 == p_field_key->data)
                {
                    GetIpeVxlanNvgreIpdaCtl(A, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &p_field_key->data);
                }
                else if(2 == p_field_key->data)
                {
                    GetIpeVxlanNvgreIpdaCtl(A, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &p_field_key->data);
                }
                else if(3 == p_field_key->data)
                {
                    GetIpeVxlanNvgreIpdaCtl(A, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &p_field_key->data);
                }
            }
        }
        else /* use field_key.ext_data */
        {
            switch(key_field_type)
            {/* process data format to user */
                case CTC_FIELD_KEY_MAC_SA:
                case CTC_FIELD_KEY_MAC_DA:
                    SYS_USW_SET_USER_MAC((uint8*)p_field_key->ext_data, (uint32*)drv_data_temp);
                    SYS_USW_SET_USER_MAC((uint8*)p_field_key->ext_mask, (uint32*)drv_mask_temp);
                    break;
                case CTC_FIELD_KEY_IPV6_SA:
                case CTC_FIELD_KEY_IPV6_DA:
                    SYS_USW_REVERT_IP6((uint32*)p_field_key->ext_data, (uint32*)drv_data_temp);
                    SYS_USW_REVERT_IP6((uint32*)p_field_key->ext_mask, (uint32*)drv_mask_temp);
                    break;
                default:
                    break;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_show_key(uint8 lchip, sys_scl_sw_entry_t* pe, uint32 key_id, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    ctc_field_key_t field_key;
    uint8 data_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
    uint8 mask_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint16 key_field_recored_i = 0;
    uint16 show_key_field_array[17] = {CTC_FIELD_KEY_PORT,CTC_FIELD_KEY_DST_GPORT,CTC_FIELD_KEY_SVLAN_ID,CTC_FIELD_KEY_CVLAN_ID,
                                                        CTC_FIELD_KEY_MAC_SA,CTC_FIELD_KEY_MAC_DA,CTC_FIELD_KEY_IP_SA,CTC_FIELD_KEY_IP_DA,
                                                        CTC_FIELD_KEY_IPV6_SA,CTC_FIELD_KEY_IPV6_DA,CTC_FIELD_KEY_L4_SRC_PORT,CTC_FIELD_KEY_L4_DST_PORT,
                                                        CTC_FIELD_KEY_GRE_KEY,CTC_FIELD_KEY_VN_ID,CTC_FIELD_KEY_MPLS_LABEL0,CTC_FIELD_KEY_MPLS_LABEL1,
                                                        CTC_FIELD_KEY_MPLS_LABEL2};
    uint8  show_key_field_array_i = 0;
    uint8  grep_i = 0;
    uint32 tempip = 0;
    char buf[CTC_IPV6_ADDR_STR_LEN];
    uint32 tempip_mask = 0;
    char buf_mask[CTC_IPV6_ADDR_STR_LEN];
    char* print_str = NULL;
    uint8 is_tcam = SCL_ENTRY_IS_TCAM(pe->key_type);
    uint8 pgrep_result =0;
    CTC_PTR_VALID_CHECK(pe);
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    field_key.ext_data = data_temp;
    field_key.ext_mask = mask_temp;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Primary Key(Value/Mask):\n");

    for (show_key_field_array_i = 0; show_key_field_array_i < 17; show_key_field_array_i++)
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

        if ((1 == _sys_usw_scl_get_key_field_value(lchip, pe, key_field_recored_i, key_id, &field_key, &print_str, NULL))
            || (NULL == print_str))
        {
            continue;
        }

        if (key_field_recored_i == CTC_FIELD_KEY_MAC_SA || key_field_recored_i == CTC_FIELD_KEY_MAC_DA)
        {
            if(is_tcam)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %x.%x.%x/%X.%X.%X \n", print_str, sal_ntohs(*(unsigned short*)field_key.ext_data), sal_ntohs(*(unsigned short*)(field_key.ext_data + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_data + 4)),sal_ntohs(*(unsigned short*)field_key.ext_mask), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_mask + 4)));
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %x.%x.%x \n", print_str, sal_ntohs(*(unsigned short*)field_key.ext_data), sal_ntohs(*(unsigned short*)(field_key.ext_data + 2)), sal_ntohs(*(unsigned short*)(field_key.ext_data + 4)));
            }
        }
        else if(key_field_recored_i == CTC_FIELD_KEY_IP_SA || key_field_recored_i == CTC_FIELD_KEY_IP_DA)
        {
            tempip = sal_ntohl(field_key.data);
            sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
            if (is_tcam)
            {
                tempip_mask = sal_ntohl(field_key.mask);
                sal_inet_ntop(AF_INET, &tempip_mask, buf_mask, CTC_IPV6_ADDR_STR_LEN);
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s/%s \n", print_str, buf, buf_mask);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", print_str, buf);
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
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s/%s\n", print_str, buf, buf_mask);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %s\n", print_str, buf);
            }
        }
        else if((0 == sal_strcasecmp(print_str,"DST Gport")) || (0 == sal_strcasecmp(print_str,"Gport")))
        {
            if (is_tcam)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x/0x%X\n", print_str, field_key.data, field_key.mask);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: 0x%x\n", print_str, field_key.data);
            }
        }
        else
        {
            if (is_tcam)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u/0x%X\n", print_str, field_key.data, field_key.mask);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-16s: %u\n", print_str, field_key.data);
            }
        }
        print_str = NULL;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_show_entry_detail(uint8 lchip, sys_scl_sw_entry_t* pe, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint8  block_id = 0;
    uint32 key_id   = 0;
    uint32 act_id   = 0;
    char key_name[50] = {0};
    char ad_name[50]  = {0};
    uint32 hw_key_index  = 0;
    uint32 ad_index   = 0;

    CTC_PTR_VALID_CHECK(pe);
    block_id = SYS_SCL_IS_HASH_COM_MODE(pe)? 0: pe->group->priority;

    _sys_usw_scl_get_table_id(lchip, block_id, pe, &key_id, &act_id);
    _sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, &ad_index);

    drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, key_id, 0, key_name);

    if (0xFFFFFFFF != act_id) /* normal egress ad DsVlanXlate is in key */
    {
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, act_id, 0, ad_name);
    }
    else
    {
        sal_memcpy(ad_name, key_name, sizeof(key_name));
    }

    if((SYS_SCL_ACTION_TUNNEL== pe->action_type) && (pe->rpf_check_en))
    {
        sal_strcpy(ad_name, "DsTunnelIdRpf");
    }

    if(SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        if(SYS_SCL_ACTION_INGRESS == pe->action_type)
        {
            switch(block_id)
            {
                case 0:
                    sal_strcpy(ad_name, "DsUserId0Tcam");
                    break;
                case 1:
                    sal_strcpy(ad_name, "DsUserId1Tcam");
                    break;
                case 2:
                    sal_strcpy(ad_name, "DsUserId2");
                    break;
                case 3:
                    sal_strcpy(ad_name, "DsUserId3");
                    break;
                default :
                    break;
            }
        }
        else if((SYS_SCL_ACTION_TUNNEL == pe->action_type) && (pe->rpf_check_en))
        {
            if (1 == block_id)
            {
                sal_strcpy(ad_name, "DsTunnelId1Tcam");
            }
            else if (0 == block_id)
            {
                sal_strcpy(ad_name, "DsTunnelId0Tcam");
            }
        }
        else if(SYS_SCL_ACTION_TUNNEL == pe->action_type)
        {
            if (1 == block_id)
            {
                sal_strcpy(ad_name, "DsTunnelId1Tcam");
            }
            else if (0 == block_id)
            {
                sal_strcpy(ad_name, "DsTunnelId0Tcam");
            }
        }
        else if(SYS_SCL_ACTION_FLOW == pe->action_type)
        {
            if (1 == block_id)
            {
                sal_strcpy(ad_name, "DsSclFlow1Tcam");
            }
            else if (0 == block_id)
            {
                sal_strcpy(ad_name, "DsSclFlow0Tcam");
            }
            else if (2 == block_id)
            {
                sal_strcpy(ad_name, "DsSclFlow2Tcam");
            }
            else if (3 == block_id)
            {
                sal_strcpy(ad_name, "DsSclFlow3Tcam");
            }
        }
        else if(SYS_SCL_ACTION_MPLS == pe->action_type)
        {
            if (1 == block_id)
            {
                sal_strcpy(ad_name, "DsMpls1Tcam");
            }
            else if (0 == block_id)
            {
                sal_strcpy(ad_name, "DsMpls0Tcam");
            }
        }

    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#ENTRY_ID: 0x%X \n", pe->entry_id);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Table:\n");

    if((SYS_SCL_KEY_PORT_DEFAULT_INGRESS != pe->key_type) && (SYS_SCL_KEY_PORT_DEFAULT_EGRESS != pe->key_type))
    {
        if((SCL_ENTRY_IS_TCAM(pe->key_type)) || (pe->resolve_conflict))
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-32s :%-8u\n", key_name, hw_key_index);
        }
        else
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-32s :0x%-8X\n", key_name, hw_key_index);
        }
    }

    if ((SCL_ENTRY_IS_TCAM(pe->key_type)) || (pe->resolve_conflict) )
    {
        /*SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-32s :0x%-8X\n", ad_name, tcam_ad_index);*/
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-32s :%-8u\n", ad_name, ad_index);
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-32s :0x%-8X\n", ad_name, ad_index);
    }

    CTC_ERROR_RETURN(_sys_usw_scl_show_key(lchip, pe, key_id,p_grep, grep_cnt));

    switch (pe->action_type)
    {
    case SYS_SCL_ACTION_INGRESS:
        _sys_usw_scl_show_igs_action(lchip, pe);
        break;
    case SYS_SCL_ACTION_EGRESS:
        _sys_usw_scl_show_egs_action(lchip, pe);
        break;
    case SYS_SCL_ACTION_TUNNEL:
        _sys_usw_scl_show_tunnel_action(lchip, pe);
        break;
    case SYS_SCL_ACTION_FLOW:
        _sys_usw_scl_show_flow_action(lchip, pe);
        break;
    default:
        break;
    }

    return CTC_E_NONE;
}

STATIC int32
 _sys_usw_scl_filter_by_key(uint8 lchip, sys_scl_sw_entry_t* pe, ctc_field_key_t* p_grep, uint8 grep_cnt,uint32 key_id)
{
    ctc_field_key_t field_key;
    uint8 data_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
    uint8 mask_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
    uint8 result_grep_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
    uint8 result_data_temp[SYS_USW_SCL_MAX_KEY_SIZE] = {0};
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

        if(1 == _sys_usw_scl_get_key_field_value(lchip, pe, p_grep[grep_i].type, key_id, &field_key,&print_str, &key_size))
        {   /* initial conditions */
            return CTC_E_INVALID_PARAM;    /* grep fail */
        }

        if(SCL_ENTRY_IS_TCAM(pe->key_type))
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
 * show scl entry to a specific entry with specific key type
 */
STATIC int32
_sys_usw_scl_show_entry(uint8 lchip, sys_scl_sw_entry_t* pe, uint32* p_cnt, sys_scl_key_type_t key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    sys_scl_sw_group_t* pg;
    char*               type_name;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_key_index = 0;
    uint8   scl_id  = SYS_SCL_IS_HASH_COM_MODE(pe)? 0: pe->group->priority;
    int32  ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(pe);

    pg = pe->group;
    if (!pg)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Group not exist \n");
			return CTC_E_NOT_EXIST;

    }

    if ((SYS_SCL_KEY_NUM != key_type)
        && (pe->key_type != key_type))
    {
        return CTC_E_NONE;
    }

    type_name = _sys_usw_scl_get_type(lchip, pe->key_type);
    _sys_usw_scl_get_table_id(lchip, scl_id, pe, &key_id, &act_id);
    _sys_usw_scl_get_index(lchip, key_id, pe, &hw_key_index, NULL);

    if (!pe->uninstall)
    {
        CTC_ERROR_RETURN(_sys_usw_scl_resume_buffer_from_hw(lchip, pe));
    }

    if (0 != grep_cnt)
    {
        if (_sys_usw_scl_filter_by_key(lchip, pe, p_grep, grep_cnt, key_id) != CTC_E_NONE)
        {
            ret = CTC_E_NONE;
            goto clean_up;
        }
    }

    if (1 == *p_cnt)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-13s%-13s%-7s%-7s%-20s%s\n", "No.", "ENTRY_ID", "GROUP_ID", "HW", "PRI", "KEY_INDEX", "  KEY_TYPE");
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------------------------\n");
    }

    if ((!detail) || (1 == show_type) || (2 == show_type))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", *p_cnt);

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8X%3s", pe->entry_id, "");

        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8X%3s",  pg->group_id, "");

        if((SCL_ENTRY_IS_TCAM(pe->key_type)) || pe->resolve_conflict)
        {

            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-7u%-20u%s%s\n",
                        (pe->uninstall) ? "no" : "yes ", p_usw_scl_master[lchip]->block[pg->priority].fpab.entries[pe->key_index]->priority, hw_key_index, type_name,pe->resolve_conflict ? "(resolve_conflict)" : "");
        }
        else
        {
            /* show hash entry */
            if (pe->key_exist)   /* hash key index is not avaliable */
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-7u%-20s%s\n", (pe->uninstall) ? "no" : "yes ", 1, "keyExist", type_name);
            }
            else if (pe->key_conflict)   /* hash key index is not avaliable */
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-7u%-20s%s\n", (pe->uninstall) ? "no" : "yes ", 1, "bucketFull", type_name);
            }
            else if (!pe->hash_valid)   /* hash key index is not avaliable */
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-7u%-20s%s\n", (pe->uninstall) ? "no" : "yes ", 1, "keyIncomplete", type_name);
            }
            else
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-7u0x%-18X%s\n", (pe->uninstall) ? "no" : "yes ", 1, hw_key_index, type_name);
            }
        }

        (*p_cnt)++;
    }

    if (detail)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_show_entry_detail(lchip, pe, p_grep, grep_cnt), ret, clean_up);
    }

clean_up:
    if(!pe->uninstall)
    {
        mem_free(pe->temp_entry);
    }
    return ret;
}




/*
 * show scl entriy by entry id
 */
int32
_sys_usw_scl_show_entry_by_entry_id(uint8 lchip, uint32 eid, sys_scl_key_type_t key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint32 count = 1;
    sys_scl_sw_entry_t* pe = NULL;

    _sys_usw_scl_get_nodes_by_eid(lchip, eid, &pe);
    if (!pe)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    /* SCL_PRINT_ENTRY_ROW(eid); */

    CTC_ERROR_RETURN(_sys_usw_scl_show_entry(lchip, pe, &count, key_type, detail, show_type, p_grep, grep_cnt));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_show_entry_in_group(void* bucket_data, void* user_data)
{
    int32                    ret = 0;
    sys_scl_sw_group_t* pg = NULL;
    sys_scl_sw_entry_t *pe;
    _scl_cb_para_t* p_para = NULL;
    ctc_slistnode_t* pe_node  = NULL;
    uint32 inner_group_mod_id = 0;
    ctc_linklist_t*   p_entry_list = NULL;
    ctc_listnode_t*   node = NULL;
    uint8 external_list = 0;

    pg = (sys_scl_sw_group_t*) bucket_data;
    p_para  = (_scl_cb_para_t*) user_data;
    p_entry_list = p_para->p_linklist;
    inner_group_mod_id = SYS_SCL_GET_INNER_GRP_MOD_ID(pg->group_id);
    if (p_para->care_grp_mod_id)
    {
        if ((pg->group_id <= CTC_SCL_GROUP_ID_MAX_NORMAL)
            || (inner_group_mod_id != p_para->grp_mod_id))
        {
            return CTC_E_NONE;
        }
    }

    /* if external link list not exist, create new*/
    external_list = p_entry_list ? 1 : 0;
    if(!external_list)
    {
        p_entry_list = ctc_list_create(_sys_usw_scl_linklist_entry_cmp, NULL);
        if(!p_entry_list)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
            return CTC_E_NO_MEMORY;
        }
    }

    CTC_SLIST_LOOP(pg->entry_list, pe_node)
    {
        pe = _ctc_container_of(pe_node, sys_scl_sw_entry_t, head);
        p_para->key_type = p_para->care_key_type ? p_para->key_type:SYS_SCL_KEY_NUM;
        CTC_ERROR_GOTO(_sys_usw_scl_store_data_to_list(pe, p_entry_list), ret, error);
    }

    if(!external_list)
    {
        CTC_LIST_LOOP(p_entry_list, pe, node)
        {
            CTC_ERROR_GOTO(_sys_usw_scl_show_entry(p_para->lchip, pe, &p_para->count, p_para->key_type, p_para->detail, \
                  p_para->show_type, p_para->p_grep, p_para->grep_cnt), ret, error);
        }
    }

error:
    if(!external_list)
    {
        ctc_list_delete(p_entry_list);
    }
    return ret;
}

int32
_sys_usw_scl_show_all_group_entry_by_modid(uint8 lchip, uint8 key_type, uint8 mod_id, uint8 detail)
{
    int                  ret = 0;
    _scl_cb_para_t  para;
    ctc_linklist_t*      p_entry_list = NULL;
    ctc_listnode_t*      node;
    sys_scl_sw_entry_t*  pe;

    sal_memset(&para, 0, sizeof(_scl_cb_para_t));
    para.detail   = detail;
    para.grp_mod_id = mod_id;
    para.care_key_type = 1;
    para.key_type = key_type;
    para.care_grp_mod_id = 1;
    para.lchip = lchip;
    para.count = 1;

    if(CTC_FEATURE_IP_TUNNEL == mod_id || CTC_FEATURE_WLAN == mod_id || CTC_FEATURE_SECURITY == mod_id || CTC_FEATURE_TRILL == mod_id || CTC_FEATURE_OVERLAY == mod_id)
    {
        para.care_key_type = 0;
    }

    p_entry_list = ctc_list_create(_sys_usw_scl_linklist_entry_cmp, NULL);
    if(!p_entry_list)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
        return CTC_E_NO_MEMORY;
    }
    para.p_linklist = p_entry_list;

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->group, _sys_usw_scl_show_entry_in_group, (void*)(&para)),\
                                                                                ret, error);
    CTC_LIST_LOOP(p_entry_list, pe, node)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_show_entry(para.lchip, pe, &(para.count), para.key_type, para.detail, 0, NULL, 0), \
                                                                                          ret, error);
    }

    if (CTC_FEATURE_VLAN == mod_id)
    {
        if (SYS_SCL_KEY_HASH_MAC == key_type || SYS_SCL_KEY_HASH_IPV4 == key_type || SYS_SCL_KEY_HASH_IPV6 == key_type)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total hash entry number : %u\n", (para.count - 1));
        }
        else
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total tcam entry number : %u\n", (para.count - 1));
        }
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total entry number : %u\n", (para.count - 1));
    }

error:
    if(p_entry_list)
    {
        ctc_list_delete(p_entry_list);
    }
    return ret;
}

int32
_sys_usw_scl_show_group_entry_key_type(uint8 lchip, sys_scl_key_type_t key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    int32                ret = 0;
    _scl_cb_para_t  para;
    ctc_linklist_t*      p_entry_list = NULL;
    ctc_listnode_t*      node;
    sys_scl_sw_entry_t*  pe;

    sal_memset(&para, 0, sizeof(_scl_cb_para_t));

    para.key_type = key_type;
    para.care_key_type = 1;
    para.lchip = lchip;
    para.count = 1;
    para.p_grep = p_grep;
    para.grep_cnt = grep_cnt;
    para.show_type = show_type;

    p_entry_list = ctc_list_create(_sys_usw_scl_linklist_entry_cmp, NULL);
    if(!p_entry_list)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
        return CTC_E_NO_MEMORY;
    }
    para.p_linklist = p_entry_list;

    para.detail = 0;
    CTC_ERROR_RETURN(ctc_hash_traverse(p_usw_scl_master[lchip]->group, _sys_usw_scl_show_entry_in_group, (void*)(&para)));
    CTC_LIST_LOOP(p_entry_list, pe, node)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_show_entry(para.lchip, pe, &(para.count), para.key_type, para.detail, para.show_type, para.p_grep, para.grep_cnt), \
                                                                                            ret, error);
    }

    if (detail)
    {
        para.detail = 1;
        CTC_LIST_LOOP(p_entry_list, pe, node)
        {
            CTC_ERROR_GOTO(_sys_usw_scl_show_entry(para.lchip, pe, &(para.count), para.key_type, para.detail, para.show_type, para.p_grep, para.grep_cnt), \
                                                                                                ret, error);
        }
    }


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total entry number : %u\n",(para.count-1));
error:
    if(p_entry_list)
    {
        ctc_list_delete(p_entry_list);
    }
    return ret;
}

/*
 * show scl entries by group id with specific key type
 */
int32
_sys_usw_scl_show_entry_by_group_id(uint8 lchip, uint32 gid, sys_scl_key_type_t key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint32                   count = 1;
    int32                    ret   = 0;
    sys_scl_sw_group_t* pg    = NULL;
    ctc_slistnode_t*  s_node    = NULL;
    ctc_linklist_t*   p_entry_list = NULL;
    ctc_listnode_t*   next = NULL;
    sys_scl_sw_entry_t* pe = NULL;

    _sys_usw_scl_get_group_by_gid(lchip, gid, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, FALSE);

    p_entry_list = ctc_list_create(_sys_usw_scl_linklist_entry_cmp, NULL);
    if(!p_entry_list)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
        return CTC_E_NO_MEMORY;
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n>>group-id %u :\n", (gid))

    CTC_SLIST_LOOP(pg->entry_list, s_node)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_store_data_to_list(_ctc_container_of(s_node, sys_scl_sw_entry_t, head), \
                                    p_entry_list), ret, error);
    }

    CTC_LIST_LOOP(p_entry_list, pe, next)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_show_entry(lchip, pe, &count, key_type, detail, show_type, p_grep, grep_cnt), ret, error);
    }
error:
    if(p_entry_list)
    {
        ctc_list_delete(p_entry_list);
    }
    return CTC_E_NONE;
}

/*
 * show acl entries in a block with specific key type
 */
STATIC int32
_sys_usw_scl_show_entry_in_block(uint8 lchip, sys_scl_sw_block_t* pb, uint32* p_cnt, uint8 key_type, uint8 detail,uint8 show_type,  ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    sys_scl_sw_tcam_entry_t* pe   = NULL;
    uint8               step      = 0;
    uint16              block_idx = 0;

    CTC_PTR_VALID_CHECK(pb);
    CTC_PTR_VALID_CHECK(p_cnt);

    step = 1;
    for (block_idx = 0; block_idx < pb->fpab.entry_count; block_idx = block_idx + step)
    {

        if (block_idx >= pb->fpab.start_offset[3] && (pb->fpab.sub_entry_count[3]))   /*640 bit key*/
        {
            step = 8;
        }
        else if (block_idx >= pb->fpab.start_offset[2] && (pb->fpab.sub_entry_count[2]))   /*320 bit key*/
        {
            step = 4;
        }
        else if (block_idx >= pb->fpab.start_offset[1] && (pb->fpab.sub_entry_count[1]))   /*160 bit key*/
        {
            step = 2;
        }
        else                                            /*80 bit key*/
        {
            step = 1;
        }
        pe = (sys_scl_sw_tcam_entry_t*)pb->fpab.entries[block_idx];

        if (pe)
        {
            CTC_ERROR_RETURN(_sys_usw_scl_show_entry(lchip, pe->entry, p_cnt, key_type, detail, show_type, p_grep, grep_cnt));
        }
    }

    return CTC_E_NONE;
}

/*
 * show scl entries in tcam sort by index
 */
int32
_sys_usw_scl_show_tcam_entries(uint8 lchip, uint8 prio, sys_scl_key_type_t key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint32              count     = 1;
    sys_scl_sw_block_t* pb        = NULL;


    SYS_SCL_CHECK_GROUP_PRIO(prio);

    count = 1;
    pb = &p_usw_scl_master[lchip]->block[prio];
    if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
    {
        CTC_ERROR_RETURN(_sys_usw_scl_show_entry_in_block(lchip, pb, &count, key_type, 0,show_type, p_grep, grep_cnt));
    }

    if(detail)
    {
        pb = &p_usw_scl_master[lchip]->block[prio];
        if (pb->fpab.free_count != pb->fpab.entry_count) /*not empty*/
        {
            CTC_ERROR_RETURN(_sys_usw_scl_show_entry_in_block(lchip, pb, &count, key_type, detail, show_type, p_grep, grep_cnt));
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_scl_show_priority_status(uint8 lchip)
{
    uint8 vlan_mac;
    uint8 ipv4;
    uint8 ipv6;
    uint8 idx;

    vlan_mac = p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_MAC];
    ipv4     = p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV4];
    ipv6     = p_usw_scl_master[lchip]->fpa_size[SYS_SCL_KEY_TCAM_IPV6];

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Tcam Status (lchip %d):\n", lchip);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL_ID      Vlan|Mac|Ipv4-S   Ipv4   Ipv6 \n");

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------\n");

    for (idx = 0; idx < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); idx++)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL%d         %d/%d            %d/%d   %d/%d  \n",
                         idx,
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[vlan_mac] - p_usw_scl_master[lchip]->block[idx].fpab.sub_free_count[vlan_mac],
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[vlan_mac],
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[ipv4] - p_usw_scl_master[lchip]->block[idx].fpab.sub_free_count[ipv4],
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[ipv4],
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[ipv6] - p_usw_scl_master[lchip]->block[idx].fpab.sub_free_count[ipv6],
                         p_usw_scl_master[lchip]->block[idx].fpab.sub_entry_count[ipv6]);
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n\n");
    return CTC_E_NONE;
}

typedef struct
{
    uint32              group_id;
    char*               name;
    sys_scl_sw_group_t* pg;
    uint32              count;
} sys_scl_rsv_group_info_t;


STATIC int32
_sys_usw_scl_show_traverse_group(void* bucket_data, void* user_data)
{
    sys_scl_sw_group_t* pg = (sys_scl_sw_group_t*) bucket_data;
    _scl_cb_para_t  *para = user_data;
    char group_str[100] = {0};
    uint8 mod_id;
    if (pg->group_id > CTC_SCL_GROUP_ID_MAX_NORMAL && pg->group_id < CTC_SCL_GROUP_ID_HASH_PORT)
    {
        mod_id =   SYS_SCL_GET_INNER_GRP_MOD_ID(pg->group_id);
        switch (mod_id)
        {
            case CTC_FEATURE_VLAN_MAPPING:
                sal_strcpy(group_str, "Internal(vlan mapping)");
                break;
            case CTC_FEATURE_WLAN:
                sal_strcpy(group_str, "Internal(wlan)");
                break;
            case CTC_FEATURE_SECURITY:
                sal_strcpy(group_str, "Internal(security)");
                break;
            case CTC_FEATURE_VLAN:
                sal_strcpy(group_str, "Internal(vlan class)");
                break;
            case CTC_FEATURE_L3IF:
                sal_strcpy(group_str, "Internal(l3if)");
                break;
            case CTC_FEATURE_IP_TUNNEL:
                sal_strcpy(group_str, "Internal(ip tunnel)");
                break;
            case CTC_FEATURE_OVERLAY:
                sal_strcpy(group_str, "Internal(overlay)");
                break;

            case CTC_FEATURE_TRILL:
                sal_strcpy(group_str, "Internal(trill)");
                break;
            case CTC_FEATURE_MPLS:
                sal_strcpy(group_str, "Internal(mpls)");
                break;
                default:
                  sal_strcpy(group_str, "Internal");
        }


    }
    else if (pg->group_id >= CTC_SCL_GROUP_ID_HASH_PORT )
    {
        switch (pg->group_id)
        {
            case CTC_SCL_GROUP_ID_HASH_PORT:
                sal_strcpy(group_str, "Reserved(hash_port)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_CVLAN:
                sal_strcpy(group_str, "Reserved(hash_port_cvlan)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_SVLAN:
                sal_strcpy(group_str, "Reserved(hash_port_svlan)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_2VLAN:
                sal_strcpy(group_str, "Reserved(hash_port_double_vlan)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_CVLAN_COS:
                sal_strcpy(group_str, "Reserved(hash_port_cvlan_cos)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_SVLAN_COS:
                sal_strcpy(group_str, "Reserved(hash_port_svlan_cos)");
                break;
            case CTC_SCL_GROUP_ID_HASH_MAC:
                sal_strcpy(group_str, "Reserved(hash_mac)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_MAC:
                sal_strcpy(group_str, "Reserved(hash_port_mac)");
                break;
            case CTC_SCL_GROUP_ID_HASH_IPV4:
                sal_strcpy(group_str, "Reserved(hash_ipv4)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_IPV4:
                sal_strcpy(group_str, "Reserved(hash_port_ipv4)");
                break;
            case CTC_SCL_GROUP_ID_HASH_IPV6:
                sal_strcpy(group_str, "Reserved(hash_ipv6)");
                break;
            case CTC_SCL_GROUP_ID_HASH_L2:
                sal_strcpy(group_str, "Reserved(hash_l2)");
                break;
            case CTC_SCL_GROUP_ID_HASH_PORT_IPV6:
                sal_strcpy(group_str, "Reserved(hash_port_ipv6)");
                break;
            case CTC_SCL_GROUP_ID_TCAM0:
                sal_strcpy(group_str, "Reserved(tcam0)");
                break;
            case CTC_SCL_GROUP_ID_TCAM1:
                sal_strcpy(group_str, "Reserved(tcam1)");
                break;
            case CTC_SCL_GROUP_ID_TCAM2:
                sal_strcpy(group_str, "Reserved(tcam2)");
                break;
            case CTC_SCL_GROUP_ID_TCAM3:
                sal_strcpy(group_str, "Reserved(tcam3)");
                break;
            default:
                sal_strcpy(group_str, "Reserved");
        }
    }
    else
    {
        sal_strcpy(group_str, "User-define");
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7u0x%-.8X%7s%-35s%u\n",
                          para->count, pg->group_id, "", group_str, CTC_SLISTCOUNT(pg->entry_list));

      para->count++;

    return CTC_E_NONE;
}

int32
_sys_usw_scl_show_hash_default_entries(int8 lchip, uint32 gport, uint8 detail)
{

    uint16 lport = 0;
    uint8 index = 0;
    sys_scl_default_action_t action;
    ctc_vlan_miss_t vlan_class;

    LCHIP_CHECK(lchip);
    SYS_SCL_INIT_CHECK();


    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&action, 0, sizeof(action));
    action.action_type = SYS_SCL_ACTION_INGRESS;
    action.vlan_miss = &vlan_class;
    action.lport = lport;


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SCL default action\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "GPort 0x%.4x default Ingress action%s :  ",gport, (0 == index? "": " 1"));
        sal_memset(action.vlan_miss, 0, sizeof(ctc_vlan_miss_t));
        action.scl_id= index;
        CTC_ERROR_RETURN(_sys_usw_scl_get_default_action(lchip, &action));
        if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_TO_CPU)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "TO_CPU\n");
        }
        else if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_DISCARD)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DISCARD\n");
        }
        else if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "USER_VLANPTR\n");
        }
        else if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_APPEND_STAG)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "APPEND\n");
        }
        else
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FWD\n");
        }
        if (detail)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--DsUserId%s :%d\n", (0 == index? "": "1"), action.ad_index[index]);
        }
    }
    action.action_type = CTC_SCL_ACTION_EGRESS;
    action.lport = lport;
    sal_memset(action.vlan_miss, 0, sizeof(ctc_vlan_miss_t));

    CTC_ERROR_RETURN(_sys_usw_scl_get_default_action(lchip,  &action));
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "GPort 0x%.4x default Egress action  :  ",gport);
    if (action.vlan_miss->flag ==  CTC_VLAN_MISS_ACTION_DISCARD)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DISCARD\n");
    }
    else
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FWD\n");
    }

    if (detail)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--DsVlanXlateDefault :%d\n ", action.ad_index[0]);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_scl_show_group_status(uint8 lchip)
{
    _scl_cb_para_t  para;
    ctc_linklist_t*      p_group_list = NULL;
    ctc_listnode_t*      next = NULL;
    void*                pg = NULL;
    int32               ret = 0;

    SYS_SCL_INIT_CHECK();
    sal_memset(&para, 0, sizeof(_scl_cb_para_t));
    para.detail   = 0;
    para.key_type = 0;

    p_group_list = ctc_list_create(_sys_usw_scl_linklist_group_cmp, NULL);
    if(!p_group_list)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
        return CTC_E_NO_MEMORY;
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s%-17s%-30s%s\n", "No.", "GROUP_ID", "TYPE", "ENTRY_COUNT");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------\n");

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_scl_master[lchip]->group, _sys_usw_scl_store_data_to_list, (void*)(p_group_list)), ret, error);
    CTC_LIST_LOOP(p_group_list, pg, next)
    {
        CTC_ERROR_GOTO(_sys_usw_scl_show_traverse_group(pg, (void*)(&para)), ret, error);
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total Group Count:%d\n",para.count);

error:
    if(p_group_list)
    {
        ctc_list_delete(p_group_list);
    }
    return ret;
}

STATIC int32
_sys_usw_scl_get_overlay_tunnel_entry_in_group(void* bucket_data, void* user_data)
{
    uint32 inner_group_mod_id = 0;
    sys_scl_sw_group_t* pg = NULL;
    sys_scl_sw_entry_t* pe = NULL;
    ctc_slistnode_t*  node = NULL;
    _scl_cb_para_t* p_para = NULL;

    pg = (sys_scl_sw_group_t*)bucket_data;
    p_para = (_scl_cb_para_t*)user_data;

    inner_group_mod_id = SYS_SCL_GET_INNER_GRP_MOD_ID(pg->group_id);

    if(inner_group_mod_id != CTC_FEATURE_OVERLAY)
    {
        return CTC_E_NONE;
    }

    CTC_SLIST_LOOP(pg->entry_list, node)
    {
        pe = (sys_scl_sw_entry_t*)node;
        if(pe->key_type == p_para->key_type)
        {
            p_para->count += 1;
        }
    }

    return CTC_E_NONE;
}

#define __SCL_FPA_CALLBACK__

int32
_sys_usw_scl_get_block_by_pe_fpa(ctc_fpa_entry_t* pe, ctc_fpa_block_t** pb)
{

    uint8             block_id;
    uint8             lchip = 0;
    sys_scl_sw_entry_t* entry;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pb);

    lchip = pe->lchip;
    _sys_usw_scl_get_nodes_by_eid(lchip, pe->entry_id, &entry);
    if (!entry || !entry->group)
    {
        *pb = NULL;
    }
    else
    {
        block_id = entry->group->priority;
        *pb = &(p_usw_scl_master[lchip]->block[block_id].fpab);
    }

    return CTC_E_NONE;
}

/*
 * move entry in hardware table to an new index.
 */
int32
_sys_usw_scl_entry_move_hw_fpa(ctc_fpa_entry_t* pe, int32 tcam_idx_new)
{
    uint8  lchip = 0;
    uint32 eid;
    sys_scl_sw_entry_t* pe_lkup = NULL;
    uint32 cmd = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 hw_key_index = 0;
    uint32 hw_ad_index = 0;
    tbl_entry_t              tcam_key;
    sys_scl_sw_temp_entry_t sw_temp;
    uint32 hw_new_key_index = 0;
    uint32 hw_new_ad_index = 0;

    CTC_PTR_VALID_CHECK(pe);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    lchip = pe->lchip;
    eid = pe->entry_id;
    _sys_usw_scl_get_nodes_by_eid(lchip, eid, &pe_lkup);

    if (pe_lkup->uninstall)
    {
        /*just update key_index in pe_entry*/
        pe_lkup->key_index = tcam_idx_new;
        pe_lkup->ad_index = tcam_idx_new;
        return CTC_E_NONE;
    }

    _sys_usw_scl_get_table_id(lchip, pe_lkup->group->priority, pe_lkup, &key_id, &act_id);

    /*1. Get old entry info*/
    _sys_usw_scl_get_index(lchip, key_id, pe_lkup, &hw_key_index, &hw_ad_index);

    /*2. get old key from hw*/
    cmd = DRV_IOR(key_id, DRV_ENTRY_FLAG);
    tcam_key.data_entry = sw_temp.key;
    tcam_key.mask_entry = sw_temp.mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_key_index, cmd, &tcam_key));

    /*3. get old action from hw*/
    cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_ad_index, cmd, &sw_temp.action));

    /*4. encoding new lkup info to get new entry info*/
    pe_lkup->key_index = tcam_idx_new;
    _sys_usw_scl_get_index(lchip, key_id, pe_lkup, &hw_new_key_index, &hw_new_ad_index);

    /*5. move old action to new ad index*/
    cmd = DRV_IOW(act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_new_ad_index, cmd, &sw_temp.action));

    /*5. move old key to new key index*/
    cmd = DRV_IOW(key_id, DRV_ENTRY_FLAG);
    tcam_key.data_entry = sw_temp.key;
    tcam_key.mask_entry = sw_temp.mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_new_key_index, cmd, &tcam_key));

    /*6. then delete old key*/
    cmd = DRV_IOD(key_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, hw_key_index, cmd, &cmd));

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "move_hw_fpa: :entry id:%u move key_index:%u to %u\n", pe_lkup->entry_id, pe->offset_a,tcam_idx_new);

    return CTC_E_NONE;
}

#define _________api_function__________

int32
sys_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner)
{
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LCHIP_CHECK(lchip);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_LOCK(lchip);
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_create_group(lchip, group_id, group_info, inner));
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

/* to destroy a group, make sure all entries in the group have been removed first */
int32
sys_usw_scl_destroy_group(uint8 lchip, uint32 group_id, uint8 inner)
{
    sys_scl_sw_group_t* pg = NULL;


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: group_id: %u \n", group_id);

    SYS_SCL_INIT_CHECK();
    if (!inner && group_id > CTC_SCL_GROUP_ID_MAX_NORMAL)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Invalid group id \n");
		return CTC_E_BADID;


    }
    /*
    * #1 check if entry all removed.
    * #2 remove from hash. free sys_scl_sw_group_t.
    */

    SYS_SCL_LOCK(lchip);
    /* check if group exist */
    _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);

    /* check if all entry removed */
    if (!CTC_SLIST_ISEMPTY(pg->entry_list))
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_NOT_READY);
    }

    ctc_hash_remove(p_usw_scl_master[lchip]->group, pg);
    /* free slist */
    ctc_slist_free(pg->entry_list);
    /* free pg */
    mem_free(pg);
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_install_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner)
{
    int32                    ret         = CTC_E_NONE;
    int32                    ret_temp    = CTC_E_NONE;
    sys_scl_sw_group_t* pg          = NULL;
    sys_scl_sw_entry_t* p_scl_entry = NULL;
    struct ctc_slistnode_s*  pe          = NULL;

    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: gid %u \n", group_id);

    if (!inner)
    {
        SYS_SCL_CHECK_OUTER_GROUP_ID(group_id, FALSE);
    }

    /* get group node */
    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);

    CTC_SLIST_LOOP(pg->entry_list, pe)
    {
        p_scl_entry = (sys_scl_sw_entry_t*) pe;
        if (p_scl_entry->uninstall)
        {
            ret = _sys_usw_scl_install_entry(lchip, p_scl_entry, 0);
            if(CTC_E_NONE != ret && ret_temp == CTC_E_NONE)
            {
                ret_temp = ret;
            }
        }
    }

    SYS_SCL_UNLOCK(lchip);

    return ret_temp;
}

/*
 * uninstall a group (empty or NOT) from hardware table
 */
 int32
 sys_usw_scl_uninstall_group(uint8 lchip, uint32 group_id, uint8 inner)
 {
     sys_scl_sw_group_t* pg          = NULL;
     sys_scl_sw_entry_t* p_scl_entry = NULL;
     struct ctc_slistnode_s*  pe          = NULL;
     struct ctc_slistnode_s*  pe_new          = NULL;

     SYS_SCL_INIT_CHECK();

     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: gid %u \n", group_id);

     /* get group node */
     SYS_SCL_LOCK(lchip);
     _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
     SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);

     CTC_SLIST_LOOP_DEL(pg->entry_list, pe, pe_new)
     {
         p_scl_entry = (sys_scl_sw_entry_t*)pe;
         if (!p_scl_entry->uninstall)
         {
             CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_uninstall_entry(lchip, p_scl_entry, 0));
         }
     }
     SYS_SCL_UNLOCK(lchip);

     return CTC_E_NONE;
 }

int32
sys_usw_scl_get_group_info(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner)
{
    sys_scl_sw_group_t* pg    = NULL;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: gid: %u \n", group_id);

    if (!inner)
    {
        SYS_SCL_CHECK_OUTER_GROUP_ID(group_id, FALSE);
    }
    CTC_PTR_VALID_CHECK(group_info);

    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);
    group_info->type     = pg->type;
    group_info->lchip    = lchip;
    group_info->priority = pg->priority;
    if (CTC_SCL_GROUP_TYPE_PORT == pg->type)
    {
        group_info->un.gport = pg->gport;
    }
    else if (CTC_SCL_GROUP_TYPE_PORT_CLASS == pg->type)
    {
        group_info->un.port_class_id = pg->gport;
    }
    else if (CTC_SCL_GROUP_TYPE_LOGIC_PORT == pg->type)
    {
        group_info->un.logic_port = pg->gport;
    }

    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_add_entry(uint8 lchip, uint32 group_id, ctc_scl_entry_t* scl_entry, uint8 inner)
{
    sys_scl_sw_group_t* pg      = NULL;
    sys_scl_sw_entry_t* pe_lkup = NULL;
    sys_scl_sw_entry_t* pe      = NULL;
    sys_scl_sw_tcam_entry_t* pe_tcam = NULL;
    ctc_field_key_t          field_key;
    ctc_scl_field_action_t field_action;
    int32                    ret     = 0;
    uint32                   entry_id = 0;
    uint8                    vlan_dscp_invalid = 0;
    sys_usw_opf_t opf;
    ctc_field_port_t field_port_data;
    ctc_field_port_t field_port_mask;

    sal_memset(&field_port_data, 0, sizeof(ctc_field_port_t));
    sal_memset(&field_port_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_SCL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(scl_entry);
    SYS_SCL_CHECK_MODE_TYPE(scl_entry->mode);
    CTC_MAX_VALUE_CHECK(scl_entry->hash_field_sel_id, (SYS_SCL_HASH_SEL_ID_MAX - 1));

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (inner)
    {
        vlan_dscp_invalid = SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP == scl_entry->key_type && scl_entry->resolve_conflict && DRV_IS_TSINGMA(lchip);
        vlan_dscp_invalid = vlan_dscp_invalid || (SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP == scl_entry->key_type && DRV_IS_DUET2(lchip));
    }
    else
    {
        vlan_dscp_invalid = CTC_SCL_KEY_HASH_PORT_SVLAN_DSCP == scl_entry->key_type && scl_entry->resolve_conflict && DRV_IS_TSINGMA(lchip);
        vlan_dscp_invalid = vlan_dscp_invalid || (CTC_SCL_KEY_HASH_PORT_SVLAN_DSCP == scl_entry->key_type && DRV_IS_DUET2(lchip));
    }
    CTC_ERROR_RETURN(vlan_dscp_invalid? CTC_E_NOT_SUPPORT: CTC_E_NONE);

    SYS_SCL_LOCK(lchip);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM:gid:%u eid:%u key_type:%d\n", group_id,scl_entry->entry_id,scl_entry->key_type);
    /* check sys group unexist */
    _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);

    if ((CTC_SCL_KEY_TCAM_UDF_EXT != scl_entry->key_type) && (CTC_SCL_KEY_TCAM_UDF != scl_entry->key_type) && \
        (CTC_SCL_KEY_TCAM_MAC != scl_entry->key_type) && (CTC_SCL_KEY_TCAM_VLAN != scl_entry->key_type) && \
        (CTC_SCL_KEY_TCAM_IPV6_SINGLE != scl_entry->key_type) && (CTC_SCL_KEY_TCAM_IPV6 != scl_entry->key_type) && \
        (CTC_SCL_KEY_TCAM_IPV4_SINGLE != scl_entry->key_type) && (CTC_SCL_KEY_TCAM_IPV4 != scl_entry->key_type) && \
        (1 < pg->priority) && (!inner))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Current group priority is %u \n", pg->priority);
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_INTR_INVALID_PARAM;
    }

    if (inner) /* get inner entry id from opf */
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;
        opf.pool_index = 0;

        CTC_USW_ERROR_RETURN_SCL_UNLOCK(sys_usw_opf_alloc_offset(lchip, &opf, 1, &entry_id));
        scl_entry->entry_id = entry_id + SYS_SCL_MIN_INNER_ENTRY_ID;
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:,Alloc new Entry Id:%u \n", scl_entry->entry_id);
    }
    else
    {
        SYS_SCL_CHECK_ENTRY_ID(scl_entry->entry_id, TRUE);
        _sys_usw_scl_get_nodes_by_eid(lchip, scl_entry->entry_id, &pe_lkup);
        if (pe_lkup)
        {
            SYS_SCL_UNLOCK(lchip);
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry exists \n");
			return CTC_E_EXIST;

        }
    }

    MALLOC_ZERO(MEM_SCL_MODULE, pe, sizeof(sys_scl_sw_entry_t));
    if (!pe)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }
    MALLOC_ZERO(MEM_SCL_MODULE, pe->temp_entry, sizeof(sys_scl_sw_temp_entry_t));
    if (!pe->temp_entry)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }
    pe->group = pg; /* get chip need this */
    pe->entry_id = scl_entry->entry_id;
    if (inner)
    { /*internal module use scl define key and action type*/
        pe->key_type = scl_entry->key_type; /*sys_scl_key_type_t */
        pe->action_type = scl_entry->action_type;
    }
    else
    {
        ret = _sys_usw_scl_map_key_action_type(scl_entry, pe);
        if (CTC_E_NONE != ret)
        {
            goto error2;
        }
    }

    if(SYS_SCL_ACTION_EGRESS == pe->action_type)
    {
        /* for ingress action only suit for ingress key */
        pe->direction = CTC_EGRESS;
    }
    else
    {
        pe->direction = CTC_INGRESS;
    }
    pe->resolve_conflict = scl_entry->resolve_conflict;
    pe->uninstall = 1;
    pe->head.next = NULL;
    pe->lchip = lchip;
    pe->hash_field_sel_id = scl_entry->hash_field_sel_id;
    pe->vlan_profile_id = 0xFFFF; /*invalid*/
    pe->acl_profile_id = 0xFFFF; /*invalid*/

    ret = _sys_usw_scl_check_action_type(lchip, pe);
    if (CTC_E_NONE != ret)
    {
        goto error2;
    }

    if (!SCL_ENTRY_IS_TCAM(pe->key_type) && !(pe->resolve_conflict))
    {
        /* hash entry */
        if (!inner)
        {
            /* inner module do not need set bitmap, assure by itself */
            ret = _sys_usw_scl_get_hash_sel_bmp(lchip, pe);
            if (CTC_E_NONE != ret)
            {
                goto error2;
            }
        }
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;

    if(SYS_SCL_KEY_HASH_L2 == pe->key_type)
    {
        field_key.data = pe->hash_field_sel_id;
    }

    if (p_usw_scl_master[lchip]->build_key_func[pe->key_type])
    {
        ret = p_usw_scl_master[lchip]->build_key_func[pe->key_type](lchip, &field_key, pe, TRUE);

        if(CTC_E_NONE != ret)
        {
            goto error2;
        }
    }
    else
    {
        goto error2;
    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        MALLOC_ZERO(MEM_SCL_MODULE, pe_tcam, sizeof(sys_scl_sw_tcam_entry_t));
        if (!pe_tcam)
        {
            ret = CTC_E_NO_MEMORY;
            goto error2;
        }
        pe_tcam->entry = pe;
        ret = _sys_usw_scl_tcam_entry_add(lchip, scl_entry, pe_tcam);
        if (ret != CTC_E_NONE)
        {
            goto error3;
        }
    }

    ret = _sys_usw_scl_add_entry(lchip, pg, pe);
    if (ret != CTC_E_NONE)
    {
        goto error4;
    }

    /* all these situations add port info to key by calling corresponding API themselves*/
    if ((CTC_SCL_GROUP_TYPE_NONE != pg->type) && (CTC_SCL_GROUP_TYPE_GLOBAL != pg->type) && (CTC_SCL_GROUP_TYPE_HASH != pg->type))
    {
        if (CTC_SCL_GROUP_TYPE_PORT == pg->type)
        {
            field_port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port_data.gport = pg->gport;
            field_port_mask.gport = 0xFFFF;
        }
        else if (CTC_SCL_GROUP_TYPE_PORT_CLASS == pg->type)
        {
            field_port_data.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port_data.port_class_id = pg->gport;
            field_port_mask.port_class_id = 0xFF;
        }
        else if (CTC_SCL_GROUP_TYPE_LOGIC_PORT == pg->type)
        {
            field_port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port_data.logic_port = pg->gport;
            field_port_mask.logic_port = 0xFFFF;
        }
        if(CTC_INGRESS == pe->direction)
        {
            field_key.type = CTC_FIELD_KEY_PORT;
        }
        else
        {
            field_key.type = CTC_FIELD_KEY_DST_GPORT;
        }
        field_key.ext_data = &field_port_data;
        field_key.ext_mask = &field_port_mask;
        ret = _sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key);
        if (ret != CTC_E_NONE)
        {
            goto error5;
        }
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON;
    if (p_usw_scl_master[lchip]->build_ad_func[pe->action_type] && pe->action_type != SYS_SCL_ACTION_EGRESS && pe->action_type != SYS_SCL_ACTION_MPLS)
    {
        CTC_ERROR_GOTO(p_usw_scl_master[lchip]->build_ad_func[pe->action_type](lchip, &field_action, pe, TRUE), ret, error5)
    }
    /* update statistic data & check*/
    if (!SCL_ENTRY_IS_TCAM(pe->key_type) && !(pe->resolve_conflict))
    {
        uint32 key_id = 0;
        uint32 act_id = 0;
        uint32 entry_size = 0;
        uint32 count_size = 0;
        uint8  scl_id = (SYS_SCL_IS_HASH_COM_MODE(pe) || 2 == p_usw_scl_master[lchip]->hash_mode ? 0: pe->group->priority);
        ctc_ftm_spec_t scl_spec[] = {CTC_FTM_SPEC_SCL, CTC_FTM_SPEC_SCL, CTC_FTM_SPEC_TUNNEL, CTC_FTM_SPEC_SCL_FLOW};

        CTC_ERROR_GOTO(_sys_usw_scl_get_table_id(lchip, scl_id, pe, &key_id, &act_id), ret, error5);
        CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_size(lchip, key_id, &entry_size), ret, error5);
        count_size = entry_size / 12;
        p_usw_scl_master[lchip]->entry_count[pe->action_type] += count_size;

        if (((pe->action_type != SYS_SCL_ACTION_EGRESS) && (p_usw_scl_master[lchip]->entry_count[pe->action_type] > SYS_FTM_SPEC(lchip, scl_spec[pe->action_type])))
            || ((pe->action_type == SYS_SCL_ACTION_EGRESS) && (0 == p_usw_scl_master[lchip]->egs_entry_num)))
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			ret =  CTC_E_NO_RESOURCE;
            goto error5;

        }
    }
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
error5:
    _sys_usw_scl_remove_entry(lchip, pe);
error4:
    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        _sys_usw_scl_tcam_entry_remove(lchip, pe);
    }
error3:
    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        mem_free(pe_tcam);
    }
error2:
    mem_free(pe->temp_entry);
error1:
    mem_free(pe);
error0:
    if (inner)
    {
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;
        opf.pool_index = 0;
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(sys_usw_opf_free_offset(lchip, &opf, 1, (scl_entry->entry_id - SYS_SCL_MIN_INNER_ENTRY_ID)));
    }
    SYS_SCL_UNLOCK(lchip);
    return ret;
}

/* to remove one entry, make sure the entry hash been uninstalled from hw table first */
int32
sys_usw_scl_remove_entry(uint8 lchip, uint32 entry_id, uint8 inner)
{
    sys_scl_sw_entry_t* pe = NULL;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: eid %u \n", entry_id);

    if (!inner)
    {
        SYS_SCL_CHECK_ENTRY_ID(entry_id, FALSE);
    }
    SYS_SCL_LOCK(lchip);
    /* check raw entry */
    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);

    if (!pe)
    {
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
		return CTC_E_NOT_EXIST;

    }

    if (pe->uninstall == 0) /* must uninstall hw table first*/
    {
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Entry installed \n");
		return CTC_E_NOT_READY;

    }
    _sys_usw_scl_remove_entry(lchip, pe);
    if (SCL_ENTRY_IS_TCAM(pe->key_type) || (pe->resolve_conflict))
    {
        _sys_usw_scl_tcam_entry_remove(lchip, pe);
    }
    mem_free(pe->temp_entry);
    mem_free(pe);

    if (inner)
    {
        sys_usw_opf_t opf;
        /* sdk assigned, recycle */
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = 0; /* care not chip */
        opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;

        sys_usw_opf_free_offset(lchip, &opf, 1, (entry_id - SYS_SCL_MIN_INNER_ENTRY_ID));
    }
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_install_entry(uint8 lchip, uint32 eid, uint8 inner)
{
    sys_scl_sw_entry_t* pe = NULL;
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: eid %u \n", eid);
    if (!inner)
    {
        SYS_SCL_CHECK_ENTRY_ID(eid, FALSE);
    }

    SYS_SCL_LOCK(lchip);

    /* check raw entry */
    _sys_usw_scl_get_nodes_by_eid(lchip, eid, &pe);
    if (!pe)
    {
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }

    if (pe->uninstall) /*add new*/
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_install_entry(lchip, pe, 1));
    }
    else if(0 == pe->uninstall && pe->temp_entry)  /*update*/
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_add_hw(lchip, pe));
        mem_free(pe->p_pe_backup);
        pe->p_pe_backup = NULL;
    }

    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}


/*
 * uninstall entry from hardware table
 */
 int32
 sys_usw_scl_uninstall_entry(uint8 lchip, uint32 eid, uint8 inner)
 {
     sys_scl_sw_entry_t  *pe;
     SYS_SCL_INIT_CHECK();

     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: eid %u \n", eid);

     if (!inner)
     {
         SYS_SCL_CHECK_ENTRY_ID(eid, FALSE);
     }
     /* check raw entry */
     SYS_SCL_LOCK(lchip);
     _sys_usw_scl_get_nodes_by_eid(lchip, eid, &pe);

     if (!pe)
     {
         SYS_SCL_UNLOCK(lchip);
         SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

     }
     if (!pe->uninstall)
     {
         CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_uninstall_entry(lchip, pe, 1));
     }
     SYS_SCL_UNLOCK(lchip);

     return CTC_E_NONE;
 }

 int32
 sys_usw_scl_remove_all_entry(uint8 lchip, uint32 group_id, uint8 inner)
 {

     uint32 entry_id                    = 0;
     sys_scl_sw_entry_t* scl_entry = NULL;
     sys_scl_sw_group_t* pg        = NULL;
     struct ctc_slistnode_s*  pe        = NULL;
     struct ctc_slistnode_s*  pe_next   = NULL;

     SYS_SCL_INIT_CHECK();
     if (!inner)
     {
         SYS_SCL_CHECK_OUTER_GROUP_ID(group_id, FALSE);
     }


     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: gid %u \n", group_id);

     /* get group node */
     SYS_SCL_LOCK(lchip);
     _sys_usw_scl_get_group_by_gid(lchip, group_id, &pg);
     SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);

     /* check if all uninstalled */
     CTC_SLIST_LOOP(pg->entry_list, pe)
     {
         if (!((sys_scl_sw_entry_t*) pe)->uninstall)
         {
             SYS_SCL_UNLOCK(lchip);
             SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Entry installed \n");
			return CTC_E_NOT_READY;

         }
     }

     CTC_SLIST_LOOP_DEL(pg->entry_list, pe, pe_next)
     {
         scl_entry = (sys_scl_sw_entry_t *)pe;
         entry_id  = scl_entry->entry_id;

         _sys_usw_scl_remove_entry(lchip, scl_entry);
         if (SCL_ENTRY_IS_TCAM(scl_entry->key_type) || (scl_entry->resolve_conflict))
         {
             _sys_usw_scl_tcam_entry_remove(lchip, scl_entry);
         }
         mem_free(scl_entry->temp_entry);
         mem_free(scl_entry);
         if (inner)
         {
             sys_usw_opf_t opf;
             /* sdk assigned, recycle */
             sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
             opf.pool_index = 0; /* care not chip */
             opf.pool_type  = p_usw_scl_master[lchip]->opf_type_entry_id;

             sys_usw_opf_free_offset(lchip, &opf, 1, (entry_id - SYS_SCL_MIN_INNER_ENTRY_ID));
         }
     }

     SYS_SCL_UNLOCK(lchip);
     return CTC_E_NONE;
 }

int32
sys_usw_scl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority, uint8 inner)
{
    sys_scl_sw_entry_t* pe    = NULL;
    sys_scl_sw_block_t* pb    = NULL;
    ctc_fpa_entry_t*         pfpae = NULL;
    uint8 fpa_size = 0;

    SYS_SCL_INIT_CHECK();

    if (!inner)
    {
        SYS_SCL_CHECK_ENTRY_ID(entry_id, FALSE);
    }

    SYS_SCL_CHECK_ENTRY_PRIO(priority);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: eid %u -- prio %u \n", entry_id, priority);

    /* get sys entry */
    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if (!pe)
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_NOT_EXIST);
    }
    if (!pe->group)
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_NOT_EXIST);
    }
    if (!SCL_ENTRY_IS_TCAM(pe->key_type) && !(pe->resolve_conflict)) /* hash entry */
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    pb  = &p_usw_scl_master[lchip]->block[pe->group->priority];

    /* tcam entry check pb */
    if (!pb)
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_INVALID_PTR);
    }
    pfpae = pb->fpab.entries[pe->key_index];

    fpa_size = p_usw_scl_master[lchip]->fpa_size[pe->key_type];
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(fpa_usw_set_entry_prio(p_usw_scl_master[lchip]->fpa, pfpae, &pb->fpab, fpa_size, priority)); /* may move fpa entries */
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_get_multi_entry(uint8 lchip, ctc_scl_query_t* query, uint8 inner)
{
    uint32                entry_index = 0;
    sys_scl_sw_group_t    * pg        = NULL;
    struct ctc_slistnode_s* pe        = NULL;

    SYS_SCL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(query);
    if (!inner)
    {
        SYS_SCL_CHECK_OUTER_GROUP_ID(query->group_id, FALSE);
    }

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: gid %u -- entry_sz %u\n", query->group_id, query->entry_size);

    /* get group node */
    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_group_by_gid(lchip, query->group_id, &pg);
    SYS_SCL_CHECK_GROUP_UNEXIST(pg, TRUE);
    query->entry_count = pg->entry_list ? CTC_SLISTCOUNT(pg->entry_list) : 0;


    if (query->entry_size && query->entry_array)
    {
        if (query->entry_count > query->entry_size)
        {
            query->entry_count = query->entry_size;
        }

        CTC_SLIST_LOOP(pg->entry_list, pe)
        {
            query->entry_array[entry_index] = ((sys_scl_sw_entry_t *) pe)->entry_id;
            entry_index++;
            if (entry_index == query->entry_count)
            {
                break;
            }
        }
    }
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}
#if 0
int32
sys_usw_scl_copy_entry(uint8 lchip, ctc_scl_copy_entry_t* copy_entry, uint8 inner)
{
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
	return CTC_E_NOT_SUPPORT;

}

int32
sys_usw_scl_update_action(uint8 lchip, uint32 entry_id, ctc_scl_action_t* action, uint8 inner)
{
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
	return CTC_E_NOT_SUPPORT;

}
#endif
int32
sys_usw_scl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key)
{
    CTC_PTR_VALID_CHECK(p_field_key);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_key_type %u\n", entry_id, p_field_key->type);

    SYS_SCL_LOCK(lchip);
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_add_key_field(lchip, entry_id, p_field_key));
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;

}

int32
sys_usw_scl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key)
{
    sys_scl_sw_entry_t* pe = NULL;
    ds_t  remove_ext_data = {0};
    uint8 sys_field_type = SYS_SCL_SHOW_FIELD_KEY_NUM;

    CTC_PTR_VALID_CHECK(p_field_key);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_key_type %u\n", entry_id, p_field_key->type);

    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if (!pe )
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (!pe->temp_entry || !pe->uninstall)
    {
        CTC_USW_RETURN_SCL_UNLOCK(CTC_E_NOT_READY);
    }

    if (!SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        /* hash entry and resolve conflic entry */
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
		return CTC_E_NOT_SUPPORT;

    }

    if(SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        p_field_key->ext_data = remove_ext_data;
        p_field_key->ext_mask = remove_ext_data;
        p_field_key->mask = 0;
    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_check_tcam_key_union(lchip, p_field_key, pe, 0));
    }

    if (p_usw_scl_master[lchip]->build_key_func[pe->key_type])
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(p_usw_scl_master[lchip]->build_key_func[pe->key_type](lchip, p_field_key, pe, FALSE));
    }
    else
    {
        SYS_SCL_UNLOCK(lchip);
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    if (SCL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_set_tcam_key_union(lchip, p_field_key, pe, 0));
    }

    _sys_usw_scl_map_key_field_type(lchip, pe, p_field_key->type, &sys_field_type);

    if (SYS_SCL_SHOW_FIELD_KEY_NUM != sys_field_type)
    {
        CTC_BIT_UNSET(pe->key_bmp, sys_field_type);
    }

    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    sys_scl_sw_entry_t*     pe = NULL;
    int32 ret = 0;
    uint8 free_memroy = 0;

    CTC_PTR_VALID_CHECK(p_field_action);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_action_type %u\n", entry_id, p_field_action->type);


    SYS_SCL_LOCK(lchip);

    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if (!pe)
    {
        CTC_ERROR_GOTO(CTC_E_NOT_EXIST, ret, error0);
    }

    if (!pe->temp_entry && pe->uninstall)
    {
        CTC_ERROR_GOTO(CTC_E_NOT_SUPPORT, ret, error0);
    }


    if (CTC_SCL_FIELD_ACTION_TYPE_PENDING == p_field_action->type)
    {
        if (pe->uninstall)
        {
            CTC_ERROR_GOTO(CTC_E_NONE, ret, error0);
        }

        /*If intstalled entry with peding action, update to db*/
        if (!pe->uninstall && !pe->temp_entry)
        {
            MALLOC_ZERO(MEM_SCL_MODULE, pe->temp_entry, sizeof(sys_scl_sw_temp_entry_t));

            if (NULL == pe->temp_entry)
            {
                CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, error0);
            }

            /*Recovery DB from Hardware*/
            CTC_ERROR_GOTO(_sys_usw_scl_recovery_db_from_hw(lchip, pe, pe->temp_entry), ret, error1);
            free_memroy = 1;
        }
    }

    if (pe->temp_entry)
    {
        CTC_ERROR_GOTO(p_usw_scl_master[lchip]->build_ad_func[pe->action_type](lchip, p_field_action, pe, TRUE), ret, error1);
    }
    else
    {
        /*If intstalled entry with no pending status,  install to hardware instantly*/
        CTC_ERROR_GOTO(_sys_usw_scl_update_action_field(lchip, pe, p_field_action, TRUE), ret, error0);
    }

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;

error1:

    if (free_memroy)
    {
        mem_free(pe->temp_entry);
    }

error0:
    SYS_SCL_UNLOCK(lchip);

    return ret;
}

int32
sys_usw_scl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    sys_scl_sw_entry_t*     pe = NULL;
    int32 ret = 0;

    CTC_PTR_VALID_CHECK(p_field_action);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  %% PARAM: entry_id %u field_key_type %u\n", entry_id, p_field_action->type);


    SYS_SCL_LOCK(lchip);

    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    if (!pe)
    {
        CTC_ERROR_GOTO(CTC_E_NOT_EXIST, ret, error0);
    }

    if (!pe->temp_entry && pe->uninstall)
    {
        CTC_ERROR_GOTO(CTC_E_NOT_SUPPORT, ret, error0);
    }

    if (CTC_SCL_FIELD_ACTION_TYPE_PENDING == p_field_action->type)
    {
        if (pe->uninstall)
        {
            CTC_ERROR_GOTO(CTC_E_NONE, ret, error0);
        }
    }

    if ((pe->temp_entry))
    {
        CTC_ERROR_GOTO(p_usw_scl_master[lchip]->build_ad_func[pe->action_type](lchip, p_field_action, pe, FALSE), ret, error0);
    }
    else
    {
        /*If intstalled entry with no pending status,  install to hardware instantly*/
        CTC_ERROR_GOTO(_sys_usw_scl_update_action_field(lchip, pe, p_field_action, FALSE), ret, error0);
    }

    /*If intstalled entry with peding action, update to db, used for roll back */
    if ( CTC_SCL_FIELD_ACTION_TYPE_PENDING == p_field_action->type &&
        !pe->uninstall && pe->temp_entry)
    {
        mem_free(pe->temp_entry);
    }


    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    SYS_SCL_UNLOCK(lchip);

    return ret;
}

int32
sys_usw_scl_get_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    SYS_SCL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_field_action);

    SYS_SCL_LOCK(lchip);
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_get_action_field(lchip, entry_id, p_field_action));
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_construct_lkup_key(uint8 lchip, ctc_field_key_t* p_field_key, sys_scl_lkup_key_t* p_lkup_key)
{
    sys_scl_sw_entry_t sw_entry;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);
    sal_memset(&sw_entry, 0, sizeof(sys_scl_sw_entry_t));

    sw_entry.temp_entry  =  &p_lkup_key->temp_entry;
    sw_entry.action_type = p_lkup_key->action_type;
    sw_entry.resolve_conflict = p_lkup_key->resolve_conflict;
    if (SYS_SCL_ACTION_EGRESS == sw_entry.action_type)
    {
        sw_entry.direction = CTC_EGRESS;
    }
    else
    {
        sw_entry.direction = CTC_INGRESS;
    }
    sw_entry.key_type = p_lkup_key->key_type;

    if (p_usw_scl_master[lchip]->build_key_func[sw_entry.key_type])
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(p_usw_scl_master[lchip]->build_key_func[sw_entry.key_type](lchip, p_field_key, &sw_entry, TRUE));
    }

    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_scl_get_entry_id_by_lkup_key(uint8 lchip, sys_scl_lkup_key_t* p_lkup_key)
{
    sys_scl_sw_entry_t sw_entry;
    sys_scl_sw_group_t sw_group;
    uint8 is_not_found = 1;
    uint8 loop = 1;
    sys_scl_sw_entry_t* pe_lkup = NULL;

    CTC_PTR_VALID_CHECK(p_lkup_key);
    SYS_SCL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_lkup_key);

    sal_memset(&sw_entry, 0, sizeof(sys_scl_sw_entry_t));
    sal_memset(&sw_group, 0, sizeof(sys_scl_sw_group_t));
    sw_entry.group = &sw_group;

    SYS_SCL_LOCK(lchip);
    sw_entry.temp_entry = &p_lkup_key->temp_entry;
    sw_entry.action_type = p_lkup_key->action_type;
    sw_entry.key_type = p_lkup_key->key_type;
    sw_entry.resolve_conflict = p_lkup_key->resolve_conflict;
    sw_group.priority = p_lkup_key->group_priority;

    if (!SCL_ENTRY_IS_TCAM(p_lkup_key->key_type) && !p_lkup_key->resolve_conflict )
    {/*compare hw table*/

        sys_scl_hash_key_entry_t hash_entry;
        sys_scl_hash_key_entry_t* p_hash_entry;
        uint32 key_index;

        if (CTC_E_EXIST == _sys_usw_scl_asic_hash_lkup(lchip, &sw_entry, 1, 0, &key_index))
        {
            hash_entry.key_index = key_index;
            hash_entry.action_type = p_lkup_key->action_type;
            hash_entry.scl_id = sw_entry.group->priority;
            p_hash_entry = ctc_hash_lookup(p_usw_scl_master[lchip]->hash_key_entry, &hash_entry);
            if (!p_hash_entry)
            {
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
                SYS_SCL_UNLOCK(lchip);
			    return CTC_E_NOT_EXIST;

            }
            p_lkup_key->entry_id = p_hash_entry->entry_id;
            is_not_found = 0;
        }

    }
    else
    {
        sys_scl_tcam_entry_key_t pe_in;
        sys_scl_tcam_entry_key_t* pe_out = NULL;
        sal_memset(&pe_in, 0, sizeof(sys_scl_tcam_entry_key_t));
        /* check tcam entry key exit */
        if (SCL_ENTRY_IS_TCAM(p_lkup_key->key_type) || p_lkup_key->resolve_conflict)
        {
            for (loop = 0; loop < SYS_SCL_MAX_KEY_SIZE_IN_WORD; loop++)
            {
                pe_in.key[loop] = p_lkup_key->temp_entry.key[loop] & p_lkup_key->temp_entry.mask[loop];
            }
            sal_memcpy(pe_in.mask, p_lkup_key->temp_entry.mask, SYS_SCL_MAX_KEY_SIZE_IN_WORD * 4);
            pe_in.scl_id = p_lkup_key->group_priority;
            pe_out = ctc_hash_lookup(p_usw_scl_master[lchip]->tcam_entry_by_key, &pe_in);
            if (pe_out) /* exit */
            {
                p_lkup_key->entry_id = pe_out->p_entry->entry_id;
                is_not_found = 0;
            }
        }
    }

    _sys_usw_scl_get_nodes_by_eid(lchip, p_lkup_key->entry_id, &pe_lkup);
    if (is_not_found
		|| !pe_lkup
		|| !pe_lkup->group
		|| (pe_lkup->group->group_id != p_lkup_key->group_id))
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        SYS_SCL_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_scl_set_hash_field_sel(uint8 lchip, ctc_scl_hash_field_sel_t* field_sel)
{
    SclFlowHashFieldSelect_m   ds_sel;
    uint32 cmd = 0;

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(field_sel);
    CTC_MAX_VALUE_CHECK(field_sel->field_sel_id, 3);

    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);
    if(p_usw_scl_master[lchip]->hash_sel_profile_count[field_sel->field_sel_id])
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Already in used \n");
        SYS_SCL_UNLOCK(lchip);
		return CTC_E_IN_USE;

    }

    sal_memset(&ds_sel, 0, sizeof(SclFlowHashFieldSelect_m));
    if (CTC_SCL_KEY_HASH_L2 == field_sel->key_type)
    {
        SetSclFlowHashFieldSelect(V, etherTypeEn_f,   &ds_sel, !!field_sel->u.l2.eth_type);
        SetSclFlowHashFieldSelect(V, svlanIdValidEn_f, &ds_sel, !!field_sel->u.l2.tag_valid);
        SetSclFlowHashFieldSelect(V, stagCosEn_f,     &ds_sel, !!field_sel->u.l2.cos);
        SetSclFlowHashFieldSelect(V, svlanIdEn_f,     &ds_sel, !!field_sel->u.l2.vlan_id);
        SetSclFlowHashFieldSelect(V, stagCfiEn_f,     &ds_sel, !!field_sel->u.l2.cfi);
        SetSclFlowHashFieldSelect(V, macSaEn_f,       &ds_sel, !!field_sel->u.l2.mac_sa);
        SetSclFlowHashFieldSelect(V, macDaEn_f,       &ds_sel, !!field_sel->u.l2.mac_da);
        SetSclFlowHashFieldSelect(V, userIdLabelEn_f,    &ds_sel, !!field_sel->u.l2.class_id);

        cmd = DRV_IOW(SclFlowHashFieldSelect_t, DRV_ENTRY_FLAG);
    }
    else
    {
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    CTC_USW_ERROR_RETURN_SCL_UNLOCK(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &ds_sel));

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/* this function is used to get group type, further to check if the group type has already set, we do not need to map port info when add entry by struct(only for tcam entry) */
int32
sys_usw_scl_get_group_type(uint8 lchip, uint32 entry_id, uint8* group_type)
{
    sys_scl_sw_entry_t* pe = NULL;
    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe);
    *group_type = pe->group->type;
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_set_default_action(uint8 lchip, sys_scl_default_action_t* p_default_action)
{
    int32  ret   = CTC_E_NONE;
    uint8 action_type = 0;
    uint8 bindingEn = 0;
    uint8 vlan_edit_valid = 0;
    uint8 drv_key_type = 0;
    uint8 is_rollback = 0;
    uint16 lport = 0;
    uint16 profile_id = 0;
    uint32 cmd   = 0;
    uint32 tbl_id = 0;
    uint8 scl_id = 0;
    uint8 act_is_ingress_or_tunnel = 0;
    sys_scl_sw_entry_t* p_sw_entry = NULL;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    ctc_scl_field_action_t* field_action = NULL;
    sys_scl_sp_vlan_edit_t vlan_edit;
    sys_scl_action_acl_t acl_profile;

    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_default_action);
    CTC_MAX_VALUE_CHECK(p_default_action->scl_id, 1);

    sal_memset(&vlan_edit, 0, sizeof(sys_scl_sp_vlan_edit_t));
    sal_memset(&acl_profile, 0, sizeof(sys_scl_action_acl_t));

    lport = p_default_action->lport;
    if (lport  >= SYS_USW_MAX_PORT_NUM_PER_CHIP)
    {
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] Invalid default port\n");
        return CTC_E_INVALID_PORT;
    }
    CTC_MAX_VALUE_CHECK(p_default_action->action_type, SYS_SCL_ACTION_NUM-1);
    SYS_SCL_LOCK(lchip);
    action_type = p_default_action->action_type;
    field_action = p_default_action->field_action;
    is_rollback = p_default_action->is_rollback;
    act_is_ingress_or_tunnel = (SYS_SCL_ACTION_INGRESS == p_default_action->action_type || SYS_SCL_ACTION_TUNNEL == p_default_action->action_type);
    scl_id = (1 == p_usw_scl_master[lchip]->hash_mode ? p_default_action->scl_id : 0);

    if (SYS_SCL_ACTION_TUNNEL == p_default_action->action_type)
    {
        _sys_usw_scl_map_drv_tunnel_type(lchip, p_default_action->hash_key_type, &drv_key_type);
        lport = drv_key_type;
    }

    if (NULL == p_usw_scl_master[lchip]->buffer[lport][action_type])
    {
        MALLOC_ZERO(MEM_SCL_MODULE, p_usw_scl_master[lchip]->buffer[lport][action_type], sizeof(sys_scl_sw_entry_t));
        if (NULL == p_usw_scl_master[lchip]->buffer[lport][action_type])
        {
            SYS_SCL_UNLOCK(lchip);
            return CTC_E_NO_MEMORY;
        }
    }
    p_sw_entry = p_usw_scl_master[lchip]->buffer[lport][action_type];

    if (NULL == p_usw_scl_master[lchip]->buffer[lport][action_type]->temp_entry)
    {
        MALLOC_ZERO(MEM_SCL_MODULE, p_usw_scl_master[lchip]->buffer[lport][action_type]->temp_entry, sizeof(sys_scl_sw_temp_entry_t));
        if (NULL == p_usw_scl_master[lchip]->buffer[lport][action_type]->temp_entry)
        {
            ret = CTC_E_NO_MEMORY;
            goto error0;
        }
    }
    p_temp_entry = p_usw_scl_master[lchip]->buffer[lport][action_type]->temp_entry;

    /* after first time set pending status, check repeating set pending status before cancel pending */
    if (p_sw_entry && p_sw_entry->is_pending && (CTC_SCL_FIELD_ACTION_TYPE_PENDING == field_action->type) && field_action->data0)
    {
        /* keep the previous config still effective so do not need free buffer in master */
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SCL] This entry is already in pending status ! \n");
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_INVALID_CONFIG;
    }

    if(p_sw_entry)
    {
        p_sw_entry->is_default = 1;
    }

    if (CTC_SCL_FIELD_ACTION_TYPE_PENDING == field_action->type && !field_action->data0)
    {
        /* means clear pending status and install entry immediately */
        if (!is_rollback && act_is_ingress_or_tunnel)
        {
            if(p_sw_entry->vlan_edit)
            {
                /* vlan edit valid */
                cmd = DRV_IOW(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_sw_entry->vlan_edit->profile_id, cmd ,&p_sw_entry->vlan_edit->action_profile);
                p_sw_entry->vlan_profile_id = p_sw_entry->vlan_edit->profile_id;
            }
            if(p_sw_entry->acl_profile)
            {
                /* acl profile valid */
                cmd =  DRV_IOW((p_sw_entry->acl_profile->is_scl? DsSclAclControlProfile_t: DsTunnelAclControlProfile_t), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_sw_entry->acl_profile->profile_id, cmd ,&p_sw_entry->acl_profile->acl_control_profile);
                p_sw_entry->acl_profile_id = p_sw_entry->acl_profile->profile_id;
            }

            if (SYS_SCL_ACTION_INGRESS == action_type)
            {
                tbl_id = (0 == scl_id? DsUserId_t : DsUserId1_t);
                cmd = DRV_IOW(tbl_id , DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_usw_scl_master[lchip]->igs_default_base[scl_id] + 2 * lport, cmd, &p_temp_entry->action);
                if (0 == p_usw_scl_master[lchip]->hash_mode)
                {
                    tbl_id = (0 != scl_id? DsUserId_t : DsUserId1_t);
                    cmd = DRV_IOW(tbl_id , DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, p_usw_scl_master[lchip]->igs_default_base[1 - scl_id] + 2 * lport, cmd, &p_temp_entry->action);
                }
                SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Defaut action index: scl0[%u], scl1[%u] ! \n", p_usw_scl_master[lchip]->igs_default_base[0], p_usw_scl_master[lchip]->igs_default_base[1]);
            }
            else
            {
                tbl_id = (0 == scl_id? DsTunnelId_t: DsTunnelId1_t);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_usw_scl_master[lchip]->tunnel_default_base + 2 * drv_key_type, cmd, &p_temp_entry->action);
                if (0 == p_usw_scl_master[lchip]->hash_mode)
                {
                    tbl_id = (0 != scl_id? DsTunnelId_t: DsTunnelId1_t);
                    cmd = DRV_IOW(tbl_id , DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, p_usw_scl_master[lchip]->tunnel_default_base + 2 * drv_key_type, cmd, &p_temp_entry->action);
                }
            }
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  default tunnel index:%u, acl profile id:%u \n", p_usw_scl_master[lchip]->tunnel_default_base + 2 * drv_key_type, p_sw_entry->acl_profile_id);
        }
        else if (!is_rollback && (SYS_SCL_ACTION_EGRESS == action_type))
        {
            cmd = DRV_IOW(DsVlanXlateDefault_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, lport, cmd, &p_temp_entry->action);
        }

        if (is_rollback && (p_usw_scl_master[lchip]->build_ad_func[action_type]))
        {
            /* remove pending only used for roll back */
            p_usw_scl_master[lchip]->build_ad_func[action_type](lchip, field_action, p_sw_entry, FALSE);
        }

        /* need free p_pe_backup which is alloced in enable pending status regardless of success or fail(in fact p_pe_back has free in internal func) */
        mem_free(p_sw_entry->p_pe_backup)

        /* free memory both for rollback and normal cancel pending */
        mem_free(p_usw_scl_master[lchip]->buffer[lport][action_type]->temp_entry);
        p_temp_entry = NULL;
        mem_free(p_usw_scl_master[lchip]->buffer[lport][action_type]);
        p_sw_entry = NULL;
    }
    else
    {
        if (CTC_SCL_FIELD_ACTION_TYPE_PENDING == field_action->type && field_action->data0 && SYS_SCL_ACTION_INGRESS == action_type)
        {
            /* pending status */
            /* invalid value */
            p_sw_entry->vlan_profile_id = 0xFFFF;
            p_sw_entry->acl_profile_id = 0xFFFF;
            p_sw_entry->ad_index = p_usw_scl_master[lchip]->igs_default_base[scl_id] + 2 * lport;
            tbl_id = (0 == scl_id? DsUserId_t : DsUserId1_t);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, p_usw_scl_master[lchip]->igs_default_base[scl_id] + 2 * lport, cmd, &p_temp_entry->action);
            vlan_edit_valid = GetDsUserId(V, vlanActionProfileValid_f, &p_temp_entry->action);
            profile_id = GetDsUserId(V, vlanActionProfileId_f, &p_temp_entry->action);
            bindingEn = GetDsUserId(V, bindingEn_f, &p_temp_entry->action);
            if (vlan_edit_valid && !bindingEn)
            {
                p_sw_entry->vlan_edit = &vlan_edit;
                cmd = DRV_IOR(DsVlanActionProfile_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, profile_id, cmd, &p_sw_entry->vlan_edit->action_profile);
                p_sw_entry->vlan_edit->profile_id = profile_id;
                p_sw_entry->vlan_profile_id = profile_id;
            }
            p_sw_entry->is_pending = 1;
        }
        else if(CTC_SCL_FIELD_ACTION_TYPE_PENDING == field_action->type && field_action->data0 && SYS_SCL_ACTION_TUNNEL == action_type)
        {
            /* pending status */
            /* invalid value */
            p_sw_entry->vlan_profile_id = 0xFFFF;
            p_sw_entry->acl_profile_id = 0xFFFF;
            p_sw_entry->ad_index = p_usw_scl_master[lchip]->tunnel_default_base + 2 * drv_key_type;
            tbl_id = (0 == scl_id? DsTunnelId_t: DsTunnelId1_t);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, p_usw_scl_master[lchip]->tunnel_default_base + 2 * drv_key_type, cmd, &p_temp_entry->action);
            profile_id = GetDsTunnelId(V, aclControlProfileId_f, &p_temp_entry->action);
            if (GetDsTunnelId(V, isAclFlowTcamOperation_f, &p_temp_entry->action) && GetDsTunnelId(V, aclControlProfileIdValid_f, &p_temp_entry->action) && (GetDsTunnelId(V, u3Type_f, &p_temp_entry->action) == 3))
            {
                p_sw_entry->acl_profile = &acl_profile;
                cmd = DRV_IOR(DsTunnelAclControlProfile_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, profile_id, cmd, &p_sw_entry->acl_profile->acl_control_profile);
                p_sw_entry->acl_profile->profile_id = profile_id;
                p_sw_entry->acl_profile_id = profile_id;
            }
            /* free inner router mac is encaped in setting pending status*/
            p_sw_entry->is_pending = 1;
        }
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(p_usw_scl_master[lchip]->build_ad_func[action_type](lchip, field_action, p_sw_entry, TRUE));

    }

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    mem_free(p_sw_entry);
    p_usw_scl_master[lchip]->buffer[lport][action_type] = 0;
    SYS_SCL_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_scl_get_default_action(uint8 lchip, sys_scl_default_action_t* p_default_action)
{
    SYS_SCL_INIT_CHECK();
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_default_action);

    SYS_SCL_LOCK(lchip);
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_get_default_action(lchip, p_default_action));
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_show_entry(uint8 lchip, uint8 type, uint32 param, uint32 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt)
{
    uint8 grep_i = 0;
    SYS_SCL_INIT_CHECK();
    LCHIP_CHECK(lchip);
    if(0xFFFFFFFF == key_type)
    {
        key_type = SYS_SCL_KEY_NUM;
    }
    /* SYS_SCL_KEY_NUM represents all type*/
    CTC_MAX_VALUE_CHECK(key_type, SYS_SCL_KEY_NUM);

    if (DRV_IS_DUET2(lchip) && ((SYS_SCL_KEY_TCAM_UDF == key_type) || (SYS_SCL_KEY_TCAM_UDF_EXT == key_type)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (grep_cnt)
    {
        CTC_PTR_VALID_CHECK(p_grep);
    }

    CTC_MAX_VALUE_CHECK(grep_cnt, 8);

    for (grep_i = 0; grep_i < grep_cnt; grep_i++)
    {
        if (CTC_FIELD_KEY_PORT == p_grep[grep_i].type)
        {
            return CTC_E_NOT_SUPPORT;    /* not support grep CTC_FIELD_KEY_PORT */
        }
    }

    SYS_SCL_LOCK(lchip);

    switch (type)
    {
    case 0:
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_group_entry_key_type(lchip, key_type, detail, type, p_grep, grep_cnt));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        break;

    case 1:
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_entry_by_entry_id(lchip, param, key_type, detail, type, p_grep, grep_cnt));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        break;

    case 2:
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_entry_by_group_id(lchip, param, key_type, detail, type, p_grep, grep_cnt));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        break;

    case 3:
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_tcam_entries(lchip, param, key_type, detail, type, p_grep, grep_cnt));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        break;

    case 4:
       CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_hash_default_entries(lchip, param, detail));
        SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        break;
    default:
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
type
0 : vlan edit
1 : acl
2 : ad
*/
int32
sys_usw_scl_show_spool(uint8 lchip, uint8 type)
{
    uint32 cnt = 0;

    SYS_SCL_INIT_CHECK();
    LCHIP_CHECK(lchip);

    SYS_SCL_LOCK(lchip);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5s %10s %10s\n", "No.", "ProfileId", "RefCnt");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------\n")

    switch (type)
    {
        case 0:
            ctc_spool_traverse(p_usw_scl_master[lchip]->vlan_edit_spool, (spool_traversal_fn)_sys_usw_scl_show_vlan_profile, &cnt);
            break;

        case 1:
        default:
            break;
    }

    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;

}

int32
sys_usw_scl_show_vlan_mapping_entry(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, SYS_SCL_KEY_NUM, CTC_FEATURE_VLAN_MAPPING, 0);
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_scl_show_vlan_class_entry(uint8 lchip, uint8 key_type, uint8 detail)
{
    LCHIP_CHECK(lchip);
    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);
    if (SYS_SCL_KEY_TCAM_MAC == key_type)
    {
        _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type , CTC_FEATURE_VLAN, detail);
        _sys_usw_scl_show_all_group_entry_by_modid(lchip,  SYS_SCL_KEY_HASH_MAC, CTC_FEATURE_VLAN, detail);
    }
    else if(CTC_SCL_KEY_TCAM_IPV4_SINGLE == key_type)
    {
        _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type , CTC_FEATURE_VLAN, detail);
        _sys_usw_scl_show_all_group_entry_by_modid(lchip,  SYS_SCL_KEY_HASH_IPV4, CTC_FEATURE_VLAN, detail);
    }
    else if(CTC_SCL_KEY_TCAM_IPV6 == key_type)
    {
        _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type , CTC_FEATURE_VLAN, detail);
        _sys_usw_scl_show_all_group_entry_by_modid(lchip,  SYS_SCL_KEY_HASH_IPV6, CTC_FEATURE_VLAN, detail);
    }
    SYS_SCL_UNLOCK(lchip);

    return CTC_E_NONE;
}


int32
sys_usw_scl_show_tunnel_entry(uint8 lchip, uint8 key_type, uint8 detail)
{
    LCHIP_CHECK(lchip);
    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IP_TUNNEL:\n");
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type, CTC_FEATURE_IP_TUNNEL, detail);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nSECURITY:\n");
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type, CTC_FEATURE_SECURITY, detail);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nTRILL:\n");
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type, CTC_FEATURE_TRILL, detail);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nWLAN:\n");
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type, CTC_FEATURE_WLAN, detail);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nOVERLAY:\n");
    _sys_usw_scl_show_all_group_entry_by_modid(lchip, key_type, CTC_FEATURE_OVERLAY, detail);

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_show_group_status(uint8 lchip)
{
    SYS_SCL_INIT_CHECK();
    LCHIP_CHECK(lchip);
    SYS_SCL_LOCK(lchip);
    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_show_group_status(lchip));
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_scl_show_status(uint8 lchip)
{
    SYS_SCL_INIT_CHECK();
    SYS_SCL_LOCK(lchip);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "================= SCL Overall Status ==================\n");


    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    _sys_usw_scl_show_priority_status(lchip);

    _sys_usw_scl_show_group_status(lchip);

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_show_tcam_alloc_status(uint8 lchip, uint8 block_id)
{
    char* key_str[4] = {"80 bit Key", "160bit Key", "320bit Key", "640bit Key"};
    sys_scl_sw_block_t *pb = NULL;
    uint8 idx = 0;
    uint8 step = 1;

    SYS_SCL_INIT_CHECK();

    CTC_MAX_VALUE_CHECK(block_id, MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)- 1);

    pb = &(p_usw_scl_master[lchip]->block[block_id]);

    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nblock id: %d\n", block_id);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n(unit 80bit)  entry count: %d, used count: %d", pb->fpab.entry_count, pb->fpab.entry_count - pb->fpab.free_count);
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------------------------------------------------------------\n");
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s%-15s%-15s%-15s%-15s\n", "key size", "range", "entry count", "used count", "rsv count");

    SYS_SCL_LOCK(lchip);
    for(idx = 0; idx < CTC_FPA_KEY_SIZE_NUM; idx++)
    {
        if(pb->fpab.sub_entry_count[idx] > 0)
        {
            SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s[%4d,%4d]    %-15u%-15u%-15u\n", key_str[idx], pb->fpab.start_offset[idx],
                                            pb->fpab.start_offset[idx] + pb->fpab.sub_entry_count[idx] * step - 1,
                                            pb->fpab.sub_entry_count[idx], pb->fpab.sub_entry_count[idx] - pb->fpab.sub_free_count[idx],
                                            pb->fpab.sub_rsv_count[idx]);
        }
        step = step * 2;
    }
    SYS_SCL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------\n");

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_get_overlay_tunnel_entry_status(uint8 lchip, uint8 key_type, uint8 mod_id, uint32* entry_count)
{
    _scl_cb_para_t para;

    SYS_SCL_INIT_CHECK();
    LCHIP_CHECK(lchip);
    SYS_SCL_LOCK(lchip);

    sal_memset(&para, 0, sizeof(_scl_cb_para_t));

    para.grp_mod_id = CTC_FEATURE_OVERLAY;
    para.key_type = key_type;

    ctc_hash_traverse(p_usw_scl_master[lchip]->group, _sys_usw_scl_get_overlay_tunnel_entry_in_group, (void*)(&para));

    *entry_count = para.count;

    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_scl_get_overlay_tunnel_dot1ae_info(uint8 lchip, uint32 entry_id, sys_scl_dot1ae_ingress_t* p_dot1ae_ingress)
{
    sys_scl_sw_entry_t* pe = NULL;
    sys_scl_sw_temp_entry_t temp_entry;
    sys_scl_sw_temp_entry_t* p_temp_entry = NULL;
    ctc_scl_field_action_t field_action;

    CTC_PTR_VALID_CHECK(p_dot1ae_ingress);
    SYS_SCL_INIT_CHECK();

    SYS_SCL_LOCK(lchip);
    sal_memset(&temp_entry, 0, sizeof(sys_scl_sw_temp_entry_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_get_nodes_by_eid(lchip, entry_id, &pe));

    if (!pe)
    {
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_INVALID_PTR;
    }

    if (pe->uninstall)
    {
        SYS_SCL_UNLOCK(lchip);
        return CTC_E_NOT_READY;
    }

    if (NULL != p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_TUNNEL])        /* get dot1ae info */
    {
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(_sys_usw_scl_recovery_db_from_hw(lchip, pe, &temp_entry));
        p_temp_entry = pe->temp_entry;
        pe->temp_entry = &temp_entry;
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS;
        field_action.ext_data = (void*)p_dot1ae_ingress;
        CTC_USW_ERROR_RETURN_SCL_UNLOCK(p_usw_scl_master[lchip]->get_ad_func[SYS_SCL_ACTION_TUNNEL](lchip, &field_action, pe));
        pe->temp_entry = p_temp_entry;
    }
    SYS_SCL_UNLOCK(lchip);
    return CTC_E_NONE;
}

