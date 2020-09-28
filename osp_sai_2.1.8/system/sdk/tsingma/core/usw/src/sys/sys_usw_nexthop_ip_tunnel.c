/**
 @file sys_usw_nexthop_ip_tunnel.c

 @date 2011-05-23

 @version v2.0

 The file contains all nexthop ip tunnel related callback function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_linklist.h"

#include "sys_usw_chip.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_vlan.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_opf.h"
#include "sys_usw_l3if.h"
#include "sys_usw_register.h"
#include "sys_usw_common.h"
#include "sys_usw_wb_nh.h"

#include "usw/include/drv_io.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/


#define SYS_NH_OVERLAY_TUNNEL_MIN_INNER_FID      (0x1FFE)   /*reserve fid*/
#define SYS_NH_OVERLAY_TUNNEL_MAX_INNER_FID      (0x3FFE)
#define SYS_NH_OVERLAY_IHASH_MASK         0xFFF


#define SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE 41
#define SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE 4

#define SYS_NH_IP_TUNNEL_MAX_DSCP_VALUE  63

#define INTERCEPT_IPV6_ADDR(ip, len)                    \
{                                                       \
    uint8 sublen = len % 32;                            \
    int8 index = len / 32;                              \
    if (sublen)                                         \
    {                                                   \
        (ip)[index] >>= 32 - sublen;                    \
        (ip)[index] <<= 32 - sublen;                    \
        index += 1;                                     \
    }                                                   \
    for (; index < 4; index++)                          \
    {                                                   \
        (ip)[index] = 0;                                \
    }                                                   \
}

#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define MIX(a, b, c) \
    do \
    { \
        a -= c;  a ^= ROT(c, 4);  c += b; \
        b -= a;  b ^= ROT(a, 6);  a += c; \
        c -= b;  c ^= ROT(b, 8);  b += a; \
        a -= c;  a ^= ROT(c, 16);  c += b; \
        b -= a;  b ^= ROT(a, 19);  a += c; \
        c -= b;  c ^= ROT(b, 4);  b += a; \
    } while (0)
extern int32 sys_usw_global_get_gint_en(uint8 lchip, uint8* enable);
STATIC int32 _sys_usw_nh_vxlan_add_vni (uint8 lchip,  uint32 vn_id, uint16* p_fid);
STATIC int32 _sys_usw_nh_vxlan_remove_vni(uint8 lchip,  uint32 vn_id);

STATIC int32
_sys_usw_nh_ip_tunnel_udf_profile_param_check(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_ip_param)
{
   ctc_nh_udf_profile_param_t profile_edit;

    sal_memset(&profile_edit, 0, sizeof(profile_edit));
    CTC_MAX_VALUE_CHECK(p_nh_ip_param->tunnel_info.udf_profile_id,8);

    if (!CTC_IS_BIT_SET(g_usw_nh_master[lchip]->udf_profile_bitmap, p_nh_ip_param->tunnel_info.udf_profile_id))
    {
        return CTC_E_NOT_EXIST;
    }

    /*check tunnel type and udf profile edit type*/
    profile_edit.profile_id = p_nh_ip_param->tunnel_info.udf_profile_id;
    CTC_ERROR_RETURN(_sys_usw_nh_get_udf_profile(lchip, &profile_edit));

    switch (profile_edit.offset_type)
    {
        case CTC_NH_UDF_OFFSET_TYPE_RAW:
            if (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_NONE)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tunnel type not match udf profile offset type\n");
                return CTC_E_INVALID_CONFIG;
            }
            break;

        case CTC_NH_UDF_OFFSET_TYPE_UDP:
            if ((p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN6) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GENEVE_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GENEVE_IN6))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tunnel type not match udf profile offset type\n");
                return CTC_E_INVALID_CONFIG;
            }
            break;
        case CTC_NH_UDF_OFFSET_TYPE_GRE:
            if (((p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GRE_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GRE_IN6)) ||
                CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tunnel type not match udf profile offset type\n");
                return CTC_E_INVALID_CONFIG;
            }

            break;
        case CTC_NH_UDF_OFFSET_TYPE_GRE_WITH_KEY:
            if ((((p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GRE_IN4)&&(p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_GRE_IN6)) ||
                !CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY)) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_NVGRE_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_NVGRE_IN6))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tunnel type not match udf profile offset type\n");
                return CTC_E_INVALID_CONFIG;
            }
            break;
        case CTC_NH_UDF_OFFSET_TYPE_IP:
            if ((p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_IPV4_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_IPV6_IN4) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_IPV4_IN6) &&
                (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_IPV6_IN6))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tunnel type not match udf profile offset type\n");
                return CTC_E_INVALID_CONFIG;
            }
            break;
        default :
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_ip_tunnel_get_port_extend_mcast_en(uint8 lchip, uint8 *p_enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_enable);
    cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_portExtenderMcastEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    *p_enable = value;
    return CTC_E_NONE;
}

int32
_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(uint8 lchip, uint8 ip_ver, void* p_add, uint8* p_index, uint8 old_index)
{
    uint8 idx = 0;
    uint8 max_idx = 0;
    sys_nh_ip_tunnel_sa_v4_node_t* p_node = NULL;
    sys_nh_ip_tunnel_sa_v4_node_t* p_free = NULL;
    sys_nh_ip_tunnel_sa_v6_node_t* p_v6_node = NULL;
    uint8 free_idx = MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM);
    int32 match = 1;
    uint32 cmd;
    ds_t v4_sa = {0};
    ds_t v6_sa = {0};
    ipv6_addr_t tmp = {0};
    uint32 tbl_id;
    uint8  replace_en = 0;

	sys_usw_nh_master_t* p_nh_master = NULL;
	CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));


    max_idx = (ip_ver == CTC_IP_VER_4) ? MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM)
        : MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM);
    p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)p_nh_master->ipsa[CTC_IP_VER_4];
    p_v6_node = (sys_nh_ip_tunnel_sa_v6_node_t*)p_nh_master->ipsa[CTC_IP_VER_6];

    *p_index = 0xFF;

    p_node = (CTC_IP_VER_4 == ip_ver) ? p_node : (sys_nh_ip_tunnel_sa_v4_node_t*)p_v6_node;
    CTC_PTR_VALID_CHECK(p_node);

    if (old_index != 0xff)
    {
        if (CTC_IP_VER_4 == ip_ver)
        {
            p_node = p_node + old_index;
        }
        else
        {
            p_v6_node = p_v6_node + old_index;
            p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)p_v6_node;
        }

        if (p_node->count == 1)
        {
            free_idx = old_index;
            p_free = p_node;
            replace_en = 1;
        }

        p_node = (CTC_IP_VER_4 == ip_ver) ? (sys_nh_ip_tunnel_sa_v4_node_t*)p_nh_master->ipsa[CTC_IP_VER_4] :
            (sys_nh_ip_tunnel_sa_v4_node_t*)p_nh_master->ipsa[CTC_IP_VER_6];
    }

    for (idx = 0; idx <  max_idx ; idx++)
    {
        if ((p_node->count == 0) && (free_idx == MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM)))
        {
            free_idx = idx;
            p_free = p_node;
        }
        else if (p_node->count != 0)
        {
            match = sal_memcmp(&(p_node->ipv4), p_add,
                               ((CTC_IP_VER_4 == ip_ver) ? sizeof(ip_addr_t) : sizeof(ipv6_addr_t)));
        }

        if (0 == match)
        {
            *p_index = idx ;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Use old SA INDEX = %d old count is %d \n", *p_index, p_node->count);
            p_node->count++;
            return CTC_E_NONE;
        }

        if (CTC_IP_VER_4 == ip_ver)
        {
            p_node++;
        }
        else
        {
            p_v6_node++;
            p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)p_v6_node;
        }
    }

    if ( free_idx != MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM))
    {
        *p_index = free_idx;
        p_free->count++;
        sal_memcpy(&(p_free->ipv4), p_add, ((CTC_IP_VER_4 == ip_ver) ? sizeof(ip_addr_t) : sizeof(ipv6_addr_t)));
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;
    }

    tbl_id = (CTC_IP_VER_4 == ip_ver) ? DsL3TunnelV4IpSa_t : DsL3TunnelV6IpSa_t;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    if (CTC_IP_VER_4 == ip_ver)
    {
        DRV_SET_FIELD_V(lchip, DsL3TunnelV4IpSa_t, DsL3TunnelV4IpSa_ipSa_f , v4_sa, p_free->ipv4);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, free_idx, cmd, &v4_sa));
    }
    else
    {
        p_v6_node = (sys_nh_ip_tunnel_sa_v6_node_t*)p_free;
        tmp[0] = p_v6_node->ipv6[3];
        tmp[1] = p_v6_node->ipv6[2];
        tmp[2] = p_v6_node->ipv6[1];
        tmp[3] = p_v6_node->ipv6[0];
        SetDsL3TunnelV6IpSa(A, ipSa_f, v6_sa, tmp);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, free_idx, cmd, &v6_sa));
    }

    if (!replace_en)
    {
        if (CTC_IP_VER_4 == ip_ver)
        {
            p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]++;
        }
        else
        {
            p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA]++;
        }
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add new SA INDEX = %d \n", free_idx);

    return CTC_E_NONE;
}

