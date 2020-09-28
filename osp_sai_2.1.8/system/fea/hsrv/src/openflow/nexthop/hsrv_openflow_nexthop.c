#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_nexthop_priv.h"
#include "hsrv_openflow_nexthop.h"
#include "hsrv_openflow_tpoam.h"

extern uint32
ctc_sai_copp_get_nhid_for_controller(void);

extern int32
hsrv_openflow_mgroup_id_opf_alloc_by_sai(uint32* p_group_id);

extern int32
hsrv_openflow_mgroup_id_opf_free_by_sai(uint32 group_id);

struct hsrv_of_nh_table_entry_info_s
{
    uint8 opf_size_1;
    uint8 opf_type_1;
    
    uint8 opf_size_2;
    uint8 opf_type_2;
};
typedef struct hsrv_of_nh_table_entry_info_s hsrv_of_nh_table_entry_info_t;

const static hsrv_of_nh_table_entry_info_t g_ofnh_offset_size_info[HSRV_OF_NH_TYPE_MAX] =
{
    /* 0 : HSRV_OF_NH_TYPE_NH_NONE */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,
        
        .opf_size_2 = 0,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    },

    /* 1 : HSRV_OF_NH_TYPE_NH_MCAST */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,
        
        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET,

    },

    /* 2 : HSRV_OF_NH_TYPE_NH_ECMP */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    },

    /* 3 : HSRV_OF_NH_TYPE_NH_FLEX */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET,
    },

    /* 4 : HSRV_OF_NH_TYPE_NH_FLEX_CPU */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET,
    },
    /* Added by weizj for tunnel */
    /* 5 : HSRV_OF_NH_TYPE_NH_IPUC */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 0,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    },

    /* 6 : HSRV_OF_NH_TYPE_NH_IP_TUNNEL */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    },

    /* 7 : HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    },
    /* End by weizj for tunnel */

    /* 8 : HSRV_OF_NH_TYPE_NH_MPLS */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET,
    },

    /* 9 : HSRV_OF_NH_TYPE_NH_XLATE */
    {
        .opf_size_1 = 1,
        .opf_type_1 = HSRV_OPENFLOW_NEXTHOP_OPF_NHID,

        .opf_size_2 = 1,
        .opf_type_2 = HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET,
    },    
};

struct hsrv_openflow_nexthop_debug_data_s
{
    FILE* fp;
    int32_ofp i;
};
typedef struct hsrv_openflow_nexthop_debug_data_s hsrv_ofnh_debug_data_t;

struct hsrv_openflow_nexthop_debug_data1_s
{
    uint32 hash_ptr;
    bool exist;
};
typedef struct hsrv_openflow_nexthop_debug_data1_s hsrv_openflow_nexthop_debug_data1_t;

hsrv_openflow_nexthop_master_t* g_pst_openflow_nexthop_master;


static inline uint32 
_hsrv_hash_make(uint8* p_array, uint32 len)  
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


#define _____FLEX_NH_____

static inline uint32
hsrv_openflow_nexthop_flex_make(hsrv_ofact_flex_nh_info_t* p_info)
{
    hsrv_openflow_action_combo_t info;

    memcpy(&info, &p_info->combo, sizeof(hsrv_openflow_action_combo_t));

    GLB_OFPACT_FLD_UNSET(info.flag, TUNNEL_ID);
    GLB_OFPACT_FLD_UNSET(info.flag, METER);
    GLB_OFPACT_FLD_UNSET(info.flag, QUEUE_ID);
    info.tunnel_id    = 0;
    info.meter_id     = 0;
    info.queue_id     = 0;
    //info.is_resvd_nh  = 0;

    return ofp_hash_make((uint8*)&info, sizeof(info));
}

static bool
hsrv_openflow_nexthop_flex_compare(hsrv_ofact_flex_nh_info_t* pva, hsrv_ofact_flex_nh_info_t* pvb)
{
    hsrv_openflow_action_combo_t info1; 
    hsrv_openflow_action_combo_t info2;
    
    memcpy(&info1, &pva->combo, sizeof(hsrv_openflow_action_combo_t));
    memcpy(&info2, &pvb->combo, sizeof(hsrv_openflow_action_combo_t));
    
    GLB_OFPACT_FLD_UNSET(info1.flag, TUNNEL_ID);
    GLB_OFPACT_FLD_UNSET(info1.flag, METER);
    GLB_OFPACT_FLD_UNSET(info1.flag, QUEUE_ID);
    info1.tunnel_id    = 0;
    info1.meter_id     = 0;
    info1.queue_id     = 0;
    //info1.is_resvd_nh  = 0;
    
    GLB_OFPACT_FLD_UNSET(info2.flag, TUNNEL_ID);
    GLB_OFPACT_FLD_UNSET(info2.flag, METER);
    GLB_OFPACT_FLD_UNSET(info2.flag, QUEUE_ID);
    info2.tunnel_id    = 0;
    info2.meter_id     = 0;
    info2.queue_id     = 0;
    //info2.is_resvd_nh  = 0;
    
    if (0 != memcmp(&info1, &info2, sizeof(hsrv_openflow_action_combo_t)))
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_nexthop_lookup_flex_nh(hsrv_openflow_action_combo_t* p_combo, hsrv_ofact_flex_nh_info_t** pp_info)
{
    hsrv_ofact_flex_nh_info_t lookup_key;

    memcpy(&lookup_key.combo, p_combo, sizeof(hsrv_openflow_action_combo_t));

    *pp_info = ctclib_hash_lookup(HSRV_OFNH_FLEX_NH_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_add_flex_nh(hsrv_ofact_flex_nh_info_t* p_info)
{
    if (ctclib_hash_lookup(HSRV_OFNH_FLEX_NH_HASH, p_info))
    {
        HSRV_OPENFLOW_LOG_ERR("OpenFlow flex nexthop database error");
    }
    else
    {
        if (p_info->combo.is_resvd_nh)
        {
            g_pst_openflow_nexthop_master->reserved_nh_count ++;
        }

        ctclib_hash_get(HSRV_OFNH_FLEX_NH_HASH, p_info, ctclib_hash_alloc_intern);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_flex_nh(hsrv_ofact_flex_nh_info_t* p_info)
{
    if (p_info->combo.is_resvd_nh)
    {
        if (g_pst_openflow_nexthop_master->reserved_nh_count > 0)
        {
            g_pst_openflow_nexthop_master->reserved_nh_count --;
        }
        else
        {
            HSRV_OPENFLOW_LOG_ERR("OpenFlow flex nexthop reserved nexthop count error");
        }
    }
    
    ctclib_hash_release(HSRV_OFNH_FLEX_NH_HASH, p_info);
    
    return HSRV_E_NONE;
}


static int32
hsrv_openflow_map_flex_nh_mac(hsrv_openflow_action_combo_t* p_combo, HalMsgOpenFlowFlexNh_t* p_nh)
{
    HalMsgOpenFlowL2Edit_t* p_mac = &p_nh->macEdit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM] = {0};

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;
    
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, MACSA))
    {
        GLB_OFPACT_FLD_SET(flags, MACSA);
        p_mac->macSa.buf = p_combo->mac_sa;
        p_mac->macSa.size = sizeof(mac_addr_t);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, MACDA))
    {
        GLB_OFPACT_FLD_SET(flags, MACDA);
        p_mac->macDa.buf = p_combo->mac_da;
        p_mac->macDa.size = sizeof(mac_addr_t);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SVLAN_VID))
    {
        GLB_OFPACT_FLD_SET(flags, SVLAN_VID);
        p_mac->sVlanId = p_combo->vlan_id;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SVLAN_COS))
    {
        GLB_OFPACT_FLD_SET(flags, SVLAN_COS);
        p_mac->sTagCos = p_combo->vlan_pcp;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, CVLAN_COS))
    {
        GLB_OFPACT_FLD_SET(flags, CVLAN_COS);
        p_mac->cTagCos = p_combo->cvlan_pcp;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, CVLAN_VID))
    {
        GLB_OFPACT_FLD_SET(flags, CVLAN_VID);
        p_mac->cVlanId = p_combo->cvlan_id;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, STRIP_SVLAN))
    {
        GLB_OFPACT_FLD_SET(flags, STRIP_SVLAN);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, STRIP_CVLAN))
    {
        GLB_OFPACT_FLD_SET(flags, STRIP_CVLAN);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, PUSH_SVLAN))
    {
        GLB_OFPACT_FLD_SET(flags, PUSH_SVLAN);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, PUSH_CVLAN))
    {
        GLB_OFPACT_FLD_SET(flags, PUSH_CVLAN);
    }

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    p_nh->flag1 = flags[0];
    p_nh->flag2 = flags[1];

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flex_nh_arp(hsrv_openflow_action_combo_t* p_combo, HalMsgOpenFlowFlexNh_t* p_nh)
{
    HalMsgOpenFlowArpEdit_t* p_arp = &p_nh->unionEdit.arpEdit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM] = {0};

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;
    
    GLB_OFPACT_FLD_SET(flags, ARP_);
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ARP_OP))
    {
        GLB_OFPACT_FLD_SET(flags, ARP_OP);
        p_arp->arpOp = p_combo->l3.arp.arp_op;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ARP_SPA))
    {
        GLB_OFPACT_FLD_SET(flags, ARP_SPA);
        p_arp->arpSpa = p_combo->l3.arp.arp_spa;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ARP_TPA))
    {
        GLB_OFPACT_FLD_SET(flags, ARP_TPA);
        p_arp->arpTpa = p_combo->l3.arp.arp_tpa;
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ARP_SHA))
    {
        GLB_OFPACT_FLD_SET(flags, ARP_SHA);
        p_arp->arpSha.buf = p_combo->l3.arp.arp_sha;
        p_arp->arpSha.size = sizeof(mac_addr_t);
    }
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ARP_THA))
    {
        GLB_OFPACT_FLD_SET(flags, ARP_THA);
        p_arp->arpTha.buf = p_combo->l3.arp.arp_tha;
        p_arp->arpTha.size = sizeof(mac_addr_t);
    }

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    p_nh->flag1 = flags[0];
    p_nh->flag2 = flags[1];

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flex_nh_ipv4(hsrv_openflow_action_combo_t* p_combo, HalMsgOpenFlowFlexNh_t* p_nh)
{
    HalMsgOpenFlowIpv4Edit_t* p_ipv4 = &p_nh->unionEdit.ipv4Edit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM] = {0};

    /* If action field is larger than 2 bytes, we should expand p_nh->flagX */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;
    
    GLB_OFPACT_FLD_SET(flags, IPV4_);
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPSA))
    {
        GLB_OFPACT_FLD_SET(flags, IPSA);
        p_ipv4->ipSa = p_combo->l3.ipv4.ipsa;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPDA))
    {
        GLB_OFPACT_FLD_SET(flags, IPDA);
        p_ipv4->ipDa = p_combo->l3.ipv4.ipda;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_DSCP))
    {
        GLB_OFPACT_FLD_SET(flags, IP_DSCP);
        p_ipv4->dscp = p_combo->l3.ipv4.dscp;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_ECN))
    {
        GLB_OFPACT_FLD_SET(flags, IP_ECN);
        p_ipv4->ecn = p_combo->l3.ipv4.ecn;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_TTL_))
    {
        GLB_OFPACT_FLD_SET(flags, IP_TTL_);
        p_ipv4->ttl = p_combo->l3.ipv4.ttl;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DEC_IP_TTL))
    {
        GLB_OFPACT_FLD_SET(flags, DEC_IP_TTL);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DEC_TTL))
    {
        GLB_OFPACT_FLD_SET(flags, DEC_TTL);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_PROTO))
    {
        GLB_OFPACT_FLD_SET(flags, IP_PROTO);
        p_ipv4->protocol = p_combo->l3.ipv4.proto;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, TCP_))
    {
        GLB_OFPACT_FLD_SET(flags, TCP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, UDP_))
    {
        GLB_OFPACT_FLD_SET(flags, UDP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SCTP_))
    {
        GLB_OFPACT_FLD_SET(flags, SCTP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SPORT))
    {
        GLB_OFPACT_FLD_SET(flags, SPORT);
        p_ipv4->l4SrcPort = p_combo->l4.l4port.src_port;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DPORT))
    {
        GLB_OFPACT_FLD_SET(flags, DPORT);
        p_ipv4->l4DestPort = p_combo->l4.l4port.dst_port;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ICMP_TYPE))
    {
        GLB_OFPACT_FLD_SET(flags, ICMP_TYPE);
        p_ipv4->icmpType = p_combo->l4.icmp.type;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ICMP_CODE))
    {
        GLB_OFPACT_FLD_SET(flags, ICMP_CODE);
        p_ipv4->icmpCode = p_combo->l4.icmp.code;
    }

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    p_nh->flag1 = flags[0];
    p_nh->flag2 = flags[1];

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flex_nh_ipv6(hsrv_openflow_action_combo_t* p_combo, HalMsgOpenFlowFlexNh_t* p_nh)
{
    HalMsgOpenFlowIpv6Edit_t* p_ipv6 = &p_nh->unionEdit.ipv6Edit;
    uint32 flags[GLB_OFPACT_FLD_FLAG_NUM] = {0};

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    flags[0] = p_nh->flag1;
    flags[1] = p_nh->flag2;

    GLB_OFPACT_FLD_SET(flags, IPV6_);
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPSA))
    {
        GLB_OFPACT_FLD_SET(flags, IPSA);
        p_ipv6->ipSa.buf = (uint8 *)p_combo->l3.ipv6.ipsa;
        p_ipv6->ipSa.size = sizeof(ipv6_addr_t);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPDA))
    {
        GLB_OFPACT_FLD_SET(flags, IPDA);
        p_ipv6->ipDa.buf = (uint8 *)p_combo->l3.ipv6.ipda;
        p_ipv6->ipDa.size = sizeof(ipv6_addr_t);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV6_LBL))
    {
        GLB_OFPACT_FLD_SET(flags, IPV6_LBL);
        p_ipv6->label = p_combo->l3.ipv6.label;
    }
    
    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_DSCP))
    {
        GLB_OFPACT_FLD_SET(flags, IP_DSCP);
        p_ipv6->dscp = p_combo->l3.ipv6.dscp;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_ECN))
    {
        GLB_OFPACT_FLD_SET(flags, IP_ECN);
        p_ipv6->ecn = p_combo->l3.ipv6.ecn;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_TTL_))
    {
        GLB_OFPACT_FLD_SET(flags, IP_TTL_);
        p_ipv6->ttl = p_combo->l3.ipv6.ttl;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DEC_IP_TTL))
    {
        GLB_OFPACT_FLD_SET(flags, DEC_IP_TTL);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DEC_TTL))
    {
        GLB_OFPACT_FLD_SET(flags, DEC_TTL);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IP_PROTO))
    {
        GLB_OFPACT_FLD_SET(flags, IP_PROTO);
        p_ipv6->protocol = p_combo->l3.ipv6.proto;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, TCP_))
    {
        GLB_OFPACT_FLD_SET(flags, TCP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, UDP_))
    {
        GLB_OFPACT_FLD_SET(flags, UDP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SCTP_))
    {
        GLB_OFPACT_FLD_SET(flags, SCTP_);
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, SPORT))
    {
        GLB_OFPACT_FLD_SET(flags, SPORT);
        p_ipv6->l4SrcPort = p_combo->l4.l4port.src_port;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, DPORT))
    {
        GLB_OFPACT_FLD_SET(flags, DPORT);
        p_ipv6->l4DestPort = p_combo->l4.l4port.dst_port;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ICMP_TYPE))
    {
        GLB_OFPACT_FLD_SET(flags, ICMP_TYPE);
        p_ipv6->icmpType = p_combo->l4.icmp.type;
    }

    if (GLB_OFPACT_FLD_ISSET(p_combo->flag, ICMP_CODE))
    {
        GLB_OFPACT_FLD_SET(flags, ICMP_CODE);
        p_ipv6->icmpCode = p_combo->l4.icmp.code;
    }

    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    p_nh->flag1 = flags[0];
    p_nh->flag2 = flags[1];
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_map_flex_nh(hsrv_openflow_action_combo_t* p_combo, HalMsgOpenFlowFlexNh_t* p_nh)
{
    uint32 tmp_flag[GLB_OFPACT_FLD_FLAG_NUM] = {0}; 

    sal_memcpy(tmp_flag, p_combo->flag, sizeof(tmp_flag));

    if (GLB_OFPACT_FLD_ISSET(tmp_flag, OUTPUT))
    {
        p_nh->gport = p_combo->output_gport;
    }

    /* added by hansf for v4/v6 transform */
    if (GLB_OPENFLOW_FLOW_TNL_TYPE_V4_TO_V6 == p_combo->flow_tunnel_type)
    {
        GLB_OFPACT_FLD_UNSET(tmp_flag, IPV4_);
    }
    else if (GLB_OPENFLOW_FLOW_TNL_TYPE_V6_TO_V4 == p_combo->flow_tunnel_type)
    {
        GLB_OFPACT_FLD_UNSET(tmp_flag, IPV6_);
    }
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flex_nh_mac(p_combo, p_nh));
    if (GLB_OFPACT_FLD_ISSET(tmp_flag, ARP_))
    {
        p_nh->unionEdit.present = unionEdit_PR_arpEdit;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flex_nh_arp(p_combo, p_nh));
    }
    else if (GLB_OFPACT_FLD_ISSET(tmp_flag, IPV4_))
    {
        p_nh->unionEdit.present = unionEdit_PR_ipv4Edit;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flex_nh_ipv4(p_combo, p_nh));
    }
    else if (GLB_OFPACT_FLD_ISSET(tmp_flag, IPV6_))
    {
        p_nh->unionEdit.present = unionEdit_PR_ipv6Edit;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flex_nh_ipv6(p_combo, p_nh));
    }

    return HSRV_E_NONE;
}
#define _____TUNNEL_NH_____
/* Added by weizj for tunnel */

