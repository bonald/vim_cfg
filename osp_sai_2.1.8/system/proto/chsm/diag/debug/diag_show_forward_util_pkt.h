/*****************************************************************************
 * diag_show_forward_util_pkt.h      packet generator
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:    R0.01
 * Author:      liul
 * Date:        2010-11-11
 * Reason:      Initial version
 *****************************************************************************/
#ifndef _DIAG_SHOW_FORWARD_UTIL_PKT_H_
#define _DIAG_SHOW_FORWARD_UTIL_PKT_H_

#include "sal.h"

#define L2_TYPE_ETH_V2                 1
#define L2_TYPE_ETH_SAP                2
#define L2_TYPE_ETH_SAP2               3
#define L2_TYPE_ETH_SNAP               4
#define L2_TYPE_RAW_SNAP               5
#define L2_TYPE_PPP                    6
#define MAX_MPLS_LABELS 10
#define MAX_NUM_IGMP_V3_SRC_ADDRESS 10

/* byte operation macros */
#define MAKE_UINT16(hb, lb) (((hb) << 8) | (lb))
#define MAKE_UINT32(b3, b2, b1, b0) (((b3) << 24) | ((b2) << 16)\
                                       | ((b1) << 8) | (b0))

/*
 * byte swapping macros
 */
#define SHOW_FORWARD_SWAP16(x) ((((uint16)(x) & (uint16)0x00ff) << 8) \
                      | (((uint16)(x) & (uint16)0xff00) >> 8))

#define SHOW_FORWARD_SWAP32(x) ((((uint32)(x) & (uint32)0x000000ff) << 24) \
                      | (((uint32)(x) & (uint32)0x0000ff00) << 8) \
                      | (((uint32)(x) & (uint32)0x00ff0000) >> 8) \
                      | (((uint32)(x) & (uint32)0xff000000) >> 24))

#define SHOW_FORWARD_SWAP64(x) \
        ((((uint64)(x) & (uint64)0x00000000000000ffULL) << 56)  \
        | (((uint64)(x) & (uint64)0x000000000000ff00ULL) << 40) \
        | (((uint64)(x) & (uint64)0x0000000000ff0000ULL) << 24) \
        | (((uint64)(x) & (uint64)0x00000000ff000000ULL) << 8)  \
        | (((uint64)(x) & (uint64)0x000000ff00000000ULL) >> 8)  \
        | (((uint64)(x) & (uint64)0x0000ff0000000000ULL) >> 24) \
        | (((uint64)(x) & (uint64)0x00ff000000000000ULL) >> 40) \
        | (((uint64)(x) & (uint64)0xff00000000000000ULL) >> 56))


#define FORWARD_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1: 0)

enum diag_show_forward_record_type
{
    DIAG_REC_STRING,
    DIAG_REC_UINT8,
    DIAG_REC_UINT16,
    DIAG_REC_UINT32,
    DIAG_REC_MAC,
    DIAG_REC_IPV4,
    DIAG_REC_IPV6
};

enum diag_swap_direction
{
    DIAG_HOST_TO_NETWORK,
    DIAG_NETWORK_TO_HOST
};

#if CTC_HOST_IS_LITTLE_ENDIAN
struct diag_bay_packet_header_s
{
    uint32 dest_id             :16;
    uint32 dest_chip_id        :5;
    uint32 multi_cast          :1;
    uint32 pkt_offset         :7;
    uint32 dest_id_discard     :1;
    uint32 hdr_type            :2;

    uint32 src_port            :14;
    uint32 untagged_pkt         :1;
    uint32 vpls_port_type       :1;
    uint32 header_hash         :3;
    uint32 src_queue_select :1;
    uint32 src_cos             :3;
    uint32 pkt_type            :3;
    uint32 priority            :6;

    uint32 nxt_hop_ptr         :18;
    uint32 color               :2;
    uint32 src_vlan_id         :12;

    uint32 hdr_crc             :8;
    uint32 ttl                 :8;
    uint32 pkt_len             :14;
    uint32 critical_pkt        :1;
    uint32 len_adj_type        :1;
};


struct diag_humber_exception_info_s
{
    uint32 exceptions_vect_11_0                                        :12;
    uint32 qos_log_id                                                           :2;
    uint32 acl_log_id                                                            :2;
    uint32 l3_span_id                                                           :2;
    uint32 l2_span_id                                                           :2;
    uint32 exception_pkt_type                                             :3;
    uint32 egress_exception                                                :1;
    uint32 exception_sub_idx                                              :4;
    uint32 exp_vect_12                                                        :1;
    uint32 exp_vect_13                                                        :1;
    uint32 exp_vect_14                                                        :1;
    uint32 exp_vect_15                                                        :1;

