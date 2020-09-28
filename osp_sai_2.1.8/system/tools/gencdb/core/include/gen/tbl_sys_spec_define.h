
#ifndef __TBL_SYS_SPEC_DEFINE_H__
#define __TBL_SYS_SPEC_DEFINE_H__

/* TBL_SYS_SPEC field defines */
typedef enum
{
    TBL_SYS_SPEC_FLD_KEY                  = 0 ,  /* READ */
    TBL_SYS_SPEC_FLD_UCAST_FDB            = 1 ,  /* READ */
    TBL_SYS_SPEC_FLD_STATIC_FDB           = 2 ,  /* READ */
    TBL_SYS_SPEC_FLD_MSTP_INSTANCE        = 3 ,  /* READ */
    TBL_SYS_SPEC_FLD_VLAN_INSTANCE        = 4 ,  /* READ */
    TBL_SYS_SPEC_FLD_MAC_FILTER           = 5 ,  /* READ */
    TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS = 6 ,  /* READ */
    TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS = 7 ,  /* READ */
    TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD    = 8 ,  /* READ */
    TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT = 9 ,  /* READ */
    TBL_SYS_SPEC_FLD_DOT1X_MAC            = 10,  /* READ */
    TBL_SYS_SPEC_FLD_INDIRECT_ROUTES      = 11,  /* READ */
    TBL_SYS_SPEC_FLD_STATIC_ROUTES        = 12,  /* READ */
    TBL_SYS_SPEC_FLD_HOST_ROUTES          = 13,  /* READ */
    TBL_SYS_SPEC_FLD_ECMP_ROUTES          = 14,  /* READ */
    TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM  = 15,  /* READ */
    TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES   = 16,  /* READ */
    TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES     = 17,  /* READ */
    TBL_SYS_SPEC_FLD_HOST_V6_ROUTES       = 18,  /* READ */
    TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES       = 19,  /* READ */
    TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM = 20,  /* READ */
    TBL_SYS_SPEC_FLD_PBR_ENTRIES          = 21,  /* READ */
    TBL_SYS_SPEC_FLD_L2MC_ENTRIES         = 22,  /* READ */
    TBL_SYS_SPEC_FLD_L3MC_ENTRIES         = 23,  /* READ */
    TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS    = 24,  /* READ */
    TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS    = 25,  /* READ */
    TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS    = 26,  /* READ */
    TBL_SYS_SPEC_FLD_VOICE_VLAN           = 27,  /* READ */
    TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS = 28,  /* READ */
    TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS  = 29,  /* READ */
    TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS = 30,  /* READ */
    TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS = 31,  /* READ */
    TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS = 32,  /* READ */
    TBL_SYS_SPEC_FLD_G8031_GROUPS         = 33,  /* READ */
    TBL_SYS_SPEC_FLD_G8032_RINGS          = 34,  /* READ */
    TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING = 35,  /* READ */
    TBL_SYS_SPEC_FLD_FTN_ENTRIES          = 36,  /* READ */
    TBL_SYS_SPEC_FLD_ILM_ENTRIES          = 37,  /* READ */
    TBL_SYS_SPEC_FLD_MPLS_LABLES          = 38,  /* READ */
    TBL_SYS_SPEC_FLD_VPWS                 = 39,  /* READ */
    TBL_SYS_SPEC_FLD_LSP_PE               = 40,  /* READ */
    TBL_SYS_SPEC_FLD_LSP_P                = 41,  /* READ */
    TBL_SYS_SPEC_FLD_VPLS_PEER            = 42,  /* READ */
    TBL_SYS_SPEC_FLD_VPLS_AC              = 43,  /* READ */
    TBL_SYS_SPEC_FLD_VSI                  = 44,  /* READ */
    TBL_SYS_SPEC_FLD_LSP_OAM              = 45,  /* READ */
    TBL_SYS_SPEC_FLD_PW_OAM               = 46,  /* READ */
    TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL      = 47,  /* READ */
    TBL_SYS_SPEC_FLD_CFM_LCK              = 48,  /* READ */
    TBL_SYS_SPEC_FLD_IP_TUNNEL            = 49,  /* READ */
    TBL_SYS_SPEC_FLD_IVI_PEERS            = 50,  /* READ */
    TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES       = 51,  /* READ */
    TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES      = 52,  /* READ */
    TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES      = 53,  /* READ */
    TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES      = 54,  /* READ */
    TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS = 55,  /* READ */
    TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD    = 56,  /* READ */
    TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS = 57,  /* READ */
    TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS = 58,  /* READ */
    TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY = 59,  /* READ */
    TBL_SYS_SPEC_FLD_BFD_SESSIONS         = 60,  /* READ */
    TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS = 61,  /* READ */
    TBL_SYS_SPEC_FLD_NVGRE_PEERS          = 62,  /* READ */
    TBL_SYS_SPEC_FLD_NVGRE_TUNNELS        = 63,  /* READ */
    TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES = 64,  /* READ */
    TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM  = 65,  /* READ */
    TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM = 66,  /* READ */
    TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM = 67,  /* READ */
    TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM     = 68,  /* READ */
    TBL_SYS_SPEC_FLD_LAG_GROUP_NUM        = 69,  /* READ */
    TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM  = 70,  /* READ */
    TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM = 71,  /* READ */
    TBL_SYS_SPEC_FLD_TAP_FLOW_NUM         = 72,  /* READ */
    TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM   = 73,  /* READ */
    TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM       = 74,  /* READ */
    TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM = 75,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_POOL_NUM         = 76,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_RULE_NUM         = 77,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM  = 78,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM = 79,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM = 80,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM = 81,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM = 82,  /* READ */
    TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM = 83,  /* READ */
    TBL_SYS_SPEC_FLD_MAX                  = 84
} tbl_sys_spec_field_id_t;