int32
_sys_usw_nh_ip_tunnel_free_ipsa_idx(uint8 lchip, uint8 ip_ver, uint8 index)
{

    sys_nh_ip_tunnel_sa_v4_node_t* p_node;
    sys_nh_ip_tunnel_sa_v6_node_t* p_v6_node;

	sys_usw_nh_master_t* p_nh_master = NULL;
	CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    if (index == 0xFF)
    {
        return CTC_E_NONE;
    }

    if (((ip_ver == CTC_IP_VER_4) && index > MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM))
        || ((ip_ver == CTC_IP_VER_6) && index > MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM)))
    {
        return CTC_E_INVALID_PARAM;
    }


    p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)p_nh_master->ipsa[CTC_IP_VER_4];
    p_v6_node = (sys_nh_ip_tunnel_sa_v6_node_t*)p_nh_master->ipsa[CTC_IP_VER_6];

	 if (CTC_IP_VER_4 == ip_ver)
     {
         p_node = (p_node + index);
     }
     else
     {
        p_v6_node = (p_v6_node + index);
     }


    p_node = (CTC_IP_VER_4 == ip_ver) ? p_node : (sys_nh_ip_tunnel_sa_v4_node_t*)p_v6_node;

    if (0 == p_node->count)
    {
        return CTC_E_NONE;
    }

    p_node->count--;
    if (0 == p_node->count)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free SA INDEX = %d \n", index);
        sal_memset(&(p_node->ipv4), 0, ((ip_ver == CTC_IP_VER_4) ? sizeof(ip_addr_t) : sizeof(ipv6_addr_t)));
        if (CTC_IP_VER_4 == ip_ver)
        {
            p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA] =
                p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]?(p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]-1):0;
        }
        else
        {
            p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA] =
                p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA]?(p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA]-1):0;
        }
        return CTC_E_NONE;
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free SA INDEX = %d counter is %d \n", index, p_node->count);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_prepare(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    if(p_nh_param->tunnel_info.dot1ae_chan_id)
    {
        sys_usw_register_dot1ae_bind_sc_t bind_sc;
        uint32 derive_mode = 0;

        if ((p_nh_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN4) &&
                (p_nh_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN6))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Dot1ae only support for Vxlan tunnel \n");
            return CTC_E_INVALID_CONFIG;
        }

        if (p_edit_db->dot1ae_en && (p_edit_db->dot1ae_channel != p_nh_param->tunnel_info.dot1ae_chan_id))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Dot1ae channel have already bind! \n");
            return CTC_E_INVALID_CONFIG;
        }
        else if(p_edit_db->dot1ae_en && (p_edit_db->dot1ae_channel == p_nh_param->tunnel_info.dot1ae_chan_id))
        {
            return CTC_E_NONE;
        }

        sal_memset(&bind_sc,0,sizeof(bind_sc));
        bind_sc.type = 1;
        bind_sc.dir = 0;  /*tx*/
        bind_sc.gport = p_edit_db->hdr.nh_id;

        CTC_ERROR_RETURN(sys_usw_global_ctl_get(lchip, CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE, &derive_mode));

        if (derive_mode == CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE2)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop derive mode conflict with tunnel dot1ae! \n");
            return CTC_E_INVALID_CONFIG;
        }

        bind_sc.chan_id = p_nh_param->tunnel_info.dot1ae_chan_id;
        if (NULL == REGISTER_API(lchip)->dot1ae_bind_sec_chan)
        {
            return CTC_E_NOT_SUPPORT;
        }

        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_bind_sec_chan(lchip,&bind_sc));

        p_edit_db->dot1ae_en = 1;
        p_edit_db->dot1ae_channel = p_nh_param->tunnel_info.dot1ae_chan_id;
        p_edit_db->sc_index = (bind_sc.sc_index<<2);
        p_edit_db->sci_en   = bind_sc.include_sci;
    }
    else if (p_edit_db->dot1ae_en)
    {
        sys_usw_register_dot1ae_bind_sc_t bind_sc;
        sal_memset(&bind_sc,0,sizeof(bind_sc));
        bind_sc.type = 1;
        bind_sc.dir = 0;  /*tx*/
        bind_sc.gport = p_edit_db->hdr.nh_id;
        bind_sc.sc_index= (p_edit_db->sc_index>>2);
        if (NULL == REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
        {
            return CTC_E_NOT_SUPPORT;
        }

        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip,&bind_sc));

        p_edit_db->dot1ae_en = 0;
        p_edit_db->dot1ae_channel = 0;
        p_edit_db->sc_index = 0;
        p_edit_db->sci_en = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_v4(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_tunnelv4_t  dsl3edit4w;
    uint8 sa_index = MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM);
    int32 ret = CTC_E_NONE;
    sys_dsl3edit_tunnelv4_t*  p_dsl3edit4w;

    sal_memset(&dsl3edit4w, 0, sizeof(dsl3edit4w));
    dsl3edit4w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_TUNNEL_V4;
    dsl3edit4w.ds_type   = SYS_NH_DS_TYPE_L3EDIT;

    /*ttl*/
    dsl3edit4w.ttl = p_nh_param->tunnel_info.ttl;
    dsl3edit4w.map_ttl = CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,
                                        CTC_IP_NH_TUNNEL_FLAG_MAP_TTL);

    if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_NONE)
    {
        dsl3edit4w.ecn_mode = 3;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_ASSIGN)
    {
        return CTC_E_NOT_SUPPORT;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_MAP)
    {
        dsl3edit4w.ecn_mode = 1;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_PACKET)
    {
        dsl3edit4w.ecn_mode = 0;
    }

    /*stats*/
    if (p_nh_param->tunnel_info.stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_nh_param->tunnel_info.stats_id,
                                                           &(dsl3edit4w.stats_ptr)));
    }

    if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_COPY_DONT_FRAG))
    {
        dsl3edit4w.copy_dont_frag = 1;
        dsl3edit4w.dont_frag  = 0;
    }
    else
    {
        /*dont frag*/
        dsl3edit4w.copy_dont_frag = 0;
        dsl3edit4w.dont_frag = CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,
                                              CTC_IP_NH_TUNNEL_FLAG_DONT_FRAG);
    }

    /* dscp */
    if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_ASSIGN)
    {
        CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.dscp_or_tos, SYS_NH_IP_TUNNEL_MAX_DSCP_VALUE);
        dsl3edit4w.derive_dscp = 1;
        dsl3edit4w.dscp = p_nh_param->tunnel_info.dscp_or_tos;
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_MAP)
    {
        dsl3edit4w.derive_dscp = 2;
        dsl3edit4w.dscp_domain = p_nh_param->tunnel_info.dscp_domain;
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_PACKET)
    {
        dsl3edit4w.derive_dscp = 3;
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_NONE)
    {
        dsl3edit4w.derive_dscp = 0;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* mtu check */
    if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_MTU_CHECK))
    {
        if (p_nh_param->tunnel_info.mtu_size > 0x3FFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3edit4w.mtu_check_en = 1;
        dsl3edit4w.mtu_size = p_nh_param->tunnel_info.mtu_size & 0x3FFF;
    }

    if (p_nh_param->tunnel_info.dot1ae_chan_id)
    {
        dsl3edit4w.cloud_sec_en = 1;
        dsl3edit4w.sc_index = p_edit_db->sc_index;
        dsl3edit4w.sci_en = p_edit_db->sci_en;
    }

    if (p_nh_param->tunnel_info.udf_profile_id)
    {
        uint8 index = 0;
        dsl3edit4w.udf_profile_id = p_nh_param->tunnel_info.udf_profile_id;
        p_edit_db->udf_profile_id = p_nh_param->tunnel_info.udf_profile_id;

        for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
        {
            dsl3edit4w.udf_edit[index] = p_nh_param->tunnel_info.udf_replace_data[index];

            if (index != 3)
            {
                *((uint16*)&dsnh->macDa[index*2]) = dsl3edit4w.udf_edit[index];
            }
            else
            {
                dsnh->rid = dsl3edit4w.udf_edit[index] & 0xff;
                dsnh->wlan_mc_en = (dsl3edit4w.udf_edit[index]>>8) & 0xff;
            }
        }

        dsnh->flag |= SYS_NH_PARAM_FLAG_UDF_EDIT_EN;
    }

    switch (p_nh_param->tunnel_info.tunnel_type)
    {
    case CTC_TUNNEL_TYPE_IPV4_IN4:
        /* ip protocol */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_IP_BFD ))
        {
            dsl3edit4w.ip_protocol_type = 17; /*UDP*/
        }
        else
        {
            dsl3edit4w.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE;
        }

        /* SYSTEM MODIFIED for hybrid v4/v6 exchange */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL ))
        {
            dsl3edit4w.ip_protocol_type = p_nh_param->tunnel_info.openflow_ip_protocol;
        }

        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6TO4;
        break;

    case CTC_TUNNEL_TYPE_IPV6_IN4:
        dsl3edit4w.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE;
        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6TO4;
        break;

    case CTC_TUNNEL_TYPE_GRE_IN4:
        dsl3edit4w.inner_header_valid   = 1;
        dsl3edit4w.gre_protocol     = p_nh_param->tunnel_info.gre_info.protocol_type;
        if (!p_nh_param->tunnel_info.gre_info.protocol_type)
        {
            dsl3edit4w.gre_protocol = 0x8847;
            dsl3edit4w.is_gre_auto = 1;
        }
        dsl3edit4w.gre_version = p_nh_param->tunnel_info.gre_info.gre_ver & 0x1;
        if (p_nh_param->tunnel_info.gre_info.protocol_type == 0x6558)
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        }

        /*Encapsulate Xerspan Extend header */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR))
        {
            dsl3edit4w.xerspan_hdr_en = 1;

            if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR_HASH))
            {
                dsl3edit4w.xerspan_hdr_with_hash_en = 1;
            }
        }

        /* gre with key */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY))
        {
            dsl3edit4w.inner_header_type = 0x1;
            dsl3edit4w.gre_flags = 0x2;
            dsl3edit4w.gre_key = p_nh_param->tunnel_info.gre_info.gre_key;
        }
        else /* gre without key */
        {
            dsl3edit4w.inner_header_type = 0x0;
        }

        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_4,
                                                       &(p_nh_param->tunnel_info.ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
        dsl3edit4w.ipsa_index = sa_index & 0xF;

        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_GRE;
        p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IN_V4;
        p_edit_db->sa_index = sa_index;
        if (CTC_TUNNEL_TYPE_GRE_IN4 == p_nh_param->tunnel_info.tunnel_type && dsnh->inner_l2edit_ptr)
        {
            dsl3edit4w.out_l2edit_valid = 1;
            dsl3edit4w.l2edit_ptr = dsnh->l2edit_ptr;
        }
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
        }
        break;

    case CTC_TUNNEL_TYPE_ISATAP:
        dsl3edit4w.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE;
        dsl3edit4w.isatp_tunnel = 1;
        dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6TO4;
        break;

    case CTC_TUNNEL_TYPE_6TO4:
        dsl3edit4w.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE;
        dsl3edit4w.tunnel6_to4_da = 1;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_6TO4_USE_MANUAL_IPSA))
        {
            dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        }
        else
        {
            dsl3edit4w.tunnel6_to4_sa = 1;
        }
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6TO4;
        break;

    case CTC_TUNNEL_TYPE_6RD:
        if (p_nh_param->tunnel_info.sixrd_info.sp_prefix_len >= MAX_CTC_NH_SP_PREFIX_LENGTH)
        {
            return CTC_E_INVALID_PARAM;
        }

        if ((p_nh_param->tunnel_info.sixrd_info.v4_da_masklen % 8) || (p_nh_param->tunnel_info.sixrd_info.v4_sa_masklen % 8))
        {
            return CTC_E_NOT_SUPPORT;
        }

        dsl3edit4w.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE;

        /* ipda */
        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        if (p_nh_param->tunnel_info.sixrd_info.v4_da_masklen < 32)
        {
            dsl3edit4w.tunnel6_to4_da = 1;
            dsl3edit4w.tunnel6_to4_da_ipv4_prefixlen = p_nh_param->tunnel_info.sixrd_info.v4_da_masklen & 0x1F;
            dsl3edit4w.tunnel6_to4_da_ipv6_prefixlen = p_nh_param->tunnel_info.sixrd_info.sp_prefix_len & 0x7;
        }
        else if (p_nh_param->tunnel_info.sixrd_info.v4_da_masklen == 32)
        {
            dsl3edit4w.tunnel6_to4_da = 0;
            /* When tunnel6_to4_da is 0, the below two fields are not care, only for debug info */
            dsl3edit4w.tunnel6_to4_da_ipv4_prefixlen = 1;
            dsl3edit4w.tunnel6_to4_da_ipv6_prefixlen = 1;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        /* ipsa */
        if (p_nh_param->tunnel_info.sixrd_info.v4_sa_masklen < 32)
        {
            dsl3edit4w.tunnel6_to4_sa = 1;
            dsl3edit4w.ipsa_6to4 = p_nh_param->tunnel_info.ip_sa.ipv4;
            dsl3edit4w.tunnel6_to4_sa_ipv4_prefixlen = p_nh_param->tunnel_info.sixrd_info.v4_sa_masklen & 0x1F;
            dsl3edit4w.tunnel6_to4_sa_ipv6_prefixlen = p_nh_param->tunnel_info.sixrd_info.sp_prefix_len & 0x7;
        }
        else if (p_nh_param->tunnel_info.sixrd_info.v4_sa_masklen == 32)
        {
            dsl3edit4w.tunnel6_to4_sa = 0;
            /* When tunnel6_to4_da is 0, the below two fields are not care, only for debug info */
            dsl3edit4w.tunnel6_to4_sa_ipv4_prefixlen = 1;
            dsl3edit4w.tunnel6_to4_sa_ipv6_prefixlen = 1;
            dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6RD;
        break;

    case CTC_TUNNEL_TYPE_NVGRE_IN4:
        dsl3edit4w.ipsa_valid = 1;
        dsl3edit4w.inner_header_valid   = 1;
        /*dsl3edit4w.gre_protocol     = p_nh_param->tunnel_info.gre_info.protocol_type;*/
        /*dsl3edit4w.gre_version = p_nh_param->tunnel_info.gre_info.gre_ver & 0x1;*/

        dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
        }
        /* gre with key */
        dsl3edit4w.inner_header_type = 0x1;
        dsl3edit4w.gre_flags = 0x2;
        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_NVGRE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
        {
            dsl3edit4w.out_l2edit_valid = 1;
            dsl3edit4w.l2edit_ptr = dsnh->l2edit_ptr;
        }
        break;

    case CTC_TUNNEL_TYPE_GENEVE_IN4:  /*almost as vxlan*/
        dsl3edit4w.is_geneve = 1;
    case CTC_TUNNEL_TYPE_VXLAN_IN4:
        dsl3edit4w.inner_header_valid   = 1;
        dsl3edit4w.ipsa_valid = 1;
        dsl3edit4w.inner_header_type = 0x2; /* 2'b10: 8 byte UDP header */
        dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
        dsl3edit4w.ipsa = p_nh_param->tunnel_info.ip_sa.ipv4;
        dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
        }
        dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_VXLAN;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
        {
            dsl3edit4w.out_l2edit_valid = 1;
            dsl3edit4w.l2edit_ptr = dsnh->l2edit_ptr;
        }
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE))
        {
            dsl3edit4w.is_evxlan = 1;
            if (PKT_TYPE_ETH == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3edit4w.evxlan_protocl_index =  2;
            }
            else if (PKT_TYPE_IPV4 == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3edit4w.evxlan_protocl_index =  0;
            }
            else if (PKT_TYPE_IPV6 == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3edit4w.evxlan_protocl_index =  1;
            }
            else
            {
                if (PKT_TYPE_FLEXIBLE == p_nh_param->tunnel_info.inner_packet_type)
                {
                    dsl3edit4w.is_vxlan_auto = 1;
                }
                else
                {
                    dsl3edit4w.evxlan_protocl_index =  3;
                }
            }

        }
        else if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN))
        {
            /*using rsv profile 1 as GBP encap*/
            dsl3edit4w.is_geneve = 0;
            dsl3edit4w.evxlan_protocl_index = 1;
        }

        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT))
        {
            dsl3edit4w.udp_src_port_en = 1;
            dsl3edit4w.l4_src_port = p_nh_param->tunnel_info.l4_src_port;
        }
        break;

    case CTC_TUNNEL_TYPE_NONE:
        //-dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_NONE;
        p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IN_V4;
        dsl3edit4w.ttl = 255;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if ((p_nh_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV4_IN4) || (p_nh_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV6_IN4))
    {
        if (p_nh_param->tunnel_info.l4_src_port && p_nh_param->tunnel_info.l4_dst_port)
        {
            /*UDP tunnel enable*/
            dsl3edit4w.ipsa = 0;
            dsl3edit4w.inner_header_valid   = 1;
            dsl3edit4w.inner_header_type = 0x2; /* 2'b10: 8 byte UDP header */
            CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_4,
                                           &(p_nh_param->tunnel_info.ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
            dsl3edit4w.ipsa_index = sa_index & 0xF;
            dsl3edit4w.l4_dest_port = p_nh_param->tunnel_info.l4_dst_port;
            dsl3edit4w.l4_src_port = p_nh_param->tunnel_info.l4_src_port;
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_NONE;
        }
    }

    p_dsl3edit4w = &dsl3edit4w;
    CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3edit4w, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4, &dsnh->l3edit_ptr));

    p_edit_db->p_l3edit = p_dsl3edit4w;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;

    p_edit_db->sa_index = sa_index;

    return ret;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_v6(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_tunnelv6_t dsl3editv6;
    uint32 ttl_index = 0;
    uint8 sa_index = MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM);
    sys_dsl3edit_tunnelv6_t* p_dsl3editv6;

    sal_memset(&dsl3editv6, 0, sizeof(dsl3editv6));
    dsl3editv6.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_TUNNEL_V6;
    dsl3editv6.ds_type   = SYS_NH_DS_TYPE_L3EDIT;

    /*ttl*/
    CTC_ERROR_RETURN(sys_usw_lkup_ttl_index(lchip, p_nh_param->tunnel_info.ttl, &ttl_index));
    dsl3editv6.ttl = p_nh_param->tunnel_info.ttl;
    dsl3editv6.map_ttl = CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,
                                        CTC_IP_NH_TUNNEL_FLAG_MAP_TTL);

    if (p_nh_param->tunnel_info.dot1ae_chan_id)
    {
        dsl3editv6.cloud_sec_en = 1;
        dsl3editv6.sc_index = p_edit_db->sc_index;
        dsl3editv6.sci_en = p_edit_db->sci_en;
    }

    if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_NONE)
    {
        dsl3editv6.ecn_mode = 3;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_ASSIGN)
    {
        return CTC_E_NOT_SUPPORT;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_MAP)
    {
        dsl3editv6.ecn_mode = 1;
    }
    else if (p_nh_param->tunnel_info.ecn_select ==  CTC_NH_ECN_SELECT_PACKET)
    {
        dsl3editv6.ecn_mode = 0;
    }

    /*stats*/
    if (p_nh_param->tunnel_info.stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_nh_param->tunnel_info.stats_id,
                                                           &(dsl3editv6.stats_ptr)));
    }

    if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_ASSIGN)
    {
        CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.dscp_or_tos, SYS_NH_IP_TUNNEL_MAX_DSCP_VALUE);
        dsl3editv6.derive_dscp = 1;
        dsl3editv6.tos = (p_nh_param->tunnel_info.dscp_or_tos<<2);
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_MAP)
    {
        dsl3editv6.derive_dscp = 2;
        dsl3editv6.tos = 1 << 0;
        dsl3editv6.dscp_domain = p_nh_param->tunnel_info.dscp_domain;
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_PACKET)
    {
        dsl3editv6.derive_dscp = 3;
        dsl3editv6.tos = 0 << 0;
    }
    else if (p_nh_param->tunnel_info.dscp_select == CTC_NH_DSCP_SELECT_NONE)
    {
        dsl3editv6.derive_dscp = 0;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_param->tunnel_info.flow_label_mode == CTC_NH_FLOW_LABEL_NONE)
    {
        dsl3editv6.new_flow_label_valid = 0x0;
    }
    else if (p_nh_param->tunnel_info.flow_label_mode == CTC_NH_FLOW_LABEL_WITH_HASH ||
             p_nh_param->tunnel_info.flow_label_mode == CTC_NH_FLOW_LABEL_ASSIGN)
    {
        if (p_nh_param->tunnel_info.flow_label > 0xFFFFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3editv6.new_flow_label_valid = 0x1;
        dsl3editv6.new_flow_label_mode = (p_nh_param->tunnel_info.flow_label_mode == CTC_NH_FLOW_LABEL_WITH_HASH);
        dsl3editv6.flow_label = p_nh_param->tunnel_info.flow_label;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* mtu check */
    if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_MTU_CHECK))
    {
        if (p_nh_param->tunnel_info.mtu_size > 0x3FFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3editv6.mtu_check_en = 1;
        dsl3editv6.mtu_size = p_nh_param->tunnel_info.mtu_size & 0x3FFF;
    }

    /* ipda, use little india for DRV_SET_FIELD_A */
    dsl3editv6.ipda[0] = p_nh_param->tunnel_info.ip_da.ipv6[3];
    dsl3editv6.ipda[1] = p_nh_param->tunnel_info.ip_da.ipv6[2];
    dsl3editv6.ipda[2] = p_nh_param->tunnel_info.ip_da.ipv6[1];
    dsl3editv6.ipda[3] = p_nh_param->tunnel_info.ip_da.ipv6[0];

    if (p_nh_param->tunnel_info.udf_profile_id)
    {
        uint8 index = 0;
        dsl3editv6.udf_profile_id = p_nh_param->tunnel_info.udf_profile_id;
        p_edit_db->udf_profile_id = p_nh_param->tunnel_info.udf_profile_id;

        for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
        {
            dsl3editv6.udf_edit[index] = p_nh_param->tunnel_info.udf_replace_data[index];
            if (index != 3)
            {
                *((uint16*)&dsnh->macDa[index*2]) = dsl3editv6.udf_edit[index];
            }
            else
            {
                dsnh->rid = dsl3editv6.udf_edit[index] & 0xff;
                dsnh->wlan_mc_en = (dsl3editv6.udf_edit[index]>>8) & 0xff;
            }
        }
        dsnh->flag |= SYS_NH_PARAM_FLAG_UDF_EDIT_EN;
    }

    switch (p_nh_param->tunnel_info.tunnel_type)
    {
    case CTC_TUNNEL_TYPE_IPV4_IN6:
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_IP_BFD ))
        {
            dsl3editv6.ip_protocol_type = 17; /*UDP*/

        }
        else
        {
            dsl3editv6.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE;
        }

        /* SYSTEM MODIFIED for hybrid v4/v6 exchange */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL ))
        {
            dsl3editv6.ip_protocol_type = p_nh_param->tunnel_info.openflow_ip_protocol;
        }

        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                                               &(p_nh_param->tunnel_info.ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
        dsl3editv6.ipsa_index = sa_index & 0x1F;
        dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NORMAL;
        break;

    case CTC_TUNNEL_TYPE_IPV6_IN6:
        dsl3editv6.ip_protocol_type = SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE;
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                                               &(p_nh_param->tunnel_info.ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
        dsl3editv6.ipsa_index = sa_index & 0x1F;
        dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NORMAL;
        break;

    case CTC_TUNNEL_TYPE_GRE_IN6:
        dsl3editv6.inner_header_valid   = 1;
        dsl3editv6.gre_protocol         = p_nh_param->tunnel_info.gre_info.protocol_type;
        dsl3editv6.gre_version     = p_nh_param->tunnel_info.gre_info.gre_ver & 0x1;
        if (!p_nh_param->tunnel_info.gre_info.protocol_type)
        {
            dsl3editv6.gre_protocol = 0x8847;
            dsl3editv6.is_gre_auto = 1;
        }
        if (p_nh_param->tunnel_info.gre_info.protocol_type == 0x6558)
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        }

        /*Encapsulate Xerspan Extend header */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR))
        {
            dsl3editv6.xerspan_hdr_en = 1;

            if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR_HASH))
            {
                dsl3editv6.xerspan_hdr_with_hash_en = 1;
            }
        }

        /* gre with key */
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY))
        {
            dsl3editv6.inner_header_type = 0x1;
            dsl3editv6.gre_flags = 0x2;
            dsl3editv6.gre_key= p_nh_param->tunnel_info.gre_info.gre_key;
        }
        else /* gre without key */
        {
            dsl3editv6.inner_header_type = 0x0;
        }

        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                                &(p_nh_param->tunnel_info.ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
        dsl3editv6.ipsa_index = sa_index & 0x1F;
        dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_GRE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
        }
        break;

    case CTC_TUNNEL_TYPE_NVGRE_IN6:
        dsl3editv6.inner_header_valid   = 1;
        /*dsl3editv6.gre_protocol     = p_nh_param->tunnel_info.gre_info.protocol_type;*/
        /*dsl3editv6.gre_version = p_nh_param->tunnel_info.gre_info.gre_ver & 0x1;*/

        dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
           }
        /* gre with key */
        dsl3editv6.inner_header_type = 0x1;
        dsl3editv6.gre_flags = 0x2;

        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                              &(p_nh_param->tunnel_info.ip_sa.ipv6), &sa_index, p_edit_db->sa_index));
        dsl3editv6.ipsa_index = sa_index & 0xF;

        dsl3editv6.ipda[0] = p_nh_param->tunnel_info.ip_da.ipv6[3];
        dsl3editv6.ipda[1] = p_nh_param->tunnel_info.ip_da.ipv6[2];
        dsl3editv6.ipda[2] = p_nh_param->tunnel_info.ip_da.ipv6[1];
        dsl3editv6.ipda[3] = p_nh_param->tunnel_info.ip_da.ipv6[0];
        dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NVGRE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
        {
            dsl3editv6.out_l2edit_valid = 1;
            dsl3editv6.l2edit_ptr = dsnh->l2edit_ptr;
        }
        break;
    case CTC_TUNNEL_TYPE_GENEVE_IN6: /* almost as vxlan*/
        dsl3editv6.is_geneve = 1;
    case CTC_TUNNEL_TYPE_VXLAN_IN6:
        dsl3editv6.inner_header_valid   = 1;
        dsl3editv6.inner_header_type = 0x2; /* 2'b10: 8 byte UDP header */
        dsl3editv6.ipda[0] = p_nh_param->tunnel_info.ip_da.ipv6[3];
        dsl3editv6.ipda[1] = p_nh_param->tunnel_info.ip_da.ipv6[2];
        dsl3editv6.ipda[2] = p_nh_param->tunnel_info.ip_da.ipv6[1];
        dsl3editv6.ipda[3] = p_nh_param->tunnel_info.ip_da.ipv6[0];
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                                  &(p_nh_param->tunnel_info.ip_sa.ipv6), &sa_index, p_edit_db->sa_index));

        dsl3editv6.ipsa_index = sa_index & 0xF;
        dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
        {
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN;
           }
        dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_VXLAN;
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
        {
            dsl3editv6.out_l2edit_valid = 1;
            dsl3editv6.l2edit_ptr = dsnh->l2edit_ptr;
        }
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE))
        {
            dsl3editv6.is_evxlan = 1;
            if (PKT_TYPE_ETH == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3editv6.evxlan_protocl_index =  2;
            }
            else if (PKT_TYPE_IPV4 == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3editv6.evxlan_protocl_index =  0;
            }
            else if (PKT_TYPE_IPV6 == p_nh_param->tunnel_info.inner_packet_type)
            {
                dsl3editv6.evxlan_protocl_index =  1;
            }
            else
            {
                if (PKT_TYPE_FLEXIBLE == p_nh_param->tunnel_info.inner_packet_type)
                {
                    dsl3editv6.is_vxlan_auto = 1;
                }
                else
                {
                    dsl3editv6.evxlan_protocl_index =  3;
                }
            }
        }
        else if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN))
        {
            /*using rsv profile 1 as GBP encap*/
            dsl3editv6.is_geneve = 0;
            dsl3editv6.evxlan_protocl_index = 1;
        }
        if (CTC_FLAG_ISSET(p_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT))
        {
            dsl3editv6.udp_src_port_en = 1;
            dsl3editv6.l4_src_port = p_nh_param->tunnel_info.l4_src_port;
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if ((p_nh_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV4_IN6) || (p_nh_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV6_IN6))
    {
        if (p_nh_param->tunnel_info.l4_src_port && p_nh_param->tunnel_info.l4_dst_port)
        {
            /*UDP tunnel enable*/
            dsl3editv6.inner_header_valid   = 1;
            dsl3editv6.inner_header_type = 0x2; /* 2'b10: 8 byte UDP header */
            dsl3editv6.l4_dest_port = p_nh_param->tunnel_info.l4_dst_port;
            dsl3editv6.l4_src_port = p_nh_param->tunnel_info.l4_src_port;
            dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
        }
    }

    p_dsl3editv6 = &dsl3editv6;
    CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3editv6, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, &dsnh->l3edit_ptr));


    p_edit_db->p_l3edit = p_dsl3editv6;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IN_V6;
    p_edit_db->sa_index = sa_index;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_nat_v4(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_nat4w_t  dsl3edit4w;
    sys_dsl3edit_nat4w_t*  p_dsl3edit4w;
    int32 ret = 0;

    sal_memset(&dsl3edit4w, 0, sizeof(dsl3edit4w));

    dsl3edit4w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_NAT_4W;
    dsl3edit4w.ds_type = SYS_NH_DS_TYPE_L3EDIT;
    dsl3edit4w.ipda = p_nh_param->tunnel_info.ip_da.ipv4;
    dsl3edit4w.replace_ip_da = 1;

    /* for NAPT */
    if (p_nh_param->tunnel_info.l4_dst_port)
    {
        dsl3edit4w.replace_l4_dest_port = 1;
        dsl3edit4w.l4_dest_port = p_nh_param->tunnel_info.l4_dst_port;
    }

    p_dsl3edit4w = &dsl3edit4w;
    ret = sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3edit4w, SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W, &dsnh->l3edit_ptr);
    if (ret)
    {
        return ret;
    }

    p_edit_db->p_l3edit = p_dsl3edit4w;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_NAT_V4;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_ivi_6to4(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_nat4w_t  dsl3edit4w;
    sys_dsl3edit_nat4w_t*  p_dsl3edit4w;
    int32 ret = 0;

    CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.ivi_info.prefix_len, (MAX_CTC_NH_IVI_PREFIX_LEN-1));
    CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.ivi_info.is_rfc6052, 1);

    sal_memset(&dsl3edit4w, 0, sizeof(dsl3edit4w));

    dsl3edit4w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_NAT_4W;
    dsl3edit4w.ds_type = SYS_NH_DS_TYPE_L3EDIT;
    dsl3edit4w.ip_da_prefix_length = p_nh_param->tunnel_info.ivi_info.prefix_len;
    dsl3edit4w.ipv4_embeded_mode = p_nh_param->tunnel_info.ivi_info.is_rfc6052;

    p_dsl3edit4w = &dsl3edit4w;
    ret = sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3edit4w, SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W, &dsnh->l3edit_ptr);
    if (ret)
    {
        return ret;
    }

    p_edit_db->p_l3edit = p_dsl3edit4w;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit_ivi_4to6(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                              sys_nh_info_ip_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_nat8w_t  dsl3edit8w;
    sys_dsl3edit_nat8w_t*  p_dsl3edit8w;
    uint8 prefix_len[8] = {32, 40, 48, 56, 64, 72, 80, 96};

    CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.ivi_info.prefix_len, (MAX_CTC_NH_IVI_PREFIX_LEN-1));
    CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.ivi_info.is_rfc6052, 1);

    sal_memset(&dsl3edit8w, 0, sizeof(dsl3edit8w));

    INTERCEPT_IPV6_ADDR(p_nh_param->tunnel_info.ivi_info.ipv6, prefix_len[p_nh_param->tunnel_info.ivi_info.prefix_len]);

    dsl3edit8w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_NAT_8W;
    dsl3edit8w.ds_type = SYS_NH_DS_TYPE_L3EDIT;
    dsl3edit8w.ip_da_prefix_length = p_nh_param->tunnel_info.ivi_info.prefix_len;
    dsl3edit8w.ipv4_embeded_mode = p_nh_param->tunnel_info.ivi_info.is_rfc6052;

    /* ipda, use little india for DRV_SET_FIELD_A */
    dsl3edit8w.ipda[0] = p_nh_param->tunnel_info.ivi_info.ipv6[3];
    dsl3edit8w.ipda[1] = p_nh_param->tunnel_info.ivi_info.ipv6[2];
    dsl3edit8w.ipda[2] = p_nh_param->tunnel_info.ivi_info.ipv6[1];
    dsl3edit8w.ipda[3] = p_nh_param->tunnel_info.ivi_info.ipv6[0];

    p_dsl3edit8w = &dsl3edit8w;
    CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3edit8w, SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W, &dsnh->l3edit_ptr));

    p_edit_db->p_l3edit = p_dsl3edit8w;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl3edit(uint8 lchip, ctc_ip_tunnel_nh_param_t* p_nh_param,
                                           sys_nh_info_ip_tunnel_t* p_edit_db,
                                           sys_nh_param_dsnh_t* dsnh)
{

    CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_prepare(lchip, p_nh_param, p_edit_db, dsnh));

    switch (p_nh_param->tunnel_info.tunnel_type)
    {
    case CTC_TUNNEL_TYPE_IPV4_IN4:
    case CTC_TUNNEL_TYPE_IPV6_IN4:
    case CTC_TUNNEL_TYPE_GRE_IN4:
    case CTC_TUNNEL_TYPE_ISATAP:
    case CTC_TUNNEL_TYPE_6TO4:
    case CTC_TUNNEL_TYPE_6RD:
    case CTC_TUNNEL_TYPE_NVGRE_IN4:
    case CTC_TUNNEL_TYPE_VXLAN_IN4:
    case CTC_TUNNEL_TYPE_GENEVE_IN4:
    case CTC_TUNNEL_TYPE_NONE:
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_v4(lchip, p_nh_param, p_edit_db, dsnh));
        break;

    case CTC_TUNNEL_TYPE_IPV4_IN6:
    case CTC_TUNNEL_TYPE_IPV6_IN6:
    case CTC_TUNNEL_TYPE_GRE_IN6:
    case CTC_TUNNEL_TYPE_NVGRE_IN6:
    case CTC_TUNNEL_TYPE_VXLAN_IN6:
    case CTC_TUNNEL_TYPE_GENEVE_IN6:
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_v6(lchip, p_nh_param, p_edit_db, dsnh));
        break;

    case CTC_TUNNEL_TYPE_IPV4_NAT:
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_nat_v4(lchip, p_nh_param, p_edit_db, dsnh));
        break;

    case CTC_TUNNEL_TYPE_IVI_6TO4:
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_ivi_6to4(lchip, p_nh_param, p_edit_db, dsnh));
        break;

    case CTC_TUNNEL_TYPE_IVI_4TO6:
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_build_dsl3edit_ivi_4to6(lchip, p_nh_param, p_edit_db, dsnh));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsl2edit(uint8 lchip, sys_nh_param_ip_tunnel_t* p_nh_param,
                                           sys_nh_info_ip_tunnel_t* p_edit_db,
                                           sys_nh_param_dsnh_t* dsnh)
{
    sys_nh_db_dsl2editeth4w_t   dsl2edit;
    sys_nh_db_dsl2editeth4w_t   dsl2edit_swap;
    sys_nh_db_dsl2editeth4w_t* p_dsl2edit;
    sys_nh_db_dsl2editeth8w_t   dsl2edit6w;
    sys_nh_db_dsl2editeth8w_t   dsl2edit8w;
    sys_nh_db_dsl2editeth8w_t*  p_dsl2edit6w;
    sys_nh_db_dsl2editeth4w_t   dsl2edit3w;
    mac_addr_t mac = {0};
    ctc_ip_tunnel_nh_param_t* p_ip_nh_param;
    uint16 ether_type = 0;
    uint8 gint_en = 0;

    sal_memset(&dsl2edit, 0 , sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit_swap, 0 , sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit6w, 0 , sizeof(sys_nh_db_dsl2editeth8w_t));
    sal_memset(&dsl2edit8w, 0 , sizeof(sys_nh_db_dsl2editeth8w_t));
    sal_memset(&dsl2edit3w, 0 , sizeof(sys_nh_db_dsl2editeth4w_t));

    sys_usw_global_get_gint_en(lchip, &gint_en);

    p_ip_nh_param = p_nh_param->p_ip_nh_param;
    if ((CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR)
         &&(0 == p_ip_nh_param->loop_nhid) && (0 == p_ip_nh_param->tunnel_info.dot1ae_chan_id)) || gint_en)
    {
        dsnh->l2edit_ptr = 0;
        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR)
        && (0 != p_ip_nh_param->tunnel_info.dot1ae_chan_id))
    {
        p_ip_nh_param->mac[0] = 0x0;
        p_ip_nh_param->mac[1] = 0xe;
        p_ip_nh_param->mac[2] = 0x0;
        p_ip_nh_param->mac[3] = 0xe;
        p_ip_nh_param->mac[4] = 0x0;
        p_ip_nh_param->mac[5] = 0xe;
    }

    if (p_edit_db->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_USE_ECMP_IF)
    {
        dsl2edit.is_ecmp_if = 1;
    }

    if (p_ip_nh_param->tunnel_info.udf_profile_id)
    {
        ether_type = g_usw_nh_master[lchip]->udf_ether_type[p_ip_nh_param->tunnel_info.udf_profile_id-1];
    }

    if (( SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_ip_nh_param) || CTC_TUNNEL_TYPE_GRE_IN4 == p_ip_nh_param->tunnel_info.tunnel_type)
        && CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
    {
        if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA)) /* inner L2EthAdd */
        {
            if (0 == sal_memcmp(p_ip_nh_param->tunnel_info.inner_macsa, mac, 6))
            {
                sal_memcpy(&dsl2edit3w, &dsl2edit, sizeof(sys_nh_db_dsl2editeth4w_t));
                sal_memcpy(dsl2edit3w.mac_da, p_ip_nh_param->tunnel_info.inner_macda, sizeof(mac_addr_t));
                if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
                {
                    dsl2edit3w.strip_inner_vlan = 1;
                }
                p_dsl2edit = &dsl2edit3w;
                CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_3w_inner(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_dsl2edit,
                                    SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, &dsnh->inner_l2edit_ptr));
                CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_3W);
                p_edit_db->p_dsl2edit_inner = p_dsl2edit;
                p_edit_db->inner_l2_edit_offset = dsnh->inner_l2edit_ptr;
            }
            else
            {
                sal_memcpy(dsl2edit6w.mac_da, p_ip_nh_param->tunnel_info.inner_macda, sizeof(mac_addr_t));
                sal_memcpy(dsl2edit6w.mac_sa, p_ip_nh_param->tunnel_info.inner_macsa, sizeof(mac_addr_t));
                dsl2edit6w.update_mac_sa = 1;
                if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
                {
                    dsl2edit6w.strip_inner_vlan = 1;
                }
                p_dsl2edit6w = &dsl2edit6w;
                CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_6w_inner(lchip, (sys_nh_db_dsl2editeth8w_t**)&p_dsl2edit6w,
                                    &dsnh->inner_l2edit_ptr));
                CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_6W);
                p_edit_db->p_dsl2edit_inner = p_dsl2edit6w;
                p_edit_db->inner_l2_edit_offset = dsnh->inner_l2edit_ptr;
            }
        }
        else  /* inner L2EthSwap */
        {
            sal_memcpy(&dsl2edit_swap, &dsl2edit, sizeof(sys_nh_db_dsl2editeth4w_t));
            sal_memcpy(dsl2edit_swap.mac_da, p_ip_nh_param->tunnel_info.inner_macda, sizeof(mac_addr_t));
            if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN))
            {
                dsl2edit_swap.strip_inner_vlan = 1;
            }
            p_dsl2edit = &dsl2edit_swap;
            CTC_ERROR_RETURN(sys_usw_nh_add_swap_l2edit_inner(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_dsl2edit,
                                &dsnh->inner_l2edit_ptr));
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT);
            p_edit_db->p_dsl2edit_inner = p_dsl2edit;
            p_edit_db->inner_l2_edit_offset = dsnh->inner_l2edit_ptr;
        }
    }

    /*outer l2 edit*/
    if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR)
        && (p_ip_nh_param->loop_nhid))
    {
        CTC_ERROR_RETURN(sys_usw_nh_add_loopback_l2edit(lchip, p_ip_nh_param->loop_nhid, TRUE, &dsnh->l2edit_ptr));
        p_edit_db->loop_nhid = p_ip_nh_param->loop_nhid;
        CTC_SET_FLAG(p_edit_db->flag, SYS_NH_IP_TUNNEL_LOOP_NH);
    }
    else if ((0 == sal_memcmp(p_ip_nh_param->mac_sa, mac, sizeof(mac_addr_t))) && !ether_type)
    {
        if (p_ip_nh_param->arp_id)
        {
            sys_nh_db_arp_t *p_arp = NULL;
            sys_nh_info_arp_param_t arp_parm;
            uint32 offset = 0;

            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_nh_param->arp_id:%d\n", p_ip_nh_param->arp_id);

            if (dsl2edit.is_ecmp_if)
            {
                sys_usw_nh_get_emcp_if_l2edit_offset(lchip, &offset);
                p_edit_db->outer_l2_edit_offset = offset;
                dsnh->l2edit_ptr = offset;
                return CTC_E_NONE;
            }

            sal_memset(&arp_parm, 0, sizeof(sys_nh_info_arp_param_t));
            arp_parm.nh_entry_type  = SYS_NH_TYPE_IP_TUNNEL;
            CTC_ERROR_RETURN(sys_usw_nh_bind_arp_cb(lchip, &arp_parm, p_ip_nh_param->arp_id));

            p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_ip_nh_param->arp_id));
            if (NULL == p_arp)
            {
                return CTC_E_NOT_EXIST;
            }
            dsnh->l2edit_ptr = p_arp->offset;
            p_edit_db->arp_id = p_ip_nh_param->arp_id;
            p_edit_db->outer_l2_edit_offset = dsnh->l2edit_ptr;
        }
        else
        {
            sal_memcpy(dsl2edit.mac_da, p_ip_nh_param->mac, sizeof(mac_addr_t));
            dsl2edit.output_vid =  p_ip_nh_param->oif.vid;
            p_dsl2edit = &dsl2edit;
            CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_dsl2edit, &dsnh->l2edit_ptr));
            p_edit_db->p_dsl2edit_4w = p_dsl2edit;
            p_edit_db->outer_l2_edit_offset = dsnh->l2edit_ptr;
        }
    }
    else
    {
        sal_memcpy(dsl2edit8w.mac_da, p_ip_nh_param->mac, sizeof(mac_addr_t));
        sal_memcpy(dsl2edit8w.mac_sa, p_ip_nh_param->mac_sa, sizeof(mac_addr_t));
        dsl2edit8w.output_vid =  p_ip_nh_param->oif.vid;
        dsl2edit8w.ether_type = ether_type;
        if (sal_memcmp(p_ip_nh_param->mac_sa, mac, sizeof(mac_addr_t)))
        {
            dsl2edit8w.update_mac_sa = 1;
        }
        p_dsl2edit = (sys_nh_db_dsl2editeth4w_t*)&dsl2edit8w;
        CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, (sys_nh_db_dsl2editeth8w_t**)&p_dsl2edit, &dsnh->l2edit_ptr));

        CTC_SET_FLAG(p_edit_db->flag , SYS_NH_IP_TUNNEL_FLAG_L2EDIT8W);
        p_edit_db->p_dsl2edit_4w = p_dsl2edit;
        p_edit_db->outer_l2_edit_offset = dsnh->l2edit_ptr;
    }


    return CTC_E_NONE;
}