static inline uint32
hsrv_openflow_nexthop_ip_tunnel_make(hsrv_ofact_ip_tunnel_nh_info_t* p_info)
{
    uint32 dsnh_offset;
    uint32 key = 0;

    if (!p_info)
    {
        return HSRV_E_INVALID_PTR;
    }

    dsnh_offset = p_info->nh_offset.offset;

    p_info->nh_offset.offset = 0;

    key = _hsrv_hash_make((uint8*)&p_info->param, sizeof(p_info->param));

    p_info->nh_offset.offset = dsnh_offset;

    return key % ADPT_NEXTHOP_IP_TUNNEL_NH_BLOCK_SIZE;
}

static bool
hsrv_openflow_nexthop_ip_tunnel_compare(hsrv_ofact_ip_tunnel_nh_info_t* p_info1,
    hsrv_ofact_ip_tunnel_nh_info_t* p_info2)
{
    bool is_same = TRUE;
    uint32 dsnh_offset1;
    uint32 dsnh_offset2;

    if (!p_info1 || !p_info2)
    {
        return HSRV_E_INVALID_PTR;
    }

    dsnh_offset1 = p_info1->nh_offset.offset;
    dsnh_offset2 = p_info2->nh_offset.offset;

    p_info1->nh_offset.offset = 0;
    p_info2->nh_offset.offset = 0;

    if (0 != sal_memcmp(&p_info1->param, &p_info2->param, sizeof(hsrv_openflow_ip_nh_tunnel_info_t)))
    {
        is_same = FALSE;
    }

    p_info1->nh_offset.offset = dsnh_offset1;
    p_info2->nh_offset.offset = dsnh_offset2;

    return is_same;
}

int32
hsrv_openflow_nexthop_lookup_ip_tunnel_nh(hsrv_openflow_ip_nh_tunnel_info_t* p_nh_param,
    hsrv_ofact_ip_tunnel_nh_info_t** pp_tnl_info)
{
    hsrv_ofact_ip_tunnel_nh_info_t lookup_key;

    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(pp_tnl_info);

    sal_memset(&lookup_key, 0x0, sizeof(hsrv_ofact_ip_tunnel_nh_info_t));
    sal_memcpy(&lookup_key.param, p_nh_param, sizeof(hsrv_openflow_ip_nh_tunnel_info_t));

    *pp_tnl_info = ctclib_hash_lookup(HSRV_OFNH_IP_TUNNEL_NH_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_add_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_info)
{
    HSRV_PTR_CHECK(p_info);
    if (ctclib_hash_lookup(HSRV_OFNH_IP_TUNNEL_NH_HASH, p_info))
    {
        HSRV_LOG_WARN("Tunnel nh hash add tunnel nh error");
    }
    else
    {
        ctclib_hash_get(HSRV_OFNH_IP_TUNNEL_NH_HASH, p_info, ctclib_hash_alloc_intern);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_info)
{
    HSRV_PTR_CHECK(p_info);

    ctclib_hash_release(HSRV_OFNH_IP_TUNNEL_NH_HASH, p_info);
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_map_ip_tunnel_nh(hsrv_openflow_ip_nh_tunnel_info_t* p_param,
    HalMsgOpenFlowTunnelNh_t* p_nh)
{
    HalMsgOpenFlowL2Edit_t* p_mac = NULL;
    HalMsgOpenFlowL2Edit_t* p_inner_mac = NULL;
    HalMsgOpenFlowIpv4Edit_t* p_ip = NULL;
    HalMsgOpenFlowTunnelEdit_t* p_tnl = NULL;
    uint8 mac_init[MAC_ADDR_LEN]; 
    
    p_nh->gport = p_param->gport;
    p_nh->fid = p_param->fid;
    sal_memset(mac_init, 0x0, MAC_ADDR_LEN);
    /* map tunnel info */
    p_tnl = &p_nh->tnl;
    p_tnl->type = p_param->tunnel_type;
    p_tnl->vid = p_param->vlan_id;
    p_tnl->ttl = p_param->ttl;
    p_tnl->dscpOrTos = p_param->dscp_or_tos;
    p_tnl->tunnelProtocol = p_param->inner.protocol_type;
    if (p_param->inner.tunnel_key)
    {
        GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL);
        p_tnl->tunnelKey = p_param->inner.tunnel_key;
    }
    /* Added by weizj for tunnel bug 33408 */
    if (p_param->strip_svlan)
    {
        GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN);
    }
    
    /* map mac */
    p_mac = &p_nh->macEdit;
    p_mac->macSa.buf = p_param->mac_sa;
    p_mac->macSa.size = sizeof(mac_addr_t); 
    GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC);

    p_mac->macDa.buf = p_param->mac_da;
    p_mac->macDa.size = sizeof(mac_addr_t);
    GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST);

    /* map ip */
    p_ip = &p_nh->ipv4Edit;
    p_ip->ipSa = p_param->ip_sa.ipv4;
    GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC);

    p_ip->ipDa = p_param->ip_da.ipv4;
    GLB_SET_BIT(p_nh->flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST);
    //more

    /* map inner mac */
    p_inner_mac = &p_nh->innerMacEdit;
    if(sal_memcmp(mac_init, p_param->inner_mac_sa, MAC_ADDR_LEN))
    {
        p_inner_mac->macSa.buf = p_param->inner_mac_sa;
        p_inner_mac->macSa.size = sizeof(mac_addr_t); 
        GLB_SET_BIT(p_nh->nextFlags, GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_SRC);
    }
    if(sal_memcmp(mac_init, p_param->inner_mac_da, MAC_ADDR_LEN))
    {
        p_inner_mac->macDa.buf = p_param->inner_mac_da;
        p_inner_mac->macDa.size = sizeof(mac_addr_t);
        GLB_SET_BIT(p_nh->nextFlags, GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_DST);
    }

    return HSRV_E_NONE;
}

#if 0
uint32
hsrv_openflow_nexthop_get_tunnel_hash_max(void)
{
    return g_pst_openflow_nexthop_master->nexthop_max_num;
}

uint32
hsrv_openflow_nexthop_get_tunnel_hash_count(void)
{
    return HSRV_OFNH_IP_TUNNEL_NH_HASH->count;
}

int32
hsrv_openflow_nexthop_check_ip_tunnel_nexthop_hash_count(void)
{
    if (hsrv_openflow_nexthop_get_tunnel_hash_count() > hsrv_openflow_nexthop_get_tunnel_hash_max())
    {
        return HSRV_E_EXCEED_MAX_SIZE;
    }

    return HSRV_E_NONE;
}
#endif

/* End by weizj for tunnel */


#define _____MPLS_NH_____

static inline uint32
_hsrv_openflow_mpls_tunnel_hash_make(hsrv_openflow_mpls_tunnel_t* p_mpls_tunnel)
{
    return _hsrv_hash_make((uint8*)&(p_mpls_tunnel->tunnel_param), sizeof(p_mpls_tunnel->tunnel_param));
}

