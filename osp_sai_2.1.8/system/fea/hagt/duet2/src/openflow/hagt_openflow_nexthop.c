
#include "ctc_api.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_openflow.h"
#include "ctclib_vector.h"
#include "hagt_openflow_tpoam.h"
#include <gen/stbl_qos_mpls_domain_define.h>
#include <gen/stbl_qos_mpls_domain.h>
#include <gen/stbl_qos_mpls_flow_define.h>
#include <gen/stbl_qos_mpls_flow.h>

/****************************************************************************************
 *  
 * Globals
 *
 ****************************************************************************************/

/****************************************************************************************
 *  
 * Function
 *
 ****************************************************************************************/

#define ________HAGT_DB________

extern hagt_openflow_port_mac_t*
openflow_port_mac_get_by_gport(uint32 gport);

extern int32
hsrv_openflow_get_port_mac_by_gport(uint32 gport, uint8 *mac);

extern void
hsrv_openflow_get_vxlan_src_port(uint32* p_is_set, uint32* p_vxlan_srcport);

static inline uint32 
_hagt_hash_make(uint8* p_array, uint32 len)  
{  
    /* Invented by Dr. Daniel J.Bernstein */
    uint32 hash = 5381;  
    uint32 i;
    
    for(i = 0; i < len; i++)  
    {
        hash = ((hash << 5) + hash) + p_array[i];  
    }

    return hash;  
}


static int32
hagt_openflow_group_lookup_mcast_mem(ctc_mcast_nh_param_group_t* nh_param, int32 nhid, hagt_openflow_mcast_mem_t** pp_info)
{
    hagt_openflow_mcast_mem_t lookup_key;

    sal_memset(&lookup_key, 0, sizeof(hagt_openflow_mcast_mem_t));
    lookup_key.nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    lookup_key.nh_param.mem_info.ref_nhid = nh_param->mem_info.ref_nhid;
    lookup_key.nhid = nhid;

    *pp_info = ctclib_hash_lookup(HAGT_OFNH_GROUP_MCAST_MEM_HASH, &lookup_key);

    return HAGT_E_NONE;
}