    uint32 mac_valid                                                            :1;
    uint32 loopback_en                                                          :1;
};

#else
struct diag_bay_packet_header_s
{
    uint32 hdr_type            :2;
    uint32 dest_id_discard     :1;
    uint32 pkt_offset         :7;
    uint32 multi_cast          :1;
    uint32 dest_chip_id        :5;
    uint32 dest_id             :16;

    uint32 priority            :6;
    uint32 pkt_type            :3;
    uint32 src_cos             :3;
    uint32 src_queue_select :1;
    uint32 header_hash         :3;
    uint32 vpls_port_type       :1;
    uint32 untagged_pkt         :1;
    uint32 src_port            :14;

    uint32 src_vlan_id         :12;
    uint32 color               :2;
    uint32 nxt_hop_ptr         :18;

    uint32 len_adj_type        :1;
    uint32 critical_pkt        :1;
    uint32 pkt_len             :14;
    uint32 ttl                 :8;
    uint32 hdr_crc             :8;
};


struct diag_humber_exception_info_s
{
    uint32 loopback_en                                                            :1;
    uint32 mac_valid                                                          :1;

    uint32 exp_vect_15                                                      :1;
    uint32 exp_vect_14                                                      :1;
    uint32 exp_vect_13                                                      :1;
    uint32 exp_vect_12                                                      :1;
    uint32 exception_sub_idx                                            :4;
    uint32 egress_exception                                              :1;
    uint32 exception_pkt_type                                           :3;
    uint32 l2_span_id                                                         :2;
    uint32 l3_span_id                                                         :2;
    uint32 acl_log_id                                                          :2;
    uint32 qos_log_id                                                         :2;
    uint32 exceptions_vect_11_0                                      :12;
};

#endif
typedef struct diag_bay_packet_header_s diag_bay_packet_header_t;
typedef struct diag_humber_exception_info_s diag_humber_exception_info_t;

enum diag_hdr_type
{
    DIAG_INVALID_HDR=0,
    DIAG_BAY_HEADER_HDR,
    DIAG_BAY_EXCEPTION_HDR,
    DIAG_ETHERNET_HDR,
    DIAG_IPV4_HDR,
    DIAG_IPV6_HDR,
    DIAG_MPLS_HDR,
    DIAG_GRE_HDR,
    DIAG_ARP_HDR,
    DIAG_TCP_HDR,
    DIAG_UDP_HDR,
    DIAG_BPDU_HDR,
    DIAG_ICMPV6_NSMF,
    DIAG_ICMPV6_NAMF,
    DIAG_IGMP_V1_HDR,
    DIAG_IGMP_V2_HDR,
    DIAG_IGMP_V3_HDR,
    DIAG_PAD_HDR,
    DIAG_PAYLOAD
};


struct diag_humber_exception_config
{
    uint32 exceptions_vect_11_0;
    uint32 qos_log_id          ;
    uint32 acl_log_id          ;
    uint32 l3_span_id          ;
    uint32 l2_span_id          ;
    uint32 exception_pkt_type  ;
    uint32 egress_exception    ;
    uint32 exception_sub_idx   ;
    uint32 exp_vect_12         ;
    uint32 exp_vect_13         ;
    uint32 exp_vect_14         ;
    uint32 exp_vect_15         ;
    uint32 mac_valid           ;
    uint32 loopback_en         ;
};
typedef struct diag_humber_exception_config diag_humber_exception_config_t;

struct diag_bay_hdr_config
{
    uint32 nxt_hop_ptr;

    uint8 hdr_type;
    uint8 dest_id_discard;
    uint8 pkt_offset;
    uint8 multi_cast;

    uint8 dest_chip_id;
    uint8 priority;
    uint8 pkt_type;
    uint8 src_cos;

    uint16 src_port;
    uint16 dest_id;

    uint16 src_vlan_id;
    uint8 header_hash;

    uint8 hdr_crc;
    uint8 ttl;
    uint16 pkt_len;

    uint8 color;
    uint8 len_adj_type;
    uint8 critical_pkt;

    uint8 src_que_select;
    uint8 vpls_port_type;
    uint8 untagged_pkt;