int32
_sys_usw_nh_free_ip_tunnel_resource(uint8 lchip, sys_nh_info_ip_tunnel_t* p_nhinfo, uint8 is_update)
{

    uint8 gchip = 0;
    uint8 lchip2 = 0;
    uint8 l3edit_type = 0;
    sys_nh_db_dsl2editeth4w_t  dsl2edit;
    sys_nh_db_dsl2editeth4w_t  dsl2edit_swap;
    sys_nh_db_dsl2editeth8w_t  dsl2edit_8w;

    sal_memset(&dsl2edit, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit_swap, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));

    if (!CTC_FLAG_ISSET(p_nhinfo->flag, SYS_NH_IP_TUNNEL_REROUTE))
    {
        gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nhinfo->gport);
        sys_usw_get_local_chip_id(gchip, &lchip2);
    }


    if (CTC_FLAG_ISSET(p_nhinfo->flag, SYS_NH_IP_TUNNEL_REROUTE))
    {
        lchip2 = lchip;

    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        if (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IN_V4)
        {
            l3edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4;
        }
        else if (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IN_V6)
        {
            l3edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6;
        }
        else if ((p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4)
            || (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_NAT_V4))
        {
            l3edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W;
        }
        else if (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6)
        {
            l3edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W;
        }

        sys_usw_nh_remove_l3edit_tunnel(lchip, p_nhinfo->p_l3edit, l3edit_type);

        if (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IN_V4)
        {
            _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_4, p_nhinfo->sa_index);
        }
        else if (p_nhinfo->flag & SYS_NH_IP_TUNNEL_FLAG_IN_V6)
        {
            _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_6, p_nhinfo->sa_index);
        }

        /*unbind dot1ae channel*/
        if ((p_nhinfo->dot1ae_en)&&(!is_update))
        {
            sys_usw_register_dot1ae_bind_sc_t bind_sc;
            sal_memset(&bind_sc,0,sizeof(bind_sc));
            bind_sc.type = 1;
            bind_sc.dir = 0;
            bind_sc.gport = p_nhinfo->hdr.nh_id;

            bind_sc.sc_index= (p_nhinfo->sc_index>>2);
            if (REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
            {
                CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip,&bind_sc));
            }
            p_nhinfo->dot1ae_en = 0;
        }

        /*unbind udf edit profile*/
        if (p_nhinfo->udf_profile_id)
        {
            if (g_usw_nh_master[lchip]->udf_profile_ref_cnt[p_nhinfo->udf_profile_id-1])
            {
                g_usw_nh_master[lchip]->udf_profile_ref_cnt[p_nhinfo->udf_profile_id-1]--;
            }
            p_nhinfo->udf_profile_id = 0;
        }
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
    {
        if (CTC_FLAG_ISSET(p_nhinfo->flag, SYS_NH_IP_TUNNEL_LOOP_NH))
        {
            sys_usw_nh_remove_loopback_l2edit(lchip, p_nhinfo->loop_nhid, TRUE);
        }
        else if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_ECMP_IF))
        {
            if (CTC_FLAG_ISSET(p_nhinfo->flag , SYS_NH_IP_TUNNEL_FLAG_L2EDIT8W))
            {
                sys_usw_nh_remove_route_l2edit_8w_outer(lchip, (sys_nh_db_dsl2editeth8w_t*)(p_nhinfo->p_dsl2edit_4w));
            }
            else if(!p_nhinfo->arp_id)
            {
                sys_usw_nh_remove_route_l2edit_outer(lchip, p_nhinfo->p_dsl2edit_4w);
            }
        }
    }

    if (p_nhinfo->arp_id)
    {
        sys_usw_nh_unbind_arp_cb(lchip, p_nhinfo->arp_id, 0);
        p_nhinfo->arp_id = 0;
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
    {

        sys_usw_nh_remove_l2edit_6w_inner(lchip, (sys_nh_db_dsl2editeth8w_t*)p_nhinfo->p_dsl2edit_inner);
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W))
    {
        sys_usw_nh_remove_l2edit_3w_inner(lchip, (sys_nh_db_dsl2editeth4w_t*)p_nhinfo->p_dsl2edit_inner, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W);
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT))
    {
        sys_usw_nh_remove_swap_l2edit_inner(lchip, (sys_nh_db_dsl2editeth4w_t*)p_nhinfo->p_dsl2edit_inner);
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_ROUTE_OVERLAY))
    {
        _sys_usw_nh_vxlan_remove_vni(lchip,  p_nhinfo->vn_id);
    }

    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_ip_tunnel_build_dsnh(uint8 lchip, sys_nh_param_ip_tunnel_t* p_nh_param,
                                       sys_nh_info_ip_tunnel_t* p_edit_db,
                                       sys_nh_param_dsfwd_t * p_fwd_info)
{
    uint8 gchip, lchip2;
    sys_nh_param_dsnh_t dsnh_param;
    sys_l3if_prop_t l3if_prop;
    sys_l3if_ecmp_if_t ecmp_if = {0};
    uint16 gport = 0;
    uint16 lport = 0;
    int32 ret = CTC_E_NONE;
    uint8 port_ext_mc_en = 0;
    uint8 is_route_overlay = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;
    ctc_ip_tunnel_nh_param_t* p_ip_nh_param;
    uint8 is_update = (p_nh_param->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);
    uint8 gint_en = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_nh_param);
    CTC_PTR_VALID_CHECK(p_edit_db);

    lchip2 = 0;
    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));
    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    if (!is_update)
    {
        p_edit_db->sa_index = 0xFF;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    p_ip_nh_param = p_nh_param->p_ip_nh_param;

    if (!CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR))
    {
        if (p_ip_nh_param->oif.ecmp_if_id > 0)
        {
            CTC_ERROR_RETURN(sys_usw_l3if_get_ecmp_if_info(lchip, p_ip_nh_param->oif.ecmp_if_id, &ecmp_if));
            p_edit_db->ecmp_if_id = p_ip_nh_param->oif.ecmp_if_id;
            p_edit_db->hdr.nh_entry_flags |= SYS_NH_INFO_FLAG_USE_ECMP_IF;

            /*use port mapping interface id*/
            if (DRV_IS_DUET2(lchip))
            {
                l3if_prop.vlan_ptr = 0x1400;
            }
            else
            {
                l3if_prop.vlan_ptr = 0x1000;
            }
        }
        else
        {
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_ip_nh_param->oif.gport);
            sys_usw_get_local_chip_id(gchip, &lchip2);
            gport = p_ip_nh_param->oif.gport;
            if (!(CTC_IS_CPU_PORT(p_ip_nh_param->oif.gport) || p_ip_nh_param->oif.is_l2_port || p_nh_param->hdr.is_drop))
            {
                CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, gport, p_ip_nh_param->oif.vid, &l3if_prop))
                p_edit_db->l3ifid  = l3if_prop.l3if_id;
            }
            else
            {
                l3if_prop.vlan_ptr = 0xFFFF;
            }
            p_edit_db->gport = gport;
        }
    }

        if (p_ip_nh_param->tunnel_info.adjust_length != 0)
        {
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN);
            p_edit_db->hdr.adjust_len = p_ip_nh_param->tunnel_info.adjust_length;
        }

        if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_HOVERLAY))
        {
            CTC_MAX_VALUE_CHECK(p_ip_nh_param->tunnel_info.logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_PORT);
            p_edit_db->dest_logic_port = p_ip_nh_param->tunnel_info.logic_port;
            CTC_UNSET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);
            dsnh_param.hvpls = 1;/*use for tunnel split horizon*/
        }
        else
        {
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);
            dsnh_param.hvpls = 0;/*use for tunnel split horizon*/
        }
        /*if need loopback, get egress loopback port*/
        if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR))
        {
            if (p_ip_nh_param->oif.ecmp_if_id > 0)
            {
                /* if output port is ecmp interface, can not do reroute */
                return CTC_E_INVALID_PARAM;
            }

            CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_IP_TUNNEL, &lport));

            if (SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_ip_nh_param))
            {
                CTC_ERROR_RETURN(sys_usw_internal_port_set_overlay_property(lchip, lport));
            }

            sys_usw_get_gchip_id(lchip, &gchip);
            lchip2 = lchip;
            p_ip_nh_param->oif.gport = lport; /*only use lport (IP tunnel i/e loop port id). gchip_id will write later. */
            p_edit_db->gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
            p_ip_nh_param->oif.vid = 0;
            l3if_prop.vlan_ptr = MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID) + 0x1000;
            CTC_SET_FLAG(p_edit_db->flag, SYS_NH_IP_TUNNEL_REROUTE);
            if ((CTC_TUNNEL_TYPE_IPV4_NAT == p_ip_nh_param->tunnel_info.tunnel_type)
                || (CTC_TUNNEL_TYPE_IVI_6TO4 == p_ip_nh_param->tunnel_info.tunnel_type)
                || (CTC_TUNNEL_TYPE_IVI_4TO6 == p_ip_nh_param->tunnel_info.tunnel_type))
            {
                dsnh_param.flag |= SYS_NH_PARAM_FLAG_ROUTE_NOTTL;
            }
        }
        else
        {
            CTC_UNSET_FLAG(p_edit_db->flag, SYS_NH_IP_TUNNEL_REROUTE);
        }

        switch (p_ip_nh_param->tunnel_info.tunnel_type)
        {
        case CTC_TUNNEL_TYPE_IPV4_IN4:
        case CTC_TUNNEL_TYPE_IPV6_IN4:
        case CTC_TUNNEL_TYPE_GRE_IN4:
        case CTC_TUNNEL_TYPE_ISATAP:
        case CTC_TUNNEL_TYPE_6TO4:
        case CTC_TUNNEL_TYPE_6RD:
        case CTC_TUNNEL_TYPE_NVGRE_IN4:
        case CTC_TUNNEL_TYPE_VXLAN_IN4:
        case CTC_TUNNEL_TYPE_GENEVE_IN4:
            p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IN_V4;
            break;

        case CTC_TUNNEL_TYPE_IPV4_IN6:
        case CTC_TUNNEL_TYPE_IPV6_IN6:
        case CTC_TUNNEL_TYPE_GRE_IN6:
        case CTC_TUNNEL_TYPE_NVGRE_IN6:
        case CTC_TUNNEL_TYPE_VXLAN_IN6:
        case CTC_TUNNEL_TYPE_GENEVE_IN6:
            p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IN_V6;
            break;

        case CTC_TUNNEL_TYPE_IPV4_NAT:
            p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_NAT_V4;
            break;

        case CTC_TUNNEL_TYPE_IVI_6TO4:
            p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4;
            break;

        case CTC_TUNNEL_TYPE_IVI_4TO6:
            p_edit_db->flag |= SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6;
            break;
        case CTC_TUNNEL_TYPE_NONE:
            /*for udf edit*/
            sys_usw_global_get_gint_en(lchip, &gint_en);
            if (gint_en)
            {
                dsnh_param.flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

        dsnh_param.dest_vlan_ptr = l3if_prop.vlan_ptr;
        p_edit_db->dest_vlan_ptr = dsnh_param.dest_vlan_ptr;


        /*1. Build dsL2Edit*/
        ret = _sys_usw_nh_ip_tunnel_build_dsl2edit(lchip, p_nh_param, p_edit_db, &dsnh_param);
        if (ret)
        {
           goto error_proc;
        }

        if ((dsnh_param.l2edit_ptr != 0))
        {
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
        }

        /*2. Build dsL3Edit*/
         ret = _sys_usw_nh_ip_tunnel_build_dsl3edit(lchip, p_ip_nh_param, p_edit_db, &dsnh_param);
        if (ret)
        {
           goto error_proc;
        }

        if (dsnh_param.l3edit_ptr != 0)
        {
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
        }

        if (p_ip_nh_param->tunnel_info.flag & CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR)
        {
            dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL;
        }
        else if (p_ip_nh_param->tunnel_info.flag & CTC_IP_NH_TUNNEL_FLAG_MIRROR)
        {
            dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL_FOR_MIRROR;
        }
        else
        {
            dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL;
        }

        if (p_ip_nh_param->tunnel_info.flag & CTC_IP_NH_TUNNEL_FLAG_INNER_TTL_NO_DEC)
        {
            CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL);
        }

        dsnh_param.logic_port = p_ip_nh_param->tunnel_info.logic_port;
        dsnh_param.logic_port_check = CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_LOGIC_PORT_CHECK)?1:0;

        _sys_usw_nh_ip_tunnel_get_port_extend_mcast_en(lchip, &port_ext_mc_en);


       is_route_overlay = ((SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_ip_nh_param)|| p_ip_nh_param->tunnel_info.tunnel_type)/*overlay*/
                &&((CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))/*L2 overlay*/
            || (PKT_TYPE_IPV4 == p_ip_nh_param->tunnel_info.inner_packet_type )/*L3 overlay*/
            || (PKT_TYPE_IPV6 == p_ip_nh_param->tunnel_info.inner_packet_type ) ));/*L3 overlay*/

       if (CTC_FLAG_ISSET(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
       {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lchip = %d, Write DsNexthop8w Table\n", lchip);
           p_edit_db->vn_id = p_ip_nh_param->tunnel_info.vn_id;
           if (is_route_overlay)
           {
                CTC_UNSET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE);

                if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA)
                || (PKT_TYPE_ETH != p_ip_nh_param->tunnel_info.inner_packet_type)) /* is_vxlan_route_op = 0*/
                {
                    CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_INNER);
                }
                CTC_ERROR_GOTO(_sys_usw_nh_vxlan_add_vni(lchip,  p_ip_nh_param->tunnel_info.vn_id, &dsnh_param.fid), ret, error_proc);
                CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_ROUTE_OVERLAY);
           }
           else
           {
               CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_INNER);
               if ((SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_ip_nh_param) && port_ext_mc_en)
                   || (0 != p_ip_nh_param->tunnel_info.vn_id ))
               {
                    CTC_ERROR_GOTO(_sys_usw_nh_vxlan_add_vni(lchip, p_ip_nh_param->tunnel_info.vn_id, &(dsnh_param.fid)), ret, error_proc);
                    CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_ROUTE_OVERLAY);
               }
           }

           if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR))
           {
               dsnh_param.span_id = p_ip_nh_param->tunnel_info.span_id;
               p_edit_db->span_id = dsnh_param.span_id;
               CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_ERSPAN_EN);

               if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_KEEP_IGS_TS))
               {
                   CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_ERSPAN_KEEP_IGS_TS);
                   CTC_SET_FLAG(p_edit_db->flag, SYS_NH_IP_TUNNEL_KEEP_IGS_TS);
               }
           }

           dsnh_param.dsnh_offset = p_edit_db->hdr.dsfwd_info.dsnh_offset;
           ret = sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param);
           p_fwd_info->nexthop_ext = 1;
       }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lchip = %d, Write DsNexthop4w Table\n", lchip);
            dsnh_param.dsnh_offset = p_edit_db->hdr.dsfwd_info.dsnh_offset;
            dsnh_param.loop_edit = CTC_FLAG_ISSET(p_edit_db->flag, SYS_NH_IP_TUNNEL_LOOP_NH) ? 1 : 0;
            ret = sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param);
        }

        if (ret)
        {
           goto error_proc;
        }

   return CTC_E_NONE;