static inline bool
_hsrv_openflow_mpls_tunnel_hash_compare(hsrv_openflow_mpls_tunnel_t* pva, hsrv_openflow_mpls_tunnel_t* pvb)
{
    if (!pva || !pvb)
    {
        return FALSE;
    }

    if (0 != memcmp(&pva->tunnel_param, &pvb->tunnel_param, sizeof(glb_openflow_mpls_tunnel_param_t)))
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_mpls_tunnel_lookup(glb_openflow_mpls_tunnel_param_t *p_param, hsrv_openflow_mpls_tunnel_t** pp_mpls_tunnel)
{
    hsrv_openflow_mpls_tunnel_t lookup_key;

    sal_memset(&lookup_key, 0, sizeof(hsrv_openflow_mpls_tunnel_t));
    sal_memcpy(&(lookup_key.tunnel_param), p_param, sizeof(glb_openflow_mpls_tunnel_param_t));

    *pp_mpls_tunnel = ctclib_hash_lookup(HSRV_OFNH_MPLS_TUNNEL_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_mpls_tunnel_save(uint32 tunnel_id, glb_openflow_mpls_tunnel_param_t *p_param, hsrv_openflow_mpls_tunnel_t** pp_mpls_tunnel)
{
    hsrv_openflow_mpls_tunnel_t* p_mpls_tunnel = NULL;

    p_mpls_tunnel = HSRV_OPENFLOW_CALLOC(sizeof(hsrv_openflow_mpls_tunnel_t));
    HSRV_OPENFLOW_MEM_CHECK(p_mpls_tunnel);
    sal_memset(p_mpls_tunnel, 0, sizeof(hsrv_openflow_mpls_tunnel_t));
    
    sal_memcpy(&(p_mpls_tunnel->tunnel_param), p_param, sizeof(glb_openflow_mpls_tunnel_param_t));
    ctclib_hash_get(HSRV_OFNH_MPLS_TUNNEL_HASH, p_mpls_tunnel, ctclib_hash_alloc_intern);

    /* Record tunnel id for later lookup and reuse. */
    p_mpls_tunnel->tunnel_id = tunnel_id;

    *pp_mpls_tunnel = p_mpls_tunnel;
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_mpls_tunnel_release(hsrv_openflow_mpls_tunnel_t* p_mpls_tunnel)
{
    ctclib_hash_release(HSRV_OFNH_MPLS_TUNNEL_HASH, p_mpls_tunnel);

    HSRV_OPENFLOW_FREE(p_mpls_tunnel);
    
    return HSRV_E_NONE;
}

static inline uint32
hsrv_openflow_nexthop_mpls_make(hsrv_ofact_mpls_nh_info_t* p_info)
{
    uint32 dsnh_offset;
    uint32 key = 0;

    if (!p_info)
    {
        return HSRV_E_INVALID_PTR;
    }

    dsnh_offset = p_info->nh_offset.offset;

    p_info->nh_offset.offset = 0;

    key = _hsrv_hash_make((uint8*)&p_info->param, sizeof(p_info->param));

    p_info->nh_offset.offset = dsnh_offset;

    return key % ADPT_NEXTHOP_MPLS_NH_BLOCK_SIZE;
}

static bool
hsrv_openflow_nexthop_mpls_compare(hsrv_ofact_mpls_nh_info_t* p_info1,
    hsrv_ofact_mpls_nh_info_t* p_info2)
{
    bool is_same = TRUE;
    uint32 dsnh_offset1;
    uint32 dsnh_offset2;

    if (!p_info1 || !p_info2)
    {
        return HSRV_E_INVALID_PTR;
    }

    dsnh_offset1 = p_info1->nh_offset.offset;
    dsnh_offset2 = p_info2->nh_offset.offset;

    p_info1->nh_offset.offset = 0;
    p_info2->nh_offset.offset = 0;

    if (0 != sal_memcmp(&p_info1->param, &p_info2->param, sizeof(hsrv_ofnh_mpls_param_t)))
    {
        is_same = FALSE;
    }

    p_info1->nh_offset.offset = dsnh_offset1;
    p_info2->nh_offset.offset = dsnh_offset2;

    return is_same;
}

static int32
hsrv_openflow_nexthop_lookup_mpls_nh(hsrv_ofnh_mpls_param_t* p_nh_param,
    hsrv_ofact_mpls_nh_info_t** pp_nh_info)
{
    hsrv_ofact_mpls_nh_info_t lookup_key;

    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(pp_nh_info);

    sal_memset(&lookup_key, 0x0, sizeof(hsrv_ofact_mpls_nh_info_t));
    sal_memcpy(&lookup_key.param, p_nh_param, sizeof(hsrv_ofnh_mpls_param_t));

    *pp_nh_info = ctclib_hash_lookup(HSRV_OFNH_MPLS_NH_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_add_mpls_nh(hsrv_ofact_mpls_nh_info_t* p_info)
{
    HSRV_PTR_CHECK(p_info);
    if (ctclib_hash_lookup(HSRV_OFNH_MPLS_NH_HASH, p_info))
    {
        HSRV_LOG_WARN("MPLS nh hash add mpls nh error");
    }
    else
    {
        ctclib_hash_get(HSRV_OFNH_MPLS_NH_HASH, p_info, ctclib_hash_alloc_intern);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_mpls_nh(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info)
{
    HSRV_PTR_CHECK(p_mpls_nh_info);

    ctclib_hash_release(HSRV_OFNH_MPLS_NH_HASH, p_mpls_nh_info);
    
    return HSRV_E_NONE;
}

/* modified by yaom for openflow MPLS L2VPN 20160214 */
#define _____IPUC_NH_____

static inline uint32
hsrv_openflow_nexthop_ipuc_make(hsrv_ofact_ipuc_nh_info_t* p_info)
{
    uint32 key = 0;

    if (!p_info)
    {
        return HSRV_E_INVALID_PTR;
    }

    key = _hsrv_hash_make((uint8*)&p_info->param, sizeof(p_info->param));

    return key % ADPT_NEXTHOP_IPUC_NH_BLOCK_SIZE;
}

static bool
hsrv_openflow_nexthop_ipuc_compare(hsrv_ofact_ipuc_nh_info_t* p_info1,
    hsrv_ofact_ipuc_nh_info_t* p_info2)
{
    bool is_same = TRUE;

    if (!p_info1 || !p_info2)
    {
        return HSRV_E_INVALID_PTR;
    }

    if (0 != sal_memcmp(&p_info1->param, &p_info2->param, sizeof(hsrv_ofnh_ipuc_param_t)))
    {
        is_same = FALSE;
    }

    return is_same;
}

static int32
hsrv_openflow_nexthop_lookup_ipuc_nh(hsrv_ofnh_ipuc_param_t* p_nh_param,
    hsrv_ofact_ipuc_nh_info_t** pp_nh_info)
{
    hsrv_ofact_ipuc_nh_info_t lookup_key;

    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(pp_nh_info);

    sal_memset(&lookup_key, 0x0, sizeof(hsrv_ofact_ipuc_nh_info_t));
    sal_memcpy(&lookup_key.param, p_nh_param, sizeof(hsrv_ofnh_ipuc_param_t));

    *pp_nh_info = ctclib_hash_lookup(HSRV_OFNH_IPUC_NH_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_add_ipuc_nh(hsrv_ofact_ipuc_nh_info_t* p_info)
{
    HSRV_PTR_CHECK(p_info);
    if (ctclib_hash_lookup(HSRV_OFNH_IPUC_NH_HASH, p_info))
    {
        HSRV_LOG_WARN("IPUC nh hash add ipuc nh error");
    }
    else
    {
        ctclib_hash_get(HSRV_OFNH_IPUC_NH_HASH, p_info, ctclib_hash_alloc_intern);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_ipuc_nh(hsrv_ofact_ipuc_nh_info_t* p_ipuc_nh_info)
{
    HSRV_PTR_CHECK(p_ipuc_nh_info);

    ctclib_hash_release(HSRV_OFNH_IPUC_NH_HASH, p_ipuc_nh_info);
    
    return HSRV_E_NONE;
}

#define _____XLATE_NH_____

static inline uint32
hsrv_openflow_nexthop_xlate_make(hsrv_ofact_xlate_nh_info_t* p_info)
{
    uint32 key = 0;

    if (!p_info)
    {
        return HSRV_E_INVALID_PTR;
    }

    key = _hsrv_hash_make((uint8*)&p_info->param, sizeof(p_info->param));

    return key % ADPT_NEXTHOP_XLATE_NH_BLOCK_SIZE;
}

static bool
hsrv_openflow_nexthop_xlate_compare(hsrv_ofact_xlate_nh_info_t* p_info1,
    hsrv_ofact_xlate_nh_info_t* p_info2)
{
    bool is_same = TRUE;

    if (!p_info1 || !p_info2)
    {
        return HSRV_E_INVALID_PTR;
    }

    if (0 != sal_memcmp(&p_info1->param, &p_info2->param, sizeof(hsrv_ofnh_xlate_param_t)))
    {
        is_same = FALSE;
    }

    return is_same;
}

static int32
hsrv_openflow_nexthop_lookup_xlate_nh(hsrv_ofnh_xlate_param_t* p_nh_param,
    hsrv_ofact_xlate_nh_info_t** pp_nh_info)
{
    hsrv_ofact_xlate_nh_info_t lookup_key;

    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(pp_nh_info);

    sal_memset(&lookup_key, 0x0, sizeof(hsrv_ofact_xlate_nh_info_t));
    sal_memcpy(&lookup_key.param, p_nh_param, sizeof(hsrv_ofnh_xlate_param_t));

    *pp_nh_info = ctclib_hash_lookup(HSRV_OFNH_XLATE_NH_HASH, &lookup_key);

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_add_xlate_nh(hsrv_ofact_xlate_nh_info_t* p_info)
{
    HSRV_PTR_CHECK(p_info);
    if (ctclib_hash_lookup(HSRV_OFNH_XLATE_NH_HASH, p_info))
    {
        HSRV_LOG_WARN("XLATE nh hash add xlate nh error");
    }
    else
    {
        ctclib_hash_get(HSRV_OFNH_XLATE_NH_HASH, p_info, ctclib_hash_alloc_intern);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_xlate_nh(hsrv_ofact_xlate_nh_info_t* p_xlate_nh_info)
{
    HSRV_PTR_CHECK(p_xlate_nh_info);

    ctclib_hash_release(HSRV_OFNH_XLATE_NH_HASH, p_xlate_nh_info);
    
    return HSRV_E_NONE;
}

#define _____ID_OFFSET_API_____

static int32
hsrv_openflow_nexthop_opf_alloc(hsrv_openflow_nexthop_opf_type_t type, uint32 size, uint32* p_offset)
{
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    
    HSRV_OPENFLOW_PTR_CHECK(p_offset);
    
    offset = ctclib_opf_alloc_offset(g_pst_openflow_nexthop_master->pst_opf[type], 0, size);
    if( CTCLIB_OPF_INVALID_OFFSET == offset )
    {
        HSRV_LOG_WARN("OpenFlow not enough offset resource.");
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }
    
    *p_offset = offset;
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_opf_release(hsrv_openflow_nexthop_opf_type_t type, uint32 size, uint32 offset)
{
    bool ret;
    
    ret = ctclib_opf_free_offset (g_pst_openflow_nexthop_master->pst_opf[type], 0, size, offset);
    if (FALSE == ret)
    {
        HSRV_LOG_WARN("OpenFlow offset resource release failed, offset = %d\n", offset);
        return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

#if 0
static int32
hsrv_openflow_nh_id_alloc(uint32* p_nh_id)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_opf_alloc(HSRV_OPENFLOW_NEXTHOP_OPF_NHID, p_nh_id));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nh_id_release(uint32 nh_id)
{
    hsrv_openflow_nexthop_opf_release(HSRV_OPENFLOW_NEXTHOP_OPF_NHID, nh_id);
    
    return HSRV_E_NONE;
}

static inline  int32
hsrv_openflow_nh_offset_alloc(uint32* p_offset, hsrv_of_nh_type_t nh_type)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_opf_alloc(HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET, p_nh_id));

    return HSRV_E_NONE;
}

static inline  int32
hsrv_openflow_nh_offset_release(uint32 offset, hsrv_of_nh_type_t nh_type)
{
    bool ret;
    
    ret = ctclib_opf_free_offset (HSRV_OFNH_NH_OFFSET_OPF, 0, 
                                  g_ofnh_offset_size_info[nh_type].entry_size, offset);
    if (FALSE == ret)
    {
        HSRV_LOG_WARN("OpenFlow offset resource release failed, offset = %d\n", offset);
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nh_mc_offset_alloc(uint32* p_offset)
{
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    
    HSRV_OPENFLOW_PTR_CHECK(p_offset);
    
    offset = ctclib_opf_alloc_offset (HSRV_OFNH_MC_OFFSET_OPF, 0, 1);
     if (CTCLIB_OPF_INVALID_OFFSET == offset)
    {
        HSRV_LOG_WARN("OpenFlow MC offset, no enough offset resource.\n");
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }
    
    *p_offset = offset;
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nh_mc_offset_release(uint32 offset)
{
    bool ret = TRUE;

    ret = ctclib_opf_free_offset (HSRV_OFNH_MC_OFFSET_OPF, 0, 1, offset);

    if (FALSE == ret)
    {
        HSRV_LOG_WARN("OpenFlow MC offset resource release failed, offset = %d.\n",
                      offset);
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
#endif

int32
hsrv_openflow_nexthop_offset_alloc(hsrv_of_nh_offset_t* p_nh_offset)
{
    hsrv_of_nh_type_t type;

    type = p_nh_offset->nh_type;
    if (g_ofnh_offset_size_info[type].opf_type_1)
    {
        hsrv_openflow_nexthop_id_opf_alloc_by_sai(&p_nh_offset->nhid);
        if (p_nh_offset->is_resvd)
        {
            g_pst_openflow_nexthop_master->resvd_opf[g_ofnh_offset_size_info[type].opf_type_1] ++;
        }
    }

    if (g_ofnh_offset_size_info[type].opf_type_2)
    {
        /*only multicast need offset, used as groupid, GG sdk is not needed offset for nexthop, */
        if(HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET == g_ofnh_offset_size_info[type].opf_type_2)
        {
            hsrv_openflow_mgroup_id_opf_alloc_by_sai(&p_nh_offset->offset);
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_opf_alloc(
                g_ofnh_offset_size_info[type].opf_type_2,
                g_ofnh_offset_size_info[type].opf_size_2,
                &p_nh_offset->offset));
            if (p_nh_offset->is_resvd)
            {
                g_pst_openflow_nexthop_master->resvd_opf[g_ofnh_offset_size_info[type].opf_type_2] ++;
            }
        }
    }
    g_pst_openflow_nexthop_master->nh_type_count[type]++;
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_offset_release(hsrv_of_nh_offset_t* p_nh_offset)
{
    hsrv_of_nh_type_t type;
    
    type = p_nh_offset->nh_type;
    
    if (g_ofnh_offset_size_info[type].opf_type_1)
    {
        hsrv_openflow_nexthop_id_opf_release_by_sai(p_nh_offset->nhid);
        if (p_nh_offset->is_resvd)
        {
            g_pst_openflow_nexthop_master->resvd_opf[g_ofnh_offset_size_info[type].opf_type_1] --;
        }
    }
    if (g_ofnh_offset_size_info[type].opf_type_2)
    {
         /*only multicast need offset, used as groupid, GG sdk is not needed offset for nexthop, */
        if(HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET == g_ofnh_offset_size_info[type].opf_type_2)
        {
            hsrv_openflow_mgroup_id_opf_free_by_sai(p_nh_offset->offset);
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_opf_release(
                g_ofnh_offset_size_info[type].opf_type_2,
                g_ofnh_offset_size_info[type].opf_size_2,
                p_nh_offset->offset));
            if (p_nh_offset->is_resvd)
            {
                g_pst_openflow_nexthop_master->resvd_opf[g_ofnh_offset_size_info[type].opf_type_2] --;
            }
        }
    }
    g_pst_openflow_nexthop_master->nh_type_count[type]--;
    
    return HSRV_E_NONE;
}

#define _____MESSAGE_API_____

static int32
hsrv_openflow_nexthop_send_mcast_group(hsrv_of_nh_offset_t* p_group_nh)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddMcastNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMcastNh;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMcastNh;
    p_req->nhid = p_group_nh->nhid;
    p_req->offset = p_group_nh->offset;
    p_req->statsEn = p_group_nh->stats_en;
    p_req->statsId = p_group_nh->stats_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_mcast_group(hsrv_of_nh_offset_t* p_group_nh)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelMcastNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMcastNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMcastNh;
    p_req->nhid = p_group_nh->nhid;
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_add_mcast_member(hsrv_ofnh_mcast_member_param_t* p_param)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddMcastMemberNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMcastMemberNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMcastMemberNh;
    p_req->nhid = p_param->group_nhid;
    p_req->memberInfo.nhid = p_param->mem_nhid;
    p_req->memberInfo.gport = p_param->mem_gport;
    p_req->memberInfo.portCheckDiscard = p_param->port_check_discard;
    p_req->memberInfo.redirectToIngressPort = p_param->redirect_to_ingress_port;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_mcast_member(hsrv_ofnh_mcast_member_param_t* p_param)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelMcastMemberNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMcastMemberNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMcastMemberNh;
    p_req->nhid = p_param->group_nhid;
    p_req->memberInfo.nhid = p_param->mem_nhid;
    p_req->memberInfo.gport = p_param->mem_gport;
    p_req->memberInfo.portCheckDiscard = p_param->port_check_discard;
    p_req->memberInfo.redirectToIngressPort = p_param->redirect_to_ingress_port;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

/* Added by weizj for tunnel */
int32
hsrv_openflow_nexthop_send_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddTunnelNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddTunnelNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddTunnelNh;
    p_req->nhid = p_info->nh_offset.nhid;
    p_req->offset = p_info->nh_offset.offset;
    p_req->statsEn = p_info->nh_offset.stats_en;
    p_req->statsId = p_info->nh_offset.stats_id;
    p_req->isUpdate = 0;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_map_ip_tunnel_nh(&p_info->param, &p_req->nhInfo));
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_update_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddTunnelNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddTunnelNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddTunnelNh;
    p_req->nhid = p_info->nh_offset.nhid;
    p_req->offset = p_info->nh_offset.offset;
    p_req->statsEn = p_info->nh_offset.stats_en;
    p_req->statsId = p_info->nh_offset.stats_id;
    p_req->isUpdate = 1;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_map_ip_tunnel_nh(&p_info->param, &p_req->nhInfo));
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_tnl_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelTunnelNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelTunnelNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelTunnelNh;
    p_req->nhid = p_tnl_info->nh_offset.nhid;
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}
/* End by weizj for tunnel */

/* for cubro mpls. */
int32
hsrv_openflow_nexthop_send_mpls_tunnel(hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddMplsTunnelReq_t *req;
    glb_openflow_mpls_tunnel_param_t *p_mpls_tunnel_param = &p_mpls_tunnel->tunnel_param;
        
    req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMplsTunnel;
    req->gport = p_mpls_tunnel_param->gport;
    req->nhMac.buf = p_mpls_tunnel_param->nexthop_mac;
    req->nhMac.size = MAC_ADDR_LEN;
    req->outerVlanVid = p_mpls_tunnel_param->vid;
    req->tunnelLabel.flag = p_mpls_tunnel_param->tunnel_label.label_flag;
    req->tunnelLabel.label = p_mpls_tunnel_param->tunnel_label.label;
    req->tunnelLabel.ttl = p_mpls_tunnel_param->tunnel_label.ttl;
    req->tunnelLabel.expType = p_mpls_tunnel_param->tunnel_label.exp_type;
    req->tunnelLabel.exp = p_mpls_tunnel_param->tunnel_label.exp;
    req->tunnelId = p_mpls_tunnel->tunnel_id;
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMplsTunnel;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_mpls_tunnel(uint32 tunnel_id)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelMplsTunnelReq_t *req;
        
    req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMplsTunnel;
    req->tunnelId = tunnel_id;

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMplsTunnel;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_send_mpls_nh(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info, hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel)
{
    Hsrv2AgtMsg_t hsrv_msg;
    hsrv_ofnh_mpls_param_t *p_mpls_param = &p_mpls_nh_info->param;
    hsrv_of_nh_offset_t *p_nh_offset = &p_mpls_nh_info->nh_offset;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    switch(p_mpls_nh_info->param.type)
    {
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN:
            {
                HalMsgOpenFlowAddPushL2vpnNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
                req->nhid = p_nh_offset->nhid;
                req->offset = p_nh_offset->offset;
                req->tunnelId = p_mpls_tunnel->tunnel_id;
                req->innerSvlanTagOp = p_mpls_param->u.push_l2vpn.inner_svlan_tag_op;
                /* modified by yaom for openflow MPLS L2VPN 20160214 */
                req->innerSvlanId = p_mpls_param->u.push_l2vpn.inner_svlan_id;                
                req->pwLabel.flag = p_mpls_param->u.push_l2vpn.pw_label.label_flag;
                req->pwLabel.label = p_mpls_param->u.push_l2vpn.pw_label.label;
                req->pwLabel.ttl = p_mpls_param->u.push_l2vpn.pw_label.ttl;
                req->pwLabel.expType = p_mpls_param->u.push_l2vpn.pw_label.exp_type;
                req->pwLabel.exp = p_mpls_param->u.push_l2vpn.pw_label.exp;
                
                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
            }
            break;
        
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN:
            {
                HalMsgOpenFlowAddPushL3vpnNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL3vpnNh;
                req->nhid = p_nh_offset->nhid;
                req->offset = p_nh_offset->offset;
                req->tunnelId = p_mpls_tunnel->tunnel_id;
                
                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL3vpnNh;
            }
            break;

        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS:
            {
                HalMsgOpenFlowAddSwapMplsNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddSwapMplsNh;
                req->nhid = p_nh_offset->nhid;
                req->offset = p_nh_offset->offset;
                req->tunnelId = p_mpls_tunnel->tunnel_id;
                req->pwLabel.flag = p_mpls_param->u.swap_mpls.pw_label.label_flag;
                req->pwLabel.label = p_mpls_param->u.swap_mpls.pw_label.label;
                req->pwLabel.ttl = p_mpls_param->u.swap_mpls.pw_label.ttl;
                req->pwLabel.expType = p_mpls_param->u.swap_mpls.pw_label.exp_type;
                req->pwLabel.exp = p_mpls_param->u.swap_mpls.pw_label.exp;
                
                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddSwapMplsNh;
            }
            break;

        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP:
            {
                HalMsgOpenFlowAddPopPHPNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPopPHPNh;
                req->nhid = p_nh_offset->nhid;
                req->offset = p_nh_offset->offset;
                req->gport = p_mpls_param->gport;
                req->nhMac.buf = p_mpls_param->u.pop_php.nexthop_mac;
                req->nhMac.size = MAC_ADDR_LEN;
                req->outerVlanVid = p_mpls_param->u.pop_php.outer_vlan_vid;
                
                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPopPHPNh;
            }
            break;
        
        default:
            return HSRV_E_INVALID_PARAM;
    }
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_mpls_nh(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    hsrv_of_nh_offset_t *p_nh_offset = &p_mpls_nh_info->nh_offset;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    switch(p_mpls_nh_info->param.type)
    {
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN:
            {
                HalMsgOpenFlowDelPushL2vpnNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPushL2vpnNh;
                req->nhid = p_nh_offset->nhid;

                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPushL2vpnNh;
            }
            break;

        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN:
            {
                HalMsgOpenFlowDelPushL3vpnNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPushL3vpnNh;
                req->nhid = p_nh_offset->nhid;

                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPushL3vpnNh;
            }
            break;

        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS:
            {
                HalMsgOpenFlowDelSwapMplsNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelSwapMplsNh;
                req->nhid = p_nh_offset->nhid;

                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelSwapMplsNh;
            }
            break;
            
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP:
            {
                HalMsgOpenFlowDelPopPHPNhReq_t *req;

                req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPopPHPNh;
                req->nhid = p_nh_offset->nhid;

                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPopPHPNh;
            }
            break;
        
        default:
            return HSRV_E_INVALID_PARAM;
    }
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

/* modified by yaom for openflow MPLS L2VPN 20160214 */
int32
hsrv_openflow_nexthop_send_ipuc_nh(hsrv_ofact_ipuc_nh_info_t* p_ipuc_nh_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddIpucNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddIpucNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddIpucNh;
    p_req->nhid = p_ipuc_nh_info->nh_offset.nhid;
    p_req->offset = p_ipuc_nh_info->nh_offset.offset;
    p_req->nhInfo.gport = p_ipuc_nh_info->param.gport;
    p_req->nhInfo.vid = p_ipuc_nh_info->param.vlan_vid;
    p_req->nhInfo.nhMac.buf = p_ipuc_nh_info->param.nexthop_mac;
    p_req->nhInfo.nhMac.size = MAC_ADDR_LEN;
    p_req->nhInfo.rewriteEtherType = p_ipuc_nh_info->param.rewrite_ether_type;
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_ipuc_nh(hsrv_ofact_ipuc_nh_info_t* p_ipuc_nh_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelIpucNhReq_t *req;

    req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelIpucNh;
    req->nhid = p_ipuc_nh_info->nh_offset.nhid;

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelIpucNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_send_xlate_nh(hsrv_ofact_xlate_nh_info_t* p_xlate_nh_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddXlateNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddXlateNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddXlateNh;
    p_req->nhid = p_xlate_nh_info->nh_offset.nhid;
    p_req->offset = p_xlate_nh_info->nh_offset.offset;
    p_req->nhInfo.gport = p_xlate_nh_info->param.gport;
    p_req->nhInfo.innerSvlanTagOp = p_xlate_nh_info->param.inner_svlan_tag_op;
    p_req->nhInfo.innerSvlanId = p_xlate_nh_info->param.inner_svlan_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_xlate_nh(hsrv_ofact_xlate_nh_info_t* p_xlate_nh_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelXlateNhReq_t *req;

    req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelXlateNh;
    req->nhid = p_xlate_nh_info->nh_offset.nhid;

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelXlateNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

#define _____ECMP_MESSAGE_API_____
static int32
hsrv_openflow_nexthop_send_ecmp_group(hsrv_of_nh_offset_t* p_group_nh)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddECMPNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddECMPNh;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddECMPNh;
    p_req->nhid = p_group_nh->nhid;
    p_req->offset = p_group_nh->offset;
    p_req->statsEn = p_group_nh->stats_en;
    p_req->statsId = p_group_nh->stats_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_nexthop_del_ecmp_group(hsrv_of_nh_offset_t* p_group_nh)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelECMPNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelECMPNh;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelECMPNh;
    p_req->nhid = p_group_nh->nhid;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_add_ecmp_member(uint32 group_nhid, uint32 stats_id, uint32 member_count, uint32* nhid_array, bool first)
{
    uint32 member_index;
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddECMPMemberNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddECMPMemberNh;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddECMPMemberNh;
    p_req->memberNhid.ecmpid = group_nhid;
    p_req->memberNhid.nhCnt = member_count;
    p_req->statsId = stats_id;
    p_req->isFirst = first;

    for (member_index = 0; member_index< p_req->memberNhid.nhCnt; member_index++)
    {
        ASN_SET_ADD(&(p_req->memberNhid.nhidList.list), (nhid_array + member_index));
    }

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    asn_set_empty(&(p_req->memberNhid.nhidList.list));
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_ecmp_member(uint32 group_nhid, uint32 member_count, uint32* nhid_array)
{
    uint32 member_index;
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelECMPMemberNhReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelECMPMemberNh;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelECMPMemberNh;
    p_req->memberNhid.ecmpid = group_nhid;
    p_req->memberNhid.nhCnt = member_count;

    for (member_index = 0; member_index< p_req->memberNhid.nhCnt; member_index++)
    {
        ASN_SET_ADD(&(p_req->memberNhid.nhidList.list), (nhid_array + member_index));
    }

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    asn_set_empty(&(p_req->memberNhid.nhidList.list));
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_send_flex_nh(hsrv_ofact_flex_nh_info_t* p_flex_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddFlexNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlexNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlexNh;
    p_req->nhid = p_flex_info->nh_offset.nhid;
    p_req->offset = p_flex_info->nh_offset.offset;
    p_req->statsEn = p_flex_info->nh_offset.stats_en;
    p_req->statsId = p_flex_info->nh_offset.stats_id;
    p_req->nhInfo.flowType = p_flex_info->combo.flow_tunnel_type;
    p_req->nhInfo.pldPacketType = p_flex_info->combo.payload_packet_type;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_map_flex_nh(&p_flex_info->combo, &p_req->nhInfo));
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_send_flex_cpu_nh(hsrv_ofact_flex_nh_info_t* p_flex_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddFlexCpuNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlexCpuNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlexCpuNh;
    p_req->nhid = p_flex_info->nh_offset.nhid;

    /* Modifed by weizj for inband */
    if (p_flex_info->nh_offset.is_resvd)
    {
        p_req->reason = GLB_PKT_OPENFLOW_TOCPU_LOCAL;
    }
    else
    {
        p_req->reason = GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN;
    }
    
    //TODO: delete this unneccesary field. p_req->cpuPort = GLB_SYSTEM_CPU_PORT;
    p_req->statsEn = p_flex_info->nh_offset.stats_en;
    p_req->statsId = p_flex_info->nh_offset.stats_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_remove_flex_nh(hsrv_ofact_flex_nh_info_t* p_flex_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelFlexNhReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelFlexNh;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelFlexNh;
    p_req->nhid = p_flex_info->nh_offset.nhid;
    p_req->flowType = p_flex_info->combo.flow_tunnel_type;
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

#define _____MODULE_API_____

int32
hsrv_openflow_nexthop_alloc_mcast_group(hsrv_ofnh_mcast_param_t* p_param, hsrv_of_nh_offset_t* p_group_nh)
{
    p_group_nh->stats_en = p_param->stats_en;
    p_group_nh->nh_type = HSRV_OF_NH_TYPE_NH_MCAST;
    p_group_nh->is_resvd = p_param->is_resvd;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_group_nh));

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_mcast_group(p_group_nh));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_alloc_ecmp_group(hsrv_ofnh_mcast_param_t* p_param, hsrv_of_nh_offset_t* p_group_nh)
{
    p_group_nh->stats_en = p_param->stats_en;
    p_group_nh->nh_type = HSRV_OF_NH_TYPE_NH_ECMP;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_group_nh));

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_ecmp_group(p_group_nh));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_alloc_flex_nh(hsrv_ofnh_flex_param_t* p_param, hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_flex_nh_info_t* p_flex_info;
    hsrv_of_nh_offset_t* p_nh_offset;

    hsrv_openflow_nexthop_lookup_flex_nh(p_param->p_action_combo, &p_flex_info);
    if (p_flex_info)
    {
        p_flex_info->ref_cnt ++;
        p_nh_info->share_info = p_flex_info;
        sal_memcpy(&p_nh_info->nh_offset, &p_flex_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        
        return HSRV_E_NONE;
    }
    
    p_nh_offset = &p_nh_info->nh_offset;
    p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_FLEX;
    p_nh_offset->is_resvd = p_param->p_action_combo->is_resvd_nh;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_nh_offset));
    
    p_flex_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_flex_nh_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_flex_info);
    sal_memset(p_flex_info, 0, sizeof(hsrv_ofact_flex_nh_info_t));
    
    sal_memcpy(&p_flex_info->combo, p_param->p_action_combo, sizeof(hsrv_openflow_action_combo_t));
    sal_memcpy(&p_flex_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_flex_info->ref_cnt = 1;
    p_nh_info->share_info = p_flex_info;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_flex_nh(p_flex_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_flex_nh(p_flex_info));
    
    return HSRV_E_NONE;
}

/* Added by weizj for tunnel */
int32
hsrv_openflow_nexthop_alloc_ip_tunnel_nh(hsrv_openflow_ip_nh_tunnel_info_t* p_nh_param,
    hsrv_of_nh_info_t* p_mem_info)
{
    hsrv_ofact_ip_tunnel_nh_info_t* p_tnl_info = NULL;
    hsrv_of_nh_offset_t* p_nh_offset = NULL;
    
    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(p_mem_info);

    /* tunnel nh exist */
    hsrv_openflow_nexthop_lookup_ip_tunnel_nh(p_nh_param, &p_tnl_info);
    if (p_tnl_info)
    {
        p_tnl_info->ref_cnt ++;
        p_mem_info->share_info = p_tnl_info;
        sal_memcpy(&p_mem_info->nh_offset, &p_tnl_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        p_mem_info->gport = p_nh_param->gport;
        
        return HSRV_E_NONE;
    }

    // NO NEED TO DO THIS IN HSRV. HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_check_ip_tunnel_nexthop_hash_count());

    /* alloc tunnel nh */
    p_nh_offset = &p_mem_info->nh_offset;
    if (0 < p_nh_param->inner.tunnel_key)
    {
        p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID;
    }
    else
    {
        p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL;
    }
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_nh_offset));

    /* alloc memery */
    p_tnl_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_ip_tunnel_nh_info_t));
    HSRV_MEM_CHECK(p_tnl_info);
    sal_memset(p_tnl_info, 0, sizeof(hsrv_ofact_ip_tunnel_nh_info_t));

    sal_memcpy(&p_tnl_info->param, p_nh_param, sizeof(hsrv_openflow_ip_nh_tunnel_info_t));
    sal_memcpy(&p_tnl_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_tnl_info->ref_cnt = 1;
    p_mem_info->share_info = p_tnl_info;
    p_mem_info->gport = p_nh_param->gport;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_ip_tunnel_nh(p_tnl_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ip_tunnel_nh(p_tnl_info));

    p_tnl_info = NULL;
    p_nh_offset = NULL;

    return HSRV_E_NONE;
}
/* End by weizj for tunnel */

int32
hsrv_openflow_nexthop_alloc_cpu_flex_nh(hsrv_ofnh_cpu_flex_param_t* p_param, hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofact_flex_nh_info_t* p_flex_info;
    hsrv_of_nh_offset_t* p_nh_offset;
    hsrv_ofnh_flex_param_t param;
    
    sal_memset(&param, 0, sizeof(hsrv_ofnh_flex_param_t));
    sal_memset(&action_combo, 0, sizeof(hsrv_openflow_action_combo_t));
    
    action_combo.output_gport = GLB_SYSTEM_CPU_PORT;
    GLB_OFPACT_FLD_SET(action_combo.flag, OUTPUT);
    action_combo.is_resvd_nh = p_param->is_local;
    hsrv_openflow_nexthop_lookup_flex_nh(&action_combo, &p_flex_info);
    if (p_flex_info)
    {
        p_flex_info->ref_cnt ++;
        p_nh_info->share_info = p_flex_info;
        sal_memcpy(&p_nh_info->nh_offset, &p_flex_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        
        return HSRV_E_NONE;
    }
    
    p_nh_offset = &p_nh_info->nh_offset;
    p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_FLEX_CPU;
    p_nh_offset->is_resvd = p_param->is_local;
    p_nh_offset->nhid = ctc_sai_copp_get_nhid_for_controller();
    
    p_flex_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_flex_nh_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_flex_info);
    sal_memset(p_flex_info, 0, sizeof(hsrv_ofact_flex_nh_info_t));
    
    sal_memcpy(&p_flex_info->combo, &action_combo, sizeof(hsrv_openflow_action_combo_t));
    sal_memcpy(&p_flex_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_flex_info->ref_cnt = 1;
    p_nh_info->share_info = p_flex_info;
    /* Added by weizj for inband */
    if (p_param->is_local)
    {
        p_flex_info->nh_offset.is_resvd = TRUE;
    }
    
    //HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_flex_cpu_nh(p_flex_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_flex_nh(p_flex_info));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_alloc_port_flex_nh(uint16 gport, hsrv_of_nh_info_t* p_info)
{
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_flex_param_t param;
    
    sal_memset(&param, 0, sizeof(hsrv_ofnh_flex_param_t));
    sal_memset(&action_combo, 0, sizeof(hsrv_openflow_action_combo_t));
    
    action_combo.is_resvd_nh = TRUE;
    action_combo.output_gport = gport;
    GLB_OFPACT_FLD_SET(action_combo.flag, OUTPUT);
    
    param.p_action_combo = &action_combo;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, p_info));
    
    return HSRV_E_NONE;
}

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
int32 
hsrv_openflow_nexthop_alloc_tpoam_nh(uint16 out_gport, glb_openflow_tpoam_t* flow, uint32* nhid, uint32* offset)
{
    int32                                            rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                               hagt_msg;
    hsrv_of_nh_offset_t                        nh_offset;
    ctclib_hash_backet_t*                     hb;
    ctclib_hash_backet_t*                     next;
    hsrv_openflow_mpls_tunnel_t*         p_mpls_tunnel = NULL;
    HalMsgOpenFlowSectionNh_t*           req_section;
    uint32 i = 0;
    bool tunnel_exist = FALSE;
    glb_openflow_mpls_tunnel_param_t   tunnel_param;
    hsrv_ofact_mpls_nh_info_t*              p_nh_info = NULL;
    bool                                               pw_nexthop_exist = FALSE;
    hsrv_openflow_group_t*                   p_group = NULL;
    hsrv_openflow_lsp_aps_nh_t             nh;
    hsrv_openflow_lsp_aps_nh_t*           pst_nh = NULL;
    hsrv_ofnh_mpls_param_t                  mpls_param;
    
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&tunnel_param, 0, sizeof(glb_openflow_mpls_tunnel_param_t));
    sal_memset(&nh, 0, sizeof(nh));
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    
    if (GLB_OAM_TYPE_MPLS_SECTION == flow->oam_type)
    {
        nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IPUC;
        hsrv_openflow_nexthop_offset_alloc(&nh_offset);

        req_section = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddSectionNh;
        req_section->nhid = nh_offset.nhid;
        req_section->offset = nh_offset.offset;

        req_section->nhInfo.gport = out_gport;
        req_section->nhInfo.nhMac.buf = flow->nexthop_mac;
        req_section->nhInfo.nhMac.size = MAC_ADDR_LEN;
    
        HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
        hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddSectionNh;
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

        *nhid = nh_offset.nhid;
        *offset = nh_offset.offset;
    
        return rc;
    }

    if (GLB_OAM_TYPE_MPLS_LSP == flow->oam_type)
    {
        /* aps group */
        if (flow->aps_group_id || flow->lsp_aps_group_id) 
        {
            if (flow->lsp_aps_group_id)
            {
                hsrv_openflow_groupdb_lookup_group(flow->lsp_aps_group_id, &p_group);
            }
            else
            {
                hsrv_openflow_groupdb_lookup_group(flow->aps_group_id, &p_group);
            }
            
            if (p_group)
            {
                if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
                {
                    nh.pw_label = flow->outlabel_pw;
                    pst_nh = ctclib_hash_lookup (p_group->aps_group.lsp_aps_nh_hash, (void*)&nh);
                    if (pst_nh)
                    {
                        *nhid = pst_nh->nhid;
                        *offset = pst_nh->offset;   
                        return rc;
                    }
                }
            }

            return HSRV_E_INVALID_PARAM; 
        }
        
        /* find tunnel by tunnel_param */
        tunnel_param.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        tunnel_param.tunnel_label.ttl = flow->lsp_ttl;
        tunnel_param.vid = flow->vid;

        if (flow->lsp_outlabel_exp_is_set)
        {
            tunnel_param.tunnel_label.exp_type = CTC_NH_EXP_SELECT_ASSIGN;
            tunnel_param.tunnel_label.exp = flow->lsp_exp;
        }
        else
        {
            tunnel_param.tunnel_label.exp_type = CTC_NH_EXP_SELECT_MAP;
        }
        tunnel_param.tunnel_label.label = flow->outlabel_lsp;
        tunnel_param.gport = out_gport;
        sal_memcpy(tunnel_param.nexthop_mac, flow->nexthop_mac, sizeof(mac_addr_t));

        hsrv_openflow_mpls_tunnel_lookup(&tunnel_param, &p_mpls_tunnel);

        if (p_mpls_tunnel)
        {
            tunnel_exist = TRUE;
        }
        else
        {
            /* find tunnel from oam gport and outlabel */
            for (i = 0; i < HSRV_OFNH_MPLS_TUNNEL_HASH->size; i++)
            {
                for (hb = HSRV_OFNH_MPLS_TUNNEL_HASH->index[i]; hb; hb = next)
                {
                    next = hb->next;
                    p_mpls_tunnel = (hsrv_openflow_mpls_tunnel_t *)hb->data;
        
                    if (p_mpls_tunnel)
                    {
                        if ((p_mpls_tunnel->tunnel_param.gport == out_gport)
                            && (p_mpls_tunnel->tunnel_param.tunnel_label.label == flow->outlabel_lsp))
                        {
                            tunnel_exist = TRUE;
                            break;    
                        }
                    }
                }
        
                if (tunnel_exist)
                {
                    break;    
                }
            }
        }

        if ((tunnel_exist) && (p_mpls_tunnel))
        {
            *nhid = p_mpls_tunnel->nh_id;
            *offset = p_mpls_tunnel->offset;  
        }
        return rc;
    }
    else
    {
        /* aps group */
        if ((flow->aps_group_id || flow->lsp_aps_group_id) && (GLB_INVALID_GPORTID == out_gport))
        {
            if (flow->lsp_aps_group_id)
            {
                hsrv_openflow_groupdb_lookup_group(flow->lsp_aps_group_id, &p_group);
            }
            else
            {
                hsrv_openflow_groupdb_lookup_group(flow->aps_group_id, &p_group);
            }
            
            if (p_group)
            {
                if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
                {
                    nh.pw_label = flow->outlabel_pw;
                    pst_nh = ctclib_hash_lookup (p_group->aps_group.lsp_aps_nh_hash, (void*)&nh);
                    if (pst_nh)
                    {
                        *nhid = pst_nh->nhid;
                        *offset = pst_nh->offset;   
                        return rc;
                    }
                }
                else
                {
                    *nhid = p_group->aps_group.aps_nhid;
                    *offset = p_group->aps_group.aps_offset;
                    return rc;
                }
            }

            return HSRV_E_INVALID_PARAM; 
        }
        
        /* find nh_info by mpls_param*/
        mpls_param.gport = out_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
        mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_KEEP_UNCHANGE;
        
        sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), flow->nexthop_mac, sizeof(mpls_param.u.push_l2vpn.nexthop_mac));

        if (flow->vid)
        {
            mpls_param.u.push_l2vpn.outer_vlan_vid = flow->vid;
        }
        else
        {
            mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;   
        }
        
        mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        mpls_param.u.push_l2vpn.tunnel_label.label = flow->outlabel_lsp;
        
        if (flow->lsp_outlabel_exp_is_set)
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
            mpls_param.u.push_l2vpn.tunnel_label.exp = flow->lsp_exp;
        }
        else
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
            mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        mpls_param.u.push_l2vpn.tunnel_label.ttl = flow->lsp_ttl;
        
        mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        mpls_param.u.push_l2vpn.pw_label.label = flow->outlabel_pw;
        
        if (flow->pw_outlabel_exp_is_set)
        {
            mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
            mpls_param.u.push_l2vpn.pw_label.exp = flow->pw_exp;
        }
        else
        {
            mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
            mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        
        mpls_param.u.push_l2vpn.pw_label.ttl = flow->pw_ttl;
        
        hsrv_openflow_nexthop_lookup_mpls_nh(&mpls_param, &p_nh_info);

        if (p_nh_info)
        {
            pw_nexthop_exist = TRUE;
        }
        else
        {
            /* find nh_info from oam gport and outlabel */
            for (i = 0; i < HSRV_OFNH_MPLS_NH_HASH->size; i++)
            {
                for (hb = HSRV_OFNH_MPLS_NH_HASH->index[i]; hb; hb = next)
                {
                    next = hb->next;
                    p_nh_info = (hsrv_ofact_mpls_nh_info_t *)hb->data;
        
                    if (p_nh_info)
                    {
                        if (p_nh_info->param.u.push_l2vpn.tunnel_label.label_flag
                            && p_nh_info->param.u.push_l2vpn.pw_label.label_flag
                            && (p_nh_info->param.gport == out_gport)
                            && (p_nh_info->param.u.push_l2vpn.tunnel_label.label == flow->outlabel_lsp)
                            && (p_nh_info->param.u.push_l2vpn.pw_label.label == flow->outlabel_pw))
                        {
                            pw_nexthop_exist = TRUE;
                            break;    
                        }
                    }
                }
        
                if (pw_nexthop_exist)
                {
                    break;    
                }
            }
        }

        if ((pw_nexthop_exist) && p_nh_info)
        {
            *nhid = p_nh_info->nh_offset.nhid;
            *offset = p_nh_info->nh_offset.offset;    
        }
        
        return rc; 
    }

    return rc;
}

int32 
hsrv_openflow_nexthop_release_tpoam(uint32 oam_type, uint32 nhid, uint32 offset)
{
    int32 rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t hagt_msg;
    hsrv_of_nh_offset_t   nh_offset;
    
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));

    if (GLB_OAM_TYPE_MPLS_SECTION != oam_type) 
    {
        return rc;
    }
    
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);

    /* release oam nexthop from sdk */
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelSectionNh.nhid = nhid;
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelSectionNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    /* release nexthop from database */
    nh_offset.nhid = nhid;
    nh_offset.offset = offset;
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IPUC;      
    hsrv_openflow_nexthop_offset_release(&nh_offset);

    return rc;
}

int32 
hsrv_openflow_nexthop_alloc_lsp_aps_nh(
    uint32 tunnel_id, uint32 outlabel_pw, uint32* nhid, uint32* offset, uint32* mcastnhid, uint32* mcastoffset, ctclib_hash_t* lsp_aps_nh_hash)
{
    hsrv_of_nh_offset_t  nh_offset;
    hsrv_of_nh_offset_t  mcast_nh_offset;
    
    Hsrv2AgtMsg_t                                      hagt_msg;
    int32                                                    rc = HSRV_E_NONE;
    HalMsgOpenFlowAddPushL2vpnNhReq_t*   req = NULL;
    hsrv_openflow_lsp_aps_nh_t*                  pst_nh = NULL;
    hsrv_openflow_lsp_aps_nh_t                    nh;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(&mcast_nh_offset, 0, sizeof(mcast_nh_offset));

    HSRV_OPENFLOW_MEM_CHECK(lsp_aps_nh_hash);

    nh.pw_label = outlabel_pw;
    pst_nh = ctclib_hash_lookup (lsp_aps_nh_hash, (void*)&nh);
    if (pst_nh)
    {
        pst_nh->count++;
        *nhid = pst_nh->nhid;
        *offset = pst_nh->offset;
        *mcastnhid = pst_nh->mcastnhid;
        *mcastoffset = pst_nh->mcastoffset;

        return rc;
    }
    
    /* allocate oam nexthop */
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MPLS;

    hsrv_openflow_nexthop_offset_alloc(&nh_offset);
    req = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
    req->nhid = nh_offset.nhid;
    req->offset = nh_offset.offset;
    req->tunnelId = tunnel_id;
    req->innerSvlanTagOp = 0;

/*modified by yejl for bug 52155, 2019-05-22*/
#ifdef USW
    mcast_nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(&mcast_nh_offset));
    req->mcastNhid = mcast_nh_offset.nhid;
    req->mcastOffset = mcast_nh_offset.offset;
#endif
/*ended*/

    if (outlabel_pw)
    {
        req->pwLabel.flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        req->pwLabel.label = outlabel_pw;
        req->pwLabel.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
        req->pwLabel.expType = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        req->pwLabel.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP; 
    }

    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    *nhid = nh_offset.nhid;
    *offset = nh_offset.offset;
    *mcastnhid = mcast_nh_offset.nhid;
    *mcastoffset = mcast_nh_offset.offset;

    /* add lsp aps nhid to hash table */
    pst_nh = HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW, sizeof(hsrv_openflow_lsp_aps_nh_t));
    HSRV_OPENFLOW_MEM_CHECK(pst_nh);
    pst_nh->pw_label = outlabel_pw;
    pst_nh->nhid = nh_offset.nhid;
    pst_nh->offset = nh_offset.offset;
    pst_nh->mcastnhid = mcast_nh_offset.nhid;
    pst_nh->mcastoffset = mcast_nh_offset.offset;
    pst_nh->count++;
    ctclib_hash_get(lsp_aps_nh_hash, (void*)pst_nh, ctclib_hash_alloc_intern);
   
    return rc;
}

int32 
hsrv_openflow_nexthop_release_aps(uint32 nhid, uint32 mcastnhid)
{
    Hsrv2AgtMsg_t          hagt_msg;
    int32                        rc = HSRV_E_NONE;
    HalMsgOpenFlowDelPushL2vpnNhReq_t   *req = NULL;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
 
    req = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelPushL2vpnNh;
    req->nhid = nhid;
    req->mcastNhid = mcastnhid;/*added by yejl for D2&TM chip, GG&GB does not care*/

    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelPushL2vpnNh;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    return rc;
}

int32
hsrv_openflow_nexthop_release_lsp_aps_nh(ctclib_hash_t*  lsp_aps_nh_hash, uint32  pw_label)
{
    int32                                           rc = HSRV_E_NONE;
    hsrv_openflow_lsp_aps_nh_t          nh;
    hsrv_openflow_lsp_aps_nh_t*         pst_nh = NULL;
    hsrv_of_nh_offset_t                       nh_offset;

    sal_memset(&nh, 0, sizeof(nh));

    HSRV_OPENFLOW_MEM_CHECK(lsp_aps_nh_hash);

    nh.pw_label = pw_label;
    pst_nh = ctclib_hash_lookup (lsp_aps_nh_hash, (void*)&nh);
    if (pst_nh)
    {
        pst_nh->count--;
        if (0 == pst_nh->count)
        {
            /* modified by liwh for bug 47434, 2018-05-28 */
            sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
            nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MPLS;
            nh_offset.nhid = pst_nh->nhid;
            nh_offset.offset = pst_nh->offset;
            hsrv_openflow_nexthop_offset_release(&nh_offset);
            /* liwh end */

            /*modified by yejl for bug 52155, 2019-05-22*/
            if (pst_nh->mcastnhid || pst_nh->mcastoffset)
            {
                sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
                nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;
                nh_offset.nhid = pst_nh->mcastnhid;
                nh_offset.offset = pst_nh->mcastoffset;
                hsrv_openflow_nexthop_offset_release(&nh_offset);
            }
            /*ended by yejl*/
    
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_aps(pst_nh->nhid, pst_nh->mcastnhid));
            ctclib_hash_release(lsp_aps_nh_hash,  (void*)pst_nh);
            HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_nh);
        }
    }

    return rc;
}

int32 
hsrv_openflow_nexthop_alloc_pw_aps_nh(hsrv_openflow_pw_aps_nh_info_t* p_nh_info, uint32* nhid, uint32* offset, uint32* mcastnhid, uint32* mcastoffset)
{
    hsrv_of_nh_offset_t   nh_offset;
    hsrv_of_nh_offset_t   mcast_nh_offset;
    Hsrv2AgtMsg_t          hagt_msg;
    int32                        rc = HSRV_E_NONE;
    HalMsgOpenFlowAddApsPushL2vpnNhReq_t   *req = NULL;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&mcast_nh_offset, 0, sizeof(mcast_nh_offset));
    
    /* allocate oam nexthop */
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MPLS;
    hsrv_openflow_nexthop_offset_alloc(&nh_offset);
    req = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddApsPushL2vpnNh;
    req->nhid = nh_offset.nhid;
    req->offset = nh_offset.offset;
    req->apsGroupId = p_nh_info->aps_id;
    req->gport = p_nh_info->gport;
    req->pgport = p_nh_info->pgport;
    req->lspApsGroupId = p_nh_info->lsp_aps_group_id;
    req->plspApsGroupId = p_nh_info->plsp_aps_group_id;

    /*modified by yejl for bug 52155, 2019-05-22*/
    #ifdef USW
    mcast_nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;
    hsrv_openflow_nexthop_offset_alloc(&mcast_nh_offset);
    req->mcastNhid = mcast_nh_offset.nhid;
    req->mcastOffset = mcast_nh_offset.offset;
    #endif
    /*ended by yejl*/

    req->tunnelId = p_nh_info->tunnel_id;
    req->innerSvlanTagOp = 0;
    req->pwLabel.flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    req->pwLabel.label = p_nh_info->pw_label;
    req->pwLabel.ttl = p_nh_info->mpls_ttl0;
    req->pwLabel.expType = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    req->pwLabel.exp = p_nh_info->mpls_tc0; 

    req->ptunnelId = p_nh_info->ptunnel_id;
    req->pinnerSvlanTagOp = 0;
    req->ppwLabel.flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    req->ppwLabel.label = p_nh_info->ppw_label;
    req->ppwLabel.ttl = p_nh_info->mpls_ttl1;
    req->ppwLabel.expType = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_PACKET;
    req->ppwLabel.exp = p_nh_info->mpls_tc1;

    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddApsPushL2vpnNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    *nhid = nh_offset.nhid;
    *offset = nh_offset.offset;
    *mcastnhid = mcast_nh_offset.nhid;
    *mcastoffset = mcast_nh_offset.offset;
   
    return rc;
}

int32 
hsrv_openflow_nexthop_release_pw_aps_nh(uint32  aps_group_id, uint32 nhid, uint32 offset, uint32 mcastnhid, uint32 mcastoffset)
{
    Hsrv2AgtMsg_t                             hagt_msg;
    int32                                           rc = HSRV_E_NONE;
    hsrv_of_nh_offset_t                       nh_offset;

    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    /* modified by liwh for bug 47434, 2018-05-28 */
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MPLS;
    nh_offset.nhid =nhid;
    nh_offset.offset = offset;
    hsrv_openflow_nexthop_offset_release(&nh_offset);
    /* liwh end */

    /*modified by yejl for bug 52155, 2019-05-22*/
#ifdef USW
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;
    nh_offset.nhid = mcastnhid;
    nh_offset.offset = mcastoffset;
    hsrv_openflow_nexthop_offset_release(&nh_offset);
#endif
    /* ended by yejl */

    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh.apsGroupId = aps_group_id;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh.nhid = nhid;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh.mcastNhid = mcastnhid;
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelApsPushL2vpnNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));
   
    return rc;
}


int32
hsrv_openflow_alloc_mpls_tunnelid(uint32* p_tunnelId)
{
    int32 rc = HSRV_E_NONE;
    uint32 tunnel_id = 0;
    
    tunnel_id = ctclib_opf_alloc_offset(HSRV_OFNH_MPLS_TUNNEL_ID_OPF, 0, 1);
    if (tunnel_id == CTCLIB_OPF_INVALID_OFFSET)
    {
        /* NOTE: It's hard for OVS adapter to detect this error because OVS adapter can not parse
           every mpls nexthop easilly. 
           So we just warn this limit has reached and skip to add the nexthop to mcast. */
        HSRV_LOG_WARN("MPLS tunnel id resource is full, this nexthop will not working.");
        return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
    }

    *p_tunnelId = tunnel_id;

    return rc;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_add_lsp_aps_tunnel
 * Purpose      : Fea layer add lsp aps group tunnel
 * Input         : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_add_lsp_aps_tunnel(glb_openflow_tpoam_bucket_t*   p_tpoam_bucket_w, glb_openflow_tpoam_bucket_t*   p_tpoam_bucket_p,
    uint32* p_aps_group_offset, uint32* p_tunnel_id)
{
    hsrv_ofnh_mpls_param_t           mpls_param;
    Hsrv2AgtMsg_t                         hagt_msg;
    uint32                                     tunnel_id = 0;
    uint32                                     aps_group_offset;
    
    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket_w);
    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket_p);

    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    HSRV_IF_ERROR_RETURN(hsrv_openflow_alloc_mpls_tunnelid(&tunnel_id));
    HSRV_IF_ERROR_RETURN(hsrv_aps_group_offset_alloc(&aps_group_offset));

    /* add working lsp nh and tunnel */
    p_tpoam_bucket_w->p_nh_info = HSRV_OPENFLOW_GROUP_CALLOC(sizeof(hsrv_of_nh_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket_w->p_nh_info);

    mpls_param.gport = p_tpoam_bucket_w->gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE;    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(p_tpoam_bucket_w->nexthop_mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    mpls_param.u.push_l2vpn.outer_vlan_vid = 0;    
    mpls_param.u.push_l2vpn.pw_label.label_flag = 0;
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    mpls_param.u.push_l2vpn.tunnel_label.label = p_tpoam_bucket_w->label1;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, p_tpoam_bucket_w->p_nh_info));

    /* add protection lsp nh and tunnel */
    p_tpoam_bucket_p->p_nh_info = HSRV_OPENFLOW_GROUP_CALLOC(sizeof(hsrv_of_nh_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket_p->p_nh_info);

    mpls_param.gport = p_tpoam_bucket_p->gport;
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(p_tpoam_bucket_p->nexthop_mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    mpls_param.u.push_l2vpn.tunnel_label.label = p_tpoam_bucket_p->label1;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, p_tpoam_bucket_p->p_nh_info));

    /* add lsp aps tunnel */
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.mapTTL1 = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.gportid = p_tpoam_bucket_w->gport;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.vid = p_tpoam_bucket_w->vid;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.macAddr.buf = p_tpoam_bucket_w->nexthop_mac;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.macAddr.size = sizeof(mac_addr_t);
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.labelNumber = 1;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.tunnelLabel1 = p_tpoam_bucket_w->label1;

    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.pmapTTL1 = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.pgportid = p_tpoam_bucket_p->gport;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.pvid = p_tpoam_bucket_p->vid;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.pmacAddr.buf = p_tpoam_bucket_p->nexthop_mac;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.pmacAddr.size = sizeof(mac_addr_t);
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.plabelNumber = 1;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.ptunnelLabel1 = p_tpoam_bucket_p->label1;  
         
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddLspApsTunnel;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.tunnelId = tunnel_id;

    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.apsGroupId = aps_group_offset;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLspApsTunnel.apsType = GLB_MPLS_ONE_LEVEL_APS;

    HSRV_IF_ERROR_RETURN(hsrv_message_send_msg_to_hagt(&hagt_msg));

    *p_tunnel_id = tunnel_id;
    *p_aps_group_offset = aps_group_offset;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_delete_lsp_aps_tunnel(uint32 aps_group_offset, uint32 tunnel_id)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowLspApsTunnelReq_t *req = NULL;

    hsrv_aps_group_offset_release(aps_group_offset);
    ctclib_opf_free_offset(HSRV_OFNH_MPLS_TUNNEL_ID_OPF, 0, 1, tunnel_id);
        
    req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLspApsTunnel;
    req->apsGroupId = aps_group_offset;
    req->tunnelId = tunnel_id;

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLspApsTunnel;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}


/*************************************************************************************************
 * Name         : hsrv_openflow_add_pw_tunnel
 * Purpose      : Fea layer add pw tunnel for pw aps group
 * Input         : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_add_pw_tunnel(glb_openflow_tpoam_bucket_t*   p_tpoam_bucket, uint32* p_tunnel_id)
{
    hsrv_ofnh_mpls_param_t           mpls_param;

    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket);

    sal_memset(&mpls_param, 0, sizeof(hsrv_ofnh_mpls_param_t));

    /* allocate nh info */
    p_tpoam_bucket->p_nh_info = HSRV_OPENFLOW_GROUP_CALLOC(sizeof(hsrv_of_nh_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tpoam_bucket->p_nh_info);
  
    /* add pw nexthop and tunnel */     
    mpls_param.gport = p_tpoam_bucket->gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE;    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(p_tpoam_bucket->nexthop_mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    mpls_param.u.push_l2vpn.outer_vlan_vid = 0;    
    mpls_param.u.push_l2vpn.pw_label.label_flag = 0;
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    /* modified by yangl for bug 53718 to support mpls tpoam, 2019-10-24 */
    if (p_tpoam_bucket->mpls_tc1_is_set)
    {        
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        mpls_param.u.push_l2vpn.tunnel_label.exp = p_tpoam_bucket->mpls_tc1;
    }
    else   
    {       
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;    
    }
    mpls_param.u.push_l2vpn.tunnel_label.ttl = p_tpoam_bucket->mpls_ttl1;
    mpls_param.u.push_l2vpn.tunnel_label.label = p_tpoam_bucket->label1;
  
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (p_tpoam_bucket->mpls_tc0_is_set)
    {        
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        mpls_param.u.push_l2vpn.pw_label.exp = p_tpoam_bucket->mpls_tc0;
    }
    else
    {        
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    /*yangl end*/
    mpls_param.u.push_l2vpn.pw_label.ttl = p_tpoam_bucket->mpls_ttl0;
    mpls_param.u.push_l2vpn.pw_label.label = p_tpoam_bucket->label0;
  
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, p_tpoam_bucket->p_nh_info));
  
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_get_tunnelid(p_tpoam_bucket->p_nh_info, p_tunnel_id));

    return HSRV_E_NONE;
}

static uint32
_hsrv_openflow_nexthop_lsp_aps_hash_key(hsrv_openflow_lsp_aps_nh_t* pst_nh)
{
    uint32 key = 0;

    key = pst_nh->pw_label;

    return key;
}

static bool
_hsrv_openflow_nexthop_lsp_aps_hash_cmp(void* pva, void* pvb)
{
    hsrv_openflow_lsp_aps_nh_t* pst_nh_a = (hsrv_openflow_lsp_aps_nh_t*)pva;
    hsrv_openflow_lsp_aps_nh_t* pst_nh_b = (hsrv_openflow_lsp_aps_nh_t*)pvb;

    if (pst_nh_a->pw_label == pst_nh_b->pw_label)
    {
        return TRUE;
    }

    return FALSE;
}

int32
hsrv_openflow_nexthop_create_lsp_aps_tbl(ctclib_hash_t**   lsp_aps_nh_hash)
{
    int32 rc = HSRV_E_NONE;
    
    *lsp_aps_nh_hash = ctclib_hash_create(
        _hsrv_openflow_nexthop_lsp_aps_hash_key,
        _hsrv_openflow_nexthop_lsp_aps_hash_cmp);
    HSRV_OPENFLOW_MEM_CHECK(*lsp_aps_nh_hash);

    return rc;
}

/* liwh end */

/* for cubro mpls. */
static int32
_hsrv_openflow_nexthop_alloc_mpls_tunnel(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info, hsrv_openflow_mpls_tunnel_t **pp_mpls_tunnel, bool* p_create)
{
    hsrv_ofnh_mpls_param_t *p_mpls_param = &p_mpls_nh_info->param;
    glb_openflow_mpls_tunnel_param_t tunnel_param;
    hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel = NULL;
    uint32 tunnel_id;
    
    switch(p_mpls_nh_info->param.type)
    {
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN:
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN:
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS:
            {
                memset(&tunnel_param, 0, sizeof(tunnel_param));

                tunnel_param.gport = p_mpls_param->gport;
                if (GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN == p_mpls_nh_info->param.type)
                {
                    memcpy(&tunnel_param.tunnel_label, &(p_mpls_param->u.push_l2vpn.tunnel_label), sizeof(glb_openflow_mpls_nh_label_param_t));
                    memcpy(&tunnel_param.nexthop_mac, &p_mpls_param->u.push_l2vpn.nexthop_mac, sizeof(mac_addr_t));
                    tunnel_param.vid = p_mpls_param->u.push_l2vpn.outer_vlan_vid;
                }
                else if (GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN == p_mpls_nh_info->param.type)
                {
                    memcpy(&tunnel_param.tunnel_label, &(p_mpls_param->u.push_l3vpn.tunnel_label), sizeof(glb_openflow_mpls_nh_label_param_t));
                    memcpy(&tunnel_param.nexthop_mac, &p_mpls_param->u.push_l3vpn.nexthop_mac, sizeof(mac_addr_t));
                    tunnel_param.vid = p_mpls_param->u.push_l3vpn.outer_vlan_vid;
                }
                else
                {
                    memcpy(&tunnel_param.tunnel_label, &(p_mpls_param->u.swap_mpls.tunnel_label), sizeof(glb_openflow_mpls_nh_label_param_t));
                    memcpy(&tunnel_param.nexthop_mac, &p_mpls_param->u.swap_mpls.nexthop_mac, sizeof(mac_addr_t));
                    tunnel_param.vid = p_mpls_param->u.swap_mpls.outer_vlan_vid;
                }

                HSRV_IF_ERROR_RETURN(hsrv_openflow_mpls_tunnel_lookup(&tunnel_param, &p_mpls_tunnel));
                if (NULL == p_mpls_tunnel)
                {
                    tunnel_id = ctclib_opf_alloc_offset(HSRV_OFNH_MPLS_TUNNEL_ID_OPF, 0, 1);
                    if (tunnel_id == CTCLIB_OPF_INVALID_OFFSET)
                    {
                        /* NOTE: It's hard for OVS adapter to detect this error because OVS adapter can not parse
                           every mpls nexthop easilly. 
                           So we just warn this limit has reached and skip to add the nexthop to mcast. */
                        HSRV_LOG_WARN("MPLS tunnel id resource is full, this nexthop will not working.");
                        return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
                    }

                    HSRV_IF_ERROR_RETURN(_hsrv_openflow_mpls_tunnel_save(tunnel_id, &tunnel_param, &p_mpls_tunnel));
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_mpls_tunnel(p_mpls_tunnel));

                    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 */
                    *p_create = TRUE;
                    /* liwh end */
                } 
            }
            break;
        
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        case GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP:
            /* no need to alloc tunnel for pop php */
            break;
        
        default:
            return HSRV_E_INVALID_PARAM;
    }    

    *pp_mpls_tunnel = p_mpls_tunnel; 
        
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_nexthop_ref_mpls_nh_to_tunnel(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info, hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel)
{
    p_mpls_tunnel->ref_cnt++;
    p_mpls_nh_info->p_mpls_tunnel = p_mpls_tunnel;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_alloc_mpls_tunnel_nh(hsrv_ofnh_mpls_param_t *p_nh_param, hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel)
{
    hsrv_of_nh_offset_t   nh_offset;
    hsrv_ofnh_mpls_param_t mpls_param;
    Hsrv2AgtMsg_t hagt_msg;
    HalMsgOpenFlowAddPushL2vpnNhReq_t *req;
    hsrv_ofact_mpls_nh_info_t* p_nh_info = NULL;

    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    
    mpls_param.gport = p_nh_param->gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE;

    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(p_nh_param->u.push_l2vpn.nexthop_mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    mpls_param.u.push_l2vpn.outer_vlan_vid = 0;    
    mpls_param.u.push_l2vpn.pw_label.label_flag = 0;
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    mpls_param.u.push_l2vpn.tunnel_label.label = p_nh_param->u.push_l2vpn.tunnel_label.label;

    hsrv_openflow_nexthop_lookup_mpls_nh(&mpls_param, &p_nh_info);
    if (p_nh_info)
    {
        p_nh_info->ref_cnt ++;
        return HSRV_E_NONE;
    }
    
    /* allocate oam nexthop */
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MPLS;
    hsrv_openflow_nexthop_offset_alloc(&nh_offset);

    /* alloc memery */
    p_nh_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_mpls_nh_info_t));
    HSRV_MEM_CHECK(p_nh_info);
    sal_memset(p_nh_info, 0, sizeof(hsrv_ofact_mpls_nh_info_t));

    sal_memcpy(&p_nh_info->param, &mpls_param, sizeof(hsrv_ofnh_mpls_param_t));
    sal_memcpy(&p_nh_info->nh_offset, &nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_nh_info->ref_cnt = 1;

    req = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
    req->nhid = nh_offset.nhid;
    req->offset = nh_offset.offset;
    req->tunnelId = p_mpls_tunnel->tunnel_id;
    req->innerSvlanTagOp = 0;
    
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddPushL2vpnNh;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    /* reserve nexthop */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mpls_nh(p_nh_info));

    p_mpls_tunnel->p_mpls_nh_info = (void*)p_nh_info;
    p_mpls_tunnel->nh_id = nh_offset.nhid;
    p_mpls_tunnel->offset = nh_offset.offset;

    /* modified by liwh for bug 47318, 2018-05-22 */
    p_nh_info->p_mpls_tunnel = p_mpls_tunnel;
    /* liwh end */

    return HSRV_E_NONE;
}

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-03 */
int32
hsrv_openflow_nexthop_get_tunnelid(hsrv_of_nh_info_t* p_mem_info, uint32* p_tunnel_id)
{
    hsrv_ofact_mpls_nh_info_t*       p_nh_info = NULL;
     
    if (p_mem_info)
    {
        p_nh_info = (hsrv_ofact_mpls_nh_info_t*)p_mem_info->share_info;
        if (p_nh_info && p_nh_info->p_mpls_tunnel)
        {
            *p_tunnel_id = p_nh_info->p_mpls_tunnel->tunnel_id;
        }
    }

    return HSRV_E_NONE;
}
/* liwh end */

int32
hsrv_openflow_nexthop_alloc_mpls_nh(hsrv_ofnh_mpls_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info)
{
    hsrv_ofact_mpls_nh_info_t* p_nh_info = NULL;
    hsrv_of_nh_offset_t* p_nh_offset = NULL;
    hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel = NULL;
    int32 ret;
    bool  create = FALSE;
    
    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(p_mem_info);

    /* mpls nh exist */
    hsrv_openflow_nexthop_lookup_mpls_nh(p_nh_param, &p_nh_info);
    if (p_nh_info)
    {
        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
            for tunnel nhid, tunnel ref_cnt should be added */
        if (p_nh_info->p_mpls_tunnel 
            && (p_nh_info->p_mpls_tunnel->nh_id != p_nh_info->nh_offset.nhid))
        {
            p_nh_info->ref_cnt ++;
        }
        /* liwh end */
        
        p_mem_info->share_info = p_nh_info;
        sal_memcpy(&p_mem_info->nh_offset, &p_nh_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        p_mem_info->gport = p_nh_param->gport;

        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
            for tunnel nhid, tunnel ref_cnt should be added */
        if (p_nh_info->p_mpls_tunnel 
            && (p_nh_info->p_mpls_tunnel->nh_id == p_nh_info->nh_offset.nhid))
        {
            p_nh_info->p_mpls_tunnel->ref_cnt++;
            return HSRV_E_NONE;
        }
        /* liwh end */
        
        return HSRV_E_NONE;
    }
    
    /* alloc mpls nh */
    p_nh_offset = &p_mem_info->nh_offset;
    p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_MPLS;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_nh_offset));

    /* alloc memery */
    p_nh_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_mpls_nh_info_t));
    HSRV_MEM_CHECK(p_nh_info);
    sal_memset(p_nh_info, 0, sizeof(hsrv_ofact_mpls_nh_info_t));

    sal_memcpy(&p_nh_info->param, p_nh_param, sizeof(hsrv_ofnh_mpls_param_t));
    sal_memcpy(&p_nh_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_nh_info->ref_cnt = 1;
    p_mem_info->share_info = p_nh_info;
    p_mem_info->gport = p_nh_param->gport;

    ret = _hsrv_openflow_nexthop_alloc_mpls_tunnel(p_nh_info, &p_mpls_tunnel, &create);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL)
    {
        p_mem_info->gport = 0;
        p_mem_info->share_info = NULL;
        HSRV_OPENFLOW_NH_FREE(p_nh_info);
        hsrv_openflow_nexthop_offset_release(p_nh_offset);
        /* Return without error log to let caller to handle. */
        return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
    }
    HSRV_IF_ERROR_RETURN(ret);
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_mpls_nh(p_nh_info, p_mpls_tunnel));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mpls_nh(p_nh_info));
    if (GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP != p_nh_param->type)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_nexthop_ref_mpls_nh_to_tunnel(p_nh_info, p_mpls_tunnel));
    }

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
        for tunnel, the nexthop without adding lable will be added */
    if (create)
    {
        if (p_nh_param->u.push_l2vpn.pw_label.label_flag)
        {
            hsrv_openflow_nexthop_alloc_mpls_tunnel_nh(p_nh_param, p_mpls_tunnel);
        }
        else if (p_nh_param->u.push_l2vpn.tunnel_label.label_flag)
        {
            p_mpls_tunnel->p_mpls_nh_info = (void*)p_nh_info;
            p_mpls_tunnel->nh_id = p_nh_offset->nhid;
            p_mpls_tunnel->offset = p_nh_offset->offset;
        }
    }
    /* liwh end */

    return HSRV_E_NONE;
}