    uint32 customer_label;
    uint32 martini_encap;
    uint8 hdr_crc_config;

};
typedef struct diag_bay_hdr_config diag_bay_hdr_config_t;

struct diag_carmel_hdr_config
{
    uint32 nxt_hop_ptr;

    uint8 hdr_type;
    uint8 dest_id_discard;
    uint8 pkt_offset;
    uint8 mcast;

    uint8 dest_chip_id;
    uint16 dest_id;
    uint8 priority;

    uint8 pkt_type_2to0;
    uint8 src_cos;
    uint8 src_queue_select;
    uint8 header_hash;

    uint8 vpls_port_type;
    uint16 src_port;

    uint16 src_svlan_id;
    uint8 color;

    uint8 length_adjust_type;
    uint8 critical_pkt;
    uint16 pkt_len;

    uint8 ttl;
    uint8 header_crc;
    uint8 src_port_isolated_id;
    uint8 pbb_src_port_type;

    uint8 src_port_oui_index;
    uint8 ether_oam_discard;
    uint8 from_ipe;
    uint8 tx_ether_oam;

    uint8 deny_replace_cos;
    uint8 deny_replace_dscp;
    uint8 src_svlan_id_valid;
    uint8 src_cvlan_id_valid;

    uint16 src_cvlan_id;
    uint16 src_vlan_ptr;

    uint16 vrf_id;
    uint16 esp_id;

    uint8 rx_oam_type;
    uint8 use_outer_vrfid;
    uint8 next_hop_ext;
    uint8 tx_ether_down_oam;

     uint32 customer_label;
    uint32 martini_encap;
    uint8 hdr_crc_config;

};
typedef struct diag_carmel_hdr_config diag_carmel_hdr_config_t;

struct diag_ethernet_config
{
    uint32 l2_type;
    uint8  mac_da[6];
    uint8  mac_sa[6];
    uint8 num_vlan;

    uint16 vlan_tag1;
    uint8  cos1;
    uint8  cfi1;
    uint16 vid1;
    uint16 vlan_tag2;
    uint8  cos2;
    uint8  cfi2;
    uint16 vid2;

    uint16 type;
    uint16 len;
    uint8  dsap;
    uint8  ssap;
    uint8  ctl0;
    uint8  ctl1;
    uint32 oui;

    uint16 protocol;

};
typedef struct diag_ethernet_config diag_ethernet_config_t;

struct diag_ipv4_config
{
    uint32 ipv4_da;
    uint32 ipv4_sa;
    uint16 ipv4_id;
    uint16 ipv4_flags;
    uint16 ipv4_frag_offset;
    uint8  ipv4_ttl;
    uint8  ipv4_proto;
    uint8 ipv4_type_serv;
    uint16 ipv4_total_len;
    uint16 ipv4_hdr_checksum;
    uint8 ipv4_ihl;
};
typedef struct diag_ipv4_config diag_ipv4_config_t;
 
struct diag_ipv6_config
{
    uint32 ipv6_flow_label;
    uint32 ipv6_sa[4];
    uint32 ipv6_da[4];
    uint8  ipv6_traf_class;
    uint8  ipv6_nxt_hdr;
    uint8  ipv6_hp_limit;
    uint16 ipv6_pld_len;
    uint8  pad;
};
typedef struct  diag_ipv6_config diag_ipv6_config_t;

struct diag_arp_config
{
    uint16 arp_hdw_type;
    uint16 arp_protocol_type;
    uint8  arp_hdw_addr_len;
    uint8  arp_protocol_addr_len;
    uint16 arp_operation_code;
    uint8  arp_sender_hdw_add[6];
    uint32 arp_sender_protocol_add;
    uint8  arp_dst_hdw_add[6];
    uint32 arp_dst_protocol_add;
};
typedef struct diag_arp_config diag_arp_config_t;

struct diag_mpls_cfg
{
    uint32 mpls_label;
    uint8  mpls_exp;
    uint8  mpls_s;
    uint8  mpls_ttl;
    uint8  pad;
};
typedef struct diag_mpls_cfg diag_mpls_cfg_t;

struct diag_mpls_config
{
    uint8  mpls_label_num;
    diag_mpls_cfg_t  mpls[MAX_MPLS_LABELS];
    uint8  pad;
};
typedef struct diag_mpls_config diag_mpls_config_t;


struct diag_gre_config
{
    uint8 gre_c;
    uint8 gre_r;
    uint8 gre_k;
    uint8 gre_s;

