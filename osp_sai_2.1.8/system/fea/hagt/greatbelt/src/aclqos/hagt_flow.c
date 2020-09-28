/*********************************************************************
 * hagt_flow.c:  Hal agent flow related functions
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Xing'an Yan
 * Date         : September 8, 2010
 * Reason       : Humber system.
*********************************************************************/
#include "sal.h"
#include "sal_types.h"
#include "glb_macro.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "ctc_api.h"
#include "hagt_error.h"
#include "hagt_debug.h"
#include "hagt_flow.h"
#include "hagt_flow_priv.h"
/*********************************************************************
 *  
 * Functions
 *
*********************************************************************/


/*********************************************************************
 * Name    : _hagt_flow_convert_ipv6_prefix_length_to_mask
 * Purpose : convert ipv6 prefix length to mask, call Bay SDK APIs
 * Input   : len - ipv6 prefix length 
 * Output  : mask - mask of ipv6 address
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32 
_hagt_flow_convert_ipv6_prefix_length_to_mask(uint8 len, uint32 *mask)
{
    if (len == 0)
    {
        mask[3] = 0;
        mask[2] = 0;
        mask[1] = 0;
        mask[0] = 0;
    }
    else if (len <= 32)
    {
        mask[3] =0;
        mask[2] =0;
        mask[1] =0;
        mask[0] =0xFFFFFFFF << (32 - len);
    }
    else if (len <= 64)
    {
        mask[3] =0;
        mask[2] =0;
        mask[1] =0xFFFFFFFF << (64 - len);
        mask[0] =0xFFFFFFFF;
    }
    else if (len <= 96)
    {
        mask[3] =0;
        mask[2] =0xFFFFFFFF << (96 - len);
        mask[1] =0xFFFFFFFF;
        mask[0] =0xFFFFFFFF;
    }
    else if (len <= 128)
    {
        mask[3] =0xFFFFFFFF << (128 - len);
        mask[2] =0xFFFFFFFF;
        mask[1] =0xFFFFFFFF;
        mask[0] =0xFFFFFFFF;
    }
    else
    {
        return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}


/*********************************************************************
 * Name    : hagt_flow_map_mac_key
 * Purpose : Map mac ace from message to ctc key
 * Input   : p_from_mac    -  message mac key pointer
 * Output  : p_to_mac      -  ctc mac key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_mac_key(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_mac_key_t* p_to_mac)
{
    uint8 i;
    
    HAGT_DEBUG_FLOW("  mac key :");
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_mac->mac_sa[i] = p_from_mac->macSa.buf[i];
            /* reverse the mask*/
            p_to_mac->mac_sa_mask[i] = ~p_from_mac->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mac->mac_sa[0], p_to_mac->mac_sa[1], p_to_mac->mac_sa[2],
            p_to_mac->mac_sa[3], p_to_mac->mac_sa[4], p_to_mac->mac_sa[5],
            p_to_mac->mac_sa_mask[0], p_to_mac->mac_sa_mask[1], p_to_mac->mac_sa_mask[2],
            p_to_mac->mac_sa_mask[3], p_to_mac->mac_sa_mask[4], p_to_mac->mac_sa_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_mac->mac_da[i] = p_from_mac->macDa.buf[i];
            /* reverse the mask*/
            p_to_mac->mac_da_mask[i] = ~p_from_mac->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mac->mac_da[0], p_to_mac->mac_da[1], p_to_mac->mac_da[2],
            p_to_mac->mac_da[3], p_to_mac->mac_da[4], p_to_mac->mac_da[5],
            p_to_mac->mac_da_mask[0], p_to_mac->mac_da_mask[1], p_to_mac->mac_da_mask[2],
            p_to_mac->mac_da_mask[3], p_to_mac->mac_da_mask[4], p_to_mac->mac_da_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_CVLAN);
        p_to_mac->cvlan = p_from_mac->cVlanId;
        p_to_mac->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    cvlan = %d", p_to_mac->cvlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_SVLAN);
        p_to_mac->svlan = p_from_mac->sVlanId;
        p_to_mac->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    svlan = %d", p_to_mac->svlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_CTAG_COS);
        p_to_mac->ctag_cos = p_from_mac->cTagCos;
        p_to_mac->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    ctag_cos = %d", p_to_mac->ctag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_STAG_COS);
        p_to_mac->stag_cos = p_from_mac->sTagCos;
        p_to_mac->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    stag_cos = %d", p_to_mac->stag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L2TYPE))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_L2_TYPE);
        p_to_mac->l2_type = p_from_mac->l2Type;
        p_to_mac->l2_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    l2_type = %s", GLB_FLOW_L2TYPE_TO_STR(p_to_mac->l2_type));
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L3TYPE))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_L3_TYPE);
        p_to_mac->l3_type = p_from_mac->l3Type;
        p_to_mac->l3_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    l3_type = %s", GLB_FLOW_L3TYPE_TO_STR(p_to_mac->l3_type));
    }
    /*Added by yanxa, for bug 17674, 2012-03-06*/
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_ETH_TYPE);
       // p_to_mac->eth_type_mask_valid = 1;
        p_to_mac->eth_type = p_from_mac->ethType;
        /* reverse the mask*/
        p_to_mac->eth_type_mask = (~p_from_mac->ethTypeMask) & 0xffff;
        HAGT_DEBUG_FLOW("    ether_type = %d", p_to_mac->eth_type);
        HAGT_DEBUG_FLOW("    mask = %d", p_to_mac->eth_type_mask);
    }

    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_ARP_OP_CODE))
    {
    	  GLB_SET_FLAG(p_to_mac->flag, CTC_ACL_MAC_KEY_FLAG_ARP_OP_CODE);
        p_to_mac->arp_op_code = p_from_mac->arpOpCode;
        p_to_mac->arp_op_code_mask = 0xffff;
	  HAGT_DEBUG_FLOW("  arp_op_code = %d", p_to_mac->arp_op_code);
    }

    if (!p_from_mac->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_ipv4_key
 * Purpose : Map ipv4 key from message to ctc 
 * Input   : p_from_ipv4   -  message ipv4 key pointer
 * Output  : p_to_ipv4     -  ctc ipv4 key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_pbr_ipv4_key(HalMsgFlowIpv4Rule_t* p_from_ipv4, ctc_acl_pbr_ipv4_key_t* p_to_ipv4)
{
    uint8* p_ip = NULL;
    uint8* p_ip_mask = NULL;

    (void)p_ip;
    (void)p_ip_mask;
    
    HAGT_DEBUG_FLOW("  ipv4 key :");
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_DA);
        p_to_ipv4->ip_da = ntohl(p_from_ipv4->ipDa);
        p_to_ipv4->ip_da_mask = ntohl(~p_from_ipv4->ipDaMask);
        p_ip = (uint8 *) &p_to_ipv4->ip_da;
        p_ip_mask = (uint8 *) &p_to_ipv4->ip_da_mask;
        HAGT_DEBUG_FLOW("    ip_da=%d.%d.%d.%d, ip_da_mask=%d.%d.%d.%d", 
                        p_ip[3], p_ip[2], p_ip[1], p_ip[0],
                        p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_SA);
        p_to_ipv4->ip_sa = ntohl(p_from_ipv4->ipSa);
        p_to_ipv4->ip_sa_mask = ntohl(~p_from_ipv4->ipSaMask);
        p_ip = (uint8 *) &p_to_ipv4->ip_sa;
        p_ip_mask = (uint8 *) &p_to_ipv4->ip_sa_mask;
        HAGT_DEBUG_FLOW("    ip_sa=%d.%d.%d.%d, ip_sa_mask=%d.%d.%d.%d", 
                        p_ip[3], p_ip[2], p_ip[1], p_ip[0],
                        p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_PBR_IPV4_KEY_FLAG_DSCP);
        p_to_ipv4->dscp = p_from_ipv4->dscp;
        p_to_ipv4->dscp_mask = 0xFF;
        HAGT_DEBUG_FLOW("    dscp=%d", p_to_ipv4->dscp);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_PBR_IPV4_KEY_FLAG_L4_PROTOCOL);
        p_to_ipv4->l4_protocol = p_from_ipv4->l4Protocol;
        p_to_ipv4->l4_protocol_mask = 0xFF;
        HAGT_DEBUG_FLOW("    l4_protocol=%s", GLB_FLOW_L4PROTO_TO_STR(p_to_ipv4->l4_protocol));

        if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_SRC_PORT))
        {
            GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
            if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv4->l4SrcPort.port == p_from_ipv4->l4SrcPort.port2)
            {
                p_to_ipv4->l4_src_port_use_mask = 1;
                p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port;
                p_to_ipv4->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv4->l4_src_port_use_mask = 0;
                p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port;
                p_to_ipv4->l4_src_port_1 = p_from_ipv4->l4SrcPort.port2;
            }
            if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv4->l4_src_port_use_mask = 0;
                p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port + 1;
                p_to_ipv4->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv4->l4_src_port_use_mask = 0;
                p_to_ipv4->l4_src_port_0 = 0;
                p_to_ipv4->l4_src_port_1 = p_from_ipv4->l4SrcPort.port - 1;
            }
            HAGT_DEBUG_FLOW("    l4_src_port port=%d, op=%s, port2=%d", 
                p_to_ipv4->l4_src_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv4->l4SrcPort.operator),
                p_to_ipv4->l4_src_port_1);
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_DST_PORT))
        {
            GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
            if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv4->l4DestPort.port == p_from_ipv4->l4DestPort.port2)
            {
                p_to_ipv4->l4_dst_port_use_mask = 1;
                p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port;
                p_to_ipv4->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv4->l4_dst_port_use_mask = 0;
                p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port;
                p_to_ipv4->l4_dst_port_1 = p_from_ipv4->l4DestPort.port2;
            }
            if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv4->l4_dst_port_use_mask = 0;
                p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port + 1;
                p_to_ipv4->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv4->l4_dst_port_use_mask = 0;
                p_to_ipv4->l4_dst_port_0 = 0;
                p_to_ipv4->l4_dst_port_1 = p_from_ipv4->l4DestPort.port - 1;
            }
            
            HAGT_DEBUG_FLOW("    l4_dst_port port=%d, op=%s, port2=%d", 
                p_to_ipv4->l4_dst_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv4->l4DestPort.operator),
                p_to_ipv4->l4_dst_port_1);
        }

        if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_TCP_FLAG))
        {
            GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_TCP_FLAGS);

            p_to_ipv4->tcp_flags_match_any = HAGT_FLOW_MAP_TCP_FLAG_OPERATOR(p_from_ipv4->tcpFlag.operator);
            HAGT_DEBUG_FLOW("    tcp_flag op=%s ", GLB_FLOW_TCP_FLAG_OP_TO_STR(p_from_ipv4->tcpFlag.operator));        
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_URG))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_URG_FLAG);
                HAGT_DEBUG_FLOW("      urg");
            }
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_ACK))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_ACK_FLAG);
                HAGT_DEBUG_FLOW("      ack");
            }
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_PSH))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_PSH_FLAG);
                HAGT_DEBUG_FLOW("      psh");
            }
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_RST))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_RST_FLAG);
                HAGT_DEBUG_FLOW("      rst");
            }
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_SYN))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_SYN_FLAG);
                HAGT_DEBUG_FLOW("      syn");
            }
            if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_FIN))
            {
                GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_FIN_FLAG);
                HAGT_DEBUG_FLOW("      fin");
            }
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ICMP_TYPE))
        {
            GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_TYPE);
            p_to_ipv4->icmp_type = p_from_ipv4->icmpType;
            p_to_ipv4->icmp_type_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_type=%d", p_to_ipv4->icmp_type);
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ICMP_CODE))
        {
            GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_CODE);
            p_to_ipv4->icmp_code = p_from_ipv4->icmpCode;
            p_to_ipv4->icmp_code_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_code=%d", p_to_ipv4->icmp_code);
        }
    }  
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_FRAG);
        p_to_ipv4->ip_frag = p_from_ipv4->ipFrag;
        HAGT_DEBUG_FLOW("    ip-fragment=%d", p_to_ipv4->ip_frag);
    }
    
    if (!p_from_ipv4->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_pbr_ipv6_key
 * Purpose : Map ipv6 key from message to ctc 
 * Input   : p_from_ipv6   -  message ipv6 key pointer
 * Output  : p_to_ipv6     -  ctc ipv6 key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_pbr_ipv6_key(HalMsgFlowIpv6Rule_t* p_from_ipv6, ctc_acl_pbr_ipv6_key_t* p_to_ipv6)
{
    uint8 i;
    
    HAGT_DEBUG_FLOW("  ipv6 key :");
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_SA);
        sal_memcpy(p_to_ipv6->ip_sa, p_from_ipv6->sprefix.buf, sizeof(ipv6_addr_t));
        for(i = 0; i < p_from_ipv6->sprefix.size/4; i++)
        {
            p_to_ipv6->ip_sa[i] = ntohl(p_to_ipv6->ip_sa[i]);
        }
        HAGT_IF_ERROR_RETURN(_hagt_flow_convert_ipv6_prefix_length_to_mask(p_from_ipv6->sprefixLen, p_to_ipv6->ip_sa_mask));
        HAGT_DEBUG_FLOW("    ip_sa=0x%08X:%08X:%08X:%08X, ipsa_mask=0x%08X:%08X:%08X:%08X",
            p_to_ipv6->ip_sa[0], p_to_ipv6->ip_sa[1], p_to_ipv6->ip_sa[2], p_to_ipv6->ip_sa[3],
            p_to_ipv6->ip_sa_mask[0], p_to_ipv6->ip_sa_mask[1], p_to_ipv6->ip_sa_mask[2], p_to_ipv6->ip_sa_mask[3]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_DA);
        sal_memcpy(p_to_ipv6->ip_da, p_from_ipv6->dprefix.buf, sizeof(ipv6_addr_t));
        for(i = 0; i < p_from_ipv6->dprefix.size/4; i++)
        {
            p_to_ipv6->ip_da[i] = ntohl(p_to_ipv6->ip_da[i]);
        }
        HAGT_IF_ERROR_RETURN(_hagt_flow_convert_ipv6_prefix_length_to_mask(p_from_ipv6->dprefixLen, p_to_ipv6->ip_da_mask));
        HAGT_DEBUG_FLOW("    ip_da=0x%08X:%08X:%08X:%08X, ipda_mask=0x%08X:%08X:%08X:%08X",
            p_to_ipv6->ip_da[0], p_to_ipv6->ip_sa[1], p_to_ipv6->ip_da[2], p_to_ipv6->ip_da[3],
            p_to_ipv6->ip_da_mask[0], p_to_ipv6->ip_da_mask[1], p_to_ipv6->ip_da_mask[2], p_to_ipv6->ip_da_mask[3]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_L4_PROTOCOL);
        p_to_ipv6->l4_protocol = p_from_ipv6->l4Protocol;
        p_to_ipv6->l4_protocol_mask = 0xff;
        HAGT_DEBUG_FLOW("    l4_protocol=%s", GLB_FLOW_L4PROTO_TO_STR(p_to_ipv6->l4_protocol));
        
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_SRC_PORT))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv6->l4SrcPort.port == p_from_ipv6->l4SrcPort.port2)
            {
                p_to_ipv6->l4_src_port_use_mask = 1;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port;
                p_to_ipv6->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port;
                p_to_ipv6->l4_src_port_1 = p_from_ipv6->l4SrcPort.port2;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port + 1;
                p_to_ipv6->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = 0;
                p_to_ipv6->l4_src_port_1 = p_from_ipv6->l4SrcPort.port - 1;
            }
            HAGT_DEBUG_FLOW("    l4_src_port port=%d, op=%s, port2=%d", 
                p_to_ipv6->l4_src_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv6->l4SrcPort.operator),
                p_to_ipv6->l4_src_port_1);
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_DST_PORT))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv6->l4DestPort.port == p_from_ipv6->l4DestPort.port2)
            {
                p_to_ipv6->l4_dst_port_use_mask = 1;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port;
                p_to_ipv6->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port;
                p_to_ipv6->l4_dst_port_1 = p_from_ipv6->l4DestPort.port2;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port + 1;
                p_to_ipv6->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = 0;
                p_to_ipv6->l4_dst_port_1 = p_from_ipv6->l4DestPort.port - 1;
            }
            
            HAGT_DEBUG_FLOW("    l4_dst_port port=%d, op=%s, port2=%d", 
                p_to_ipv6->l4_dst_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv6->l4DestPort.operator),
                p_to_ipv6->l4_dst_port_1);
        }

        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_TCP_FLAG))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_TCP_FLAGS);

            p_to_ipv6->tcp_flags_match_any = HAGT_FLOW_MAP_TCP_FLAG_OPERATOR(p_from_ipv6->tcpFlag.operator);
            HAGT_DEBUG_FLOW("    tcp_flag op=%s ", GLB_FLOW_TCP_FLAG_OP_TO_STR(p_from_ipv6->tcpFlag.operator));        
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_URG))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_URG_FLAG);
                HAGT_DEBUG_FLOW("      urg");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_ACK))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_ACK_FLAG);
                HAGT_DEBUG_FLOW("      ack");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_PSH))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_PSH_FLAG);
                HAGT_DEBUG_FLOW("      psh");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_RST))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_RST_FLAG);
                HAGT_DEBUG_FLOW("      rst");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_SYN))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_SYN_FLAG);
                HAGT_DEBUG_FLOW("      syn");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_FIN))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_FIN_FLAG);
                HAGT_DEBUG_FLOW("      fin");
            }
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_ICMP_TYPE))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_TYPE);
            p_to_ipv6->icmp_type = p_from_ipv6->icmpType;
            p_to_ipv6->icmp_type_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_type=%d", p_to_ipv6->icmp_type);
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_ICMP_CODE))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_CODE);
            p_to_ipv6->icmp_code = p_from_ipv6->icmpCode;
            p_to_ipv6->icmp_code_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_code=%d", p_to_ipv6->icmp_code);
        }
    }
 
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_DSCP);
        p_to_ipv6->dscp = p_from_ipv6->dscp;
        p_to_ipv6->dscp_mask = 0xff;
        HAGT_DEBUG_FLOW("    dscp=%d", p_to_ipv6->dscp);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_FLOW_LABEL))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_FLOW_LABEL);
        p_to_ipv6->flow_label = p_from_ipv6->flowLabel;
        p_to_ipv6->flow_label_mask = 0xffffffff;
        HAGT_DEBUG_FLOW("    flow_label=%d", p_to_ipv6->flow_label);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_FRAG);
        p_to_ipv6->ip_frag = p_from_ipv6->ipFrag;
        HAGT_DEBUG_FLOW("    ip-fragment=%d", p_to_ipv6->ip_frag);
    }
    if (!p_from_ipv6->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_ipv4_key
 * Purpose : Map ipv4 key from message to ctc 
 * Input   : p_from_ipv4   -  message ipv4 key pointer
 * Output  : p_to_ipv4     -  ctc ipv4 key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_ipv4_key(HalMsgFlowIpv4Rule_t* p_from_ipv4, ctc_acl_ipv4_key_t* p_to_ipv4)
{
    uint8* p_ip = NULL;
    uint8* p_ip_mask = NULL;
    uint32 i;
    
    (void)p_ip;
    (void)p_ip_mask;
    
    HAGT_DEBUG_FLOW("  ipv4 key :");
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);
        p_to_ipv4->ip_sa = ntohl(p_from_ipv4->ipSa);
        p_to_ipv4->ip_sa_mask = ntohl(~p_from_ipv4->ipSaMask);
        p_ip = (uint8 *) &p_to_ipv4->ip_sa;
        p_ip_mask = (uint8 *) &p_to_ipv4->ip_sa_mask;
        HAGT_DEBUG_FLOW("    ip_sa=%d.%d.%d.%d, ip_sa_mask=%d.%d.%d.%d", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0],
            p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);

    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA);
        p_to_ipv4->ip_da = ntohl(p_from_ipv4->ipDa);
        p_to_ipv4->ip_da_mask = ntohl(~p_from_ipv4->ipDaMask);
        p_ip = (uint8 *) &p_to_ipv4->ip_da;
        p_ip_mask = (uint8 *) &p_to_ipv4->ip_da_mask;
        HAGT_DEBUG_FLOW("    ip_da=%d.%d.%d.%d, ip_da_mask=%d.%d.%d.%d", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0],
            p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL);
        p_to_ipv4->l4_protocol = p_from_ipv4->l4Protocol;
        p_to_ipv4->l4_protocol_mask = 0xff; 
        HAGT_DEBUG_FLOW("    l4_protocol=%s", GLB_FLOW_L4PROTO_TO_STR(p_to_ipv4->l4_protocol));
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_SRC_PORT))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
        if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ)
        {
            p_to_ipv4->l4_src_port_use_mask = 1;
            p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port;
            p_to_ipv4->l4_src_port_1 = 0xffff;
        }
        if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
        {
            p_to_ipv4->l4_src_port_use_mask = 0;
            p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port;
            p_to_ipv4->l4_src_port_1 = p_from_ipv4->l4SrcPort.port2;

            /*modify by chenxw for bug 24264, 2013-08-29.*/
            if (p_from_ipv4->l4SrcPort.port == p_from_ipv4->l4SrcPort.port2)
            {
                p_to_ipv4->l4_src_port_use_mask = 1;
                p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port;
                p_to_ipv4->l4_src_port_1 = 0xffff;                                
            }             
        }
        if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
        {
            p_to_ipv4->l4_src_port_use_mask = 0;
            p_to_ipv4->l4_src_port_0 = p_from_ipv4->l4SrcPort.port + 1;
            p_to_ipv4->l4_src_port_1 = 0xffff;
        }
        if(p_from_ipv4->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
        {
            p_to_ipv4->l4_src_port_use_mask = 0;
            p_to_ipv4->l4_src_port_0 = 0;
            p_to_ipv4->l4_src_port_1 = p_from_ipv4->l4SrcPort.port - 1;
        }
        HAGT_DEBUG_FLOW("    l4_src_port port=%d, op=%s, port2=%d", 
            p_to_ipv4->l4_src_port_0,
            GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv4->l4SrcPort.operator),
            p_to_ipv4->l4_src_port_1);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_L4_DST_PORT))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
        if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ)
        {
            p_to_ipv4->l4_dst_port_use_mask = 1;
            p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port;
            p_to_ipv4->l4_dst_port_1 = 0xffff;
        }
        if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
        {
            p_to_ipv4->l4_dst_port_use_mask = 0;
            p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port;
            p_to_ipv4->l4_dst_port_1 = p_from_ipv4->l4DestPort.port2;

            /*modify by chenxw for bug 24264, 2013-08-29.*/
            if (p_from_ipv4->l4DestPort.port == p_from_ipv4->l4DestPort.port2)
            {
                p_to_ipv4->l4_dst_port_use_mask = 1;
                p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port;
                p_to_ipv4->l4_dst_port_1 = 0xffff;                
            }
        }
        if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
        {
            p_to_ipv4->l4_dst_port_use_mask = 0;
            p_to_ipv4->l4_dst_port_0 = p_from_ipv4->l4DestPort.port + 1;
            p_to_ipv4->l4_dst_port_1 = 0xffff;
        }
        if(p_from_ipv4->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
        {
            p_to_ipv4->l4_dst_port_use_mask = 0;
            p_to_ipv4->l4_dst_port_0 = 0;
            p_to_ipv4->l4_dst_port_1 = p_from_ipv4->l4DestPort.port - 1;
        }
        
        HAGT_DEBUG_FLOW("    l4_dst_port port=%d, op=%s, port2=%d", 
            p_to_ipv4->l4_dst_port_0,
            GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv4->l4DestPort.operator),
            p_to_ipv4->l4_dst_port_1);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_TCP_FLAG))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS);

        p_to_ipv4->tcp_flags_match_any = HAGT_FLOW_MAP_TCP_FLAG_OPERATOR(p_from_ipv4->tcpFlag.operator); 
        
        HAGT_DEBUG_FLOW("    tcp_flag op=%s ", GLB_FLOW_TCP_FLAG_OP_TO_STR(p_from_ipv4->tcpFlag.operator));
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_URG))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_URG_FLAG);
            HAGT_DEBUG_FLOW("      urg");
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_ACK))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_ACK_FLAG);
            HAGT_DEBUG_FLOW("      ack");
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_PSH))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_PSH_FLAG);
            HAGT_DEBUG_FLOW("      psh");
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_RST))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_RST_FLAG);
            HAGT_DEBUG_FLOW("      rst");
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_SYN))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_SYN_FLAG);
            HAGT_DEBUG_FLOW("      syn");
        }
        if (GLB_FLAG_ISSET(p_from_ipv4->tcpFlag.flag, GLB_FLOW_TCP_FLAG_FIN))
        {
            GLB_SET_FLAG(p_to_ipv4->tcp_flags, CTC_ACL_TCP_FIN_FLAG);
            HAGT_DEBUG_FLOW("      fin");
        }
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ICMP_TYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE);
        p_to_ipv4->icmp_type = p_from_ipv4->icmpType;
        p_to_ipv4->icmp_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    icmp_type=%d", p_to_ipv4->icmp_type);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IGMP_TYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE);
        p_to_ipv4->igmp_type = p_from_ipv4->igmpType;
        p_to_ipv4->igmp_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    igmp_type=%d", p_to_ipv4->igmp_type);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ICMP_CODE))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE);
        p_to_ipv4->icmp_code = p_from_ipv4->icmpCode;
        p_to_ipv4->icmp_code_mask = 0xff;
        HAGT_DEBUG_FLOW("    icmp_code=%d", p_to_ipv4->icmp_code);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_DSCP);
        p_to_ipv4->dscp = p_from_ipv4->dscp;
        p_to_ipv4->dscp_mask = 0xff;
        HAGT_DEBUG_FLOW("    dscp=%d", p_to_ipv4->dscp);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG);
        p_to_ipv4->ip_frag = p_from_ipv4->ipFrag;
        HAGT_DEBUG_FLOW("    ip-fragment=%d", p_to_ipv4->ip_frag);
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_IP_OPTIONS))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_OPTION);
        p_to_ipv4->ip_option = 1;
        HAGT_DEBUG_FLOW("    ip-option");
    }
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ROUTED_PACKET))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET);
        p_to_ipv4->routed_packet = p_from_ipv4->routedPacket;
        HAGT_DEBUG_FLOW("    routed_packet=%d", p_to_ipv4->routed_packet);
    }