/* modified by yaom for openflow MPLS L2VPN 20160214 */
int32
hsrv_openflow_nexthop_alloc_ipuc_nh(hsrv_ofnh_ipuc_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info)
{
    hsrv_ofact_ipuc_nh_info_t* p_nh_info = NULL;
    hsrv_of_nh_offset_t* p_nh_offset = NULL;
    
    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(p_mem_info);

    /* ipuc nh exist */
    hsrv_openflow_nexthop_lookup_ipuc_nh(p_nh_param, &p_nh_info);
    if (p_nh_info)
    {
        p_nh_info->ref_cnt ++;
        p_mem_info->share_info = p_nh_info;
        sal_memcpy(&p_mem_info->nh_offset, &p_nh_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        p_mem_info->gport = p_nh_param->gport;
        
        return HSRV_E_NONE;
    }
    
    /* alloc ipuc nh */
    p_nh_offset = &p_mem_info->nh_offset;
    p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_IPUC;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_nh_offset));

    /* alloc memery */
    p_nh_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_ipuc_nh_info_t));
    HSRV_MEM_CHECK(p_nh_info);
    sal_memset(p_nh_info, 0, sizeof(hsrv_ofact_ipuc_nh_info_t));

    sal_memcpy(&p_nh_info->param, p_nh_param, sizeof(hsrv_ofnh_ipuc_param_t));
    sal_memcpy(&p_nh_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_nh_info->ref_cnt = 1;
    p_mem_info->share_info = p_nh_info;
    p_mem_info->gport = p_nh_param->gport;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_ipuc_nh(p_nh_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ipuc_nh(p_nh_info));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_alloc_xlate_nh(hsrv_ofnh_xlate_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info)
{
    hsrv_ofact_xlate_nh_info_t* p_nh_info = NULL;
    hsrv_of_nh_offset_t* p_nh_offset = NULL;
    
    HSRV_PTR_CHECK(p_nh_param);
    HSRV_PTR_CHECK(p_mem_info);

    /* xlate nh exist */
    hsrv_openflow_nexthop_lookup_xlate_nh(p_nh_param, &p_nh_info);
    if (p_nh_info)
    {
        p_nh_info->ref_cnt ++;
        p_mem_info->share_info = p_nh_info;
        sal_memcpy(&p_mem_info->nh_offset, &p_nh_info->nh_offset, sizeof(hsrv_of_nh_offset_t));
        p_mem_info->gport = p_nh_param->gport;
        
        return HSRV_E_NONE;
    }
    
    /* alloc xlate nh */
    p_nh_offset = &p_mem_info->nh_offset;
    p_nh_offset->nh_type = HSRV_OF_NH_TYPE_NH_XLATE;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(p_nh_offset));

    /* alloc memery */
    p_nh_info = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_ofact_xlate_nh_info_t));
    HSRV_MEM_CHECK(p_nh_info);
    sal_memset(p_nh_info, 0, sizeof(hsrv_ofact_xlate_nh_info_t));

    sal_memcpy(&p_nh_info->param, p_nh_param, sizeof(hsrv_ofnh_xlate_param_t));
    sal_memcpy(&p_nh_info->nh_offset, p_nh_offset, sizeof(hsrv_of_nh_offset_t));
    p_nh_info->ref_cnt = 1;
    p_mem_info->share_info = p_nh_info;
    p_mem_info->gport = p_nh_param->gport;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_xlate_nh(p_nh_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_xlate_nh(p_nh_info));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_mcast_group(hsrv_of_nh_offset_t* p_group_nh)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_del_mcast_group(p_group_nh));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_release(p_group_nh));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_ecmp_group(hsrv_of_nh_offset_t* p_group_nh)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_del_ecmp_group(p_group_nh));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_release(p_group_nh));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_flex_nh(hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_flex_nh_info_t* p_flex_info;
    
    p_flex_info = (hsrv_ofact_flex_nh_info_t*)p_nh_info->share_info;
    if (p_flex_info)
    {
        if (0 == p_flex_info->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow flex nexthop reference count error");
            return HSRV_E_NONE;
        }
        p_flex_info->ref_cnt --;
        if (0 == p_flex_info->ref_cnt)
        {
            if(HSRV_OF_NH_TYPE_NH_FLEX_CPU == p_nh_info->nh_offset.nh_type)
            {
                hsrv_openflow_nexthop_del_flex_nh(p_flex_info);
                HSRV_OPENFLOW_NH_FREE(p_flex_info);
            }
            else
            {
                hsrv_openflow_nexthop_remove_flex_nh(p_flex_info);
                hsrv_openflow_nexthop_del_flex_nh(p_flex_info);
                hsrv_openflow_nexthop_offset_release(&p_flex_info->nh_offset);
                HSRV_OPENFLOW_NH_FREE(p_flex_info);
            }
        }
    }
    p_nh_info->share_info = NULL;
    
    return HSRV_E_NONE;
}