error_proc:
     return ret;

}
STATIC int32
_sys_usw_nh_ip_tunnel_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
    sys_nh_info_ip_tunnel_t* p_nh_info = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = CTC_E_NONE;

    p_nh_info     = (sys_nh_info_ip_tunnel_t*)(p_com_db);

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);

    if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_REROUTE))
    {
        sys_usw_get_gchip_id(lchip, &dsfwd_param.dest_chipid);
    }
    else
    {
        dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_info->gport);
    }

    if (p_nh_info->arp_id)
    {
        sys_nh_db_arp_t *p_arp = NULL;
        p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_nh_info->arp_id));
        if (NULL == p_arp)
        {
            return CTC_E_NOT_EXIST;
        }
        dsfwd_param.dest_id = p_arp->destmap_profile;
        dsfwd_param.use_destmap_profile = 1;
    }
    else
    {
        dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_info->gport);
    }

    if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN))
    {
        sys_usw_lkup_adjust_len_index(lchip, p_nh_info->hdr.adjust_len, &dsfwd_param.adjust_len_idx);
    }

    dsfwd_param.cut_through_dis = 1;/*disable cut through for non-ip packet when encap ip-tunnel*/
    dsfwd_param.drop_pkt = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
    dsfwd_param.cloud_sec_en = p_nh_info->dot1ae_en;

    /*2.2 Build DsFwd Table*/

    if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {

        ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                             &(p_nh_info->hdr.dsfwd_info.dsfwd_offset));

    }
    dsfwd_param.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.dsnh_offset = p_nh_info->hdr.dsfwd_info.dsnh_offset;
    dsfwd_param.is_egress_edit =  CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);

     /*2.3 Write table*/
    ret = ret ? ret : sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param);
    CTC_SET_FLAG(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);

    if (ret != CTC_E_NONE)
    {
      sys_usw_nh_offset_free(lchip,  SYS_NH_ENTRY_TYPE_FWD, 1, p_nh_info->hdr.dsfwd_info.dsfwd_offset);

    }
    else
    {
        CTC_SET_FLAG(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsFwd(Lchip : %d) :: DestChipId = %d, DestId = %d"
                                         "DsNexthop Offset = %d, DsNexthopExt = %d\n", 0,
                                          dsfwd_param.dest_chipid, dsfwd_param.dest_id,
                                          dsfwd_param.dsnh_offset, dsfwd_param.nexthop_ext);

    return ret;
}
int32
sys_usw_nh_create_ip_tunnel_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db)
{

    sys_nh_param_ip_tunnel_t* p_nh_param = NULL;
    ctc_ip_tunnel_nh_param_t* p_nh_ip_param = NULL;
    sys_nh_info_ip_tunnel_t* p_nhdb = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = 0;
    uint8 port_ext_mc_en = 0;

    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db)
    CTC_EQUAL_CHECK(SYS_NH_TYPE_IP_TUNNEL, p_com_nh_para->hdr.nh_param_type);
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    p_nh_param = (sys_nh_param_ip_tunnel_t*)(p_com_nh_para);
    p_nh_ip_param = p_nh_param->p_ip_nh_param;
    p_nhdb     = (sys_nh_info_ip_tunnel_t*)(p_com_db);
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_IP_TUNNEL;


    CTC_MAX_VALUE_CHECK(p_nh_ip_param->tunnel_info.ecn_select, (MAX_CTC_NH_ECN_SELECT_MODE-1));

    /* if need logic port do range check*/
    if (p_nh_ip_param->tunnel_info.logic_port && (p_nh_ip_param->tunnel_info.logic_port > 0x3FFF))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");
		return CTC_E_INVALID_PORT;
    }

    /* span id range check*/
    if (CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR)
        && (p_nh_ip_param->tunnel_info.span_id > 1023))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* re-route and crossing vni can not co-exist */
    if (CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR)
        && CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
		return CTC_E_NOT_SUPPORT;
    }

    if ((!CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI))
        && (CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA)))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* VxLAN-GPE check */
    if (CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE)
        && (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN4)
        && (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN6))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (!SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_nh_param->p_ip_nh_param)
        && (p_nh_ip_param->tunnel_info.inner_packet_type != PKT_TYPE_ETH))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY)
        && !p_nh_ip_param->tunnel_info.gre_info.gre_key)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_ip_param->tunnel_info.udf_profile_id)
    {
        CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_udf_profile_param_check(lchip, p_nh_ip_param));
    }

    /* VxLAN-GBP check */
    if ((p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN4) &&
        (p_nh_ip_param->tunnel_info.tunnel_type != CTC_TUNNEL_TYPE_VXLAN_IN6) &&
        CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (((p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_VXLAN_IN4) ||
        (p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_VXLAN_IN6)) &&
        CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN) &&
        CTC_FLAG_ISSET(p_nh_ip_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV4_IN4) ||
        (p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV6_IN4) ||
        (p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV4_IN6) ||
        (p_nh_ip_param->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV6_IN6))
    {
        if ((p_nh_ip_param->tunnel_info.l4_dst_port && !p_nh_ip_param->tunnel_info.l4_src_port) ||
                (!p_nh_ip_param->tunnel_info.l4_dst_port && p_nh_ip_param->tunnel_info.l4_src_port))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    /*1.Create unresolved ip-tunnel nh*/
    if (CTC_FLAG_ISSET(p_nh_ip_param->flag, CTC_IP_NH_FLAG_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create unresolved ip tunnel nexthop\n");

        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
        return CTC_E_NONE;
    }

    _sys_usw_nh_ip_tunnel_get_port_extend_mcast_en(lchip, &port_ext_mc_en);

    if ((SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_nh_param->p_ip_nh_param) && port_ext_mc_en)
       && (0 != p_nh_param->p_ip_nh_param->tunnel_info.vn_id )
       && (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)))
    {
        return CTC_E_INVALID_PARAM;
    }

    /*2.Create normal ip-tunnel nh*/
    /*2.1 op dsnh,l2edit,l3edit*/
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = 0;
    CTC_ERROR_GOTO(_sys_usw_nh_ip_tunnel_build_dsnh(lchip, p_nh_param, p_nhdb,&dsfwd_param),ret,error_proc);

     /*3.Create Dwfwd*/
    if(p_nh_param->hdr.have_dsfwd)
    {
      CTC_ERROR_GOTO(_sys_usw_nh_ip_tunnel_add_dsfwd(lchip,  p_com_db),ret,error_proc);
    }

    if (p_nh_ip_param->tunnel_info.udf_profile_id)
    {
       g_usw_nh_master[lchip]->udf_profile_ref_cnt[p_nh_ip_param->tunnel_info.udf_profile_id-1]++;
    }
  return CTC_E_NONE;
 error_proc:
    _sys_usw_nh_free_ip_tunnel_resource(lchip, p_nhdb, 0);
    return ret;

}