#if 1 /* SYSTEM MODIFIED: mac acl support add by wangqj at 2016/9/6 */
    /* mac */
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS);
        p_to_ipv4->stag_cos = p_from_ipv4->scos;
        p_to_ipv4->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    scos=%d", p_to_ipv4->scos);
    }

    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
        p_to_ipv4->svlan = p_from_ipv4->svlan;
        p_to_ipv4->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    svlan=%d", p_to_ipv4->svlan);
    }

    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS);
        p_to_ipv4->ctag_cos = p_from_ipv4->ccos;
        p_to_ipv4->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    ccos=%d", p_to_ipv4->ccos);
    }

    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN);
        p_to_ipv4->cvlan = p_from_ipv4->cvlan;
        p_to_ipv4->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    cvlan=%d", p_to_ipv4->cvlan);
    }
    
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
        p_to_ipv4->eth_type      = p_from_ipv4->ethType;
        p_to_ipv4->eth_type_mask = ~p_from_ipv4->ethTypeMask;
        HAGT_DEBUG_FLOW("    eth_type=%d", p_to_ipv4->eth_type);
        HAGT_DEBUG_FLOW("    eth_type_mask=%d", p_to_ipv4->eth_type_mask);
    }

    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_ipv4->macSaMask.size; i++)
        {
            p_to_ipv4->mac_sa[i] = p_from_ipv4->macSa.buf[i];
            p_to_ipv4->mac_sa_mask[i] = ~p_from_ipv4->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv4->mac_sa[0], p_to_ipv4->mac_sa[1], p_to_ipv4->mac_sa[2],
            p_to_ipv4->mac_sa[3], p_to_ipv4->mac_sa[4], p_to_ipv4->mac_sa[5],
            p_to_ipv4->mac_sa_mask[0], p_to_ipv4->mac_sa_mask[1], p_to_ipv4->mac_sa_mask[2],
            p_to_ipv4->mac_sa_mask[3], p_to_ipv4->mac_sa_mask[4], p_to_ipv4->mac_sa_mask[5]);
    }

    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_ipv4->macDaMask.size; i++)
        {
            p_to_ipv4->mac_da[i] = p_from_ipv4->macDa.buf[i];
            p_to_ipv4->mac_da_mask[i] = ~p_from_ipv4->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv4->mac_da[0], p_to_ipv4->mac_da[1], p_to_ipv4->mac_da[2],
            p_to_ipv4->mac_da[3], p_to_ipv4->mac_da[4], p_to_ipv4->mac_da[5],
            p_to_ipv4->mac_da_mask[0], p_to_ipv4->mac_da_mask[1], p_to_ipv4->mac_da_mask[2],
            p_to_ipv4->mac_da_mask[3], p_to_ipv4->mac_da_mask[4], p_to_ipv4->mac_da_mask[5]);
    }
