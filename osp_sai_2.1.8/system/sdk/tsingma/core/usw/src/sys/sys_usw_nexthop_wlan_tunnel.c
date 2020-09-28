/**
 @file sys_usw_nexthop_wlan_tunnel.c

 @date 2016-02-24

 @version v2.0

 The file contains all nexthop wlan tunnel related callback function
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
#include "sys_usw_l3if.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_vlan.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_opf.h"
#include "sys_usw_register.h"
#include "sys_usw_common.h"

#include "usw/include/drv_io.h"

extern int32 _sys_usw_nh_ip_tunnel_alloc_ipsa_idx(uint8 lchip, uint8 ip_ver, void* p_add, uint8* p_index, uint8 old_index);
extern int32 _sys_usw_nh_ip_tunnel_free_ipsa_idx(uint8 lchip, uint8 ip_ver, uint8 index);
extern int32 _sys_usw_nh_write_entry_dsl2editeth4w(uint8 lchip, uint32 offset,
                    sys_dsl2edit_t* p_ds_l2_edit_4w , uint8 type);
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
STATIC int32
_sys_usw_nh_wlan_tunnel_build_dsl3edit_v4(uint8 lchip, ctc_nh_wlan_tunnel_param_t* p_nh_param,
                                              sys_nh_info_wlan_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_tunnelv4_t  dsl3edit4w;
    sys_dsl3edit_tunnelv4_t*  p_dsl3edit4w;
    uint8 sa_index = 0xFF;
    int32 ret = CTC_E_NONE;

    sal_memset(&dsl3edit4w, 0, sizeof(dsl3edit4w));
    dsl3edit4w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_TUNNEL_V4;
    dsl3edit4w.ds_type   = SYS_NH_DS_TYPE_L3EDIT;

    /*ttl*/
    dsl3edit4w.ttl = p_nh_param->ttl;
    dsl3edit4w.map_ttl = CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MAP_TTL);

    if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_NONE)
    {
        dsl3edit4w.ecn_mode = 3;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_ASSIGN)
    {
        return CTC_E_NOT_SUPPORT;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_MAP)
    {
        dsl3edit4w.ecn_mode = 1;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_PACKET)
    {
        dsl3edit4w.ecn_mode = 0;
    }

    /*stats*/
    if (p_nh_param->stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_nh_param->stats_id,
                                                           &(dsl3edit4w.stats_ptr)));
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_COPY_DONT_FRAG))
    {
        dsl3edit4w.copy_dont_frag = 1;
        dsl3edit4w.dont_frag  = 0;
    }
    else
    {
        /*dont frag*/
        dsl3edit4w.copy_dont_frag = 0;
        dsl3edit4w.dont_frag = CTC_FLAG_ISSET(p_nh_param->flag,CTC_NH_WLAN_TUNNEL_FLAG_DONT_FRAG);
    }

    /* dscp */
    if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_ASSIGN)
    {
        CTC_MAX_VALUE_CHECK(p_nh_param->dscp_or_tos, 63);
        dsl3edit4w.derive_dscp = 1;
        dsl3edit4w.dscp = p_nh_param->dscp_or_tos;
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_MAP)
    {
        dsl3edit4w.derive_dscp = 2;
        dsl3edit4w.dscp = 1 << 0;
        dsl3edit4w.dscp_domain = p_nh_param->dscp_domain;
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_PACKET)
    {
        dsl3edit4w.derive_dscp = 3;
        dsl3edit4w.dscp = 0 << 0;
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_NONE)
    {
        dsl3edit4w.derive_dscp = 0;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* mtu check */
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_FRAGMENT_EN))
    {
        if (p_nh_param->mtu_size > 0x3FFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3edit4w.mtu_check_en = 1;
        dsl3edit4w.mtu_size = p_nh_param->mtu_size/144;
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_KEEPALIVE))
    {
        dsl3edit4w.data_keepalive = 1;
    }

    dsl3edit4w.share_type = SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_WLAN;
    dsl3edit4w.inner_header_type = 3;
    dsl3edit4w.inner_header_valid = 1;
    dsl3edit4w.ipda = p_nh_param->ip_da.ipv4;
    CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_4,
                                           &(p_nh_param->ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
    dsl3edit4w.ipsa_index = sa_index & 0xF;
    dsl3edit4w.ipsa_valid = 0;
    dsl3edit4w.l4_dest_port = p_nh_param->l4_dst_port;
    dsl3edit4w.l4_src_port = p_nh_param->l4_src_port;
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_ENCRYPT_EN))
    {
        dsl3edit4w.encrypt_en = 1;
        dsl3edit4w.encrypt_id = (0x80|p_nh_param->encrypt_id);
    }
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_RADIO_MAC_EN)
        && (!CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN)))
    {
        dsl3edit4w.rmac_en = 1;
    }
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN))
    {
        dsl3edit4w.bssid_bitmap = p_nh_param->bssid_bitmap;
    }

    p_dsl3edit4w = &dsl3edit4w;
    ret = sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3edit4w, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4, &dsnh->l3edit_ptr);
    if (ret)
    {
        _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_4, sa_index);
        return ret;
    }

    p_edit_db->p_dsl3edit_tunnel = p_dsl3edit4w;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_WLAN_TUNNEL_FLAG_IN_V4;
    p_edit_db->sa_index = sa_index;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_wlan_tunnel_build_dsl3edit_v6(uint8 lchip, ctc_nh_wlan_tunnel_param_t* p_nh_param,
                                              sys_nh_info_wlan_tunnel_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_tunnelv6_t dsl3editv6;
    sys_dsl3edit_tunnelv6_t*  p_dsl3editv6;
    uint32 ttl_index = 0;
    uint8 sa_index = 0xFF;
    int32 ret = CTC_E_NONE;

    sal_memset(&dsl3editv6, 0, sizeof(dsl3editv6));
    dsl3editv6.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_TUNNEL_V6;
    dsl3editv6.ds_type   = SYS_NH_DS_TYPE_L3EDIT;

    /*ttl*/
    CTC_ERROR_RETURN(sys_usw_lkup_ttl_index(lchip, p_nh_param->ttl, &ttl_index));
    dsl3editv6.ttl = p_nh_param->ttl;
    dsl3editv6.map_ttl = CTC_FLAG_ISSET(p_nh_param->flag,
                                        CTC_NH_WLAN_TUNNEL_FLAG_MAP_TTL);
    if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_NONE)
    {
        dsl3editv6.ecn_mode = 3;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_ASSIGN)
    {
        return CTC_E_NOT_SUPPORT;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_MAP)
    {
        dsl3editv6.ecn_mode = 1;
    }
    else if (p_nh_param->ecn_select ==  CTC_NH_ECN_SELECT_PACKET)
    {
        dsl3editv6.ecn_mode = 0;
    }

    /*stats*/
    if (p_nh_param->stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_nh_param->stats_id,
                                                           &(dsl3editv6.stats_ptr)));
    }

    if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_ASSIGN)
    {
        CTC_MAX_VALUE_CHECK(p_nh_param->dscp_or_tos, 63);
        dsl3editv6.derive_dscp = 1;
        dsl3editv6.tos = (p_nh_param->dscp_or_tos<<2);
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_MAP)
    {
        dsl3editv6.derive_dscp = 2;
        dsl3editv6.tos = 1 << 0;
        dsl3editv6.dscp_domain = p_nh_param->dscp_domain;
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_PACKET)
    {
        dsl3editv6.derive_dscp = 3;
        dsl3editv6.tos = 0 << 0;
    }
    else if (p_nh_param->dscp_select == CTC_NH_DSCP_SELECT_NONE)
    {
        dsl3editv6.derive_dscp = 0;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_param->flow_label_mode == CTC_NH_FLOW_LABEL_NONE)
    {
        dsl3editv6.new_flow_label_valid = 0x0;
    }
    else if (p_nh_param->flow_label_mode == CTC_NH_FLOW_LABEL_WITH_HASH ||
             p_nh_param->flow_label_mode == CTC_NH_FLOW_LABEL_ASSIGN)
    {
        if (p_nh_param->flow_label > 0xFFFFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3editv6.new_flow_label_valid = 0x1;
        dsl3editv6.new_flow_label_mode = (p_nh_param->flow_label_mode == CTC_NH_FLOW_LABEL_WITH_HASH);
        dsl3editv6.flow_label = p_nh_param->flow_label;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* mtu check */
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_FRAGMENT_EN))
    {
        if (p_nh_param->mtu_size > 0x3FFF)
        {
            return CTC_E_INVALID_PARAM;
        }

        dsl3editv6.mtu_check_en = 1;
        dsl3editv6.mtu_size = p_nh_param->mtu_size/144;
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_KEEPALIVE))
    {
        dsl3editv6.data_keepalive = 1;
    }

    /* ipda, use little india for DRV_SET_FIELD_A */
    dsl3editv6.ipda[0] = p_nh_param->ip_da.ipv6[3];
    dsl3editv6.ipda[1] = p_nh_param->ip_da.ipv6[2];
    dsl3editv6.ipda[2] = p_nh_param->ip_da.ipv6[1];
    dsl3editv6.ipda[3] = p_nh_param->ip_da.ipv6[0];
    dsl3editv6.share_type = SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_WLAN;
    dsl3editv6.inner_header_type = 3;
    dsl3editv6.inner_header_valid = 1;
    CTC_ERROR_RETURN(_sys_usw_nh_ip_tunnel_alloc_ipsa_idx(lchip, CTC_IP_VER_6,
                                           &(p_nh_param->ip_sa.ipv4), &sa_index, p_edit_db->sa_index));
    dsl3editv6.ipsa_index = sa_index & 0xF;
    dsl3editv6.l4_dest_port = p_nh_param->l4_dst_port;
    dsl3editv6.l4_src_port = p_nh_param->l4_src_port;
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_ENCRYPT_EN))
    {
        dsl3editv6.encrypt_en = 1;
        dsl3editv6.encrypt_id = (0x80|p_nh_param->encrypt_id);
    }
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_RADIO_MAC_EN))
    {
        dsl3editv6.rmac_en = 1;
    }
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN))
    {
        dsl3editv6.bssid_bitmap = p_nh_param->bssid_bitmap;
    }

    p_dsl3editv6 = &dsl3editv6;
    ret = sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsl3editv6, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, &dsnh->l3edit_ptr);
    if (ret)
    {
        _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_6, sa_index);
        return ret;
    }

    p_edit_db->p_dsl3edit_tunnel = p_dsl3editv6;
    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->flag |= SYS_NH_WLAN_TUNNEL_FLAG_IN_V6;
    p_edit_db->sa_index = sa_index;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_wlan_tunnel_build_dsl3edit(uint8 lchip, ctc_nh_wlan_tunnel_param_t* p_nh_param,
                                           sys_nh_info_wlan_tunnel_t* p_edit_db,
                                           sys_nh_param_dsnh_t* dsnh)
{

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IPV6))
    {
        CTC_ERROR_RETURN(_sys_usw_nh_wlan_tunnel_build_dsl3edit_v6(lchip, p_nh_param, p_edit_db, dsnh));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_nh_wlan_tunnel_build_dsl3edit_v4(lchip, p_nh_param, p_edit_db, dsnh));
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_wlan_tunnel_build_dsl2edit(uint8 lchip, ctc_nh_wlan_tunnel_param_t* p_nh_param,
                                           sys_nh_info_wlan_tunnel_t* p_edit_db,
                                           sys_nh_param_dsnh_t* dsnh)
{
    sys_nh_db_dsl2editeth4w_t* p_dsl2edit;
    sys_nh_db_dsl2editeth4w_t   dsl2edit3w;
    sys_nh_db_dsl2editeth8w_t   dsl2edit6w;
    sys_nh_db_dsl2editeth8w_t* p_dsl2edit6w;

    uint32 new_offset = 0;
    uint8 inner_mac_edit = 0;
    uint8 type = 0;

    sal_memset(&dsl2edit3w, 0 , sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit6w, 0 , sizeof(sys_nh_db_dsl2editeth8w_t));

    inner_mac_edit = (p_nh_param->mac_da[0] || p_nh_param->mac_da[1] || p_nh_param->mac_da[2] ||
        p_nh_param->mac_da[3] || p_nh_param->mac_da[4] || p_nh_param->mac_da[5]);

    if ((!CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET)))
    {
       dsnh->flag |= SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE;
    }

    if (inner_mac_edit && !CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET) &&
        !CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN))
    {
        type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP;
    }
    else if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS) && p_nh_param->cos_domain)
    {
        type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W;
    }
    else
    {
        type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W;
    }

    if ((!CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN))
        && (!CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET))
        && !inner_mac_edit)
    {
        dsnh->inner_l2edit_ptr = 0;
        return CTC_E_NONE;
    }


    sal_memcpy(dsl2edit3w.mac_da, p_nh_param->mac_da, sizeof(mac_addr_t));

    dsl2edit3w.dynamic = 1;
    dsl2edit3w.mac_da_en = inner_mac_edit;

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET))
    {
        /*for route packet, vlan edit decided by p_nh_param->vlan_id is 0 or not*/
        if (p_nh_param->vlan_id)
        {
            dsl2edit3w.output_vid = p_nh_param->vlan_id;
            dsl2edit3w.output_vlan_is_svlan = 1;
        }
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN))
    {
        if (p_nh_param->dot11_sub_type > 0xF)
        {
            return CTC_E_INVALID_PARAM;
        }
        dsl2edit3w.derive_mac_from_label = (p_nh_param->dot11_sub_type&0x08)?1:0;
        dsl2edit3w.derive_mcast_mac_for_mpls = 1;
        dsl2edit3w.derive_mcast_mac_for_trill = 0;
        dsl2edit3w.dot11_sub_type = (p_nh_param->dot11_sub_type&0x07);
        if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS))
        {
            dsl2edit3w.map_cos = 1;
        }
        else
        {
            dsl2edit3w.map_cos = 0;
        }
        dsl2edit3w.is_dot11 = 1;
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN))
    {
        dsl2edit3w.derive_mcast_mac_for_mpls = 1;
        dsl2edit3w.derive_mcast_mac_for_trill = 1;
        sal_memcpy(dsl2edit3w.mac_da, p_nh_param->mac_da, sizeof(mac_addr_t));
    }

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN))
    {
        dsl2edit3w.derive_mcast_mac_for_ip = 1;
    }

    p_dsl2edit6w = &dsl2edit6w;

    if (type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W)
    {
        sal_memcpy(p_dsl2edit6w, &dsl2edit3w, sizeof(sys_nh_db_dsl2editeth4w_t));
        p_dsl2edit6w->cos_domain = p_nh_param->cos_domain;
    }

    /*route packet && mcast enable, need support fallbackbridge, donot using edithash to share innerL2Edit*/
    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET)
        && (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN)))
    {
        sys_dsl2edit_t l2edit_param;
        sal_memset(&l2edit_param, 0, sizeof(sys_dsl2edit_t));
        l2edit_param.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
        l2edit_param.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_4W;
        l2edit_param.dynamic = 1;
        l2edit_param.is_vlan_valid = (dsl2edit3w.output_vid != 0);
        l2edit_param.output_vlan_id = dsl2edit3w.output_vid;
        l2edit_param.is_dot11 = dsl2edit3w.is_dot11;
        l2edit_param.derive_mac_from_label = dsl2edit3w.derive_mac_from_label;
        l2edit_param.derive_mcast_mac_for_mpls = dsl2edit3w.derive_mcast_mac_for_mpls;
        l2edit_param.derive_mcast_mac_for_trill = dsl2edit3w.derive_mcast_mac_for_trill;
        l2edit_param.map_cos = dsl2edit3w.map_cos;
        l2edit_param.dot11_sub_type = dsl2edit3w.dot11_sub_type;
        l2edit_param.derive_mcast_mac_for_ip = dsl2edit3w.derive_mcast_mac_for_ip;
        sal_memcpy(l2edit_param.mac_da, dsl2edit3w.mac_da, sizeof(mac_addr_t));

        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 2, &new_offset));
        l2edit_param.offset = new_offset;

        CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, new_offset, &l2edit_param,
            SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W));

        sal_memset(l2edit_param.mac_da, 0, sizeof(mac_addr_t));
        l2edit_param.offset = new_offset+1;
        CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, (new_offset+1), &l2edit_param,
            SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W));

        CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_3W);
        p_edit_db->inner_l2_ptr = new_offset;
        p_edit_db->use_multi_l2_ptr = 1;
        p_edit_db->l2_edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W;
        dsnh->inner_l2edit_ptr = new_offset;
    }
    else
    {
        p_dsl2edit = &dsl2edit3w;
        if (type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W)
        {
            CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_6w_inner(lchip, (sys_nh_db_dsl2editeth8w_t**)&p_dsl2edit6w, &dsnh->inner_l2edit_ptr));
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_6W);
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_3w_inner(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_dsl2edit, type, &dsnh->inner_l2edit_ptr));
            CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_3W);
        }
        p_edit_db->p_dsl2edit_inner = p_dsl2edit;
        p_edit_db->inner_l2_ptr = dsnh->inner_l2edit_ptr;
        p_edit_db->l2_edit_type = type;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_free_wlan_tunnel_resource(uint8 lchip, sys_nh_info_wlan_tunnel_t* p_nhinfo)
{

    sys_nh_db_dsl2editeth4w_t  dsl2edit;
    sys_nh_db_dsl2editeth8w_t  dsl2edit_8w;

    sal_memset(&dsl2edit, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&dsl2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));



    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        if (p_nhinfo->flag & SYS_NH_WLAN_TUNNEL_FLAG_IN_V4)
        {
            sys_usw_nh_remove_l3edit_tunnel(lchip, p_nhinfo->p_dsl3edit_tunnel, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4);
            _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_4, p_nhinfo->sa_index);
        }
        else if (p_nhinfo->flag & SYS_NH_WLAN_TUNNEL_FLAG_IN_V6)
        {
            sys_usw_nh_remove_l3edit_tunnel(lchip, p_nhinfo->p_dsl3edit_tunnel, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6);
            _sys_usw_nh_ip_tunnel_free_ipsa_idx(lchip, CTC_IP_VER_6, p_nhinfo->sa_index);
        }
    }


    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W))
    {
        if (p_nhinfo->p_dsl2edit_inner && !p_nhinfo->use_multi_l2_ptr)
        {
            sys_usw_nh_remove_l2edit_3w_inner(lchip, (sys_nh_db_dsl2editeth4w_t*)p_nhinfo->p_dsl2edit_inner, p_nhinfo->l2_edit_type);
        }

        if (p_nhinfo->use_multi_l2_ptr)
        {
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 2, p_nhinfo->inner_l2_ptr);
        }

        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W);
    }

    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_wlan_tunnel_build_dsnh(uint8 lchip, ctc_nh_wlan_tunnel_param_t* p_wlan_nh_param,
                                       sys_nh_info_wlan_tunnel_t* p_edit_db,
                                       sys_nh_param_dsfwd_t * p_fwd_info)
{
    sys_nh_param_dsnh_t dsnh_param;
    int32 ret = CTC_E_NONE;
    uint8 gchip = 0;
    uint16 lport = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_wlan_nh_param);
    CTC_PTR_VALID_CHECK(p_edit_db);

    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));

    sys_usw_get_gchip_id(lchip, &gchip);
    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_E2ILOOP, &lport));
    p_edit_db->gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

    /*1. Build dsL2Edit*/
    CTC_ERROR_RETURN(_sys_usw_nh_wlan_tunnel_build_dsl2edit(lchip, p_wlan_nh_param, p_edit_db, &dsnh_param));

    /*2. Build dsL3Edit*/
    CTC_ERROR_RETURN(_sys_usw_nh_wlan_tunnel_build_dsl3edit(lchip, p_wlan_nh_param, p_edit_db, &dsnh_param));

    if (dsnh_param.l3edit_ptr != 0)
    {
        CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    }

    dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_WLANTUNNEL;
    dsnh_param.logic_port = p_wlan_nh_param->logic_port;
    dsnh_param.logic_port_check = CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_LOGIC_PORT_CHECK)?1:0;

    dsnh_param.dest_vlan_ptr  = MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID)+4096;

    dsnh_param.wlan_mc_en = CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN) && (p_wlan_nh_param->bssid_bitmap != 0);

    if (CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_POP_POA_ROAM))
    {
        if (p_wlan_nh_param->is_pop)
        {
            dsnh_param.wlan_tunnel_type = 1;
        }
        else
        {
            dsnh_param.wlan_tunnel_type = 2;
        }
    }

    dsnh_param.rid = p_wlan_nh_param->radio_id;

    dsnh_param.stag_en = 1;

    if (!CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET) &&
        CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_UNTAG_EN))
    {
        /*for bridge to wtp, remove vlan ID*/
        dsnh_param.stag_en = 0;
    }

    dsnh_param.is_dot11 = CTC_FLAG_ISSET(p_wlan_nh_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN);
    sal_memcpy(dsnh_param.macDa, p_wlan_nh_param->radio_mac, sizeof(mac_addr_t));

   if (CTC_FLAG_ISSET(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
   {
       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lchip = %d, Write DsNexthop8w Table\n", lchip);

       dsnh_param.dsnh_offset = p_edit_db->hdr.dsfwd_info.dsnh_offset;
       ret = sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param);
       p_fwd_info->nexthop_ext = 1;
   }

   return ret;

}

