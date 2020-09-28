#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_ipmc.c

 @date 2011-11-30

 @version v2.0

 The file contains all ipmc related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_ipmc.h"
#include "ctc_stats.h"
#include "ctc_linklist.h"

#include "sys_usw_ftm.h"
#include "sys_usw_chip.h"
#include "sys_usw_common.h"
#include "sys_usw_opf.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_ipmc.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_rpf_spool.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_learning_aging.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
enum ipv4_key_type {
    IPV4KEYTYPE_IPUC,
    IPV4KEYTYPE_IPMC,
    IPV4KEYTYPE_L2MC,
    IPV4KEYTYPE_NATSA,
};

#define SYS_IS_LOOP_PORT(port) ((SYS_RSV_PORT_ILOOP_ID == port) || (SYS_RSV_PORT_IP_TUNNEL == port))

sys_ipmc_master_t* p_usw_ipmc_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

extern int32 sys_usw_nh_remove_all_members(uint8 lchip,  uint32 nhid);

int32
_sys_usw_ipmc_construct_ipv4_hash_key(uint8 lchip,uint8 is_host0_key, uint8 valid,sys_ipmc_group_param_t * p_group_param)
{
    uint32 ipv4type = 0;
    uint32 hash_type = 0;
    ctc_ipmc_group_info_t *p_group = p_group_param->group;

     if (is_host0_key)
    {/* ipmc (*,g) or ip_l2mc/!l2mc_mode (*,g) */

        SetDsFibHost0Ipv4HashKey(V, dsAdIndex_f, &p_group_param->key, p_group_param->ad_index);
        SetDsFibHost0Ipv4HashKey(V, hashType_f, &p_group_param->key, FIBHOST0PRIMARYHASHTYPE_IPV4);
        SetDsFibHost0Ipv4HashKey(V, ipDa_f, &p_group_param->key, p_group_param->group->address.ipv4.group_addr);
        ipv4type =  CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC) ? IPV4KEYTYPE_L2MC : IPV4KEYTYPE_IPMC;
        SetDsFibHost0Ipv4HashKey(V, ipv4Type_f, &p_group_param->key, ipv4type);
        SetDsFibHost0Ipv4HashKey(V, valid_f, &p_group_param->key, valid);
        SetDsFibHost0Ipv4HashKey(V, vrfId_f, &p_group_param->key, p_group_param->group->address.ipv4.vrfid);
        SetDsFibHost0Ipv4HashKey(V, pointer_f,&p_group_param->key, p_group_param->g_node.pointer);  /*AD*/
    }
    else  /*(s,g)*/
    {
        /*  (s,g) and (*,g)of l2 mode == 1*/
        hash_type = CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ?FIBHOST0PRIMARYHASHTYPE_MAC: FIBHOST1PRIMARYHASHTYPE_IPV4;
        SetDsFibHost1Ipv4McastHashKey(V, dsAdIndex_f, &p_group_param->key, p_group_param->ad_index);
        SetDsFibHost1Ipv4McastHashKey(V, hashType_f, &p_group_param->key, hash_type);
        ipv4type =  CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ? IPV4KEYTYPE_L2MC : IPV4KEYTYPE_IPMC;
        SetDsFibHost1Ipv4McastHashKey(V, ipv4Type_f, &p_group_param->key, ipv4type);
        SetDsFibHost1Ipv4McastHashKey(V, valid_f, &p_group_param->key, valid);
        if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) && (1 == p_group_param->l2mc_mode))
        {
            SetDsFibHost1Ipv4McastHashKey(V, ipSa_f, &p_group_param->key, p_group_param->group->address.ipv4.group_addr);
            SetDsFibHost1Ipv4McastHashKey(V, pointer_f, &p_group_param->key, p_group_param->group->address.ipv4.vrfid);
        }
        else
        {
            SetDsFibHost1Ipv4McastHashKey(V, ipSa_f, &p_group_param->key, p_group_param->group->address.ipv4.src_addr);
            SetDsFibHost1Ipv4McastHashKey(V, pointer_f, &p_group_param->key, p_group_param->g_node.pointer); /*key*/
        }
    }

    return CTC_E_NONE;
}
int32
_sys_usw_ipmc_construct_ipv6_hash_key(uint8 lchip,uint8 is_host0_key, uint8 valid, sys_ipmc_group_param_t * p_group_param)
{
    uint32 ipv6type = 0;
    ctc_ipmc_group_info_t *p_group = p_group_param->group;
    uint32 hash_type = 0;
    ctc_ipmc_addr_info_t address;

    sal_memcpy(&address,&p_group->address,sizeof(ctc_ipmc_addr_info_t));

    if (p_group->ip_version)
    {
        SYS_IPMC_GROUP_ADDRESS_SORT(address);
    }
    SYS_IPMC_MASK_ADDR(address, p_group->src_ip_mask_len, p_group->group_ip_mask_len, p_group->ip_version);

    if (is_host0_key && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    { /*(*,g) */
        hash_type =  FIBHOST0PRIMARYHASHTYPE_MACIPV6MCAST;
        SetDsFibHost0MacIpv6McastHashKey(V, dsAdIndex_f, &p_group_param->key, p_group_param->ad_index);
        SetDsFibHost0MacIpv6McastHashKey(V, hashType0_f, &p_group_param->key, hash_type);
        SetDsFibHost0MacIpv6McastHashKey(V, hashType1_f, &p_group_param->key, hash_type);
        SetDsFibHost0MacIpv6McastHashKey(A, ipDa_f, &p_group_param->key, address.ipv6.group_addr);
        SetDsFibHost0MacIpv6McastHashKey(V, valid0_f, &p_group_param->key, valid);
        SetDsFibHost0MacIpv6McastHashKey(V, valid1_f, &p_group_param->key, valid);
        SetDsFibHost0MacIpv6McastHashKey(V, vsiId_f, &p_group_param->key, address.ipv6.vrfid);
        SetDsFibHost0MacIpv6McastHashKey(V, pointer_f, &p_group_param->key, p_group_param->g_node.pointer);/*AD*/
    }
    else if(is_host0_key)
    {
        hash_type = FIBHOST0PRIMARYHASHTYPE_IPV6MCAST;
        SetDsFibHost0Ipv6McastHashKey(V, dsAdIndex_f, &p_group_param->key, p_group_param->ad_index);
        SetDsFibHost0Ipv6McastHashKey(V, hashType0_f, &p_group_param->key, hash_type);
        SetDsFibHost0Ipv6McastHashKey(V, hashType1_f, &p_group_param->key, hash_type);
        SetDsFibHost0Ipv6McastHashKey(A, ipDa_f, &p_group_param->key, address.ipv6.group_addr);
        SetDsFibHost0Ipv6McastHashKey(V, valid0_f, &p_group_param->key, valid);
        SetDsFibHost0Ipv6McastHashKey(V, valid1_f, &p_group_param->key, valid);
        SetDsFibHost0Ipv6McastHashKey(V, vrfId_f, &p_group_param->key, address.ipv6.vrfid);
        SetDsFibHost0Ipv6McastHashKey(V, pointer_f, &p_group_param->key, p_group_param->g_node.pointer);  /*AD*/

    }
    else  /*(s,g) or ip_l2mc + l2mc_mode*/
    {
        hash_type = FIBHOST1PRIMARYHASHTYPE_OTHER;
        SetDsFibHost1Ipv6McastHashKey(V, dsAdIndex_f, &p_group_param->key, p_group_param->ad_index);
        SetDsFibHost1Ipv6McastHashKey(V, hashType0_f, &p_group_param->key, hash_type);
        SetDsFibHost1Ipv6McastHashKey(V, hashType1_f, &p_group_param->key, hash_type);
        ipv6type =  CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ? 3 : 2;
        SetDsFibHost1Ipv6McastHashKey(V, _type_f, &p_group_param->key, ipv6type);
        SetDsFibHost1Ipv6McastHashKey(V, valid0_f, &p_group_param->key, valid);
        SetDsFibHost1Ipv6McastHashKey(V, valid1_f, &p_group_param->key, valid);

        if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) && (1 == p_group_param->l2mc_mode))
        {/*(*,g) */
            SetDsFibHost1Ipv6McastHashKey(A, ipSa_f, &p_group_param->key, address.ipv6.group_addr);
            SetDsFibHost1Ipv6McastHashKey(V, pointer_f, &p_group_param->key, address.ipv6.vrfid);
        }
        else
        {/*(s,g)*/
            SetDsFibHost1Ipv6McastHashKey(A, ipSa_f, &p_group_param->key, address.ipv6.src_addr);
            SetDsFibHost1Ipv6McastHashKey(V, pointer_f, &p_group_param->key, p_group_param->g_node.pointer); /*Key*/
        }
    }

    return CTC_E_NONE;
}


int32
 _sys_usw_ipmc_check(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint32 l2mc_mode = 0;
    uint8 ip_version = 0;
    uint8 loop = 0;
    uint32 rpf_chk_port = 0;
    uint32 cmd = 0;

    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_group);
    CTC_IP_VER_CHECK(p_group->ip_version);
    if (!p_group->member_number)
    {
        if (!(CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_USE_VLAN) || CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC)))
        {
            SYS_IPMC_VRFID_CHECK(p_group->address, p_group->ip_version);
        }
        else
        {
            SYS_L2MC_VRFID_CHECK(p_group->address, p_group->ip_version);
        }
    }
    if ((CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK) || CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU))&& sys_usw_chip_get_rchain_en())
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

#if 0
    if ((p_group->group_ip_mask_len || p_group->src_ip_mask_len)
        && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU))
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
#endif
    if ((CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_DROP) +
            CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU) +
            CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK)) > 1)
    {
            return CTC_E_INVALID_PARAM;
    }

    if (!((0 == p_group->group_ip_mask_len) && (0 == p_group->src_ip_mask_len)))
    {
        SYS_IPMC_MASK_LEN_CHECK(p_group->ip_version, p_group->src_ip_mask_len, p_group->group_ip_mask_len);
        SYS_IPMC_GROUP_ADDRESS_CHECK(p_group->ip_version, p_group->address);
    }

     /*SYS_IPMC_MASK_ADDR(p_group->address, p_group->src_ip_mask_len, p_group->group_ip_mask_len, p_group->ip_version);*/
    cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rpf_chk_port));
    ip_version = p_group->ip_version;
    if (CTC_IP_VER_4 == ip_version)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "source IP is 0x%x, dest group is 0x%x\n",
                         p_group->address.ipv4.src_addr,
                         p_group->address.ipv4.group_addr);
    }
    else
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "source IP is %x:%x:%x:%x, dest group is %x:%x:%x:%x\n",
                         p_group->address.ipv6.src_addr[0],
                         p_group->address.ipv6.src_addr[1],
                         p_group->address.ipv6.src_addr[2],
                         p_group->address.ipv6.src_addr[3],
                         p_group->address.ipv6.group_addr[0],
                         p_group->address.ipv6.group_addr[1],
                         p_group->address.ipv6.group_addr[2],
                         p_group->address.ipv6.group_addr[3]);
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK) \
        && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_BIDPIM_CHECK))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK) \
        && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK))
    {
        for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM; loop++)
        {
            if (!p_group->rpf_intf_valid[loop])
            {
                continue;
            }

            if (!rpf_chk_port
                && p_group->rpf_intf[loop] > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1))
            {
                SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Invalid interface ID! \n");
                return CTC_E_BADID;
            }
            else if (rpf_chk_port)
            {
               SYS_GLOBAL_PORT_CHECK(p_group->rpf_intf[loop]);
            }
        }
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_BIDPIM_CHECK) \
        && !CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    {
        SYS_IPMC_BIDIPIM_CHECK(p_group->rp_id);
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_STATS)
        && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(FibEngineLookupCtl_t, FibEngineLookupCtl_l2mcMacIpParallel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l2mc_mode));

    if ((l2mc_mode == 1) && (p_group->src_ip_mask_len != 0)
        && CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
    {
        sys_nh_info_dsnh_t nh_info;
        if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_SHARE_GROUP) || CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) || !p_group->nhid)
        {
            return CTC_E_INVALID_PARAM;
        }
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_group->nhid, &nh_info, 0));
        if ((p_group->src_ip_mask_len !=0 || p_group->group_ip_mask_len !=0) && (nh_info.nh_entry_type != SYS_NH_TYPE_MCAST))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipmc_write_ipda(uint8 lchip,  sys_ipmc_group_param_t *ipmc_param)
{
    uint32 cmd;
    DsIpDa_m ds_ip_da; /* ds_ipv4_mcast_da_tcam_t is as same as DsIpDa_m */
    DsMac_m ds_mac;
    uint32 ad_index;
    uint32 stats_ptr = 0;
    uint32 dsfwd_offset = 0;
    ctc_ipmc_group_info_t *p_group= ipmc_param->group;

    sal_memset(&ds_ip_da, 0, sizeof(ds_ip_da));
    sal_memset(&ds_mac, 0, sizeof(ds_mac));

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "[PROCESS] _sys_usw_ipmc_write_ipda\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPMC: ad_index  %d\r\n", ipmc_param->ad_index);
    ad_index = ipmc_param->ad_index;
    if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC))
    {
        if (ipmc_param->is_group_exist)
        {
            cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &ds_mac));
        }
        else
        {
            SetDsMac(V, adNextHopPtr_f, &ds_mac, 0);
        }

        if (p_group->group_id)
        {
            SetDsMac(V, nextHopPtrValid_f, &ds_mac, 1);
            SetDsMac(V, adDestMap_f, &ds_mac, SYS_ENCODE_MCAST_IPE_DESTMAP(p_group->group_id));
        }

        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &ds_mac));
    }
    else
    {
        if (ipmc_param->is_group_exist)
        {
            cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipmc_param->ad_index, cmd, &ds_ip_da));

            SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 1);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, 0);
        }

        SetDsIpDa(V, ttlCheckEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_TTL_CHECK)?1:0);
        SetDsIpDa(V, selfAddress_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_PTP_ENTRY)?1:0);

        SetDsIpDa(V, exception3CtlEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_COPY_TOCPU)?1:0);
        /*to CPU must be equal to 63(get the low 5bits is 31), refer to sys_usw_pdu.h:SYS_L3PDU_PER_L3IF_ACTION_INDEX_RSV_IPDA_TO_CPU*/
        if(DRV_IS_DUET2(lchip))
		{
		    SetDsIpDa(V, exceptionSubIndex_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_COPY_TOCPU)?(63 & 0x1F):0);
		}
		else
		{
		    SetDsIpDa(V, exceptionSubIndex_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_COPY_TOCPU)?(63 & 0x3F):0);
		}

        if (p_group->src_ip_mask_len !=0 || p_group->group_ip_mask_len !=0 )
        {

            /*rpf check*/
            SetDsIpDa(V, rpfCheckEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK)?1:0);
            SetDsIpDa(V, rpfCheckMode_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK)?ipmc_param->rpf_mode:0);
            /* bidipim check */
            SetDsIpDa(V, bidiPimGroupValid_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_BIDPIM_CHECK)?1:0);
            if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK))
            {
                SetDsIpDa(V, rpfIfId_f, &ds_ip_da, ipmc_param->rpf_id);
                SetDsIpDa(V, mcastRpfFailCpuEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU)?1:0);
            }
            else if(CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_BIDPIM_CHECK))
            {
                SetDsIpDa(V, rpfIfId_f, &ds_ip_da, p_group->rp_id);
                SetDsIpDa(V, mcastRpfFailCpuEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_BIDIPIM_FAIL_BRIDGE_AND_TOCPU)?1:0);
            }
            else
            {
                SetDsIpDa(V, rpfIfId_f, &ds_ip_da, 0);
                SetDsIpDa(V, mcastRpfFailCpuEn_f, &ds_ip_da, 0);
            }

            if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_STATS))
            {
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_group->stats_id, &stats_ptr));
                if (!CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_DROP))
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, ipmc_param->nh_id, &dsfwd_offset, 0));
                }
                /*clear u1g2*/
                SetDsIpDa(V, adApsBridgeEn_f, &ds_ip_da, 0);
                SetDsIpDa(V, adDestMap_f, &ds_ip_da, 0);
                SetDsIpDa(V, adLengthAdjustType_f, &ds_ip_da, 0);
                SetDsIpDa(V, adMacDaLowBits_f, &ds_ip_da, 0);
                SetDsIpDa(V, adNextHopExt_f, &ds_ip_da, 0);
                SetDsIpDa(V, adNextHopPtr_f, &ds_ip_da, 0);
                SetDsIpDa(V, adNextHopSaveEn_f, &ds_ip_da, 0);
				if(DRV_IS_TSINGMA(lchip))
				{
					SetDsIpDa(V, adMacDaLowBits_f, &ds_ip_da, 0);
					SetDsIpDa(V, adNextHopSaveEn_f, &ds_ip_da, 0);
				}
                /*set u1g1*/
                SetDsIpDa(V, tunnelGreOption_f, &ds_ip_da, stats_ptr&0x7);
                SetDsIpDa(V, tunnelPacketType_f, &ds_ip_da, (stats_ptr>>3)&0x7);
                SetDsIpDa(V, tunnelPayloadOffset_f, &ds_ip_da, (stats_ptr>>6)&0xF);
                SetDsIpDa(V, payloadOffsetType_f, &ds_ip_da, (stats_ptr>>10)&0x1);
                SetDsIpDa(V, payloadSelect_f, &ds_ip_da, (stats_ptr>>11)&0x3);
                if(DRV_IS_DUET2(lchip))
                {
                    SetDsIpDa(V, statsPtrHighBits_f, &ds_ip_da, (stats_ptr >> 13)&0x3);
                }
                else
                {
                    SetDsIpDa(V, statsPtrHighBits_f, &ds_ip_da, (stats_ptr >> 13)&0x7);
                }

                SetDsIpDa(V, statsOrDecapSel_f, &ds_ip_da, 1);
                SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 0);
                SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, dsfwd_offset);
            }
            else if (ipmc_param->is_group_exist && (1 == GetDsIpDa(V, statsOrDecapSel_f, &ds_ip_da)))
            {
                if (!CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_DROP))
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, ipmc_param->nh_id, &dsfwd_offset, 0));
                }
                SetDsIpDa(V, tunnelGreOption_f, &ds_ip_da, 0);
                SetDsIpDa(V, tunnelPacketType_f, &ds_ip_da, 0);
                SetDsIpDa(V, tunnelPayloadOffset_f, &ds_ip_da, 0);
                SetDsIpDa(V, payloadOffsetType_f, &ds_ip_da, 0);
                SetDsIpDa(V, payloadSelect_f, &ds_ip_da, 0);
                SetDsIpDa(V, statsPtrHighBits_f, &ds_ip_da, 0);

                SetDsIpDa(V, statsOrDecapSel_f, &ds_ip_da, 0);
                SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 0);
                SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, dsfwd_offset);
            }
            else if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
            {
                sys_nh_info_dsnh_t nh_info;
                sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_group->nhid, &nh_info, 0));
                SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 1);
                SetDsIpDa(V, adDestMap_f, &ds_ip_da, nh_info.dest_map);
            }
            else if (p_group->group_id)
            {
                SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 1);
                SetDsIpDa(V, adDestMap_f, &ds_ip_da, SYS_ENCODE_MCAST_IPE_DESTMAP(p_group->group_id));
            }
        }
        else
        {/*default route*/
            /* rpf check */
            SetDsIpDa(V, rpfCheckEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_CHECK)?1:0);
            SetDsIpDa(V, rpfIfId_f, &ds_ip_da, 0xFFFF);
            SetDsIpDa(V, rpfCheckMode_f, &ds_ip_da, 1);
            SetDsIpDa(V, mcastRpfFailCpuEn_f, &ds_ip_da, CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU)?1:0);

            SetDsIpDa(V, isDefaultRoute_f, &ds_ip_da, 1);
         }

         if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_DROP))
         { /*drop*/
            SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 0);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, 0xFFFF);
         }

        if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU))
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, SYS_NH_RESOLVED_NHID_FOR_TOCPU, &dsfwd_offset, 0));
            SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 0);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, dsfwd_offset);
        }

        if (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_group->nhid, &dsfwd_offset, 0));
            SetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da, 0);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da, dsfwd_offset);
        }

        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipmc_param->ad_index, cmd, &ds_ip_da));
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ipmc_ipv4_write_key(uint8 lchip, sys_ipmc_group_param_t *ipmc_param,uint8 is_del)
{
    drv_acc_in_t  key_acc_in;
    drv_acc_out_t key_acc_out;
    uint8 valid = 0;
    uint8 l2mc_mode ;
    uint8 is_host0_key;

    sal_memset(&key_acc_in, 0, sizeof(drv_acc_in_t));
    sal_memset(&key_acc_out, 0, sizeof(drv_acc_out_t));

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&ipmc_param->key, 0, sizeof(ipmc_param->key));
    if (ipmc_param->is_g_pointer_key)
    {
        is_host0_key = 1;
    }
    else
    {
        l2mc_mode = ipmc_param->l2mc_mode && CTC_FLAG_ISSET(ipmc_param->group->flag, CTC_IPMC_FLAG_L2MC);
        is_host0_key = (ipmc_param->group->src_ip_mask_len == 0) && !l2mc_mode;
    }

    valid = is_host0_key ? (is_del ? 0 : 1) : 1;
    _sys_usw_ipmc_construct_ipv4_hash_key(lchip,is_host0_key, valid, ipmc_param);

    if (is_host0_key)
    { /* ipmc (*,g) or ip_l2mc/!l2mc_mode (*,g) */
        key_acc_in.tbl_id = DsFibHost0Ipv4HashKey_t;
    }
    else
    {  /*  (s,g) and (*,g)of l2 mode == 1*/
       key_acc_in.tbl_id = DsFibHost1Ipv4McastHashKey_t;
    }

    if (is_host0_key)
    {
        key_acc_in.type = DRV_ACC_TYPE_ADD;
        key_acc_in.data = (void*)&ipmc_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_BIT_SET(key_acc_in.flag, DRV_ACC_OVERWRITE_EN);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    }
    else
    {
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.data = (void *)&ipmc_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;

        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        if (!key_acc_out.is_conflict)
        {
            key_acc_in.type = is_del?DRV_ACC_TYPE_DEL:DRV_ACC_TYPE_ADD;
            key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
            key_acc_in.index = key_acc_out.key_index;
            CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        }
    }

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Write %s key_index:%d, ad_index: %d, conflict: %d, hit: %d\n",
	     (is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index,ipmc_param->ad_index,key_acc_out.is_conflict,key_acc_out.is_hit);

    if (key_acc_out.is_conflict == TRUE)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPMC] Ipmc hash conflict \n");
		return CTC_E_HASH_CONFLICT;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipmc_ipv6_write_key(uint8 lchip, sys_ipmc_group_param_t *ipmc_param,uint8 is_del)
{
    drv_acc_in_t  key_acc_in;
    drv_acc_out_t key_acc_out;
    uint8 valid = 0;
    uint8 l2mc_mode = 0;
    uint8 is_host0_key = 0;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&key_acc_in, 0, sizeof(drv_acc_in_t));
    sal_memset(&key_acc_out, 0, sizeof(drv_acc_out_t));
    sal_memset(&ipmc_param->key, 0, sizeof(ipmc_param->key));

    if (ipmc_param->is_g_pointer_key)
    {
        is_host0_key = 1;
    }
    else
    {
        l2mc_mode = ipmc_param->l2mc_mode && CTC_FLAG_ISSET(ipmc_param->group->flag, CTC_IPMC_FLAG_L2MC);
        is_host0_key = ipmc_param->group->src_ip_mask_len == 0 && !l2mc_mode;
    }

    valid = is_host0_key ? (is_del ? 0 : 1) : 1;
    _sys_usw_ipmc_construct_ipv6_hash_key(lchip,is_host0_key, valid, ipmc_param);

    if (is_host0_key)
    {
        key_acc_in.tbl_id = CTC_FLAG_ISSET(ipmc_param->group->flag, CTC_IPMC_FLAG_L2MC) ?DsFibHost0MacIpv6McastHashKey_t:DsFibHost0Ipv6McastHashKey_t;
    }
    else
    { /*(s,g) or (*,g) + l2mc_mode*/
        key_acc_in.tbl_id = DsFibHost1Ipv6McastHashKey_t;
    }

    if (is_host0_key)
    {
        key_acc_in.type = DRV_ACC_TYPE_ADD;
        key_acc_in.data = (void*)&ipmc_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_BIT_SET(key_acc_in.flag, DRV_ACC_OVERWRITE_EN);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    }
    else
    {
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.data = (void *)&ipmc_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        if (!key_acc_out.is_conflict)
        {
            key_acc_in.type = is_del?DRV_ACC_TYPE_DEL:DRV_ACC_TYPE_ADD;
            key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
            key_acc_in.index = key_acc_out.key_index;
            CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        }
    }

   SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Write %s key_index:%d, ad_index: %d, conflict: %d, hit: %d\n",
	     (is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index,ipmc_param->ad_index,key_acc_out.is_conflict,key_acc_out.is_hit);

    if (key_acc_out.is_conflict == TRUE)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPMC] Ipmc hash conflict \n");
		return CTC_E_HASH_CONFLICT;
    }

    return CTC_E_NONE;
}
int32
_sys_usw_ipmc_write_key(uint8 lchip, sys_ipmc_group_param_t *ipmc_param, uint8 is_del)
{
    uint8  ip_version = 0;

    ip_version = ipmc_param->group->ip_version ? CTC_IP_VER_6 : CTC_IP_VER_4;
    if (ipmc_param->group->src_ip_mask_len == 0 && ipmc_param->group->group_ip_mask_len == 0)
    {
        return CTC_E_NONE;
    }

    if (CTC_IP_VER_4 == ip_version)
    {
        return _sys_usw_ipmc_ipv4_write_key(lchip, ipmc_param, is_del);
    }
    else
    {
        return _sys_usw_ipmc_ipv6_write_key(lchip, ipmc_param, is_del);
    }

    return CTC_E_NONE;
}