#endif


/* modified by cuixl for bug 27208, 2014-03-31 */
/* modification start */
/* CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET is not suppported on GG, so comment off*/   
#if 0
    /*ipv4 acl is only used to filter ipv4 packets*/
    GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
    p_to_ipv4->ipv4_packet = 1;
#else
    /* we use 320Bits even only ipv4 key*/  
    p_to_ipv4->key_size = CTC_ACL_KEY_SIZE_DOUBLE;
    /* for ipv4 packet, we set layer3 type as CTC_ACL_IPV4_KEY_FLAG_L3_TYPE*/
    GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
    p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_IPV4;
    p_to_ipv4->l3_type_mask = 0xFF;    
#endif    
/* modification end */

#if 1 /* SYSTEM MODIFIED: mac acl support add by wangqj at 2016/9/29 */
    /* default unset l3 type */
    GLB_UNSET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);

    if((GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA) && (0 != p_to_ipv4->ip_sa_mask)) ||
       (GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA) && (0 != p_to_ipv4->ip_da_mask)) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_DSCP) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_PRECEDENCE) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IP_OPTION) || 
       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET) || 


       GLB_FLAG_ISSET(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_DSCP) ||
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE) ||
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE) ||
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE) ||
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS) ||
       
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT) ||
       GLB_FLAG_ISSET(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT)
    )
    {
        p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_IPV4;
        p_to_ipv4->l3_type_mask = 0xf;    
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
    }