STATIC int32
_sys_usw_nh_wlan_tunnel_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
#if 0
    sys_nh_info_wlan_tunnel_t* p_nh_info = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = CTC_E_NONE;

    p_nh_info     = (sys_nh_info_wlan_tunnel_t*)(p_com_db);

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);


    dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_info->gport);

    dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_info->gport);
    dsfwd_param.cut_through_dis = 1;/*disable cut through for non-ip packet when encap ip-tunnel*/

    /*2.2 Build DsFwd Table*/

    if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {

        ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                             &(p_nh_info->hdr.dsfwd_info.dsfwd_offset));

    }
    dsfwd_param.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.dsnh_offset = p_nh_info->hdr.dsfwd_info.dsnh_offset;
    dsfwd_param.drop_pkt = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
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
#endif
    return CTC_E_NONE;
}
int32
sys_usw_nh_create_wlan_tunnel_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db)
{

    sys_nh_param_wlan_tunnel_t* p_nh_param = NULL;
    ctc_nh_wlan_tunnel_param_t* p_nh_wlan_param = NULL;
    sys_nh_info_wlan_tunnel_t* p_nhdb = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = 0;


    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db)
    CTC_EQUAL_CHECK(SYS_NH_TYPE_WLAN_TUNNEL, p_com_nh_para->hdr.nh_param_type);
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    p_nh_param = (sys_nh_param_wlan_tunnel_t*)(p_com_nh_para);
    p_nh_wlan_param = p_nh_param->p_wlan_nh_param;
    p_nhdb     = (sys_nh_info_wlan_tunnel_t*)(p_com_db);
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_WLAN_TUNNEL;


    /* if need logic port do range check*/
    if (p_nh_wlan_param->logic_port && (p_nh_wlan_param->logic_port > 0x3FFF))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");
		return CTC_E_INVALID_PORT;
    }

    if (CTC_FLAG_ISSET(p_nh_wlan_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN) && CTC_FLAG_ISSET(p_nh_wlan_param->flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Wds cannot supoort route packet\n");
		return CTC_E_PARAM_CONFLICT;
    }

    /*2.Create normal ip-tunnel nh*/
    /*2.1 op dsnh,l2edit,l3edit*/
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = 0;
    CTC_ERROR_GOTO(_sys_usw_nh_wlan_tunnel_build_dsnh(lchip, p_nh_wlan_param, p_nhdb,&dsfwd_param),ret,error_proc);

     /*3.Create Dwfwd*/
    if(p_nh_param->hdr.have_dsfwd)
    {
       CTC_ERROR_GOTO(_sys_usw_nh_wlan_tunnel_add_dsfwd(lchip,  p_com_db),ret,error_proc);
    }
    return CTC_E_NONE;
error_proc:
       _sys_usw_nh_free_wlan_tunnel_resource(lchip, p_nhdb);
    return ret;

}