sys_ipmc_g_node_t *
_sys_usw_ipmc_lkup_g_pointer(uint8 lchip, sys_ipmc_g_node_t *p_g_node)
{
    return ctc_hash_lookup(p_usw_ipmc_master[lchip]->g_hash[p_g_node->ipmc_type], p_g_node);
}

int32
_sys_usw_ipmc_add_g_pointer(uint8 lchip, sys_ipmc_group_param_t *p_group_param)
{
    sys_ipmc_g_node_t *p_new_g_node = NULL;
    sys_ipmc_g_node_t *p_bucket_g_node = NULL;

    ctc_hash_t *p_g_hash;
    ctc_hash_bucket_t* bucket;
    sys_usw_opf_t opf;

    uint8   use_opf = 1;
    uint8   exist_hash_pointer = 0;
    uint32  index = 0;
    uint32  idx_1d  = 0;
    uint32  idx_2d  = 0;
    int32   ret = CTC_E_NONE;

    sal_memset(&opf,0,sizeof(sys_usw_opf_t));
    p_g_hash = p_usw_ipmc_master[lchip]->g_hash[p_group_param->ipmc_type];
    p_new_g_node = ctc_hash_lookup2(p_g_hash, &p_group_param->g_node, &index);
    if (p_new_g_node)
    {
        p_new_g_node->ref_cnt++;
        p_new_g_node->is_group =  p_group_param->group->src_ip_mask_len ? p_new_g_node->is_group : 1;

        p_group_param->g_node.pointer = p_new_g_node->pointer;
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get (*,G) pointer %d \n",p_new_g_node->pointer);

        return CTC_E_NONE;
    }

    /*creat (*,g) node*/
    p_new_g_node = mem_malloc(MEM_IPMC_MODULE,sizeof(sys_ipmc_g_node_t));
    if(!p_new_g_node)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    sal_memcpy(p_new_g_node,&p_group_param->g_node,sizeof(sys_ipmc_g_node_t));
    p_new_g_node->ref_cnt = 1;
    p_new_g_node->is_group =  p_group_param->group->src_ip_mask_len ? 0 : 1;

    idx_1d  = index / p_g_hash->block_size;
    idx_2d  = index % p_g_hash->block_size;
    if (p_g_hash->index[idx_1d] == NULL ||
        p_g_hash->index[idx_1d][idx_2d] == NULL)
    {
        use_opf = 0;
    }
    else
    {
        for(bucket = p_g_hash->index[idx_1d][idx_2d]; bucket != NULL; bucket = bucket->next)
        {
            p_bucket_g_node = bucket->data;
            if (((p_bucket_g_node->ipmc_type == SYS_IPMC_V4
				|| p_bucket_g_node->ipmc_type == SYS_IPMC_V4_L2MC)
				 && p_bucket_g_node->pointer <= SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE)
				 || (p_bucket_g_node->ipmc_type == SYS_IPMC_V6
				 && p_bucket_g_node->pointer <= SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE)
				 || (p_bucket_g_node->ipmc_type == SYS_IPMC_V6_L2MC
				 && p_bucket_g_node->pointer <= SYS_USW_IPMC_L2_V6_G_POINTER_HASH_SIZE))
            {
                exist_hash_pointer = 1;
                break;
            }
        }
        use_opf = exist_hash_pointer;
    }

    if (use_opf)
    {
        opf.pool_type = p_usw_ipmc_master[lchip]->opf_g_pointer[p_new_g_node->ipmc_type];
        opf.pool_index = 0;
        ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &index);
        if (ret)
        {
            ret = (ret == CTC_E_NO_RESOURCE) ? CTC_E_HASH_CONFLICT : ret;
            goto error0;
        }

        p_new_g_node->pointer = index & 0xFFFF;
    }
    else
    {
        /*increase 1 to avoid pointer equal to zero*/
        p_new_g_node->pointer = index + 1;
    }

    p_bucket_g_node = ctc_hash_insert(p_g_hash, p_new_g_node);
    if (!p_bucket_g_node)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    if (p_group_param->group->src_ip_mask_len && (p_new_g_node->ref_cnt == 1))
    { /*only add (s,g) group's  (*,G) pointer*/
        sys_ipmc_group_param_t ipmc_param2;
        sal_memcpy(&ipmc_param2, p_group_param, sizeof(sys_ipmc_group_param_t));
        ipmc_param2.is_g_pointer_key = 1;
        ipmc_param2.g_node.pointer = p_new_g_node->pointer;
        ipmc_param2.ad_index  = p_usw_ipmc_master[lchip]->default_base[ipmc_param2.ipmc_type] + (sys_usw_l3if_get_default_entry_mode(lchip) ? 0 : p_group_param->g_node.vrfid) ;/*use Default AD*/
        CTC_ERROR_GOTO(_sys_usw_ipmc_write_key(lchip, &ipmc_param2, 0), ret, error2);
    }
    p_group_param->g_node.pointer = p_new_g_node->pointer;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Alloc new (*,G) pointer %d \n",p_new_g_node->pointer);

    return CTC_E_NONE;

error2:
    ctc_hash_remove(p_g_hash, p_new_g_node);

error1:
    if (use_opf)
    {
        sys_usw_opf_free_offset(lchip, &opf, 1, p_new_g_node->pointer);
    }

error0:
    mem_free(p_new_g_node);

    return ret;
}

sys_ipmc_g_node_t *
_sys_usw_ipmc_remove_g_pointer(uint8 lchip,sys_ipmc_group_param_t *p_group_param)
{
    sys_ipmc_g_node_t *p_new_g_node = NULL;
    ctc_hash_t *p_g_hash = NULL;
    uint8  use_opf = 1;

    if (!p_group_param)
    {
        return NULL;
    }
    p_g_hash = p_usw_ipmc_master[lchip]->g_hash[p_group_param->ipmc_type];
    p_new_g_node = ctc_hash_lookup(p_g_hash,&p_group_param->g_node);
    if (!p_new_g_node)
    {
        return NULL;
    }

    p_new_g_node->is_group =  p_group_param->group->src_ip_mask_len ? p_new_g_node->is_group:0;
    p_new_g_node->ref_cnt--;

    if (p_group_param->group->src_ip_mask_len && !p_new_g_node->ref_cnt)
    {
        sys_ipmc_group_param_t ipmc_param2;
        sal_memcpy(&ipmc_param2, p_group_param, sizeof(sys_ipmc_group_param_t));
        ipmc_param2.is_g_pointer_key = 1;
        _sys_usw_ipmc_write_key(lchip,  &ipmc_param2,1);
    }

    if (p_new_g_node->ref_cnt != 0 || (p_new_g_node->is_group && !p_group_param->is_group_exist) )
    {
        return p_new_g_node;
    }
	/*delete (*.g) Node*/
    if (p_new_g_node->ipmc_type == SYS_IPMC_V4 || p_new_g_node->ipmc_type == SYS_IPMC_V4_L2MC  )
    {
        use_opf = p_new_g_node->pointer > SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE ;
    }
    else if (p_new_g_node->ipmc_type == SYS_IPMC_V6)
    {
        use_opf = p_new_g_node->pointer > SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE ;
    }
    else if (p_new_g_node->ipmc_type == SYS_IPMC_V6_L2MC)
    {
        use_opf = p_new_g_node->pointer > SYS_USW_IPMC_L2_V6_G_POINTER_HASH_SIZE ;
    }

    if (use_opf)
    {
        sys_usw_opf_t opf;
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = p_usw_ipmc_master[lchip]->opf_g_pointer[p_group_param->ipmc_type];
        opf.pool_index = 0;
        sys_usw_opf_free_offset(lchip, &opf, 1, p_new_g_node->pointer);
    }

    ctc_hash_remove(p_g_hash, p_new_g_node);
    mem_free(p_new_g_node);
    p_new_g_node = NULL;

    return p_new_g_node;
}