#endif

#ifdef GOLDENGATE
#if 1 /* SYSTEM MODIFIED: mac acl support add by wangqj at 2016/9/6 */ /* TODO: select l3 type */
    if (GLB_FLAG_ISSET(p_from_ipv4->flag, GLB_FLOW_IPV4_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        /* clear ether_type flag */
        GLB_UNSET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);

        if (0x0800 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_IPV4;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x0806 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_ARP;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x86DD == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_IPV6;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x8847 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_MPLS;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x8848 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_MPLS_MCAST;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x8906 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_FCOE;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x22F3 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_TRILL;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x8902 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_ETHER_OAM;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x88E7 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_CMAC;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x8809 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_SLOW_PROTO;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else if (0x88F7 == p_to_ipv4->eth_type 
            && 0xffff == p_to_ipv4->eth_type_mask)
        {
            p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_PTP;
            p_to_ipv4->l3_type_mask = 0xf;
        }
        else
        {
            if (0 == p_to_ipv4->eth_type_mask)
            {
                CTC_UNSET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
                CTC_UNSET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
            }
            else
            {
                p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_NONE;
                p_to_ipv4->l3_type_mask = 0xf;
                CTC_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
            }
        }

    }
#endif
#endif

    if (!p_from_ipv4->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}


/*********************************************************************
 * Name    : hagt_flow_map_ipv6_key
 * Purpose : Map ipv6 key from message to ctc
 * Input   : p_from_ipv6  -  message ipv6 key pointer
 * Output  : p_to_ipv6    -  ctc ipv6 key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_ipv6_key(HalMsgFlowIpv6Rule_t* p_from_ipv6, ctc_acl_ipv6_key_t* p_to_ipv6)
{
    uint8 i;

    HAGT_DEBUG_FLOW("  ipv6 key:");
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA);
        sal_memcpy(p_to_ipv6->ip_sa, p_from_ipv6->sprefix.buf, sizeof(ipv6_addr_t));
        for(i = 0; i < p_from_ipv6->sprefix.size/4; i++)
        {
            p_to_ipv6->ip_sa[i] = ntohl(p_to_ipv6->ip_sa[i]);
        }
        HAGT_IF_ERROR_RETURN(_hagt_flow_convert_ipv6_prefix_length_to_mask(p_from_ipv6->sprefixLen, p_to_ipv6->ip_sa_mask));
        HAGT_DEBUG_FLOW("    ip_sa=0x%08X:%08X:%08X:%08X, ipsa_mask=0x%08X:%08X:%08X:%08X",
            p_to_ipv6->ip_sa[0], p_to_ipv6->ip_sa[1], p_to_ipv6->ip_sa[2], p_to_ipv6->ip_sa[3],
            p_to_ipv6->ip_sa_mask[0], p_to_ipv6->ip_sa_mask[1], p_to_ipv6->ip_sa_mask[2], p_to_ipv6->ip_sa_mask[3]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA);
        sal_memcpy(p_to_ipv6->ip_da, p_from_ipv6->dprefix.buf, sizeof(ipv6_addr_t));
        for(i = 0; i < p_from_ipv6->dprefix.size/4; i++)
        {
            p_to_ipv6->ip_da[i] = ntohl(p_to_ipv6->ip_da[i]);
        }
        HAGT_IF_ERROR_RETURN(_hagt_flow_convert_ipv6_prefix_length_to_mask(p_from_ipv6->dprefixLen, p_to_ipv6->ip_da_mask));
        HAGT_DEBUG_FLOW("    ip_da=0x%08X:%08X:%08X:%08X, ipda_mask=0x%08X:%08X:%08X:%08X",
            p_to_ipv6->ip_da[0], p_to_ipv6->ip_sa[1], p_to_ipv6->ip_da[2], p_to_ipv6->ip_da[3],
            p_to_ipv6->ip_da_mask[0], p_to_ipv6->ip_da_mask[1], p_to_ipv6->ip_da_mask[2], p_to_ipv6->ip_da_mask[3]);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL);
        p_to_ipv6->l4_protocol = p_from_ipv6->l4Protocol;
        p_to_ipv6->l4_protocol_mask = 0xff;
        HAGT_DEBUG_FLOW("    l4_protocol=%s", GLB_FLOW_L4PROTO_TO_STR(p_to_ipv6->l4_protocol));

        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_SRC_PORT))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv6->l4SrcPort.port == p_from_ipv6->l4SrcPort.port2)
            {
                p_to_ipv6->l4_src_port_use_mask = 1;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port;
                p_to_ipv6->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port;
                p_to_ipv6->l4_src_port_1 = p_from_ipv6->l4SrcPort.port2;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = p_from_ipv6->l4SrcPort.port + 1;
                p_to_ipv6->l4_src_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4SrcPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv6->l4_src_port_use_mask = 0;
                p_to_ipv6->l4_src_port_0 = 0;
                p_to_ipv6->l4_src_port_1 = p_from_ipv6->l4SrcPort.port - 1;
            }
            HAGT_DEBUG_FLOW("    l4_src_port port=%d, op=%s, port2=%d", 
                p_to_ipv6->l4_src_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv6->l4SrcPort.operator),
                p_to_ipv6->l4_src_port_1);
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_L4_DST_PORT))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_EQ
                || p_from_ipv6->l4DestPort.port == p_from_ipv6->l4DestPort.port2)
            {
                p_to_ipv6->l4_dst_port_use_mask = 1;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port;
                p_to_ipv6->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port;
                p_to_ipv6->l4_dst_port_1 = p_from_ipv6->l4DestPort.port2;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_GT)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = p_from_ipv6->l4DestPort.port + 1;
                p_to_ipv6->l4_dst_port_1 = 0xffff;
            }
            if(p_from_ipv6->l4DestPort.operator == GLB_FLOW_L4PORT_OPERATOR_LT)
            {
                p_to_ipv6->l4_dst_port_use_mask = 0;
                p_to_ipv6->l4_dst_port_0 = 0;
                p_to_ipv6->l4_dst_port_1 = p_from_ipv6->l4DestPort.port - 1;
            }
            
            HAGT_DEBUG_FLOW("    l4_dst_port port=%d, op=%s, port2=%d", 
                p_to_ipv6->l4_dst_port_0,
                GLB_FLOW_L4PORT_OP_TO_STR(p_from_ipv6->l4DestPort.operator),
                p_to_ipv6->l4_dst_port_1);
        }

        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_TCP_FLAG))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_TCP_FLAGS);

            p_to_ipv6->tcp_flags_match_any = HAGT_FLOW_MAP_TCP_FLAG_OPERATOR(p_from_ipv6->tcpFlag.operator);
            HAGT_DEBUG_FLOW("    tcp_flag op=%s ", GLB_FLOW_TCP_FLAG_OP_TO_STR(p_from_ipv6->tcpFlag.operator));        
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_URG))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_URG_FLAG);
                HAGT_DEBUG_FLOW("      urg");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_ACK))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_ACK_FLAG);
                HAGT_DEBUG_FLOW("      ack");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_PSH))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_PSH_FLAG);
                HAGT_DEBUG_FLOW("      psh");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_RST))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_RST_FLAG);
                HAGT_DEBUG_FLOW("      rst");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_SYN))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_SYN_FLAG);
                HAGT_DEBUG_FLOW("      syn");
            }
            if (GLB_FLAG_ISSET(p_from_ipv6->tcpFlag.flag, GLB_FLOW_TCP_FLAG_FIN))
            {
                GLB_SET_FLAG(p_to_ipv6->tcp_flags, CTC_ACL_TCP_FIN_FLAG);
                HAGT_DEBUG_FLOW("      fin");
            }
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_ICMP_TYPE))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_TYPE);
            p_to_ipv6->icmp_type = p_from_ipv6->icmpType;
            p_to_ipv6->icmp_type_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_type=%d", p_to_ipv6->icmp_type);
        }
        if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_ICMP_CODE))
        {
            GLB_SET_FLAG(p_to_ipv6->sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_CODE);
            p_to_ipv6->icmp_code = p_from_ipv6->icmpCode;
            p_to_ipv6->icmp_code_mask = 0xFF;
            HAGT_DEBUG_FLOW("    icmp_code=%d", p_to_ipv6->icmp_code);
        }
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_DSCP);
        p_to_ipv6->dscp = p_from_ipv6->dscp;
        p_to_ipv6->dscp_mask = 0xff;
        HAGT_DEBUG_FLOW("    dscp=%d", p_to_ipv6->dscp);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_FLOW_LABEL))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL);
        p_to_ipv6->flow_label = p_from_ipv6->flowLabel;
        p_to_ipv6->flow_label_mask = 0xffffffff;
        HAGT_DEBUG_FLOW("    flow_label=%d", p_to_ipv6->flow_label);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_IP_FRAG);
        p_to_ipv6->ip_frag = p_from_ipv6->ipFrag;
        HAGT_DEBUG_FLOW("    ip-fragment=%d", p_to_ipv6->ip_frag);
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_IP_OPTION))
    {
        /*in GB ip-option is used for hop-by-hop header*/
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_IP_OPTION);
        p_to_ipv6->ip_option = p_from_ipv6->ipOptions;
        HAGT_DEBUG_FLOW("    ip-option");
    }
    if (GLB_FLAG_ISSET(p_from_ipv6->flag, GLB_FLOW_IPV6_RULE_FLAG_ROUTED_PACKET))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_ROUTED_PACKET);
        p_to_ipv6->routed_packet = p_from_ipv6->routedPacket;
        HAGT_DEBUG_FLOW("    routed_packet=%d", p_to_ipv6->routed_packet);
    }
    if (!p_from_ipv6->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_key_mac2ipv4
 * Purpose : Map key from mac key to ipv4 key
 * Input   : p_from_mac
 * Output  : p_to_ipv4
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_key_mac2ipv4(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_ipv4_key_t* p_to_ipv4, bool mac_acl)
{
    uint8 i;
    uint8* p_ip = NULL;
    uint8* p_ip_mask = NULL;
    
    (void)p_ip;
    (void)p_ip_mask;
    
    HAGT_DEBUG_FLOW("  mac-ipv4 key :");
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_ipv4->mac_sa[i] = p_from_mac->macSa.buf[i];
            p_to_ipv4->mac_sa_mask[i] = ~p_from_mac->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv4->mac_sa[0], p_to_ipv4->mac_sa[1], p_to_ipv4->mac_sa[2],
            p_to_ipv4->mac_sa[3], p_to_ipv4->mac_sa[4], p_to_ipv4->mac_sa[5],
            p_to_ipv4->mac_sa_mask[0], p_to_ipv4->mac_sa_mask[1], p_to_ipv4->mac_sa_mask[2],
            p_to_ipv4->mac_sa_mask[3], p_to_ipv4->mac_sa_mask[4], p_to_ipv4->mac_sa_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_mac->macDaMask.size; i++)
        {
            p_to_ipv4->mac_da[i] = p_from_mac->macDa.buf[i];
            p_to_ipv4->mac_da_mask[i] = ~p_from_mac->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv4->mac_da[0], p_to_ipv4->mac_da[1], p_to_ipv4->mac_da[2],
            p_to_ipv4->mac_da[3], p_to_ipv4->mac_da[4], p_to_ipv4->mac_da[5],
            p_to_ipv4->mac_da_mask[0], p_to_ipv4->mac_da_mask[1], p_to_ipv4->mac_da_mask[2],
            p_to_ipv4->mac_da_mask[3], p_to_ipv4->mac_da_mask[4], p_to_ipv4->mac_da_mask[5]);
    }

    /*modify by chenxw for bug 24076, 2013-09-04.*/
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_UNTAG))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);                
        p_to_ipv4->ctag_valid = 0;
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);                
        p_to_ipv4->stag_valid = 0;     
        HAGT_DEBUG_FLOW("  untag");
    }    
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN);
        p_to_ipv4->cvlan = p_from_mac->cVlanId;
        p_to_ipv4->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("  cvlan = %d", p_to_ipv4->cvlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
        p_to_ipv4->svlan = p_from_mac->sVlanId;
        p_to_ipv4->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("  svlan = %d", p_to_ipv4->svlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS);
        p_to_ipv4->ctag_cos = p_from_mac->cTagCos;
        p_to_ipv4->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("  ctag_cos = %d", p_to_ipv4->ctag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS);
        p_to_ipv4->stag_cos = p_from_mac->sTagCos;
        p_to_ipv4->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("  stag_cos = %d", p_to_ipv4->stag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L3TYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        p_to_ipv4->l3_type = p_from_mac->l3Type;
        p_to_ipv4->l3_type_mask = 0xff;
        HAGT_DEBUG_FLOW("  l3_type = %s", GLB_FLOW_L3TYPE_TO_STR(p_to_ipv4->l3_type));
    }
    else/*Added by xgu for GG sdk need always enable l3 type, 2014-11-6*/
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        p_to_ipv4->l3_type = 0;
        p_to_ipv4->l3_type_mask = 0;
        HAGT_DEBUG_FLOW("  l3_type = %s", GLB_FLOW_L3TYPE_TO_STR(p_to_ipv4->l3_type));
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L2TYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L2_TYPE);
        p_to_ipv4->l2_type = p_from_mac->l2Type;
        p_to_ipv4->l2_type_mask = 0xff;
        HAGT_DEBUG_FLOW("  l2_type = %s", GLB_FLOW_L2TYPE_TO_STR(p_to_ipv4->l2_type));
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L3TYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        p_to_ipv4->l3_type = p_from_mac->l3Type;
        p_to_ipv4->l3_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    l3_type = %s", GLB_FLOW_L3TYPE_TO_STR(p_to_ipv4->l3_type));
    }
    /*Added by yanxa, for bug 17674, 2012-03-06*/
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
        if ((p_from_mac->ethType & ((~p_from_mac->ethTypeMask)&0xffff)) == 
            (GLB_FLOW_ETHER_TYPE_IP & ((~p_from_mac->ethTypeMask)&0xffff)))
        {
            GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
            p_to_ipv4->ipv4_packet = 1;
        }
        else
        {
            GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
            p_to_ipv4->ipv4_packet = 0;
            p_to_ipv4->eth_type = p_from_mac->ethType;
            /* reverse the mask*/
            p_to_ipv4->eth_type_mask = (~p_from_mac->ethTypeMask) & 0xffff;
        }
        HAGT_DEBUG_FLOW("    ether_type = %d", p_to_ipv4->eth_type);
        HAGT_DEBUG_FLOW("    mask = %d", p_to_ipv4->eth_type_mask);
    
    }
    if(GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_IPV4_EXTEND_RULE_FLAG_ARP_PACKET))
    {
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET);
        p_to_ipv4->arp_packet = 1;
        HAGT_DEBUG_FLOW("    arp_packet = %d", p_to_ipv4->arp_packet);
    }
    else if(! mac_acl)
    {
/* modified by cuixl for bug 27208, 2014-04-10 */
/* modification start */
/* CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET is not suppported on GG, so comment off*/   
#if 0    
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
        p_to_ipv4->ipv4_packet = 1;
        HAGT_DEBUG_FLOW("    ipv4_packet = %d", p_to_ipv4->ipv4_packet);
#else
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        p_to_ipv4->l3_type = CTC_PARSER_L3_TYPE_IPV4;   
        p_to_ipv4->l3_type_mask = 0xFF;
#endif        
    }
    if(GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_ARP_OP_CODE))
    {
        /*mac merge to ipv4 need to set arp packet flag*/
        GLB_SET_FLAG(p_to_ipv4->flag, CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET);
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE);
        p_to_ipv4->arp_packet = 1;
        p_to_ipv4->ipv4_packet = 0;
        p_to_ipv4->arp_op_code = p_from_mac->arpOpCode;
        p_to_ipv4->arp_op_code_mask = 0xffff;
        HAGT_DEBUG_FLOW("    arp_packet = %d", p_to_ipv4->arp_packet);
        HAGT_DEBUG_FLOW("    ipv4_packet = %d", p_to_ipv4->ipv4_packet);
        HAGT_DEBUG_FLOW("    arp_op_code = %d", p_to_ipv4->arp_op_code);
        HAGT_DEBUG_FLOW("    arp_op_code_mask = %d", p_to_ipv4->arp_op_code_mask);
    }
    if(GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_IPV4_EXTEND_RULE_FLAG_SENDER_IP))	
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP);
        p_to_ipv4->sender_ip = ntohl(p_from_mac->senderIp);
        p_to_ipv4->sender_ip_mask = ntohl(~p_from_mac->senderIpMask);
        p_ip = (uint8 *) &p_to_ipv4->sender_ip;
        p_ip_mask = (uint8 *) &p_to_ipv4->sender_ip_mask;
        HAGT_DEBUG_FLOW("    sender_ip=%d.%d.%d.%d, sender_ip_mask=%d.%d.%d.%d", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0],
            p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);
    }
    if(GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_IPV4_EXTEND_RULE_FLAG_TARGET_IP))
    {
        GLB_SET_FLAG(p_to_ipv4->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP);
        p_to_ipv4->target_ip = ntohl(p_from_mac->targetIp);
        p_to_ipv4->target_ip_mask = ntohl(~p_from_mac->targetIpMask);
        p_ip = (uint8 *) &p_to_ipv4->target_ip;
        p_ip_mask = (uint8 *) &p_to_ipv4->target_ip_mask;
        HAGT_DEBUG_FLOW("    target_ip=%d.%d.%d.%d, target_ip_mask=%d.%d.%d.%d", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0],
            p_ip_mask[3], p_ip_mask[2], p_ip_mask[1], p_ip_mask[0]);
    }