static int32
hagt_openflow_group_add_mcast_mem_to_db(ctc_mcast_nh_param_group_t *nh_param)
{
    hagt_openflow_mcast_mem_t* p_mcast_mem = NULL;

    p_mcast_mem = HAGT_OPENFLOW_CALLOC(sizeof(hagt_openflow_mcast_mem_t));
    HAGT_OPENFLOW_MEM_CHECK(p_mcast_mem);
    sal_memset(p_mcast_mem, 0, sizeof(hagt_openflow_mcast_mem_t));
    
    p_mcast_mem->nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    p_mcast_mem->nh_param.mem_info.ref_nhid = nh_param->mem_info.ref_nhid;
    p_mcast_mem->nhid = nh_param->mc_grp_id;
    p_mcast_mem->ref_cnt = 1;
    p_mcast_mem->is_source_check_dis = nh_param->mem_info.is_source_check_dis;

    if (ctclib_hash_lookup(HAGT_OFNH_GROUP_MCAST_MEM_HASH, nh_param))
    {
        HAGT_OPENFLOW_LOG_ERR("OpenFlow group add macst member error");
    }
    else
    {
        ctclib_hash_get(HAGT_OFNH_GROUP_MCAST_MEM_HASH, p_mcast_mem, ctclib_hash_alloc_intern);
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_group_del_mcast_mem_from_db(hagt_openflow_mcast_mem_t* p_info)
{
    if (!ctclib_hash_lookup(HAGT_OFNH_GROUP_MCAST_MEM_HASH, p_info))
    {
        HAGT_OPENFLOW_LOG_ERR("OpenFlow group del macst member error");
    }
    
    ctclib_hash_release(HAGT_OFNH_GROUP_MCAST_MEM_HASH, p_info);
    
    return HAGT_E_NONE;
}

static inline uint32
_hagt_openflow_mcast_mem_hash_make(hagt_openflow_mcast_mem_t* p_info_void)
{
    hagt_openflow_mcast_mem_t* p_info = NULL;

    p_info = (hagt_openflow_mcast_mem_t *)p_info_void;
    return (uint32) p_info->nhid;
}

static inline bool
_hagt_openflow_mcast_mem_hash_compare(hagt_openflow_mcast_mem_t* pva, hagt_openflow_mcast_mem_t* pvb)
{
    if (!pva || !pvb)
    {
        return FALSE;
    }

    if (0 != sal_memcmp(&pva->nh_param, &pvb->nh_param, sizeof(ctc_mcast_nh_param_group_t))
        ||pva->nhid != pvb->nhid)
    {
        return FALSE;
    }

    return TRUE;
}

#define ________HAGT_TRANSLATOR________
static int32
hagt_openflow_map_flex_nexthop_mac(HalMsgOpenFlowFlexNh_t* p_nh, ctc_misc_nh_flex_edit_param_t* p_param)
{
    HalMsgOpenFlowL2Edit_t* p_mac_edit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM];
    uint8 i;

    /* If action field is larger than 64 bits, we should expand p_nh->flagX */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;

    p_mac_edit = &p_nh->macEdit;
    if (GLB_OFPACT_FLD_ISSET(flags, MACSA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_MACSA);
        for (i = 0; i < p_mac_edit->macSa.size; i++)
        {
            p_param->mac_sa[i] = p_mac_edit->macSa.buf[i];
        }
    }
    if (GLB_OFPACT_FLD_ISSET(flags, MACDA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_MACDA);
        for (i = 0; i < p_mac_edit->macDa.size; i++)
        {
            p_param->mac_da[i] = p_mac_edit->macDa.buf[i];
        }
    }
    if (GLB_OFPACT_FLD_ISSET(flags, SVLAN_VID))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        p_param->svid_sl = CTC_VLAN_TAG_SL_NEW;
        p_param->new_svid = p_mac_edit->sVlanId;

        /*default scos =0*/
        if(GLB_OFPACT_FLD_ISSET(flags, PUSH_SVLAN))
        {
            p_param->stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
            /* mod by hansf for new push stag, scos inherit form original outer packet */
            #if 0
            p_param->scos_sl = CTC_VLAN_TAG_SL_NEW;
            p_param->new_scos = 0;
            #endif
            p_param->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
        }
    }
    if (GLB_OFPACT_FLD_ISSET(flags, SVLAN_COS))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        /* mod by hansf for when only set_cos action, does not add a new tag if original packet untag */
        if (!p_param->stag_op)
        {
            p_param->stag_op = CTC_VLAN_TAG_OP_REP;
        }
        p_param->scos_sl = CTC_VLAN_TAG_SL_NEW;
        p_param->new_scos = p_mac_edit->sTagCos;
    }
    if (GLB_OFPACT_FLD_ISSET(flags, CVLAN_VID))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->ctag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        p_param->cvid_sl = CTC_VLAN_TAG_SL_NEW;
        p_param->new_cvid = p_mac_edit->cVlanId;

        /*default ccos =0*/
        if(GLB_OFPACT_FLD_ISSET(flags, PUSH_CVLAN))
        {
            p_param->ctag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
            /* mod by hansf for new push ctag, ccos inherit form original outer packet */
            #if 0
            p_param->ccos_sl = CTC_VLAN_TAG_SL_NEW;
            p_param->new_ccos = 0;
            #endif
            p_param->ccos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
        }
    }
    if (GLB_OFPACT_FLD_ISSET(flags, CVLAN_COS))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        /* mod by hansf for when only set_cos action, does not add a new tag if original packet untag */
        if (!p_param->ctag_op)
        {
            p_param->ctag_op = CTC_VLAN_TAG_OP_REP;
        }
        p_param->ccos_sl = CTC_VLAN_TAG_SL_NEW;
        p_param->new_ccos = p_mac_edit->cTagCos;
    }
    if (GLB_OFPACT_FLD_ISSET(flags, STRIP_SVLAN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->stag_op = CTC_VLAN_TAG_OP_DEL;
    }
    if (GLB_OFPACT_FLD_ISSET(flags, STRIP_CVLAN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->ctag_op = CTC_VLAN_TAG_OP_DEL;
    }
    if (GLB_OFPACT_FLD_ISSET(flags, PUSH_SVLAN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->stag_op = CTC_VLAN_TAG_OP_ADD;
        p_param->svid_sl = CTC_VLAN_TAG_SL_NEW;
     }
    if (GLB_OFPACT_FLD_ISSET(flags, PUSH_CVLAN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG);
        p_param->ctag_op = CTC_VLAN_TAG_OP_ADD;
        p_param->cvid_sl = CTC_VLAN_TAG_SL_NEW;
     }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_flex_nexthop_arp(HalMsgOpenFlowFlexNh_t* p_nh, ctc_misc_nh_flex_edit_param_t* p_param)
{
    HalMsgOpenFlowArpEdit_t* p_arp_edit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM];
    uint8 i;

    /* If action field is larger than 2 bytes, we should expand p_nh->flagX */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;

    p_arp_edit = &p_nh->unionEdit.arpEdit;
    p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_ARP;
    if (GLB_OFPACT_FLD_ISSET(flags, ARP_OP))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_OP);
        p_param->arp_op = p_arp_edit->arpOp;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ARP_SPA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SPA);
        p_param->arp_spa = p_arp_edit->arpSpa;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ARP_TPA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_TPA);
        p_param->arp_tpa = p_arp_edit->arpTpa;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ARP_SHA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SHA);
        for (i = 0; i < p_arp_edit->arpSha.size; i++)
        {
            p_param->arp_sha[i] = p_arp_edit->arpSha.buf[i];
        }
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ARP_THA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_THA);
        for (i = 0; i < p_arp_edit->arpTha.size; i++)
        {
            p_param->arp_tha[i] = p_arp_edit->arpTha.buf[i];
        }
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_flex_nexthop_ipv4(HalMsgOpenFlowFlexNh_t* p_nh, ctc_misc_nh_flex_edit_param_t* p_param)
{
    HalMsgOpenFlowIpv4Edit_t* p_ipv4_edit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM];

    /* If action field is larger than 2 bytes, we should expand p_nh->flagX */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;

    p_ipv4_edit = &p_nh->unionEdit.ipv4Edit;
    GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_IPV4);

    /* By default, dscp is derived from the incoming packet, if we do not
       set this value, SDK will use the default 0 (CTC_NH_DSCP_SELECT_ASSIGN)
       if we modify L3 or L4 headers, and ASIC will set the dscp to 0 which
       is not the desired behavior in a OpenFlow enabled SW. */
    p_param->dscp_select = CTC_NH_DSCP_SELECT_PACKET;

    if (GLB_OFPACT_FLD_ISSET(flags, IPSA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA);
        p_param->ip_sa.ipv4 = p_ipv4_edit->ipSa;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IPDA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA);
        p_param->ip_da.ipv4 = p_ipv4_edit->ipDa;
    }
    
    if (GLB_OFPACT_FLD_ISSET(flags, IP_DSCP))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        p_param->dscp_select = CTC_NH_DSCP_SELECT_ASSIGN;
        p_param->dscp_or_tos = p_ipv4_edit->dscp;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_ECN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ECN);
        p_param->ecn = p_ipv4_edit->ecn;
        p_param->ecn_select = CTC_NH_ECN_SELECT_ASSIGN;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_TTL_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_TTL);
        p_param->ttl = p_ipv4_edit->ttl;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, DEC_IP_TTL) ||
        GLB_OFPACT_FLD_ISSET(flags, DEC_TTL))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_DECREASE_TTL);
        p_param->ttl = 1;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_PROTO))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_PROTOCOL);
        p_param->protocol = p_ipv4_edit->protocol;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ICMP_TYPE))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        /* Note: replace_l4_hdr is not used by SDK actually. */
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_TYPE);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_ICMP;
        p_param->icmp_type = p_ipv4_edit->icmpType;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ICMP_CODE))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_CODE);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_ICMP;
        p_param->icmp_code = p_ipv4_edit->icmpCode;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, TCP_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_TCP_PORT);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_UDPORTCP;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, UDP_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_UDP_PORT);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_UDPORTCP;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, SPORT))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_SRC_PORT);
        p_param->l4_src_port = p_ipv4_edit->l4SrcPort;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, DPORT))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_DST_PORT);
        p_param->l4_dst_port = p_ipv4_edit->l4DestPort;
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_flex_nexthop_ipv6(HalMsgOpenFlowFlexNh_t* p_nh, ctc_misc_nh_flex_edit_param_t* p_param)
{
    HalMsgOpenFlowIpv6Edit_t* p_ipv6_edit = NULL;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM];

    /* If action field is larger than 2 bytes, we should expand p_nh->flagX */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;

    p_ipv6_edit = &p_nh->unionEdit.ipv6Edit;
    
    //GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_IPV4);

    /* By default, dscp is derived from the incoming packet, if we do not
       set this value, SDK will use the default 0 (CTC_NH_DSCP_SELECT_ASSIGN)
       if we modify L3 or L4 headers, and ASIC will set the dscp to 0 which
       is not the desired behavior in a OpenFlow enabled SW. */
    p_param->dscp_select = CTC_NH_DSCP_SELECT_PACKET;

    if (GLB_OFPACT_FLD_ISSET(flags, IPSA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA);
        sal_memcpy(p_param->ip_sa.ipv6, p_ipv6_edit->ipSa.buf, sizeof(ipv6_addr_t));
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IPDA))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA);
        sal_memcpy(p_param->ip_da.ipv6, p_ipv6_edit->ipDa.buf, sizeof(ipv6_addr_t));
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IPV6_LBL))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_FLOW_LABEL);
        p_param->flow_label = p_ipv6_edit->label;
    }
    
    if (GLB_OFPACT_FLD_ISSET(flags, IP_DSCP))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        p_param->dscp_select = CTC_NH_DSCP_SELECT_ASSIGN;
        p_param->dscp_or_tos = p_ipv6_edit->dscp;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_ECN))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ECN);
        p_param->ecn = p_ipv6_edit->ecn;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_TTL_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_TTL);
        p_param->ttl = p_ipv6_edit->ttl;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, DEC_IP_TTL) ||
        GLB_OFPACT_FLD_ISSET(flags, DEC_TTL))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_DECREASE_TTL);
        p_param->ttl = 1;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, IP_PROTO))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_PROTOCOL);
        p_param->protocol = p_ipv6_edit->protocol;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ICMP_TYPE))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        /* Note: replace_l4_hdr is not used by SDK actually. */
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_TYPE);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_ICMP;
        p_param->icmp_type = p_ipv6_edit->icmpType;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, ICMP_CODE))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_CODE);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_ICMP;
        p_param->icmp_code = p_ipv6_edit->icmpCode;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, TCP_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_TCP_PORT);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_UDPORTCP;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, UDP_))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_UDP_PORT);
        p_param->packet_type = CTC_MISC_NH_PACKET_TYPE_UDPORTCP;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, SPORT))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_SRC_PORT);
        p_param->l4_src_port = p_ipv6_edit->l4SrcPort;
    }

    if (GLB_OFPACT_FLD_ISSET(flags, DPORT))
    {
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR);
        GLB_SET_FLAG(p_param->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_DST_PORT);
        p_param->l4_dst_port = p_ipv6_edit->l4DestPort;
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_flex_nexthop(HalMsgOpenFlowFlexNh_t* p_nh, ctc_misc_nh_flex_edit_param_t* p_param)
{
    HAGT_IF_ERROR_RETURN(hagt_openflow_map_flex_nexthop_mac(p_nh, p_param));
    switch(p_nh->unionEdit.present)
    {
    case unionEdit_PR_NOTHING:
        break;

    case unionEdit_PR_arpEdit:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_flex_nexthop_arp(p_nh, p_param));
        break;

    case unionEdit_PR_ipv4Edit:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_flex_nexthop_ipv4(p_nh, p_param));
        break;

    case unionEdit_PR_ipv6Edit:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_flex_nexthop_ipv6(p_nh, p_param));
        break;

    default:
        break;
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_v4_v6_exchange_nexthop(HalMsgOpenFlowFlexNh_t* p_nh, ctc_ip_tunnel_nh_param_t* p_param)
{
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM];
    uint8 i;
    HalMsgOpenFlowL2Edit_t *p_mac_edit;

    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;
    p_mac_edit = &p_nh->macEdit;

    p_param->oif.gport = p_nh->gport;
    p_param->oif.is_l2_port = 1;

    if (GLB_OFPACT_FLD_ISSET(flags, MACSA))
    {
        for (i = 0; i < p_mac_edit->macSa.size; i++)
        {
            p_param->mac_sa[i] = p_mac_edit->macSa.buf[i];
        }
    }
    else
    {
        /* use port mac */
        HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(p_nh->gport, p_param->mac_sa));
    }
    
    if (GLB_OFPACT_FLD_ISSET(flags, MACDA))
    {
        for (i = 0; i < p_mac_edit->macDa.size; i++)
        {
            p_param->mac[i] = p_mac_edit->macDa.buf[i];
        }
    }
    
    if (GLB_OFPACT_FLD_ISSET(flags, SVLAN_VID))
    {
        p_param->oif.vid = p_mac_edit->sVlanId;
    }

    p_param->tunnel_info.dscp_select = CTC_NH_DSCP_SELECT_PACKET;
    if (GLB_OPENFLOW_FLOW_TNL_TYPE_V4_TO_V6 == p_nh->flowType)
    {
        //v4 to v6
        HalMsgOpenFlowIpv6Edit_t* p_ipv6_edit;
        p_ipv6_edit = &p_nh->unionEdit.ipv6Edit;
        p_param->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN6;
        /* set ip protocol type */
        if (GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_TCP == p_nh->pldPacketType)
        {
            p_param->tunnel_info.openflow_ip_protocol = 6;
            GLB_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL);
        }
        else if (GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_UDP == p_nh->pldPacketType)
        {
            p_param->tunnel_info.openflow_ip_protocol = 17;
            GLB_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL);
        }
        
        if (GLB_OFPACT_FLD_ISSET(flags, IPSA))
        {
            sal_memcpy(p_param->tunnel_info.ip_sa.ipv6, p_ipv6_edit->ipSa.buf, sizeof(ipv6_addr_t));
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IPDA))
        {
            sal_memcpy(p_param->tunnel_info.ip_da.ipv6, p_ipv6_edit->ipDa.buf, sizeof(ipv6_addr_t));
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IPV6_LBL))
        {
            p_param->tunnel_info.flow_label_mode = CTC_NH_FLOW_LABEL_ASSIGN;
            p_param->tunnel_info.flow_label = p_ipv6_edit->label;
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IP_DSCP))
        {
            p_param->tunnel_info.dscp_select= CTC_NH_DSCP_SELECT_ASSIGN;
            p_param->tunnel_info.dscp_or_tos = p_ipv6_edit->dscp;
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IP_TTL_))
        {
            p_param->tunnel_info.ttl = p_ipv6_edit->ttl;
        }
        else
        {
            p_param->tunnel_info.ttl = 64;
        }

    }
    else if (GLB_OPENFLOW_FLOW_TNL_TYPE_V6_TO_V4 == p_nh->flowType)
    {
        //v6 to v4
        HalMsgOpenFlowIpv4Edit_t* p_ipv4_edit;
        p_ipv4_edit = &p_nh->unionEdit.ipv4Edit;
        p_param->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN4;
        /* set ip protocol type */
        if (GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_TCP == p_nh->pldPacketType)
        {
            p_param->tunnel_info.openflow_ip_protocol = 6;
            GLB_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL);
        }
        else if (GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_UDP == p_nh->pldPacketType)
        {
            p_param->tunnel_info.openflow_ip_protocol = 17;            
            GLB_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL);
        }
        
        if (GLB_OFPACT_FLD_ISSET(flags, IPSA))
        {
            p_param->tunnel_info.ip_sa.ipv4 = p_ipv4_edit->ipSa;
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IPDA))
        {
            p_param->tunnel_info.ip_da.ipv4 = p_ipv4_edit->ipDa;
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IP_DSCP))
        {
            p_param->tunnel_info.dscp_select= CTC_NH_DSCP_SELECT_ASSIGN;
            p_param->tunnel_info.dscp_or_tos = p_ipv4_edit->dscp;
        }
        if (GLB_OFPACT_FLD_ISSET(flags, IP_TTL_))
        {
            p_param->tunnel_info.ttl = p_ipv4_edit->ttl;
        }
        else
        {
            p_param->tunnel_info.ttl = 64;
        }
    }
    else
    {
        return HAGT_E_INVALID_PARAM;
    }
    
    return HAGT_E_NONE;
}