STATIC int32
_sys_usw_ipmc_asic_ipv4_lookup(uint8 lchip, sys_ipmc_group_param_t* p_group_param)
{
    drv_acc_in_t  key_acc_in;
    drv_acc_out_t key_acc_out;
    uint8 l2mc_mode = 0;
    uint8 is_host0_key = 0;

    sal_memset(&key_acc_in, 0, sizeof(drv_acc_in_t));
    sal_memset(&key_acc_out, 0, sizeof(drv_acc_out_t));
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&p_group_param->key, 0, sizeof(p_group_param->key));

    /*lookup (*,G) to get pointer*/
    if (p_group_param->group->src_ip_mask_len
        && p_group_param->group->group_ip_mask_len)
    {
        is_host0_key = 1;
        _sys_usw_ipmc_construct_ipv4_hash_key(lchip,is_host0_key, 1, p_group_param);
        key_acc_in.tbl_id = DsFibHost0Ipv4HashKey_t;
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.data = (void *)&p_group_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));

        if(!key_acc_out.is_hit)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;
        }

        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.index = key_acc_out.key_index;
        key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        p_group_param->g_node.pointer = GetDsFibHost0Ipv4HashKey(V, pointer_f, &key_acc_out.data);
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lookup %s key_index: %d conflict: %d, hit: %d\n",
                         (is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index, key_acc_out.is_conflict, key_acc_out.is_hit);
        p_group_param->g_key_index = key_acc_out.key_index;
        p_group_param->g_key_tbl_id = key_acc_in.tbl_id;


        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lookup (*,G) pointer:%d\n", p_group_param->g_node.pointer);
    }

    /*lookup mcast group from hw table*/
    l2mc_mode = p_group_param->l2mc_mode && CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC);
    is_host0_key = p_group_param->group->src_ip_mask_len == 0 && !l2mc_mode;
    _sys_usw_ipmc_construct_ipv4_hash_key(lchip, is_host0_key, 1, p_group_param);
    if(is_host0_key)
    { /* ipmc (*,g) or ip_l2mc/!l2mc_mode (*,g) */
        key_acc_in.tbl_id = DsFibHost0Ipv4HashKey_t;
    }
    else
    {
        key_acc_in.tbl_id = DsFibHost1Ipv4McastHashKey_t;
    }

    key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
    key_acc_in.data = (void *)&p_group_param->key;
    key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    p_group_param->is_conflict = key_acc_out.is_conflict;
    p_group_param->is_key_exist = key_acc_out.is_hit;
    p_group_param->key_index = key_acc_out.key_index;
    if (key_acc_out.is_hit)
    {
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.index = key_acc_out.key_index;
        key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    }

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lookup %s key_index:%d, conflict: %d, hit: %d",
    (is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index,key_acc_out.is_conflict,key_acc_out.is_hit);

    if(!p_group_param->is_key_exist)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if(p_group_param->is_conflict)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
		return CTC_E_HASH_CONFLICT;
    }

    if(is_host0_key)
    {
        p_group_param->ad_index = GetDsFibHost0Ipv4HashKey(V, dsAdIndex_f, &key_acc_out.data);
        p_group_param->g_node.pointer = GetDsFibHost0Ipv4HashKey(V, pointer_f, &key_acc_out.data);
    }
    else
    {
        p_group_param->ad_index = GetDsFibHost1Ipv4McastHashKey(V, dsAdIndex_f, &key_acc_out.data);
    }

    p_group_param->tbl_id = key_acc_in.tbl_id;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ad_index:%d\n",p_group_param->ad_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipmc_asic_ipv6_lookup(uint8 lchip, sys_ipmc_group_param_t* p_group_param)
{
    drv_acc_in_t  key_acc_in;
    drv_acc_out_t key_acc_out;
    uint8 l2mc_mode = 0;
    uint8 is_host0_key = 0;
    sal_memset(&key_acc_in, 0, sizeof(drv_acc_in_t));
    sal_memset(&key_acc_out, 0, sizeof(drv_acc_out_t));
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

	/*lookup (*,G) to get pointer*/
    if (p_group_param->group->src_ip_mask_len
        && p_group_param->group->group_ip_mask_len)
    {
        is_host0_key = 1;
        _sys_usw_ipmc_construct_ipv6_hash_key(lchip,is_host0_key,1, p_group_param);
        key_acc_in.tbl_id = CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC) ? DsFibHost0MacIpv6McastHashKey_t:DsFibHost0Ipv6McastHashKey_t;
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.data = (void *)&p_group_param->key;
        key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));


        if (!key_acc_out.is_hit)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;
        }


        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.index = key_acc_out.key_index;
        key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
        if ( CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC))
        {
            p_group_param->g_node.pointer = GetDsFibHost0MacIpv6McastHashKey(V, pointer_f, &key_acc_out.data);
        }
        else
        {
            p_group_param->g_node.pointer = GetDsFibHost0Ipv6McastHashKey(V, pointer_f, &key_acc_out.data);
        }
        p_group_param->g_key_index = key_acc_out.key_index;
        p_group_param->g_key_tbl_id = key_acc_in.tbl_id;


        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lookup %s key_index: %d, conflict: %d, hit: %d\n",
                  (is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index, key_acc_out.is_conflict, key_acc_out.is_hit);


        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lookup (*,G) pointer:%d\n", p_group_param->g_node.pointer);
    }

    /*lookup mcast group from hw table*/
    l2mc_mode = p_group_param->l2mc_mode && CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC);
    is_host0_key = (p_group_param->group->src_ip_mask_len == 0) && !l2mc_mode;

    _sys_usw_ipmc_construct_ipv6_hash_key(lchip,is_host0_key,1, p_group_param);
    if (is_host0_key)
    {
        key_acc_in.tbl_id = CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC) ? DsFibHost0MacIpv6McastHashKey_t:DsFibHost0Ipv6McastHashKey_t;
    }
    else
    {
        key_acc_in.tbl_id = DsFibHost1Ipv6McastHashKey_t;
    }

    key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
    key_acc_in.data = (void *)&p_group_param->key;
    key_acc_in.op_type = DRV_ACC_OP_BY_KEY;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    p_group_param->is_conflict = key_acc_out.is_conflict;
    p_group_param->is_key_exist = key_acc_out.is_hit;
    p_group_param->key_index = key_acc_out.key_index;
    if (key_acc_out.is_hit)
    {
        key_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        key_acc_in.index = key_acc_out.key_index;
        key_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &key_acc_in, &key_acc_out));
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lookup %s key_index:%d, conflict: %d, hit: %d\n",
	(is_host0_key?"FibHost0":"FibHost1"), key_acc_out.key_index,key_acc_out.is_conflict,key_acc_out.is_hit);

    if(!p_group_param->is_key_exist)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if(p_group_param->is_conflict)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
		return CTC_E_HASH_CONFLICT;
    }

    if (is_host0_key && CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC))
    {
        p_group_param->ad_index = GetDsFibHost0MacIpv6McastHashKey(V, dsAdIndex_f, &key_acc_out.data);
        p_group_param->g_node.pointer = GetDsFibHost0MacIpv6McastHashKey(V, pointer_f, &key_acc_out.data);
    }
    else if(is_host0_key)
    {
        p_group_param->ad_index = GetDsFibHost0Ipv6McastHashKey(V, dsAdIndex_f, &key_acc_out.data);
        p_group_param->g_node.pointer = GetDsFibHost0Ipv6McastHashKey(V, pointer_f, &key_acc_out.data);
    }
    else
    {
        p_group_param->ad_index = GetDsFibHost1Ipv6McastHashKey(V, dsAdIndex_f, &key_acc_out.data);
    }

    p_group_param->tbl_id = key_acc_in.tbl_id;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ad_index:%d \n",p_group_param->ad_index);

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_asic_lookup(uint8 lchip, sys_ipmc_group_param_t* p_group_param)
{
    DsIpDa_m ds_ip_da; /* ds_ipv4_mcast_da_tcam_t is as same as DsIpDa_m */
    DsMac_m ds_mac;
    uint32 cmd = 0;
    uint32 l2mc_mode = 0;
    uint32 dest_map = 0;
    uint32 dsfwd_ptr = 0;
    uint16 vrfid = 0;
    int32 ret = CTC_E_NONE;
    sys_ipmc_g_node_t  *p_ipmc_g_node = NULL;
    ctc_ipmc_group_info_t *p_group = p_group_param->group;

    if (!p_group->src_ip_mask_len)
    {
        cmd = DRV_IOR(FibEngineLookupCtl_t, FibEngineLookupCtl_l2mcMacIpParallel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l2mc_mode));
        p_group_param->l2mc_mode = (l2mc_mode ? 1 : 0);
    }

    if (p_group_param->group->ip_version)
    {
        vrfid = p_group->address.ipv6.vrfid;
        p_group_param->ipmc_type = CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ? SYS_IPMC_V6_L2MC : SYS_IPMC_V6;
    }
    else
    {
        vrfid = p_group->address.ipv4.vrfid;
        p_group_param->ipmc_type = CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ? SYS_IPMC_V4_L2MC : SYS_IPMC_V4;
    }

    if (p_group->src_ip_mask_len == 0 && p_group->group_ip_mask_len == 0 )
    {
        p_group_param->is_default_route = 1;
        p_group_param->ad_index = p_usw_ipmc_master[lchip]->default_base[p_group_param->ipmc_type] + (sys_usw_l3if_get_default_entry_mode(lchip) ? 0 : vrfid) ;
        return CTC_E_NONE;
    }
    /*mapping g_node & lkup g_node from g_node DB*/
    if (p_group->group_ip_mask_len)
    {
        sal_memset(&p_group_param->g_node, 0, sizeof(sys_ipmc_g_node_t));
        if (!p_group->ip_version)
        {
            p_group_param->g_node.ip_addr.ipv4 = p_group->address.ipv4.group_addr;
            p_group_param->g_node.vrfid = vrfid;
        }
        else
        {
            sal_memcpy(&p_group_param->g_node.ip_addr.ipv6[0], &p_group->address.ipv6.group_addr[0], sizeof(ipv6_addr_t));
            p_group_param->g_node.vrfid = vrfid;
        }
        p_group_param->g_node.ipmc_type = p_group_param->ipmc_type;
        p_ipmc_g_node = _sys_usw_ipmc_lkup_g_pointer(lchip, &p_group_param->g_node);
        if (p_ipmc_g_node != NULL)
        {
            p_group_param->g_node.is_group = p_ipmc_g_node->is_group;
        }
    }
   /*lookup from hw table*/
    if (p_group_param->group->ip_version)
    {
        ret = (_sys_usw_ipmc_asic_ipv6_lookup(lchip, p_group_param));
    }
    else
    {
        ret = (_sys_usw_ipmc_asic_ipv4_lookup(lchip, p_group_param));
    }
    if (ret)
    {
        return ret;
    }

    if(p_ipmc_g_node && p_ipmc_g_node->pointer != p_group_param->g_node.pointer)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(*,G) Pointer is mismatch\n");
    }

    if(!p_group->src_ip_mask_len && p_group->group_ip_mask_len)
    { /*(*,G)*/
        p_group_param->is_group_exist = p_group_param->g_node.is_group && p_group_param->is_key_exist;
    }
    else
    {
        p_group_param->is_group_exist = p_group_param->is_key_exist;
    }

    if(!p_group_param->is_group_exist)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
		return CTC_E_NOT_EXIST;
    }

    if (CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_L2MC))
    {
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_group_param->ad_index, cmd, &ds_mac));
        dest_map = GetDsMac(V, adDestMap_f,&ds_mac);
        if(GetDsMac(V, nextHopPtrValid_f, &ds_mac) && (dest_map >>18) )
        {
            p_group_param->hw_grp_id = dest_map & 0xFFFF;
        }
        p_group_param->rpf_mode = 0;
        p_group_param->rpf_id = 0;
        p_group_param->rpf_chk_en = 0;
    }
    else
    {
        uint8 mergeDsFwd = 0;
        uint32 dsfwd_offset = 0;
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, SYS_NH_RESOLVED_NHID_FOR_TOCPU, &dsfwd_offset, 0));
        cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_group_param->ad_index, cmd, &ds_ip_da));
        dest_map = GetDsIpDa(V, adDestMap_f,&ds_ip_da);
        dsfwd_ptr = GetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da);
        p_group_param->rp_chk_en = GetDsIpDa(V, bidiPimGroupValid_f, &ds_ip_da);
        p_group_param->rpf_mode = GetDsIpDa(V, rpfCheckMode_f, &ds_ip_da);
        p_group_param->rpf_id = GetDsIpDa(V, rpfIfId_f, &ds_ip_da);
        p_group_param->rpf_chk_en =  GetDsIpDa(V, rpfCheckEn_f, &ds_ip_da);
        mergeDsFwd = GetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da);
        if (!mergeDsFwd)
        {
            p_group_param->is_drop = (dsfwd_ptr == 0xFFFF);
            p_group_param->is_rd_cpu = (dsfwd_ptr == dsfwd_offset);
        }
        if ( mergeDsFwd && (dest_map >>18) )
        {
            p_group_param->hw_grp_id = dest_map & 0xFFFF;
        }
        else if(!mergeDsFwd && (dsfwd_ptr != 0xFFFF) && (dsfwd_ptr != dsfwd_offset))
        {
            DsFwd_m dsfwd;
            cmd = DRV_IOR(DsFwd_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsfwd_ptr/2, cmd, &dsfwd));
            dest_map = GetDsFwd(V, destMap_f, &dsfwd) ;
            if(dest_map >>18)
            {
                p_group_param->hw_grp_id = dest_map & 0xFFFF;
            }
        }
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lookup mcast group ID: %d, rpfChkEn: %d, rpfMode: %d, rpfid: %d\n",p_group_param->hw_grp_id,
    					p_group_param->rpf_chk_en,p_group_param->rpf_mode, p_group_param->rpf_id);

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_mapping_group_node(ctc_ipmc_group_info_t* p_group_info, sys_ipmc_group_node_t *p_ipmc_group_node)
{
    CTC_PTR_VALID_CHECK(p_group_info);
    CTC_PTR_VALID_CHECK(p_ipmc_group_node);

    if (p_group_info->ip_version)
    {
        p_ipmc_group_node->address.ipv6.vrfid = p_group_info->address.ipv6.vrfid;
        sal_memcpy(p_ipmc_group_node->address.ipv6.group_addr, p_group_info->address.ipv6.group_addr, sizeof(ipv6_addr_t));
        sal_memcpy(p_ipmc_group_node->address.ipv6.src_addr, p_group_info->address.ipv6.src_addr, sizeof(ipv6_addr_t));
    }
    else
    {
        p_ipmc_group_node->address.ipv4.vrfid = p_group_info->address.ipv4.vrfid;
        p_ipmc_group_node->address.ipv4.group_addr = p_group_info->address.ipv4.group_addr;
        p_ipmc_group_node->address.ipv4.src_addr = p_group_info->address.ipv4.src_addr;
    }

    p_ipmc_group_node->is_drop = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_DROP);
    p_ipmc_group_node->is_l2mc = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_L2MC);
    p_ipmc_group_node->share_grp = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_SHARE_GROUP);
    p_ipmc_group_node->with_nexthop = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP);
    p_ipmc_group_node->group_ip_mask_len = p_group_info->group_ip_mask_len;
    p_ipmc_group_node->src_ip_mask_len  = p_group_info->src_ip_mask_len;
    p_ipmc_group_node->ip_version = p_group_info->ip_version ? 1 : 0;
    p_ipmc_group_node->is_rd_cpu = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU);

    return CTC_E_NONE;
}

sys_ipmc_group_node_t *
sys_usw_ipmc_get_group_from_db(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    ctc_list_pointer_t  *p_linklist = NULL;
    ctc_list_pointer_node_t* p_node = NULL;
    sys_ipmc_group_node_t *p_ipmc_group_node, ipmc_group_node;
    uint8 addr_size = 0;
    p_linklist = ctc_vector_get( p_usw_ipmc_master[lchip]->group_vec, p_group_info->group_id);
    if (!p_linklist)
    {
        return NULL;
    }

    _sys_usw_ipmc_mapping_group_node(p_group_info, &ipmc_group_node);

    addr_size = p_group_info->ip_version ? sizeof(ctc_ipmc_ipv6_addr_t) : sizeof(ctc_ipmc_ipv4_addr_t);

    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_ipmc_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        if( (ipmc_group_node.ip_version == p_ipmc_group_node->ip_version)
                && !sal_memcmp(&ipmc_group_node.address, &p_ipmc_group_node->address, addr_size)
                && (ipmc_group_node.is_l2mc == p_ipmc_group_node->is_l2mc))
        {
            return p_ipmc_group_node;
        }
    }
    return NULL;
}