/* modified by cuixl for bug 27208, 2014-03-28 */
/* put mac info to ipv4, we need use 320bits key_size*/
/* modification start */
    p_to_ipv4->key_size = CTC_ACL_KEY_SIZE_DOUBLE;
/* modification end */
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_key_mac2ipv6
 * Purpose : Map key from mac key to ipv6 key
 * Input   : p_from_mac
 * Output  : p_to_ipv6
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_key_mac2ipv6(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_ipv6_key_t* p_to_ipv6)
{
    uint8 i;
    
    HAGT_DEBUG_FLOW("  mac-ipv6 key :");
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_ipv6->mac_sa[i] = p_from_mac->macSa.buf[i];
            p_to_ipv6->mac_sa_mask[i] = ~p_from_mac->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv6->mac_sa[0], p_to_ipv6->mac_sa[1], p_to_ipv6->mac_sa[2],
            p_to_ipv6->mac_sa[3], p_to_ipv6->mac_sa[4], p_to_ipv6->mac_sa[5],
            p_to_ipv6->mac_sa_mask[0], p_to_ipv6->mac_sa_mask[1], p_to_ipv6->mac_sa_mask[2],
            p_to_ipv6->mac_sa_mask[3], p_to_ipv6->mac_sa_mask[4], p_to_ipv6->mac_sa_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_mac->macDaMask.size; i++)
        {
            p_to_ipv6->mac_da[i] = p_from_mac->macDa.buf[i];
            p_to_ipv6->mac_da_mask[i] = ~p_from_mac->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("  mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_ipv6->mac_da[0], p_to_ipv6->mac_da[1], p_to_ipv6->mac_da[2],
            p_to_ipv6->mac_da[3], p_to_ipv6->mac_da[4], p_to_ipv6->mac_da[5],
            p_to_ipv6->mac_da_mask[0], p_to_ipv6->mac_da_mask[1], p_to_ipv6->mac_da_mask[2],
            p_to_ipv6->mac_da_mask[3], p_to_ipv6->mac_da_mask[4], p_to_ipv6->mac_da_mask[5]);
    }
    
    /*modify by chenxw for bug 24076, 2013-09-04.*/    
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_UNTAG))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);                
        p_to_ipv6->ctag_valid = 0;
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);                
        p_to_ipv6->stag_valid = 0;     
        HAGT_DEBUG_FLOW("  untag");
    }    
    
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN);
        p_to_ipv6->cvlan = p_from_mac->cVlanId;
        p_to_ipv6->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("  cvlan = %d", p_to_ipv6->cvlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_SVLAN);
        p_to_ipv6->svlan = p_from_mac->sVlanId;
        p_to_ipv6->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("  svlan = %d", p_to_ipv6->svlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS);
        p_to_ipv6->ctag_cos = p_from_mac->cTagCos;
        p_to_ipv6->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("  ctag_cos = %d", p_to_ipv6->ctag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS);
        p_to_ipv6->stag_cos = p_from_mac->sTagCos;
        p_to_ipv6->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("  stag_cos = %d", p_to_ipv6->stag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L3TYPE))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE);
        p_to_ipv6->l3_type = p_from_mac->l3Type;
        p_to_ipv6->l3_type_mask = 0xff;
        HAGT_DEBUG_FLOW("  l3_type = %s", GLB_FLOW_L3TYPE_TO_STR(p_to_ipv6->l3_type));
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_L2TYPE))
    {
        GLB_SET_FLAG(p_to_ipv6->flag, CTC_ACL_IPV6_KEY_FLAG_L2_TYPE);
        p_to_ipv6->l2_type = p_from_mac->l2Type;
        p_to_ipv6->l2_type_mask = 0xff;
        HAGT_DEBUG_FLOW("  l2_type = %s", GLB_FLOW_L2TYPE_TO_STR(p_to_ipv6->l2_type));
    }

    return HAGT_E_NONE;
}