/* Added by weizj for tunnel */
static int32
hagt_openflow_map_tunnel_nexthop(HalMsgOpenFlowTunnelNh_t* p_nh, ctc_ip_tunnel_nh_param_t* p_param)
{
    HalMsgOpenFlowTunnelEdit_t* p_tnl_edit = NULL;
    HalMsgOpenFlowL2Edit_t* p_mac_edit = NULL;
    HalMsgOpenFlowL2Edit_t* p_inner_mac_edit = NULL;
    HalMsgOpenFlowIpv4Edit_t* p_ipv4_edit = NULL;
    uint32 tun_id;
    uint8 i;
    uint32 is_set = 0;
    uint32 vxlan_srcport = 0;
    /* map mac */
    p_mac_edit = &p_nh->macEdit;
    p_inner_mac_edit = &p_nh->innerMacEdit;
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(p_nh->gport, p_param->mac_sa));

    if (GLB_IS_BIT_SET(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST))
    {
        for (i = 0; i < p_mac_edit->macDa.size; i++)
        {
            p_param->mac[i] = p_mac_edit->macDa.buf[i];
        }
    }

    /* map ip */
    p_ipv4_edit = &p_nh->ipv4Edit;
    if (GLB_IS_BIT_SET(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC))
    {
        p_param->tunnel_info.ip_sa.ipv4 = p_ipv4_edit->ipSa;
    }

    if (GLB_IS_BIT_SET(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST))
    {
        p_param->tunnel_info.ip_da.ipv4 = p_ipv4_edit->ipDa;
    }
    
    /* map tunnel info */
    p_tnl_edit = &p_nh->tnl;
    p_param->tunnel_info.gre_info.protocol_type = p_tnl_edit->tunnelProtocol;
    p_param->tunnel_info.inner_packet_type = PKT_TYPE_ETH;
    p_param->tunnel_info.dscp_select= CTC_NH_DSCP_SELECT_ASSIGN;
    p_param->tunnel_info.dscp_or_tos = p_tnl_edit->dscpOrTos;
    p_param->tunnel_info.ttl = p_tnl_edit->ttl;
    
    p_param->oif.gport = p_nh->gport;
    p_param->oif.vid = p_tnl_edit->vid;
    /* not check l3if */
    p_param->oif.is_l2_port = 1;

    /* strip inner vlan, Added by weizj for tunnel bug 33408 */
    if (GLB_IS_BIT_SET(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN))
    {
        CTC_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN);
    }

    if (GLB_IS_BIT_SET(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL))
    {
        tun_id = p_tnl_edit->tunnelKey;
        CTC_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY);
    }
    else
    {
        tun_id = 0;
    }

    if(GLB_IS_BIT_SET(p_nh->nextFlags, GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_SRC))
    {
        for (i = 0; i < p_inner_mac_edit->macSa.size; i++)
        {
            p_param->tunnel_info.inner_macsa[i] = p_inner_mac_edit->macSa.buf[i];
        }
        CTC_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA);
    }

    if(GLB_IS_BIT_SET(p_nh->nextFlags, GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_DST))
    {
        for (i = 0; i < p_inner_mac_edit->macDa.size; i++)
        {
            p_param->tunnel_info.inner_macda[i] = p_inner_mac_edit->macDa.buf[i];
        }
        CTC_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI);
    }

    switch (p_tnl_edit->type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            p_param->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN4;
            p_param->tunnel_info.gre_info.gre_key = tun_id;
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            p_param->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NVGRE_IN4;
            p_param->tunnel_info.gre_info.gre_key = tun_id;/*added by yejl, if gre_key is 0, sdk check will return error*/
            p_param->tunnel_info.vn_id = tun_id;
            HAGT_IF_ERROR_RETURN(hagt_openflow_tunnel_set_fid(tun_id, p_nh->fid));
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            hsrv_openflow_get_vxlan_src_port(&is_set, &vxlan_srcport);
            if(is_set)
            {
                CTC_SET_FLAG(p_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT);
                p_param->tunnel_info.l4_src_port = vxlan_srcport;
            }
            p_param->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
            p_param->tunnel_info.gre_info.gre_key = tun_id;/*added by yejl, if gre_key is 0, sdk check will return error*/
            p_param->tunnel_info.vn_id = tun_id;
            HAGT_IF_ERROR_RETURN(hagt_openflow_tunnel_set_fid(tun_id, p_nh->fid));
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }
    //more

    return HAGT_E_NONE;
}
/* End by weizj for tunnel */