int32
sys_usw_ipmc_add_group_to_db(uint8 lchip, sys_ipmc_group_param_t* p_group_param)
{
    uint8 first_add = 0;
    uint8 db_info_size = 0;
    uint16 group_id = 0;
    sys_ipmc_group_node_t *p_ipmc_group_node = NULL;
    ctc_list_pointer_t  *p_linklist = NULL, *p_linklist_t = NULL;

    group_id = (CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_DROP) || CTC_FLAG_ISSET(p_group_param->group->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU)) ? 0 : p_group_param->group->group_id;

    db_info_size = p_group_param->group->ip_version ? sizeof(sys_ipmc_group_node_t) :
        (sizeof(sys_ipmc_group_node_t) - sizeof(ctc_ipmc_ipv6_addr_t) + sizeof(ctc_ipmc_ipv4_addr_t));
    if (p_group_param->is_group_exist)
    {
        p_linklist_t = ctc_vector_get(p_usw_ipmc_master[lchip]->group_vec, p_group_param->hw_grp_id);
        if (!p_linklist_t)
        {
            return CTC_E_NOT_EXIST;
        }

        p_group_param->group->group_id = p_group_param->hw_grp_id;
        p_ipmc_group_node = sys_usw_ipmc_get_group_from_db(lchip, p_group_param->group);
        if (!p_ipmc_group_node)
        {
            return CTC_E_NOT_EXIST;
        }
    }

    p_linklist = ctc_vector_get(p_usw_ipmc_master[lchip]->group_vec, group_id);
    if (!p_linklist)
    {
        p_linklist = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_list_pointer_t));
        if (!p_linklist)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;
        }
        first_add = 1;
        ctc_list_pointer_init(p_linklist);
    }

    if (p_group_param->is_group_exist)
    {
        if (group_id)
        {
            p_ipmc_group_node->is_drop = 0;
            p_ipmc_group_node->is_rd_cpu = 0;
        }

        ctc_list_pointer_delete(p_linklist_t, &p_ipmc_group_node->list_head);
        if (CTC_LIST_POINTER_ISEMPTY(p_linklist_t))
       {
           ctc_vector_del( p_usw_ipmc_master[lchip]->group_vec, p_group_param->hw_grp_id);
           mem_free(p_linklist_t);
       }
    }
    else
    {
        p_ipmc_group_node = mem_malloc(MEM_IPMC_MODULE, db_info_size);
        if (!p_ipmc_group_node)
        {
            if (first_add)
            {
                mem_free(p_linklist);
            }
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    		return CTC_E_NO_MEMORY;
        }

        _sys_usw_ipmc_mapping_group_node(p_group_param->group, p_ipmc_group_node);
        p_ipmc_group_node->ad_index = p_group_param->ad_index;
    }

    if (CTC_LIST_POINTER_ISEMPTY(p_linklist))
    {
        ctc_list_pointer_insert_head(p_linklist, &p_ipmc_group_node->list_head);
    }
    else
    {
        ctc_list_pointer_insert_tail(p_linklist, &p_ipmc_group_node->list_head);
    }

    if (!ctc_vector_add(p_usw_ipmc_master[lchip]->group_vec, group_id, p_linklist))
    {
        if (first_add)
        {
            mem_free(p_linklist);
        }
        mem_free(p_ipmc_group_node);

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_delete_group_to_db(uint8 lchip, ctc_ipmc_group_info_t* p_group_info, uint8* del_nh)
{
    uint8 addr_size = 0;
    ctc_list_pointer_t  *p_linklist = NULL;
    ctc_list_pointer_node_t* p_node = NULL;
    sys_ipmc_group_node_t *p_ipmc_group_node = NULL;
    sys_ipmc_group_node_t ipmc_group_node;
    sys_ipmc_group_node_t tmp_ipmc_group_node;

    sal_memset(&ipmc_group_node, 0, sizeof(sys_ipmc_group_node_t));
    p_linklist = ctc_vector_get(p_usw_ipmc_master[lchip]->group_vec, p_group_info->group_id);
    if (!p_linklist)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        return CTC_E_NOT_EXIST;
    }

    _sys_usw_ipmc_mapping_group_node(p_group_info, &ipmc_group_node);
    SYS_IPMC_MASK_ADDR(ipmc_group_node.address, ipmc_group_node.src_ip_mask_len,
                ipmc_group_node.group_ip_mask_len, ipmc_group_node.ip_version);

    addr_size = p_group_info->ip_version ? sizeof(ctc_ipmc_ipv6_addr_t) : sizeof(ctc_ipmc_ipv4_addr_t);

    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_ipmc_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        sal_memcpy(&tmp_ipmc_group_node.address, &p_ipmc_group_node->address, addr_size);
        SYS_IPMC_MASK_ADDR(tmp_ipmc_group_node.address, p_ipmc_group_node->src_ip_mask_len,
                p_ipmc_group_node->group_ip_mask_len, p_ipmc_group_node->ip_version);

        if ((ipmc_group_node.ip_version == p_ipmc_group_node->ip_version)
                && !sal_memcmp(&ipmc_group_node.address, &tmp_ipmc_group_node.address, addr_size)
                && (ipmc_group_node.is_l2mc == p_ipmc_group_node->is_l2mc))
        {
            *del_nh = !(p_ipmc_group_node->share_grp || p_ipmc_group_node->is_drop || p_ipmc_group_node->with_nexthop || p_ipmc_group_node->is_rd_cpu);

            ctc_list_pointer_delete(p_linklist, &(p_ipmc_group_node->list_head));
            mem_free(p_ipmc_group_node);
            break;
        }
    }

   if (CTC_LIST_POINTER_ISEMPTY(p_linklist))
   {
       ctc_vector_del( p_usw_ipmc_master[lchip]->group_vec, p_group_info->group_id);
       mem_free(p_linklist);
   }
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_add_group(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    int32 ret = 0;
    uint32 tbl_id = 0;
    uint32 group_count = 0;
    uint8 loop = 0;
    sys_ipmc_group_param_t ipmc_param;
    sys_rpf_info_t rpf_info;
    sys_nh_info_dsnh_t nhinfo;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group_info));

    sal_memset(&rpf_info, 0, sizeof(sys_rpf_info_t));
    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    IPMC_LOCK;

    ipmc_param.group = p_group_info;
    ipmc_param.nh_id = p_group_info->nhid;
    ret = _sys_usw_ipmc_asic_lookup(lchip, &ipmc_param);
    if (ipmc_param.is_group_exist && !CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_SHARE_GROUP))
    {
        /* The group to add hash already existed*/
        CTC_RETURN_IPMC_UNLOCK(CTC_E_EXIST);
    }
    else if (ipmc_param.is_conflict)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_HASH_CONFLICT);
    }

    group_count = (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4])
                   + (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4_L2MC])
                   + (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6]*2)
                   + (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6_L2MC]*2);
    if (!ipmc_param.is_group_exist && (group_count >= SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPMC)))
    {
        IPMC_UNLOCK;
        return CTC_E_NO_RESOURCE;
    }

    /*default route*/
    if (ipmc_param.is_default_route)
    {
        _sys_usw_ipmc_write_ipda(lchip, &ipmc_param);
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NONE);
    }
    /*rpf check*/
    if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_RPF_CHECK))
    {
        for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM && loop < CTC_IP_MAX_RPF_IF ; loop++)
        {
            if (p_group_info->rpf_intf_valid[loop])
            {
                rpf_info.rpf_intf[loop] = p_group_info->rpf_intf[loop];
                rpf_info.rpf_intf_cnt++;
            }
        }
        rpf_info.is_ipmc = 1;
        ret = sys_usw_rpf_add_profile(lchip, &rpf_info);
        ipmc_param.rpf_mode = rpf_info.mode;
        ipmc_param.rpf_id =  rpf_info.rpf_id;
        ipmc_param.rpf_chk_en = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_RPF_CHECK);

       if (ret != CTC_E_NONE)
       {
           goto error0;
       }
    }


    /* creat Nexthop for IP Mcast according to Group id */
    if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_SHARE_GROUP))
    {
        if (p_group_info->group_id)
        {
            CTC_ERROR_GOTO(sys_usw_nh_get_mcast_nh(lchip, p_group_info->group_id, &ipmc_param.nh_id), ret, error1);
        }
        else if (!ipmc_param.is_group_exist)
        {
            CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_DROP);
        }
    }
    else if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_DROP) &&
            !CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))/*dummy entry, no need to create nexthop*/
    {
        ipmc_param.nh_id = CTC_NH_RESERVED_NHID_FOR_DROP;
    }
    else if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU) &&
            !CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
    {
        ipmc_param.nh_id = CTC_NH_RESERVED_NHID_FOR_TOCPU;
    }
    else if (!CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
    {
        sys_nh_param_mcast_group_t  nh_mcast_group;
        sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));
        CTC_ERROR_GOTO(sys_usw_mcast_nh_create(lchip, p_group_info->group_id,  &nh_mcast_group), ret, error1);
        ipmc_param.nh_id = nh_mcast_group.nhid;
    }

    if (ipmc_param.is_group_exist)
    {
        ret = _sys_usw_ipmc_write_ipda(lchip,&ipmc_param);

        if (!p_group_info->group_id && (ipmc_param.hw_grp_id == p_group_info->group_id))
        {
            sys_ipmc_group_param_t ipmc_param1 = {0};
            ipmc_param1.group = p_group_info;

            CTC_ERROR_GOTO(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param1), ret, error1);
            p_group_info->group_id = ipmc_param1.hw_grp_id;
        }

        if (ipmc_param.hw_grp_id != p_group_info->group_id)
        {
            ret = sys_usw_ipmc_add_group_to_db(lchip, &ipmc_param);
        }
        goto error1;
    }

    if(CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
    {
        sys_usw_nh_get_nhinfo(lchip, p_group_info->nhid, &nhinfo, 0);
        ipmc_param.group->group_id = nhinfo.gport;
    }

    /*create (*,g) node*/
    CTC_ERROR_GOTO(_sys_usw_ipmc_add_g_pointer(lchip, &ipmc_param), ret, error2);


    tbl_id = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_L2MC) ? DsMac_t : DsIpDa_t;
    ret = sys_usw_ftm_alloc_table_offset(lchip, tbl_id, CTC_INGRESS, 1, 1, &(ipmc_param.ad_index));
    if (ret != CTC_E_NONE)
    {
        goto error3;
    }

    ret = _sys_usw_ipmc_write_ipda(lchip,&ipmc_param);
    if (ret != CTC_E_NONE)
    {
        goto error4;
    }

    ret = _sys_usw_ipmc_write_key(lchip, &ipmc_param, 0);
    if (ret != CTC_E_NONE)
    {
        goto error4;
    }

    ret = sys_usw_ipmc_add_group_to_db(lchip, &ipmc_param);
    if (ret != CTC_E_NONE)
    {
        goto error5;
    }

    if (p_group_info->group_ip_mask_len != 0 )
    {
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[ipmc_param.ipmc_type]++;
    }
    if (p_group_info->group_ip_mask_len != 0 && p_group_info->src_ip_mask_len)
    {
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[ipmc_param.ipmc_type]++;
    }

    IPMC_UNLOCK;
    return CTC_E_NONE;

error5:
     _sys_usw_ipmc_write_key(lchip, &ipmc_param, 1);
error4:
    sys_usw_ftm_free_table_offset(lchip, tbl_id, CTC_INGRESS, 1, ipmc_param.ad_index);
error3:
    _sys_usw_ipmc_remove_g_pointer(lchip, &ipmc_param);
error2:
    if (!CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP))
    {
        sys_usw_mcast_nh_delete(lchip, ipmc_param.nh_id);
    }
error1:
    sys_usw_rpf_remove_profile(lchip,&rpf_info);
error0:

    IPMC_UNLOCK;
    return ret;
}

int32
sys_usw_ipmc_remove_group(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    sys_ipmc_group_param_t ipmc_param;
    sys_ipmc_g_node_t    *p_g_node = NULL;
    sys_rpf_info_t rpf_info;
    uint8 del_nh = 0;
    uint8 delete_group = 0;
    uint32 tbl_id = 0;
    uint32 ad_index = 0;
    uint32 nh_id = 0;
    uint16 group_id = 0;
    int32 ret = CTC_E_NONE;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    sal_memset(&rpf_info, 0, sizeof(sys_rpf_info_t));
    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    IPMC_LOCK;
    ipmc_param.group = p_group_info;

    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param));

    if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_KEEP_EMPTY_ENTRY))
    {
        group_id = ipmc_param.hw_grp_id ? ipmc_param.hw_grp_id : p_group_info->group_id;
        CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_nh_get_mcast_nh(lchip, group_id, &nh_id));

        CTC_RETURN_IPMC_UNLOCK(sys_usw_nh_remove_all_members(lchip, nh_id));
    }
	 /*default route*/
    if (ipmc_param.is_default_route)
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }
    ad_index = ipmc_param.ad_index;

    p_group_info->group_id = ipmc_param.hw_grp_id;
    /*remove (*,g node)*/
    if (p_group_info->group_ip_mask_len)
    {
        p_g_node = _sys_usw_ipmc_remove_g_pointer(lchip, &ipmc_param);
    }

    if (p_group_info->src_ip_mask_len || !p_g_node)
    { /*delete key*/
        delete_group = 1;
    }
    else
    {  /*update key*/
        /*use Default AD*/
        delete_group = 0;
        ipmc_param.ad_index =  p_usw_ipmc_master[lchip]->default_base[ipmc_param.ipmc_type] +
            (sys_usw_l3if_get_default_entry_mode(lchip) ? 0 : ipmc_param.g_node.vrfid);
    }
    _sys_usw_ipmc_write_key(lchip,  &ipmc_param, delete_group);

    /*remove rpf profile*/
    rpf_info.is_ipmc = 1;
    rpf_info.mode = ipmc_param.rpf_chk_en ? ipmc_param.rpf_mode:SYS_RPF_CHK_MODE_IFID;
    rpf_info.rpf_id = ipmc_param.rpf_id;
    sys_usw_rpf_remove_profile(lchip, &rpf_info);

    if (p_group_info->group_ip_mask_len)
    {
        tbl_id = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_L2MC) ? DsMac_t : DsIpDa_t;
        sys_usw_ftm_free_table_offset(lchip, tbl_id, CTC_INGRESS, 1, ad_index);
    }

    if (p_group_info->group_ip_mask_len)
    {
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[ipmc_param.ipmc_type]--;
        if (p_group_info->src_ip_mask_len)
        {
            p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[ipmc_param.ipmc_type]--;
        }
    }

    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_ipmc_delete_group_to_db(lchip,  p_group_info, &del_nh));
    if(del_nh)
    {
        CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_nh_get_mcast_nh(lchip, p_group_info->group_id, &ipmc_param.nh_id));
        /*remove nexthop*/
        sys_usw_mcast_nh_delete(lchip, ipmc_param.nh_id);
    }

    IPMC_UNLOCK;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipmc_update_member(uint8 lchip, ctc_ipmc_member_info_t* ipmc_member, sys_nh_param_mcast_group_t* nh_mcast_group, uint8  ip_l2mc)
{
    uint8 aps_brg_en = 0;
    uint32 dest_id = 0;

    if (ipmc_member->re_route)
    {
        nh_mcast_group->mem_info.destid  = SYS_RSV_PORT_IP_TUNNEL;
        nh_mcast_group->mem_info.member_type = SYS_NH_PARAM_IPMC_MEM_LOOP_BACK;
        dest_id = SYS_RSV_PORT_IP_TUNNEL;
    }
    else if (ipmc_member->is_nh)
    {
        nh_mcast_group->mem_info.ref_nhid = ipmc_member->nh_id;

        if (CTC_FLAG_ISSET(ipmc_member->flag, CTC_IPMC_MEMBER_FLAG_ASSIGN_PORT))
        {
            nh_mcast_group->mem_info.destid = CTC_MAP_GPORT_TO_LPORT(ipmc_member->global_port);
            nh_mcast_group->mem_info.member_type = SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH_DESTID;
            dest_id = ipmc_member->global_port;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_port(lchip, ipmc_member->nh_id, &aps_brg_en, &dest_id));
            nh_mcast_group->mem_info.destid  = aps_brg_en ? dest_id : CTC_MAP_GPORT_TO_LPORT(dest_id);
            nh_mcast_group->mem_info.member_type = aps_brg_en ? SYS_NH_PARAM_BRGMC_MEM_LOCAL_WITH_APS_BRIDGE :
                                               SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH;
        }
    }
    else
    {
        nh_mcast_group->mem_info.destid  = CTC_MAP_GPORT_TO_LPORT(ipmc_member->global_port);
        dest_id = ipmc_member->global_port;

        if (ipmc_member->remote_chip)
        {
            nh_mcast_group->mem_info.member_type = SYS_NH_PARAM_MCAST_MEM_REMOTE;
        }
        else if (ipmc_member->vlan_port
            || (ip_l2mc && (ipmc_member->l3_if_type == MAX_L3IF_TYPE_NUM)))
        {
            nh_mcast_group->mem_info.member_type = SYS_NH_PARAM_BRGMC_MEM_LOCAL;
        }
        else
        {
            nh_mcast_group->mem_info.member_type = SYS_NH_PARAM_IPMC_MEM_LOCAL;
        }

        if (CTC_L3IF_TYPE_PHY_IF == ipmc_member->l3_if_type)
        {
            /* don't care vlan id in case of Routed Port */
            nh_mcast_group->mem_info.vid     = 0;
        }
        else
        {
            /* vlan interface */
            nh_mcast_group->mem_info.vid     = ipmc_member->vlan_id;
        }

        nh_mcast_group->mem_info.l3if_type       = ipmc_member->l3_if_type;
    }

    nh_mcast_group->mem_info.is_linkagg = 0;
    nh_mcast_group->mem_info.lchip = lchip;
    nh_mcast_group->mem_info.leaf_check_en = CTC_FLAG_ISSET(ipmc_member->flag, CTC_IPMC_MEMBER_FLAG_LEAF_CHECK_EN);
    nh_mcast_group->mem_info.mtu_no_chk = CTC_FLAG_ISSET(ipmc_member->flag, CTC_IPMC_MEMBER_FLAG_MTU_NO_CHECK);

    if (0 != ipmc_member->logic_dest_port)
    {
        nh_mcast_group->mem_info.logic_port = ipmc_member->logic_dest_port;
        nh_mcast_group->mem_info.is_logic_port = 1;
    }

    if (aps_brg_en || CTC_IS_LINKAGG_PORT(dest_id) || SYS_IS_LOOP_PORT(dest_id))
    {
        if (CTC_IS_LINKAGG_PORT(dest_id))
        {
            nh_mcast_group->mem_info.is_linkagg  = 1;
        }
    }
    else
    {
        if (FALSE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(dest_id)))
        {
            return CTC_E_NONE;
        }
    }

    return sys_usw_mcast_nh_update(lchip, nh_mcast_group);
}