    uint16 gre_reserved0;
    uint8 gre_ver;
    uint16 gre_protocal_type;
    uint16 gre_checksum;
    uint16 gre_reserved1;
    uint32 gre_key;
    uint32 gre_sq_num;
    uint32 gre_routing;
};
typedef struct diag_gre_config diag_gre_config_t;

struct diag_bpdu_config
{
    uint8 llc_dsap;
    uint8 llc_ssap;
    uint8 llc_ctlo;
    uint16 protocol_id;
    uint8 ver;
    uint8 type;
    uint8 flag;
    uint16 root_id_pri;
    uint8 root_id_mac[6];
    uint32 root_pathcost;
    uint16 bridge_id_pri;
    uint8 bridge_id_mac[6];
    uint16 port_id;
    uint16 msg_age;
    uint16 max_age;
    uint16 hello_time;
    uint16 fwd_delay;
    uint32 dlc_value1;
    uint32 dlc_value2;
};
typedef struct diag_bpdu_config diag_bpdu_config_t;

struct diag_tcp_config
{
    uint16 sport;
    uint16 dport;
    uint32 seq;
    uint32 ack_seq;
    uint8  data_offset;
    uint16 flag;
    uint16 window;
    uint16 chksum;
    uint16 urg_ptr;
    uint8  pad;
} ;
typedef struct diag_tcp_config diag_tcp_config_t;

struct diag_udp_config{
    uint16 sport;
    uint16 dport;
    uint16 len;
    uint16 chksum;
    uint8 mip_type;
    uint8  pad;
};
typedef struct diag_udp_config diag_udp_config_t;

struct diag_icmpv6_nsmf_config{
    uint8 type;
    uint8 code;
    uint16 chksum;
    uint32 reserved;
    uint32 target_addr[4];
    uint16 len;
    uint8 targ_link_lay_addr[6];
};
typedef struct diag_icmpv6_nsmf_config diag_icmpv6_nsmf_config_t;

struct diag_icmpv6_namf_config{
    uint8 type;
    uint8 code;
    uint16 chksum;
    uint8 r;
    uint8 s;
    uint8 o;
    uint32 reserved;
    uint32 target_addr[4];
    uint16 len;
    uint8 targ_link_lay_addr[6];
};
typedef struct diag_icmpv6_namf_config diag_icmpv6_namf_config_t;

struct diag_igmp_v1_config
{
    uint8 version;
    uint8 type;
    uint16 checksum;

    uint32 group_address;
};
typedef struct diag_igmp_v1_config diag_igmp_v1_config_t;

struct diag_igmp_v2_config
{
    uint8 type;
    uint8 max_resp_time;
    uint16 checksum;

    uint32 group_address;
};
typedef struct diag_igmp_v2_config diag_igmp_v2_config_t;

struct diag_igmp_v3_config
{
    uint8 type;
    uint8 max_resp_time;
    uint16 checksum;

    uint32 group_address;
    uint8 resv;
    uint8 s;
    uint8 qrv;
    uint8 qqic;
    uint8 num_of_src;
    uint32 sr_address[MAX_NUM_IGMP_V3_SRC_ADDRESS];
    uint8  pad;
 };
typedef struct diag_igmp_v3_config diag_igmp_v3_config_t;

struct diag_pad_config
{
    uint32 pad_length;
};
typedef struct diag_pad_config diag_pad_config_t;

struct diag_pld_config
{
    uint32 pld_len;
};
typedef struct diag_pld_config diag_pld_config_t;


struct diag_config
{
    int32 header_type;
    union
    {
        diag_bay_hdr_config_t bay_hdr;
        diag_carmel_hdr_config_t carmel_hdr;
        diag_humber_exception_config_t exception;
        diag_ethernet_config_t ethernet;
        diag_ipv4_config_t ipv4;
        diag_ipv6_config_t ipv6;
        diag_arp_config_t arp;
        diag_mpls_config_t mpls;
        diag_gre_config_t gre;
        diag_bpdu_config_t bpdu;
        diag_tcp_config_t tcp;
        diag_udp_config_t udp;
        diag_icmpv6_nsmf_config_t icmpv6_nsmf;
        diag_icmpv6_namf_config_t icmpv6_namf;
        diag_igmp_v1_config_t igmp_v1;
        diag_igmp_v2_config_t igmp_v2;
        diag_igmp_v3_config_t igmp_v3;
        diag_pad_config_t pad;
        diag_pld_config_t payload;
    };

};
typedef struct diag_config diag_config_t;