static int32 
hagt_openflow_add_mcast_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddMcastNhReq_t *req;
    ctc_mcast_nh_param_group_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMcastNh;

    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    nh_param.mc_grp_id = req->offset;
    nh_param.opcode = CTC_NH_PARAM_MCAST_NONE;
    nh_param.stats_en = req->statsEn;
    nh_param.stats_id = hagt_openflow_stats_get_ctc_stats_id(req->statsId);
    nh_param.is_mirror = 0;

    HAGT_IF_ERROR_RETURN(ctc_nh_add_mcast(req->nhid, &nh_param));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_mcast_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelMcastNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMcastNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mcast(req->nhid));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_add_mcast_member(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddMcastMemberNhReq_t *req;
    ctc_mcast_nh_param_group_t nh_param;
    hagt_openflow_mcast_mem_t* p_mcast_mem = NULL;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMcastMemberNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    nh_param.mc_grp_id = req->nhid;
    nh_param.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_param.mem_info.ref_nhid = req->memberInfo.nhid;
    nh_param.mem_info.destid = req->memberInfo.gport;
    nh_param.mem_info.is_reflective = req->memberInfo.redirectToIngressPort ? 1 : 0;
    /* If redirectToIngressPort is true, we must turn off the check unconditionally because
       SDK will not do that for us. */
    if (nh_param.mem_info.is_reflective) 
    {
        nh_param.mem_info.is_source_check_dis = 1;    /* no check discard. */
    }
    else
    {
        /* Is source check dis mean is_source_check_disabled, so should do the translation. */
        nh_param.mem_info.is_source_check_dis = req->memberInfo.portCheckDiscard ? 0 : 1;
    }

    /* check */
    hagt_openflow_group_lookup_mcast_mem(&nh_param, req->nhid, &p_mcast_mem);
    if (p_mcast_mem)
    {
        p_mcast_mem->ref_cnt ++;
        return HAGT_E_NONE;
    }

    /* add sdk */
    HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->nhid, &nh_param));

    /* add db */    
    HAGT_IF_ERROR_RETURN(hagt_openflow_group_add_mcast_mem_to_db(&nh_param));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_mcast_member(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelMcastMemberNhReq_t *req;
    ctc_mcast_nh_param_group_t nh_param;
    hagt_openflow_mcast_mem_t* p_mcast_mem = NULL;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMcastMemberNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    nh_param.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_param.mem_info.ref_nhid = req->memberInfo.nhid;
    nh_param.mem_info.destid = req->memberInfo.gport;

    /* check */
    hagt_openflow_group_lookup_mcast_mem(&nh_param, req->nhid, &p_mcast_mem);
    if (!p_mcast_mem)
    {
        HAGT_OPENFLOW_LOG_ERR("Openflow mcast member reference count error");
        return HAGT_E_NONE;
    }

    p_mcast_mem->ref_cnt --;
    if (0 == p_mcast_mem->ref_cnt)
    {
        /* need this flag for delete member */
        nh_param.mem_info.is_source_check_dis = p_mcast_mem->is_source_check_dis;
        HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->nhid, &nh_param));
        hagt_openflow_group_del_mcast_mem_from_db(p_mcast_mem);
        HAGT_OPENFLOW_FREE(p_mcast_mem);
    }

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_add_ecmp_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddECMPNhReq_t *req;
    ctc_nh_ecmp_nh_param_t nh_param;
    uint32 ctc_stats_id = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddECMPNh;

    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    ctc_stats_id = hagt_openflow_stats_get_ctc_stats_id(req->statsId);
    nh_param.stats_id = ctc_stats_id;
    //nh_param.stats_id = req->statsId;
    /* can not add an ecmp group with no members */
    /* workaround: add a drop member for ecmp group */
    nh_param.nhid[0] = CTC_NH_RESERVED_NHID_FOR_DROP;
    nh_param.nh_num = 1;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_ecmp(req->nhid, &nh_param));

    return HAGT_E_NONE;
}

static int32
hagt_openflow_del_ecmp_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelECMPNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelECMPNh;
    HAGT_IF_ERROR_RETURN(ctc_nh_remove_ecmp(req->nhid));

    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_ecmp_member(Hsrv2AgtMsg_t* p_msg_data)
{
    uint32 memeber_index = 0;
    HalMsgOpenFlowAddECMPMemberNhReq_t *req;
    ctc_nh_ecmp_nh_param_t nh_param;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddECMPMemberNh;

    /* 1. delete drop nhid, which added when ecmp group create */
    /*modified by wangl for bug 38435*/
    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    nh_param.upd_type = CTC_NH_ECMP_REMOVE_MEMBER;
    nh_param.failover_en = TRUE;
    nh_param.nhid[0] = CTC_NH_RESERVED_NHID_FOR_DROP;
    nh_param.nh_num = 1;
    ctc_nh_update_ecmp(req->memberNhid.ecmpid, &nh_param);
        
    /* 2. add ecmp member */
    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    nh_param.upd_type = CTC_NH_ECMP_ADD_MEMBER;
    nh_param.failover_en = TRUE;
    nh_param.stats_id = hagt_openflow_stats_get_ctc_stats_id(req->statsId);
    nh_param.nh_num = req->memberNhid.nhCnt;

    for (memeber_index = 0; memeber_index < nh_param.nh_num; memeber_index++)
    {
        nh_param.nhid[memeber_index] = *(uint32*) req->memberNhid.nhidList.list.array[memeber_index];
    }

    HAGT_IF_ERROR_RETURN(ctc_nh_update_ecmp(req->memberNhid.ecmpid, &nh_param));

    return HAGT_E_NONE;
}

static int32
hagt_openflow_del_ecmp_member(Hsrv2AgtMsg_t* p_msg_data)
{
    uint32 memeber_index = 0;
    HalMsgOpenFlowDelECMPMemberNhReq_t *req;
    ctc_nh_ecmp_nh_param_t nh_param;
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelECMPMemberNh;
    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    nh_param.upd_type = CTC_NH_ECMP_REMOVE_MEMBER;
    nh_param.failover_en = TRUE;
    nh_param.nh_num = req->memberNhid.nhCnt;

    for (memeber_index = 0; memeber_index < nh_param.nh_num; memeber_index++)
    {
        nh_param.nhid[memeber_index] = *(uint32*) req->memberNhid.nhidList.list.array[memeber_index];
    }

    HAGT_IF_ERROR_RETURN(ctc_nh_update_ecmp(req->memberNhid.ecmpid, &nh_param));

    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_flex_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddFlexNhReq_t *req;
    ctc_misc_nh_param_t nh_param;
    ctc_ip_tunnel_nh_param_t tunnel_nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlexNh;

    if (GLB_OPENFLOW_FLOW_TNL_TYPE_V4_TO_V6 == req->nhInfo.flowType || GLB_OPENFLOW_FLOW_TNL_TYPE_V6_TO_V4 == req->nhInfo.flowType)
    {
        /* added by hansf for v4/v6 transform */
        sal_memset(&tunnel_nh_param, 0x0, sizeof(ctc_ip_tunnel_nh_param_t));
        tunnel_nh_param.dsnh_offset = req->offset;
    
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_v4_v6_exchange_nexthop(&req->nhInfo, &tunnel_nh_param));
        HAGT_IF_ERROR_RETURN(ctc_nh_add_ip_tunnel(req->nhid, &tunnel_nh_param));
    }
    else
    {
        sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));
        nh_param.type = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;
        nh_param.dsnh_offset = req->offset;
        nh_param.gport = req->nhInfo.gport;
        /* Translate IN_PORT to cpu port to make SDK happy. 
       20150707: when _sys_goldengate_nh_mcast_write_dsmet SDK use cpu port to do
       cpu reason setting which will make in_port forwarding not work, so we should
       Choose drop port instead since this nexthop will only be used in_port forwarding. */
        if (GLB_OFPP_IN_PORT == nh_param.gport)
        {
            nh_param.gport = GLB_DROP_PORT;
        }

        HAGT_IF_ERROR_RETURN(hagt_openflow_map_flex_nexthop(&req->nhInfo, &nh_param.misc_param.flex_edit));
        HAGT_IF_ERROR_RETURN(ctc_nh_add_misc(req->nhid, &nh_param));
    }
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_add_flex_cpu_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddFlexCpuNhReq_t *req;
    ctc_misc_nh_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlexCpuNh;
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));
    nh_param.type = CTC_MISC_NH_TYPE_TO_CPU;
    nh_param.misc_param.cpu_reason.cpu_reason_id = CTC_PKT_CPU_REASON_CUSTOM_BASE + req->reason;
    
    HAGT_IF_ERROR_RETURN(ctc_nh_add_misc(req->nhid, &nh_param));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_flex_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelFlexNhReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelFlexNh;
    if (GLB_OPENFLOW_FLOW_TNL_TYPE_V4_TO_V6 == req->flowType || GLB_OPENFLOW_FLOW_TNL_TYPE_V6_TO_V4 == req->flowType)
    {
        /* added by hansf for v4/v6 transform 2018/11/21 */
        HAGT_IF_ERROR_RETURN(ctc_nh_remove_ip_tunnel(req->nhid));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_nh_remove_misc(req->nhid));
    }
    
    return HAGT_E_NONE;
}