STATIC int32
_sys_usw_ipmc_update_member_with_bitmap(uint8 lchip, ctc_ipmc_member_info_t* ipmc_member, sys_nh_param_mcast_group_t* nh_mcast_group, uint8  ip_l2mc)
{
    uint8 loop1 = 0;
    uint8 loop2 = 0;
    uint16 lport = 0;
    int32  ret = 0;
    uint8 lchip_temp = 0;

    if (ipmc_member->gchip_id != CTC_LINKAGG_CHIPID)
    {
        SYS_MAP_GCHIP_TO_LCHIP(ipmc_member->gchip_id, lchip_temp);
        SYS_LCHIP_CHECK_ACTIVE(lchip_temp);
    }

    if (CTC_FLAG_ISSET(ipmc_member->flag, CTC_IPMC_MEMBER_FLAG_LEAF_CHECK_EN) || (0 != ipmc_member->logic_dest_port))
    {
        return CTC_E_INVALID_PARAM;
    }

    for (loop1 = 0; loop1 < sizeof(ipmc_member->port_bmp) / 4; loop1++)
    {
        if (ipmc_member->port_bmp[loop1] == 0)
        {
            continue;
        }
        for (loop2 = 0; loop2 < 32; loop2++)
        {
            if (!CTC_IS_BIT_SET(ipmc_member->port_bmp[loop1], loop2))
            {
                continue;
            }
            lport = loop2 + loop1 * 32;
            ipmc_member->global_port = CTC_MAP_LPORT_TO_GPORT(ipmc_member->gchip_id, lport);
            ret = _sys_usw_ipmc_update_member(lchip, ipmc_member, nh_mcast_group, ip_l2mc);
            if ((ret < 0) && (nh_mcast_group->opcode == SYS_NH_PARAM_MCAST_ADD_MEMBER))
            {
                return ret;
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_add_member(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 member_index = 0;
    int32 ret = 0;
    uint8 ip_l2mc = 0;
    sys_ipmc_group_param_t ipmc_param;
    sys_nh_param_mcast_group_t nh_mcast_group;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    CTC_PTR_VALID_CHECK(p_group);
    CTC_MAX_VALUE_CHECK(p_group->member_number, CTC_IPMC_MAX_MEMBER_PER_GROUP);

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));
    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group));

    IPMC_LOCK;
    ipmc_param.group = p_group;
    ret = _sys_usw_ipmc_asic_lookup(lchip, &ipmc_param);
    if(CTC_E_NONE != ret)
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }

    if (ipmc_param.is_drop || ipmc_param.is_rd_cpu)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    ip_l2mc = CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC); // ??shoud used sw db info??
    ret = sys_usw_nh_get_mcast_nh(lchip, ipmc_param.hw_grp_id, &ipmc_param.nh_id);
    if(ret != CTC_E_NONE)
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }
    nh_mcast_group.nhid = ipmc_param.nh_id;
    nh_mcast_group.dsfwd_valid = 0;
    nh_mcast_group.opcode = SYS_NH_PARAM_MCAST_ADD_MEMBER;

    for (member_index = 0; member_index < p_group->member_number; member_index++)
    {
        if ((p_group->ipmc_member[member_index].l3_if_type >= MAX_L3IF_TYPE_NUM)
            && !(p_group->ipmc_member[member_index].is_nh)
            && !(p_group->ipmc_member[member_index].re_route)
            && !(ip_l2mc))
        {
            IPMC_UNLOCK;
            return CTC_E_INVALID_PARAM;
        }

        ret = p_group->ipmc_member[member_index].port_bmp_en ?
            _sys_usw_ipmc_update_member_with_bitmap(lchip, &(p_group->ipmc_member[member_index]), &nh_mcast_group, ip_l2mc):
            _sys_usw_ipmc_update_member(lchip, &(p_group->ipmc_member[member_index]), &nh_mcast_group, ip_l2mc);
        if (ret < 0)
        {
            break;
        }
    }

    /*if error, rollback*/
    IPMC_UNLOCK;
    if (ret < 0)
    {
        p_group->member_number = member_index;
        sys_usw_ipmc_remove_member(lchip, p_group);
        return ret;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_remove_member(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    uint8 ip_l2mc = 0;
    uint8 member_index = 0;
    int32 ret = 0;
    sys_ipmc_group_param_t ipmc_param;
    sys_nh_param_mcast_group_t nh_mcast_group;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    CTC_PTR_VALID_CHECK(p_group_info);
    CTC_MAX_VALUE_CHECK(p_group_info->member_number, CTC_IPMC_MAX_MEMBER_PER_GROUP);

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group_info));

    IPMC_LOCK;
    ipmc_param.group = p_group_info;
    ret = _sys_usw_ipmc_asic_lookup(lchip, &ipmc_param);
    if(CTC_E_NONE != ret )
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }

    if (ipmc_param.is_drop || ipmc_param.is_rd_cpu)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    ip_l2mc = CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_L2MC); // ??shoud used sw db info??
    ret = sys_usw_nh_get_mcast_nh(lchip, ipmc_param.hw_grp_id, &ipmc_param.nh_id);
    if (ret != CTC_E_NONE)
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }

    nh_mcast_group.nhid = ipmc_param.nh_id;
    nh_mcast_group.dsfwd_valid = 0;
    nh_mcast_group.opcode = SYS_NH_PARAM_MCAST_DEL_MEMBER;

    for (member_index = 0; member_index < p_group_info->member_number; member_index++)
    {
        if ((p_group_info->ipmc_member[member_index].l3_if_type >= MAX_L3IF_TYPE_NUM)
            && !(p_group_info->ipmc_member[member_index].is_nh)
            && !(p_group_info->ipmc_member[member_index].re_route)
            && !(ip_l2mc))
        {
            IPMC_UNLOCK;
            return CTC_E_INVALID_PARAM;
        }

        ret = p_group_info->ipmc_member[member_index].port_bmp_en ?
            _sys_usw_ipmc_update_member_with_bitmap(lchip, &(p_group_info->ipmc_member[member_index]), &nh_mcast_group, ip_l2mc):
            _sys_usw_ipmc_update_member(lchip, &(p_group_info->ipmc_member[member_index]), &nh_mcast_group, ip_l2mc);
    }

    IPMC_UNLOCK;

    return ret;
}

int32
sys_usw_ipmc_add_default_entry(uint8 lchip, uint8 ip_version, ctc_ipmc_default_action_t type)
{
    uint16 max_vrfid_num = 0;
    uint16  loop = 0;
    uint32 default_base = 0;
    sys_ipmc_group_param_t ipmc_param;
    ctc_ipmc_group_info_t* p_group = NULL;
    int32 ret = CTC_E_NONE;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));

    SYS_IPMC_INIT_CHECK();
    CTC_IP_VER_CHECK(ip_version);

    p_group = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_ipmc_group_info_t));
    if(!p_group)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_group, 0, sizeof(ctc_ipmc_group_info_t));

    sal_memset(p_group, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_param.group = p_group;
    if (CTC_IPMC_DEFAULT_ACTION_DROP == type)
    {
        ipmc_param.group->flag = CTC_IPMC_FLAG_DROP;
    }
    else if (CTC_IPMC_DEFAULT_ACTION_TO_CPU == type)
    {
        ipmc_param.group->flag = CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }
    else if (CTC_IPMC_DEFAULT_ACTION_FALLBACK_BRIDGE == type)
    {
        ipmc_param.group->flag = CTC_IPMC_FLAG_RPF_CHECK | CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }
    IPMC_LOCK;
    max_vrfid_num = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) + 1;
    max_vrfid_num = sys_usw_l3if_get_default_entry_mode(lchip) ? 1 : max_vrfid_num;
    default_base = (ip_version == CTC_IP_VER_6) ? p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6] : p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4];
    IPMC_UNLOCK;
    for (loop = 0; loop < max_vrfid_num; loop++)
    {
        /* write ipda entry */
        ipmc_param.ad_index = default_base + loop;
        CTC_ERROR_GOTO(_sys_usw_ipmc_write_ipda(lchip, &ipmc_param), ret, ERROR_FREE_MEM);
    }

ERROR_FREE_MEM:
    mem_free(p_group);
    return ret;
}

STATIC int32
_sys_usw_ipmc_set_rp_intf(uint8 lchip, uint8 action, ctc_ipmc_rp_t * p_rp)
{
    uint8 i, j = 0;
    uint8 offset = 0;

    uint32 bidipim_group_index = 0;
    uint32 cmd = 0;
    DsBidiPimGroup_m ds_bidipim_group;
    uint32 bidipim_group[2] = {0};
    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_rp);
    SYS_IPMC_BIDIPIM_CHECK(p_rp->rp_id);
    CTC_MAX_VALUE_CHECK(p_rp->rp_intf_count, SYS_USW_IPMC_RP_INTF);
    sal_memset(&ds_bidipim_group, 0, sizeof(ds_bidipim_group));

    for (i = 0; i < p_rp->rp_intf_count; i++)
    {
        CTC_MIN_VALUE_CHECK(p_rp->rp_intf[i], 1);
        switch (p_usw_ipmc_master[lchip]->bidipim_mode)
        {
        case SYS_IPMC_BIDIPIM_MODE0:
             CTC_MAX_VALUE_CHECK(p_rp->rp_intf[i], MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM) - 1);
            break;

        case SYS_IPMC_BIDIPIM_MODE1:
            CTC_MAX_VALUE_CHECK(p_rp->rp_intf[i], (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)+1)/2 - 1);
            break;

        case SYS_IPMC_BIDIPIM_MODE2:
            CTC_MAX_VALUE_CHECK(p_rp->rp_intf[i], (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)+1)/4 - 1);
            break;

        case SYS_IPMC_BIDIPIM_MODE3:
            CTC_MAX_VALUE_CHECK(p_rp->rp_intf[i], (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)+1)/8 - 1);
            break;

        default:
            return CTC_E_INVALID_PARAM;
            break;
        }
    }

    for (i = 0; i < p_rp->rp_intf_count; i++)
    {
        switch (p_usw_ipmc_master[lchip]->bidipim_mode)
        {
        case SYS_IPMC_BIDIPIM_MODE0:
            bidipim_group_index = ((p_rp->rp_intf[i] >> 2) & 0xFF);
            offset = ((p_rp->rp_intf[i] & 0x3) << 4) | (p_rp->rp_id & 0xF);
            break;

        case SYS_IPMC_BIDIPIM_MODE1:
            bidipim_group_index = (p_rp->rp_intf[i] >> 1) & 0xFF;
            offset = ((p_rp->rp_intf[i] & 0x1) << 5) | (p_rp->rp_id & 0x1F);
            break;

        case SYS_IPMC_BIDIPIM_MODE2:
            bidipim_group_index = p_rp->rp_intf[i] & 0xFF;
            offset = p_rp->rp_id & 0x3F;
            break;

        case SYS_IPMC_BIDIPIM_MODE3:
            bidipim_group_index = ((p_rp->rp_intf[i] << 1) & 0xFF) | ((p_rp->rp_id & 0x40) >> 6);
            offset = p_rp->rp_id & 0x3F;
            break;

        default:
            return CTC_E_INVALID_PARAM;
            break;
        }

        cmd = DRV_IOR(DsBidiPimGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, bidipim_group_index, cmd, &ds_bidipim_group));
        sal_memcpy(&bidipim_group, &ds_bidipim_group, sizeof(bidipim_group));
        j = (offset >= 32) ? 1 : 0;
        offset = (offset >= 32) ? (offset - 32) : offset;
        bidipim_group[j] = (action == TRUE) ? (bidipim_group[j] | (1 << offset)) : (bidipim_group[j] & (~(uint32)(1 << offset)));
        SetDsBidiPimGroup(A, bidiPimBlock_f, &ds_bidipim_group, bidipim_group);
        cmd = DRV_IOW(DsBidiPimGroup_t, DRV_ENTRY_FLAG);

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, bidipim_group_index, cmd, &ds_bidipim_group));
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_show_rp_intf(uint8 lchip, uint8 rp_id)
{
    uint32 idx = 0;
    uint32 idx1 = 0;
    uint8 offset = 0;
    uint32 cmd = 0;
    uint16 l3if_ct = 0;
    uint16 *l3if = NULL;
    DsBidiPimGroup_m ds_bidipim_group;
    uint32 bidipim_group[2] = {0};
    int32 ret = CTC_E_NONE;

    SYS_IPMC_INIT_CHECK();
    SYS_IPMC_BIDIPIM_CHECK(rp_id);
    sal_memset(&ds_bidipim_group, 0, sizeof(ds_bidipim_group));

    l3if = mem_malloc(MEM_IPMC_MODULE, sizeof(uint16)*(MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM) + 1));

    if (l3if == NULL)
    {
        return 0;
    }

    sal_memset(l3if, 0, sizeof(*l3if));
    IPMC_LOCK;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Bidipim mode: %d\n", p_usw_ipmc_master[lchip]->bidipim_mode) ;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Rp_id %d block l3if list\n", rp_id) ;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------------\n") ;


    for (idx = 0; idx < (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)+1)/4; idx++)
    {
        cmd = DRV_IOR(DsBidiPimGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, idx, cmd, &ds_bidipim_group), ret, out);
        sal_memcpy(&bidipim_group, &ds_bidipim_group, sizeof(bidipim_group));

        switch (p_usw_ipmc_master[lchip]->bidipim_mode)
        {
        case SYS_IPMC_BIDIPIM_MODE0:
            for (idx1 = 0; idx1 < 4; idx1++)
            {
                offset = (idx1 << 4) | (rp_id & 0xF);
                if (CTC_BMP_ISSET(bidipim_group, offset))
                {
                    l3if[l3if_ct++] = (idx << 2) |idx1;
                }
            }
            break;

        case SYS_IPMC_BIDIPIM_MODE1:
            for (idx1 = 0; idx1 < 2; idx1++)
            {
                offset = (idx1 << 5) | (rp_id & 0x1F);
                if (CTC_BMP_ISSET(bidipim_group, offset))
                {
                    l3if[l3if_ct++] = (idx << 1) |idx1;
                }
            }
            break;

        case SYS_IPMC_BIDIPIM_MODE2:
            offset = rp_id & 0x3F;
            if (CTC_BMP_ISSET(bidipim_group, offset))
            {
                l3if[l3if_ct++] = idx;
            }
            break;

        case SYS_IPMC_BIDIPIM_MODE3:
            if (((rp_id > 0x3F) && (idx % 2 == 0)) || ((rp_id <= 0x3F) && (idx % 2 == 1)))
            {
                continue;
            }

            offset = rp_id & 0x3F;
            if (CTC_BMP_ISSET(bidipim_group, offset))
            {
                l3if[l3if_ct++] = (idx >> 1);
            }
            break;

        default:
            ret = CTC_E_INVALID_PARAM;
            goto out;
            break;
        }
    }

    for (idx = 0; idx < l3if_ct; idx++)
    {
        if (idx % 16)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, ", %3d", l3if[idx]) ;
        }
        else
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    %3d", l3if[idx]) ;
        }

        if ((idx % 16 == 15) || (idx == l3if_ct -1))
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n") ;
        }
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------------\n") ;
out:
    mem_free(l3if);
    IPMC_UNLOCK;
    return ret;
}

int32
sys_usw_ipmc_add_rp_intf(uint8 lchip, ctc_ipmc_rp_t * p_rp)
{
    int32 ret = 0;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();
    IPMC_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_ipmc_set_rp_intf(lchip, TRUE, p_rp), p_usw_ipmc_master[lchip]->p_ipmc_mutex);
    IPMC_UNLOCK;
    return ret;
}


int32
sys_usw_ipmc_remove_rp_intf(uint8 lchip, ctc_ipmc_rp_t * p_rp)
{
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    return _sys_usw_ipmc_set_rp_intf(lchip, FALSE, p_rp);
}

int32
sys_usw_ipmc_update_rpf(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    int32 ret = CTC_E_NONE;
    uint8 loop;
    uint32 cmd =0;
    DsIpDa_m ds_ip_da;
    sys_ipmc_group_param_t ipmc_param;
    sys_rpf_info_t rpf_info;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    sal_memset(&rpf_info, 0, sizeof(sys_rpf_info_t));

    if (CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_L2MC))
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPMC] Rpf check disable \n");
        return CTC_E_NOT_READY;
    }
    if ((CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_RPF_CHECK) || CTC_FLAG_ISSET(p_group_info->flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU))&& sys_usw_chip_get_rchain_en())
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    CTC_SET_FLAG(p_group_info->flag,CTC_IPMC_FLAG_RPF_CHECK);

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group_info));

    IPMC_LOCK;
    ipmc_param.group = p_group_info;
    ret = _sys_usw_ipmc_asic_lookup(lchip, &ipmc_param);
    if(CTC_E_NONE != ret )
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }

    if(!ipmc_param.rpf_chk_en)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_READY);
    }

    for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM; loop++)
    {
        if (p_group_info->rpf_intf_valid[loop])
        {
            rpf_info.rpf_intf[rpf_info.rpf_intf_cnt] = p_group_info->rpf_intf[loop];
            rpf_info.rpf_intf_cnt++;
        }
    }

    rpf_info.is_ipmc =1;
    rpf_info.mode =  ipmc_param.rpf_chk_en ? ipmc_param.rpf_mode : SYS_RPF_CHK_MODE_IFID;
    rpf_info.rpf_id = ipmc_param.rpf_id;
    ret = sys_usw_rpf_update_profile(lchip, &rpf_info);
    if(ret < 0)
    {
        CTC_RETURN_IPMC_UNLOCK(ret);
    }

    cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(DRV_IOCTL(lchip, ipmc_param.ad_index, cmd, &ds_ip_da));
    SetDsIpDa(V, rpfCheckMode_f, &ds_ip_da, rpf_info.mode);
    SetDsIpDa(V, rpfIfId_f, &ds_ip_da, rpf_info.rpf_id);
    SetDsIpDa(V, rpfCheckEn_f, &ds_ip_da, 1);
    cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(DRV_IOCTL(lchip, ipmc_param.ad_index, cmd, &ds_ip_da));

    IPMC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_set_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8 hit)
{
    sys_ipmc_group_param_t ipmc_param;
    uint8 timer = 0;
    uint8 domain_type = 0;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group));
    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));

    IPMC_LOCK;
    ipmc_param.group = p_group;
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param));
    if (!ipmc_param.is_group_exist)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_EXIST);
    }
    if (ipmc_param.is_default_route)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_SUPPORT);
    }
    hit = hit ? 1:0;
    domain_type = ipmc_param.group->src_ip_mask_len?SYS_AGING_DOMAIN_HOST1:SYS_AGING_DOMAIN_IP_HASH;
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_aging_get_aging_timer(lchip, domain_type, ipmc_param.key_index, &timer));
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_aging_set_aging_status(lchip, domain_type, ipmc_param.key_index, timer, hit));
    IPMC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_get_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8* hit)
{
    sys_ipmc_group_param_t ipmc_param;
    uint8 domain_type = 0;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(hit);

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group));
    IPMC_LOCK;
    ipmc_param.group = p_group;
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param));
    if (!ipmc_param.is_group_exist)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_EXIST);
    }
    if (ipmc_param.is_default_route)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_SUPPORT);
    }

    domain_type = ipmc_param.group->src_ip_mask_len?SYS_AGING_DOMAIN_HOST1:SYS_AGING_DOMAIN_IP_HASH;
    CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(sys_usw_aging_get_aging_status(lchip, domain_type, ipmc_param.key_index, hit));
    IPMC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_get_group_info(uint8 lchip, ctc_ipmc_group_info_t* p_group_info)
{
    uint8 loop = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd;
    uint32 l3if;
    DsRpf_m ds_rpf;
    DsIpDa_m ds_da;
    sys_ipmc_group_param_t ipmc_param;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    ipmc_param.group = p_group_info;
    IPMC_LOCK;
    ret = _sys_usw_ipmc_asic_lookup(lchip, &ipmc_param);
    if(!ipmc_param.is_group_exist)
    {
        CTC_RETURN_IPMC_UNLOCK(CTC_E_NOT_EXIST);
    }

    p_group_info->group_id = ipmc_param.hw_grp_id;
    p_group_info->rp_id = ipmc_param.rpf_id;

    sal_memset(p_group_info->rpf_intf_valid, 0, sizeof(uint8) * SYS_USW_MAX_RPF_IF_NUM);

    if (ipmc_param.rpf_chk_en && ipmc_param.rpf_mode == SYS_RPF_CHK_MODE_PROFILE)
    {
        cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(DRV_IOCTL(lchip, ipmc_param.rpf_id, cmd, &ds_rpf));

        for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM; loop++)
        {
            p_group_info->rpf_intf_valid[loop] = GetDsRpf(V, array_0_rpfIfIdValid_f+2*loop, &ds_rpf);
            if (1 == p_group_info->rpf_intf_valid[loop])
            {
                p_group_info->rpf_intf[loop] = GetDsRpf(V, array_0_rpfIfId_f+2*loop, &ds_rpf);
            }
        }
    }
    else if (ipmc_param.rpf_chk_en && ipmc_param.rpf_mode == SYS_RPF_CHK_MODE_IFID)
    {
        sal_memset(&ds_da, 0, sizeof(DsIpDa_m));
        cmd = DRV_IOR(DsIpDa_t, DsIpDa_rpfIfId_f);
        CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(DRV_IOCTL(lchip, ipmc_param.ad_index, cmd, &l3if));

        p_group_info->rpf_intf[0] = l3if;
        p_group_info->rpf_intf_valid[0] = 1;
    }

    IPMC_UNLOCK;
    return ret;
}