int32
sys_usw_nh_delete_ip_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{

    uint32 entry_type = SYS_NH_TYPE_IP_TUNNEL;
    sys_nh_param_dsfwd_t    dsfwd_param = {0};
    sys_nh_info_ip_tunnel_t* p_nhinfo   = NULL;
    sys_nh_ref_list_node_t* p_ref_node, * p_tmp_node;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_EQUAL_CHECK(entry_type, p_data->hdr.nh_entry_type);
    p_nhinfo = (sys_nh_info_ip_tunnel_t*)(p_data);

    sys_usw_nh_update_ecmp_member(lchip, *p_nhid, SYS_NH_CHANGE_TYPE_NH_DELETE);

    /*1. delete all reference ecmp nh*/
    p_ref_node = p_nhinfo->hdr.p_ref_nh_list;

    while (p_ref_node)
    {
        /*Remove from db*/
        p_tmp_node = p_ref_node;
        p_ref_node = p_ref_node->p_next;
        mem_free(p_tmp_node);
    }
    p_nhinfo->hdr.p_ref_nh_list = NULL;

    /*2. Delete dsfwd*/
    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        dsfwd_param.dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
        dsfwd_param.drop_pkt = 1;
        /*Write table*/
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
        /*Free DsFwd offset*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset));
    }

    _sys_usw_nh_free_ip_tunnel_resource(lchip, p_nhinfo, 0);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_ip_tunnel_fwd_to_spec(uint8 lchip, sys_nh_param_ip_tunnel_t* p_nhpara,
                                               sys_nh_info_ip_tunnel_t* p_nhinfo)
{

    sys_nh_param_special_t nh_para_spec;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&nh_para_spec, 0, sizeof(sys_nh_param_special_t));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        nh_para_spec.hdr.have_dsfwd = TRUE;
        nh_para_spec.hdr.nh_param_type = SYS_NH_TYPE_UNROV;
        nh_para_spec.hdr.is_internal_nh = TRUE;

       /*1.2 update dsfwd to unrov nh's dsfwd, write dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_create_special_cb(lchip, (
                                                                sys_nh_param_com_t*)(&nh_para_spec), (sys_nh_info_com_t*)(p_nhinfo)));
    }

    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);

    _sys_usw_nh_free_ip_tunnel_resource(lchip, p_nhinfo, 0);
    _sys_usw_nh_free_offset_by_nhinfo(lchip, SYS_NH_TYPE_IP_TUNNEL, (sys_nh_info_com_t*)p_nhinfo);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_ip_tunnel_fwd_attr(uint8 lchip, sys_nh_param_ip_tunnel_t* p_nhpara,
                                            sys_nh_info_ip_tunnel_t* p_nhinfo)
{

       sys_nh_param_ip_tunnel_t* p_para = (sys_nh_param_ip_tunnel_t*)p_nhpara;
       uint8 port_ext_mc_en = 0;
       sys_nh_info_ip_tunnel_t old_nh_info;
       int32 ret = 0;
       uint8 is_update = (p_nhpara->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);
       uint32 dsnh_offset = 0;
       uint32 cmd = 0;
       uint32 tbl_id = 0;
       DsNextHop8W_m data_w;

       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

       _sys_usw_nh_ip_tunnel_get_port_extend_mcast_en(lchip, &port_ext_mc_en);

       if (p_para->p_ip_nh_param && p_para->p_ip_nh_param->tunnel_info.udf_profile_id)
       {
            CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_udf_profile_param_check(lchip, p_para->p_ip_nh_param));
       }

       if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)
           && p_para->p_ip_nh_param
       && (CTC_FLAG_ISSET(p_para->p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI)
       || (PKT_TYPE_IPV4 == p_para->p_ip_nh_param->tunnel_info.inner_packet_type )
       || (PKT_TYPE_IPV6 == p_para->p_ip_nh_param->tunnel_info.inner_packet_type )
       || (p_para->p_ip_nh_param->tunnel_info.logic_port)
       || (SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_para->p_ip_nh_param) && port_ext_mc_en)
       || (CTC_FLAG_ISSET(p_para->p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR))
       || (0 != p_para->p_ip_nh_param->tunnel_info.vn_id))
       && (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV)))
       {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Should use DsNexthop8W table \n");
           return CTC_E_INVALID_CONFIG;
       }

     if(is_update)
     {
         /*store nh old data, for recover*/
        tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        sal_memset(&data_w, 0, sizeof(DsNextHop8W_m));
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);

         sal_memcpy(&old_nh_info, p_nhinfo, sizeof(sys_nh_info_ip_tunnel_t));
         sal_memset(p_nhinfo, 0, sizeof(sys_nh_info_ip_tunnel_t));
         sys_usw_nh_copy_nh_entry_flags(lchip, &old_nh_info.hdr, &p_nhinfo->hdr);
         p_nhinfo->updateAd = old_nh_info.updateAd;
         p_nhinfo->data = old_nh_info.data;
         p_nhinfo->dot1ae_channel = old_nh_info.dot1ae_channel;
         p_nhinfo->dot1ae_en = old_nh_info.dot1ae_en;
         p_nhinfo->sci_en = old_nh_info.sci_en;
         p_nhinfo->sc_index = old_nh_info.sc_index;
         p_nhinfo->sa_index = old_nh_info.sa_index;
     }

    CTC_ERROR_GOTO(sys_usw_nh_create_ip_tunnel_cb(lchip, (sys_nh_param_com_t*)p_nhpara,
        (sys_nh_info_com_t*)p_nhinfo), ret, error);

    /*for update free resource here*/
   if(is_update)
   {
         _sys_usw_nh_free_ip_tunnel_resource(lchip, &old_nh_info, is_update);
   }
   return CTC_E_NONE;