/* Added by weizj for tunnel */
static int32
hagt_openflow_add_tunnel_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddTunnelNhReq_t *req;
    ctc_ip_tunnel_nh_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddTunnelNh;
    sal_memset(&nh_param, 0, sizeof(ctc_ip_tunnel_nh_param_t));
    nh_param.dsnh_offset = req->offset;
    if (req->statsEn)
    {
        nh_param.tunnel_info.stats_id = hagt_openflow_stats_get_ctc_stats_id(req->statsId);
    }
    
    HAGT_IF_ERROR_RETURN(hagt_openflow_map_tunnel_nexthop(&req->nhInfo, &nh_param));
    HAGT_IF_ERROR_RETURN(ctc_nh_add_ip_tunnel(req->nhid, &nh_param));

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_tunnel_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelTunnelNhReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelTunnelNh;
    HAGT_IF_ERROR_RETURN(ctc_nh_remove_ip_tunnel(req->nhid));

    return HAGT_E_NONE;
}
/* End by weizj for tunnel */

/* for cubro mpls. */
static int32
_hagt_openflow_map_mpls_label_param(HalMsgOpenFlowMplsNhLabelParam_t *p_in_param, 
                                                ctc_mpls_nh_label_param_t *p_out_param)
{
    stbl_qos_mpls_domain_t * p_db_mpls_domain = NULL;
    stbl_qos_mpls_domain_key_t mpls_domain_key;
    switch(p_in_param->expType)
    {
        case GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN:
            p_out_param->exp_type = CTC_NH_EXP_SELECT_ASSIGN;
            p_out_param->exp = p_in_param->exp;
            break;
            
        case GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP:
            p_out_param->exp_type = CTC_NH_EXP_SELECT_MAP;

            /*added by yejl for bug 52805, 2019-07-25*/
            sal_memset(&mpls_domain_key, 0, sizeof(mpls_domain_key));
            mpls_domain_key.mpls_label = p_in_param->label;
            p_db_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&mpls_domain_key);
            if (p_db_mpls_domain)
            {
                p_out_param->exp_domain = p_db_mpls_domain->exp_domain;
            }
            else
            {
                p_out_param->exp_domain = 0;
            }
            /*ended by yejl*/
            break;
            
        case GLB_OPENFLOW_MPLS_NH_EXP_SELECT_PACKET:
            p_out_param->exp_type = CTC_NH_EXP_SELECT_PACKET;
            break;
                
        case GLB_OPENFLOW_MPLS_NH_EXP_MAX:
            return HAGT_E_INVALID_PARAM;
    }

    if (GLB_FLAG_ISSET(p_in_param->flag, GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL))
    {
        GLB_SET_FLAG(p_out_param->lable_flag,CTC_MPLS_NH_LABEL_MAP_TTL);
    }
    if (GLB_FLAG_ISSET(p_in_param->flag, GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST))
    {
        GLB_SET_FLAG(p_out_param->lable_flag,CTC_MPLS_NH_LABEL_IS_MCAST);
    }
    p_out_param->ttl = p_in_param->ttl;
    p_out_param->label = p_in_param->label;

    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_mpls_tunnel(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddMplsTunnelReq_t *req;
    ctc_mpls_nexthop_tunnel_param_t tunnel_param;
    
    sal_memset(&tunnel_param, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMplsTunnel;

    if (GLB_FLAG_ISSET(req->tunnelLabel.flag, GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID))
    {
        tunnel_param.nh_param.label_num = 1;
        HAGT_IF_ERROR_RETURN(
            _hagt_openflow_map_mpls_label_param(&(req->tunnelLabel), &(tunnel_param.nh_param.tunnel_label[0])));
    }
    else
    {
        tunnel_param.nh_param.label_num = 0;
    }

    tunnel_param.nh_param.oif.gport = req->gport;
#ifndef DUET2
    tunnel_param.nh_param.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
#endif
    tunnel_param.nh_param.oif.vid = req->outerVlanVid;
    tunnel_param.nh_param.oif.is_l2_port = 1;
    sal_memcpy(&(tunnel_param.nh_param.mac), req->nhMac.buf, sizeof(tunnel_param.nh_param.mac));

    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->gport, tunnel_param.nh_param.mac_sa));

    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls_tunnel_label(req->tunnelId, &tunnel_param));

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_mpls_tunnel(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelMplsTunnelReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMplsTunnel;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls_tunnel_label(req->tunnelId));

    return HAGT_E_NONE;
}

static int32
_hagt_openflow_map_mpls_inner_svlan_op(uint32 svlan_tag_op, uint16 svlan_id,
                                                   ctc_vlan_egress_edit_info_t *vlan_edit_info)
{
    switch(svlan_tag_op)
    {
        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE:
            break;
            
        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_INSERT:
            vlan_edit_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            vlan_edit_info->output_svid = svlan_id;
            CTC_SET_FLAG(vlan_edit_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            vlan_edit_info->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;
            
        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_STRIP:
            vlan_edit_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_REPLACE:
            vlan_edit_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            vlan_edit_info->output_svid = svlan_id;
            CTC_SET_FLAG(vlan_edit_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_KEEP_UNCHANGE:
            vlan_edit_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;
                
        case GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_MAX:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_push_l2vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddPushL2vpnNhReq_t *req;
    ctc_mpls_nexthop_param_t nh_param;
    ctc_mcast_nh_param_group_t m_nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mpls_nexthop_param_t));

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid || req->mcastOffset)
    {
        sal_memset(&m_nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        m_nh_param.mc_grp_id = req->mcastOffset;
        m_nh_param.opcode = CTC_NH_PARAM_MCAST_NONE;
        m_nh_param.stats_en = 0;
        m_nh_param.stats_id = 0;
        m_nh_param.is_mirror = 0;
        HAGT_IF_ERROR_RETURN(ctc_nh_add_mcast(req->mcastNhid, &m_nh_param));
    }
    /*ended by yejl*/

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.dsnh_offset = req->offset;

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-14 
        for lsp, label will be added in tunnel, label shouldn't be added in nexthop */
    if (0 == req->pwLabel.flag)
    {
        nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_NONE;
        nh_param.nh_para.nh_param_push.label_num = 0;
    }
    else
    /* liwh end */
    {
        nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;

        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-27 */
        if (hagt_tpoam_get_pw_mode())
        {
            nh_param.nh_para.nh_param_push.martini_encap_valid = TRUE;
        }
        /* liwh end */
        
        GLB_SET_FLAG(nh_param.nh_para.nh_param_push.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_NO_SRC_DSCP_COS);        
        HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_inner_svlan_op(req->innerSvlanTagOp, req->innerSvlanId, 
                                                            &(nh_param.nh_para.nh_param_push.nh_com.vlan_info)));
        nh_param.nh_para.nh_param_push.label_num = 1;
        HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_label_param(&(req->pwLabel), 
                                                            &(nh_param.nh_para.nh_param_push.push_label[0])));
    }
    nh_param.nh_para.nh_param_push.tunnel_id = req->tunnelId;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls(req->nhid, &nh_param));

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid || req->mcastOffset)
    {
        sal_memset(&m_nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        m_nh_param.mc_grp_id = req->mcastNhid;
        m_nh_param.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
        m_nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        m_nh_param.mem_info.ref_nhid = req->nhid;
        m_nh_param.mem_info.is_reflective = 0;
        m_nh_param.mem_info.is_source_check_dis = 0;
        HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->mcastNhid, &m_nh_param));
    }
    /*ended by yejl*/

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_push_l2vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelPushL2vpnNhReq_t *req;
    ctc_mcast_nh_param_group_t nh_param;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPushL2vpnNh;

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid)
    {
        sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        nh_param.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
        nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        nh_param.mem_info.ref_nhid = req->nhid;
        HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->mcastNhid, &nh_param));
    }
    /*ended by yejl*/
    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls(req->nhid));

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid)
    {
        HAGT_IF_ERROR_RETURN(ctc_nh_remove_mcast(req->mcastNhid));
    }
    /*ended by yejl*/
    
    return HAGT_E_NONE;
}