int32
sys_usw_nh_delete_wlan_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{

    uint32 entry_type = SYS_NH_TYPE_WLAN_TUNNEL;
    sys_nh_param_dsfwd_t    dsfwd_param = {0};
    sys_nh_info_wlan_tunnel_t* p_nhinfo   = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_EQUAL_CHECK(entry_type, p_data->hdr.nh_entry_type);
    p_nhinfo = (sys_nh_info_wlan_tunnel_t*)(p_data);

    /*1. Delete dsfwd*/
    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        dsfwd_param.dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
        dsfwd_param.drop_pkt = 1;
        /*Write table*/
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
        /*Free DsFwd offset*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
        SYS_NH_ENTRY_TYPE_FWD, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset));
    }

    _sys_usw_nh_free_wlan_tunnel_resource(lchip, p_nhinfo);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_wlan_tunnel_fwd_to_spec(uint8 lchip, sys_nh_param_wlan_tunnel_t* p_nhpara,
                                               sys_nh_info_wlan_tunnel_t* p_nhinfo)
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

    _sys_usw_nh_free_wlan_tunnel_resource(lchip, p_nhinfo);
    _sys_usw_nh_free_offset_by_nhinfo(lchip, SYS_NH_TYPE_WLAN_TUNNEL, (sys_nh_info_com_t*)p_nhinfo);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_wlan_tunnel_fwd_attr(uint8 lchip, sys_nh_param_wlan_tunnel_t* p_nhpara,
                                            sys_nh_info_wlan_tunnel_t* p_nhinfo)
{
    int32 ret = 0;
    sys_nh_info_wlan_tunnel_t old_nh_info;
    uint8 is_update = (p_nhpara->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

     if(is_update)
     {   /*store nh old data, for recover*/
        sal_memcpy(&old_nh_info, p_nhinfo, sizeof(sys_nh_info_wlan_tunnel_t));
        sal_memset(p_nhinfo, 0, sizeof(sys_nh_info_wlan_tunnel_t));
        sys_usw_nh_copy_nh_entry_flags(lchip, &old_nh_info.hdr, &p_nhinfo->hdr);
     }
     CTC_ERROR_GOTO(sys_usw_nh_create_wlan_tunnel_cb(lchip, (sys_nh_param_com_t*)p_nhpara,
        (sys_nh_info_com_t*)p_nhinfo), ret, error);

    if(is_update)
    {
       _sys_usw_nh_free_wlan_tunnel_resource(lchip, &old_nh_info);
    }
    return CTC_E_NONE;
