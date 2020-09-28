
#ifndef __CDB_DS_FIELD_PRIV_H__
#define __CDB_DS_FIELD_PRIV_H__

static cdb_field_t ds_brgif_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pvid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_native_vid" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "qinq_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "qinq_default_fwd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_translation_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_trans_default_fwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "vlan_mapping_table" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1q_tag_native_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "allowed_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ingress_filter" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "egress_filter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tpid_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_learning_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oper_mac_learning_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_security_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "violate_mode" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_is_peer_link" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_is_port_block" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_is_group" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "untagged_vlan" },
};

static cdb_field_t ds_aclqos_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "default_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "domain" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trust" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "replace" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "shape_pir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "queue_stats_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "input_policy_map" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "output_policy_map" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "input_policer" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "output_policer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_shape_profile" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_profile" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_class" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_weight" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_mode" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_ecn_enable" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "queue_transimt_pkt" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "queue_transimt_byte" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "queue_drop_pkt" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "queue_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue0_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue0_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue0_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue0_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue1_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue1_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue1_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue1_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue2_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue2_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue2_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue2_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue3_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue3_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue3_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue3_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue4_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue4_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue4_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue4_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue5_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue5_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue5_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue5_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue6_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue6_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue6_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue6_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue7_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue7_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue7_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "queue7_drop_byte" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "phb_enable" },
};

static cdb_field_t ds_lag_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "mode" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lag_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mismatch" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "bundle_ports_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "load_balance_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_active_backup_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_active_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cur_active_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cur_active_max_priority" },
    { CDB_ACCESS_RD,  CDB_DT_REFERLIST,  -1,                  "member_ports" },
};

static cdb_field_t ds_lacp_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "actor_port_priority" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "actor_oper_port_key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "actor_port_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "short_timeout" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "actor_oper_port_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_admin_system_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "partner_admin_system" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_admin_key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_admin_port_priority" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_admin_port_number" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_admin_port_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_oper_system_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "partner_oper_system" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_oper_key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_oper_port_priority" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_oper_port_number" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "partner_oper_port_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tx_lacp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tx_error_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_lacp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_error_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ntt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lacp_enabled" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ready_n" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_moved" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "periodic_tx_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "receive_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "mux_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "selected" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "pdu" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "current_while_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "periodic_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "wait_while_timer" },
};

static cdb_field_t ds_connected_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "destination" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "flags_secondary" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "flags_anycast" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "flags_virtual" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "flags_duplicate" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "installed" },
};

static cdb_field_t ds_connected_v6_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "destination" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_secondary" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_anycast" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_virtual" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_duplicate" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_global" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_linklocal" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_auto_linklocal" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_global_eui64" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_dad_in_progress" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_na_in_progress" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_configured" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_v6_real" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "installed" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dad_ns_send" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "unsolicited_na" },
};

static cdb_field_t ds_static_route_dup_ipv4_nh_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t ds_ospf_auth_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ospf_auth_md5_key" },
};

static cdb_field_t ds_storm_control_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bcast_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mcast_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ucast_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bcast_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mcast_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ucast_rate" },
};

static cdb_field_t ds_openflow_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "obey_vlan_filter" },
};

static cdb_field_t ds_dhclient_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "requested_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lease" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "option" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "client_flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "client_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "class_flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "class_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "hostname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "iana_t1" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "iana_t2" },
};

static cdb_field_t ds_pvlan_community_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_id" },
    { CDB_ACCESS_RW,  CDB_DT_REFERLIST,  -1,                  "ports" },
};

static cdb_field_t ds_circuit_id_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "circuit" },
};

static cdb_field_t ds_flush_fdb_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "subtype" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac" },
};

static cdb_field_t ds_lldp_neighbour_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_ttl" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "chassis_id_sub_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_id_sub_type" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "system_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "system_desc" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "port_desc" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "vlan_name" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vlan_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sys_cap" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sys_cap_en" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "mant_addr" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "oid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oid_len" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mgmt_addr_sub_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "if_numbering" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "if_number" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pvid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ppvid_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ppvid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "protocol_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "autonego_support" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "autonego_cap" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_mau_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "link_aggr_status" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "link_aggr_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_frame_size" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "power_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "power_pair" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "power_class" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_capbility" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_dev_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_policy_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_power_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_power_value" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_lci_format" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_hard_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_firm_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_soft_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_serial_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_manufac_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_moname_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_asset_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_lci_coordinate" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "med_lci_elin" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "med_lci_civic" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rx_ttl_timer" },
};

static cdb_field_t ds_mrouter_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "uptime" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "expire_timer" },
};

static cdb_field_t ds_query_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_resp_time" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "response_timer" },
};

static cdb_field_t ds_l2protocol_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_standard_pro" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2pro_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2pro_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "l2pro_evc_name" },
};

static cdb_field_t ds_nd_prefix_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "config" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlifetime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "plifetime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "onlink" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "autoconfig" },
};

static cdb_field_t ds_join_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "type" },
};

static cdb_field_t ds_cfm_error_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "level" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "md_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mep_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "err_reason" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rmep_mac" },
};

#endif /* !__CDB_DS_FIELD_PRIV_H__ */