/* modified by yaom for openflow MPLS L2VPN 20160214 */
static int32
hagt_openflow_add_push_l3vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddPushL3vpnNhReq_t *req;
    ctc_mpls_nexthop_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL3vpnNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mpls_nexthop_param_t));

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.dsnh_offset = req->offset;
    nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_ROUTE;
    nh_param.nh_para.nh_param_push.label_num = 0;
    nh_param.nh_para.nh_param_push.tunnel_id = req->tunnelId;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls(req->nhid, &nh_param));

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_push_l3vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelPushL3vpnNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPushL3vpnNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls(req->nhid));
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_swap_mpls_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddSwapMplsNhReq_t *req;
    ctc_mpls_nexthop_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddSwapMplsNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mpls_nexthop_param_t));

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.dsnh_offset = req->offset;
    nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_NONE;
    nh_param.nh_para.nh_param_push.label_num = 0;
    if (req->pwLabel.flag & GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID)
    {
        nh_param.nh_para.nh_param_push.label_num = 1;
        HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_label_param(&(req->pwLabel), 
                                                                &(nh_param.nh_para.nh_param_push.push_label[0])));
    }
    nh_param.nh_para.nh_param_push.tunnel_id = req->tunnelId;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls(req->nhid, &nh_param));

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_swap_mpls_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelSwapMplsNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelSwapMplsNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls(req->nhid));
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_pop_php_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddPopPHPNhReq_t *req;
    ctc_mpls_nexthop_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPopPHPNh;
    sal_memset(&nh_param, 0, sizeof(ctc_mpls_nexthop_param_t));

    nh_param.nh_prop = CTC_MPLS_NH_POP_TYPE;
    nh_param.dsnh_offset = req->offset;
    nh_param.nh_para.nh_param_pop.ttl_mode = CTC_MPLS_TUNNEL_MODE_UNIFORM;
    nh_param.nh_para.nh_param_pop.nh_com.opcode = CTC_MPLS_NH_PHP;
 #ifndef DUET2   
    nh_param.nh_para.nh_param_pop.nh_com.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
 #endif
    nh_param.nh_para.nh_param_pop.nh_com.oif.gport = req->gport;
    nh_param.nh_para.nh_param_pop.nh_com.oif.vid = req->outerVlanVid;
    nh_param.nh_para.nh_param_pop.nh_com.oif.is_l2_port = 1;
    GLB_SET_FLAG(nh_param.nh_para.nh_param_pop.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_L2EDIT_OUTPUT_VID);
    sal_memcpy(nh_param.nh_para.nh_param_pop.nh_com.mac, req->nhMac.buf, sizeof(mac_addr_t));
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->gport, nh_param.nh_para.nh_param_pop.nh_com.mac_sa));
    
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls(req->nhid, &nh_param));

    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_pop_php_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelPopPHPNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPopPHPNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls(req->nhid));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_add_ipuc_nh(Hsrv2AgtMsg_t* p_msg_data)
{
#if 0
    HalMsgOpenFlowAddIpucNhReq_t *req;
    uint8 i;
    ctc_misc_nh_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddIpucNh;

    sal_memset(&nh_param, 0x0, sizeof(ctc_misc_nh_param_t));
    nh_param.type = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;

    nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
    nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACDA;
    nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACSA;
    for (i = 0; i < req->nhInfo.nhMac.size; i++)
    {
        nh_param.misc_param.flex_edit.mac_da[i] = req->nhInfo.nhMac.buf[i];
    }
    nh_param.gport = req->nhInfo.gport;
    if(GLB_MAX_VLAN_ID != req->nhInfo.vid)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG;
        nh_param.misc_param.flex_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        nh_param.misc_param.flex_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
        nh_param.misc_param.flex_edit.new_svid = req->nhInfo.vid;
    }
    nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ETHERTYPE;
    nh_param.misc_param.flex_edit.ether_type = 0x800;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_misc(req->nhid, &nh_param));
    return HAGT_E_NONE;
#else

    HalMsgOpenFlowAddIpucNhReq_t *req;
    ctc_ip_nh_param_t nh_param;
    uint8 i;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddIpucNh;

    sal_memset(&nh_param, 0, sizeof(ctc_ip_nh_param_t));
    GLB_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_USE_L2EDIT);
    if (req->nhInfo.rewriteEtherType)
    {
        GLB_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_REWRITE_ETHER_TYPE);
    }
    nh_param.dsnh_offset = req->offset;
#ifndef DUET2
    nh_param.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
#endif
    nh_param.oif.is_l2_port = 1;
    nh_param.oif.gport = req->nhInfo.gport;
    nh_param.oif.vid = req->nhInfo.vid;
    for (i = 0; i < req->nhInfo.nhMac.size; i++)
    {
        nh_param.mac[i] = req->nhInfo.nhMac.buf[i];
    }
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->nhInfo.gport, nh_param.mac_sa));
    HAGT_IF_ERROR_RETURN(ctc_nh_add_ipuc(req->nhid, &nh_param));
    
    return HAGT_E_NONE;
    #endif
}

static int32 
hagt_openflow_del_ipuc_nh(Hsrv2AgtMsg_t* p_msg_data)
{
#if 0
    HalMsgOpenFlowDelIpucNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelIpucNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_misc(req->nhid));
    
    return HAGT_E_NONE;
#else

    HalMsgOpenFlowDelIpucNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelIpucNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_ipuc(req->nhid));
    
    return HAGT_E_NONE;
#endif
}

static int32 
hagt_openflow_add_xlate_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddXlateNhReq_t *req;
    ctc_vlan_edit_nh_param_t nh_param;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddXlateNh;

    sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    nh_param.dsnh_offset = req->offset;
    nh_param.gport_or_aps_bridge_id = req->nhInfo.gport;
    GLB_SET_FLAG(nh_param.vlan_edit_info.flag, CTC_VLAN_NH_HAVE_DSFWD);
    HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_inner_svlan_op(req->nhInfo.innerSvlanTagOp, req->nhInfo.innerSvlanId, 
                                                            &(nh_param.vlan_edit_info)));
    
    HAGT_IF_ERROR_RETURN(ctc_nh_add_xlate(req->nhid, &nh_param));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_xlate_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelXlateNhReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelXlateNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_xlate(req->nhid));
    
    return HAGT_E_NONE;
}

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-20 */
static int32 
hagt_openflow_add_section_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSectionNh_t *req;
    ctc_ip_nh_param_t nh_param;
    uint8   i = 0;
    
    sal_memset(&nh_param, 0, sizeof(nh_param));
   
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddSectionNh;

    GLB_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_TPOAM_SECTION);
    nh_param.dsnh_offset = req->offset;
//    nh_param.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
    nh_param.oif.is_l2_port = 1;
    nh_param.oif.gport = req->nhInfo.gport;
    nh_param.oif.vid = 0;
    for (i = 0; i < req->nhInfo.nhMac.size; i++)
    {
        nh_param.mac[i] = req->nhInfo.nhMac.buf[i];
    }
    /*add by yangl for tpoam 2019-09-20*/
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->nhInfo.gport, nh_param.mac_sa));
    HAGT_IF_ERROR_RETURN(ctc_nh_add_ipuc(req->nhid, &nh_param));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_section_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSectionNh_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelSectionNh;

    HAGT_IF_ERROR_RETURN(ctc_nh_remove_ipuc(req->nhid));
    
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_add_lsp_aps_tunnel(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowLspApsTunnelReq_t *req = NULL;
    ctc_mpls_nexthop_tunnel_param_t tunnel_label_param;
    ctc_aps_bridge_group_t aps_group;   
    uint16 tunnel_id = 0;
    uint16 aps_group_id = 0;

    sal_memset(&tunnel_label_param, 0, sizeof(tunnel_label_param));
    sal_memset(&aps_group, 0, sizeof(ctc_aps_bridge_group_t));

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel;

    /* add aps group */
    aps_group_id = req->apsGroupId;
    aps_group.working_gport = req->gportid;
    aps_group.protection_gport = req->pgportid;
    aps_group.w_l3if_id = 1021;  /*MAX_CTC_L3IF_ID -1 */
    aps_group.p_l3if_id = 1021;  /*MAX_CTC_L3IF_ID -1 */
    aps_group.physical_isolated = 1;
    HAGT_IF_ERROR_RETURN(ctc_aps_create_aps_bridge_group(aps_group_id, &aps_group));
    
    /* add tunnel */
    tunnel_id = req->tunnelId;
    
    GLB_SET_FLAG(tunnel_label_param.aps_flag, CTC_NH_MPLS_APS_EN);
    tunnel_label_param.aps_bridge_group_id = req->apsGroupId;

    /* working label */
    tunnel_label_param.nh_param.label_num = req->labelNumber;
    tunnel_label_param.nh_param.oif.gport = req->gportid;
    tunnel_label_param.nh_param.oif.vid = req->vid;
//    tunnel_label_param.nh_param.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
    tunnel_label_param.nh_param.oif.is_l2_port = 1;
    sal_memcpy(tunnel_label_param.nh_param.mac, req->macAddr.buf, sizeof(mac_addr_t));
    tunnel_label_param.nh_param.tunnel_label[0].label = req->tunnelLabel1;
    tunnel_label_param.nh_param.tunnel_label[0].lable_flag |= CTC_MPLS_NH_LABEL_IS_VALID;
    tunnel_label_param.nh_param.tunnel_label[0].ttl = req->mapTTL1;
    tunnel_label_param.nh_param.tunnel_label[0].exp_type = CTC_NH_EXP_SELECT_MAP;
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->gportid, tunnel_label_param.nh_param.mac_sa));
    
    /* protection label */
    tunnel_label_param.nh_p_param.label_num = req->plabelNumber;
    tunnel_label_param.nh_p_param.oif.gport = req->pgportid;
    tunnel_label_param.nh_p_param.oif.vid = req->pvid;
