
#ifndef __CDB_TBL_FIELD_PRIV_H__
#define __CDB_TBL_FIELD_PRIV_H__

static sdb_field_t stbl_sai_glb_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nh_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nh_group_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vrf_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "rif_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "route_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "stp_id_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nh_group_curr_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "hostif_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "hostif_group_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vrrp_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlan_class_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "hash_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "lag_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "qos_wred_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "qos_map_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "qos_scheduler_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "qos_policer_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "default_stp_id" },
    { SDB_ACCESS_RD,  SDB_DT_VOID,       -1,                  "hostif_fd_hash" },
    { SDB_ACCESS_RD,  SDB_DT_VOID,       -1,                  "hostif_portif_hash" },
    { SDB_ACCESS_RD,  SDB_DT_VOID,       -1,                  "hostif_vlanif_hash" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "l2mc_group_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "isolation_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "mirror_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nexthop_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nexthop_group_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "stpid_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "tap_group_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vlan_class_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vrf_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "hash_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "lag_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "qos_wred_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "qos_map_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "qos_scheduler_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "qos_policer_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "udf_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "copp_entry_opb" },
};

static sdb_field_t stbl_port_glb_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "group_id_igs_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "group_id_egs_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "ttp_group_id_igs_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_class_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "hostif_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "hostif_group_id_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "route_if_opb" },
};

static sdb_field_t stbl_port_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "flag" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "gport" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "lag_id" },
    { SDB_ACCESS_RD,  SDB_DT_INT,        -1,                  "timestamp_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "timestamp_nexthopid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_id_igs" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_id_egs" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ttp_group_id_igs" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "port_class_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vlan_bmp" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlanclass_group_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlanclass_default_action" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlanclass_active_type" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "isolation_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "isolation_dir" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "isolation_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "tag_ctrl" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "dot1q_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "transmit_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "receive_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "keep_vlan_tag" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ingress_vlan_filter" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "egress_vlan_filter" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "src_discard" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "src_outer_is_svlan" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "lag_bind_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "flowctl_dir" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "flowctl_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "type_is_access" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "metadata_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "egress_acl_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "reflective_bridge_en" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "dhcprelay_exception" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "mac_limit_num" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "mac_limit_action" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "port_state" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "rx_stats_ptr" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "tx_stats_ptr" },
};

static sdb_field_t stbl_hostif_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT64,     -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vid" },
    { SDB_ACCESS_RD,  SDB_DT_INT,        -1,                  "fd" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nexthop_ptr" },
    { SDB_ACCESS_RD,  SDB_DT_STRING,     -1,                  "ifname" },
};

static sdb_field_t stbl_lag_memport_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "tid" },
};

static sdb_field_t stbl_vr_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT64,     -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vr_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "smac" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "v4_admin" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "v6_admin" },
    { SDB_ACCESS_RD,  SDB_DT_REFERLIST,  -1,                  "rif_list" },
};

static sdb_field_t stbl_rif_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT64,     -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "type" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vr_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlan_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "smac" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nat_enable" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nat_category_id" },
};

static sdb_field_t stbl_nexthop_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nh_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nh_type" },
};

static sdb_field_t stbl_nexthop_group_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ipmc_group_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ecmp_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "failover_en" },
    { SDB_ACCESS_RD,  SDB_DT_REFERLIST,  -1,                  "nh_list" },
};

static sdb_field_t stbl_neighbor_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "mac" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nh_oid" },
    { SDB_ACCESS_RD,  SDB_DT_ENUM32,     -1,                  "action" },
};

static sdb_field_t stbl_vlan_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "learn_disable" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "stp_oid" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "stats_id_l2_in" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "stats_id_l2_out" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "stats_id_l3_in" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "stats_id_l3_out" },
};

static sdb_field_t stbl_vlan_class_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "group_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "member_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "entry_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "proto_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "vlan_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "ipaddr" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "mac" },
};

static sdb_field_t stbl_stp_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "stp_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vlan_bmp" },
};

static sdb_field_t stbl_l2mc_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_id" },
};

static sdb_field_t stbl_route_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "rif_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nh_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "priority" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "action" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "l3if_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "category_id" },
};

static sdb_field_t stbl_vrrp_vip_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vmac" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "if_mac" },
};

static sdb_field_t stbl_copp_trap_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "group_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ctc_reason_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "channel" },
};

static sdb_field_t stbl_copp_group_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "police_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "police_ssh_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "police_telnet_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "police_mlag_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "queue_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ctc_reason_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "channel" },
};

static sdb_field_t stbl_copp_reason_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "channel" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "trap_id" },
};

static sdb_field_t stbl_copp_entry_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "entry_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "counter_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nh_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ip_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "protocol_num" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "dst_port" },
};

static sdb_field_t stbl_acl_glb_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "acl_tbl_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "acl_entry_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "sdk_acl_entry_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "tunnel_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "erspan_service_opb_max_count" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "acl_tbl_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "acl_entry_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "sdk_acl_entry_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "tunnel_opb" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "erspan_service_opb" },
};

static sdb_field_t stbl_acl_port_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT64,     -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ingress_en_cnt" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "egress_en_cnt" },
};

static sdb_field_t stbl_acl_tbl_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "priority" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "field_bitmap" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "stage" },
};

static sdb_field_t stbl_acl_entry_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "tbl_oid" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "mirror_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "priority" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "dirction" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "enable" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "apply_inner_lookup" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "in_port" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "ctc_entry_id_list" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "ctc_entry_id_count" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "port_oid_list" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "port_oid_count" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "outer_fields" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "inner_fields" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "action" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "action_attr" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "tap" },
};

static sdb_field_t stbl_acl_tunnel_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_INT,        -1,                  "count" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "entry_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "erspan_service_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "erspan_service_group_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "erspan_nexthop_id" },
};

static sdb_field_t stbl_acl_udf_entry_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_length" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "offset_type" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "offset_num" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "offset" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "installed" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key_flags" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "ethtype" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "vlannum" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "ipproto" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "l4_type" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "srcport" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "dstport" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "mplsnum" },
};

static sdb_field_t stbl_ipmc_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "group_id" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nexthop_id" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "counter_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "nexthop_group_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "rpf_router_interface" },
};

static sdb_field_t stbl_nat_forward_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "key" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "iif_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "oif_oid" },
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "nhid" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "use_cnt" },
};

static sdb_field_t stbl_qos_mpls_domain_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "exp_domain" },
};

static sdb_field_t stbl_qos_mpls_flow_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "mpls_flow_en" },
};

static sdb_field_t stbl_cfm_lmep_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "gport" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "vlan_id" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "md_level" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "lmep_num" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "mep_id" },
    { SDB_ACCESS_RW,  SDB_DT_COMPOSE,    -1,                  "maid" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "direction" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "d_unexp_mep" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "d_mismerge" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "d_meg_lvl" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "cci_enabled" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "cci_interval" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "rdi" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "ccm_vlan_cos" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "ifindex" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "mode" },
};

static sdb_field_t stbl_cfm_rmep_fields[] = 
{
    { SDB_ACCESS_RD,  SDB_DT_UINT,       -1,                  "key" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "gport" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "vlan_id" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "md_level" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "rmep_id" },
    { SDB_ACCESS_RD,  SDB_DT_COMPOSE,    -1,                  "rmep_mac" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "ifindex" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "first_pkt_rx" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "d_loc" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "d_unexp_period" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "ma_sa_mismatch" },
    { SDB_ACCESS_RW,  SDB_DT_UINT,       -1,                  "rmep_last_rdi" },
};

#endif /* !__CDB_TBL_FIELD_PRIV_H__ */