/* Added by weizj for tunnel */
int32
hsrv_openflow_nexthop_release_ip_tunnel_nh(hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_ip_tunnel_nh_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_nh_info);

    p_tnl_info = (hsrv_ofact_ip_tunnel_nh_info_t*)p_nh_info->share_info;
    if (p_tnl_info)
    {
        if (0 == p_tnl_info->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow tunnel nexthop reference count error");
            return HSRV_E_NONE;
        }

        /* Added by weizj for tunnel bug 34170 */
        if (p_tnl_info->param.tunnel_type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            p_tnl_info->param.tunnel_type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            hsrv_openflow_tunnel_release_fid(p_tnl_info->param.inner.tunnel_key);
        }

        p_tnl_info->ref_cnt --;
        if (0 == p_tnl_info->ref_cnt)
        {
            hsrv_openflow_nexthop_remove_ip_tunnel_nh(p_tnl_info);
            hsrv_openflow_nexthop_del_ip_tunnel_nh(p_tnl_info);
            hsrv_openflow_nexthop_offset_release(&p_tnl_info->nh_offset);
            /* Deleted by weizj for tunnel bug 34170 */
            HSRV_OPENFLOW_NH_FREE(p_tnl_info);
        }
    }
    
    return HSRV_E_NONE;
}
/* End by weizj for tunnel */