//    tunnel_label_param.nh_p_param.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
    tunnel_label_param.nh_p_param.oif.is_l2_port = 1;
    sal_memcpy(tunnel_label_param.nh_p_param.mac, req->pmacAddr.buf, sizeof(mac_addr_t));
    tunnel_label_param.nh_p_param.tunnel_label[0].label = req->ptunnelLabel1;
    tunnel_label_param.nh_p_param.tunnel_label[0].lable_flag |= CTC_MPLS_NH_LABEL_IS_VALID;
    tunnel_label_param.nh_p_param.tunnel_label[0].ttl = req->pmapTTL1;
    tunnel_label_param.nh_p_param.tunnel_label[0].exp_type = CTC_NH_EXP_SELECT_MAP;
    HAGT_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(req->pgportid, tunnel_label_param.nh_p_param.mac_sa));
            
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls_tunnel_label(tunnel_id, &tunnel_label_param));       
    return HAGT_E_NONE;
}

static int32 
hagt_openflow_del_lsp_aps_tunnel(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowLspApsTunnelReq_t *req = NULL;
    uint16 tunnel_id = 0;
    uint16 aps_group_id = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLspApsTunnel;
    aps_group_id = req->apsGroupId;
    tunnel_id = req->tunnelId;

    /* delete tunnel*/          
    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls_tunnel_label(tunnel_id));

    /* add aps group */
    HAGT_IF_ERROR_RETURN(ctc_aps_destroy_aps_bridge_group(aps_group_id));
            
    return HAGT_E_NONE;
}
/* liwh end */

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-03 */
static int32
hagt_openflow_add_aps_push_l2vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddApsPushL2vpnNhReq_t *req;
    ctc_mpls_nexthop_param_t nh_param;
    ctc_mcast_nh_param_group_t m_nh_param;
    ctc_aps_bridge_group_t aps_group; 

    sal_memset(&nh_param, 0, sizeof(ctc_mpls_nexthop_param_t));
    sal_memset(&aps_group, 0, sizeof(ctc_aps_bridge_group_t));
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddApsPushL2vpnNh;

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid || req->mcastOffset)
    {
        sal_memset(&m_nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        m_nh_param.mc_grp_id = req->mcastOffset;
        m_nh_param.opcode = CTC_NH_PARAM_MCAST_NONE;
        m_nh_param.stats_en = 0;
        m_nh_param.stats_id = 0;
        m_nh_param.is_mirror = 0;
        HAGT_IF_ERROR_RETURN(ctc_nh_add_mcast(req->mcastNhid, &m_nh_param));
    }
    /*ended by yejl*/
    
    /* add aps group */
    if (GLB_INVALID_GPORTID != req->gport)
    {
        aps_group.working_gport = req->gport;
        aps_group.protection_gport = req->pgport;
        aps_group.w_l3if_id = 1021;  /*MAX_CTC_L3IF_ID -1 */
        aps_group.p_l3if_id = 1021;  /*MAX_CTC_L3IF_ID -1 */
        aps_group.physical_isolated = 1;
    }
    else
    {
        aps_group.next_aps.w_group_id = req->lspApsGroupId;
        aps_group.next_aps.p_group_id = req->plspApsGroupId;
        aps_group.next_w_aps_en = TRUE;
        aps_group.next_p_aps_en = TRUE;
    }
    HAGT_IF_ERROR_RETURN(ctc_aps_create_aps_bridge_group(req->apsGroupId, &aps_group));

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.dsnh_offset = req->offset;
    nh_param.aps_bridge_group_id = req->apsGroupId;
    nh_param.aps_en = TRUE;
    
    nh_param.nh_para.nh_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-27 */
    if (hagt_tpoam_get_pw_mode())
    {
        nh_param.nh_para.nh_param_push.martini_encap_valid = TRUE;
    }
    /* liwh end */
         
    GLB_SET_FLAG(nh_param.nh_para.nh_param_push.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_NO_SRC_DSCP_COS);        
    HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_inner_svlan_op(req->innerSvlanTagOp, req->innerSvlanId, 
                                                        &(nh_param.nh_para.nh_param_push.nh_com.vlan_info)));
    nh_param.nh_para.nh_param_push.label_num = 1;
    HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_label_param(&(req->pwLabel), 
                                                        &(nh_param.nh_para.nh_param_push.push_label[0])));
    nh_param.nh_para.nh_param_push.tunnel_id = req->tunnelId;

    nh_param.nh_p_para.nh_p_param_push.nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;
    
    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-27 */
    if (hagt_tpoam_get_pw_mode())
    {
        nh_param.nh_p_para.nh_p_param_push.martini_encap_valid = TRUE;
    }
    /* liwh end */
         
    GLB_SET_FLAG(nh_param.nh_p_para.nh_p_param_push.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_NO_SRC_DSCP_COS);        
    HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_inner_svlan_op(req->pinnerSvlanTagOp, req->pinnerSvlanId, 
                                                        &(nh_param.nh_p_para.nh_p_param_push.nh_com.vlan_info)));
    nh_param.nh_p_para.nh_p_param_push.label_num = 1;
    HAGT_IF_ERROR_RETURN(_hagt_openflow_map_mpls_label_param(&(req->ppwLabel), 
                                                        &(nh_param.nh_p_para.nh_p_param_push.push_label[0])));
     nh_param.nh_p_para.nh_p_param_push.tunnel_id = req->ptunnelId;
    HAGT_IF_ERROR_RETURN(ctc_nh_add_mpls(req->nhid, &nh_param));

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid || req->mcastOffset)
    {
        sal_memset(&m_nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        m_nh_param.mc_grp_id = req->mcastNhid;
        m_nh_param.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
        m_nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        m_nh_param.mem_info.ref_nhid = req->nhid;
        m_nh_param.mem_info.is_reflective = 0;
        m_nh_param.mem_info.is_source_check_dis = 0;
        HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->mcastNhid, &m_nh_param));
    }
    /*ended by yejl*/

    return HAGT_E_NONE;
}

static int32
hagt_openflow_del_aps_push_l2vpn_nh(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddApsPushL2vpnNhReq_t *req;
    ctc_mcast_nh_param_group_t nh_param;
    uint16 aps_bridge_group_id = 0;
    uint32 nhid = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh;
    aps_bridge_group_id = req->apsGroupId;
    nhid = req->nhid;

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid)
    {
        sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        nh_param.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
        nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        nh_param.mem_info.ref_nhid = req->nhid;
        HAGT_IF_ERROR_RETURN(ctc_nh_update_mcast(req->mcastNhid, &nh_param));
    }
    /*ended by yejl*/
    
    HAGT_IF_ERROR_RETURN(ctc_nh_remove_mpls(nhid));
    HAGT_IF_ERROR_RETURN(ctc_aps_destroy_aps_bridge_group(aps_bridge_group_id));

    /*modified by yejl for bug 52155, 2019-05-22*/
    if (req->mcastNhid)
    {
        HAGT_IF_ERROR_RETURN(ctc_nh_remove_mcast(req->mcastNhid));
    }
    /*ended by yejl*/
    return HAGT_E_NONE;
}
/* liwh end */

#define _____INIT_____