error:
   if(is_update)
   {
       sal_memcpy( p_nhinfo, &old_nh_info,sizeof(sys_nh_info_wlan_tunnel_t));
   }
    return ret;

}

int32
sys_usw_nh_update_wlan_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                     sys_nh_param_com_t* p_para)
{
    sys_nh_info_wlan_tunnel_t* p_nh_info;
    sys_nh_param_wlan_tunnel_t* p_nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_WLAN_TUNNEL, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_WLAN_TUNNEL, p_nh_db->hdr.nh_entry_type);

    p_nh_info = (sys_nh_info_wlan_tunnel_t*)(p_nh_db);
    p_nh_para = (sys_nh_param_wlan_tunnel_t*)(p_para);

    switch (p_nh_para->hdr.change_type)
    {
        case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
            CTC_ERROR_RETURN(_sys_usw_nh_update_wlan_tunnel_fwd_to_spec(lchip, p_nh_para, p_nh_info));
            break;

        case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
           p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

            CTC_ERROR_RETURN(_sys_usw_nh_update_wlan_tunnel_fwd_attr(lchip, p_nh_para, p_nh_info));
            break;

        case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:
            if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Current nexthop isnt unresolved nexthop \n");
    			return CTC_E_INVALID_CONFIG;
            }
            p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

            CTC_ERROR_RETURN(_sys_usw_nh_update_wlan_tunnel_fwd_attr(lchip, p_nh_para, p_nh_info));
            break;
      case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
            {
              if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
              {
                  return CTC_E_NONE;
              }
              _sys_usw_nh_wlan_tunnel_add_dsfwd(lchip,  p_nh_db);
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wlan_create_reserve_nh(uint8 lchip, sys_nh_entry_table_type_t nh_type, uint32* p_data)
{
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_dsl2edit_t l2edit;
    uint8 gchip_id = 0;
    uint16 lport = 0;
    int32 ret;
    uint32 fwd_ptr = 0;
    uint32 l2_edit_ptr = 0;

    if (nh_type == SYS_NH_ENTRY_TYPE_FWD)
    {
        /*reserve dsfwd resource*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, &fwd_ptr));

        /* write dsfwd */
        CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip_id), ret, error_proc);
        CTC_ERROR_GOTO(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_E2ILOOP, &lport), ret, error_proc);

        sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

        dsfwd_param.dest_chipid = gchip_id;
        dsfwd_param.dest_id = lport;
        dsfwd_param.dsfwd_offset = fwd_ptr;
        dsfwd_param.dsnh_offset = SYS_NH_BUILD_INT_OFFSET(SYS_DSNH4WREG_INDEX_FOR_WLAN_DECAP);

        CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param), ret, error_proc);

        *p_data = fwd_ptr;
    }
    else if (nh_type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W)
    {
        sal_memset(&l2edit, 0, sizeof(l2edit));

        l2edit.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
        l2edit.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_4W;
        l2edit.map_cos = 1;
        l2edit.dynamic = 1;

        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, &l2_edit_ptr));

        CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W,
            l2_edit_ptr, &l2edit), ret, error_proc);
        *p_data = l2_edit_ptr;
    }
    return CTC_E_NONE;
error_proc:
    if (nh_type == SYS_NH_ENTRY_TYPE_FWD)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, fwd_ptr);
    }
    else if (nh_type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, l2_edit_ptr);
    }

    return ret;
}