struct diag_common_ctr
{
    uint32 pkt_count;
    uint32 delay;
    uint32 pkt_sn_incre;
    uint32 channel_incre;
};
typedef struct diag_common_ctr diag_common_ctr_t;

struct diag_ethernet_ctr
{
    uint32 mac_da_incre;
    uint32 mac_sa_incre;
    uint32 vid_incre;
    uint16 type_incr;

    uint32 mac_da_offset;
    uint32 mac_sa_offset;
    uint16 vid_offset;
    uint16 type_offset;
};
typedef struct diag_ethernet_ctr diag_ethernet_ctr_t;

struct diag_ipv4_ctr
{
    uint32 ip_sa_incre;
    uint32 ip_da_incre;
    uint16 ip_id_incre;
    uint8 *ipv4_start;

    uint32 ipsa_offset;
    uint32 ipda_offset;
    uint16 ip_id_offset;
    uint16 check_sum_offset;
    uint16 total_len_offset;
};
typedef struct diag_ipv4_ctr  diag_ipv4_ctr_t;

struct diag_ipv6_ctr
{
    uint32 ip_sa_incre;
    uint32 ip_da_incre;
    uint8 *ipv6_start;

    uint32 ipsa_offset;
    uint32 ipda_offset;
    uint16 total_len_offset;
};
typedef struct diag_ipv6_ctr  diag_ipv6_ctr_t;

struct diag_mpls_ctr
{
    uint32 label_incre;
    uint32 label_offset;

};
typedef struct diag_mpls_ctr diag_mpls_ctr_t;

struct diag_tcp_ctr
{
    uint16 sport_incre;
    uint16 dport_incre;
    uint32 seq_incre;
    uint8 *tcp_start;

    uint16 sport_offset;
    uint16 dport_offset;
    uint32 seq_offset;
    uint16 check_sum_offset;
};
typedef struct diag_tcp_ctr diag_tcp_ctr_t;

struct diag_udp_ctr
{
    uint16 sport_incre;
    uint16 dport_incre;
    uint8 *udp_start;
    uint8 udp_len_offset;

    uint16 sport_offset;
    uint16 dport_offset;
    uint8  pad;
};
typedef struct diag_udp_ctr diag_udp_ctr_t;

struct diag_payload_ctr
{
    uint32 payload_len_incr;
};
typedef struct diag_payload_ctr diag_payload_ctr_t;

struct diag_bay_header_ctr
{
    uint8 bay_hdr_start_offset;
    uint16 dest_id_incr;
    uint32 nxt_hop_ptr_incr;
    uint16 src_vlan_id_incr;
    uint8  pad;
};
typedef struct diag_bay_header_ctr diag_bay_header_ctr_t;

struct diag_carmel_header_ctr
{
    uint8 bay_hdr_start_offset;
    uint16 dest_id_incr;
    uint32 nxt_hop_ptr_incr;
    uint16 src_vlan_id_incr;
    uint8  pad;
};
typedef struct diag_carmel_header_ctr diag_carmel_header_ctr_t;

struct diag_pad_ctr
{
    uint32 pad_length_incr;
    uint8  pad_offset;
    uint8  pad;
};
typedef struct diag_pad_ctr diag_pad_ctr_t;

struct diag_ctrl_var
{
    diag_common_ctr_t  common;
    diag_bay_header_ctr_t bay_hdr;
    diag_carmel_header_ctr_t carmel_hdr;
    diag_ethernet_ctr_t  ethernet;
    diag_ipv4_ctr_t  ipv4;
    diag_ipv6_ctr_t  ipv6;
    diag_mpls_ctr_t  mpls[MAX_MPLS_LABELS];
    diag_tcp_ctr_t  tcp;
    diag_udp_ctr_t udp;
    diag_pad_ctr_t pad;
    diag_payload_ctr_t payload;
};
typedef struct diag_ctrl_var diag_ctrl_var_t;

struct diag_pkt_s
{
    uint32      pkt_id;
    uint8       *pkt;
    uint32      *pkt_info;
    uint16      pkt_len;
    uint8       chan_id;
    uint8       chip_id;
    uint8       reserved[3];
    uint8       has_hdr;
};
typedef struct diag_pkt_s diag_pkt_t;

struct diag_ipv6_addr_s
{
    uint32 address[4];
    uint32 mask;
};
typedef struct diag_ipv6_addr_s diag_ipv6_addr_sipv6_addr_t;

#endif /* _DIAG_SHOW_FORWARD_UTIL_PKT_H_ */