/* TBL_SYS_SPEC defines */
typedef struct
{
    uint32               type;
} tbl_sys_spec_key_t;

typedef struct
{
    tbl_sys_spec_key_t   key;
    uint32               ucast_fdb;
    uint32               static_fdb;
    uint32               mstp_instance;
    uint32               vlan_instance;
    uint32               mac_filter;
    uint32               mac_based_vlan_class;
    uint32               ipv4_based_vlan_class;
    uint32               ipv4_source_guard;
    uint32               vlan_mapping_entry_applied_port;
    uint32               dot1x_mac;
    uint32               indirect_routes;
    uint32               static_routes;
    uint32               host_routes;
    uint32               ecmp_routes;
    uint32               ecmp_member_maximum;
    uint32               indirect_v6_routes;
    uint32               static_v6_routes;
    uint32               host_v6_routes;
    uint32               ecmp_v6_routes;
    uint32               ecmp_v6_member_maximum;
    uint32               pbr_entries;
    uint32               l2mc_entries;
    uint32               l3mc_entries;
    uint32               l2mc_member_ports;
    uint32               l3mc_member_ports;
    uint32               vlan_member_ports;
    uint32               voice_vlan;
    uint32               flow_entries_ingress;
    uint32               flow_entries_egress;
    uint32               flow6_entries_ingress;
    uint32               flow6_entries_egress;
    uint32               cfm_local_and_remote_meps;
    uint32               g8031_groups;
    uint32               g8032_rings;
    uint32               g8032_member_ports_per_ring;
    uint32               ftn_entries;
    uint32               ilm_entries;
    uint32               mpls_lables;
    uint32               vpws;
    uint32               lsp_pe;
    uint32               lsp_p;
    uint32               vpls_peer;
    uint32               vpls_ac;
    uint32               vsi;
    uint32               lsp_oam;
    uint32               pw_oam;
    uint32               mpls_aps_tunnel;
    uint32               cfm_lck;
    uint32               ip_tunnel;
    uint32               ivi_peers;
    uint32               pbr_v6_entries;
    uint32               l3mc_v6_entries;
    uint32               l2mc_v6_entries;
    uint32               flow_v6_entries;
    uint32               ipv6_based_vlan_class;
    uint32               ipv6_source_guard;
    uint32               l3mc_v6_member_ports;
    uint32               l2mc_v6_member_ports;
    uint32               vlan_mapping_default_entry;
    uint32               bfd_sessions;
    uint32               nvgre_tunnel_id_mappings;
    uint32               nvgre_peers;
    uint32               nvgre_tunnels;
    uint32               openflow_flow_entries;
    uint32               policer_profile_num;
    uint32               port_shape_profile_num;
    uint32               queue_shape_profile_num;
    uint32               drop_profile_num;
    uint32               lag_group_num;
    uint32               ospf_area_range_num;
    uint32               tap_group_profile_num;
    uint32               tap_flow_num;
    uint32               tap_flow_entry_num;
    uint32               l2protocol_num;
    uint32               openflow_dynamic_tunnel_num;
    uint32               nat_pool_num;
    uint32               nat_rule_num;
    uint32               nat_inside_rtif_num;
    uint32               nat_outside_rtif_num;
    uint32               nat_session_upstream_num;
    uint32               nat_session_dnstream_num;
    uint32               nat_session_fast_path_entry_num;
    uint32               nat_session_limit_node_num;
} tbl_sys_spec_t;

typedef struct
{
    tbl_sys_spec_t       *sys_spec_array[GLB_STM_MODE_MAX];
} tbl_sys_spec_master_t;

#endif /* !__TBL_SYS_SPEC_DEFINE_H__ */

