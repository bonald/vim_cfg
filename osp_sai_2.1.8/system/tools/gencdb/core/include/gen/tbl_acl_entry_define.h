
#ifndef __TBL_ACL_ENTRY_DEFINE_H__
#define __TBL_ACL_ENTRY_DEFINE_H__

/* TBL_ACL_ENTRY field defines */
typedef enum
{
    TBL_ACL_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_ENTRY_FLD_TBLID                = 1 ,  /* RW */
    TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY       = 2 ,  /* RW */
    TBL_ACL_ENTRY_FLD_TIME_RANGE           = 3 ,  /* RW */
    TBL_ACL_ENTRY_FLD_KEY_FLAGS            = 4 ,  /* RW */
    TBL_ACL_ENTRY_FLD_KEY_FLAGS2           = 5 ,  /* RW */
    TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS       = 6 ,  /* RW */
    TBL_ACL_ENTRY_FLD_IS_IPV6              = 7 ,  /* RW */
    TBL_ACL_ENTRY_FLD_ACL_TYPE             = 8 ,  /* RW */
    TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS        = 9 ,  /* RW */
    TBL_ACL_ENTRY_FLD_ARP_OP_CODE          = 10,  /* RW */
    TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK     = 11,  /* RW */
    TBL_ACL_ENTRY_FLD_INVALID              = 12,  /* RW */
    TBL_ACL_ENTRY_FLD_IS_COPP              = 13,  /* RW */
    TBL_ACL_ENTRY_FLD_IN_PORT              = 14,  /* RW */
    TBL_ACL_ENTRY_FLD_OUT_PORT             = 15,  /* RW */
    TBL_ACL_ENTRY_FLD_ETHER_TYPE           = 16,  /* RW */
    TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK      = 17,  /* RW */
    TBL_ACL_ENTRY_FLD_SRC_MAC              = 18,  /* RW */
    TBL_ACL_ENTRY_FLD_SRC_MAC_MASK         = 19,  /* RW */
    TBL_ACL_ENTRY_FLD_DST_MAC              = 20,  /* RW */
    TBL_ACL_ENTRY_FLD_DST_MAC_MASK         = 21,  /* RW */
    TBL_ACL_ENTRY_FLD_SVLAN                = 22,  /* RW */
    TBL_ACL_ENTRY_FLD_SVLAN_MASK           = 23,  /* RW */
    TBL_ACL_ENTRY_FLD_SVLAN_COS            = 24,  /* RW */
    TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK       = 25,  /* RW */
    TBL_ACL_ENTRY_FLD_CVLAN                = 26,  /* RW */
    TBL_ACL_ENTRY_FLD_CVLAN_MASK           = 27,  /* RW */
    TBL_ACL_ENTRY_FLD_CVLAN_COS            = 28,  /* RW */
    TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK       = 29,  /* RW */
    TBL_ACL_ENTRY_FLD_UNTAG_VLAN           = 30,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_TYPE              = 31,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_TYPE_MASK         = 32,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_FLAGS             = 33,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK        = 34,  /* RW */
    TBL_ACL_ENTRY_FLD_SRC_IP               = 35,  /* RW */
    TBL_ACL_ENTRY_FLD_SRC_IP_MASK          = 36,  /* RW */
    TBL_ACL_ENTRY_FLD_DST_IP               = 37,  /* RW */
    TBL_ACL_ENTRY_FLD_DST_IP_MASK          = 38,  /* RW */
    TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL      = 39,  /* RW */
    TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK = 40,  /* RW */
    TBL_ACL_ENTRY_FLD_DSCP                 = 41,  /* RW */
    TBL_ACL_ENTRY_FLD_DSCP_MASK            = 42,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_PRECEDENCE        = 43,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK   = 44,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_PROTOCOL          = 45,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK     = 46,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE     = 47,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_SRC_PORT          = 48,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK     = 49,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE     = 50,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_DST_PORT          = 51,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK     = 52,  /* RW */
    TBL_ACL_ENTRY_FLD_IP_FRAG              = 53,  /* RW */
    TBL_ACL_ENTRY_FLD_TCP_FLAGS            = 54,  /* RW */
    TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK       = 55,  /* RW */
    TBL_ACL_ENTRY_FLD_IGMP_TYPE            = 56,  /* RW */
    TBL_ACL_ENTRY_FLD_ICMP_TYPE            = 57,  /* RW */
    TBL_ACL_ENTRY_FLD_ICMP_CODE            = 58,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_ID               = 59,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_TYPE             = 60,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_OFFSET           = 61,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_VALUE            = 62,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_MASK             = 63,  /* RW */
    TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID      = 64,  /* RW */
    TBL_ACL_ENTRY_FLD_ERSPAN_ID            = 65,  /* RW */
    TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK       = 66,  /* RW */
    TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS     = 67,  /* RW */
    TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET  = 68,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI         = 69,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK    = 70,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_GRE_TYPE          = 71,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_GRE_KEY           = 72,  /* RW */
    TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK      = 73,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER     = 74,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP         = 75,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_DST_IP         = 76,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC        = 77,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM        = 78,  /* RW */
    TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN        = 79,  /* RW */
    TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER     = 80,  /* RW */
    TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP         = 81,  /* RW */
    TBL_ACL_ENTRY_FLD_L3GRE_DST_IP         = 82,  /* RW */
    TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC        = 83,  /* RW */
    TBL_ACL_ENTRY_FLD_CATEGORY_ID          = 84,  /* RW */
    TBL_ACL_ENTRY_FLD_DENY                 = 85,  /* RW */
    TBL_ACL_ENTRY_FLD_STATS_EN             = 86,  /* RW */
    TBL_ACL_ENTRY_FLD_OPTIONS              = 87,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS      = 88,  /* READ */
    TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS = 89,  /* READ */
    TBL_ACL_ENTRY_FLD_INNER_IS_IPV6        = 90,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE     = 91,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK = 92,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SRC_MAC        = 93,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK   = 94,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DST_MAC        = 95,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK   = 96,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SVLAN          = 97,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK     = 98,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS      = 99,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK = 100,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_CVLAN          = 101,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK     = 102,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS      = 103,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK = 104,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_TYPE        = 105,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK   = 106,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS       = 107,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK  = 108,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SRC_IP         = 109,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK    = 110,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DST_IP         = 111,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK    = 112,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL = 113,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK = 114,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DSCP           = 115,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK      = 116,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE  = 117,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK = 118,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL    = 119,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK = 120,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE = 121,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT    = 122,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK = 123,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE = 124,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT    = 125,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK = 126,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IP_FRAG        = 127,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS      = 128,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK = 129,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE      = 130,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE      = 131,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE      = 132,  /* RW */
    TBL_ACL_ENTRY_FLD_INNER_OPTIONS        = 133,  /* RW */
    TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER  = 134,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT  = 135,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN = 136,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG     = 137,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION = 138,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN = 139,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC = 140,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN = 141,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC = 142,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN = 143,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA = 144,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN = 145,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA = 146,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN = 147,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN = 148,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION = 149,  /* RW */
    TBL_ACL_ENTRY_FLD_TAP_GROUP_OID        = 150,  /* RW */
    TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER     = 151,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC        = 152,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP         = 153,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_DST_IP         = 154,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT        = 155,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT       = 156,  /* RW */
    TBL_ACL_ENTRY_FLD_VXLAN_VNI            = 157,  /* RW */
    TBL_ACL_ENTRY_FLD_MAX                  = 158
} tbl_acl_entry_field_id_t;