int32
sys_usw_ipmc_set_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data)
{
    uint32 cmd;
    uint32 field_val;
    uint32 ipv4_mask = 0;
    ipv6_addr_t ipv6_addr;
    ipv6_addr_t ipv6_mask;
    IpeIpv4McastForceRoute_m ipe_ipv4_mcast_force_route;
    IpeIpv6McastForceRoute_m ipe_ipv6_mcast_force_route;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_data);
    CTC_IP_VER_CHECK(p_data->ip_version);

    sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));
    sal_memset(&ipe_ipv4_mcast_force_route, 0, sizeof(ipe_ipv4_mcast_force_route));
    sal_memset(&ipe_ipv6_mcast_force_route, 0, sizeof(ipe_ipv6_mcast_force_route));


    if (CTC_IP_VER_4 == p_data->ip_version)
    {
        cmd = DRV_IOR(IpeIpv4McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv4_mcast_force_route));

        if (p_data->ipaddr0_valid)
        {
            if (CTC_IPV4_ADDR_LEN_IN_BIT < p_data->addr0_mask)
            {
                return CTC_E_INVALID_PARAM;
            }
            IPV4_LEN_TO_MASK(ipv4_mask, p_data->addr0_mask);
            SetIpeIpv4McastForceRoute(V, array_0_addrV4Value_f, &ipe_ipv4_mcast_force_route, p_data->ip_addr0.ipv4);
            SetIpeIpv4McastForceRoute(V, array_0_addrV4Mask_f, &ipe_ipv4_mcast_force_route, ipv4_mask);
            SetIpeIpv4McastForceRoute(V, array_0_camV4Valid_f, &ipe_ipv4_mcast_force_route, 1);
        }

        if (p_data->ipaddr1_valid)
        {
            if (CTC_IPV4_ADDR_LEN_IN_BIT < p_data->addr1_mask)
            {
                return CTC_E_INVALID_PARAM;
            }
            IPV4_LEN_TO_MASK(ipv4_mask, p_data->addr1_mask);
            SetIpeIpv4McastForceRoute(V, array_1_addrV4Value_f, &ipe_ipv4_mcast_force_route, p_data->ip_addr1.ipv4);
            SetIpeIpv4McastForceRoute(V, array_1_addrV4Mask_f, &ipe_ipv4_mcast_force_route, ipv4_mask);
            SetIpeIpv4McastForceRoute(V, array_1_camV4Valid_f, &ipe_ipv4_mcast_force_route, 1);

        }

        cmd = DRV_IOW(IpeIpv4McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv4_mcast_force_route));

        field_val = p_data->force_bridge_en ? 1 : 0;
        cmd = DRV_IOW(IpePreLookupCtl_t, IpePreLookupCtl_ipv4McastForceBridgeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        field_val = p_data->force_ucast_en ? 1 : 0;
        cmd = DRV_IOW(IpePreLookupCtl_t, IpePreLookupCtl_ipv4McastForceUnicastEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    else
    {
        cmd = DRV_IOR(IpeIpv6McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv6_mcast_force_route));

        if (p_data->ipaddr0_valid)
        {
            SYS_IPMC_IPV6_ADDRESS_SORT(p_data->ip_addr0.ipv6);
            sal_memcpy(&ipv6_addr, &(p_data->ip_addr0.ipv6), sizeof(ipv6_addr_t));
            SetIpeIpv6McastForceRoute(A, array_0_addrV6Value_f, &ipe_ipv6_mcast_force_route, ipv6_addr);
            if (CTC_IPV6_ADDR_LEN_IN_BIT < p_data->addr0_mask)
            {
                return CTC_E_INVALID_PARAM;
            }
            IPV6_LEN_TO_MASK(ipv6_mask, p_data->addr0_mask);
            SetIpeIpv6McastForceRoute(A, array_0_addrV6Mask_f, &ipe_ipv6_mcast_force_route, ipv6_mask);
            SetIpeIpv6McastForceRoute(V, array_0_camV6Valid_f, &ipe_ipv6_mcast_force_route, 1);
        }

        sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));
        if (p_data->ipaddr1_valid)
        {
            SYS_IPMC_IPV6_ADDRESS_SORT(p_data->ip_addr1.ipv6);
            sal_memcpy(&ipv6_addr, &(p_data->ip_addr1.ipv6), sizeof(ipv6_addr_t));
            SetIpeIpv6McastForceRoute(A, array_1_addrV6Value_f, &ipe_ipv6_mcast_force_route, ipv6_addr);
            if (CTC_IPV6_ADDR_LEN_IN_BIT < p_data->addr1_mask)
            {
                return CTC_E_INVALID_PARAM;
            }
            IPV6_LEN_TO_MASK(ipv6_mask, p_data->addr1_mask);
            SetIpeIpv6McastForceRoute(A, array_1_addrV6Mask_f, &ipe_ipv6_mcast_force_route, ipv6_mask);
            SetIpeIpv6McastForceRoute(V, array_1_camV6Valid_f, &ipe_ipv6_mcast_force_route, 1);
        }

        cmd = DRV_IOW(IpeIpv6McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv6_mcast_force_route));

        field_val = p_data->force_bridge_en ? 1 : 0;
        cmd = DRV_IOW(IpePreLookupCtl_t, IpePreLookupCtl_ipv6McastForceBridgeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        field_val = p_data->force_ucast_en ? 1 : 0;
        cmd = DRV_IOW(IpePreLookupCtl_t, IpePreLookupCtl_ipv6McastForceUnicastEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_get_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data)
{
    uint32 cmd;
    uint32 field_val;
    uint32 ipv4_mask = 0;
    ipv6_addr_t ipv6_addr;
    ipv6_addr_t ipv6_mask;
    IpeIpv4McastForceRoute_m ipe_ipv4_mcast_force_route;
    IpeIpv6McastForceRoute_m ipe_ipv6_mcast_force_route;

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_data);
    CTC_IP_VER_CHECK(p_data->ip_version);

    sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));
    sal_memset(&ipe_ipv4_mcast_force_route, 0, sizeof(ipe_ipv4_mcast_force_route));
    sal_memset(&ipe_ipv6_mcast_force_route, 0, sizeof(ipe_ipv6_mcast_force_route));

    if (CTC_IP_VER_4 == p_data->ip_version)
    {
        cmd = DRV_IOR(IpeIpv4McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv4_mcast_force_route));
        p_data->ip_addr0.ipv4 = GetIpeIpv4McastForceRoute(V, array_0_addrV4Value_f, &ipe_ipv4_mcast_force_route);
        ipv4_mask             = GetIpeIpv4McastForceRoute(V, array_0_addrV4Mask_f, &ipe_ipv4_mcast_force_route);
        IPV4_MASK_TO_LEN(ipv4_mask, p_data->addr0_mask);
        p_data->ipaddr0_valid = GetIpeIpv4McastForceRoute(V, array_0_camV4Valid_f, &ipe_ipv4_mcast_force_route);

        ipv4_mask = 0;
        p_data->ip_addr1.ipv4 = GetIpeIpv4McastForceRoute(V, array_1_addrV4Value_f, &ipe_ipv4_mcast_force_route);
        ipv4_mask             = GetIpeIpv4McastForceRoute(V, array_1_addrV4Mask_f, &ipe_ipv4_mcast_force_route);
        IPV4_MASK_TO_LEN(ipv4_mask, p_data->addr1_mask);
        p_data->ipaddr1_valid= GetIpeIpv4McastForceRoute(V, array_1_camV4Valid_f, &ipe_ipv4_mcast_force_route);

        cmd = DRV_IOR(IpePreLookupCtl_t, IpePreLookupCtl_ipv4McastForceBridgeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        p_data->force_bridge_en = field_val ? TRUE : FALSE;

        cmd = DRV_IOR(IpePreLookupCtl_t, IpePreLookupCtl_ipv4McastForceUnicastEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        p_data->force_ucast_en = field_val ? TRUE : FALSE;

    }
    else
    {
        cmd = DRV_IOR(IpeIpv6McastForceRoute_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipv6_mcast_force_route));
        GetIpeIpv6McastForceRoute(A, array_0_addrV6Value_f, &ipe_ipv6_mcast_force_route, ipv6_addr);
        SYS_IPMC_IPV6_ADDRESS_SORT(ipv6_addr);
        sal_memcpy(&(p_data->ip_addr0.ipv6),&ipv6_addr, sizeof(ipv6_addr_t));
        GetIpeIpv6McastForceRoute(A, array_0_addrV6Mask_f, &ipe_ipv6_mcast_force_route, ipv6_mask);
        IPV6_MASK_TO_LEN(ipv6_mask, p_data->addr0_mask);
        p_data->ipaddr0_valid = GetIpeIpv6McastForceRoute(V, array_0_camV6Valid_f, &ipe_ipv6_mcast_force_route);

        GetIpeIpv6McastForceRoute(A, array_1_addrV6Value_f, &ipe_ipv6_mcast_force_route, ipv6_addr);
        SYS_IPMC_IPV6_ADDRESS_SORT(ipv6_addr);
        sal_memcpy(&(p_data->ip_addr1.ipv6),&ipv6_addr, sizeof(ipv6_addr_t));
        sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));
        GetIpeIpv6McastForceRoute(A, array_1_addrV6Mask_f, &ipe_ipv6_mcast_force_route, ipv6_mask);
        IPV6_MASK_TO_LEN(ipv6_mask, p_data->addr1_mask);
        p_data->ipaddr1_valid = GetIpeIpv6McastForceRoute(V, array_1_camV6Valid_f, &ipe_ipv6_mcast_force_route);

        cmd = DRV_IOR(IpePreLookupCtl_t, IpePreLookupCtl_ipv6McastForceBridgeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        p_data->force_bridge_en = field_val ? TRUE : FALSE;

        cmd = DRV_IOR(IpePreLookupCtl_t, IpePreLookupCtl_ipv6McastForceUnicastEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        p_data->force_ucast_en = field_val ? 1 : 0;
    }
    return CTC_E_NONE;
}


STATIC uint32
_sys_usw_ipmc_g_hash_make(sys_ipmc_g_node_t* p_node)
{
    uint8  hash_seeds[20] = {0};
    uint8  hash_seeds_size = 0;

    hash_seeds[0] = p_node->ipmc_type;
    hash_seeds[1] = p_node->vrfid & 0xFF;
    hash_seeds[2] = (p_node->vrfid >> 8) & 0xFF;
    hash_seeds[3] = 0;
    hash_seeds_size = 4;

    if (p_node->ipmc_type == SYS_IPMC_V4 || p_node->ipmc_type == SYS_IPMC_V4_L2MC )
    {
        /*ip_addr(32bit) + vrfid(16bit) + ipmc_type(1bit) */
        sal_memcpy((uint8*)&hash_seeds[4], (uint8*)&p_node->ip_addr.ipv4, sizeof(uint32));
        hash_seeds_size += 4;
    }
    else
    {
        sal_memcpy((uint8*)&hash_seeds[4], (uint8*)&p_node->ip_addr.ipv6[0], 4*sizeof(uint32));
        hash_seeds_size += 16;
    }

    return ctc_hash_caculate(hash_seeds_size,hash_seeds);
}


int32
_sys_usw_ipmc_g_hash_compare(sys_ipmc_g_node_t* p_bucket, sys_ipmc_g_node_t* p_node)
{
    if (p_bucket->ipmc_type != p_node->ipmc_type)
    {
        return FALSE;
    }
    if (p_bucket->vrfid != p_node->vrfid)
    {
        return FALSE;
    }
    if (p_bucket->ipmc_type == SYS_IPMC_V4 || p_bucket->ipmc_type == SYS_IPMC_V4_L2MC )
    {
        if (p_bucket->ip_addr.ipv4  != p_node->ip_addr.ipv4 )
        {
            return FALSE;
        }
    }
    else
    {
        if (sal_memcmp(&p_bucket->ip_addr.ipv6[0],&p_node->ip_addr.ipv6[0], sizeof(ipv6_addr_t)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

int32
sys_usw_ipmc_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);

    specs_info->used_size = p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4]
                   + p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4_L2MC]
                   + (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6]*2)
                   + (p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6_L2MC]*2);

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_show_status(uint8 lchip)
{
    int32 default_num = 0;
    uint32  group_cnt = 0;

    LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    /*ipv4*/
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n===========================\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:\n", "IPV4 size");
    default_num = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) + 1;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPMC:\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(S,G) entry count", p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4]);
    group_cnt = p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4]-p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4];
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,G) entry count",group_cnt );

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,*) entry count", default_num);
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2MC:\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(S,G) entry count", p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4_L2MC]);
    group_cnt = p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4_L2MC]-p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4_L2MC];
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,G) entry count", group_cnt);

    /*ipv6*/
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n===========================\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:\n", "IPV6 size");

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPMC:\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(S,G) entry count", p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6]);
    group_cnt = p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6]-p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6];

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,G) entry count", group_cnt);

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,*) entry count", default_num);
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2MC:\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(S,G) entry count", p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6_L2MC]);
    group_cnt = p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6_L2MC]-p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6_L2MC];
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s:%4u \n", "(*,G) entry count", group_cnt);

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n===========================\n");

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_db_show_count(uint8 lchip)
{
    uint32 loop =0;
    uint16 count = 0;
    SYS_IPMC_INIT_CHECK();
    IPMC_LOCK;
    for (loop = 0; loop < MAX_CTC_IP_VER; loop++)
    {
        count = loop ? p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6_L2MC] :p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4_L2MC];
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2MCv%c group total %d routes\n", (loop == CTC_IP_VER_4) ? '4' : '6', count);
        count = loop ? p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6] :p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4];
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPMCv%c group total %d routes\n", (loop == CTC_IP_VER_4) ? '4' : '6', count);
    }
    IPMC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_db_show_member_info(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    sys_ipmc_group_param_t ipmc_param;
    uint32 nh_id;

    LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_group);

    sal_memset(&ipmc_param, 0, sizeof(ipmc_param));
    ipmc_param.group = p_group;

    CTC_ERROR_RETURN(_sys_usw_ipmc_check(lchip, p_group));

    CTC_ERROR_RETURN(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param));

    CTC_ERROR_RETURN(sys_usw_nh_get_mcast_nh(lchip, ipmc_param.hw_grp_id, &nh_id));
    sys_usw_nh_dump(lchip, nh_id, TRUE);

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_dump_group_info(uint8 lchip, sys_ipmc_group_node_t *p_group_node, uint16 group_id)
{
    char buf[CTC_IPV6_ADDR_STR_LEN];
    char buf2[5];
    uint32 tempip = 0;
    uint8 value = 0;
    uint8 sg_mode = 0;
    uint32 nh_id = 0;
    ctc_ipmc_addr_info_t address;
    char str[50] = {0};

    if (p_group_node->is_drop || p_group_node->is_rd_cpu)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s", " - ");
    }
    else
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", group_id);
    }

    if (!p_group_node->ip_version)
    {
        /* print IPv4 source IP address */
        value = p_group_node->src_ip_mask_len? 32 : 0;
        sal_sprintf(buf2, "/%-1u", value);
        tempip = sal_ntohl(p_group_node->address.ipv4.src_addr);
        sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, 5);

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-21s", buf);

        /* print IPv4 group IP address */
        value = 32;
        sal_sprintf(buf2, "/%-1u", value);
        tempip = sal_ntohl(p_group_node->address.ipv4.group_addr);
        sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, 5);

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-21s", buf);
        /* print vrfId */
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", p_group_node->address.ipv4.vrfid);
    }
    else
    {
        uint32 ipv6_address[4] = {0};
        sal_memcpy(&address,&p_group_node->address,sizeof(ctc_ipmc_addr_info_t));
        SYS_IPMC_GROUP_ADDRESS_SORT(address);

        /* print IPv6 group IP address */
        value = 128;
        sal_sprintf(buf2, "/%-1u", value);

        ipv6_address[0] = sal_htonl(address.ipv6.group_addr[3]);
        ipv6_address[1] = sal_htonl(address.ipv6.group_addr[2]);
        ipv6_address[2] = sal_htonl(address.ipv6.group_addr[1]);
        ipv6_address[3] = sal_htonl(address.ipv6.group_addr[0]);

        sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, 5);
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-46s", buf);

        /* print IPv6 source IP address */
        value = p_group_node->src_ip_mask_len ? 128 : 0;
        sal_sprintf(buf2, "/%-1u", value);

        ipv6_address[0] = sal_htonl(address.ipv6.src_addr[3]);
        ipv6_address[1] = sal_htonl(address.ipv6.src_addr[2]);
        ipv6_address[2] = sal_htonl(address.ipv6.src_addr[1]);
        ipv6_address[3] = sal_htonl(address.ipv6.src_addr[0]);

        sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, 5);

        /* print vrfId */
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", p_group_node->address.ipv6.vrfid);
    }

    if(p_group_node->src_ip_mask_len)
    {
        sg_mode = 1;
    }

    if (p_group_node->is_drop)
    {
        nh_id = CTC_NH_RESERVED_NHID_FOR_DROP;
    }
    else if (p_group_node->is_rd_cpu)
    {
        nh_id = CTC_NH_RESERVED_NHID_FOR_TOCPU;
    }
    else
    {
        sys_usw_nh_get_mcast_nh(lchip, group_id, &nh_id);
    }

    if (nh_id >= SYS_NH_INTERNAL_NHID_BASE)
    {
        sal_sprintf(str, "%u%s", (nh_id-SYS_NH_INTERNAL_NHID_BASE), "(I)");
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", str);
    }
    else
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", nh_id);
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", (sg_mode ? "(S,G)" : "(*,G)"));
    if (p_group_node->ip_version)
    {
        if(sg_mode)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s", " ");
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-46s\n", buf);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_traverse_dump_group_info(ctc_list_pointer_t *p_linklist, uint32 vec_index, void *user_data)
{
    uint8 lchip = (uint8)(((sys_traverse_t *)user_data)->value1);
    uint8 ip_ver = (uint8)(((sys_traverse_t *)user_data)->value2);
    ctc_list_pointer_node_t* p_node = NULL;
    sys_ipmc_group_node_t *p_ipmc_group_node = NULL;

    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_ipmc_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        if (p_ipmc_group_node->ip_version == ip_ver)
        {
            sys_usw_ipmc_dump_group_info(lchip, p_ipmc_group_node, vec_index);
        }
    }

    return CTC_E_NONE;
}