/*modified by yanxa, for bug 17674, 2012-03-06*/
/*********************************************************************
 * Name    : hagt_flow_map_mpls_key
 * Purpose : Map mpls ace from message to ctc key
 * Input   : p_from_mpls    -  message mpls key pointer
 * Output  : p_to_mpls      -  ctc mpls key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_key_mac2mpls(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_mpls_key_t* p_to_mpls)
{
    uint8 i;
    
    HAGT_DEBUG_FLOW("  mpls key :");
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_mpls->mac_sa[i] = p_from_mac->macSa.buf[i];
            /* reverse the mask*/
            p_to_mpls->mac_sa_mask[i] = ~p_from_mac->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mpls->mac_sa[0], p_to_mpls->mac_sa[1], p_to_mpls->mac_sa[2],
            p_to_mpls->mac_sa[3], p_to_mpls->mac_sa[4], p_to_mpls->mac_sa[5],
            p_to_mpls->mac_sa_mask[0], p_to_mpls->mac_sa_mask[1], p_to_mpls->mac_sa_mask[2],
            p_to_mpls->mac_sa_mask[3], p_to_mpls->mac_sa_mask[4], p_to_mpls->mac_sa_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_mac->macSaMask.size; i++)
        {
            p_to_mpls->mac_da[i] = p_from_mac->macDa.buf[i];
            /* reverse the mask*/
            p_to_mpls->mac_da_mask[i] = ~p_from_mac->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mpls->mac_da[0], p_to_mpls->mac_da[1], p_to_mpls->mac_da[2],
            p_to_mpls->mac_da[3], p_to_mpls->mac_da[4], p_to_mpls->mac_da[5],
            p_to_mpls->mac_da_mask[0], p_to_mpls->mac_da_mask[1], p_to_mpls->mac_da_mask[2],
            p_to_mpls->mac_da_mask[3], p_to_mpls->mac_da_mask[4], p_to_mpls->mac_da_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_CVLAN);
        p_to_mpls->cvlan = p_from_mac->cVlanId;
        p_to_mpls->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    cvlan = %d", p_to_mpls->cvlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_SVLAN);
        p_to_mpls->svlan = p_from_mac->sVlanId;
        p_to_mpls->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    svlan = %d", p_to_mpls->svlan);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_CTAG_COS);
        p_to_mpls->ctag_cos = p_from_mac->cTagCos;
        p_to_mpls->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    ctag_cos = %d", p_to_mpls->ctag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_STAG_COS);
        p_to_mpls->stag_cos = p_from_mac->sTagCos;
        p_to_mpls->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    stag_cos = %d", p_to_mpls->stag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mac->flag, GLB_FLOW_MPLS_RULE_FLAG_L2TYPE))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_L2_TYPE);
        p_to_mpls->l2_type = p_from_mac->l2Type;
        p_to_mpls->l2_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    l2_type = %s", GLB_FLOW_L2TYPE_TO_STR(p_to_mpls->l2_type));
    }

    if (!p_from_mac->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}
