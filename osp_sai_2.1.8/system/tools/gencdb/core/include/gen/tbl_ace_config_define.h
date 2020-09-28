
#ifndef __TBL_ACE_CONFIG_DEFINE_H__
#define __TBL_ACE_CONFIG_DEFINE_H__

/* TBL_ACE_CONFIG field defines */
typedef enum
{
    TBL_ACE_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACE_CONFIG_FLD_KEY_FLAGS            = 1 ,  /* RW */
    TBL_ACE_CONFIG_FLD_KEY_FLAGS2           = 2 ,  /* RW */
    TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS       = 3 ,  /* RW */
    TBL_ACE_CONFIG_FLD_IS_IPV6              = 4 ,  /* RW */
    TBL_ACE_CONFIG_FLD_ACL_TYPE             = 5 ,  /* RW */
    TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS        = 6 ,  /* RW */
    TBL_ACE_CONFIG_FLD_ARP_OP_CODE          = 7 ,  /* RW */
    TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK     = 8 ,  /* RW */
    TBL_ACE_CONFIG_FLD_TIME_RANGE           = 9 ,  /* RW */
    TBL_ACE_CONFIG_FLD_APPLY_CNT            = 10,  /* READ */
    TBL_ACE_CONFIG_FLD_IN_PORT              = 11,  /* RW */
    TBL_ACE_CONFIG_FLD_OUT_PORT             = 12,  /* RW */
    TBL_ACE_CONFIG_FLD_ETHER_TYPE           = 13,  /* RW */
    TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK      = 14,  /* RW */
    TBL_ACE_CONFIG_FLD_SRC_MAC              = 15,  /* RW */
    TBL_ACE_CONFIG_FLD_SRC_MAC_MASK         = 16,  /* RW */
    TBL_ACE_CONFIG_FLD_DST_MAC              = 17,  /* RW */
    TBL_ACE_CONFIG_FLD_DST_MAC_MASK         = 18,  /* RW */
    TBL_ACE_CONFIG_FLD_SVLAN                = 19,  /* RW */
    TBL_ACE_CONFIG_FLD_SVLAN_MASK           = 20,  /* RW */
    TBL_ACE_CONFIG_FLD_SVLAN_COS            = 21,  /* RW */
    TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK       = 22,  /* RW */
    TBL_ACE_CONFIG_FLD_CVLAN                = 23,  /* RW */
    TBL_ACE_CONFIG_FLD_CVLAN_MASK           = 24,  /* RW */
    TBL_ACE_CONFIG_FLD_CVLAN_COS            = 25,  /* RW */
    TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK       = 26,  /* RW */
    TBL_ACE_CONFIG_FLD_UNTAG_VLAN           = 27,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_TYPE              = 28,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_TYPE_MASK         = 29,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_FLAGS             = 30,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK        = 31,  /* RW */
    TBL_ACE_CONFIG_FLD_SRC_IP               = 32,  /* RW */
    TBL_ACE_CONFIG_FLD_SRC_IP_MASK          = 33,  /* RW */
    TBL_ACE_CONFIG_FLD_DST_IP               = 34,  /* RW */
    TBL_ACE_CONFIG_FLD_DST_IP_MASK          = 35,  /* RW */
    TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL      = 36,  /* RW */
    TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK = 37,  /* RW */
    TBL_ACE_CONFIG_FLD_DSCP                 = 38,  /* RW */
    TBL_ACE_CONFIG_FLD_DSCP_MASK            = 39,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_PRECEDENCE        = 40,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK   = 41,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_PROTOCOL          = 42,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK     = 43,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE     = 44,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_SRC_PORT          = 45,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK     = 46,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE     = 47,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_DST_PORT          = 48,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK     = 49,  /* RW */
    TBL_ACE_CONFIG_FLD_IP_FRAG              = 50,  /* RW */
    TBL_ACE_CONFIG_FLD_TCP_FLAGS            = 51,  /* RW */
    TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK       = 52,  /* RW */
    TBL_ACE_CONFIG_FLD_IGMP_TYPE            = 53,  /* RW */
    TBL_ACE_CONFIG_FLD_ICMP_TYPE            = 54,  /* RW */
    TBL_ACE_CONFIG_FLD_ICMP_CODE            = 55,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_ID               = 56,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_TYPE             = 57,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_OFFSET           = 58,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_VALUE            = 59,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_MASK             = 60,  /* RW */
    TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID      = 61,  /* RW */
    TBL_ACE_CONFIG_FLD_ERSPAN_ID            = 62,  /* RW */
    TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK       = 63,  /* RW */
    TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS     = 64,  /* RW */
    TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET  = 65,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI         = 66,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK    = 67,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_GRE_TYPE          = 68,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_GRE_KEY           = 69,  /* RW */
    TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK      = 70,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER     = 71,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP         = 72,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_DST_IP         = 73,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC        = 74,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM        = 75,  /* RW */
    TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN        = 76,  /* RW */
    TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER     = 77,  /* RW */
    TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP         = 78,  /* RW */
    TBL_ACE_CONFIG_FLD_L3GRE_DST_IP         = 79,  /* RW */
    TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC        = 80,  /* RW */
    TBL_ACE_CONFIG_FLD_DENY                 = 81,  /* RW */
    TBL_ACE_CONFIG_FLD_STATS_EN             = 82,  /* RW */
    TBL_ACE_CONFIG_FLD_OPTIONS              = 83,  /* RW */
    TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME      = 84,  /* READ */
    TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER  = 85,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT  = 86,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN = 87,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG     = 88,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION = 89,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN = 90,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC = 91,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN = 92,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC = 93,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN = 94,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA = 95,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN = 96,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA = 97,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN = 98,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN = 99,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION = 100,  /* RW */
    TBL_ACE_CONFIG_FLD_TAP_SNMP_SET         = 101,  /* RW */
    TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER     = 102,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC        = 103,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP         = 104,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_DST_IP         = 105,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT        = 106,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT       = 107,  /* RW */
    TBL_ACE_CONFIG_FLD_VXLAN_VNI            = 108,  /* RW */
    TBL_ACE_CONFIG_FLD_MAX                  = 109
} tbl_ace_config_field_id_t;

/* TBL_ACE_CONFIG defines */
typedef ace_config_key_t tbl_ace_config_key_t;

typedef struct
{
    tbl_ace_config_key_t key;
    uint32               key_flags;
    uint32               key_flags2;
    uint32               key_ipv6_flags;
    uint32               is_ipv6;             /* is ipv6 entry */
    uint32               acl_type;            /*glb_acl_config_type_t*/
    uint32               key_arp_flags;
    uint32               arp_op_code;
    uint32               arp_op_code_mask;
    char                 time_range[TIME_RANGE_SIZE+1];
    uint16               apply_cnt;           /* ace apply count: if not zero, time-range is started either*/
    uint32               in_port;
    uint32               out_port;
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
    uint32               deny;
    uint32               stats_en;
    uint32               options;
    char                 flow_match_name[ACL_NAME_FULL_NAME_SIZE+1];
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
    uint32               tap_snmp_set;
    uint32               add_vxlan_header;
    mac_addr_t           vxlan_dst_mac;
    addr_t               vxlan_src_ip;
    addr_t               vxlan_dst_ip;
    uint32               vxlan_srcport;
    uint32               vxlan_destport;
    uint32               vxlan_vni;
} tbl_ace_config_t;

typedef struct
{
    ctclib_hash_t        *ace_config_hash;
    ctclib_slist_t       *ace_config_list;
} tbl_ace_config_master_t;

#endif /* !__TBL_ACE_CONFIG_DEFINE_H__ */