/* for cubro mpls. */
int32
hsrv_openflow_nexthop_release_mpls_tunnel(hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info)
{
    hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel = NULL;
    hsrv_ofact_mpls_nh_info_t*     p_mpls_tunnel_nh_info = NULL;
    
    if (p_mpls_nh_info->p_mpls_tunnel == NULL)
    {
        /* No tunnel allocated, no need to free. */
        return HSRV_E_NONE;
    }
       
    p_mpls_tunnel =p_mpls_nh_info->p_mpls_tunnel;
    p_mpls_tunnel->ref_cnt--;
    
    if (p_mpls_tunnel->ref_cnt == 0)
    {
        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
            if tunnel is deleted, tunnel nexthop should be deleted */
        if (p_mpls_tunnel->p_mpls_nh_info)
        {
            p_mpls_tunnel_nh_info = (hsrv_ofact_mpls_nh_info_t*)p_mpls_tunnel->p_mpls_nh_info;
            hsrv_openflow_nexthop_remove_mpls_nh(p_mpls_tunnel_nh_info);
            hsrv_openflow_nexthop_del_mpls_nh(p_mpls_tunnel_nh_info);
            hsrv_openflow_nexthop_offset_release(&p_mpls_tunnel_nh_info->nh_offset);
            HSRV_OPENFLOW_NH_FREE(p_mpls_tunnel_nh_info);
            p_mpls_tunnel->p_mpls_nh_info = NULL;
            p_mpls_tunnel->nh_id = 0;
        }
        /* liwh end */
            
        ctclib_opf_free_offset(HSRV_OFNH_MPLS_TUNNEL_ID_OPF, 0, 1, p_mpls_tunnel->tunnel_id);            
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mpls_tunnel(p_mpls_tunnel->tunnel_id));
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_mpls_tunnel_release(p_mpls_tunnel));
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_mpls_nh(hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_mpls_nh_info_t* p_mpls_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_nh_info);

    p_mpls_nh_info = (hsrv_ofact_mpls_nh_info_t*)p_nh_info->share_info;
    if (p_mpls_nh_info)
    {
        if (0 == p_mpls_nh_info->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow mpls nexthop reference count error");
            return HSRV_E_NONE;
        }

        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
            if tunnel won't be deleted, the tunnel nexthop shouldn't be deleted */
        if (p_mpls_nh_info->p_mpls_tunnel 
            && (p_mpls_nh_info->p_mpls_tunnel->nh_id == p_mpls_nh_info->nh_offset.nhid)
            &&(1 != p_mpls_nh_info->p_mpls_tunnel->ref_cnt) )
        {
            p_mpls_nh_info->p_mpls_tunnel->ref_cnt--;
            return HSRV_E_NONE;
        }
        /* liwh end */

        p_mpls_nh_info->ref_cnt --;
        if (0 == p_mpls_nh_info->ref_cnt)
        {
            hsrv_openflow_nexthop_remove_mpls_nh(p_mpls_nh_info);

            /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-23 
                if the tunnel nexthop is deleted, it shouldn't be deleted again when delete tunnel */
            if (p_mpls_nh_info->p_mpls_tunnel 
                && (p_mpls_nh_info->p_mpls_tunnel->nh_id == p_mpls_nh_info->nh_offset.nhid))
            {
                p_mpls_nh_info->p_mpls_tunnel->nh_id = 0;
                p_mpls_nh_info->p_mpls_tunnel->p_mpls_nh_info = NULL;
            }
            /* liwh end */
             
            /* Must release tunnel after mpls nh release. */
            hsrv_openflow_nexthop_release_mpls_tunnel(p_mpls_nh_info);
            hsrv_openflow_nexthop_del_mpls_nh(p_mpls_nh_info);
            hsrv_openflow_nexthop_offset_release(&p_mpls_nh_info->nh_offset);
            /* Deleted by weizj for tunnel bug 34170 */
            HSRV_OPENFLOW_NH_FREE(p_mpls_nh_info);
        }
    }

    return HSRV_E_NONE;
}