char *
_sys_usw_ipmc_db_show_tbl_str(uint32 key_tbl_id)
{
    switch(key_tbl_id)
      {
      case DsFibHost0Ipv4HashKey_t:
          return "DsFibHost0Ipv4HashKey";
          break;
      case DsFibHost1Ipv4McastHashKey_t:
          return "DsFibHost1Ipv4McastHashKey";
          break;
      case DsFibHost1Ipv6McastHashKey_t:
          return "DsFibHost1Ipv6McastHashKey";
          break;
      case DsFibHost0MacIpv6McastHashKey_t:
          return "DsFibHost0MacIpv6McastHashKey";
          break;
      case DsFibHost0Ipv6McastHashKey_t:
          return "DsFibHost0Ipv6McastHashKey";
          break;
	  default:
          return "Error Table";
          break;
      }
}

int32
_sys_usw_ipmc_dump_rp_info(uint8 lchip, uint8 rp_id)
{
    uint16 rp_l3if_num  = 0;
    uint16 loop , loop2, index ;
    uint16 bidipim_group_index = 0;
    uint8 offset = 0;
    DsBidiPimGroup_m ds_bidipim_group;
    uint32 bidipim_group_block[2] =  {0};
    uint32 cmd = 0;
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RP Block L3if List\n");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-8s    %-8s\n", "Index", "L3ifId");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------\n");

    rp_l3if_num = 128;
    loop2 = 0;
    for (loop = SYS_USW_MIN_CTC_L3IF_ID; loop <= rp_l3if_num; loop++ )
    {
        switch (p_usw_ipmc_master[lchip]->bidipim_mode)
        {
            case SYS_IPMC_BIDIPIM_MODE0:
                bidipim_group_index = (loop >> 2);
                offset = ((loop & 0x3) << 4) | (rp_id & 0xF);
                rp_l3if_num = 1022;
                break;

            case SYS_IPMC_BIDIPIM_MODE1:
                bidipim_group_index = (loop >> 1) & 0xFF;
                offset = ((loop & 0x1) << 5) | (rp_id & 0x1F);
                rp_l3if_num = 511;
                break;

            case SYS_IPMC_BIDIPIM_MODE2:
                bidipim_group_index = loop & 0xFF;
                offset = rp_id & 0x3F;
                rp_l3if_num = 255;
                break;

            case SYS_IPMC_BIDIPIM_MODE3:
                bidipim_group_index = ((loop << 1) & 0xFF) | ((rp_id & 0x40) >> 6);
                offset = rp_id & 0x3F;
                rp_l3if_num = 128;
                break;

            default:
                return CTC_E_INVALID_PARAM;
        }

        cmd = DRV_IOR(DsBidiPimGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, bidipim_group_index, cmd, &ds_bidipim_group));
        sal_memcpy(&bidipim_group_block, &ds_bidipim_group, sizeof(ds_bidipim_group));
        index = (offset >= 32) ? 1 : 0;
        offset = (offset >= 32) ? (offset - 32) : offset;
        if (CTC_IS_BIT_SET(bidipim_group_block[index], offset))
        {
            loop2++;
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," %-8d    %-8d\n", loop2, loop);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_db_show_group_info(uint8 lchip, uint8 ip_version, ctc_ipmc_group_info_t* p_group)
{
    uint32 rpf_chk_port;
    uint32 cmd = 0;
    sys_traverse_t user_data = {0};
    LCHIP_CHECK(lchip);
    SYS_IPMC_INIT_CHECK();

    cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rpf_chk_port));

    if (CTC_IP_VER_4 == ip_version)
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-21s%-21s%-6s%-7s%s\n", "GID", "IPSA",
                       "Group-IP", "VRF", "NHID",  "Mode");
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        "------------------------------------------------------------------------------\n");
    }
    else
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-46s%-6s%-7s%s\n", "GID",
                        "Group-IP/IPSA", "VRF", "NHID", "Mode");
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        "------------------------------------------------------------------------------\n");
    }
    IPMC_LOCK;
    if (!p_group)
    {
        user_data.value1 = lchip;
        user_data.value2 = ip_version;
        ctc_vector_traverse2(p_usw_ipmc_master[lchip]->group_vec, 0,(vector_traversal_fn2)sys_usw_ipmc_traverse_dump_group_info, (void*)&user_data);
    }
    else
    {
        sys_ipmc_group_param_t ipmc_param;
        sys_ipmc_group_node_t *p_group_node;
        CTC_PTR_VALID_CHECK(p_group);
        sal_memset(&ipmc_param, 0, sizeof(ipmc_param));
        ipmc_param.group = p_group;
        CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(_sys_usw_ipmc_asic_lookup(lchip, &ipmc_param));

        p_group_node = sys_usw_ipmc_get_group_from_db(lchip, p_group);
        if (p_group_node)
        {
            sys_usw_ipmc_dump_group_info(lchip, p_group_node, ipmc_param.hw_grp_id);
        }

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\nDetail table offset\n");
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------\n");
        if (p_group->src_ip_mask_len
            && p_group->group_ip_mask_len)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-30s      :%d (*,G)\n", _sys_usw_ipmc_db_show_tbl_str(ipmc_param.g_key_tbl_id), ipmc_param.g_key_index);
        }

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-30s      :%d %s\n", _sys_usw_ipmc_db_show_tbl_str(ipmc_param.tbl_id), ipmc_param.key_index, p_group->src_ip_mask_len?"(S,G)":"(*,G)");
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-30s      :%d  \n", (CTC_FLAG_ISSET(p_group->flag, CTC_IPMC_FLAG_L2MC) ? "DsMac":"DsIpDa"), ipmc_param.ad_index);
        if(ipmc_param.rpf_chk_en && !ipmc_param.rpf_mode)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," --%-30s      :%d  \n", "DsRpf", ipmc_param.rpf_id);
        }

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        if(ipmc_param.rp_chk_en)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RP Check Enable,RP ID :%d\n",ipmc_param.rpf_id);
            _sys_usw_ipmc_dump_rp_info(lchip,ipmc_param.rpf_id);
        }
        else if (ipmc_param.rpf_chk_en)
        {
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RPF Check Mode: %s,and use %s to do Check,",ipmc_param.rpf_mode ? "One Interface":"Profile",\
                                                rpf_chk_port ? "gport":"l3if ID");
            SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RPF ID :%d\n",ipmc_param.rpf_id);
        }
    }
    IPMC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ipmc_show_default_entry(uint8 lchip, uint8 ip_version, uint16 vrf_start, uint16 vrf_end)
{
    uint8 ipmc_type = 0;
    uint16 vrfid = 0;
    uint32 cmd = 0;
    uint32 ad_index = 0;
    uint32 value1 = 0;
    uint32 value2 = 0;
    uint32 group_flag = 0;
    uint32 tocpu_dsfwd;
    DsIpDa_m ds_ip_da;
    char buf[64] = {0};

    SYS_IPMC_INIT_CHECK();

    CTC_IP_VER_CHECK(ip_version);
    SYS_IP_VRFID_CHECK(vrf_start, ip_version);
    SYS_IP_VRFID_CHECK(vrf_end, ip_version);
    CTC_MAX_VALUE_CHECK(vrf_start, vrf_end);

    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-15s%-10s\n", "VRF", "AD_index", "Type");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                "-----------------------------------------------\n");

    CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, SYS_NH_RESOLVED_NHID_FOR_TOCPU, &tocpu_dsfwd, 0));

    ipmc_type = (ip_version == CTC_IP_VER_6) ? SYS_IPMC_V6 : SYS_IPMC_V4;
    vrf_start = sys_usw_l3if_get_default_entry_mode(lchip) ? 0 : vrf_start;
    vrf_end = sys_usw_l3if_get_default_entry_mode(lchip) ? vrf_start : vrf_end;

    for (vrfid = vrf_start; vrfid <= vrf_end; vrfid++)
    {
        group_flag = 0;
        sal_memset(buf, 0, 64);
        ad_index = p_usw_ipmc_master[lchip]->default_base[ipmc_type] + vrfid;

        cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &ds_ip_da));

        value1 = GetDsIpDa(V, isDefaultRoute_f, &ds_ip_da);
        if (!value1)
        {
            continue;
        }

        value1 = GetDsIpDa(V, rpfCheckEn_f, &ds_ip_da);
        if (value1)
        {
            CTC_SET_FLAG(group_flag, CTC_IPMC_FLAG_RPF_CHECK);
        }

        value1 = GetDsIpDa(V, mcastRpfFailCpuEn_f, &ds_ip_da);
        if (value1)
        {
            CTC_SET_FLAG(group_flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU);
        }

        value1 = GetDsIpDa(V, nextHopPtrValid_f, &ds_ip_da);
        value2 = GetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &ds_ip_da);
        if ((value1 == 0) && (value2 == 0xFFFF))
        {
            CTC_SET_FLAG(group_flag, CTC_IPMC_FLAG_DROP);
        }
        else if ((value1 == 0) && (value2 == tocpu_dsfwd))
        {
            CTC_SET_FLAG(group_flag, CTC_IPMC_FLAG_REDIRECT_TOCPU);
        }
        else
        {
            value1 = GetDsIpDa(V, exception3CtlEn_f, &ds_ip_da);
            value2 = GetDsIpDa(V, exceptionSubIndex_f, &ds_ip_da);
            if ((value1 == 1) && (value2 == 0x1F))
            {
                CTC_SET_FLAG(group_flag, CTC_IPMC_FLAG_COPY_TOCPU);
            }
        }

        if (CTC_FLAG_ISSET(group_flag, CTC_IPMC_FLAG_REDIRECT_TOCPU))
        {
            sal_sprintf(buf, "%-10d%-15d%-10s", vrfid, ad_index, "redirect-tocpu");
        }
        else if (CTC_FLAG_ISSET(group_flag, CTC_IPMC_FLAG_COPY_TOCPU))
        {
            sal_sprintf(buf, "%-10d%-15d%-10s", vrfid, ad_index, "copy-tocpu");
        }
        else if (CTC_FLAG_ISSET(group_flag, CTC_IPMC_FLAG_RPF_CHECK) &&
            CTC_FLAG_ISSET(group_flag, CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU))
        {
            sal_sprintf(buf, "%-10d%-15d%-10s", vrfid, ad_index, "fallback-bridge");
        }
        else
        {
            sal_sprintf(buf, "%-10d%-15d%-10s", vrfid, ad_index, "drop");
        }

        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-40s\n", buf);
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_ipmc_dump_group_node_from_db(uint8 lchip, uint16 group_id)
{
    ctc_list_pointer_t  *p_linklist = NULL;
    ctc_list_pointer_node_t* p_node = NULL;
    sys_ipmc_group_node_t *p_ipmc_group_node;

    SYS_IPMC_INIT_CHECK();
    IPMC_LOCK;
    p_linklist = ctc_vector_get( p_usw_ipmc_master[lchip]->group_vec, group_id);
    if (!p_linklist)
    {
        IPMC_UNLOCK;
        return CTC_E_NOT_EXIST;
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-21s%-21s%-6s%-7s%s\n", "GID", "IPSA",
                     "Group-IP", "VRF", "NHID",  "Mode");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                     "------------------------------------------------------------------------------\n");
    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_ipmc_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        if (CTC_IP_VER_4 == p_ipmc_group_node->ip_version)
        {
            CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_ipmc_dump_group_info(lchip, p_ipmc_group_node, group_id), p_usw_ipmc_master[lchip]->p_ipmc_mutex);
        }
    }
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-46s%-6s%-7s%s\n", "GID",
                     "Group-IP/IPSA", "VRF", "NHID", "Mode");
    SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                     "------------------------------------------------------------------------------\n");
    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_ipmc_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        if (CTC_IP_VER_6 == p_ipmc_group_node->ip_version)
        {
            CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_ipmc_dump_group_info(lchip, p_ipmc_group_node, group_id), p_usw_ipmc_master[lchip]->p_ipmc_mutex);
        }
    }
    IPMC_UNLOCK;
    return CTC_E_NONE;
}