/* TBL_ACL_ENTRY defines */
typedef struct
{
    uint64               aclid;
} tbl_acl_entry_key_t;

typedef struct
{
    tbl_acl_entry_key_t  key;
    uint32               tblid;
    uint32               entry_priority;
    char                 time_range[TIME_RANGE_SIZE+1];
    uint32               key_flags;
    uint32               key_flags2;
    uint32               key_ipv6_flags;
    uint32               is_ipv6;             /* is ipv6 entry */
    uint32               acl_type;            /*glb_acl_config_type_t*/
    uint32               key_arp_flags;
    uint32               arp_op_code;
    uint32               arp_op_code_mask;
    uint32               invalid;
    uint32               is_copp;
    uint64               in_port;
    uint64               out_port;
    uint32               ether_type;
    uint32               ether_type_mask;
    mac_addr_t           src_mac;
    mac_addr_t           src_mac_mask;
    mac_addr_t           dst_mac;
    mac_addr_t           dst_mac_mask;
    uint32               svlan;
    uint32               svlan_mask;
    uint32               svlan_cos;
    uint32               svlan_cos_mask;
    uint32               cvlan;
    uint32               cvlan_mask;
    uint32               cvlan_cos;
    uint32               cvlan_cos_mask;
    uint32               untag_vlan;
    uint32               ip_type;
    uint32               ip_type_mask;
    uint32               ip_flags;
    uint32               ip_flags_mask;
    prefix_t             src_ip;
    prefix_t             src_ip_mask;
    prefix_t             dst_ip;
    prefix_t             dst_ip_mask;
    uint32               ipv6_flow_label;
    uint32               ipv6_flow_label_mask;
    uint32               dscp;
    uint32               dscp_mask;
    uint32               ip_precedence;
    uint32               ip_precedence_mask;
    uint32               ip_protocol;
    uint32               ip_protocol_mask;
    uint32               l4_src_port_type;
    uint32               l4_src_port;
    uint32               l4_src_port_mask;
    uint32               l4_dst_port_type;
    uint32               l4_dst_port;
    uint32               l4_dst_port_mask;
    uint32               ip_frag;
    uint32               tcp_flags;
    uint32               tcp_flags_mask;
    uint32               igmp_type;
    uint32               icmp_type;
    uint32               icmp_code;
    uint32               udf_id;              /* Duet2 udf */
    uint32               udf_type;            /* 0-disable, l3-udf: 1, l4_udf: 2, same with group type */
    uint32               udf_offset;
    uint32               udf_value[GLB_ACL_UDF_FIELD_NUM_MAX];
    uint32               udf_mask[GLB_ACL_UDF_FIELD_NUM_MAX];
    uint32               udf_value_valid[GLB_ACL_UDF_FIELD_NUM_MAX]; /* Check uniformity of udf entry */
    uint32               erspan_id;
    uint32               erspan_id_mask;
    uint32               strip_header_pos;
    uint32               strip_header_offset;
    uint32               l4_vxlan_vni;
    uint32               l4_vxlan_vni_mask;
    uint32               l4_gre_type;
    uint32               l4_gre_key;
    uint32               l4_gre_key_mask;
    uint32               l2gre_add_header;
    addr_t               l2gre_src_ip;
    addr_t               l2gre_dst_ip;
    mac_addr_t           l2gre_dst_mac;
    uint32               l2gre_key_num;
    uint32               l2gre_key_len;
    uint32               l3gre_add_header;
    addr_t               l3gre_src_ip;
    addr_t               l3gre_dst_ip;
    mac_addr_t           l3gre_dst_mac;
    uint32               category_id;
    uint32               deny;
    uint32               stats_en;
    uint32               options;
    uint64               inner_key_flags;
    uint32               inner_key_ipv6_flags;
    uint32               inner_is_ipv6;       /* is ipv6 entry */
    uint32               inner_ether_type;
    uint32               inner_ether_type_mask;
    mac_addr_t           inner_src_mac;
    mac_addr_t           inner_src_mac_mask;
    mac_addr_t           inner_dst_mac;
    mac_addr_t           inner_dst_mac_mask;
    uint32               inner_svlan;
    uint32               inner_svlan_mask;
    uint32               inner_svlan_cos;
    uint32               inner_svlan_cos_mask;
    uint32               inner_cvlan;
    uint32               inner_cvlan_mask;
    uint32               inner_cvlan_cos;
    uint32               inner_cvlan_cos_mask;
    uint32               inner_ip_type;
    uint32               inner_ip_type_mask;
    uint32               inner_ip_flags;
    uint32               inner_ip_flags_mask;
    prefix_t             inner_src_ip;
    prefix_t             inner_src_ip_mask;
    prefix_t             inner_dst_ip;
    prefix_t             inner_dst_ip_mask;
    uint32               inner_ipv6_flow_label;
    uint32               inner_ipv6_flow_label_mask;
    uint32               inner_dscp;
    uint32               inner_dscp_mask;
    uint32               inner_ip_precedence;
    uint32               inner_ip_precedence_mask;
    uint32               inner_ip_protocol;
    uint32               inner_ip_protocol_mask;
    uint32               inner_l4_src_port_type;
    uint32               inner_l4_src_port;
    uint32               inner_l4_src_port_mask;
    uint32               inner_l4_dst_port_type;
    uint32               inner_l4_dst_port;
    uint32               inner_l4_dst_port_mask;
    uint32               inner_ip_frag;
    uint32               inner_tcp_flags;
    uint32               inner_tcp_flags_mask;
    uint32               inner_igmp_type;
    uint32               inner_icmp_type;
    uint32               inner_icmp_code;
    uint32               inner_options;
    uint32               action_strip_header;
    uint32               tap_action_redirect;
    uint32               tap_action_mark_vlan;
    uint32               tap_action_untag;
    uint32               tap_action_trunction;
    uint32               tap_action_edit_dest_mac_en;
    mac_addr_t           tap_action_edit_dest_mac;
    uint32               tap_action_edit_src_mac_en;
    mac_addr_t           tap_action_edit_src_mac;
    uint32               tap_action_edit_ipda_en;
    addr_t               tap_action_edit_ipda; /* ip address v4/v6 */
    uint32               tap_action_edit_ipsa_en;
    addr_t               tap_action_edit_ipsa; /* ip address v4/v6 */
    uint32               tap_action_edit_vlan_en;
    uint16               tap_action_edit_vlan;
    uint16               tap_action_edit_vlan_action; /* 0:repalce/ 1:add/ 2:add_or_replace */
    uint64               tap_group_oid;
    uint32               add_vxlan_header;
    mac_addr_t           vxlan_dst_mac;
    addr_t               vxlan_src_ip;
    addr_t               vxlan_dst_ip;
    uint32               vxlan_srcport;
    uint32               vxlan_destport;
    uint32               vxlan_vni;
} tbl_acl_entry_t;

typedef struct
{
    ctclib_hash_t        *acl_entry_hash;
    ctclib_slist_t       *acl_entry_list;
} tbl_acl_entry_master_t;

#endif /* !__TBL_ACL_ENTRY_DEFINE_H__ */