error:
   if(is_update)
   {
       sal_memcpy( p_nhinfo, &old_nh_info,sizeof(sys_nh_info_ip_tunnel_t));
        tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);
   }
   return ret;

}

STATIC int32
_sys_usw_nh_vxlan_add_vni (uint8 lchip,  uint32 vn_id, uint16* p_fid)
{
    sys_nh_vni_mapping_key_t* p_mapping_info = NULL;
    sys_nh_vni_mapping_key_t mapping_info;
    sys_usw_nh_master_t* p_nh_master = NULL;
    DsEgressVsi_m ds_data;
    uint32 cmd = 0;
    sys_usw_opf_t opf;
    uint32 tmp_fid = 0;

    if ( vn_id > 0xFFFFFF )
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " This Virtual subnet ID is out of range or not assigned\n");
		return CTC_E_BADID;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    sal_memset(&ds_data, 0, sizeof(DsEgressVsi_m));
    sal_memset(&mapping_info, 0, sizeof(sys_nh_vni_mapping_key_t));
    mapping_info.vn_id = vn_id;
    p_mapping_info = ctc_hash_lookup(p_nh_master->vxlan_vni_hash, &mapping_info);
    if ( NULL == p_mapping_info )
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = 0;
        opf.pool_type = p_nh_master->vxlan_opf_type_inner_fid;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &tmp_fid));

        p_mapping_info =  mem_malloc(MEM_NEXTHOP_MODULE,  sizeof(sys_nh_vni_mapping_key_t));
        if (NULL == p_mapping_info)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            sys_usw_opf_free_offset(lchip, &opf, 1, tmp_fid);
			return CTC_E_NO_MEMORY;

        }
        sal_memset(p_mapping_info, 0, sizeof(sys_nh_vni_mapping_key_t));
        p_mapping_info->ref_cnt++;
        p_mapping_info->vn_id = vn_id;
        p_mapping_info->fid = tmp_fid;

        cmd = DRV_IOR(DsEgressVsi_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, (tmp_fid >> 2), cmd, &ds_data);
        if (0 == (tmp_fid % 4) )
        {
            SetDsEgressVsi(V, array_0_vni_f, &ds_data, vn_id);
        }
        else if (1 == (tmp_fid % 4))
        {
            SetDsEgressVsi(V, array_1_vni_f, &ds_data, vn_id);
        }
        else if (2 == (tmp_fid % 4))
        {
            SetDsEgressVsi(V, array_2_vni_f, &ds_data, vn_id);
        }
        else
        {
            SetDsEgressVsi(V, array_3_vni_f, &ds_data, vn_id);
        }
        cmd = DRV_IOW(DsEgressVsi_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, (tmp_fid >> 2), cmd, &ds_data);

        ctc_hash_insert(p_nh_master->vxlan_vni_hash, p_mapping_info);
        *p_fid = tmp_fid;
    }
    else
    {
        p_mapping_info->ref_cnt++;
        *p_fid = p_mapping_info->fid;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_vxlan_remove_vni(uint8 lchip,  uint32 vn_id)
{
    sys_nh_vni_mapping_key_t* p_mapping_info = NULL;
    sys_nh_vni_mapping_key_t mapping_info;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_usw_opf_t opf;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    sal_memset(&mapping_info, 0, sizeof(sys_nh_vni_mapping_key_t));
    mapping_info.vn_id = vn_id;
    p_mapping_info = ctc_hash_lookup(p_nh_master->vxlan_vni_hash, &mapping_info);
    if ( NULL != p_mapping_info )
    {
        p_mapping_info->ref_cnt = (p_mapping_info->ref_cnt)? (p_mapping_info->ref_cnt - 1) : 0;
        if (0 == p_mapping_info->ref_cnt)
        {
            sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
            opf.pool_index = 0;
            opf.pool_type = p_nh_master->vxlan_opf_type_inner_fid;
            sys_usw_opf_free_offset(lchip, &opf, 1, p_mapping_info->fid);

            p_mapping_info->vn_id = vn_id;
            ctc_hash_remove(p_nh_master->vxlan_vni_hash, p_mapping_info);
            mem_free(p_mapping_info);
        }
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_nh_vni_hash_make(sys_nh_vni_mapping_key_t* p_mapping_info)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_mapping_info->vn_id;

    MIX(a, b, c);

    return (c & SYS_NH_OVERLAY_IHASH_MASK);
}

STATIC bool
_sys_usw_nh_vni_hash_cmp(sys_nh_vni_mapping_key_t* p_mapping_info1, sys_nh_vni_mapping_key_t* p_mapping_info)
{
    if (p_mapping_info1->vn_id != p_mapping_info->vn_id)
    {
        return FALSE;
    }

    return TRUE;
}

STATIC int32
_sys_usw_nh_wb_restore_vni(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{

	sys_nh_vni_mapping_key_t* p_db_vni = NULL;
	sys_wb_nh_vni_fid_t wb_vni;
	sys_wb_nh_vni_fid_t *p_wb_vni = &wb_vni;
    sys_usw_opf_t opf;

    uint16 entry_cnt = 0;
    int ret = 0;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_arp_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_VNI);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_vni, 0, sizeof(sys_wb_nh_vni_fid_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_vni, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    p_db_vni  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_vni_mapping_key_t));
    if (NULL == p_db_vni)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    p_db_vni->vn_id = p_wb_vni->vn_id;
    p_db_vni->fid = p_wb_vni->fid;
    p_db_vni->ref_cnt = p_wb_vni->ref_cnt;

    if(NULL == ctc_hash_insert(g_usw_nh_master[lchip]->vxlan_vni_hash, p_db_vni))
    {
    	SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No Memory \n");
    	return CTC_E_NO_MEMORY;
   	}

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type = g_usw_nh_master[lchip]->vxlan_opf_type_inner_fid;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_wb_vni->fid));
    CTC_WB_QUERY_ENTRY_END(p_wb_query);
 done:
    return ret;
}