int32
sys_usw_ipmc_set_bidipim_mode(uint8 lchip, uint8 bidipim_mode)
{
    uint32 cmd = 0;
    uint32 value = 0;
    SYS_IPMC_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(bidipim_mode, SYS_IPMC_BIDIPIM_MODE3);
    IPMC_LOCK;
    p_usw_ipmc_master[lchip]->bidipim_mode = bidipim_mode;
    value = p_usw_ipmc_master[lchip]->bidipim_mode;
    IPMC_UNLOCK;
    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_bidiPimGroupType_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_wb_mapping_group_node(uint8 lchip, sys_wb_ipmc_group_node_t *p_wb_group_node, sys_ipmc_group_node_t *p_group_node)
{
    p_wb_group_node->ip_version = p_group_node->ip_version;
    p_wb_group_node->src_ip_mask_len = p_group_node->src_ip_mask_len;
    p_wb_group_node->group_ip_mask_len= p_group_node->group_ip_mask_len;
    p_wb_group_node->is_l2mc = p_group_node->is_l2mc;
    p_wb_group_node->share_grp = p_group_node->share_grp;
    p_wb_group_node->is_drop = p_group_node->is_drop;
    p_wb_group_node->with_nexthop = p_group_node->with_nexthop;
    p_wb_group_node->is_rd_cpu = p_group_node->is_rd_cpu;
    p_wb_group_node->ad_index = p_group_node->ad_index;

    if (p_group_node->ip_version == CTC_IP_VER_4)
    {
        p_wb_group_node->vrfid = p_group_node->address.ipv4.vrfid;
        sal_memset(p_wb_group_node->group_addr, 0, 4 * sizeof(uint32));
        sal_memset(p_wb_group_node->src_addr, 0, 4 * sizeof(uint32));
        sal_memcpy(p_wb_group_node->group_addr, &(p_group_node->address.ipv4.group_addr), sizeof(uint32));
        sal_memcpy(p_wb_group_node->src_addr, &(p_group_node->address.ipv4.src_addr), sizeof(uint32));
    }
    else
    {
        p_wb_group_node->vrfid = p_group_node->address.ipv6.vrfid;
        sal_memcpy(p_wb_group_node->group_addr, &(p_group_node->address.ipv6.group_addr), 4 * sizeof(uint32));
        sal_memcpy(p_wb_group_node->src_addr, &(p_group_node->address.ipv6.src_addr), 4 * sizeof(uint32));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_wb_mapping_group_info(uint8 lchip, sys_wb_ipmc_group_node_t *p_wb_group_node, ctc_ipmc_group_info_t* p_group_info)
{
    p_group_info->group_id = p_wb_group_node->group_id;
    p_group_info->ip_version = p_wb_group_node->ip_version;
    p_group_info->src_ip_mask_len = p_wb_group_node->src_ip_mask_len;
    p_group_info->group_ip_mask_len= p_wb_group_node->group_ip_mask_len;

    if (p_group_info->ip_version == CTC_IP_VER_4)
    {
        p_group_info->address.ipv4.vrfid = p_wb_group_node->vrfid;
        sal_memcpy(&(p_group_info->address.ipv4.group_addr), p_wb_group_node->group_addr, sizeof(uint32));
        sal_memcpy(&(p_group_info->address.ipv4.src_addr), p_wb_group_node->src_addr, sizeof(uint32));
    }
    else
    {
        p_group_info->address.ipv6.vrfid = p_wb_group_node->vrfid;
        sal_memcpy(&(p_group_info->address.ipv6.group_addr), p_wb_group_node->group_addr, 4 * sizeof(uint32));
        sal_memcpy(&(p_group_info->address.ipv6.src_addr), p_wb_group_node->src_addr, 4 * sizeof(uint32));
    }

    if (p_wb_group_node->is_l2mc)
    {
        CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_L2MC);
    }

    if (p_wb_group_node->share_grp)
    {
        CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_SHARE_GROUP);
    }

    if (p_wb_group_node->is_drop)
    {
        CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_DROP);
    }

    if (p_wb_group_node->is_rd_cpu)
    {
        CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_REDIRECT_TOCPU);
    }

    if (p_wb_group_node->with_nexthop)
    {
        CTC_SET_FLAG(p_group_info->flag, CTC_IPMC_FLAG_WITH_NEXTHOP);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_wb_mapping_g_node(uint8 lchip, sys_wb_ipmc_g_node_t *p_wb_g_node, sys_ipmc_g_node_t *p_g_node, uint8 sync)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if (sync)
    {
        p_wb_g_node->ipmc_type = p_g_node->ipmc_type;
        p_wb_g_node->is_group = p_g_node->is_group;
        p_wb_g_node->vrfid = p_g_node->vrfid;
        p_wb_g_node->ref_cnt = p_g_node->ref_cnt;
        p_wb_g_node->pointer = p_g_node->pointer;

        if ((p_g_node->ipmc_type == SYS_IPMC_V4) || (p_g_node->ipmc_type == SYS_IPMC_V4_L2MC))
        {
            sal_memset(p_wb_g_node->ip, 0, 4 * sizeof(uint32));
            sal_memcpy(p_wb_g_node->ip, &(p_g_node->ip_addr.ipv4), sizeof(uint32));
        }
        else
        {
            sal_memcpy(p_wb_g_node->ip, &(p_g_node->ip_addr.ipv6), 4 * sizeof(uint32));
        }
    }
    else
    {
        p_g_node->ipmc_type = p_wb_g_node->ipmc_type;
        p_g_node->is_group = p_wb_g_node->is_group;
        p_g_node->vrfid = p_wb_g_node->vrfid;
        p_g_node->ref_cnt = p_wb_g_node->ref_cnt;
        p_g_node->pointer = p_wb_g_node->pointer;

        if ((p_g_node->ipmc_type == SYS_IPMC_V4) || (p_g_node->ipmc_type == SYS_IPMC_V4_L2MC))
        {
            sal_memcpy(&(p_g_node->ip_addr.ipv4), p_wb_g_node->ip, sizeof(uint32));
            if (p_g_node->pointer >= SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE)
            {
                opf.pool_type = p_usw_ipmc_master[lchip]->opf_g_pointer[p_g_node->ipmc_type];
                opf.pool_index = 0;
                CTC_ERROR_RETURN( sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_g_node->pointer));
            }
        }
        else
        {
            sal_memcpy(&(p_g_node->ip_addr.ipv6), p_wb_g_node->ip, 4 * sizeof(uint32));
            if (p_g_node->pointer >= SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE)
            {
                opf.pool_type = p_usw_ipmc_master[lchip]->opf_g_pointer[p_g_node->ipmc_type];
                opf.pool_index = 0;
                CTC_ERROR_RETURN( sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_g_node->pointer));
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_wb_sync_group_node_func(ctc_list_pointer_t *p_linklist, uint32 vec_index, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_ipmc_group_node_t  *p_wb_group_node;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    ctc_list_pointer_node_t* p_node = NULL;
    sys_ipmc_group_node_t *p_group_node = NULL;
    uint8 lchip = (uint8)(data->value1);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    CTC_LIST_POINTER_LOOP(p_node, p_linklist)
    {
        p_wb_group_node = (sys_wb_ipmc_group_node_t *)wb_data->buffer + wb_data->valid_cnt;
        sal_memset(p_wb_group_node, 0, sizeof(sys_wb_ipmc_group_node_t));

        p_group_node = _ctc_container_of(p_node, sys_ipmc_group_node_t, list_head);
        p_wb_group_node->group_id = vec_index;
        CTC_ERROR_RETURN(_sys_usw_ipmc_wb_mapping_group_node(lchip, p_wb_group_node, p_group_node));
        if (++wb_data->valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
            wb_data->valid_cnt = 0;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipmc_wb_sync_g_node_func(sys_ipmc_g_node_t *p_g_node, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_ipmc_g_node_t  *p_wb_g_node;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    uint8 lchip = (uint8)(data->value1);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_g_node = (sys_wb_ipmc_g_node_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_g_node, 0, sizeof(sys_wb_ipmc_g_node_t));
    CTC_ERROR_RETURN(_sys_usw_ipmc_wb_mapping_g_node(lchip, p_wb_g_node, p_g_node, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipmc_wb_sync(uint8 lchip)
{
    uint32 loop = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_ipmc_master_t  *p_wb_ipmc_master;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }

    /*syncup  ipmc_matser*/
    wb_data.buffer = mem_malloc(MEM_IPMC_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipmc_master_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_MASTER);

    p_wb_ipmc_master = (sys_wb_ipmc_master_t  *)wb_data.buffer;

    p_wb_ipmc_master->lchip = lchip;
    p_wb_ipmc_master->version = SYS_WB_VERSION_IPMC;
    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        p_wb_ipmc_master->ipmc_group_cnt[loop] = p_usw_ipmc_master[lchip]->ipmc_group_cnt[loop];
        p_wb_ipmc_master->ipmc_group_sg_cnt[loop] = p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[loop];
    }

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipmc_group_node_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_GROUP_NODE);
    user_data.data = &wb_data;
    user_data.value1 = lchip;

    wb_data.valid_cnt = 0;
    CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_ipmc_master[lchip]->group_vec, 1, (vector_traversal_fn2) _sys_usw_ipmc_wb_sync_group_node_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipmc_g_node_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_G_NODE);
    user_data.data = &wb_data;
    user_data.value1 = lchip;

    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        wb_data.valid_cnt = 0;
        CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_ipmc_master[lchip]->g_hash[loop], (hash_traversal_fn) _sys_usw_ipmc_wb_sync_g_node_func, (void *)&user_data), ret, done);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_ipmc_wb_restore(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 loop = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    DsIpDa_m ds_ip_da;
    ctc_wb_query_t wb_query;
    sys_wb_ipmc_master_t  wb_ipmc_master = {0};
    sys_ipmc_group_node_t *p_group_node = NULL;
    sys_wb_ipmc_group_node_t  wb_group_node = {0};
    sys_ipmc_g_node_t *p_g_node = NULL;
    sys_wb_ipmc_g_node_t  wb_g_node = {0};
    ctc_ipmc_group_info_t* p_group_info = NULL;
    sys_ipmc_group_param_t ipmc_param;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    sal_memset(&ipmc_param, 0, sizeof(sys_ipmc_group_param_t));
    wb_query.buffer = mem_malloc(MEM_IPMC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ipmc_master_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  ipmc_master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query ipmc master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)&wb_ipmc_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_IPMC, wb_ipmc_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[loop] = wb_ipmc_master.ipmc_group_cnt[loop];
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[loop] = wb_ipmc_master.ipmc_group_sg_cnt[loop];
    }

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv4McastLookupResultCtl_defaultEntryBase_f);
    DRV_IOCTL(lchip, 0, cmd, &p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4]);

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv6McastLookupResultCtl_defaultEntryBase_f);
    DRV_IOCTL(lchip, 0, cmd, &p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6]);

    p_group_info = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_group_info)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ipmc_group_node_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_GROUP_NODE);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_group_node, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_group_node = mem_malloc(MEM_IPMC_MODULE, sizeof(sys_ipmc_group_node_t));
        if (NULL == p_group_node)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_group_node, 0, sizeof(sys_ipmc_group_node_t));
        sal_memset(p_group_info, 0, sizeof(ctc_ipmc_group_info_t));
        _sys_usw_ipmc_wb_mapping_group_info(lchip, &wb_group_node, p_group_info);
        ipmc_param.group = p_group_info;
        ipmc_param.ad_index = wb_group_node.ad_index;

        cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipmc_param.ad_index, cmd, &ds_ip_da));

        ipmc_param.rpf_chk_en =  GetDsIpDa(V, rpfCheckEn_f, &ds_ip_da);
        ipmc_param.rpf_mode = GetDsIpDa(V, rpfCheckMode_f, &ds_ip_da);
        ipmc_param.rpf_id = GetDsIpDa(V, rpfIfId_f, &ds_ip_da);

        if (ipmc_param.rpf_chk_en && (ipmc_param.rpf_mode == SYS_RPF_CHK_MODE_PROFILE))
        {
            CTC_ERROR_RETURN(sys_usw_rpf_restore_profile(lchip, ipmc_param.rpf_id));
        }

        /*add to soft table*/
        ret = sys_usw_ipmc_add_group_to_db(lchip, &ipmc_param);
        if (ret)
        {
            mem_free(p_group_node);
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ipmc_g_node_t, CTC_FEATURE_IPMC, SYS_WB_APPID_IPMC_SUBID_G_NODE);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_g_node, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_g_node = mem_malloc(MEM_IPMC_MODULE, sizeof(sys_ipmc_g_node_t));
        if (NULL == p_g_node)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_g_node, 0, sizeof(sys_ipmc_g_node_t));

        ret = _sys_usw_ipmc_wb_mapping_g_node(lchip, &wb_g_node, p_g_node, 0);
        if (ret)
        {
            mem_free(p_g_node);
            continue;
        }

        /*add to soft table*/
        if (!ctc_hash_insert(p_usw_ipmc_master[lchip]->g_hash[p_g_node->ipmc_type], p_g_node))
        {
            mem_free(p_g_node);
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (p_group_info)
    {
        mem_free(p_group_info);
    }

    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

int32
sys_usw_ipmc_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 i = 0;
    SYS_IPMC_INIT_CHECK();
    IPMC_LOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# IPMC");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "Bidipim mode: %u\n", p_usw_ipmc_master[lchip]->bidipim_mode);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-8s%-10s%-10s%-12s%-10s\n", "Type", "Dft base", "Grp cnt", "Grp sg cnt", "Opf type");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-8s%-10u%-10u%-12u%-10u\n", "IPMC V4", p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4],
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4],
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4],
        p_usw_ipmc_master[lchip]->opf_g_pointer[SYS_IPMC_V4]);
    SYS_DUMP_DB_LOG(p_f, "%-8s%-10u%-10u%-12u%-10u\n", "L2MC V4", p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4_L2MC],
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V4_L2MC],
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V4_L2MC],
        p_usw_ipmc_master[lchip]->opf_g_pointer[SYS_IPMC_V4_L2MC]);
    SYS_DUMP_DB_LOG(p_f, "%-8s%-10u%-10u%-12u%-10u\n", "IPMC V6", p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6],
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6],
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6],
        p_usw_ipmc_master[lchip]->opf_g_pointer[SYS_IPMC_V6]);
    SYS_DUMP_DB_LOG(p_f, "%-8s%-10u%-10u%-12u%-10u\n", "L2MC V6", p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6_L2MC],
        p_usw_ipmc_master[lchip]->ipmc_group_cnt[SYS_IPMC_V6_L2MC],
        p_usw_ipmc_master[lchip]->ipmc_group_sg_cnt[SYS_IPMC_V6_L2MC],
        p_usw_ipmc_master[lchip]->opf_g_pointer[SYS_IPMC_V6_L2MC]);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    for(i=0; i<SYS_IPMC_TYPE_MAX; i++)
    {
        sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_ipmc_master[lchip]->opf_g_pointer[i], p_f);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    IPMC_UNLOCK;
    return ret;
}

int32
sys_usw_ipmc_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd1, cmd2;
    uint32 tbl_id;
    uint32 value = 1;
    uint8 loop = 0;
    uint32 max_vrfid_num;
    uint32 g_pointer_hash_size[SYS_IPMC_TYPE_MAX] = {0};
    uint32 g_pointer_opf_size[SYS_IPMC_TYPE_MAX] = {0};
    sys_usw_opf_t opf;
    uint32 default_mode = 0;
    uint8  work_status = 0;

    if(NULL != p_usw_ipmc_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    LCHIP_CHECK(lchip);

    p_usw_ipmc_master[lchip] = mem_malloc(MEM_IPMC_MODULE, sizeof(sys_ipmc_master_t));
    if (NULL == p_usw_ipmc_master[lchip])
    {
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_ipmc_master[lchip], 0, sizeof(sys_ipmc_master_t));

    sal_mutex_create(&(p_usw_ipmc_master[lchip]->p_ipmc_mutex));
    if (NULL == p_usw_ipmc_master[lchip]->p_ipmc_mutex)
    {
        mem_free(p_usw_ipmc_master[lchip]);
        SYS_IPMC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    default_mode = sys_usw_l3if_get_default_entry_mode(lchip);

    tbl_id = DsIpDa_t;
    /*ipv4 ipmc*/
    max_vrfid_num = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) + 1;
    max_vrfid_num = default_mode ? 1 : max_vrfid_num;
    CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, tbl_id, CTC_INGRESS, max_vrfid_num, 1, &(p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4])), ret, error1);
    cmd1 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv4McastLookupResultCtl_defaultEntryBase_f);
    cmd2 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv4McastLookupResultCtl_defaultEntryEn_f);

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &(p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4])), ret, error2);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd2, &value), ret, error2);

    /*ipv6 ipmc*/
    CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, tbl_id, CTC_INGRESS, max_vrfid_num, 1, &(p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6])), ret, error2);
    cmd1 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv6McastLookupResultCtl_defaultEntryBase_f);
    cmd2 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv6McastLookupResultCtl_defaultEntryEn_f);

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &(p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6])), ret, error3);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd2, &value), ret, error3);

    cmd1 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv4McastLookupResultCtl_defaultEntryType_f);
    cmd2 = DRV_IOW(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv6McastLookupResultCtl_defaultEntryType_f);

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &default_mode), ret, error3);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd2, &default_mode), ret, error3);

    cmd1 = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gMacDaLookupResultCtl_defaultEntryBase_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &(p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4_L2MC])), ret, error3);

    p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6_L2MC] = p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4_L2MC];
    p_usw_ipmc_master[lchip]->bidipim_mode = SYS_IPMC_BIDIPIM_MODE0;
    value = p_usw_ipmc_master[lchip]->bidipim_mode;
    cmd1 = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_bidiPimGroupType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &value), ret, error3);

    g_pointer_hash_size[SYS_IPMC_V4] = SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE;
    g_pointer_opf_size[SYS_IPMC_V4] = SYS_USW_IPMC_V4_G_POINTER_OPF_SIZE;
    g_pointer_hash_size[SYS_IPMC_V4_L2MC] = g_pointer_hash_size[SYS_IPMC_V4];
    g_pointer_opf_size[SYS_IPMC_V4_L2MC] = g_pointer_opf_size[SYS_IPMC_V4];
    g_pointer_hash_size[SYS_IPMC_V6] = SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE;
    g_pointer_opf_size[SYS_IPMC_V6] = SYS_USW_IPMC_V6_G_POINTER_OPF_SIZE;
    g_pointer_hash_size[SYS_IPMC_V6_L2MC] = SYS_USW_IPMC_L2_V6_G_POINTER_HASH_SIZE;
    g_pointer_opf_size[SYS_IPMC_V6_L2MC] = SYS_USW_IPMC_L2_V6_G_POINTER_OPF_SIZE;

    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        p_usw_ipmc_master[lchip]->g_hash[loop] =  ctc_hash_create(1, g_pointer_hash_size[loop],
                                                                  (hash_key_fn)_sys_usw_ipmc_g_hash_make,
                                                                  (hash_cmp_fn)_sys_usw_ipmc_g_hash_compare);

        CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_ipmc_master[lchip]->opf_g_pointer[loop], 1, "opf-ipmc-pointer"), ret, error4);
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = p_usw_ipmc_master[lchip]->opf_g_pointer[loop];
        opf.pool_index = 0;
        /* the pointer 0 is invalid,  pointer will increase 1*/
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, g_pointer_hash_size[loop]+1, g_pointer_opf_size[loop]-1), ret, error4);
    }

    p_usw_ipmc_master[lchip]->group_vec = ctc_vector_init(MCHIP_CAP(SYS_CAP_SPEC_MCAST_GROUP_NUM) / SYS_USW_IPMC_VECTOR_BLOCK_SIZE, SYS_USW_IPMC_VECTOR_BLOCK_SIZE);

    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_IPMC, sys_usw_ipmc_ftm_cb);

    for (loop = 0; loop < MAX_CTC_IP_VER; loop++)
    {
        CTC_ERROR_GOTO(sys_usw_ipmc_add_default_entry(lchip, loop, CTC_IPMC_DEFAULT_ACTION_DROP), ret, error5);
    }

    CTC_ERROR_GOTO(sys_usw_rpf_init(lchip), ret, error5);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_IPMC, sys_usw_ipmc_wb_sync), ret, error5);

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
	  CTC_ERROR_GOTO(sys_usw_ipmc_wb_restore(lchip), ret, error5);
    }

    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_IPMC, sys_usw_ipmc_dump_db), ret, error5);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error5:
    if (p_usw_ipmc_master[lchip]->group_vec)
    {
        ctc_vector_release(p_usw_ipmc_master[lchip]->group_vec);
    }

error4:
    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        if (p_usw_ipmc_master[lchip]->g_hash[loop])
        {
            ctc_hash_free(p_usw_ipmc_master[lchip]->g_hash[loop]);
        }

        if (p_usw_ipmc_master[lchip]->opf_g_pointer[loop])
        {
            sys_usw_opf_deinit(lchip, p_usw_ipmc_master[lchip]->opf_g_pointer[loop]);
        }
    }

error3:
    sys_usw_ftm_free_table_offset(lchip, tbl_id, CTC_INGRESS, max_vrfid_num, p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V6]);

error2:
    sys_usw_ftm_free_table_offset(lchip, tbl_id, CTC_INGRESS, max_vrfid_num, p_usw_ipmc_master[lchip]->default_base[SYS_IPMC_V4]);

error1:
    if (p_usw_ipmc_master[lchip]->p_ipmc_mutex)
    {
        sal_mutex_destroy(p_usw_ipmc_master[lchip]->p_ipmc_mutex);
    }

error0:
    mem_free(p_usw_ipmc_master[lchip]);

    return ret;
}

int32
sys_usw_ipmc_deinit(uint8 lchip)
{
    uint8 loop = 0;

    LCHIP_CHECK(lchip);

    if (NULL == p_usw_ipmc_master[lchip])
    {
        return CTC_E_NONE;
    }

    for (loop = 0; loop < SYS_IPMC_TYPE_MAX; loop++)
    {
        ctc_hash_traverse(p_usw_ipmc_master[lchip]->g_hash[loop], (hash_traversal_fn)sys_usw_ipmc_free_node_data, NULL);
        ctc_hash_free(p_usw_ipmc_master[lchip]->g_hash[loop]);

        sys_usw_opf_deinit(lchip, p_usw_ipmc_master[lchip]->opf_g_pointer[loop]);
    }

    sal_mutex_destroy(p_usw_ipmc_master[lchip]->p_ipmc_mutex);

    ctc_vector_traverse(p_usw_ipmc_master[lchip]->group_vec, (vector_traversal_fn)sys_usw_ipmc_free_node_data, NULL);
    ctc_vector_release(p_usw_ipmc_master[lchip]->group_vec);

    sys_usw_rpf_deinit(lchip);

    mem_free(p_usw_ipmc_master[lchip]);

    return CTC_E_NONE;
}

#endif