int32
hagt_openflow_nexthop_init(void)
{
    g_pst_hagt_openflow_master->pst_mcast_mem_hash = ctclib_hash_create(
            _hagt_openflow_mcast_mem_hash_make,
            _hagt_openflow_mcast_mem_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_mcast_mem_hash);

    ctc_field_key_t field_key;
    ctc_scl_entry_t scl_entry;
    ctc_scl_field_action_t field_action;
    /*init mpls l3if*/
    ctc_l3if_t l3_if;
    sal_memset(&l3_if, 0x0, sizeof(l3_if));
    l3_if.gport = 0xfff;
    l3_if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    HAGT_IF_ERROR_RETURN(ctc_l3if_create(MAX_CTC_L3IF_ID, &l3_if));

    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID, CTC_L3IF_PROP_ROUTE_ALL_PKT, TRUE));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID, CTC_L3IF_PROP_MPLS_EN, TRUE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID, CTC_L3IF_PROP_IPV4_UCAST, FALSE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID, CTC_L3IF_PROP_CONTEXT_LABEL_EXIST, TRUE ));


    /*init ipuc next l3if*/
    sal_memset(&l3_if, 0x0, sizeof(l3_if));
    l3_if.gport = 0xffe;
    l3_if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    HAGT_IF_ERROR_RETURN(ctc_l3if_create(MAX_CTC_L3IF_ID -1, &l3_if));

    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -1, CTC_L3IF_PROP_ROUTE_ALL_PKT, TRUE));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -1, CTC_L3IF_PROP_MPLS_EN, TRUE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -1, CTC_L3IF_PROP_IPV4_UCAST, FALSE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -1, CTC_L3IF_PROP_CONTEXT_LABEL_EXIST, TRUE ));

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-21 */
    /*init ipuc next l3if*/
    sal_memset(&l3_if, 0x0, sizeof(l3_if));
    l3_if.gport = 0xffd;
    l3_if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3_if.vlan_id = 1;
    HAGT_IF_ERROR_RETURN(ctc_l3if_create(MAX_CTC_L3IF_ID -2, &l3_if));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -2, CTC_L3IF_PROP_ROUTE_ALL_PKT, TRUE));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -2, CTC_L3IF_PROP_MPLS_EN, TRUE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -2, CTC_L3IF_PROP_IPV4_UCAST, FALSE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(MAX_CTC_L3IF_ID -2, CTC_L3IF_PROP_CONTEXT_LABEL_EXIST, TRUE ));
    /* liwh end */
    
    /*create scl group*/
    ctc_scl_group_info_t group_info;
    sal_memset(&group_info, 0x0, sizeof(group_info));
    group_info.priority = 1;
    group_info.type = CTC_SCL_GROUP_TYPE_GLOBAL;
    HAGT_IF_ERROR_RETURN(ctc_scl_create_group(GLB_SCL_GROUP_FOR_HYBRID, &group_info));
    HAGT_IF_ERROR_RETURN(ctc_scl_install_group(GLB_SCL_GROUP_FOR_HYBRID, &group_info));

        /*unicast mpls*/
    sal_memset(&scl_entry, 0x0, sizeof(scl_entry));
    scl_entry.entry_id = GLB_FLOW_ENTRY_MAX;
    scl_entry.priority = GLB_FLOW_ENTRY_PRIORITY_MAX;
    scl_entry.priority_valid = 1;
    scl_entry.mode = 1;
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
    scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_entry(GLB_SCL_GROUP_FOR_HYBRID, &scl_entry));
    
    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L3_TYPE;
    field_key.data = CTC_PARSER_L3_TYPE_MPLS;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));


    sal_memset(&field_action, 0x0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    field_action.data0 = MAX_CTC_L3IF_ID + 4096;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_action_field(scl_entry.entry_id, &field_action)); 

    HAGT_IF_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id)); 


    /*multicast mpls*/
    sal_memset(&scl_entry, 0x0, sizeof(scl_entry));
    scl_entry.entry_id = GLB_FLOW_ENTRY_MAX - 1;
    scl_entry.priority = GLB_FLOW_ENTRY_PRIORITY_MAX;
    scl_entry.priority_valid = 1;
    scl_entry.mode = 1;
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
    scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_entry(GLB_SCL_GROUP_FOR_HYBRID, &scl_entry));
    
    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L3_TYPE;
    field_key.data = CTC_PARSER_L3_TYPE_MPLS_MCAST;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_action, 0x0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    field_action.data0 = MAX_CTC_L3IF_ID + 4096;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_action_field(scl_entry.entry_id, &field_action)); 

    HAGT_IF_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id)); 


    /*gre*/
    sal_memset(&scl_entry, 0x0, sizeof(scl_entry));
    scl_entry.entry_id = GLB_FLOW_ENTRY_MAX- 2;
    scl_entry.priority = GLB_FLOW_ENTRY_PRIORITY_MAX;
    scl_entry.priority_valid = 1;
    scl_entry.mode = 1;
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
    scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_entry(GLB_SCL_GROUP_FOR_HYBRID, &scl_entry));
    
    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L3_TYPE;
    field_key.data = CTC_PARSER_L3_TYPE_IPV4;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L4_TYPE;
    field_key.data = CTC_PARSER_L4_TYPE_GRE;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_action, 0x0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    field_action.data0 = MAX_CTC_L3IF_ID + 4096;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_action_field(scl_entry.entry_id, &field_action)); 

    HAGT_IF_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id)); 


     /*vxlan*/
    sal_memset(&scl_entry, 0x0, sizeof(scl_entry));
    scl_entry.entry_id = GLB_FLOW_ENTRY_MAX- 3;
    scl_entry.priority = GLB_FLOW_ENTRY_PRIORITY_MAX;
    scl_entry.priority_valid = 1;
    scl_entry.mode = 1;
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
    scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_entry(GLB_SCL_GROUP_FOR_HYBRID, &scl_entry));
    
    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L3_TYPE;
    field_key.data = CTC_PARSER_L3_TYPE_IPV4;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L4_TYPE;
    field_key.data = CTC_PARSER_L4_TYPE_UDP;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_key, 0x0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_L4_USER_TYPE;
    field_key.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
    field_key.mask = 0xff;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_key_field(scl_entry.entry_id, &field_key));

    sal_memset(&field_action, 0x0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
    field_action.data0 = MAX_CTC_L3IF_ID + 4096;
    HAGT_IF_ERROR_RETURN(ctc_scl_add_action_field(scl_entry.entry_id, &field_action)); 

    HAGT_IF_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id)); 
    return HAGT_E_NONE;
}

int32
hagt_openflow_nexthop_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)
    
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMcastNh, hagt_openflow_add_mcast_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMcastNh, hagt_openflow_del_mcast_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMcastMemberNh, hagt_openflow_add_mcast_member);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMcastMemberNh, hagt_openflow_del_mcast_member);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddECMPNh, hagt_openflow_add_ecmp_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelECMPNh, hagt_openflow_del_ecmp_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddECMPMemberNh, hagt_openflow_add_ecmp_member);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelECMPMemberNh, hagt_openflow_del_ecmp_member);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlexNh, hagt_openflow_add_flex_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlexCpuNh, hagt_openflow_add_flex_cpu_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelFlexNh, hagt_openflow_del_flex_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddTunnelNh, hagt_openflow_add_tunnel_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelTunnelNh, hagt_openflow_del_tunnel_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMplsTunnel, hagt_openflow_add_mpls_tunnel);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMplsTunnel, hagt_openflow_del_mpls_tunnel);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL2vpnNh, hagt_openflow_add_push_l2vpn_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPushL2vpnNh, hagt_openflow_del_push_l2vpn_nh);
    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL3vpnNh, hagt_openflow_add_push_l3vpn_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPushL3vpnNh, hagt_openflow_del_push_l3vpn_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddSwapMplsNh, hagt_openflow_add_swap_mpls_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelSwapMplsNh, hagt_openflow_del_swap_mpls_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPopPHPNh, hagt_openflow_add_pop_php_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPopPHPNh, hagt_openflow_del_pop_php_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddIpucNh, hagt_openflow_add_ipuc_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelIpucNh, hagt_openflow_del_ipuc_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddXlateNh, hagt_openflow_add_xlate_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelXlateNh, hagt_openflow_del_xlate_nh);

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-20 */
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddSectionNh, hagt_openflow_add_section_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelSectionNh, hagt_openflow_del_section_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddLspApsTunnel, hagt_openflow_add_lsp_aps_tunnel);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLspApsTunnel, hagt_openflow_del_lsp_aps_tunnel);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddApsPushL2vpnNh, hagt_openflow_add_aps_push_l2vpn_nh);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh, hagt_openflow_del_aps_push_l2vpn_nh);
    /* liwh end */
    
#undef Reg

    return HAGT_E_NONE;
}