int32
_sys_usw_nh_vxlan_vni_init(uint8 lchip)
{
	sys_usw_nh_master_t* p_nh_master = NULL;
    sys_usw_opf_t opf;
    uint32 max_fid = 0;
    ctc_wb_query_t    wb_query;
    int32 ret = 0;

    /*restore  vni mapping*/
    wb_query.buffer = mem_malloc(MEM_NEXTHOP_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_ERROR_GOTO(sys_usw_nh_get_nh_master(lchip, &p_nh_master), ret, error0);

    /*init opf for inner fid */
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_nh_master->vxlan_opf_type_inner_fid, 1, "opf-overlay-inner-fid"), ret, error0);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type = p_nh_master->vxlan_opf_type_inner_fid;
    max_fid = MCHIP_CAP(SYS_CAP_SPEC_MAX_FID);
    if (max_fid < SYS_NH_OVERLAY_TUNNEL_MAX_INNER_FID)
    {
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, (max_fid+1), SYS_NH_OVERLAY_TUNNEL_MAX_INNER_FID), ret, error0);
    }

    p_nh_master->vxlan_vni_hash = ctc_hash_create(256,
                                                                (SYS_NH_OVERLAY_IHASH_MASK + 1),
                                                                (hash_key_fn)_sys_usw_nh_vni_hash_make,
                                                                (hash_cmp_fn)_sys_usw_nh_vni_hash_cmp);
    if (!p_nh_master->vxlan_vni_hash)
    {
        return CTC_E_NO_MEMORY;
    }

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        _sys_usw_nh_wb_restore_vni(lchip, &wb_query);
    }

