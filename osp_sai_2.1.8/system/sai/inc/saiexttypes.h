
#if !defined (__SAIEXTTYPES_H_)
#define __SAIEXTTYPES_H_

#include <saiacl.h>

typedef struct
{
    sai_uint32_t        ipda_en;
    sai_ip4_t           ipda;
    sai_uint32_t        ipv6da_en;
    sai_ip6_t           ipv6da;
    sai_uint32_t        ipsa_en;
    sai_ip4_t           ipsa;
    sai_uint32_t        ipv6sa_en;
    sai_ip6_t           ipv6sa;
    sai_uint32_t        macda_en;
    sai_mac_t           macda;
    sai_uint32_t        macsa_en;
    sai_mac_t           macsa;
    sai_uint32_t        strip_header;
    
    /*added by yejl for bug 53475, 2019-09-10*/
    sai_uint32_t        add_l2gre_header;
    sai_ip4_t           l2gre_ipda;
    sai_ip4_t           l2gre_ipsa;
    sai_mac_t           l2gre_macda;
    sai_uint32_t        l2gre_key_num;
    sai_uint32_t        l2gre_key_len;
    
    sai_uint32_t        add_l3gre_header;
    sai_ip4_t           l3gre_ipda;
    sai_ip4_t           l3gre_ipsa;
    sai_mac_t           l3gre_macda;
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    sai_uint32_t    vxlan_add_header;
    sai_mac_t       vxlan_macda;
    sai_ip4_t         vxlan_ipsa;
    sai_ip4_t         vxlan_ipda;
    sai_uint32_t    vxlan_portsrc;
    sai_uint32_t    vxlan_portdst;
    sai_uint32_t    vxlan_vni;
    /*ended by guoxd*/
    
    sai_uint32_t        l4_vxlan_vni_en;
    sai_uint32_t        l4_vxlan_vni;
    sai_uint32_t        l4_vxlan_vni_mask;
    sai_uint32_t        trunction_en;
    sai_object_id_t     tap_group_id;

    void                *ptunnel_info;  /* stbl_acl_tunnel_t */
    void                *ptap_info;     /* ctc_sai_tap_info_t */
    void                *edit_group;    /* ctc_sai_tap_group_edit_group */
} ctc_sai_acl_tap_t;

typedef struct
{
    bool                    is_ipv6;
    bool                    is_mac;
    bool                    is_ipv4_packet;
    sai_acl_field_data_t    ether_macsa;
    sai_acl_field_data_t    ether_macda;

    sai_acl_field_data_t    ether_svlan;
    sai_acl_field_data_t    ether_svlan_pri;
    sai_acl_field_data_t    ether_svlan_cfi;
    sai_acl_field_data_t    ether_cvlan;
    sai_acl_field_data_t    ether_cvlan_pri;
    sai_acl_field_data_t    ether_cvlan_cfi;
    sai_acl_field_data_t    ether_untag_vlan;

    sai_acl_field_data_t    ether_type;
    
    sai_acl_field_data_t    ipv4_ipsa;
    sai_acl_field_data_t    ipv4_ipda;
    sai_acl_field_data_t    ipv4_src_port;
    sai_acl_field_data_t    ipv4_dst_port;
    sai_acl_field_data_t    ipv4_ip_protocol;
    sai_acl_field_data_t    ipv4_dscp;
    sai_acl_field_data_t    ipv4_ip_frag;
    sai_acl_field_data_t    ipv4_ip_precedence;
    sai_acl_field_data_t    ipv4_icmp_type;
    sai_acl_field_data_t    ipv4_icmp_code;
    sai_acl_field_data_t    ipv4_igmp_type;
    sai_acl_field_data_t    ipv4_tcp_flag;
    sai_acl_field_data_t    ipv4_ip_options;
    sai_acl_field_data_t    ipv4_routed_packet;
    sai_acl_field_data_t    ipv4_vxlan_vni;
    sai_acl_field_data_t    ipv4_vxlan_vni_dst_port;

    sai_acl_field_data_t    ipv6_ipsa;
    sai_acl_field_data_t    ipv6_ipda;
    sai_acl_field_data_t    ipv6_flow_label;

    sai_acl_field_data_t    gre_type;
    sai_acl_field_data_t    gre_key;
    sai_acl_field_data_t    nvgre_vni;

    sai_acl_field_data_t    udf_id;
    sai_acl_field_data_t    udf_type;
    sai_acl_field_data_t    udf_match_data;
    sai_acl_field_data_t    arp_op_type;
    sai_acl_field_data_t    arp_sender_ip;
    sai_acl_field_data_t    arp_target_ip;
	sai_acl_field_data_t    erspan_id;
} ctc_sai_acl_field_list_t;

typedef struct
{
    sai_acl_action_data_t    action;
    sai_acl_action_data_t    counter;
    sai_acl_action_data_t    mirror_ingress;
    sai_acl_action_data_t    mirror_egress;
    sai_acl_action_data_t    set_color;
    sai_acl_action_data_t    set_policer;
    sai_acl_action_data_t    redirect;
    sai_acl_action_data_t    redirect_vlan_id;
    sai_acl_action_data_t    redirect_vlan_untag;
    sai_acl_action_data_t    trunction;
    sai_acl_action_data_t    src_mac;
    sai_acl_action_data_t    dst_mac;
    sai_acl_action_data_t    src_ip;
    sai_acl_action_data_t    dst_ip;
    sai_acl_action_data_t    src_ipv6;
    sai_acl_action_data_t    dst_ipv6;
    sai_acl_action_data_t    strip_header;
    sai_acl_action_data_t    strip_header_pos;
    sai_acl_action_data_t    strip_header_offset;
	sai_acl_action_data_t    erspan_strip_inner_vxlan_header;
    sai_acl_action_data_t    outer_vlan;
	sai_acl_action_data_t    outer_vlan_action;

    /*added by yejl for bug 53475, 2019-09-10*/
    sai_acl_action_data_t    add_l2gre_header;
    sai_acl_action_data_t    l2gre_src_ip;
    sai_acl_action_data_t    l2gre_dst_ip;
	sai_acl_action_data_t    l2gre_dst_mac;
    sai_acl_action_data_t    l2gre_key_num;
	sai_acl_action_data_t    l2gre_key_len;

    sai_acl_action_data_t    add_l3gre_header;
    sai_acl_action_data_t    l3gre_src_ip;
    sai_acl_action_data_t    l3gre_dst_ip;
	sai_acl_action_data_t    l3gre_dst_mac;
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    sai_acl_action_data_t   add_vxlan_header;
    sai_acl_action_data_t   vxlan_dst_mac;
    sai_acl_action_data_t   vxlan_src_ip;
    sai_acl_action_data_t   vxlan_dst_ip;
    sai_acl_action_data_t   vxlan_srcport;
    sai_acl_action_data_t   vxlan_dstport;
    sai_acl_action_data_t   vxlan_vni;
    /*ended by guoxd*/
} ctc_sai_acl_action_list_t;

typedef struct
{
    ctc_sai_acl_field_list_t        fields;
    sai_object_id_t                 port_oid;
} ctc_sai_acl_tunnel_key_t;

typedef struct 
{
    sai_attribute_t array[SAI_ACL_ENTRY_ATTR_ACTION_END-SAI_ACL_ENTRY_ATTR_ACTION_START+1];
} ctc_sai_acl_action_attrlist_t;

#endif /* !__SAIEXTTYPES_H_ */