/* modified by yaom for openflow MPLS L2VPN 20160214 */
int32
hsrv_openflow_nexthop_release_ipuc_nh(hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_ipuc_nh_info_t* p_ipuc_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_nh_info);

    p_ipuc_nh_info = (hsrv_ofact_ipuc_nh_info_t*)p_nh_info->share_info;
    if (p_ipuc_nh_info)
    {
        if (0 == p_ipuc_nh_info->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow ipuc nexthop reference count error");
            return HSRV_E_NONE;
        }

        p_ipuc_nh_info->ref_cnt --;
        if (0 == p_ipuc_nh_info->ref_cnt)
        {
            hsrv_openflow_nexthop_remove_ipuc_nh(p_ipuc_nh_info);
            hsrv_openflow_nexthop_del_ipuc_nh(p_ipuc_nh_info);
            hsrv_openflow_nexthop_offset_release(&p_ipuc_nh_info->nh_offset);
            HSRV_OPENFLOW_NH_FREE(p_ipuc_nh_info);
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_xlate_nh(hsrv_of_nh_info_t* p_nh_info)
{
    hsrv_ofact_xlate_nh_info_t* p_xlate_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_nh_info);

    p_xlate_nh_info = (hsrv_ofact_xlate_nh_info_t*)p_nh_info->share_info;
    if (p_xlate_nh_info)
    {
        if (0 == p_xlate_nh_info->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow xlate nexthop reference count error");
            return HSRV_E_NONE;
        }

        p_xlate_nh_info->ref_cnt --;
        if (0 == p_xlate_nh_info->ref_cnt)
        {
            hsrv_openflow_nexthop_remove_xlate_nh(p_xlate_nh_info);
            hsrv_openflow_nexthop_del_xlate_nh(p_xlate_nh_info);
            hsrv_openflow_nexthop_offset_release(&p_xlate_nh_info->nh_offset);
            HSRV_OPENFLOW_NH_FREE(p_xlate_nh_info);
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_nh_info(hsrv_of_nh_info_t *p_nh_info)
{
    switch(p_nh_info->nh_offset.nh_type)
    {
    case HSRV_OF_NH_TYPE_NH_FLEX:
    case HSRV_OF_NH_TYPE_NH_FLEX_CPU:    
        hsrv_openflow_nexthop_release_flex_nh(p_nh_info);
        break;
    /* Added by weizj for tunnel */
    case HSRV_OF_NH_TYPE_NH_IP_TUNNEL:
    case HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID:
        hsrv_openflow_nexthop_release_ip_tunnel_nh(p_nh_info);
        break;
    /* End by weizj for tunnel */

    /* for cubro mpls. */
    case HSRV_OF_NH_TYPE_NH_MPLS:
        hsrv_openflow_nexthop_release_mpls_nh(p_nh_info);
        break;

    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    case HSRV_OF_NH_TYPE_NH_IPUC:
        hsrv_openflow_nexthop_release_ipuc_nh(p_nh_info);
        break;
        
    case HSRV_OF_NH_TYPE_NH_XLATE:
        hsrv_openflow_nexthop_release_xlate_nh(p_nh_info);
        break;
        
    default:
        break;
    }
    p_nh_info->nh_offset.nhid    = 0;
    p_nh_info->nh_offset.offset  = 0;
    p_nh_info->nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_del_mcast_member(hsrv_openflow_nexthop_info_t* p_nh_info)
{
    uint32 member_idx;

    if (!p_nh_info->use_mcast)
    {
        return HSRV_E_NONE;
    }

    for (member_idx = 0; member_idx < p_nh_info->member_count; member_idx ++)
    {
        hsrv_of_nh_info_t *member_nh;
        hsrv_ofnh_mcast_member_param_t param;

        member_nh = &p_nh_info->member_nh[member_idx];
        param.group_nhid = p_nh_info->main_nh.nh_offset.nhid;
        param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
        param.mem_gport = member_nh->gport;
        param.mem_nhid = member_nh->nh_offset.nhid;
        param.port_check_discard = member_nh->nh_offset.port_check_discard;
        param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;
        hsrv_openflow_nexthop_remove_mcast_member(&param);
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_release_nexthop_info(hsrv_openflow_nexthop_info_t* p_nh_info)
{
    uint32 member_idx;
    if(p_nh_info->use_dynamic_tunnel)
    {
        if(p_nh_info->use_ecmp)
        {
            for(member_idx = 0; member_idx < p_nh_info->member_count; member_idx ++)
            {
                if(HSRV_OF_NH_TYPE_MAX != p_nh_info->member_nh[member_idx].nh_offset.nh_type)
                {
                    hsrv_openflow_nexthop_release_nh_info(&p_nh_info->member_nh[member_idx]);
                    
                }
            }
            hsrv_openflow_nexthop_release_ecmp_group(&p_nh_info->main_nh.nh_offset);
        }
        else if(p_nh_info->use_mcast)
        {
            hsrv_openflow_nexthop_del_mcast_member(p_nh_info);
            hsrv_openflow_nexthop_release_mcast_group(&p_nh_info->main_nh.nh_offset);
            for (member_idx = 0; member_idx < p_nh_info->member_count; member_idx ++)
            {
                if(HSRV_OF_NH_TYPE_MAX != p_nh_info->member_nh[member_idx].nh_offset.nh_type)
                {
                    hsrv_openflow_nexthop_release_nh_info(&p_nh_info->member_nh[member_idx]);
                }
            }
        }
        else
        {
            if(HSRV_OF_NH_TYPE_MAX != p_nh_info->main_nh.nh_offset.nh_type)
            {
                hsrv_openflow_nexthop_release_nh_info(&p_nh_info->main_nh);
            }
        }
    }
    else
    {
        if (!p_nh_info->use_mcast)
        {
            hsrv_openflow_nexthop_release_nh_info(&p_nh_info->main_nh);
            sal_memset(p_nh_info, 0x0, sizeof(hsrv_openflow_nexthop_info_t));
            return HSRV_E_NONE;
        }

        hsrv_openflow_nexthop_del_mcast_member(p_nh_info);
        hsrv_openflow_nexthop_release_mcast_group(&p_nh_info->main_nh.nh_offset);
        
        for (member_idx = 0; member_idx < p_nh_info->member_count; member_idx ++)
        {
            hsrv_openflow_nexthop_release_nh_info(&p_nh_info->member_nh[member_idx]);
        }
    }
    sal_memset(p_nh_info, 0x0, sizeof(hsrv_openflow_nexthop_info_t));
    return HSRV_E_NONE;
}

#define ____NEXTHOP_COUNT_____

int32
hsrv_openflow_nexthop_get_hash_count(void)
{
    uint32 count = 0;

    if (HSRV_OFNH_FLEX_NH_HASH)
    {
        count += HSRV_OFNH_FLEX_NH_HASH->count;
    }
    if (HSRV_OFNH_MPLS_NH_HASH)
    {
        count += HSRV_OFNH_MPLS_NH_HASH->count;
    }
    if (HSRV_OFNH_IPUC_NH_HASH)
    {
        count += HSRV_OFNH_IPUC_NH_HASH->count;
    }
    if (HSRV_OFNH_IP_TUNNEL_NH_HASH)
    {
        count += HSRV_OFNH_IP_TUNNEL_NH_HASH->count;
    }
    if (HSRV_OFNH_XLATE_NH_HASH)
    {
        count += HSRV_OFNH_XLATE_NH_HASH->count;
    }

    return count;
}

uint32
hsrv_openflow_nexthop_get_hash_max(void)
{
    return g_pst_openflow_nexthop_master->nexthop_max_num;
}

uint32
hsrv_openflow_nexthop_get_rsvd_hash_count(void)
{
    return g_pst_openflow_nexthop_master->reserved_nh_count;
}

#if 0
int32
hsrv_openflow_nexthop_check_nexthop_hash_count(void)
{
    /* Reserved nexthop is added into hash db already */
    if (hsrv_openflow_nexthop_get_hash_count() >= 
        hsrv_openflow_nexthop_get_hash_max() +
        hsrv_openflow_nexthop_get_rsvd_hash_count())
    {
        HSRV_OPENFLOW_LOG_ERR("Only %d different combinations of fields-modifing actions and output are supported",
            hsrv_openflow_nexthop_get_hash_max());

        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }

    return HSRV_E_NONE;
}
#endif

#define _____RESV_MCAST_ALL_____
#if 0 /* Deleted by weizj for tunnel bug 33724 */
int32
hsrv_openflow_nexthop_create_output_all(void)
{
    hsrv_ofnh_mcast_param_t param;
    hsrv_of_nh_offset_t* p_nh_offset = &g_pst_openflow_nexthop_master->mcast_all_nh;

    sal_memset(&param, 0, sizeof(hsrv_ofnh_mcast_param_t));
    param.stats_en = FALSE;
    param.is_resvd = TRUE;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mcast_group(&param, p_nh_offset));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_get_output_all_nhid(hsrv_of_nh_offset_t* p_nh_offset)
{
    sal_memcpy(p_nh_offset, &g_pst_openflow_nexthop_master->mcast_all_nh, sizeof(hsrv_of_nh_offset_t));
    
    return HSRV_E_NONE;
}
#endif

#define _____DEBUG_____

static void
hsrv_openflow_nexthop_get_opf_usage(hsrv_openflow_nexthop_opf_type_t type, uint32* p_used, uint32* p_total)
{
    uint32 next_offset_index = 0;
    ctclib_opf_t* p_opf;
    uint32 size = 0;
    uint32 offset = 0;
    uint32 left = 0;

    p_opf = g_pst_openflow_nexthop_master->pst_opf[type];
    while (ctclib_opf_get_next_entry_info(p_opf, 0, &next_offset_index, &size, &offset))
    {
        left += size;
    }
    
    *p_total = p_opf->max_size_a[0];
    *p_used = *p_total - left;
}

static void
hsrv_openflow_nexthop_opf_print_alloc_info(FILE *fp, hsrv_openflow_nexthop_opf_type_t type)
{
    uint32 next_offset_index = 0;
    ctclib_opf_t* p_opf;
    uint32 size = 0;
    uint32 offset = 0;
    
    p_opf = g_pst_openflow_nexthop_master->pst_opf[type];
    if (NULL == p_opf)
    {
        return;
    }

    sal_fprintf(fp, "[%-12s][%-8s]\n", "start offset", "max size");

    sal_fprintf(fp, "[%-12d][%-8d]\n", p_opf->start_offset_a[0], p_opf->max_size_a[0]);

    sal_fprintf(fp, "%-20s\n", "----------------------");

    sal_fprintf(fp, "[%-12s][%-10s][%-12s]\n", "index", "start offset", "available");

    while (ctclib_opf_get_next_entry_info(p_opf, 0, &next_offset_index, &size, &offset))
    {
        sal_fprintf(fp, "[%-12d][%-10d][%-12d]\n", (next_offset_index - 1), offset, size);
    }
}

int32
hsrv_openflow_show_nexthop_opf(FILE *fp, char* type)
{
    if (sal_strncmp (type, "nhid", 4) == 0)
    {
        sal_fprintf(fp, "Nexthop opf type nhid: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_NHID);
    }
    else if (sal_strncmp (type, "nh-offset", 9) == 0)
    {
        sal_fprintf(fp, "Nexthop opf type nh-offset: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET);
    }
    else if (sal_strncmp (type, "mc-offset", 9) == 0)
    {
        sal_fprintf(fp, "Nexthop opf type mc-offset: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET);
    }
    else if (sal_strncmp (type, "mpls-tunnelid", 13) == 0)
    {
        sal_fprintf(fp, "Nexthop opf type mpls-tunnelid: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_MPLS_TUNNELID);
    }
    else if (sal_strncmp (type, "all", 3) == 0)
    {
        sal_fprintf(fp, "Nexthop opf type nhid: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_NHID);

        sal_fprintf(fp, "Nexthop opf type nh-offset: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET);

        sal_fprintf(fp, "Nexthop opf type mc-offset: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET);

        sal_fprintf(fp, "Nexthop opf type mpls-tunnelid: \n");
        hsrv_openflow_nexthop_opf_print_alloc_info(fp, HSRV_OPENFLOW_NEXTHOP_OPF_MPLS_TUNNELID);
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_nexthop_brief(FILE *fp)
{
    hsrv_openflow_nexthop_master_t* p_master = g_pst_openflow_nexthop_master;
    int32 type;
    uint32 used = 0;
    uint32 total = 0;
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW NEXTHOP------------------\n");
    sal_fprintf(fp, "Mcast all nhid          : %d\n", p_master->mcast_all_nh.nhid);
    sal_fprintf(fp, "Hash count         total: (%5d / %5d)\n", 
        hsrv_openflow_nexthop_get_hash_count(),
        p_master->nexthop_max_num);
    sal_fprintf(fp, "Hash count      reserved: (%5d / -----)\n", 
        p_master->reserved_nh_count);
    if (HSRV_OFNH_FLEX_NH_HASH)
    {
        sal_fprintf(fp, "Hash count          flex: (%5d / -----)\n",
                HSRV_OFNH_FLEX_NH_HASH->count);
    }

    for (type = 0; type < HSRV_OF_NH_TYPE_MAX; type ++)
    {
        if (p_master->nh_type_count[type])
        {
            sal_fprintf(fp, "Nexhop      %12s: (%5d / -----)\n", 
                HSRV_OF_NH_TYPE_STR(type),
                p_master->nh_type_count[type]);
        }
    }

    for (type = 0 ; type < HSRV_OPENFLOW_NEXTHOP_OPF_MAX; type ++)
    {
        if (p_master->pst_opf[type])
        {
            hsrv_openflow_nexthop_get_opf_usage(type, &used, &total);
            sal_fprintf(fp, "Opf %20s: (%5d / %5d) (resvd %d)\n", 
                HSRV_OPENFLOW_NEXTHOP_OPF_TYPE_STR(type),
                used,
                total,
                p_master->resvd_opf[type]);
        }
    }
    
    sal_fprintf(fp, "----------------------------------------------------\n");
    
    return HSRV_E_NONE;
}

static void 
hsrv_openflow_show_flex_nexthop_combo__(FILE* fp, hsrv_openflow_action_combo_t *p_combo, int prefix)
{
    int32   i = 0;
    uint8*  p_ip;
    uint32* p_ipv6;
    uint32  action_idx = 0;
    
    /* Compile failed, what's wrong? */
    CTC_BUILD_ASSERT(GLB_OFPACT_FLD_FLAG_NUM == 2);
    if (!p_combo->flag[0] &&
        !p_combo->flag[1])
    {
        sal_fprintf(fp, "empty");
    }

    if (p_combo->is_resvd_nh)
    {
        sal_fprintf(fp, "rsvd,");
    }
    if (p_combo->statsid)
    {
        sal_fprintf(fp, "stats_id:%u,", p_combo->statsid);
    }

    for (i = 0; i < GLB_OPENFLOW_ACTION_FIELD_MAX; i++)
    {
        if (!GLB_IS_BIT_SET(p_combo->flag[i/32], i%32))
        {
            continue;
        }
        
        if (action_idx)
        {
            sal_fprintf(fp, ",");
        }
        if ((action_idx + 1) % 5 == 0)
        {
            sal_fprintf(fp, "\n");
            if (1 == prefix)
            {
                sal_fprintf(fp, "%5s %7s %10s \n", "", "", "");
            }
            else if (2 == prefix)
            {
                sal_fprintf(fp, "          : ");
            }
        }
        
        switch(i)
        {
        case GLB_OFPACT_FLD(OUTPUT):
            sal_fprintf(fp, "gport:%u", p_combo->output_gport);
            break;
            
        case GLB_OFPACT_FLD(MACSA):
            sal_fprintf(fp, "macsa:%02x:%02x:%02x:%02x:%02x:%02x",
                p_combo->mac_sa[0], p_combo->mac_sa[1], p_combo->mac_sa[2],
                p_combo->mac_sa[3], p_combo->mac_sa[4], p_combo->mac_sa[5]);
            break;
            
        case GLB_OFPACT_FLD(MACDA):
            sal_fprintf(fp, "set_macda:%02x:%02x:%02x:%02x:%02x:%02x",
                p_combo->mac_da[0], p_combo->mac_da[1], p_combo->mac_da[2],
                p_combo->mac_da[3], p_combo->mac_da[4], p_combo->mac_da[5]);
            break;
            
        case GLB_OFPACT_FLD(SVLAN_VID):
            sal_fprintf(fp, "svid:%u", p_combo->vlan_id);
            break;
            
        case GLB_OFPACT_FLD(SVLAN_COS):
            sal_fprintf(fp, "scos:%u", p_combo->vlan_pcp);
            break;
            
        case GLB_OFPACT_FLD(CVLAN_VID):
            sal_fprintf(fp, "cvid:%u", p_combo->cvlan_id);
            break;
            
        case GLB_OFPACT_FLD(CVLAN_COS):
            sal_fprintf(fp, "ccos:%u", p_combo->cvlan_pcp);
            break;
            
        case GLB_OFPACT_FLD(STRIP_SVLAN):
            sal_fprintf(fp, "strip_svlan");
            break;
            
        case GLB_OFPACT_FLD(STRIP_CVLAN):
            sal_fprintf(fp, "strip_cvlan");
            break;
            
        case GLB_OFPACT_FLD(PUSH_SVLAN):
            sal_fprintf(fp, "push_svlan:%u", p_combo->stag_tpid);
            break;
            
        case GLB_OFPACT_FLD(PUSH_CVLAN):
            sal_fprintf(fp, "push_cvlan:%u", p_combo->ctag_tpid);
            break;
            
        case GLB_OFPACT_FLD(ETH_TYPE):
            sal_fprintf(fp, "eth_type:%u", p_combo->eth_type);
            break;
            
        case GLB_OFPACT_FLD(IPV4_):
            sal_fprintf(fp, "ipv4");
            break;
            
        case GLB_OFPACT_FLD(IPV6_):
            sal_fprintf(fp, "ipv6");
            break;
            
        case GLB_OFPACT_FLD(ARP_):
            sal_fprintf(fp, "arp");
            break;
            
        case GLB_OFPACT_FLD(MPLS_):
            sal_fprintf(fp, "mpls");
            break;
            
        case GLB_OFPACT_FLD(TCP_):
            sal_fprintf(fp, "tcp");
            break;
            
        case GLB_OFPACT_FLD(UDP_):
            sal_fprintf(fp, "udp");
            break;
            
        case GLB_OFPACT_FLD(SCTP_):
            sal_fprintf(fp, "sctp");
            break;
            
        case GLB_OFPACT_FLD(IPSA):
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV4_))
            {
                p_ip = (uint8*)&p_combo->l3.ipv4.ipsa;
                sal_fprintf(fp, "nw_src:%u.%u.%u.%u", 
                    p_ip[3], p_ip[2], p_ip[1], p_ip[0]);
            }
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV6_))
            {
                p_ipv6 = p_combo->l3.ipv6.ipsa;
                sal_fprintf(fp, "nw_src:%08X:%08X:%08X:%08X", 
                    p_ipv6[0], p_ipv6[1], p_ipv6[2], p_ipv6[3]);
            }
            break;
            
        case GLB_OFPACT_FLD(IPDA):
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV4_))
            {
                p_ip = (uint8*)&p_combo->l3.ipv4.ipda;
                sal_fprintf(fp, "nw_src:%u.%u.%u.%u", 
                    p_ip[3], p_ip[2], p_ip[1], p_ip[0]);
            }
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV6_))
            {
                p_ipv6 = p_combo->l3.ipv6.ipda;
                sal_fprintf(fp, "nw_src:%08X:%08X:%08X:%08X", 
                    p_ipv6[0], p_ipv6[1], p_ipv6[2], p_ipv6[3]);
            }
            break;
            
        case GLB_OFPACT_FLD(SPORT):
            sal_fprintf(fp, "src_port:%u", p_combo->l4.l4port.src_port);
            break;
            
        case GLB_OFPACT_FLD(DPORT):
            sal_fprintf(fp, "dst_port:%u", p_combo->l4.l4port.dst_port);
            break;
            
        case GLB_OFPACT_FLD(IP_TTL_):
            sal_fprintf(fp, "ttl:%u", p_combo->l3.ip.ttl);
            break;
            
        case GLB_OFPACT_FLD(IP_DSCP):
            sal_fprintf(fp, "dscp:%u", p_combo->l3.ip.dscp);
            break;
            
        case GLB_OFPACT_FLD(IP_ECN):
            sal_fprintf(fp, "ecn:%u", p_combo->l3.ip.ecn);
            break;
            
        case GLB_OFPACT_FLD(IP_PROTO):
            sal_fprintf(fp, "ip_proto:%u", p_combo->l3.ip.proto);
            break;
            
        case GLB_OFPACT_FLD(DEC_IP_TTL):
            sal_fprintf(fp, "dec_ip_ttl");
            break;
            
        case GLB_OFPACT_FLD(DEC_TTL):
            sal_fprintf(fp, "dec_ttl");
            break;
            
        case GLB_OFPACT_FLD(IPV6_LBL):
            sal_fprintf(fp, "ipv6_label:%u", p_combo->l3.ipv6.label);
            break;
            
        case GLB_OFPACT_FLD(ARP_OP):
            sal_fprintf(fp, "arp_op:%u", p_combo->l3.arp.arp_op);
            break;
            
        case GLB_OFPACT_FLD(ARP_SPA):
            sal_fprintf(fp, "arp_spa:%u", p_combo->l3.arp.arp_spa);
            break;
            
        case GLB_OFPACT_FLD(ARP_TPA):
            sal_fprintf(fp, "arp_tpa:%u", p_combo->l3.arp.arp_tpa);
            break;
            
        case GLB_OFPACT_FLD(ARP_SHA):
            sal_fprintf(fp, "arp_sha:%02x:%02x:%02x:%02x:%02x:%02x",
                p_combo->l3.arp.arp_sha[0], p_combo->l3.arp.arp_sha[1], 
                p_combo->l3.arp.arp_sha[2], p_combo->l3.arp.arp_sha[3], 
                p_combo->l3.arp.arp_sha[4], p_combo->l3.arp.arp_sha[5]);
            break;
            
        case GLB_OFPACT_FLD(ARP_THA):
            sal_fprintf(fp, "arp_tha:%02x:%02x:%02x:%02x:%02x:%02x",
                p_combo->l3.arp.arp_tha[0], p_combo->l3.arp.arp_tha[1], 
                p_combo->l3.arp.arp_tha[2], p_combo->l3.arp.arp_tha[3], 
                p_combo->l3.arp.arp_tha[4], p_combo->l3.arp.arp_tha[5]);
            break;
            
        case GLB_OFPACT_FLD(ICMP_TYPE):
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV4_))
            {
                sal_fprintf(fp, "icmp_type:%u", p_combo->l4.icmp.type);
            }
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV6_))
            {
                sal_fprintf(fp, "icmpv6_type:%u", p_combo->l4.icmp.type);
            }
            break;
            
        case GLB_OFPACT_FLD(ICMP_CODE):
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV4_))
            {
                sal_fprintf(fp, "icmp_code:%u", p_combo->l4.icmp.code);
            }
            if (GLB_OFPACT_FLD_ISSET(p_combo->flag, IPV6_))
            {
                sal_fprintf(fp, "icmpv6_code:%u", p_combo->l4.icmp.code);
            }
            break;
            
        default:
#define DEFINE_OFACT_FLD(FLD, STR) \
    (i == GLB_OPENFLOW_ACTION_FIELD_##FLD) ? STR :
            sal_fprintf(fp, "%s", GLB_DEFINE_OFACT_FLD "UNKNOWN");
#undef DEFINE_OFACT_FLD
            break;
        }
        action_idx ++;
    }
}

static void 
hsrv_openflow_show_flex_nexthop_hash__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_ofact_flex_nh_info_t* p_info = (hsrv_ofact_flex_nh_info_t*)phb->data;
    hsrv_openflow_action_combo_t *p_combo = &p_info->combo;
    hsrv_ofnh_debug_data_t* p_data = user_data; 
    FILE* fp = p_data->fp;
    
    sal_fprintf(fp, "%5d ", p_data->i);
    sal_fprintf(fp, "%4d ", p_info->ref_cnt);
    sal_fprintf(fp, "%p ", p_info);
    hsrv_openflow_show_flex_nexthop_combo__(fp, p_combo, 1);
    
    sal_fprintf(fp, "\n");
    
    p_data->i ++;
}

int32
hsrv_openflow_show_nexthop_hash_type(FILE *fp, char* type)
{
    hsrv_ofnh_debug_data_t data;
    
    sal_fprintf(fp, "-------------------- Nexthop hash  ------------------\n");
    
    sal_fprintf(fp, "%5s %4s %10s %7s\n",
        "index", "ref", "ptr", "combo");
    sal_fprintf(fp, "----------------------------------------------------\n");
    
    data.fp = fp;
    data.i  = 0;
    
    if (sal_strncmp (type, "flex", 4) == 0)
    {
        ctclib_hash_iterate(HSRV_OFNH_FLEX_NH_HASH, hsrv_openflow_show_flex_nexthop_hash__, &data);
    }

    sal_fprintf(fp, "----------------------------------------------------\n");

    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_nexthop_hash_ptr(FILE *fp, uint32 hash_ptr)
{
    sal_fprintf(fp, "not support");
    
    return HSRV_E_NONE;
}

#define _____INIT_____

int32
hsrv_openflow_nexthop_start(void)
{
    g_pst_openflow_nexthop_master = HSRV_OPENFLOW_NH_MALLOC(sizeof(hsrv_openflow_nexthop_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_nexthop_master);
    sal_memset(g_pst_openflow_nexthop_master, 0, sizeof(hsrv_openflow_nexthop_master_t));
    
    HSRV_OFNH_FLEX_NH_HASH = ctclib_hash_create(
        hsrv_openflow_nexthop_flex_make,
        hsrv_openflow_nexthop_flex_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OFNH_FLEX_NH_HASH);

    /* Added by weizj for tunnel */
    HSRV_OFNH_IP_TUNNEL_NH_HASH = ctclib_hash_create(
        hsrv_openflow_nexthop_ip_tunnel_make,
        hsrv_openflow_nexthop_ip_tunnel_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OFNH_IP_TUNNEL_NH_HASH);
    /* End by weizj for tunnel */

    HSRV_OFNH_MPLS_NH_HASH = ctclib_hash_create(
        hsrv_openflow_nexthop_mpls_make,
        hsrv_openflow_nexthop_mpls_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OFNH_MPLS_NH_HASH);

    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    HSRV_OFNH_IPUC_NH_HASH = ctclib_hash_create(
        hsrv_openflow_nexthop_ipuc_make,
        hsrv_openflow_nexthop_ipuc_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OFNH_IPUC_NH_HASH);

    HSRV_OFNH_XLATE_NH_HASH = ctclib_hash_create(
        hsrv_openflow_nexthop_xlate_make,
        hsrv_openflow_nexthop_xlate_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OFNH_XLATE_NH_HASH);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_init(hsrv_system_spec_info_t* p_system_info)
{
    /*based on GLB_STM_MODE_OF_DEFAULT in ops_base*/
    g_pst_openflow_nexthop_master->nexthop_max_num = GLB_OPENFLOW_NH_MAX_NUM;
    
    HSRV_OFNH_NH_ID_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OFNH_NH_ID_OPF, 
                            0,
                            (3 +
                             GLB_NH_MAX_NUM +
                             p_system_info->res_info.max_v4ecmp_groups +
                             p_system_info->res_info.max_v6ecmp_groups + 1),
                             GLB_OPENFLOW_NH_MAX_NUM);
    
    HSRV_OFNH_NH_OFFSET_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OFNH_NH_OFFSET_OPF,
                            0,
                            GLB_NH_MAX_NUM - 16 + 1,
                            GLB_OPENFLOW_NH_MAX_NUM);

    HSRV_OFNH_MC_OFFSET_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OFNH_MC_OFFSET_OPF,
                            0,
                            GLB_MCAST_GROUPID_START + 1,
                            GLB_OPENFLOW_MET_MAX_NUM);

#if 0 /* Deleted by weizj for tunnel bug 33724 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_create_output_all());
#endif


    /* for cubro mpls. */
    HSRV_OFNH_MPLS_TUNNEL_ID_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset(HSRV_OFNH_MPLS_TUNNEL_ID_OPF, 
                            0,
                            1,
                            GLB_OPENFLOW_MPLS_MAX_TUNNEL_NUM);

    HSRV_OFNH_MPLS_TUNNEL_HASH = ctclib_hash_create(
                        _hsrv_openflow_mpls_tunnel_hash_make,
                        _hsrv_openflow_mpls_tunnel_hash_compare);
    
    return HSRV_E_NONE;
}


#define _____END_LINE_____