error0:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }
    return ret;
}

int32
sys_usw_nh_ip_tunnel_init(uint8 lchip)
{
	sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    /*IPv4 SA */
    p_nh_master->ipsa[CTC_IP_VER_4] = mem_malloc(MEM_NEXTHOP_MODULE,
                                                              MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM) * sizeof(sys_nh_ip_tunnel_sa_v4_node_t));    /*IPv6 SA */
    p_nh_master->ipsa[CTC_IP_VER_6] = mem_malloc(MEM_NEXTHOP_MODULE,
                                                               MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM) * sizeof(sys_nh_ip_tunnel_sa_v6_node_t));

    if ((NULL == p_nh_master->ipsa[CTC_IP_VER_4])
        || (NULL == p_nh_master->ipsa[CTC_IP_VER_6]))
    {

        mem_free(p_nh_master->ipsa[CTC_IP_VER_4]);
        mem_free(p_nh_master->ipsa[CTC_IP_VER_6]);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_nh_master->ipsa[CTC_IP_VER_4], 0,
               MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM) *  sizeof(sys_nh_ip_tunnel_sa_v4_node_t));
    sal_memset(p_nh_master->ipsa[CTC_IP_VER_6], 0,
               MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM) * sizeof(sys_nh_ip_tunnel_sa_v6_node_t));

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_ip_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                     sys_nh_param_com_t* p_para)
{
    sys_nh_info_ip_tunnel_t* p_nh_info;
    sys_nh_param_ip_tunnel_t* p_nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_IP_TUNNEL, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_IP_TUNNEL, p_nh_db->hdr.nh_entry_type);

    p_nh_info = (sys_nh_info_ip_tunnel_t*)(p_nh_db);
    p_nh_para = (sys_nh_param_ip_tunnel_t*)(p_para);

    switch (p_nh_para->hdr.change_type)
    {
    case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
        if (p_nh_info->arp_id)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Arp nexthop cannot update to unrov \n");
			return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(_sys_usw_nh_update_ip_tunnel_fwd_to_spec(lchip, p_nh_para, p_nh_info));
        break;

    case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_ip_tunnel_fwd_attr(lchip, p_nh_para, p_nh_info));
        break;

    case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:

        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Current nexthop isnt unresolved nexthop \n");
			return CTC_E_INVALID_CONFIG;

        }
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_ip_tunnel_fwd_attr(lchip, p_nh_para, p_nh_info));
        break;
  case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
        {
          if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
          {
              return CTC_E_NONE;
          }
          CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_add_dsfwd(lchip,  p_nh_db));
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_info->hdr.p_ref_nh_list)
    {
        int32 ret = 0;
        ret = (sys_usw_nh_update_ecmp_member(lchip, p_nh_para->hdr.nhid, p_nh_para->hdr.change_type));
        if (ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop update for ecmp fail!, ret:%d \n", ret);
        }
    }

    if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED))
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_loopback_l2edit(lchip, p_nh_para->hdr.nhid));
    }

    return CTC_E_NONE;
}


int32 sys_usw_nh_ip_tunnel_update_dot1ae(uint8 lchip, void* param)
{
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_nh_info_ip_tunnel_t* p_nh_tunnel_info = NULL;
    sys_usw_register_dot1ae_bind_sc_t* bind_sc = (sys_usw_register_dot1ae_bind_sc_t*)param;
    void*  p_l3edit_old = NULL;
    uint8 old_sci_en = 0;
    void*  p_l3edit = NULL;
    uint32 dsl3edit_offset = 0;
    sys_dsl3edit_tunnelv4_t  dsl3edit4w;
    sys_dsl3edit_tunnelv6_t dsl3editv6;
    uint8 edit_type = 0;
    uint32 cmd    = 0;
    ds_t ds;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, bind_sc->gport, p_nh_master, &p_nh_info));
    p_nh_tunnel_info = (sys_nh_info_ip_tunnel_t*)p_nh_info;
    if(NULL == p_nh_tunnel_info->p_l3edit)
    {
        return CTC_E_NONE;
    }
    p_l3edit_old = p_nh_tunnel_info->p_l3edit;
    if (CTC_FLAG_ISSET(p_nh_tunnel_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V4))
    {
        edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4;
        sal_memcpy((uint8*)(&dsl3edit4w), (uint8*)p_l3edit_old, sizeof(dsl3edit4w));
        old_sci_en = dsl3edit4w.sci_en;
        dsl3edit4w.sci_en = bind_sc->include_sci;
        p_l3edit = &dsl3edit4w;
    }
    else if (CTC_FLAG_ISSET(p_nh_tunnel_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V6))
    {
        edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6;
        sal_memcpy((uint8*)(&dsl3editv6), (uint8*)p_l3edit_old, sizeof(dsl3editv6));
        old_sci_en = dsl3editv6.sci_en;
        dsl3editv6.sci_en = bind_sc->include_sci;
        p_l3edit = &dsl3editv6;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    if (old_sci_en == bind_sc->include_sci)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_l3edit, edit_type, &dsl3edit_offset));
    sal_memset(ds, 0, sizeof(ds));
    if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        cmd = DRV_IOR(DsNextHop8W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_nh_info->hdr.dsfwd_info.dsnh_offset, cmd, &ds), ret, error);
        if (p_nh_tunnel_info->p_dsl2edit_inner)
        {
            SetDsNextHop8W(V, outerEditPtr_f, &ds, dsl3edit_offset);
        }
        else
        {
            SetDsNextHop8W(V, innerEditPtr_f, &ds, dsl3edit_offset);
        }
        cmd = DRV_IOW(DsNextHop8W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_nh_info->hdr.dsfwd_info.dsnh_offset, cmd, &ds), ret, error);
    }
    else
    {
        cmd = DRV_IOR(DsNextHop4W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_nh_info->hdr.dsfwd_info.dsnh_offset, cmd, &ds), ret, error);
        if (p_nh_tunnel_info->p_dsl2edit_inner)
        {
            SetDsNextHop4W(V, outerEditPtr_f, &ds, dsl3edit_offset);
        }
        else
        {
            SetDsNextHop4W(V, innerEditPtr_f, &ds, dsl3edit_offset);
        }
        cmd = DRV_IOW(DsNextHop4W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_nh_info->hdr.dsfwd_info.dsnh_offset, cmd, &ds), ret, error);
    }

    p_nh_tunnel_info->p_l3edit = p_l3edit;
    p_nh_tunnel_info->dsl3edit_offset = dsl3edit_offset;
    p_nh_tunnel_info->sci_en = bind_sc->include_sci;
    /*remove old l3edit*/
    CTC_ERROR_RETURN(sys_usw_nh_remove_l3edit_tunnel(lchip, p_l3edit_old, edit_type));
    return CTC_E_NONE;
error:
    sys_usw_nh_remove_l3edit_tunnel(lchip, p_l3edit, edit_type);
    return ret;
}