/*********************************************************************
 * Name    : hagt_flow_map_mpls_key
 * Purpose : Map mpls ace from message to ctc key
 * Input   : p_from_mpls    -  message mpls key pointer
 * Output  : p_to_mpls      -  ctc mpls key pointer
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_mpls_key(HalMsgFlowMplsRule_t* p_from_mpls, ctc_acl_mpls_key_t* p_to_mpls)
{
    uint8 i;
    
    HAGT_DEBUG_FLOW("  mpls key :");
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_from_mpls->macSaMask.size; i++)
        {
            p_to_mpls->mac_sa[i] = p_from_mpls->macSa.buf[i];
            /* reverse the mask*/
            p_to_mpls->mac_sa_mask[i] = ~p_from_mpls->macSaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_sa=%02X%02X.%02X%02X.%02X%02X, mac_sa_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mpls->mac_sa[0], p_to_mpls->mac_sa[1], p_to_mpls->mac_sa[2],
            p_to_mpls->mac_sa[3], p_to_mpls->mac_sa[4], p_to_mpls->mac_sa[5],
            p_to_mpls->mac_sa_mask[0], p_to_mpls->mac_sa_mask[1], p_to_mpls->mac_sa_mask[2],
            p_to_mpls->mac_sa_mask[3], p_to_mpls->mac_sa_mask[4], p_to_mpls->mac_sa_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_from_mpls->macSaMask.size; i++)
        {
            p_to_mpls->mac_da[i] = p_from_mpls->macDa.buf[i];
            /* reverse the mask*/
            p_to_mpls->mac_da_mask[i] = ~p_from_mpls->macDaMask.buf[i];
        }
        HAGT_DEBUG_FLOW("    mac_da=%02X%02X.%02X%02X.%02X%02X, mac_da_mask=%02X%02X.%02X%02X.%02X%02X",
            p_to_mpls->mac_da[0], p_to_mpls->mac_da[1], p_to_mpls->mac_da[2],
            p_to_mpls->mac_da[3], p_to_mpls->mac_da[4], p_to_mpls->mac_da[5],
            p_to_mpls->mac_da_mask[0], p_to_mpls->mac_da_mask[1], p_to_mpls->mac_da_mask[2],
            p_to_mpls->mac_da_mask[3], p_to_mpls->mac_da_mask[4], p_to_mpls->mac_da_mask[5]);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_CVLAN);
        p_to_mpls->cvlan = p_from_mpls->cVlanId;
        p_to_mpls->cvlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    cvlan = %d", p_to_mpls->cvlan);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_SVLAN);
        p_to_mpls->svlan = p_from_mpls->sVlanId;
        p_to_mpls->svlan_mask = 0xffff;
        HAGT_DEBUG_FLOW("    svlan = %d", p_to_mpls->svlan);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_CTAG_COS);
        p_to_mpls->ctag_cos = p_from_mpls->cTagCos;
        p_to_mpls->ctag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    ctag_cos = %d", p_to_mpls->ctag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_STAG_COS);
        p_to_mpls->stag_cos = p_from_mpls->sTagCos;
        p_to_mpls->stag_cos_mask = 0xff;
        HAGT_DEBUG_FLOW("    stag_cos = %d", p_to_mpls->stag_cos);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_L2TYPE))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_L2_TYPE);
        p_to_mpls->l2_type = p_from_mpls->l2Type;
        p_to_mpls->l2_type_mask = 0xff;
        HAGT_DEBUG_FLOW("    l2_type = %s", GLB_FLOW_L2TYPE_TO_STR(p_to_mpls->l2_type));
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_LABEL0))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL0);
        p_to_mpls->mpls_label0      = p_from_mpls->label0;
        p_to_mpls->mpls_label0_mask = ~p_from_mpls->label0Mask;
        HAGT_DEBUG_FLOW("    label = %u, mask = %05x", p_to_mpls->mpls_label0, p_to_mpls->mpls_label0_mask);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_LABEL1))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL1);
        p_to_mpls->mpls_label1      = p_from_mpls->label1;
        p_to_mpls->mpls_label1_mask = ~p_from_mpls->label1Mask;
        HAGT_DEBUG_FLOW("    label = %u, mask = %05x", p_to_mpls->mpls_label1, p_to_mpls->mpls_label1_mask);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_LABEL2))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL2);
        p_to_mpls->mpls_label2      = p_from_mpls->label2;
        p_to_mpls->mpls_label2_mask = ~p_from_mpls->label2Mask;
        HAGT_DEBUG_FLOW("    label = %u, mask = %05x", p_to_mpls->mpls_label2, p_to_mpls->mpls_label2_mask);
    }
    if (GLB_FLAG_ISSET(p_from_mpls->flag, GLB_FLOW_MPLS_RULE_FLAG_LABEL3))
    {
        GLB_SET_FLAG(p_to_mpls->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL3);
        p_to_mpls->mpls_label3      = p_from_mpls->label3;
        p_to_mpls->mpls_label3_mask = ~p_from_mpls->label3Mask;
        HAGT_DEBUG_FLOW("    label = %u, mask = %05x", p_to_mpls->mpls_label3, p_to_mpls->mpls_label3_mask);
    }

    if (!p_from_mpls->flag)
    {
        HAGT_DEBUG_FLOW("    N/A");
    }
    return HAGT_E_NONE;
}
/*********************************************************************
 * Name    : hagt_flow_map_rule
 * Purpose : Map rule to ctc key.
 * Input   : p_hagt_rule    - pointer to hagt flow rule
 * Output  : p_ctc_entry    - pointer to ctc flow entry
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_rule(HalMsgFlowRule_t* p_hagt_rule,
                   ctc_acl_key_t* p_ctc_key)
{
    GLB_PTR_VALID_CHECK(p_hagt_rule, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_ctc_key, HAGT_E_INVALID_PTR);
    
    switch(p_hagt_rule->ruleType)
    {
        case GLB_FLOW_RULE_TYPE_MAC:
            p_ctc_key->type = CTC_ACL_KEY_IPV4;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                &p_hagt_rule->rule.macRule, &p_ctc_key->u.ipv4_key,TRUE));
            break;
            
        case GLB_FLOW_RULE_TYPE_IPV4:
        case GLB_FLOW_RULE_TYPE_EXIP:
            p_ctc_key->type = CTC_ACL_KEY_IPV4;
            /*Modified by yanxa, for bug 17674, 2012-03-06*/
            if (p_hagt_rule->rule.present == rule_PR_macRule)
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                    &p_hagt_rule->rule.macRule, &p_ctc_key->u.ipv4_key,FALSE));
            }
            else
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv4_key(
                    &p_hagt_rule->rule.ipv4Rule, &p_ctc_key->u.ipv4_key));
                
                if (p_hagt_rule->hasExtMacRule)
                {
                    HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                        &p_hagt_rule->extMacRule, &p_ctc_key->u.ipv4_key, FALSE));
                }
            }
            break;
            
        case GLB_FLOW_RULE_TYPE_IPV6:
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            p_ctc_key->type = CTC_ACL_KEY_IPV6;
            /*Modified by yanxa, for bug 17674, 2012-03-06*/
            if (p_hagt_rule->rule.present == rule_PR_macRule)
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv6(
                    &p_hagt_rule->rule.macRule, &p_ctc_key->u.ipv6_key));
            }
            else
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv6_key(
                    &p_hagt_rule->rule.ipv6Rule, &p_ctc_key->u.ipv6_key));
                if (p_hagt_rule->hasExtMacRule)
                {
                    HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv6(
                        &p_hagt_rule->extMacRule, &p_ctc_key->u.ipv6_key));
                }
            }
            break;
#if 0/*Modified by yanxa, for bug 17674, 2012-03-06*/
        case GLB_FLOW_RULE_TYPE_EXIP:
            p_ctc_key->type = CTC_ACLQOS_IPV4_KEY;
            if (p_hagt_rule->rule.present == rule_PR_macRule)
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                    &p_hagt_rule->rule.macRule, &p_ctc_key->key_info.ipv4_key));
            }
            else
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv4_key(
                    &p_hagt_rule->rule.ipv4Rule, &p_ctc_key->key_info.ipv4_key));
            }
            break;
            
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            p_ctc_key->type = CTC_ACLQOS_IPV6_KEY;
            if (p_hagt_rule->rule.present == rule_PR_macRule)
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv6(
                    &p_hagt_rule->rule.macRule, &p_ctc_key->key_info.ipv6_key));
            }
            else
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv6_key(
                    &p_hagt_rule->rule.ipv6Rule, &p_ctc_key->key_info.ipv6_key));
            }
            break;
#endif
        case GLB_FLOW_RULE_TYPE_MPLS:
            p_ctc_key->type = CTC_ACL_KEY_MPLS;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_mpls_key(
                &p_hagt_rule->rule.mplsRule, &p_ctc_key->u.mpls_key));
            /*modified by yanxa, for bug 17674, 2012-03-06*/
            if (p_hagt_rule->hasExtMacRule)
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2mpls(
                    &p_hagt_rule->extMacRule, &p_ctc_key->u.mpls_key));
            }
            break;

        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_rule2
 * Purpose : Map rule to ctc key.
 * Input   : p_from_flow   - flow rule pointer
 * Output  : p_to_ctc     - ctc key pointer
             is_ext       - if ace is in Extent ACL, is_ext will be set to 1.
             p_to_ctc2    - if ace is in Extent ACL, p_to_ctc2 will be mapped from mac ace.
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_rule2(HalMsgFlowRule_t* p_from_flow,
                   ctc_acl_key_t* p_to_ctc_key,
                   ctc_acl_key_t* p_to_ctc_key2,
                   uint8* has_ext)
{
    GLB_PTR_VALID_CHECK(p_from_flow, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_to_ctc_key, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_to_ctc_key2, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(has_ext, HAGT_E_INVALID_PTR);

    *has_ext = FALSE;
    
    switch(p_from_flow->ruleType)
    {
        case GLB_FLOW_RULE_TYPE_MAC:
            p_to_ctc_key->type = CTC_ACL_KEY_IPV4;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                &p_from_flow->rule.macRule, &p_to_ctc_key->u.ipv4_key, FALSE));
            break;
        case GLB_FLOW_RULE_TYPE_IPV4:
            p_to_ctc_key->type = CTC_ACL_KEY_IPV4;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv4_key(
                &p_from_flow->rule.ipv4Rule, &p_to_ctc_key->u.ipv4_key));
            break;
        case GLB_FLOW_RULE_TYPE_IPV6:
            p_to_ctc_key->type = CTC_ACL_KEY_IPV6;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv6_key(
                &p_from_flow->rule.ipv6Rule, &p_to_ctc_key->u.ipv6_key));
            break;
        case GLB_FLOW_RULE_TYPE_EXIP:
            if (p_from_flow->rule.present == rule_PR_macRule)
            {
                *has_ext = TRUE;
                p_to_ctc_key->type = CTC_ACL_KEY_IPV4;
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                    &p_from_flow->rule.macRule, &p_to_ctc_key->u.ipv4_key, FALSE));

                p_to_ctc_key2->type = CTC_ACL_KEY_IPV4;
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                    &p_from_flow->rule.macRule, &p_to_ctc_key2->u.ipv4_key, FALSE));

            }
            else
            {
                p_to_ctc_key->type = CTC_ACL_KEY_IPV4;
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv4_key(
                    &p_from_flow->rule.ipv4Rule, &p_to_ctc_key->u.ipv4_key));
            }
            break;
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            p_to_ctc_key->type = CTC_ACL_KEY_IPV6;
            if (p_from_flow->rule.present == rule_PR_macRule)
            {
                /*Modified by yanxa, for bug 15923, 2011-08-19*/
                *has_ext = TRUE;
                p_to_ctc_key->type = CTC_ACL_KEY_IPV6;
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv6(
                    &p_from_flow->rule.macRule, &p_to_ctc_key->u.ipv6_key));

                p_to_ctc_key2->type = CTC_ACL_KEY_IPV4;
                HAGT_IF_ERROR_RETURN(hagt_flow_map_key_mac2ipv4(
                    &p_from_flow->rule.macRule, &p_to_ctc_key2->u.ipv4_key, FALSE));
            }
            else
            {
                HAGT_IF_ERROR_RETURN(hagt_flow_map_ipv6_key(
                    &p_from_flow->rule.ipv6Rule, &p_to_ctc_key->u.ipv6_key));
            }
            break;
        /*Added by yanxa, for mpls bug 17222, 2011-12-09*/
        case GLB_FLOW_RULE_TYPE_MPLS:
            p_to_ctc_key->type = CTC_ACL_KEY_MPLS;
            HAGT_IF_ERROR_RETURN(hagt_flow_map_mpls_key(
                &p_from_flow->rule.mplsRule, &p_to_ctc_key->u.mpls_key));
            break;

        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_pbr_rule
 * Purpose : Map rule to ctc key.
 * Input   : p_hagt_rule    - pointer to hagt flow rule
 * Output  : p_ctc_entry    - pointer to ctc flow entry
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_pbr_rule(HalMsgFlowRule_t* p_hagt_rule,
                       ctc_acl_key_t* p_ctc_key)
{
    GLB_PTR_VALID_CHECK(p_hagt_rule, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_ctc_key, HAGT_E_INVALID_PTR);
    switch(p_hagt_rule->ruleType)
    {
    case GLB_FLOW_RULE_TYPE_IPV4:
        p_ctc_key->type = CTC_ACL_KEY_PBR_IPV4;
        HAGT_IF_ERROR_RETURN(hagt_flow_map_pbr_ipv4_key(
        &p_hagt_rule->rule.ipv4Rule, &p_ctc_key->u.pbr_ipv4_key));
        break;
            
    case GLB_FLOW_RULE_TYPE_IPV6:
        p_ctc_key->type = CTC_ACL_KEY_PBR_IPV6;
        HAGT_IF_ERROR_RETURN(hagt_flow_map_pbr_ipv6_key(
        &p_hagt_rule->rule.ipv6Rule, &p_ctc_key->u.pbr_ipv6_key));
        break;

    default:
        return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_flow_map_rule_type
 * Purpose : Map rule type from hagt to ctc.
 * Input   : hagt_type    - hagt flow rule type
 * Output  : p_ctc_type   - pointer to ctc flow rule type
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_flow_map_rule_type(uint32 hagt_type, ctc_acl_key_type_t* p_ctc_type)
{
    GLB_PTR_VALID_CHECK(p_ctc_type, HAGT_E_INVALID_PTR);

    switch(hagt_type)
    {
        case GLB_FLOW_RULE_TYPE_MAC:
            *p_ctc_type = CTC_ACL_KEY_MAC;
            break;
            
        case GLB_FLOW_RULE_TYPE_IPV4:
            *p_ctc_type = CTC_ACL_KEY_IPV4;
            break;
            
        case GLB_FLOW_RULE_TYPE_IPV6:
            *p_ctc_type = CTC_ACL_KEY_IPV6;
            break;
            
        case GLB_FLOW_RULE_TYPE_EXIP:
            *p_ctc_type = CTC_ACL_KEY_IPV4;
            break;
            
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            *p_ctc_type = CTC_ACL_KEY_IPV6;
            break;

        /*Added by yanxa, for mpls bug 17222, 2011-12-09*/
        case GLB_FLOW_RULE_TYPE_MPLS:
            *p_ctc_type = CTC_ACL_KEY_MPLS;
            break;
            
        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}
