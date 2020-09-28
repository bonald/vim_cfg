
#ifndef __CDB_TBL_INFO_PRIV_H__
#define __CDB_TBL_INFO_PRIV_H__

static cdb_tbl_info_t g_cdb_tbl_info[TBL_MAX] =
{
    {
        sizeof(tbl_interface_key_t),
        sizeof(tbl_interface_t),
        sizeof(tbl_interface_fields)/sizeof(cdb_field_t),
        tbl_interface_fields,
        {
            tbl_interface_iterate,
            (TBL_DUMP_FUNC)tbl_interface_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_interface_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_interface_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_interface_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_interface_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_interface_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_interface_table_dump,
            (TBL_PARSER_FUNC)tbl_interface_table_parser,
        }
    },
    {
        sizeof(tbl_route_if_key_t),
        sizeof(tbl_route_if_t),
        sizeof(tbl_route_if_fields)/sizeof(cdb_field_t),
        tbl_route_if_fields,
        {
            tbl_route_if_iterate,
            (TBL_DUMP_FUNC)tbl_route_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_route_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_route_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_route_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_route_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_route_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_route_if_table_dump,
            (TBL_PARSER_FUNC)tbl_route_if_table_parser,
        }
    },
    {
        sizeof(tbl_kernel_if_key_t),
        sizeof(tbl_kernel_if_t),
        sizeof(tbl_kernel_if_fields)/sizeof(cdb_field_t),
        tbl_kernel_if_fields,
        {
            tbl_kernel_if_iterate,
            (TBL_DUMP_FUNC)tbl_kernel_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_kernel_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_kernel_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_kernel_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_kernel_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_kernel_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_kernel_if_table_dump,
            (TBL_PARSER_FUNC)tbl_kernel_if_table_parser,
        }
    },
    {
        sizeof(tbl_fea_port_if_key_t),
        sizeof(tbl_fea_port_if_t),
        sizeof(tbl_fea_port_if_fields)/sizeof(cdb_field_t),
        tbl_fea_port_if_fields,
        {
            tbl_fea_port_if_iterate,
            (TBL_DUMP_FUNC)tbl_fea_port_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_port_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_port_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_port_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_port_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_port_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_port_if_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_port_if_table_parser,
        }
    },
    {
        sizeof(tbl_vlan_key_t),
        sizeof(tbl_vlan_t),
        sizeof(tbl_vlan_fields)/sizeof(cdb_field_t),
        tbl_vlan_fields,
        {
            tbl_vlan_iterate,
            (TBL_DUMP_FUNC)tbl_vlan_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlan_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlan_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlan_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlan_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlan_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlan_table_dump,
            (TBL_PARSER_FUNC)tbl_vlan_table_parser,
        }
    },
    {
        sizeof(tbl_pvlan_key_t),
        sizeof(tbl_pvlan_t),
        sizeof(tbl_pvlan_fields)/sizeof(cdb_field_t),
        tbl_pvlan_fields,
        {
            tbl_pvlan_iterate,
            (TBL_DUMP_FUNC)tbl_pvlan_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pvlan_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pvlan_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pvlan_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pvlan_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pvlan_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pvlan_table_dump,
            (TBL_PARSER_FUNC)tbl_pvlan_table_parser,
        }
    },
    {
        sizeof(tbl_fdb_key_t),
        sizeof(tbl_fdb_t),
        sizeof(tbl_fdb_fields)/sizeof(cdb_field_t),
        tbl_fdb_fields,
        {
            tbl_fdb_iterate,
            (TBL_DUMP_FUNC)tbl_fdb_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fdb_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fdb_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fdb_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fdb_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fdb_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fdb_table_dump,
            (TBL_PARSER_FUNC)tbl_fdb_table_parser,
        }
    },
    {
        sizeof(tbl_mcfdb_key_t),
        sizeof(tbl_mcfdb_t),
        sizeof(tbl_mcfdb_fields)/sizeof(cdb_field_t),
        tbl_mcfdb_fields,
        {
            tbl_mcfdb_iterate,
            (TBL_DUMP_FUNC)tbl_mcfdb_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mcfdb_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mcfdb_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mcfdb_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mcfdb_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mcfdb_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mcfdb_table_dump,
            (TBL_PARSER_FUNC)tbl_mcfdb_table_parser,
        }
    },
    {
        sizeof(tbl_macfilter_key_t),
        sizeof(tbl_macfilter_t),
        sizeof(tbl_macfilter_fields)/sizeof(cdb_field_t),
        tbl_macfilter_fields,
        {
            tbl_macfilter_iterate,
            (TBL_DUMP_FUNC)tbl_macfilter_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_macfilter_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_macfilter_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_macfilter_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_macfilter_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_macfilter_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_macfilter_table_dump,
            (TBL_PARSER_FUNC)tbl_macfilter_table_parser,
        }
    },
    {
        sizeof(tbl_psfdb_key_t),
        sizeof(tbl_psfdb_t),
        sizeof(tbl_psfdb_fields)/sizeof(cdb_field_t),
        tbl_psfdb_fields,
        {
            tbl_psfdb_iterate,
            (TBL_DUMP_FUNC)tbl_psfdb_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_psfdb_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_psfdb_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_psfdb_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_psfdb_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_psfdb_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_psfdb_table_dump,
            (TBL_PARSER_FUNC)tbl_psfdb_table_parser,
        }
    },
    {
        sizeof(tbl_ipsg_s_ip_key_t),
        sizeof(tbl_ipsg_s_ip_t),
        sizeof(tbl_ipsg_s_ip_fields)/sizeof(cdb_field_t),
        tbl_ipsg_s_ip_fields,
        {
            tbl_ipsg_s_ip_iterate,
            (TBL_DUMP_FUNC)tbl_ipsg_s_ip_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsg_s_ip_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsg_s_ip_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsg_s_ip_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsg_s_ip_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsg_s_ip_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsg_s_ip_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsg_s_ip_table_parser,
        }
    },
    {
        sizeof(tbl_ipsg_s_mac_key_t),
        sizeof(tbl_ipsg_s_mac_t),
        sizeof(tbl_ipsg_s_mac_fields)/sizeof(cdb_field_t),
        tbl_ipsg_s_mac_fields,
        {
            tbl_ipsg_s_mac_iterate,
            (TBL_DUMP_FUNC)tbl_ipsg_s_mac_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsg_s_mac_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsg_s_mac_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsg_s_mac_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsg_s_mac_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsg_s_mac_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsg_s_mac_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsg_s_mac_table_parser,
        }
    },
    {
        sizeof(tbl_ipsg_fib_key_t),
        sizeof(tbl_ipsg_fib_t),
        sizeof(tbl_ipsg_fib_fields)/sizeof(cdb_field_t),
        tbl_ipsg_fib_fields,
        {
            tbl_ipsg_fib_iterate,
            (TBL_DUMP_FUNC)tbl_ipsg_fib_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsg_fib_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsg_fib_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsg_fib_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsg_fib_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsg_fib_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsg_fib_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsg_fib_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_brg_global_t),
        sizeof(tbl_brg_global_fields)/sizeof(cdb_field_t),
        tbl_brg_global_fields,
        {
            tbl_brg_global_iterate,
            (TBL_DUMP_FUNC)tbl_brg_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_brg_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_brg_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_brg_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_brg_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_brg_global_table_dump,
            (TBL_PARSER_FUNC)tbl_brg_global_table_parser,
        }
    },
    {
        sizeof(tbl_mstp_port_key_t),
        sizeof(tbl_mstp_port_t),
        sizeof(tbl_mstp_port_fields)/sizeof(cdb_field_t),
        tbl_mstp_port_fields,
        {
            tbl_mstp_port_iterate,
            (TBL_DUMP_FUNC)tbl_mstp_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mstp_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mstp_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mstp_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mstp_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mstp_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mstp_port_table_dump,
            (TBL_PARSER_FUNC)tbl_mstp_port_table_parser,
        }
    },
    {
        sizeof(tbl_msti_port_key_t),
        sizeof(tbl_msti_port_t),
        sizeof(tbl_msti_port_fields)/sizeof(cdb_field_t),
        tbl_msti_port_fields,
        {
            tbl_msti_port_iterate,
            (TBL_DUMP_FUNC)tbl_msti_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_msti_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_msti_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_msti_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_msti_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_msti_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_msti_port_table_dump,
            (TBL_PARSER_FUNC)tbl_msti_port_table_parser,
        }
    },
    {
        sizeof(tbl_mstp_instance_key_t),
        sizeof(tbl_mstp_instance_t),
        sizeof(tbl_mstp_instance_fields)/sizeof(cdb_field_t),
        tbl_mstp_instance_fields,
        {
            tbl_mstp_instance_iterate,
            (TBL_DUMP_FUNC)tbl_mstp_instance_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mstp_instance_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mstp_instance_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mstp_instance_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mstp_instance_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mstp_instance_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mstp_instance_table_dump,
            (TBL_PARSER_FUNC)tbl_mstp_instance_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_mstp_global_t),
        sizeof(tbl_mstp_global_fields)/sizeof(cdb_field_t),
        tbl_mstp_global_fields,
        {
            tbl_mstp_global_iterate,
            (TBL_DUMP_FUNC)tbl_mstp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mstp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mstp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mstp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mstp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mstp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_mstp_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_lldp_global_t),
        sizeof(tbl_lldp_global_fields)/sizeof(cdb_field_t),
        tbl_lldp_global_fields,
        {
            tbl_lldp_global_iterate,
            (TBL_DUMP_FUNC)tbl_lldp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lldp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lldp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lldp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lldp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lldp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_lldp_global_table_parser,
        }
    },
    {
        sizeof(tbl_lldp_if_key_t),
        sizeof(tbl_lldp_if_t),
        sizeof(tbl_lldp_if_fields)/sizeof(cdb_field_t),
        tbl_lldp_if_fields,
        {
            tbl_lldp_if_iterate,
            (TBL_DUMP_FUNC)tbl_lldp_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_lldp_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lldp_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lldp_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lldp_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lldp_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lldp_if_table_dump,
            (TBL_PARSER_FUNC)tbl_lldp_if_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_mlag_t),
        sizeof(tbl_mlag_fields)/sizeof(cdb_field_t),
        tbl_mlag_fields,
        {
            tbl_mlag_iterate,
            (TBL_DUMP_FUNC)tbl_mlag_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mlag_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mlag_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mlag_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mlag_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mlag_table_dump,
            (TBL_PARSER_FUNC)tbl_mlag_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_mlag_peer_t),
        sizeof(tbl_mlag_peer_fields)/sizeof(cdb_field_t),
        tbl_mlag_peer_fields,
        {
            tbl_mlag_peer_iterate,
            (TBL_DUMP_FUNC)tbl_mlag_peer_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mlag_peer_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mlag_peer_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mlag_peer_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mlag_peer_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mlag_peer_table_dump,
            (TBL_PARSER_FUNC)tbl_mlag_peer_table_parser,
        }
    },
    {
        sizeof(tbl_mlag_port_key_t),
        sizeof(tbl_mlag_port_t),
        sizeof(tbl_mlag_port_fields)/sizeof(cdb_field_t),
        tbl_mlag_port_fields,
        {
            tbl_mlag_port_iterate,
            (TBL_DUMP_FUNC)tbl_mlag_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mlag_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mlag_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mlag_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mlag_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mlag_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mlag_port_table_dump,
            (TBL_PARSER_FUNC)tbl_mlag_port_table_parser,
        }
    },
    {
        sizeof(tbl_isolation_key_t),
        sizeof(tbl_isolation_t),
        sizeof(tbl_isolation_fields)/sizeof(cdb_field_t),
        tbl_isolation_fields,
        {
            tbl_isolation_iterate,
            (TBL_DUMP_FUNC)tbl_isolation_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_isolation_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_isolation_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_isolation_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_isolation_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_isolation_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_isolation_table_dump,
            (TBL_PARSER_FUNC)tbl_isolation_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_route_global_t),
        sizeof(tbl_route_global_fields)/sizeof(cdb_field_t),
        tbl_route_global_fields,
        {
            tbl_route_global_iterate,
            (TBL_DUMP_FUNC)tbl_route_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_route_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_route_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_route_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_route_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_route_global_table_dump,
            (TBL_PARSER_FUNC)tbl_route_global_table_parser,
        }
    },
    {
        sizeof(tbl_bgp_key_t),
        sizeof(tbl_bgp_t),
        sizeof(tbl_bgp_fields)/sizeof(cdb_field_t),
        tbl_bgp_fields,
        {
            tbl_bgp_iterate,
            (TBL_DUMP_FUNC)tbl_bgp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_bgp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bgp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bgp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bgp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bgp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bgp_table_dump,
            (TBL_PARSER_FUNC)tbl_bgp_table_parser,
        }
    },
    {
        sizeof(tbl_bgp_network_key_t),
        sizeof(tbl_bgp_network_t),
        sizeof(tbl_bgp_network_fields)/sizeof(cdb_field_t),
        tbl_bgp_network_fields,
        {
            tbl_bgp_network_iterate,
            (TBL_DUMP_FUNC)tbl_bgp_network_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_bgp_network_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bgp_network_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bgp_network_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bgp_network_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bgp_network_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bgp_network_table_dump,
            (TBL_PARSER_FUNC)tbl_bgp_network_table_parser,
        }
    },
    {
        sizeof(tbl_bgp_neighbor_key_t),
        sizeof(tbl_bgp_neighbor_t),
        sizeof(tbl_bgp_neighbor_fields)/sizeof(cdb_field_t),
        tbl_bgp_neighbor_fields,
        {
            tbl_bgp_neighbor_iterate,
            (TBL_DUMP_FUNC)tbl_bgp_neighbor_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_bgp_neighbor_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bgp_neighbor_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bgp_neighbor_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bgp_neighbor_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bgp_neighbor_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bgp_neighbor_table_dump,
            (TBL_PARSER_FUNC)tbl_bgp_neighbor_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_key_t),
        sizeof(tbl_ospf_t),
        sizeof(tbl_ospf_fields)/sizeof(cdb_field_t),
        tbl_ospf_fields,
        {
            tbl_ospf_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_table_parser,
        }
    },
    {
        sizeof(tbl_ospf6_key_t),
        sizeof(tbl_ospf6_t),
        sizeof(tbl_ospf6_fields)/sizeof(cdb_field_t),
        tbl_ospf6_fields,
        {
            tbl_ospf6_iterate,
            (TBL_DUMP_FUNC)tbl_ospf6_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf6_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf6_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf6_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf6_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf6_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf6_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf6_table_parser,
        }
    },
    {
        sizeof(tbl_ospf6_interface_key_t),
        sizeof(tbl_ospf6_interface_t),
        sizeof(tbl_ospf6_interface_fields)/sizeof(cdb_field_t),
        tbl_ospf6_interface_fields,
        {
            tbl_ospf6_interface_iterate,
            (TBL_DUMP_FUNC)tbl_ospf6_interface_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf6_interface_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf6_interface_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf6_interface_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf6_interface_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf6_interface_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf6_interface_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf6_interface_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_network_key_t),
        sizeof(tbl_ospf_network_t),
        sizeof(tbl_ospf_network_fields)/sizeof(cdb_field_t),
        tbl_ospf_network_fields,
        {
            tbl_ospf_network_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_network_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_network_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_network_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_network_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_network_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_network_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_network_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_network_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_area_auth_key_t),
        sizeof(tbl_ospf_area_auth_t),
        sizeof(tbl_ospf_area_auth_fields)/sizeof(cdb_field_t),
        tbl_ospf_area_auth_fields,
        {
            tbl_ospf_area_auth_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_area_auth_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_area_auth_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_area_auth_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_area_auth_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_area_auth_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_area_auth_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_area_auth_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_area_auth_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_area_range_key_t),
        sizeof(tbl_ospf_area_range_t),
        sizeof(tbl_ospf_area_range_fields)/sizeof(cdb_field_t),
        tbl_ospf_area_range_fields,
        {
            tbl_ospf_area_range_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_area_range_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_area_range_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_area_range_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_area_range_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_area_range_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_area_range_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_area_range_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_area_range_table_parser,
        }
    },
    {
        sizeof(tbl_ospf6_area_range_key_t),
        sizeof(tbl_ospf6_area_range_t),
        sizeof(tbl_ospf6_area_range_fields)/sizeof(cdb_field_t),
        tbl_ospf6_area_range_fields,
        {
            tbl_ospf6_area_range_iterate,
            (TBL_DUMP_FUNC)tbl_ospf6_area_range_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf6_area_range_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf6_area_range_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf6_area_range_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf6_area_range_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf6_area_range_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf6_area_range_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf6_area_range_table_parser,
        }
    },
    {
        sizeof(nexthop_key_t),
        sizeof(tbl_iproute_node_t),
        sizeof(tbl_iproute_node_fields)/sizeof(cdb_field_t),
        tbl_iproute_node_fields,
        {
            tbl_iproute_node_iterate,
            (TBL_DUMP_FUNC)tbl_iproute_node_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_iproute_node_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_iproute_node_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_iproute_node_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_iproute_node_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_iproute_node_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_iproute_node_table_dump,
            (TBL_PARSER_FUNC)tbl_iproute_node_table_parser,
        }
    },
    {
        sizeof(nexthop_key_t),
        sizeof(tbl_iproute_node_add_fail_t),
        sizeof(tbl_iproute_node_add_fail_fields)/sizeof(cdb_field_t),
        tbl_iproute_node_add_fail_fields,
        {
            tbl_iproute_node_add_fail_iterate,
            (TBL_DUMP_FUNC)tbl_iproute_node_add_fail_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_iproute_node_add_fail_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_iproute_node_add_fail_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_iproute_node_add_fail_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_iproute_node_add_fail_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_iproute_node_add_fail_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_iproute_node_add_fail_table_dump,
            (TBL_PARSER_FUNC)tbl_iproute_node_add_fail_table_parser,
        }
    },
    {
        sizeof(nexthop_key_t),
        sizeof(tbl_iproute_node_add_fail_count_t),
        sizeof(tbl_iproute_node_add_fail_count_fields)/sizeof(cdb_field_t),
        tbl_iproute_node_add_fail_count_fields,
        {
            tbl_iproute_node_add_fail_count_iterate,
            (TBL_DUMP_FUNC)tbl_iproute_node_add_fail_count_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_iproute_node_add_fail_count_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_iproute_node_add_fail_count_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_iproute_node_add_fail_count_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_iproute_node_add_fail_count_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_iproute_node_add_fail_count_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_iproute_node_add_fail_count_table_dump,
            (TBL_PARSER_FUNC)tbl_iproute_node_add_fail_count_table_parser,
        }
    },
    {
        sizeof(tbl_static_route_cfg_key_t),
        sizeof(tbl_static_route_cfg_t),
        sizeof(tbl_static_route_cfg_fields)/sizeof(cdb_field_t),
        tbl_static_route_cfg_fields,
        {
            tbl_static_route_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_static_route_cfg_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_static_route_cfg_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_static_route_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_static_route_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_static_route_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_static_route_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_static_route_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_static_route_cfg_table_parser,
        }
    },
    {
        sizeof(tbl_static_rt_cnt_key_t),
        sizeof(tbl_static_rt_cnt_t),
        sizeof(tbl_static_rt_cnt_fields)/sizeof(cdb_field_t),
        tbl_static_rt_cnt_fields,
        {
            tbl_static_rt_cnt_iterate,
            (TBL_DUMP_FUNC)tbl_static_rt_cnt_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_static_rt_cnt_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_static_rt_cnt_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_static_rt_cnt_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_static_rt_cnt_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_static_rt_cnt_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_static_rt_cnt_table_dump,
            (TBL_PARSER_FUNC)tbl_static_rt_cnt_table_parser,
        }
    },
    {
        sizeof(tbl_arp_fib_key_t),
        sizeof(tbl_arp_fib_t),
        sizeof(tbl_arp_fib_fields)/sizeof(cdb_field_t),
        tbl_arp_fib_fields,
        {
            tbl_arp_fib_iterate,
            (TBL_DUMP_FUNC)tbl_arp_fib_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_arp_fib_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_arp_fib_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_arp_fib_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_arp_fib_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_arp_fib_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_arp_fib_table_dump,
            (TBL_PARSER_FUNC)tbl_arp_fib_table_parser,
        }
    },
    {
        sizeof(tbl_arp_key_t),
        sizeof(tbl_arp_t),
        sizeof(tbl_arp_fields)/sizeof(cdb_field_t),
        tbl_arp_fields,
        {
            tbl_arp_iterate,
            (TBL_DUMP_FUNC)tbl_arp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_arp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_arp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_arp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_arp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_arp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_arp_table_dump,
            (TBL_PARSER_FUNC)tbl_arp_table_parser,
        }
    },
    {
        sizeof(tbl_nexthop_key_t),
        sizeof(tbl_nexthop_t),
        sizeof(tbl_nexthop_fields)/sizeof(cdb_field_t),
        tbl_nexthop_fields,
        {
            tbl_nexthop_iterate,
            (TBL_DUMP_FUNC)tbl_nexthop_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nexthop_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nexthop_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nexthop_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nexthop_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nexthop_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nexthop_table_dump,
            (TBL_PARSER_FUNC)tbl_nexthop_table_parser,
        }
    },
    {
        sizeof(tbl_nexthop_group_key_t),
        sizeof(tbl_nexthop_group_t),
        sizeof(tbl_nexthop_group_fields)/sizeof(cdb_field_t),
        tbl_nexthop_group_fields,
        {
            tbl_nexthop_group_iterate,
            (TBL_DUMP_FUNC)tbl_nexthop_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nexthop_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nexthop_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nexthop_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nexthop_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nexthop_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nexthop_group_table_dump,
            (TBL_PARSER_FUNC)tbl_nexthop_group_table_parser,
        }
    },
    {
        sizeof(tbl_nexthop_group_routed_key_t),
        sizeof(tbl_nexthop_group_routed_t),
        sizeof(tbl_nexthop_group_routed_fields)/sizeof(cdb_field_t),
        tbl_nexthop_group_routed_fields,
        {
            tbl_nexthop_group_routed_iterate,
            (TBL_DUMP_FUNC)tbl_nexthop_group_routed_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nexthop_group_routed_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nexthop_group_routed_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nexthop_group_routed_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nexthop_group_routed_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nexthop_group_routed_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nexthop_group_routed_table_dump,
            (TBL_PARSER_FUNC)tbl_nexthop_group_routed_table_parser,
        }
    },
    {
        sizeof(tbl_fea_nexthop_key_t),
        sizeof(tbl_fea_nexthop_t),
        sizeof(tbl_fea_nexthop_fields)/sizeof(cdb_field_t),
        tbl_fea_nexthop_fields,
        {
            tbl_fea_nexthop_iterate,
            (TBL_DUMP_FUNC)tbl_fea_nexthop_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_nexthop_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_nexthop_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_nexthop_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_nexthop_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_nexthop_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_nexthop_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_nexthop_table_parser,
        }
    },
    {
        sizeof(tbl_mrt_key_t),
        sizeof(tbl_mrt_t),
        sizeof(tbl_mrt_fields)/sizeof(cdb_field_t),
        tbl_mrt_fields,
        {
            tbl_mrt_iterate,
            (TBL_DUMP_FUNC)tbl_mrt_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mrt_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mrt_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mrt_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mrt_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mrt_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mrt_table_dump,
            (TBL_PARSER_FUNC)tbl_mrt_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ipmc_global_t),
        sizeof(tbl_ipmc_global_fields)/sizeof(cdb_field_t),
        tbl_ipmc_global_fields,
        {
            tbl_ipmc_global_iterate,
            (TBL_DUMP_FUNC)tbl_ipmc_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipmc_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipmc_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipmc_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipmc_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipmc_global_table_dump,
            (TBL_PARSER_FUNC)tbl_ipmc_global_table_parser,
        }
    },
    {
        sizeof(tbl_ipmc_intf_key_t),
        sizeof(tbl_ipmc_intf_t),
        sizeof(tbl_ipmc_intf_fields)/sizeof(cdb_field_t),
        tbl_ipmc_intf_fields,
        {
            tbl_ipmc_intf_iterate,
            (TBL_DUMP_FUNC)tbl_ipmc_intf_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipmc_intf_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipmc_intf_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipmc_intf_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipmc_intf_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipmc_intf_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipmc_intf_table_dump,
            (TBL_PARSER_FUNC)tbl_ipmc_intf_table_parser,
        }
    },
    {
        sizeof(tbl_pim_static_rp_key_t),
        sizeof(tbl_pim_static_rp_t),
        sizeof(tbl_pim_static_rp_fields)/sizeof(cdb_field_t),
        tbl_pim_static_rp_fields,
        {
            tbl_pim_static_rp_iterate,
            (TBL_DUMP_FUNC)tbl_pim_static_rp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_static_rp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_static_rp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_static_rp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_static_rp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_static_rp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_static_rp_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_static_rp_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_sys_global_t),
        sizeof(tbl_sys_global_fields)/sizeof(cdb_field_t),
        tbl_sys_global_fields,
        {
            tbl_sys_global_iterate,
            (TBL_DUMP_FUNC)tbl_sys_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sys_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sys_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sys_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sys_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sys_global_table_dump,
            (TBL_PARSER_FUNC)tbl_sys_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_lag_global_t),
        sizeof(tbl_lag_global_fields)/sizeof(cdb_field_t),
        tbl_lag_global_fields,
        {
            tbl_lag_global_iterate,
            (TBL_DUMP_FUNC)tbl_lag_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lag_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lag_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lag_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lag_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lag_global_table_dump,
            (TBL_PARSER_FUNC)tbl_lag_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ecmp_global_t),
        sizeof(tbl_ecmp_global_fields)/sizeof(cdb_field_t),
        tbl_ecmp_global_fields,
        {
            tbl_ecmp_global_iterate,
            (TBL_DUMP_FUNC)tbl_ecmp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ecmp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ecmp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ecmp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ecmp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ecmp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_ecmp_global_table_parser,
        }
    },
    {
        sizeof(tbl_rr_prefix_key_t),
        sizeof(tbl_rr_prefix_t),
        sizeof(tbl_rr_prefix_fields)/sizeof(cdb_field_t),
        tbl_rr_prefix_fields,
        {
            tbl_rr_prefix_iterate,
            (TBL_DUMP_FUNC)tbl_rr_prefix_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rr_prefix_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rr_prefix_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rr_prefix_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rr_prefix_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rr_prefix_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rr_prefix_table_dump,
            (TBL_PARSER_FUNC)tbl_rr_prefix_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_mem_summary_t),
        sizeof(tbl_mem_summary_fields)/sizeof(cdb_field_t),
        tbl_mem_summary_fields,
        {
            tbl_mem_summary_iterate,
            (TBL_DUMP_FUNC)tbl_mem_summary_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mem_summary_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mem_summary_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mem_summary_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mem_summary_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mem_summary_table_dump,
            (TBL_PARSER_FUNC)tbl_mem_summary_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_cpu_mem_history_t),
        sizeof(tbl_cpu_mem_history_fields)/sizeof(cdb_field_t),
        tbl_cpu_mem_history_fields,
        {
            tbl_cpu_mem_history_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_mem_history_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_mem_history_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_mem_history_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_mem_history_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_mem_history_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_mem_history_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_mem_history_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_chsm_debug_t),
        sizeof(tbl_chsm_debug_fields)/sizeof(cdb_field_t),
        tbl_chsm_debug_fields,
        {
            tbl_chsm_debug_iterate,
            (TBL_DUMP_FUNC)tbl_chsm_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_chsm_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_chsm_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_chsm_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_chsm_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_chsm_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_chsm_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_switch_debug_t),
        sizeof(tbl_switch_debug_fields)/sizeof(cdb_field_t),
        tbl_switch_debug_fields,
        {
            tbl_switch_debug_iterate,
            (TBL_DUMP_FUNC)tbl_switch_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_switch_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_switch_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_switch_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_switch_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_switch_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_switch_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_route_debug_t),
        sizeof(tbl_route_debug_fields)/sizeof(cdb_field_t),
        tbl_route_debug_fields,
        {
            tbl_route_debug_iterate,
            (TBL_DUMP_FUNC)tbl_route_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_route_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_route_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_route_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_route_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_route_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_route_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ssm_debug_t),
        sizeof(tbl_ssm_debug_fields)/sizeof(cdb_field_t),
        tbl_ssm_debug_fields,
        {
            tbl_ssm_debug_iterate,
            (TBL_DUMP_FUNC)tbl_ssm_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ssm_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ssm_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ssm_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ssm_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ssm_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_ssm_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ptp_debug_t),
        sizeof(tbl_ptp_debug_fields)/sizeof(cdb_field_t),
        tbl_ptp_debug_fields,
        {
            tbl_ptp_debug_iterate,
            (TBL_DUMP_FUNC)tbl_ptp_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ptp_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ptp_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ptp_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ptp_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ptp_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_ptp_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_quagga_debug_t),
        sizeof(tbl_quagga_debug_fields)/sizeof(cdb_field_t),
        tbl_quagga_debug_fields,
        {
            tbl_quagga_debug_iterate,
            (TBL_DUMP_FUNC)tbl_quagga_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_quagga_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_quagga_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_quagga_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_quagga_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_quagga_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_quagga_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_lsrv_debug_t),
        sizeof(tbl_lsrv_debug_fields)/sizeof(cdb_field_t),
        tbl_lsrv_debug_fields,
        {
            tbl_lsrv_debug_iterate,
            (TBL_DUMP_FUNC)tbl_lsrv_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lsrv_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lsrv_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lsrv_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lsrv_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lsrv_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_lsrv_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_hsrv_debug_t),
        sizeof(tbl_hsrv_debug_fields)/sizeof(cdb_field_t),
        tbl_hsrv_debug_fields,
        {
            tbl_hsrv_debug_iterate,
            (TBL_DUMP_FUNC)tbl_hsrv_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_hsrv_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_hsrv_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_hsrv_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_hsrv_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_hsrv_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_hsrv_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_app_debug_t),
        sizeof(tbl_app_debug_fields)/sizeof(cdb_field_t),
        tbl_app_debug_fields,
        {
            tbl_app_debug_iterate,
            (TBL_DUMP_FUNC)tbl_app_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_app_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_app_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_app_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_app_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_app_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_app_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_openflow_debug_t),
        sizeof(tbl_openflow_debug_fields)/sizeof(cdb_field_t),
        tbl_openflow_debug_fields,
        {
            tbl_openflow_debug_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_debug_table_parser,
        }
    },
    {
        sizeof(tbl_rif_key_t),
        sizeof(tbl_rif_t),
        sizeof(tbl_rif_fields)/sizeof(cdb_field_t),
        tbl_rif_fields,
        {
            tbl_rif_iterate,
            (TBL_DUMP_FUNC)tbl_rif_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rif_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rif_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rif_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rif_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rif_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rif_table_dump,
            (TBL_PARSER_FUNC)tbl_rif_table_parser,
        }
    },
    {
        sizeof(tbl_fea_lag_key_t),
        sizeof(tbl_fea_lag_t),
        sizeof(tbl_fea_lag_fields)/sizeof(cdb_field_t),
        tbl_fea_lag_fields,
        {
            tbl_fea_lag_iterate,
            (TBL_DUMP_FUNC)tbl_fea_lag_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_lag_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_lag_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_lag_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_lag_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_lag_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_lag_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_lag_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_fea_global_t),
        sizeof(tbl_fea_global_fields)/sizeof(cdb_field_t),
        tbl_fea_global_fields,
        {
            tbl_fea_global_iterate,
            (TBL_DUMP_FUNC)tbl_fea_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_global_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_global_table_parser,
        }
    },
    {
        sizeof(tbl_fea_acl_table_key_t),
        sizeof(tbl_fea_acl_table_t),
        sizeof(tbl_fea_acl_table_fields)/sizeof(cdb_field_t),
        tbl_fea_acl_table_fields,
        {
            tbl_fea_acl_table_iterate,
            (TBL_DUMP_FUNC)tbl_fea_acl_table_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_acl_table_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_acl_table_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_acl_table_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_acl_table_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_acl_table_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_acl_table_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_acl_table_table_parser,
        }
    },
    {
        sizeof(tbl_fea_acl_key_t),
        sizeof(tbl_fea_acl_t),
        sizeof(tbl_fea_acl_fields)/sizeof(cdb_field_t),
        tbl_fea_acl_fields,
        {
            tbl_fea_acl_iterate,
            (TBL_DUMP_FUNC)tbl_fea_acl_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_acl_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_acl_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_acl_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_acl_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_acl_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_acl_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_acl_table_parser,
        }
    },
    {
        sizeof(tbl_fea_fdb_key_t),
        sizeof(tbl_fea_fdb_t),
        sizeof(tbl_fea_fdb_fields)/sizeof(cdb_field_t),
        tbl_fea_fdb_fields,
        {
            tbl_fea_fdb_iterate,
            (TBL_DUMP_FUNC)tbl_fea_fdb_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_fdb_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_fdb_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_fdb_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_fdb_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_fdb_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_fdb_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_fdb_table_parser,
        }
    },
    {
        sizeof(tbl_fea_brg_if_key_t),
        sizeof(tbl_fea_brg_if_t),
        sizeof(tbl_fea_brg_if_fields)/sizeof(cdb_field_t),
        tbl_fea_brg_if_fields,
        {
            tbl_fea_brg_if_iterate,
            (TBL_DUMP_FUNC)tbl_fea_brg_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_brg_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_brg_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_brg_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_brg_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_brg_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_brg_if_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_brg_if_table_parser,
        }
    },
    {
        sizeof(tbl_acl_worm_filter_key_t),
        sizeof(tbl_acl_worm_filter_t),
        sizeof(tbl_acl_worm_filter_fields)/sizeof(cdb_field_t),
        tbl_acl_worm_filter_fields,
        {
            tbl_acl_worm_filter_iterate,
            (TBL_DUMP_FUNC)tbl_acl_worm_filter_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_worm_filter_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_worm_filter_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_worm_filter_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_worm_filter_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_worm_filter_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_worm_filter_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_worm_filter_table_parser,
        }
    },
    {
        sizeof(tbl_acl_config_key_t),
        sizeof(tbl_acl_config_t),
        sizeof(tbl_acl_config_fields)/sizeof(cdb_field_t),
        tbl_acl_config_fields,
        {
            tbl_acl_config_iterate,
            (TBL_DUMP_FUNC)tbl_acl_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_config_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_config_table_parser,
        }
    },
    {
        sizeof(tbl_ace_config_key_t),
        sizeof(tbl_ace_config_t),
        sizeof(tbl_ace_config_fields)/sizeof(cdb_field_t),
        tbl_ace_config_fields,
        {
            tbl_ace_config_iterate,
            (TBL_DUMP_FUNC)tbl_ace_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ace_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ace_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ace_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ace_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ace_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ace_config_table_dump,
            (TBL_PARSER_FUNC)tbl_ace_config_table_parser,
        }
    },
    {
        sizeof(tbl_acl_entry_key_t),
        sizeof(tbl_acl_entry_t),
        sizeof(tbl_acl_entry_fields)/sizeof(cdb_field_t),
        tbl_acl_entry_fields,
        {
            tbl_acl_entry_iterate,
            (TBL_DUMP_FUNC)tbl_acl_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_entry_table_parser,
        }
    },
    {
        sizeof(tbl_acl_entry_action_key_t),
        sizeof(tbl_acl_entry_action_t),
        sizeof(tbl_acl_entry_action_fields)/sizeof(cdb_field_t),
        tbl_acl_entry_action_fields,
        {
            tbl_acl_entry_action_iterate,
            (TBL_DUMP_FUNC)tbl_acl_entry_action_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_entry_action_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_entry_action_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_entry_action_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_entry_action_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_entry_action_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_entry_action_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_entry_action_table_parser,
        }
    },
    {
        sizeof(tbl_acl_nexthop_group_key_t),
        sizeof(tbl_acl_nexthop_group_t),
        sizeof(tbl_acl_nexthop_group_fields)/sizeof(cdb_field_t),
        tbl_acl_nexthop_group_fields,
        {
            tbl_acl_nexthop_group_iterate,
            (TBL_DUMP_FUNC)tbl_acl_nexthop_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_nexthop_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_nexthop_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_nexthop_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_nexthop_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_nexthop_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_nexthop_group_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_nexthop_group_table_parser,
        }
    },
    {
        sizeof(tbl_acl_nexthop_key_t),
        sizeof(tbl_acl_nexthop_t),
        sizeof(tbl_acl_nexthop_fields)/sizeof(cdb_field_t),
        tbl_acl_nexthop_fields,
        {
            tbl_acl_nexthop_iterate,
            (TBL_DUMP_FUNC)tbl_acl_nexthop_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_nexthop_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_nexthop_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_nexthop_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_nexthop_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_nexthop_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_nexthop_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_nexthop_table_parser,
        }
    },
    {
        sizeof(tbl_time_range_key_t),
        sizeof(tbl_time_range_t),
        sizeof(tbl_time_range_fields)/sizeof(cdb_field_t),
        tbl_time_range_fields,
        {
            tbl_time_range_iterate,
            (TBL_DUMP_FUNC)tbl_time_range_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_time_range_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_time_range_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_time_range_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_time_range_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_time_range_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_time_range_table_dump,
            (TBL_PARSER_FUNC)tbl_time_range_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ssh_cfg_t),
        sizeof(tbl_ssh_cfg_fields)/sizeof(cdb_field_t),
        tbl_ssh_cfg_fields,
        {
            tbl_ssh_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_ssh_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ssh_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ssh_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ssh_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ssh_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ssh_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_ssh_cfg_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_snmp_cfg_t),
        sizeof(tbl_snmp_cfg_fields)/sizeof(cdb_field_t),
        tbl_snmp_cfg_fields,
        {
            tbl_snmp_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_cfg_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_view_key_t),
        sizeof(tbl_snmp_view_t),
        sizeof(tbl_snmp_view_fields)/sizeof(cdb_field_t),
        tbl_snmp_view_fields,
        {
            tbl_snmp_view_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_view_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_view_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_view_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_view_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_view_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_view_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_view_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_view_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_community_key_t),
        sizeof(tbl_snmp_community_t),
        sizeof(tbl_snmp_community_fields)/sizeof(cdb_field_t),
        tbl_snmp_community_fields,
        {
            tbl_snmp_community_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_community_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_community_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_community_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_community_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_community_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_community_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_community_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_community_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_context_key_t),
        sizeof(tbl_snmp_context_t),
        sizeof(tbl_snmp_context_fields)/sizeof(cdb_field_t),
        tbl_snmp_context_fields,
        {
            tbl_snmp_context_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_context_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_context_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_context_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_context_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_context_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_context_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_context_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_context_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_usm_user_key_t),
        sizeof(tbl_snmp_usm_user_t),
        sizeof(tbl_snmp_usm_user_fields)/sizeof(cdb_field_t),
        tbl_snmp_usm_user_fields,
        {
            tbl_snmp_usm_user_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_usm_user_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_usm_user_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_usm_user_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_usm_user_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_usm_user_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_usm_user_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_usm_user_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_usm_user_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_group_usm_key_t),
        sizeof(tbl_snmp_group_usm_t),
        sizeof(tbl_snmp_group_usm_fields)/sizeof(cdb_field_t),
        tbl_snmp_group_usm_fields,
        {
            tbl_snmp_group_usm_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_group_usm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_group_usm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_group_usm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_group_usm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_group_usm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_group_usm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_group_usm_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_group_usm_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_access_usm_key_t),
        sizeof(tbl_snmp_access_usm_t),
        sizeof(tbl_snmp_access_usm_fields)/sizeof(cdb_field_t),
        tbl_snmp_access_usm_fields,
        {
            tbl_snmp_access_usm_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_access_usm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_access_usm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_access_usm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_access_usm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_access_usm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_access_usm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_access_usm_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_access_usm_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_notify_key_t),
        sizeof(tbl_snmp_notify_t),
        sizeof(tbl_snmp_notify_fields)/sizeof(cdb_field_t),
        tbl_snmp_notify_fields,
        {
            tbl_snmp_notify_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_notify_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_notify_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_notify_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_notify_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_notify_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_notify_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_notify_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_notify_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_target_addr_key_t),
        sizeof(tbl_snmp_target_addr_t),
        sizeof(tbl_snmp_target_addr_fields)/sizeof(cdb_field_t),
        tbl_snmp_target_addr_fields,
        {
            tbl_snmp_target_addr_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_target_addr_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_target_addr_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_target_addr_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_target_addr_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_target_addr_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_target_addr_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_target_addr_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_target_addr_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_target_params_key_t),
        sizeof(tbl_snmp_target_params_t),
        sizeof(tbl_snmp_target_params_fields)/sizeof(cdb_field_t),
        tbl_snmp_target_params_fields,
        {
            tbl_snmp_target_params_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_target_params_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_target_params_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_target_params_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_target_params_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_target_params_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_target_params_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_target_params_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_target_params_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_trap_key_t),
        sizeof(tbl_snmp_trap_t),
        sizeof(tbl_snmp_trap_fields)/sizeof(cdb_field_t),
        tbl_snmp_trap_fields,
        {
            tbl_snmp_trap_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_trap_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_trap_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_trap_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_trap_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_trap_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_trap_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_trap_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_trap_table_parser,
        }
    },
    {
        sizeof(tbl_snmp_inform_key_t),
        sizeof(tbl_snmp_inform_t),
        sizeof(tbl_snmp_inform_fields)/sizeof(cdb_field_t),
        tbl_snmp_inform_fields,
        {
            tbl_snmp_inform_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_inform_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_snmp_inform_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_inform_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_inform_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_inform_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_inform_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_inform_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_inform_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_snmp_trap_msg_t),
        sizeof(tbl_snmp_trap_msg_fields)/sizeof(cdb_field_t),
        tbl_snmp_trap_msg_fields,
        {
            tbl_snmp_trap_msg_iterate,
            (TBL_DUMP_FUNC)tbl_snmp_trap_msg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_snmp_trap_msg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_snmp_trap_msg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_snmp_trap_msg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_snmp_trap_msg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_snmp_trap_msg_table_dump,
            (TBL_PARSER_FUNC)tbl_snmp_trap_msg_table_parser,
        }
    },
    {
        sizeof(tbl_rmon_event_key_t),
        sizeof(tbl_rmon_event_t),
        sizeof(tbl_rmon_event_fields)/sizeof(cdb_field_t),
        tbl_rmon_event_fields,
        {
            tbl_rmon_event_iterate,
            (TBL_DUMP_FUNC)tbl_rmon_event_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rmon_event_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rmon_event_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rmon_event_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rmon_event_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rmon_event_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rmon_event_table_dump,
            (TBL_PARSER_FUNC)tbl_rmon_event_table_parser,
        }
    },
    {
        sizeof(tbl_rmon_alarm_key_t),
        sizeof(tbl_rmon_alarm_t),
        sizeof(tbl_rmon_alarm_fields)/sizeof(cdb_field_t),
        tbl_rmon_alarm_fields,
        {
            tbl_rmon_alarm_iterate,
            (TBL_DUMP_FUNC)tbl_rmon_alarm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rmon_alarm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rmon_alarm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rmon_alarm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rmon_alarm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rmon_alarm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rmon_alarm_table_dump,
            (TBL_PARSER_FUNC)tbl_rmon_alarm_table_parser,
        }
    },
    {
        sizeof(tbl_rmon_ether_stats_key_t),
        sizeof(tbl_rmon_ether_stats_t),
        sizeof(tbl_rmon_ether_stats_fields)/sizeof(cdb_field_t),
        tbl_rmon_ether_stats_fields,
        {
            tbl_rmon_ether_stats_iterate,
            (TBL_DUMP_FUNC)tbl_rmon_ether_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rmon_ether_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rmon_ether_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rmon_ether_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rmon_ether_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rmon_ether_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rmon_ether_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_rmon_ether_stats_table_parser,
        }
    },
    {
        sizeof(tbl_rmon_history_key_t),
        sizeof(tbl_rmon_history_t),
        sizeof(tbl_rmon_history_fields)/sizeof(cdb_field_t),
        tbl_rmon_history_fields,
        {
            tbl_rmon_history_iterate,
            (TBL_DUMP_FUNC)tbl_rmon_history_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rmon_history_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rmon_history_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rmon_history_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rmon_history_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rmon_history_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rmon_history_table_dump,
            (TBL_PARSER_FUNC)tbl_rmon_history_table_parser,
        }
    },
    {
        sizeof(tbl_rmon_stats_history_key_t),
        sizeof(tbl_rmon_stats_history_t),
        sizeof(tbl_rmon_stats_history_fields)/sizeof(cdb_field_t),
        tbl_rmon_stats_history_fields,
        {
            tbl_rmon_stats_history_iterate,
            (TBL_DUMP_FUNC)tbl_rmon_stats_history_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rmon_stats_history_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rmon_stats_history_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rmon_stats_history_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rmon_stats_history_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rmon_stats_history_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rmon_stats_history_table_dump,
            (TBL_PARSER_FUNC)tbl_rmon_stats_history_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_syslog_cfg_t),
        sizeof(tbl_syslog_cfg_fields)/sizeof(cdb_field_t),
        tbl_syslog_cfg_fields,
        {
            tbl_syslog_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_syslog_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_syslog_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_syslog_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_syslog_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_syslog_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_syslog_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_syslog_cfg_table_parser,
        }
    },
    {
        sizeof(tbl_ntp_server_key_t),
        sizeof(tbl_ntp_server_t),
        sizeof(tbl_ntp_server_fields)/sizeof(cdb_field_t),
        tbl_ntp_server_fields,
        {
            tbl_ntp_server_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_server_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ntp_server_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_server_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_server_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_server_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_server_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_server_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_server_table_parser,
        }
    },
    {
        sizeof(tbl_ntp_ace_key_t),
        sizeof(tbl_ntp_ace_t),
        sizeof(tbl_ntp_ace_fields)/sizeof(cdb_field_t),
        tbl_ntp_ace_fields,
        {
            tbl_ntp_ace_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_ace_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ntp_ace_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_ace_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_ace_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_ace_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_ace_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_ace_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_ace_table_parser,
        }
    },
    {
        sizeof(tbl_ntp_key_key_t),
        sizeof(tbl_ntp_key_t),
        sizeof(tbl_ntp_key_fields)/sizeof(cdb_field_t),
        tbl_ntp_key_fields,
        {
            tbl_ntp_key_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_key_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ntp_key_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_key_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_key_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_key_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_key_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_key_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_key_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ntp_cfg_t),
        sizeof(tbl_ntp_cfg_fields)/sizeof(cdb_field_t),
        tbl_ntp_cfg_fields,
        {
            tbl_ntp_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_cfg_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ntp_if_t),
        sizeof(tbl_ntp_if_fields)/sizeof(cdb_field_t),
        tbl_ntp_if_fields,
        {
            tbl_ntp_if_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_if_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_if_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_if_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ntp_syncstatus_t),
        sizeof(tbl_ntp_syncstatus_fields)/sizeof(cdb_field_t),
        tbl_ntp_syncstatus_fields,
        {
            tbl_ntp_syncstatus_iterate,
            (TBL_DUMP_FUNC)tbl_ntp_syncstatus_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ntp_syncstatus_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ntp_syncstatus_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ntp_syncstatus_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ntp_syncstatus_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ntp_syncstatus_table_dump,
            (TBL_PARSER_FUNC)tbl_ntp_syncstatus_table_parser,
        }
    },
    {
        sizeof(tbl_static_dns_key_t),
        sizeof(tbl_static_dns_t),
        sizeof(tbl_static_dns_fields)/sizeof(cdb_field_t),
        tbl_static_dns_fields,
        {
            tbl_static_dns_iterate,
            (TBL_DUMP_FUNC)tbl_static_dns_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_static_dns_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_static_dns_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_static_dns_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_static_dns_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_static_dns_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_static_dns_table_dump,
            (TBL_PARSER_FUNC)tbl_static_dns_table_parser,
        }
    },
    {
        sizeof(tbl_dynamic_dns_domain_key_t),
        sizeof(tbl_dynamic_dns_domain_t),
        sizeof(tbl_dynamic_dns_domain_fields)/sizeof(cdb_field_t),
        tbl_dynamic_dns_domain_fields,
        {
            tbl_dynamic_dns_domain_iterate,
            (TBL_DUMP_FUNC)tbl_dynamic_dns_domain_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dynamic_dns_domain_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dynamic_dns_domain_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dynamic_dns_domain_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dynamic_dns_domain_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dynamic_dns_domain_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dynamic_dns_domain_table_dump,
            (TBL_PARSER_FUNC)tbl_dynamic_dns_domain_table_parser,
        }
    },
    {
        sizeof(tbl_dynamic_dns_server_key_t),
        sizeof(tbl_dynamic_dns_server_t),
        sizeof(tbl_dynamic_dns_server_fields)/sizeof(cdb_field_t),
        tbl_dynamic_dns_server_fields,
        {
            tbl_dynamic_dns_server_iterate,
            (TBL_DUMP_FUNC)tbl_dynamic_dns_server_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dynamic_dns_server_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dynamic_dns_server_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dynamic_dns_server_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dynamic_dns_server_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dynamic_dns_server_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dynamic_dns_server_table_dump,
            (TBL_PARSER_FUNC)tbl_dynamic_dns_server_table_parser,
        }
    },
    {
        sizeof(tbl_qos_domain_key_t),
        sizeof(tbl_qos_domain_t),
        sizeof(tbl_qos_domain_fields)/sizeof(cdb_field_t),
        tbl_qos_domain_fields,
        {
            tbl_qos_domain_iterate,
            (TBL_DUMP_FUNC)tbl_qos_domain_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_domain_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_domain_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_domain_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_domain_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_domain_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_domain_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_domain_table_parser,
        }
    },
    {
        sizeof(tbl_qos_mpls_domain_key_t),
        sizeof(tbl_qos_mpls_domain_t),
        sizeof(tbl_qos_mpls_domain_fields)/sizeof(cdb_field_t),
        tbl_qos_mpls_domain_fields,
        {
            tbl_qos_mpls_domain_iterate,
            (TBL_DUMP_FUNC)tbl_qos_mpls_domain_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_mpls_domain_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_mpls_domain_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_mpls_domain_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_mpls_domain_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_mpls_domain_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_mpls_domain_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_mpls_domain_table_parser,
        }
    },
    {
        sizeof(tbl_qos_policer_res_key_t),
        sizeof(tbl_qos_policer_res_t),
        sizeof(tbl_qos_policer_res_fields)/sizeof(cdb_field_t),
        tbl_qos_policer_res_fields,
        {
            tbl_qos_policer_res_iterate,
            (TBL_DUMP_FUNC)tbl_qos_policer_res_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_policer_res_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_policer_res_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_policer_res_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_policer_res_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_policer_res_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_policer_res_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_policer_res_table_parser,
        }
    },
    {
        sizeof(tbl_qos_policer_action_profile_key_t),
        sizeof(tbl_qos_policer_action_profile_t),
        sizeof(tbl_qos_policer_action_profile_fields)/sizeof(cdb_field_t),
        tbl_qos_policer_action_profile_fields,
        {
            tbl_qos_policer_action_profile_iterate,
            (TBL_DUMP_FUNC)tbl_qos_policer_action_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_policer_action_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_policer_action_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_policer_action_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_policer_action_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_policer_action_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_policer_action_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_policer_action_profile_table_parser,
        }
    },
    {
        sizeof(tbl_qos_policer_profile_key_t),
        sizeof(tbl_qos_policer_profile_t),
        sizeof(tbl_qos_policer_profile_fields)/sizeof(cdb_field_t),
        tbl_qos_policer_profile_fields,
        {
            tbl_qos_policer_profile_iterate,
            (TBL_DUMP_FUNC)tbl_qos_policer_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_policer_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_policer_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_policer_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_policer_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_policer_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_policer_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_policer_profile_table_parser,
        }
    },
    {
        sizeof(tbl_qos_drop_profile_key_t),
        sizeof(tbl_qos_drop_profile_t),
        sizeof(tbl_qos_drop_profile_fields)/sizeof(cdb_field_t),
        tbl_qos_drop_profile_fields,
        {
            tbl_qos_drop_profile_iterate,
            (TBL_DUMP_FUNC)tbl_qos_drop_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_drop_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_drop_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_drop_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_drop_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_drop_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_drop_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_drop_profile_table_parser,
        }
    },
    {
        sizeof(tbl_qos_queue_shape_profile_key_t),
        sizeof(tbl_qos_queue_shape_profile_t),
        sizeof(tbl_qos_queue_shape_profile_fields)/sizeof(cdb_field_t),
        tbl_qos_queue_shape_profile_fields,
        {
            tbl_qos_queue_shape_profile_iterate,
            (TBL_DUMP_FUNC)tbl_qos_queue_shape_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_queue_shape_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_queue_shape_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_queue_shape_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_queue_shape_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_queue_shape_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_queue_shape_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_queue_shape_profile_table_parser,
        }
    },
    {
        sizeof(tbl_qos_port_shape_profile_key_t),
        sizeof(tbl_qos_port_shape_profile_t),
        sizeof(tbl_qos_port_shape_profile_fields)/sizeof(cdb_field_t),
        tbl_qos_port_shape_profile_fields,
        {
            tbl_qos_port_shape_profile_iterate,
            (TBL_DUMP_FUNC)tbl_qos_port_shape_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_port_shape_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_port_shape_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_port_shape_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_port_shape_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_port_shape_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_port_shape_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_port_shape_profile_table_parser,
        }
    },
    {
        sizeof(tbl_qos_queue_smart_buffer_key_t),
        sizeof(tbl_qos_queue_smart_buffer_t),
        sizeof(tbl_qos_queue_smart_buffer_fields)/sizeof(cdb_field_t),
        tbl_qos_queue_smart_buffer_fields,
        {
            tbl_qos_queue_smart_buffer_iterate,
            (TBL_DUMP_FUNC)tbl_qos_queue_smart_buffer_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_qos_queue_smart_buffer_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_queue_smart_buffer_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_queue_smart_buffer_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_queue_smart_buffer_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_queue_smart_buffer_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_queue_smart_buffer_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_queue_smart_buffer_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_qos_global_t),
        sizeof(tbl_qos_global_fields)/sizeof(cdb_field_t),
        tbl_qos_global_fields,
        {
            tbl_qos_global_iterate,
            (TBL_DUMP_FUNC)tbl_qos_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_qos_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_qos_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_qos_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_qos_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_qos_global_table_dump,
            (TBL_PARSER_FUNC)tbl_qos_global_table_parser,
        }
    },
    {
        sizeof(tbl_mirror_key_t),
        sizeof(tbl_mirror_t),
        sizeof(tbl_mirror_fields)/sizeof(cdb_field_t),
        tbl_mirror_fields,
        {
            tbl_mirror_iterate,
            (TBL_DUMP_FUNC)tbl_mirror_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mirror_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mirror_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mirror_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mirror_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mirror_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mirror_table_dump,
            (TBL_PARSER_FUNC)tbl_mirror_table_parser,
        }
    },
    {
        sizeof(tbl_mirror_mac_escape_key_t),
        sizeof(tbl_mirror_mac_escape_t),
        sizeof(tbl_mirror_mac_escape_fields)/sizeof(cdb_field_t),
        tbl_mirror_mac_escape_fields,
        {
            tbl_mirror_mac_escape_iterate,
            (TBL_DUMP_FUNC)tbl_mirror_mac_escape_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mirror_mac_escape_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mirror_mac_escape_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mirror_mac_escape_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mirror_mac_escape_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mirror_mac_escape_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mirror_mac_escape_table_dump,
            (TBL_PARSER_FUNC)tbl_mirror_mac_escape_table_parser,
        }
    },
    {
        sizeof(tbl_tap_group_ingress_key_t),
        sizeof(tbl_tap_group_ingress_t),
        sizeof(tbl_tap_group_ingress_fields)/sizeof(cdb_field_t),
        tbl_tap_group_ingress_fields,
        {
            tbl_tap_group_ingress_iterate,
            (TBL_DUMP_FUNC)tbl_tap_group_ingress_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_tap_group_ingress_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_group_ingress_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_group_ingress_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_group_ingress_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_group_ingress_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_group_ingress_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_group_ingress_table_parser,
        }
    },
    {
        sizeof(tbl_tap_group_ingress_flow_key_t),
        sizeof(tbl_tap_group_ingress_flow_t),
        sizeof(tbl_tap_group_ingress_flow_fields)/sizeof(cdb_field_t),
        tbl_tap_group_ingress_flow_fields,
        {
            tbl_tap_group_ingress_flow_iterate,
            (TBL_DUMP_FUNC)tbl_tap_group_ingress_flow_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_tap_group_ingress_flow_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_group_ingress_flow_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_group_ingress_flow_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_group_ingress_flow_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_group_ingress_flow_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_group_ingress_flow_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_group_ingress_flow_table_parser,
        }
    },
    {
        sizeof(tbl_tap_group_egress_key_t),
        sizeof(tbl_tap_group_egress_t),
        sizeof(tbl_tap_group_egress_fields)/sizeof(cdb_field_t),
        tbl_tap_group_egress_fields,
        {
            tbl_tap_group_egress_iterate,
            (TBL_DUMP_FUNC)tbl_tap_group_egress_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_tap_group_egress_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_group_egress_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_group_egress_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_group_egress_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_group_egress_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_group_egress_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_group_egress_table_parser,
        }
    },
    {
        sizeof(tbl_tap_group_key_t),
        sizeof(tbl_tap_group_t),
        sizeof(tbl_tap_group_fields)/sizeof(cdb_field_t),
        tbl_tap_group_fields,
        {
            tbl_tap_group_iterate,
            (TBL_DUMP_FUNC)tbl_tap_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_tap_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_group_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_group_table_parser,
        }
    },
    {
        sizeof(tbl_tap_five_tuple_key_t),
        sizeof(tbl_tap_five_tuple_t),
        sizeof(tbl_tap_five_tuple_fields)/sizeof(cdb_field_t),
        tbl_tap_five_tuple_fields,
        {
            tbl_tap_five_tuple_iterate,
            (TBL_DUMP_FUNC)tbl_tap_five_tuple_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_tap_five_tuple_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_five_tuple_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_five_tuple_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_five_tuple_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_five_tuple_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_five_tuple_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_five_tuple_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_tap_dpi_global_t),
        sizeof(tbl_tap_dpi_global_fields)/sizeof(cdb_field_t),
        tbl_tap_dpi_global_fields,
        {
            tbl_tap_dpi_global_iterate,
            (TBL_DUMP_FUNC)tbl_tap_dpi_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_dpi_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_dpi_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_dpi_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_dpi_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_dpi_global_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_dpi_global_table_parser,
        }
    },
    {
        sizeof(tbl_user_key_t),
        sizeof(tbl_user_t),
        sizeof(tbl_user_fields)/sizeof(cdb_field_t),
        tbl_user_fields,
        {
            tbl_user_iterate,
            (TBL_DUMP_FUNC)tbl_user_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_user_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_user_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_user_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_user_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_user_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_user_table_dump,
            (TBL_PARSER_FUNC)tbl_user_table_parser,
        }
    },
    {
        sizeof(tbl_login_record_key_t),
        sizeof(tbl_login_record_t),
        sizeof(tbl_login_record_fields)/sizeof(cdb_field_t),
        tbl_login_record_fields,
        {
            tbl_login_record_iterate,
            (TBL_DUMP_FUNC)tbl_login_record_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_login_record_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_login_record_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_login_record_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_login_record_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_login_record_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_login_record_table_dump,
            (TBL_PARSER_FUNC)tbl_login_record_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_login_security_glb_t),
        sizeof(tbl_login_security_glb_fields)/sizeof(cdb_field_t),
        tbl_login_security_glb_fields,
        {
            tbl_login_security_glb_iterate,
            (TBL_DUMP_FUNC)tbl_login_security_glb_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_login_security_glb_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_login_security_glb_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_login_security_glb_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_login_security_glb_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_login_security_glb_table_dump,
            (TBL_PARSER_FUNC)tbl_login_security_glb_table_parser,
        }
    },
    {
        sizeof(tbl_vty_key_t),
        sizeof(tbl_vty_t),
        sizeof(tbl_vty_fields)/sizeof(cdb_field_t),
        tbl_vty_fields,
        {
            tbl_vty_iterate,
            (TBL_DUMP_FUNC)tbl_vty_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vty_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vty_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vty_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vty_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vty_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vty_table_dump,
            (TBL_PARSER_FUNC)tbl_vty_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_console_t),
        sizeof(tbl_console_fields)/sizeof(cdb_field_t),
        tbl_console_fields,
        {
            tbl_console_iterate,
            (TBL_DUMP_FUNC)tbl_console_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_console_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_console_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_console_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_console_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_console_table_dump,
            (TBL_PARSER_FUNC)tbl_console_table_parser,
        }
    },
    {
        sizeof(tbl_authen_key_t),
        sizeof(tbl_authen_t),
        sizeof(tbl_authen_fields)/sizeof(cdb_field_t),
        tbl_authen_fields,
        {
            tbl_authen_iterate,
            (TBL_DUMP_FUNC)tbl_authen_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_authen_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_authen_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_authen_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_authen_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_authen_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_authen_table_dump,
            (TBL_PARSER_FUNC)tbl_authen_table_parser,
        }
    },
    {
        sizeof(tbl_login_key_t),
        sizeof(tbl_login_t),
        sizeof(tbl_login_fields)/sizeof(cdb_field_t),
        tbl_login_fields,
        {
            tbl_login_iterate,
            (TBL_DUMP_FUNC)tbl_login_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_login_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_login_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_login_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_login_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_login_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_login_table_dump,
            (TBL_PARSER_FUNC)tbl_login_table_parser,
        }
    },
    {
        sizeof(tbl_rsa_key_t),
        sizeof(tbl_rsa_t),
        sizeof(tbl_rsa_fields)/sizeof(cdb_field_t),
        tbl_rsa_fields,
        {
            tbl_rsa_iterate,
            (TBL_DUMP_FUNC)tbl_rsa_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rsa_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rsa_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rsa_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rsa_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rsa_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rsa_table_dump,
            (TBL_PARSER_FUNC)tbl_rsa_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_openflow_t),
        sizeof(tbl_openflow_fields)/sizeof(cdb_field_t),
        tbl_openflow_fields,
        {
            tbl_openflow_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_table_parser,
        }
    },
    {
        sizeof(tbl_cpu_traffic_key_t),
        sizeof(tbl_cpu_traffic_t),
        sizeof(tbl_cpu_traffic_fields)/sizeof(cdb_field_t),
        tbl_cpu_traffic_fields,
        {
            tbl_cpu_traffic_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_traffic_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cpu_traffic_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_traffic_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_traffic_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_traffic_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_traffic_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_traffic_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_traffic_table_parser,
        }
    },
    {
        sizeof(tbl_cpu_traffic_group_key_t),
        sizeof(tbl_cpu_traffic_group_t),
        sizeof(tbl_cpu_traffic_group_fields)/sizeof(cdb_field_t),
        tbl_cpu_traffic_group_fields,
        {
            tbl_cpu_traffic_group_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_traffic_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cpu_traffic_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_traffic_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_traffic_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_traffic_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_traffic_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_traffic_group_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_traffic_group_table_parser,
        }
    },
    {
        sizeof(tbl_cpu_utilization_key_t),
        sizeof(tbl_cpu_utilization_t),
        sizeof(tbl_cpu_utilization_fields)/sizeof(cdb_field_t),
        tbl_cpu_utilization_fields,
        {
            tbl_cpu_utilization_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_utilization_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cpu_utilization_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_utilization_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_utilization_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_utilization_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_utilization_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_utilization_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_utilization_table_parser,
        }
    },
    {
        sizeof(tbl_cpu_limit_key_t),
        sizeof(tbl_cpu_limit_t),
        sizeof(tbl_cpu_limit_fields)/sizeof(cdb_field_t),
        tbl_cpu_limit_fields,
        {
            tbl_cpu_limit_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_limit_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cpu_limit_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_limit_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_limit_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_limit_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_limit_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_limit_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_limit_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dhcrelay_t),
        sizeof(tbl_dhcrelay_fields)/sizeof(cdb_field_t),
        tbl_dhcrelay_fields,
        {
            tbl_dhcrelay_iterate,
            (TBL_DUMP_FUNC)tbl_dhcrelay_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcrelay_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcrelay_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcrelay_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcrelay_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcrelay_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcrelay_table_parser,
        }
    },
    {
        sizeof(tbl_dhcsrvgrp_key_t),
        sizeof(tbl_dhcsrvgrp_t),
        sizeof(tbl_dhcsrvgrp_fields)/sizeof(cdb_field_t),
        tbl_dhcsrvgrp_fields,
        {
            tbl_dhcsrvgrp_iterate,
            (TBL_DUMP_FUNC)tbl_dhcsrvgrp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dhcsrvgrp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcsrvgrp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcsrvgrp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcsrvgrp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcsrvgrp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcsrvgrp_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcsrvgrp_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dhcp_debug_t),
        sizeof(tbl_dhcp_debug_fields)/sizeof(cdb_field_t),
        tbl_dhcp_debug_fields,
        {
            tbl_dhcp_debug_iterate,
            (TBL_DUMP_FUNC)tbl_dhcp_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcp_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcp_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcp_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcp_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcp_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcp_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dhclient_t),
        sizeof(tbl_dhclient_fields)/sizeof(cdb_field_t),
        tbl_dhclient_fields,
        {
            tbl_dhclient_iterate,
            (TBL_DUMP_FUNC)tbl_dhclient_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhclient_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhclient_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhclient_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhclient_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhclient_table_dump,
            (TBL_PARSER_FUNC)tbl_dhclient_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dhcsnooping_t),
        sizeof(tbl_dhcsnooping_fields)/sizeof(cdb_field_t),
        tbl_dhcsnooping_fields,
        {
            tbl_dhcsnooping_iterate,
            (TBL_DUMP_FUNC)tbl_dhcsnooping_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcsnooping_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcsnooping_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcsnooping_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcsnooping_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcsnooping_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcsnooping_table_parser,
        }
    },
    {
        sizeof(tbl_dhcbinding_key_t),
        sizeof(tbl_dhcbinding_t),
        sizeof(tbl_dhcbinding_fields)/sizeof(cdb_field_t),
        tbl_dhcbinding_fields,
        {
            tbl_dhcbinding_iterate,
            (TBL_DUMP_FUNC)tbl_dhcbinding_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dhcbinding_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcbinding_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcbinding_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcbinding_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcbinding_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcbinding_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcbinding_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_iptables_prevent_t),
        sizeof(tbl_iptables_prevent_fields)/sizeof(cdb_field_t),
        tbl_iptables_prevent_fields,
        {
            tbl_iptables_prevent_iterate,
            (TBL_DUMP_FUNC)tbl_iptables_prevent_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_iptables_prevent_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_iptables_prevent_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_iptables_prevent_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_iptables_prevent_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_iptables_prevent_table_dump,
            (TBL_PARSER_FUNC)tbl_iptables_prevent_table_parser,
        }
    },
    {
        sizeof(tbl_errdisable_key_t),
        sizeof(tbl_errdisable_t),
        sizeof(tbl_errdisable_fields)/sizeof(cdb_field_t),
        tbl_errdisable_fields,
        {
            tbl_errdisable_iterate,
            (TBL_DUMP_FUNC)tbl_errdisable_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_errdisable_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_errdisable_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_errdisable_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_errdisable_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_errdisable_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_errdisable_table_dump,
            (TBL_PARSER_FUNC)tbl_errdisable_table_parser,
        }
    },
    {
        sizeof(tbl_ns_port_forwarding_key_t),
        sizeof(tbl_ns_port_forwarding_t),
        sizeof(tbl_ns_port_forwarding_fields)/sizeof(cdb_field_t),
        tbl_ns_port_forwarding_fields,
        {
            tbl_ns_port_forwarding_iterate,
            (TBL_DUMP_FUNC)tbl_ns_port_forwarding_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ns_port_forwarding_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ns_port_forwarding_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ns_port_forwarding_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ns_port_forwarding_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ns_port_forwarding_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ns_port_forwarding_table_dump,
            (TBL_PARSER_FUNC)tbl_ns_port_forwarding_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_log_global_t),
        sizeof(tbl_log_global_fields)/sizeof(cdb_field_t),
        tbl_log_global_fields,
        {
            tbl_log_global_iterate,
            (TBL_DUMP_FUNC)tbl_log_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_log_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_log_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_log_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_log_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_log_global_table_dump,
            (TBL_PARSER_FUNC)tbl_log_global_table_parser,
        }
    },
    {
        sizeof(tbl_log_key_t),
        sizeof(tbl_log_t),
        sizeof(tbl_log_fields)/sizeof(cdb_field_t),
        tbl_log_fields,
        {
            tbl_log_iterate,
            (TBL_DUMP_FUNC)tbl_log_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_log_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_log_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_log_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_log_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_log_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_log_table_dump,
            (TBL_PARSER_FUNC)tbl_log_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_sys_load_t),
        sizeof(tbl_sys_load_fields)/sizeof(cdb_field_t),
        tbl_sys_load_fields,
        {
            tbl_sys_load_iterate,
            (TBL_DUMP_FUNC)tbl_sys_load_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sys_load_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sys_load_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sys_load_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sys_load_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sys_load_table_dump,
            (TBL_PARSER_FUNC)tbl_sys_load_table_parser,
        }
    },
    {
        sizeof(tbl_cem_key_t),
        sizeof(tbl_cem_t),
        sizeof(tbl_cem_fields)/sizeof(cdb_field_t),
        tbl_cem_fields,
        {
            tbl_cem_iterate,
            (TBL_DUMP_FUNC)tbl_cem_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cem_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cem_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cem_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cem_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cem_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cem_table_dump,
            (TBL_PARSER_FUNC)tbl_cem_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_clock_t),
        sizeof(tbl_clock_fields)/sizeof(cdb_field_t),
        tbl_clock_fields,
        {
            tbl_clock_iterate,
            (TBL_DUMP_FUNC)tbl_clock_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_clock_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_clock_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_clock_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_clock_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_clock_table_dump,
            (TBL_PARSER_FUNC)tbl_clock_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_tag_summer_clock_t),
        sizeof(tbl_tag_summer_clock_fields)/sizeof(cdb_field_t),
        tbl_tag_summer_clock_fields,
        {
            tbl_tag_summer_clock_iterate,
            (TBL_DUMP_FUNC)tbl_tag_summer_clock_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tag_summer_clock_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tag_summer_clock_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tag_summer_clock_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tag_summer_clock_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tag_summer_clock_table_dump,
            (TBL_PARSER_FUNC)tbl_tag_summer_clock_table_parser,
        }
    },
    {
        sizeof(tbl_port_stats_key_t),
        sizeof(tbl_port_stats_t),
        sizeof(tbl_port_stats_fields)/sizeof(cdb_field_t),
        tbl_port_stats_fields,
        {
            tbl_port_stats_iterate,
            (TBL_DUMP_FUNC)tbl_port_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_port_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_port_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_port_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_port_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_port_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_port_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_port_stats_table_parser,
        }
    },
    {
        sizeof(tbl_vlan_stats_key_t),
        sizeof(tbl_vlan_stats_t),
        sizeof(tbl_vlan_stats_fields)/sizeof(cdb_field_t),
        tbl_vlan_stats_fields,
        {
            tbl_vlan_stats_iterate,
            (TBL_DUMP_FUNC)tbl_vlan_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlan_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlan_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlan_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlan_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlan_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlan_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_vlan_stats_table_parser,
        }
    },
    {
        sizeof(tbl_port_stats_rate_key_t),
        sizeof(tbl_port_stats_rate_t),
        sizeof(tbl_port_stats_rate_fields)/sizeof(cdb_field_t),
        tbl_port_stats_rate_fields,
        {
            tbl_port_stats_rate_iterate,
            (TBL_DUMP_FUNC)tbl_port_stats_rate_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_port_stats_rate_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_port_stats_rate_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_port_stats_rate_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_port_stats_rate_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_port_stats_rate_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_port_stats_rate_table_dump,
            (TBL_PARSER_FUNC)tbl_port_stats_rate_table_parser,
        }
    },
    {
        sizeof(tbl_aclqos_if_key_t),
        sizeof(tbl_aclqos_if_t),
        sizeof(tbl_aclqos_if_fields)/sizeof(cdb_field_t),
        tbl_aclqos_if_fields,
        {
            tbl_aclqos_if_iterate,
            (TBL_DUMP_FUNC)tbl_aclqos_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_aclqos_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_aclqos_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_aclqos_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_aclqos_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_aclqos_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_aclqos_if_table_dump,
            (TBL_PARSER_FUNC)tbl_aclqos_if_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_l2_action_t),
        sizeof(tbl_l2_action_fields)/sizeof(cdb_field_t),
        tbl_l2_action_fields,
        {
            tbl_l2_action_iterate,
            (TBL_DUMP_FUNC)tbl_l2_action_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_l2_action_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_l2_action_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_l2_action_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_l2_action_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_l2_action_table_dump,
            (TBL_PARSER_FUNC)tbl_l2_action_table_parser,
        }
    },
    {
        sizeof(tbl_fea_qos_drop_profile_key_t),
        sizeof(tbl_fea_qos_drop_profile_t),
        sizeof(tbl_fea_qos_drop_profile_fields)/sizeof(cdb_field_t),
        tbl_fea_qos_drop_profile_fields,
        {
            tbl_fea_qos_drop_profile_iterate,
            (TBL_DUMP_FUNC)tbl_fea_qos_drop_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_qos_drop_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_qos_drop_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_qos_drop_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_qos_drop_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_qos_drop_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_qos_drop_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_qos_drop_profile_table_parser,
        }
    },
    {
        sizeof(tbl_fea_qos_domain_key_t),
        sizeof(tbl_fea_qos_domain_t),
        sizeof(tbl_fea_qos_domain_fields)/sizeof(cdb_field_t),
        tbl_fea_qos_domain_fields,
        {
            tbl_fea_qos_domain_iterate,
            (TBL_DUMP_FUNC)tbl_fea_qos_domain_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_qos_domain_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_qos_domain_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_qos_domain_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_qos_domain_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_qos_domain_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_qos_domain_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_qos_domain_table_parser,
        }
    },
    {
        sizeof(tbl_fea_g8032_ring_key_t),
        sizeof(tbl_fea_g8032_ring_t),
        sizeof(tbl_fea_g8032_ring_fields)/sizeof(cdb_field_t),
        tbl_fea_g8032_ring_fields,
        {
            tbl_fea_g8032_ring_iterate,
            (TBL_DUMP_FUNC)tbl_fea_g8032_ring_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_g8032_ring_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_g8032_ring_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_g8032_ring_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_g8032_ring_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_g8032_ring_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_g8032_ring_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_g8032_ring_table_parser,
        }
    },
    {
        sizeof(tbl_fea_g8032_ring_mc_group_key_t),
        sizeof(tbl_fea_g8032_ring_mc_group_t),
        sizeof(tbl_fea_g8032_ring_mc_group_fields)/sizeof(cdb_field_t),
        tbl_fea_g8032_ring_mc_group_fields,
        {
            tbl_fea_g8032_ring_mc_group_iterate,
            (TBL_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_g8032_ring_mc_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_g8032_ring_mc_group_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_g8032_ring_mc_group_table_parser,
        }
    },
    {
        sizeof(tbl_fea_qos_queue_shape_profile_key_t),
        sizeof(tbl_fea_qos_queue_shape_profile_t),
        sizeof(tbl_fea_qos_queue_shape_profile_fields)/sizeof(cdb_field_t),
        tbl_fea_qos_queue_shape_profile_fields,
        {
            tbl_fea_qos_queue_shape_profile_iterate,
            (TBL_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_qos_queue_shape_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_qos_queue_shape_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_qos_queue_shape_profile_table_parser,
        }
    },
    {
        sizeof(tbl_fea_qos_port_shape_profile_key_t),
        sizeof(tbl_fea_qos_port_shape_profile_t),
        sizeof(tbl_fea_qos_port_shape_profile_fields)/sizeof(cdb_field_t),
        tbl_fea_qos_port_shape_profile_fields,
        {
            tbl_fea_qos_port_shape_profile_iterate,
            (TBL_DUMP_FUNC)tbl_fea_qos_port_shape_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_qos_port_shape_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_qos_port_shape_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_qos_port_shape_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_qos_port_shape_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_qos_port_shape_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_qos_port_shape_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_qos_port_shape_profile_table_parser,
        }
    },
    {
        sizeof(tbl_fea_port_policer_apply_key_t),
        sizeof(tbl_fea_port_policer_apply_t),
        sizeof(tbl_fea_port_policer_apply_fields)/sizeof(cdb_field_t),
        tbl_fea_port_policer_apply_fields,
        {
            tbl_fea_port_policer_apply_iterate,
            (TBL_DUMP_FUNC)tbl_fea_port_policer_apply_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_port_policer_apply_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_port_policer_apply_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_port_policer_apply_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_port_policer_apply_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_port_policer_apply_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_port_policer_apply_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_port_policer_apply_table_parser,
        }
    },
    {
        sizeof(tbl_aclqos_if_stats_key_t),
        sizeof(tbl_aclqos_if_stats_t),
        sizeof(tbl_aclqos_if_stats_fields)/sizeof(cdb_field_t),
        tbl_aclqos_if_stats_fields,
        {
            tbl_aclqos_if_stats_iterate,
            (TBL_DUMP_FUNC)tbl_aclqos_if_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_aclqos_if_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_aclqos_if_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_aclqos_if_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_aclqos_if_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_aclqos_if_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_aclqos_if_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_aclqos_if_stats_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_version_t),
        sizeof(tbl_version_fields)/sizeof(cdb_field_t),
        tbl_version_fields,
        {
            tbl_version_iterate,
            (TBL_DUMP_FUNC)tbl_version_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_version_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_version_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_version_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_version_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_version_table_dump,
            (TBL_PARSER_FUNC)tbl_version_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_manage_if_t),
        sizeof(tbl_manage_if_fields)/sizeof(cdb_field_t),
        tbl_manage_if_fields,
        {
            tbl_manage_if_iterate,
            (TBL_DUMP_FUNC)tbl_manage_if_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_manage_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_manage_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_manage_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_manage_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_manage_if_table_dump,
            (TBL_PARSER_FUNC)tbl_manage_if_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_bootimage_t),
        sizeof(tbl_bootimage_fields)/sizeof(cdb_field_t),
        tbl_bootimage_fields,
        {
            tbl_bootimage_iterate,
            (TBL_DUMP_FUNC)tbl_bootimage_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bootimage_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bootimage_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bootimage_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bootimage_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bootimage_table_dump,
            (TBL_PARSER_FUNC)tbl_bootimage_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_chassis_t),
        sizeof(tbl_chassis_fields)/sizeof(cdb_field_t),
        tbl_chassis_fields,
        {
            tbl_chassis_iterate,
            (TBL_DUMP_FUNC)tbl_chassis_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_chassis_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_chassis_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_chassis_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_chassis_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_chassis_table_dump,
            (TBL_PARSER_FUNC)tbl_chassis_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ifname_info_t),
        sizeof(tbl_ifname_info_fields)/sizeof(cdb_field_t),
        tbl_ifname_info_fields,
        {
            tbl_ifname_info_iterate,
            (TBL_DUMP_FUNC)tbl_ifname_info_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ifname_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ifname_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ifname_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ifname_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ifname_info_table_dump,
            (TBL_PARSER_FUNC)tbl_ifname_info_table_parser,
        }
    },
    {
        sizeof(tbl_card_key_t),
        sizeof(tbl_card_t),
        sizeof(tbl_card_fields)/sizeof(cdb_field_t),
        tbl_card_fields,
        {
            tbl_card_iterate,
            (TBL_DUMP_FUNC)tbl_card_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_card_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_card_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_card_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_card_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_card_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_card_table_dump,
            (TBL_PARSER_FUNC)tbl_card_table_parser,
        }
    },
    {
        sizeof(tbl_port_key_t),
        sizeof(tbl_port_t),
        sizeof(tbl_port_fields)/sizeof(cdb_field_t),
        tbl_port_fields,
        {
            tbl_port_iterate,
            (TBL_DUMP_FUNC)tbl_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_port_table_dump,
            (TBL_PARSER_FUNC)tbl_port_table_parser,
        }
    },
    {
        sizeof(tbl_fiber_key_t),
        sizeof(tbl_fiber_t),
        sizeof(tbl_fiber_fields)/sizeof(cdb_field_t),
        tbl_fiber_fields,
        {
            tbl_fiber_iterate,
            (TBL_DUMP_FUNC)tbl_fiber_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fiber_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fiber_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fiber_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fiber_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fiber_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fiber_table_dump,
            (TBL_PARSER_FUNC)tbl_fiber_table_parser,
        }
    },
    {
        sizeof(tbl_sys_spec_key_t),
        sizeof(tbl_sys_spec_t),
        sizeof(tbl_sys_spec_fields)/sizeof(cdb_field_t),
        tbl_sys_spec_fields,
        {
            tbl_sys_spec_iterate,
            (TBL_DUMP_FUNC)tbl_sys_spec_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_sys_spec_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sys_spec_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sys_spec_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sys_spec_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sys_spec_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sys_spec_table_dump,
            (TBL_PARSER_FUNC)tbl_sys_spec_table_parser,
        }
    },
    {
        sizeof(tbl_fan_key_t),
        sizeof(tbl_fan_t),
        sizeof(tbl_fan_fields)/sizeof(cdb_field_t),
        tbl_fan_fields,
        {
            tbl_fan_iterate,
            (TBL_DUMP_FUNC)tbl_fan_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fan_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fan_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fan_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fan_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fan_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fan_table_dump,
            (TBL_PARSER_FUNC)tbl_fan_table_parser,
        }
    },
    {
        sizeof(tbl_psu_key_t),
        sizeof(tbl_psu_t),
        sizeof(tbl_psu_fields)/sizeof(cdb_field_t),
        tbl_psu_fields,
        {
            tbl_psu_iterate,
            (TBL_DUMP_FUNC)tbl_psu_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_psu_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_psu_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_psu_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_psu_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_psu_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_psu_table_dump,
            (TBL_PARSER_FUNC)tbl_psu_table_parser,
        }
    },
    {
        sizeof(tbl_led_key_t),
        sizeof(tbl_led_t),
        sizeof(tbl_led_fields)/sizeof(cdb_field_t),
        tbl_led_fields,
        {
            tbl_led_iterate,
            (TBL_DUMP_FUNC)tbl_led_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_led_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_led_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_led_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_led_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_led_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_led_table_dump,
            (TBL_PARSER_FUNC)tbl_led_table_parser,
        }
    },
    {
        sizeof(tbl_sensor_key_t),
        sizeof(tbl_sensor_t),
        sizeof(tbl_sensor_fields)/sizeof(cdb_field_t),
        tbl_sensor_fields,
        {
            tbl_sensor_iterate,
            (TBL_DUMP_FUNC)tbl_sensor_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_sensor_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sensor_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sensor_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sensor_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sensor_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sensor_table_dump,
            (TBL_PARSER_FUNC)tbl_sensor_table_parser,
        }
    },
    {
        sizeof(tbl_vcm_key_t),
        sizeof(tbl_vcm_t),
        sizeof(tbl_vcm_fields)/sizeof(cdb_field_t),
        tbl_vcm_fields,
        {
            tbl_vcm_iterate,
            (TBL_DUMP_FUNC)tbl_vcm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vcm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vcm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vcm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vcm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vcm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vcm_table_dump,
            (TBL_PARSER_FUNC)tbl_vcm_table_parser,
        }
    },
    {
        sizeof(tbl_poe_key_t),
        sizeof(tbl_poe_t),
        sizeof(tbl_poe_fields)/sizeof(cdb_field_t),
        tbl_poe_fields,
        {
            tbl_poe_iterate,
            (TBL_DUMP_FUNC)tbl_poe_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_poe_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_poe_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_poe_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_poe_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_poe_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_poe_table_dump,
            (TBL_PARSER_FUNC)tbl_poe_table_parser,
        }
    },
    {
        sizeof(tbl_reboot_info_key_t),
        sizeof(tbl_reboot_info_t),
        sizeof(tbl_reboot_info_fields)/sizeof(cdb_field_t),
        tbl_reboot_info_fields,
        {
            tbl_reboot_info_iterate,
            (TBL_DUMP_FUNC)tbl_reboot_info_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_reboot_info_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_reboot_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_reboot_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_reboot_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_reboot_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_reboot_info_table_dump,
            (TBL_PARSER_FUNC)tbl_reboot_info_table_parser,
        }
    },
    {
        sizeof(tbl_errdisable_flap_key_t),
        sizeof(tbl_errdisable_flap_t),
        sizeof(tbl_errdisable_flap_fields)/sizeof(cdb_field_t),
        tbl_errdisable_flap_fields,
        {
            tbl_errdisable_flap_iterate,
            (TBL_DUMP_FUNC)tbl_errdisable_flap_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_errdisable_flap_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_errdisable_flap_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_errdisable_flap_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_errdisable_flap_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_errdisable_flap_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_errdisable_flap_table_dump,
            (TBL_PARSER_FUNC)tbl_errdisable_flap_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_opm_global_t),
        sizeof(tbl_opm_global_fields)/sizeof(cdb_field_t),
        tbl_opm_global_fields,
        {
            tbl_opm_global_iterate,
            (TBL_DUMP_FUNC)tbl_opm_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_opm_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_opm_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_opm_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_opm_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_opm_global_table_dump,
            (TBL_PARSER_FUNC)tbl_opm_global_table_parser,
        }
    },
    {
        sizeof(tbl_erps_ring_key_t),
        sizeof(tbl_erps_ring_t),
        sizeof(tbl_erps_ring_fields)/sizeof(cdb_field_t),
        tbl_erps_ring_fields,
        {
            tbl_erps_ring_iterate,
            (TBL_DUMP_FUNC)tbl_erps_ring_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_erps_ring_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_erps_ring_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_erps_ring_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_erps_ring_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_erps_ring_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_erps_ring_table_dump,
            (TBL_PARSER_FUNC)tbl_erps_ring_table_parser,
        }
    },
    {
        sizeof(tbl_erps_domain_key_t),
        sizeof(tbl_erps_domain_t),
        sizeof(tbl_erps_domain_fields)/sizeof(cdb_field_t),
        tbl_erps_domain_fields,
        {
            tbl_erps_domain_iterate,
            (TBL_DUMP_FUNC)tbl_erps_domain_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_erps_domain_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_erps_domain_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_erps_domain_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_erps_domain_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_erps_domain_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_erps_domain_table_dump,
            (TBL_PARSER_FUNC)tbl_erps_domain_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_opm_debug_t),
        sizeof(tbl_opm_debug_fields)/sizeof(cdb_field_t),
        tbl_opm_debug_fields,
        {
            tbl_opm_debug_iterate,
            (TBL_DUMP_FUNC)tbl_opm_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_opm_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_opm_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_opm_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_opm_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_opm_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_opm_debug_table_parser,
        }
    },
    {
        sizeof(tbl_policy_map_config_key_t),
        sizeof(tbl_policy_map_config_t),
        sizeof(tbl_policy_map_config_fields)/sizeof(cdb_field_t),
        tbl_policy_map_config_fields,
        {
            tbl_policy_map_config_iterate,
            (TBL_DUMP_FUNC)tbl_policy_map_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_policy_map_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_policy_map_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_policy_map_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_policy_map_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_policy_map_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_policy_map_config_table_dump,
            (TBL_PARSER_FUNC)tbl_policy_map_config_table_parser,
        }
    },
    {
        sizeof(tbl_class_map_config_key_t),
        sizeof(tbl_class_map_config_t),
        sizeof(tbl_class_map_config_fields)/sizeof(cdb_field_t),
        tbl_class_map_config_fields,
        {
            tbl_class_map_config_iterate,
            (TBL_DUMP_FUNC)tbl_class_map_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_class_map_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_class_map_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_class_map_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_class_map_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_class_map_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_class_map_config_table_dump,
            (TBL_PARSER_FUNC)tbl_class_map_config_table_parser,
        }
    },
    {
        sizeof(tbl_class_in_policy_config_key_t),
        sizeof(tbl_class_in_policy_config_t),
        sizeof(tbl_class_in_policy_config_fields)/sizeof(cdb_field_t),
        tbl_class_in_policy_config_fields,
        {
            tbl_class_in_policy_config_iterate,
            (TBL_DUMP_FUNC)tbl_class_in_policy_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_class_in_policy_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_class_in_policy_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_class_in_policy_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_class_in_policy_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_class_in_policy_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_class_in_policy_config_table_dump,
            (TBL_PARSER_FUNC)tbl_class_in_policy_config_table_parser,
        }
    },
    {
        sizeof(tbl_acl_in_class_config_key_t),
        sizeof(tbl_acl_in_class_config_t),
        sizeof(tbl_acl_in_class_config_fields)/sizeof(cdb_field_t),
        tbl_acl_in_class_config_fields,
        {
            tbl_acl_in_class_config_iterate,
            (TBL_DUMP_FUNC)tbl_acl_in_class_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_in_class_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_in_class_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_in_class_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_in_class_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_in_class_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_in_class_config_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_in_class_config_table_parser,
        }
    },
    {
        sizeof(tbl_class_map_action_config_key_t),
        sizeof(tbl_class_map_action_config_t),
        sizeof(tbl_class_map_action_config_fields)/sizeof(cdb_field_t),
        tbl_class_map_action_config_fields,
        {
            tbl_class_map_action_config_iterate,
            (TBL_DUMP_FUNC)tbl_class_map_action_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_class_map_action_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_class_map_action_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_class_map_action_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_class_map_action_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_class_map_action_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_class_map_action_config_table_dump,
            (TBL_PARSER_FUNC)tbl_class_map_action_config_table_parser,
        }
    },
    {
        sizeof(tbl_fea_acl_policy_action_key_t),
        sizeof(tbl_fea_acl_policy_action_t),
        sizeof(tbl_fea_acl_policy_action_fields)/sizeof(cdb_field_t),
        tbl_fea_acl_policy_action_fields,
        {
            tbl_fea_acl_policy_action_iterate,
            (TBL_DUMP_FUNC)tbl_fea_acl_policy_action_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_acl_policy_action_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_acl_policy_action_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_acl_policy_action_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_acl_policy_action_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_acl_policy_action_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_acl_policy_action_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_acl_policy_action_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_igsp_global_t),
        sizeof(tbl_igsp_global_fields)/sizeof(cdb_field_t),
        tbl_igsp_global_fields,
        {
            tbl_igsp_global_iterate,
            (TBL_DUMP_FUNC)tbl_igsp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_igsp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_igsp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_igsp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_igsp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_igsp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_igsp_global_table_parser,
        }
    },
    {
        sizeof(tbl_igsp_intf_key_t),
        sizeof(tbl_igsp_intf_t),
        sizeof(tbl_igsp_intf_fields)/sizeof(cdb_field_t),
        tbl_igsp_intf_fields,
        {
            tbl_igsp_intf_iterate,
            (TBL_DUMP_FUNC)tbl_igsp_intf_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_igsp_intf_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_igsp_intf_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_igsp_intf_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_igsp_intf_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_igsp_intf_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_igsp_intf_table_dump,
            (TBL_PARSER_FUNC)tbl_igsp_intf_table_parser,
        }
    },
    {
        sizeof(tbl_igsp_group_key_t),
        sizeof(tbl_igsp_group_t),
        sizeof(tbl_igsp_group_fields)/sizeof(cdb_field_t),
        tbl_igsp_group_fields,
        {
            tbl_igsp_group_iterate,
            (TBL_DUMP_FUNC)tbl_igsp_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_igsp_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_igsp_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_igsp_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_igsp_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_igsp_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_igsp_group_table_dump,
            (TBL_PARSER_FUNC)tbl_igsp_group_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_auth_cfg_t),
        sizeof(tbl_auth_cfg_fields)/sizeof(cdb_field_t),
        tbl_auth_cfg_fields,
        {
            tbl_auth_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_auth_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_auth_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_auth_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_auth_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_auth_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_auth_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_auth_cfg_table_parser,
        }
    },
    {
        sizeof(tbl_auth_server_key_t),
        sizeof(tbl_auth_server_t),
        sizeof(tbl_auth_server_fields)/sizeof(cdb_field_t),
        tbl_auth_server_fields,
        {
            tbl_auth_server_iterate,
            (TBL_DUMP_FUNC)tbl_auth_server_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_auth_server_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_auth_server_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_auth_server_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_auth_server_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_auth_server_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_auth_server_table_dump,
            (TBL_PARSER_FUNC)tbl_auth_server_table_parser,
        }
    },
    {
        sizeof(tbl_auth_session_key_t),
        sizeof(tbl_auth_session_t),
        sizeof(tbl_auth_session_fields)/sizeof(cdb_field_t),
        tbl_auth_session_fields,
        {
            tbl_auth_session_iterate,
            (TBL_DUMP_FUNC)tbl_auth_session_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_auth_session_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_auth_session_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_auth_session_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_auth_session_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_auth_session_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_auth_session_table_dump,
            (TBL_PARSER_FUNC)tbl_auth_session_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_authd_debug_t),
        sizeof(tbl_authd_debug_fields)/sizeof(cdb_field_t),
        tbl_authd_debug_fields,
        {
            tbl_authd_debug_iterate,
            (TBL_DUMP_FUNC)tbl_authd_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_authd_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_authd_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_authd_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_authd_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_authd_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_authd_debug_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dot1x_global_t),
        sizeof(tbl_dot1x_global_fields)/sizeof(cdb_field_t),
        tbl_dot1x_global_fields,
        {
            tbl_dot1x_global_iterate,
            (TBL_DUMP_FUNC)tbl_dot1x_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dot1x_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dot1x_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dot1x_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dot1x_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dot1x_global_table_dump,
            (TBL_PARSER_FUNC)tbl_dot1x_global_table_parser,
        }
    },
    {
        sizeof(tbl_dot1x_port_key_t),
        sizeof(tbl_dot1x_port_t),
        sizeof(tbl_dot1x_port_fields)/sizeof(cdb_field_t),
        tbl_dot1x_port_fields,
        {
            tbl_dot1x_port_iterate,
            (TBL_DUMP_FUNC)tbl_dot1x_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dot1x_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dot1x_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dot1x_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dot1x_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dot1x_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dot1x_port_table_dump,
            (TBL_PARSER_FUNC)tbl_dot1x_port_table_parser,
        }
    },
    {
        sizeof(tbl_dot1x_radius_key_t),
        sizeof(tbl_dot1x_radius_t),
        sizeof(tbl_dot1x_radius_fields)/sizeof(cdb_field_t),
        tbl_dot1x_radius_fields,
        {
            tbl_dot1x_radius_iterate,
            (TBL_DUMP_FUNC)tbl_dot1x_radius_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dot1x_radius_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dot1x_radius_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dot1x_radius_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dot1x_radius_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dot1x_radius_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dot1x_radius_table_dump,
            (TBL_PARSER_FUNC)tbl_dot1x_radius_table_parser,
        }
    },
    {
        sizeof(tbl_dot1x_mac_key_t),
        sizeof(tbl_dot1x_mac_t),
        sizeof(tbl_dot1x_mac_fields)/sizeof(cdb_field_t),
        tbl_dot1x_mac_fields,
        {
            tbl_dot1x_mac_iterate,
            (TBL_DUMP_FUNC)tbl_dot1x_mac_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_dot1x_mac_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dot1x_mac_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dot1x_mac_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dot1x_mac_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dot1x_mac_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dot1x_mac_table_dump,
            (TBL_PARSER_FUNC)tbl_dot1x_mac_table_parser,
        }
    },
    {
        sizeof(tbl_enable_key_t),
        sizeof(tbl_enable_t),
        sizeof(tbl_enable_fields)/sizeof(cdb_field_t),
        tbl_enable_fields,
        {
            tbl_enable_iterate,
            (TBL_DUMP_FUNC)tbl_enable_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_enable_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_enable_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_enable_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_enable_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_enable_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_enable_table_dump,
            (TBL_PARSER_FUNC)tbl_enable_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_chip_t),
        sizeof(tbl_chip_fields)/sizeof(cdb_field_t),
        tbl_chip_fields,
        {
            tbl_chip_iterate,
            (TBL_DUMP_FUNC)tbl_chip_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_chip_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_chip_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_chip_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_chip_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_chip_table_dump,
            (TBL_PARSER_FUNC)tbl_chip_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_clear_acl_policy_t),
        sizeof(tbl_clear_acl_policy_fields)/sizeof(cdb_field_t),
        tbl_clear_acl_policy_fields,
        {
            tbl_clear_acl_policy_iterate,
            (TBL_DUMP_FUNC)tbl_clear_acl_policy_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_clear_acl_policy_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_clear_acl_policy_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_clear_acl_policy_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_clear_acl_policy_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_clear_acl_policy_table_dump,
            (TBL_PARSER_FUNC)tbl_clear_acl_policy_table_parser,
        }
    },
    {
        sizeof(tbl_author_key_t),
        sizeof(tbl_author_t),
        sizeof(tbl_author_fields)/sizeof(cdb_field_t),
        tbl_author_fields,
        {
            tbl_author_iterate,
            (TBL_DUMP_FUNC)tbl_author_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_author_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_author_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_author_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_author_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_author_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_author_table_dump,
            (TBL_PARSER_FUNC)tbl_author_table_parser,
        }
    },
    {
        sizeof(tbl_account_key_t),
        sizeof(tbl_account_t),
        sizeof(tbl_account_fields)/sizeof(cdb_field_t),
        tbl_account_fields,
        {
            tbl_account_iterate,
            (TBL_DUMP_FUNC)tbl_account_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_account_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_account_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_account_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_account_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_account_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_account_table_dump,
            (TBL_PARSER_FUNC)tbl_account_table_parser,
        }
    },
    {
        sizeof(tbl_accountcmd_key_t),
        sizeof(tbl_accountcmd_t),
        sizeof(tbl_accountcmd_fields)/sizeof(cdb_field_t),
        tbl_accountcmd_fields,
        {
            tbl_accountcmd_iterate,
            (TBL_DUMP_FUNC)tbl_accountcmd_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_accountcmd_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_accountcmd_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_accountcmd_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_accountcmd_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_accountcmd_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_accountcmd_table_dump,
            (TBL_PARSER_FUNC)tbl_accountcmd_table_parser,
        }
    },
    {
        sizeof(tbl_vlanclass_rule_key_t),
        sizeof(tbl_vlanclass_rule_t),
        sizeof(tbl_vlanclass_rule_fields)/sizeof(cdb_field_t),
        tbl_vlanclass_rule_fields,
        {
            tbl_vlanclass_rule_iterate,
            (TBL_DUMP_FUNC)tbl_vlanclass_rule_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlanclass_rule_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlanclass_rule_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlanclass_rule_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlanclass_rule_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlanclass_rule_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlanclass_rule_table_dump,
            (TBL_PARSER_FUNC)tbl_vlanclass_rule_table_parser,
        }
    },
    {
        sizeof(tbl_vlanclass_group_key_t),
        sizeof(tbl_vlanclass_group_t),
        sizeof(tbl_vlanclass_group_fields)/sizeof(cdb_field_t),
        tbl_vlanclass_group_fields,
        {
            tbl_vlanclass_group_iterate,
            (TBL_DUMP_FUNC)tbl_vlanclass_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlanclass_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlanclass_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlanclass_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlanclass_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlanclass_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlanclass_group_table_dump,
            (TBL_PARSER_FUNC)tbl_vlanclass_group_table_parser,
        }
    },
    {
        sizeof(tbl_acl_l4_port_range_key_t),
        sizeof(tbl_acl_l4_port_range_t),
        sizeof(tbl_acl_l4_port_range_fields)/sizeof(cdb_field_t),
        tbl_acl_l4_port_range_fields,
        {
            tbl_acl_l4_port_range_iterate,
            (TBL_DUMP_FUNC)tbl_acl_l4_port_range_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_l4_port_range_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_l4_port_range_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_l4_port_range_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_l4_port_range_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_l4_port_range_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_l4_port_range_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_l4_port_range_table_parser,
        }
    },
    {
        sizeof(tbl_acl_udf_group_key_t),
        sizeof(tbl_acl_udf_group_t),
        sizeof(tbl_acl_udf_group_fields)/sizeof(cdb_field_t),
        tbl_acl_udf_group_fields,
        {
            tbl_acl_udf_group_iterate,
            (TBL_DUMP_FUNC)tbl_acl_udf_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_udf_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_udf_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_udf_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_udf_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_udf_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_udf_group_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_udf_group_table_parser,
        }
    },
    {
        sizeof(tbl_acl_udf_entry_key_t),
        sizeof(tbl_acl_udf_entry_t),
        sizeof(tbl_acl_udf_entry_fields)/sizeof(cdb_field_t),
        tbl_acl_udf_entry_fields,
        {
            tbl_acl_udf_entry_iterate,
            (TBL_DUMP_FUNC)tbl_acl_udf_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_acl_udf_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_acl_udf_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_acl_udf_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_acl_udf_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_acl_udf_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_acl_udf_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_acl_udf_entry_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_fea_pcap_t),
        sizeof(tbl_fea_pcap_fields)/sizeof(cdb_field_t),
        tbl_fea_pcap_fields,
        {
            tbl_fea_pcap_iterate,
            (TBL_DUMP_FUNC)tbl_fea_pcap_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_pcap_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_pcap_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_pcap_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_pcap_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_pcap_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_pcap_table_parser,
        }
    },
    {
        sizeof(tbl_controller_key_t),
        sizeof(tbl_controller_t),
        sizeof(tbl_controller_fields)/sizeof(cdb_field_t),
        tbl_controller_fields,
        {
            tbl_controller_iterate,
            (TBL_DUMP_FUNC)tbl_controller_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_controller_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_controller_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_controller_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_controller_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_controller_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_controller_table_dump,
            (TBL_PARSER_FUNC)tbl_controller_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_cpu_packets_t),
        sizeof(tbl_cpu_packets_fields)/sizeof(cdb_field_t),
        tbl_cpu_packets_fields,
        {
            tbl_cpu_packets_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_packets_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_packets_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_packets_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_packets_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_packets_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_packets_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_packets_table_parser,
        }
    },
    {
        sizeof(tbl_ns_route_key_t),
        sizeof(tbl_ns_route_t),
        sizeof(tbl_ns_route_fields)/sizeof(cdb_field_t),
        tbl_ns_route_fields,
        {
            tbl_ns_route_iterate,
            (TBL_DUMP_FUNC)tbl_ns_route_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ns_route_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ns_route_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ns_route_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ns_route_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ns_route_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ns_route_table_dump,
            (TBL_PARSER_FUNC)tbl_ns_route_table_parser,
        }
    },
    {
        sizeof(tbl_ns_route_ip_key_t),
        sizeof(tbl_ns_route_ip_t),
        sizeof(tbl_ns_route_ip_fields)/sizeof(cdb_field_t),
        tbl_ns_route_ip_fields,
        {
            tbl_ns_route_ip_iterate,
            (TBL_DUMP_FUNC)tbl_ns_route_ip_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ns_route_ip_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ns_route_ip_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ns_route_ip_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ns_route_ip_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ns_route_ip_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ns_route_ip_table_dump,
            (TBL_PARSER_FUNC)tbl_ns_route_ip_table_parser,
        }
    },
    {
        sizeof(tbl_openflow_interface_key_t),
        sizeof(tbl_openflow_interface_t),
        sizeof(tbl_openflow_interface_fields)/sizeof(cdb_field_t),
        tbl_openflow_interface_fields,
        {
            tbl_openflow_interface_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_interface_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_openflow_interface_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_interface_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_interface_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_interface_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_interface_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_interface_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_interface_table_parser,
        }
    },
    {
        sizeof(tbl_openflow_manager_key_t),
        sizeof(tbl_openflow_manager_t),
        sizeof(tbl_openflow_manager_fields)/sizeof(cdb_field_t),
        tbl_openflow_manager_fields,
        {
            tbl_openflow_manager_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_manager_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_openflow_manager_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_manager_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_manager_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_manager_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_manager_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_manager_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_manager_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ptp_global_t),
        sizeof(tbl_ptp_global_fields)/sizeof(cdb_field_t),
        tbl_ptp_global_fields,
        {
            tbl_ptp_global_iterate,
            (TBL_DUMP_FUNC)tbl_ptp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ptp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ptp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ptp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ptp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ptp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_ptp_global_table_parser,
        }
    },
    {
        sizeof(tbl_ptp_port_key_t),
        sizeof(tbl_ptp_port_t),
        sizeof(tbl_ptp_port_fields)/sizeof(cdb_field_t),
        tbl_ptp_port_fields,
        {
            tbl_ptp_port_iterate,
            (TBL_DUMP_FUNC)tbl_ptp_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ptp_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ptp_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ptp_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ptp_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ptp_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ptp_port_table_dump,
            (TBL_PARSER_FUNC)tbl_ptp_port_table_parser,
        }
    },
    {
        sizeof(tbl_ptp_foreign_key_t),
        sizeof(tbl_ptp_foreign_t),
        sizeof(tbl_ptp_foreign_fields)/sizeof(cdb_field_t),
        tbl_ptp_foreign_fields,
        {
            tbl_ptp_foreign_iterate,
            (TBL_DUMP_FUNC)tbl_ptp_foreign_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ptp_foreign_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ptp_foreign_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ptp_foreign_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ptp_foreign_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ptp_foreign_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ptp_foreign_table_dump,
            (TBL_PARSER_FUNC)tbl_ptp_foreign_table_parser,
        }
    },
    {
        sizeof(tbl_ptp_residence_time_cache_key_t),
        sizeof(tbl_ptp_residence_time_cache_t),
        sizeof(tbl_ptp_residence_time_cache_fields)/sizeof(cdb_field_t),
        tbl_ptp_residence_time_cache_fields,
        {
            tbl_ptp_residence_time_cache_iterate,
            (TBL_DUMP_FUNC)tbl_ptp_residence_time_cache_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ptp_residence_time_cache_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ptp_residence_time_cache_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ptp_residence_time_cache_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ptp_residence_time_cache_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ptp_residence_time_cache_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ptp_residence_time_cache_table_dump,
            (TBL_PARSER_FUNC)tbl_ptp_residence_time_cache_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_fea_ptp_global_t),
        sizeof(tbl_fea_ptp_global_fields)/sizeof(cdb_field_t),
        tbl_fea_ptp_global_fields,
        {
            tbl_fea_ptp_global_iterate,
            (TBL_DUMP_FUNC)tbl_fea_ptp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_ptp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_ptp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_ptp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_ptp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_ptp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_ptp_global_table_parser,
        }
    },
    {
        sizeof(tbl_fea_ptp_ts_key_t),
        sizeof(tbl_fea_ptp_ts_t),
        sizeof(tbl_fea_ptp_ts_fields)/sizeof(cdb_field_t),
        tbl_fea_ptp_ts_fields,
        {
            tbl_fea_ptp_ts_iterate,
            (TBL_DUMP_FUNC)tbl_fea_ptp_ts_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_ptp_ts_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_ptp_ts_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_ptp_ts_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_ptp_ts_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_ptp_ts_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_ptp_ts_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_ptp_ts_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_fea_time_t),
        sizeof(tbl_fea_time_fields)/sizeof(cdb_field_t),
        tbl_fea_time_fields,
        {
            tbl_fea_time_iterate,
            (TBL_DUMP_FUNC)tbl_fea_time_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_time_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_time_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_time_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_time_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_time_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_time_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_bhm_global_t),
        sizeof(tbl_bhm_global_fields)/sizeof(cdb_field_t),
        tbl_bhm_global_fields,
        {
            tbl_bhm_global_iterate,
            (TBL_DUMP_FUNC)tbl_bhm_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bhm_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bhm_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bhm_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bhm_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bhm_global_table_dump,
            (TBL_PARSER_FUNC)tbl_bhm_global_table_parser,
        }
    },
    {
        sizeof(tbl_bhm_module_key_t),
        sizeof(tbl_bhm_module_t),
        sizeof(tbl_bhm_module_fields)/sizeof(cdb_field_t),
        tbl_bhm_module_fields,
        {
            tbl_bhm_module_iterate,
            (TBL_DUMP_FUNC)tbl_bhm_module_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_bhm_module_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_bhm_module_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_bhm_module_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_bhm_module_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_bhm_module_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_bhm_module_table_dump,
            (TBL_PARSER_FUNC)tbl_bhm_module_table_parser,
        }
    },
    {
        sizeof(tbl_openflow_tunnel_interface_key_t),
        sizeof(tbl_openflow_tunnel_interface_t),
        sizeof(tbl_openflow_tunnel_interface_fields)/sizeof(cdb_field_t),
        tbl_openflow_tunnel_interface_fields,
        {
            tbl_openflow_tunnel_interface_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_tunnel_interface_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_openflow_tunnel_interface_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_tunnel_interface_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_tunnel_interface_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_tunnel_interface_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_tunnel_interface_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_tunnel_interface_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_tunnel_interface_table_parser,
        }
    },
    {
        sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t),
        sizeof(tbl_openflow_tunnel_local_ip_cnt_t),
        sizeof(tbl_openflow_tunnel_local_ip_cnt_fields)/sizeof(cdb_field_t),
        tbl_openflow_tunnel_local_ip_cnt_fields,
        {
            tbl_openflow_tunnel_local_ip_cnt_iterate,
            (TBL_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_openflow_tunnel_local_ip_cnt_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_openflow_tunnel_local_ip_cnt_table_dump,
            (TBL_PARSER_FUNC)tbl_openflow_tunnel_local_ip_cnt_table_parser,
        }
    },
    {
        sizeof(tbl_inband_snat_key_t),
        sizeof(tbl_inband_snat_t),
        sizeof(tbl_inband_snat_fields)/sizeof(cdb_field_t),
        tbl_inband_snat_fields,
        {
            tbl_inband_snat_iterate,
            (TBL_DUMP_FUNC)tbl_inband_snat_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_inband_snat_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_inband_snat_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_inband_snat_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_inband_snat_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_inband_snat_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_inband_snat_table_dump,
            (TBL_PARSER_FUNC)tbl_inband_snat_table_parser,
        }
    },
    {
        sizeof(tbl_arpinsp_key_t),
        sizeof(tbl_arpinsp_t),
        sizeof(tbl_arpinsp_fields)/sizeof(cdb_field_t),
        tbl_arpinsp_fields,
        {
            tbl_arpinsp_iterate,
            (TBL_DUMP_FUNC)tbl_arpinsp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_arpinsp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_arpinsp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_arpinsp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_arpinsp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_arpinsp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_arpinsp_table_dump,
            (TBL_PARSER_FUNC)tbl_arpinsp_table_parser,
        }
    },
    {
        sizeof(tbl_arpacl_config_key_t),
        sizeof(tbl_arpacl_config_t),
        sizeof(tbl_arpacl_config_fields)/sizeof(cdb_field_t),
        tbl_arpacl_config_fields,
        {
            tbl_arpacl_config_iterate,
            (TBL_DUMP_FUNC)tbl_arpacl_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_arpacl_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_arpacl_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_arpacl_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_arpacl_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_arpacl_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_arpacl_config_table_dump,
            (TBL_PARSER_FUNC)tbl_arpacl_config_table_parser,
        }
    },
    {
        sizeof(tbl_arpace_config_key_t),
        sizeof(tbl_arpace_config_t),
        sizeof(tbl_arpace_config_fields)/sizeof(cdb_field_t),
        tbl_arpace_config_fields,
        {
            tbl_arpace_config_iterate,
            (TBL_DUMP_FUNC)tbl_arpace_config_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_arpace_config_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_arpace_config_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_arpace_config_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_arpace_config_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_arpace_config_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_arpace_config_table_dump,
            (TBL_PARSER_FUNC)tbl_arpace_config_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_copp_cfg_t),
        sizeof(tbl_copp_cfg_fields)/sizeof(cdb_field_t),
        tbl_copp_cfg_fields,
        {
            tbl_copp_cfg_iterate,
            (TBL_DUMP_FUNC)tbl_copp_cfg_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_copp_cfg_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_copp_cfg_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_copp_cfg_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_copp_cfg_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_copp_cfg_table_dump,
            (TBL_PARSER_FUNC)tbl_copp_cfg_table_parser,
        }
    },
    {
        sizeof(tbl_fea_copp_trap_group_key_t),
        sizeof(tbl_fea_copp_trap_group_t),
        sizeof(tbl_fea_copp_trap_group_fields)/sizeof(cdb_field_t),
        tbl_fea_copp_trap_group_fields,
        {
            tbl_fea_copp_trap_group_iterate,
            (TBL_DUMP_FUNC)tbl_fea_copp_trap_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_fea_copp_trap_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_fea_copp_trap_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_fea_copp_trap_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_fea_copp_trap_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_fea_copp_trap_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_fea_copp_trap_group_table_dump,
            (TBL_PARSER_FUNC)tbl_fea_copp_trap_group_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_sflow_global_t),
        sizeof(tbl_sflow_global_fields)/sizeof(cdb_field_t),
        tbl_sflow_global_fields,
        {
            tbl_sflow_global_iterate,
            (TBL_DUMP_FUNC)tbl_sflow_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sflow_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sflow_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sflow_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sflow_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sflow_global_table_dump,
            (TBL_PARSER_FUNC)tbl_sflow_global_table_parser,
        }
    },
    {
        sizeof(tbl_sflow_collector_key_t),
        sizeof(tbl_sflow_collector_t),
        sizeof(tbl_sflow_collector_fields)/sizeof(cdb_field_t),
        tbl_sflow_collector_fields,
        {
            tbl_sflow_collector_iterate,
            (TBL_DUMP_FUNC)tbl_sflow_collector_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_sflow_collector_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sflow_collector_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sflow_collector_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sflow_collector_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sflow_collector_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sflow_collector_table_dump,
            (TBL_PARSER_FUNC)tbl_sflow_collector_table_parser,
        }
    },
    {
        sizeof(tbl_sflow_counter_port_key_t),
        sizeof(tbl_sflow_counter_port_t),
        sizeof(tbl_sflow_counter_port_fields)/sizeof(cdb_field_t),
        tbl_sflow_counter_port_fields,
        {
            tbl_sflow_counter_port_iterate,
            (TBL_DUMP_FUNC)tbl_sflow_counter_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_sflow_counter_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_sflow_counter_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_sflow_counter_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_sflow_counter_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_sflow_counter_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_sflow_counter_port_table_dump,
            (TBL_PARSER_FUNC)tbl_sflow_counter_port_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_vrrp_global_t),
        sizeof(tbl_vrrp_global_fields)/sizeof(cdb_field_t),
        tbl_vrrp_global_fields,
        {
            tbl_vrrp_global_iterate,
            (TBL_DUMP_FUNC)tbl_vrrp_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vrrp_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vrrp_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vrrp_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vrrp_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vrrp_global_table_dump,
            (TBL_PARSER_FUNC)tbl_vrrp_global_table_parser,
        }
    },
    {
        sizeof(tbl_vrrp_vmac_key_t),
        sizeof(tbl_vrrp_vmac_t),
        sizeof(tbl_vrrp_vmac_fields)/sizeof(cdb_field_t),
        tbl_vrrp_vmac_fields,
        {
            tbl_vrrp_vmac_iterate,
            (TBL_DUMP_FUNC)tbl_vrrp_vmac_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vrrp_vmac_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vrrp_vmac_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vrrp_vmac_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vrrp_vmac_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vrrp_vmac_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vrrp_vmac_table_dump,
            (TBL_PARSER_FUNC)tbl_vrrp_vmac_table_parser,
        }
    },
    {
        sizeof(tbl_vrrp_vip_key_t),
        sizeof(tbl_vrrp_vip_t),
        sizeof(tbl_vrrp_vip_fields)/sizeof(cdb_field_t),
        tbl_vrrp_vip_fields,
        {
            tbl_vrrp_vip_iterate,
            (TBL_DUMP_FUNC)tbl_vrrp_vip_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vrrp_vip_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vrrp_vip_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vrrp_vip_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vrrp_vip_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vrrp_vip_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vrrp_vip_table_dump,
            (TBL_PARSER_FUNC)tbl_vrrp_vip_table_parser,
        }
    },
    {
        sizeof(tbl_vrrp_session_key_t),
        sizeof(tbl_vrrp_session_t),
        sizeof(tbl_vrrp_session_fields)/sizeof(cdb_field_t),
        tbl_vrrp_session_fields,
        {
            tbl_vrrp_session_iterate,
            (TBL_DUMP_FUNC)tbl_vrrp_session_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vrrp_session_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vrrp_session_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vrrp_session_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vrrp_session_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vrrp_session_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vrrp_session_table_dump,
            (TBL_PARSER_FUNC)tbl_vrrp_session_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ssm_global_t),
        sizeof(tbl_ssm_global_fields)/sizeof(cdb_field_t),
        tbl_ssm_global_fields,
        {
            tbl_ssm_global_iterate,
            (TBL_DUMP_FUNC)tbl_ssm_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ssm_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ssm_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ssm_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ssm_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ssm_global_table_dump,
            (TBL_PARSER_FUNC)tbl_ssm_global_table_parser,
        }
    },
    {
        sizeof(tbl_ssm_port_key_t),
        sizeof(tbl_ssm_port_t),
        sizeof(tbl_ssm_port_fields)/sizeof(cdb_field_t),
        tbl_ssm_port_fields,
        {
            tbl_ssm_port_iterate,
            (TBL_DUMP_FUNC)tbl_ssm_port_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ssm_port_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ssm_port_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ssm_port_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ssm_port_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ssm_port_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ssm_port_table_dump,
            (TBL_PARSER_FUNC)tbl_ssm_port_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_ipsla_global_t),
        sizeof(tbl_ipsla_global_fields)/sizeof(cdb_field_t),
        tbl_ipsla_global_fields,
        {
            tbl_ipsla_global_iterate,
            (TBL_DUMP_FUNC)tbl_ipsla_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsla_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsla_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsla_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsla_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsla_global_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsla_global_table_parser,
        }
    },
    {
        sizeof(tbl_ipsla_entry_key_t),
        sizeof(tbl_ipsla_entry_t),
        sizeof(tbl_ipsla_entry_fields)/sizeof(cdb_field_t),
        tbl_ipsla_entry_fields,
        {
            tbl_ipsla_entry_iterate,
            (TBL_DUMP_FUNC)tbl_ipsla_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsla_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsla_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsla_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsla_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsla_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsla_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsla_entry_table_parser,
        }
    },
    {
        sizeof(tbl_ipsla_packet_stats_key_t),
        sizeof(tbl_ipsla_packet_stats_t),
        sizeof(tbl_ipsla_packet_stats_fields)/sizeof(cdb_field_t),
        tbl_ipsla_packet_stats_fields,
        {
            tbl_ipsla_packet_stats_iterate,
            (TBL_DUMP_FUNC)tbl_ipsla_packet_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsla_packet_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsla_packet_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsla_packet_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsla_packet_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsla_packet_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsla_packet_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsla_packet_stats_table_parser,
        }
    },
    {
        sizeof(tbl_ipsla_test_stats_key_t),
        sizeof(tbl_ipsla_test_stats_t),
        sizeof(tbl_ipsla_test_stats_fields)/sizeof(cdb_field_t),
        tbl_ipsla_test_stats_fields,
        {
            tbl_ipsla_test_stats_iterate,
            (TBL_DUMP_FUNC)tbl_ipsla_test_stats_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ipsla_test_stats_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ipsla_test_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ipsla_test_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ipsla_test_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ipsla_test_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ipsla_test_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_ipsla_test_stats_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_track_global_t),
        sizeof(tbl_track_global_fields)/sizeof(cdb_field_t),
        tbl_track_global_fields,
        {
            tbl_track_global_iterate,
            (TBL_DUMP_FUNC)tbl_track_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_track_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_track_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_track_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_track_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_track_global_table_dump,
            (TBL_PARSER_FUNC)tbl_track_global_table_parser,
        }
    },
    {
        sizeof(tbl_track_object_key_t),
        sizeof(tbl_track_object_t),
        sizeof(tbl_track_object_fields)/sizeof(cdb_field_t),
        tbl_track_object_fields,
        {
            tbl_track_object_iterate,
            (TBL_DUMP_FUNC)tbl_track_object_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_track_object_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_track_object_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_track_object_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_track_object_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_track_object_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_track_object_table_dump,
            (TBL_PARSER_FUNC)tbl_track_object_table_parser,
        }
    },
    {
        sizeof(tbl_controller_attr_key_t),
        sizeof(tbl_controller_attr_t),
        sizeof(tbl_controller_attr_fields)/sizeof(cdb_field_t),
        tbl_controller_attr_fields,
        {
            tbl_controller_attr_iterate,
            (TBL_DUMP_FUNC)tbl_controller_attr_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_controller_attr_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_controller_attr_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_controller_attr_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_controller_attr_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_controller_attr_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_controller_attr_table_dump,
            (TBL_PARSER_FUNC)tbl_controller_attr_table_parser,
        }
    },
    {
        sizeof(tbl_oam_session_key_t),
        sizeof(tbl_oam_session_t),
        sizeof(tbl_oam_session_fields)/sizeof(cdb_field_t),
        tbl_oam_session_fields,
        {
            tbl_oam_session_iterate,
            (TBL_DUMP_FUNC)tbl_oam_session_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_oam_session_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_oam_session_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_oam_session_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_oam_session_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_oam_session_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_oam_session_table_dump,
            (TBL_PARSER_FUNC)tbl_oam_session_table_parser,
        }
    },
    {
        sizeof(tbl_oam_session_upload_key_t),
        sizeof(tbl_oam_session_upload_t),
        sizeof(tbl_oam_session_upload_fields)/sizeof(cdb_field_t),
        tbl_oam_session_upload_fields,
        {
            tbl_oam_session_upload_iterate,
            (TBL_DUMP_FUNC)tbl_oam_session_upload_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_oam_session_upload_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_oam_session_upload_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_oam_session_upload_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_oam_session_upload_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_oam_session_upload_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_oam_session_upload_table_dump,
            (TBL_PARSER_FUNC)tbl_oam_session_upload_table_parser,
        }
    },
    {
        sizeof(tbl_lsp_aps_group_key_t),
        sizeof(tbl_lsp_aps_group_t),
        sizeof(tbl_lsp_aps_group_fields)/sizeof(cdb_field_t),
        tbl_lsp_aps_group_fields,
        {
            tbl_lsp_aps_group_iterate,
            (TBL_DUMP_FUNC)tbl_lsp_aps_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_lsp_aps_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lsp_aps_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lsp_aps_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lsp_aps_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lsp_aps_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lsp_aps_group_table_dump,
            (TBL_PARSER_FUNC)tbl_lsp_aps_group_table_parser,
        }
    },
    {
        sizeof(tbl_pw_aps_group_key_t),
        sizeof(tbl_pw_aps_group_t),
        sizeof(tbl_pw_aps_group_fields)/sizeof(cdb_field_t),
        tbl_pw_aps_group_fields,
        {
            tbl_pw_aps_group_iterate,
            (TBL_DUMP_FUNC)tbl_pw_aps_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pw_aps_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pw_aps_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pw_aps_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pw_aps_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pw_aps_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pw_aps_group_table_dump,
            (TBL_PARSER_FUNC)tbl_pw_aps_group_table_parser,
        }
    },
    {
        sizeof(tbl_lsp_pe_key_t),
        sizeof(tbl_lsp_pe_t),
        sizeof(tbl_lsp_pe_fields)/sizeof(cdb_field_t),
        tbl_lsp_pe_fields,
        {
            tbl_lsp_pe_iterate,
            (TBL_DUMP_FUNC)tbl_lsp_pe_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_lsp_pe_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lsp_pe_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lsp_pe_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lsp_pe_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lsp_pe_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lsp_pe_table_dump,
            (TBL_PARSER_FUNC)tbl_lsp_pe_table_parser,
        }
    },
    {
        sizeof(tbl_lsp_p_key_t),
        sizeof(tbl_lsp_p_t),
        sizeof(tbl_lsp_p_fields)/sizeof(cdb_field_t),
        tbl_lsp_p_fields,
        {
            tbl_lsp_p_iterate,
            (TBL_DUMP_FUNC)tbl_lsp_p_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_lsp_p_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_lsp_p_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_lsp_p_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_lsp_p_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_lsp_p_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_lsp_p_table_dump,
            (TBL_PARSER_FUNC)tbl_lsp_p_table_parser,
        }
    },
    {
        sizeof(tbl_pw_tpe_key_t),
        sizeof(tbl_pw_tpe_t),
        sizeof(tbl_pw_tpe_fields)/sizeof(cdb_field_t),
        tbl_pw_tpe_fields,
        {
            tbl_pw_tpe_iterate,
            (TBL_DUMP_FUNC)tbl_pw_tpe_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pw_tpe_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pw_tpe_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pw_tpe_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pw_tpe_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pw_tpe_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pw_tpe_table_dump,
            (TBL_PARSER_FUNC)tbl_pw_tpe_table_parser,
        }
    },
    {
        sizeof(tbl_section_oam_key_t),
        sizeof(tbl_section_oam_t),
        sizeof(tbl_section_oam_fields)/sizeof(cdb_field_t),
        tbl_section_oam_fields,
        {
            tbl_section_oam_iterate,
            (TBL_DUMP_FUNC)tbl_section_oam_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_section_oam_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_section_oam_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_section_oam_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_section_oam_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_section_oam_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_section_oam_table_dump,
            (TBL_PARSER_FUNC)tbl_section_oam_table_parser,
        }
    },
    {
        sizeof(tbl_oam_flow_info_key_t),
        sizeof(tbl_oam_flow_info_t),
        sizeof(tbl_oam_flow_info_fields)/sizeof(cdb_field_t),
        tbl_oam_flow_info_fields,
        {
            tbl_oam_flow_info_iterate,
            (TBL_DUMP_FUNC)tbl_oam_flow_info_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_oam_flow_info_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_oam_flow_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_oam_flow_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_oam_flow_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_oam_flow_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_oam_flow_info_table_dump,
            (TBL_PARSER_FUNC)tbl_oam_flow_info_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_tpoam_global_t),
        sizeof(tbl_tpoam_global_fields)/sizeof(cdb_field_t),
        tbl_tpoam_global_fields,
        {
            tbl_tpoam_global_iterate,
            (TBL_DUMP_FUNC)tbl_tpoam_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tpoam_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tpoam_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tpoam_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tpoam_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tpoam_global_table_dump,
            (TBL_PARSER_FUNC)tbl_tpoam_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_tpoam_exec_global_t),
        sizeof(tbl_tpoam_exec_global_fields)/sizeof(cdb_field_t),
        tbl_tpoam_exec_global_fields,
        {
            tbl_tpoam_exec_global_iterate,
            (TBL_DUMP_FUNC)tbl_tpoam_exec_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tpoam_exec_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tpoam_exec_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tpoam_exec_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tpoam_exec_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tpoam_exec_global_table_dump,
            (TBL_PARSER_FUNC)tbl_tpoam_exec_global_table_parser,
        }
    },
    {
        sizeof(tbl_aps_group_info_key_t),
        sizeof(tbl_aps_group_info_t),
        sizeof(tbl_aps_group_info_fields)/sizeof(cdb_field_t),
        tbl_aps_group_info_fields,
        {
            tbl_aps_group_info_iterate,
            (TBL_DUMP_FUNC)tbl_aps_group_info_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_aps_group_info_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_aps_group_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_aps_group_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_aps_group_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_aps_group_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_aps_group_info_table_dump,
            (TBL_PARSER_FUNC)tbl_aps_group_info_table_parser,
        }
    },
    {
        sizeof(tbl_oam_inlabel_info_key_t),
        sizeof(tbl_oam_inlabel_info_t),
        sizeof(tbl_oam_inlabel_info_fields)/sizeof(cdb_field_t),
        tbl_oam_inlabel_info_fields,
        {
            tbl_oam_inlabel_info_iterate,
            (TBL_DUMP_FUNC)tbl_oam_inlabel_info_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_oam_inlabel_info_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_oam_inlabel_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_oam_inlabel_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_oam_inlabel_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_oam_inlabel_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_oam_inlabel_info_table_dump,
            (TBL_PARSER_FUNC)tbl_oam_inlabel_info_table_parser,
        }
    },
    {
        sizeof(tbl_oam_mpls_aps_check_key_t),
        sizeof(tbl_oam_mpls_aps_check_t),
        sizeof(tbl_oam_mpls_aps_check_fields)/sizeof(cdb_field_t),
        tbl_oam_mpls_aps_check_fields,
        {
            tbl_oam_mpls_aps_check_iterate,
            (TBL_DUMP_FUNC)tbl_oam_mpls_aps_check_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_oam_mpls_aps_check_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_oam_mpls_aps_check_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_oam_mpls_aps_check_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_oam_mpls_aps_check_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_oam_mpls_aps_check_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_oam_mpls_aps_check_table_dump,
            (TBL_PARSER_FUNC)tbl_oam_mpls_aps_check_table_parser,
        }
    },
    {
        sizeof(tbl_vlan_mapping_table_key_t),
        sizeof(tbl_vlan_mapping_table_t),
        sizeof(tbl_vlan_mapping_table_fields)/sizeof(cdb_field_t),
        tbl_vlan_mapping_table_fields,
        {
            tbl_vlan_mapping_table_iterate,
            (TBL_DUMP_FUNC)tbl_vlan_mapping_table_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlan_mapping_table_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlan_mapping_table_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlan_mapping_table_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlan_mapping_table_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlan_mapping_table_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlan_mapping_table_table_dump,
            (TBL_PARSER_FUNC)tbl_vlan_mapping_table_table_parser,
        }
    },
    {
        sizeof(tbl_vlan_mapping_entry_key_t),
        sizeof(tbl_vlan_mapping_entry_t),
        sizeof(tbl_vlan_mapping_entry_fields)/sizeof(cdb_field_t),
        tbl_vlan_mapping_entry_fields,
        {
            tbl_vlan_mapping_entry_iterate,
            (TBL_DUMP_FUNC)tbl_vlan_mapping_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlan_mapping_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlan_mapping_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlan_mapping_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlan_mapping_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlan_mapping_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlan_mapping_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_vlan_mapping_entry_table_parser,
        }
    },
    {
        sizeof(tbl_egress_mapping_entry_key_t),
        sizeof(tbl_egress_mapping_entry_t),
        sizeof(tbl_egress_mapping_entry_fields)/sizeof(cdb_field_t),
        tbl_egress_mapping_entry_fields,
        {
            tbl_egress_mapping_entry_iterate,
            (TBL_DUMP_FUNC)tbl_egress_mapping_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_egress_mapping_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_egress_mapping_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_egress_mapping_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_egress_mapping_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_egress_mapping_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_egress_mapping_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_egress_mapping_entry_table_parser,
        }
    },
    {
        sizeof(tbl_vlan_group_key_t),
        sizeof(tbl_vlan_group_t),
        sizeof(tbl_vlan_group_fields)/sizeof(cdb_field_t),
        tbl_vlan_group_fields,
        {
            tbl_vlan_group_iterate,
            (TBL_DUMP_FUNC)tbl_vlan_group_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_vlan_group_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_vlan_group_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_vlan_group_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_vlan_group_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_vlan_group_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_vlan_group_table_dump,
            (TBL_PARSER_FUNC)tbl_vlan_group_table_parser,
        }
    },
    {
        sizeof(tbl_evc_entry_key_t),
        sizeof(tbl_evc_entry_t),
        sizeof(tbl_evc_entry_fields)/sizeof(cdb_field_t),
        tbl_evc_entry_fields,
        {
            tbl_evc_entry_iterate,
            (TBL_DUMP_FUNC)tbl_evc_entry_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_evc_entry_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_evc_entry_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_evc_entry_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_evc_entry_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_evc_entry_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_evc_entry_table_dump,
            (TBL_PARSER_FUNC)tbl_evc_entry_table_parser,
        }
    },
    {
        sizeof(tbl_mapped_vlan_info_key_t),
        sizeof(tbl_mapped_vlan_info_t),
        sizeof(tbl_mapped_vlan_info_fields)/sizeof(cdb_field_t),
        tbl_mapped_vlan_info_fields,
        {
            tbl_mapped_vlan_info_iterate,
            (TBL_DUMP_FUNC)tbl_mapped_vlan_info_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_mapped_vlan_info_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_mapped_vlan_info_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_mapped_vlan_info_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_mapped_vlan_info_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_mapped_vlan_info_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_mapped_vlan_info_table_dump,
            (TBL_PARSER_FUNC)tbl_mapped_vlan_info_table_parser,
        }
    },
    {
        sizeof(tbl_l2_protocol_key_t),
        sizeof(tbl_l2_protocol_t),
        sizeof(tbl_l2_protocol_fields)/sizeof(cdb_field_t),
        tbl_l2_protocol_fields,
        {
            tbl_l2_protocol_iterate,
            (TBL_DUMP_FUNC)tbl_l2_protocol_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_l2_protocol_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_l2_protocol_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_l2_protocol_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_l2_protocol_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_l2_protocol_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_l2_protocol_table_dump,
            (TBL_PARSER_FUNC)tbl_l2_protocol_table_parser,
        }
    },
    {
        sizeof(tbl_nd_fib_key_t),
        sizeof(tbl_nd_fib_t),
        sizeof(tbl_nd_fib_fields)/sizeof(cdb_field_t),
        tbl_nd_fib_fields,
        {
            tbl_nd_fib_iterate,
            (TBL_DUMP_FUNC)tbl_nd_fib_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nd_fib_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nd_fib_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nd_fib_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nd_fib_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nd_fib_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nd_fib_table_dump,
            (TBL_PARSER_FUNC)tbl_nd_fib_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_nd_stats_t),
        sizeof(tbl_nd_stats_fields)/sizeof(cdb_field_t),
        tbl_nd_stats_fields,
        {
            tbl_nd_stats_iterate,
            (TBL_DUMP_FUNC)tbl_nd_stats_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nd_stats_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nd_stats_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nd_stats_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nd_stats_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nd_stats_table_dump,
            (TBL_PARSER_FUNC)tbl_nd_stats_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_cpu_mirror_global_t),
        sizeof(tbl_cpu_mirror_global_fields)/sizeof(cdb_field_t),
        tbl_cpu_mirror_global_fields,
        {
            tbl_cpu_mirror_global_iterate,
            (TBL_DUMP_FUNC)tbl_cpu_mirror_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cpu_mirror_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cpu_mirror_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cpu_mirror_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cpu_mirror_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cpu_mirror_global_table_dump,
            (TBL_PARSER_FUNC)tbl_cpu_mirror_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_tap_server_t),
        sizeof(tbl_tap_server_fields)/sizeof(cdb_field_t),
        tbl_tap_server_fields,
        {
            tbl_tap_server_iterate,
            (TBL_DUMP_FUNC)tbl_tap_server_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_tap_server_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_tap_server_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_tap_server_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_tap_server_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_tap_server_table_dump,
            (TBL_PARSER_FUNC)tbl_tap_server_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_dhcpv6_debug_t),
        sizeof(tbl_dhcpv6_debug_fields)/sizeof(cdb_field_t),
        tbl_dhcpv6_debug_fields,
        {
            tbl_dhcpv6_debug_iterate,
            (TBL_DUMP_FUNC)tbl_dhcpv6_debug_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_dhcpv6_debug_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_dhcpv6_debug_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_dhcpv6_debug_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_dhcpv6_debug_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_dhcpv6_debug_table_dump,
            (TBL_PARSER_FUNC)tbl_dhcpv6_debug_table_parser,
        }
    },
    {
        sizeof(tbl_l2edit_key_t),
        sizeof(tbl_l2edit_t),
        sizeof(tbl_l2edit_fields)/sizeof(cdb_field_t),
        tbl_l2edit_fields,
        {
            tbl_l2edit_iterate,
            (TBL_DUMP_FUNC)tbl_l2edit_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_l2edit_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_l2edit_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_l2edit_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_l2edit_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_l2edit_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_l2edit_table_dump,
            (TBL_PARSER_FUNC)tbl_l2edit_table_parser,
        }
    },
    {
        sizeof(tbl_hash_field_profile_key_t),
        sizeof(tbl_hash_field_profile_t),
        sizeof(tbl_hash_field_profile_fields)/sizeof(cdb_field_t),
        tbl_hash_field_profile_fields,
        {
            tbl_hash_field_profile_iterate,
            (TBL_DUMP_FUNC)tbl_hash_field_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_hash_field_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_hash_field_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_hash_field_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_hash_field_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_hash_field_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_hash_field_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_hash_field_profile_table_parser,
        }
    },
    {
        sizeof(tbl_hash_value_profile_key_t),
        sizeof(tbl_hash_value_profile_t),
        sizeof(tbl_hash_value_profile_fields)/sizeof(cdb_field_t),
        tbl_hash_value_profile_fields,
        {
            tbl_hash_value_profile_iterate,
            (TBL_DUMP_FUNC)tbl_hash_value_profile_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_hash_value_profile_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_hash_value_profile_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_hash_value_profile_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_hash_value_profile_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_hash_value_profile_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_hash_value_profile_table_dump,
            (TBL_PARSER_FUNC)tbl_hash_value_profile_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_hash_value_global_t),
        sizeof(tbl_hash_value_global_fields)/sizeof(cdb_field_t),
        tbl_hash_value_global_fields,
        {
            tbl_hash_value_global_iterate,
            (TBL_DUMP_FUNC)tbl_hash_value_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_hash_value_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_hash_value_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_hash_value_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_hash_value_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_hash_value_global_table_dump,
            (TBL_PARSER_FUNC)tbl_hash_value_global_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_nat_global_t),
        sizeof(tbl_nat_global_fields)/sizeof(cdb_field_t),
        tbl_nat_global_fields,
        {
            tbl_nat_global_iterate,
            (TBL_DUMP_FUNC)tbl_nat_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_global_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_global_table_parser,
        }
    },
    {
        sizeof(tbl_nat_pool_key_t),
        sizeof(tbl_nat_pool_t),
        sizeof(tbl_nat_pool_fields)/sizeof(cdb_field_t),
        tbl_nat_pool_fields,
        {
            tbl_nat_pool_iterate,
            (TBL_DUMP_FUNC)tbl_nat_pool_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_pool_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_pool_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_pool_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_pool_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_pool_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_pool_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_pool_table_parser,
        }
    },
    {
        sizeof(tbl_nat_rule_key_t),
        sizeof(tbl_nat_rule_t),
        sizeof(tbl_nat_rule_fields)/sizeof(cdb_field_t),
        tbl_nat_rule_fields,
        {
            tbl_nat_rule_iterate,
            (TBL_DUMP_FUNC)tbl_nat_rule_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_rule_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_rule_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_rule_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_rule_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_rule_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_rule_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_rule_table_parser,
        }
    },
    {
        sizeof(tbl_nat_session_key_t),
        sizeof(tbl_nat_session_t),
        sizeof(tbl_nat_session_fields)/sizeof(cdb_field_t),
        tbl_nat_session_fields,
        {
            tbl_nat_session_iterate,
            (TBL_DUMP_FUNC)tbl_nat_session_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_session_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_session_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_session_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_session_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_session_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_session_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_session_table_parser,
        }
    },
    {
        sizeof(tbl_nat_session_limit_key_t),
        sizeof(tbl_nat_session_limit_t),
        sizeof(tbl_nat_session_limit_fields)/sizeof(cdb_field_t),
        tbl_nat_session_limit_fields,
        {
            tbl_nat_session_limit_iterate,
            (TBL_DUMP_FUNC)tbl_nat_session_limit_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_session_limit_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_session_limit_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_session_limit_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_session_limit_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_session_limit_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_session_limit_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_session_limit_table_parser,
        }
    },
    {
        sizeof(tbl_nat_proxy_arp_key_t),
        sizeof(tbl_nat_proxy_arp_t),
        sizeof(tbl_nat_proxy_arp_fields)/sizeof(cdb_field_t),
        tbl_nat_proxy_arp_fields,
        {
            tbl_nat_proxy_arp_iterate,
            (TBL_DUMP_FUNC)tbl_nat_proxy_arp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_proxy_arp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_proxy_arp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_proxy_arp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_proxy_arp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_proxy_arp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_proxy_arp_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_proxy_arp_table_parser,
        }
    },
    {
        sizeof(tbl_nat_rtif_inside_key_t),
        sizeof(tbl_nat_rtif_inside_t),
        sizeof(tbl_nat_rtif_inside_fields)/sizeof(cdb_field_t),
        tbl_nat_rtif_inside_fields,
        {
            tbl_nat_rtif_inside_iterate,
            (TBL_DUMP_FUNC)tbl_nat_rtif_inside_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_rtif_inside_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_rtif_inside_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_rtif_inside_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_rtif_inside_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_rtif_inside_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_rtif_inside_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_rtif_inside_table_parser,
        }
    },
    {
        sizeof(tbl_nat_rtif_outside_key_t),
        sizeof(tbl_nat_rtif_outside_t),
        sizeof(tbl_nat_rtif_outside_fields)/sizeof(cdb_field_t),
        tbl_nat_rtif_outside_fields,
        {
            tbl_nat_rtif_outside_iterate,
            (TBL_DUMP_FUNC)tbl_nat_rtif_outside_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_nat_rtif_outside_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_nat_rtif_outside_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_nat_rtif_outside_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_nat_rtif_outside_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_nat_rtif_outside_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_nat_rtif_outside_table_dump,
            (TBL_PARSER_FUNC)tbl_nat_rtif_outside_table_parser,
        }
    },
    {
        sizeof(tbl_g8032_ring_key_t),
        sizeof(tbl_g8032_ring_t),
        sizeof(tbl_g8032_ring_fields)/sizeof(cdb_field_t),
        tbl_g8032_ring_fields,
        {
            tbl_g8032_ring_iterate,
            (TBL_DUMP_FUNC)tbl_g8032_ring_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_g8032_ring_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_g8032_ring_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_g8032_ring_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_g8032_ring_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_g8032_ring_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_g8032_ring_table_dump,
            (TBL_PARSER_FUNC)tbl_g8032_ring_table_parser,
        }
    },
    {
        0,
        sizeof(tbl_cfm_global_t),
        sizeof(tbl_cfm_global_fields)/sizeof(cdb_field_t),
        tbl_cfm_global_fields,
        {
            tbl_cfm_global_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_global_dump_one,
            NULL,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_global_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_global_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_global_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_global_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_global_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_global_table_parser,
        }
    },
    {
        sizeof(tbl_cfm_md_key_t),
        sizeof(tbl_cfm_md_t),
        sizeof(tbl_cfm_md_fields)/sizeof(cdb_field_t),
        tbl_cfm_md_fields,
        {
            tbl_cfm_md_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_md_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cfm_md_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_md_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_md_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_md_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_md_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_md_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_md_table_parser,
        }
    },
    {
        sizeof(tbl_cfm_ma_key_t),
        sizeof(tbl_cfm_ma_t),
        sizeof(tbl_cfm_ma_fields)/sizeof(cdb_field_t),
        tbl_cfm_ma_fields,
        {
            tbl_cfm_ma_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_ma_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cfm_ma_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_ma_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_ma_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_ma_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_ma_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_ma_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_ma_table_parser,
        }
    },
    {
        sizeof(tbl_cfm_maid_key_t),
        sizeof(tbl_cfm_maid_t),
        sizeof(tbl_cfm_maid_fields)/sizeof(cdb_field_t),
        tbl_cfm_maid_fields,
        {
            tbl_cfm_maid_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_maid_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cfm_maid_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_maid_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_maid_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_maid_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_maid_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_maid_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_maid_table_parser,
        }
    },
    {
        sizeof(tbl_cfm_lmep_key_t),
        sizeof(tbl_cfm_lmep_t),
        sizeof(tbl_cfm_lmep_fields)/sizeof(cdb_field_t),
        tbl_cfm_lmep_fields,
        {
            tbl_cfm_lmep_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_lmep_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cfm_lmep_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_lmep_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_lmep_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_lmep_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_lmep_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_lmep_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_lmep_table_parser,
        }
    },
    {
        sizeof(tbl_cfm_rmep_key_t),
        sizeof(tbl_cfm_rmep_t),
        sizeof(tbl_cfm_rmep_fields)/sizeof(cdb_field_t),
        tbl_cfm_rmep_fields,
        {
            tbl_cfm_rmep_iterate,
            (TBL_DUMP_FUNC)tbl_cfm_rmep_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_cfm_rmep_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_cfm_rmep_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_cfm_rmep_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_cfm_rmep_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_cfm_rmep_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_cfm_rmep_table_dump,
            (TBL_PARSER_FUNC)tbl_cfm_rmep_table_parser,
        }
    },
    {
        sizeof(tbl_pim_sm_key_t),
        sizeof(tbl_pim_sm_t),
        sizeof(tbl_pim_sm_fields)/sizeof(cdb_field_t),
        tbl_pim_sm_fields,
        {
            tbl_pim_sm_iterate,
            (TBL_DUMP_FUNC)tbl_pim_sm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_sm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_sm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_sm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_sm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_sm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_sm_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_sm_table_parser,
        }
    },
    {
        sizeof(tbl_pim_sm_bsr_key_t),
        sizeof(tbl_pim_sm_bsr_t),
        sizeof(tbl_pim_sm_bsr_fields)/sizeof(cdb_field_t),
        tbl_pim_sm_bsr_fields,
        {
            tbl_pim_sm_bsr_iterate,
            (TBL_DUMP_FUNC)tbl_pim_sm_bsr_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_sm_bsr_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_sm_bsr_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_sm_bsr_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_sm_bsr_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_sm_bsr_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_sm_bsr_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_sm_bsr_table_parser,
        }
    },
    {
        sizeof(tbl_pim_sm_rp_key_t),
        sizeof(tbl_pim_sm_rp_t),
        sizeof(tbl_pim_sm_rp_fields)/sizeof(cdb_field_t),
        tbl_pim_sm_rp_fields,
        {
            tbl_pim_sm_rp_iterate,
            (TBL_DUMP_FUNC)tbl_pim_sm_rp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_sm_rp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_sm_rp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_sm_rp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_sm_rp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_sm_rp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_sm_rp_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_sm_rp_table_parser,
        }
    },
    {
        sizeof(tbl_pim_sm_rp_gp_key_t),
        sizeof(tbl_pim_sm_rp_gp_t),
        sizeof(tbl_pim_sm_rp_gp_fields)/sizeof(cdb_field_t),
        tbl_pim_sm_rp_gp_fields,
        {
            tbl_pim_sm_rp_gp_iterate,
            (TBL_DUMP_FUNC)tbl_pim_sm_rp_gp_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_sm_rp_gp_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_sm_rp_gp_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_sm_rp_gp_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_sm_rp_gp_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_sm_rp_gp_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_sm_rp_gp_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_sm_rp_gp_table_parser,
        }
    },
    {
        sizeof(tbl_pim_dm_key_t),
        sizeof(tbl_pim_dm_t),
        sizeof(tbl_pim_dm_fields)/sizeof(cdb_field_t),
        tbl_pim_dm_fields,
        {
            tbl_pim_dm_iterate,
            (TBL_DUMP_FUNC)tbl_pim_dm_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_pim_dm_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_pim_dm_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_pim_dm_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_pim_dm_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_pim_dm_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_pim_dm_table_dump,
            (TBL_PARSER_FUNC)tbl_pim_dm_table_parser,
        }
    },
    {
        sizeof(tbl_igmp_intf_key_t),
        sizeof(tbl_igmp_intf_t),
        sizeof(tbl_igmp_intf_fields)/sizeof(cdb_field_t),
        tbl_igmp_intf_fields,
        {
            tbl_igmp_intf_iterate,
            (TBL_DUMP_FUNC)tbl_igmp_intf_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_igmp_intf_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_igmp_intf_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_igmp_intf_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_igmp_intf_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_igmp_intf_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_igmp_intf_table_dump,
            (TBL_PARSER_FUNC)tbl_igmp_intf_table_parser,
        }
    },
    {
        sizeof(tbl_rip_key_t),
        sizeof(tbl_rip_t),
        sizeof(tbl_rip_fields)/sizeof(cdb_field_t),
        tbl_rip_fields,
        {
            tbl_rip_iterate,
            (TBL_DUMP_FUNC)tbl_rip_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rip_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rip_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rip_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rip_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rip_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rip_table_dump,
            (TBL_PARSER_FUNC)tbl_rip_table_parser,
        }
    },
    {
        sizeof(tbl_ripng_key_t),
        sizeof(tbl_ripng_t),
        sizeof(tbl_ripng_fields)/sizeof(cdb_field_t),
        tbl_ripng_fields,
        {
            tbl_ripng_iterate,
            (TBL_DUMP_FUNC)tbl_ripng_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ripng_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ripng_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ripng_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ripng_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ripng_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ripng_table_dump,
            (TBL_PARSER_FUNC)tbl_ripng_table_parser,
        }
    },
    {
        sizeof(tbl_rip_network_key_t),
        sizeof(tbl_rip_network_t),
        sizeof(tbl_rip_network_fields)/sizeof(cdb_field_t),
        tbl_rip_network_fields,
        {
            tbl_rip_network_iterate,
            (TBL_DUMP_FUNC)tbl_rip_network_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rip_network_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rip_network_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rip_network_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rip_network_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rip_network_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rip_network_table_dump,
            (TBL_PARSER_FUNC)tbl_rip_network_table_parser,
        }
    },
    {
        sizeof(tbl_ripng_network_key_t),
        sizeof(tbl_ripng_network_t),
        sizeof(tbl_ripng_network_fields)/sizeof(cdb_field_t),
        tbl_ripng_network_fields,
        {
            tbl_ripng_network_iterate,
            (TBL_DUMP_FUNC)tbl_ripng_network_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ripng_network_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ripng_network_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ripng_network_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ripng_network_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ripng_network_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ripng_network_table_dump,
            (TBL_PARSER_FUNC)tbl_ripng_network_table_parser,
        }
    },
    {
        sizeof(tbl_rip_neighbor_key_t),
        sizeof(tbl_rip_neighbor_t),
        sizeof(tbl_rip_neighbor_fields)/sizeof(cdb_field_t),
        tbl_rip_neighbor_fields,
        {
            tbl_rip_neighbor_iterate,
            (TBL_DUMP_FUNC)tbl_rip_neighbor_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rip_neighbor_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rip_neighbor_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rip_neighbor_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rip_neighbor_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rip_neighbor_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rip_neighbor_table_dump,
            (TBL_PARSER_FUNC)tbl_rip_neighbor_table_parser,
        }
    },
    {
        sizeof(tbl_rip_passive_if_key_t),
        sizeof(tbl_rip_passive_if_t),
        sizeof(tbl_rip_passive_if_fields)/sizeof(cdb_field_t),
        tbl_rip_passive_if_fields,
        {
            tbl_rip_passive_if_iterate,
            (TBL_DUMP_FUNC)tbl_rip_passive_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_rip_passive_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_rip_passive_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_rip_passive_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_rip_passive_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_rip_passive_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_rip_passive_if_table_dump,
            (TBL_PARSER_FUNC)tbl_rip_passive_if_table_parser,
        }
    },
    {
        sizeof(tbl_ripng_passive_if_key_t),
        sizeof(tbl_ripng_passive_if_t),
        sizeof(tbl_ripng_passive_if_fields)/sizeof(cdb_field_t),
        tbl_ripng_passive_if_fields,
        {
            tbl_ripng_passive_if_iterate,
            (TBL_DUMP_FUNC)tbl_ripng_passive_if_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ripng_passive_if_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ripng_passive_if_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ripng_passive_if_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ripng_passive_if_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ripng_passive_if_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ripng_passive_if_table_dump,
            (TBL_PARSER_FUNC)tbl_ripng_passive_if_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_area_stub_key_t),
        sizeof(tbl_ospf_area_stub_t),
        sizeof(tbl_ospf_area_stub_fields)/sizeof(cdb_field_t),
        tbl_ospf_area_stub_fields,
        {
            tbl_ospf_area_stub_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_area_stub_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_area_stub_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_area_stub_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_area_stub_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_area_stub_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_area_stub_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_area_stub_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_area_stub_table_parser,
        }
    },
    {
        sizeof(tbl_ospf6_area_stub_key_t),
        sizeof(tbl_ospf6_area_stub_t),
        sizeof(tbl_ospf6_area_stub_fields)/sizeof(cdb_field_t),
        tbl_ospf6_area_stub_fields,
        {
            tbl_ospf6_area_stub_iterate,
            (TBL_DUMP_FUNC)tbl_ospf6_area_stub_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf6_area_stub_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf6_area_stub_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf6_area_stub_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf6_area_stub_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf6_area_stub_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf6_area_stub_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf6_area_stub_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_area_nssa_key_t),
        sizeof(tbl_ospf_area_nssa_t),
        sizeof(tbl_ospf_area_nssa_fields)/sizeof(cdb_field_t),
        tbl_ospf_area_nssa_fields,
        {
            tbl_ospf_area_nssa_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_area_nssa_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_area_nssa_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_area_nssa_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_area_nssa_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_area_nssa_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_area_nssa_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_area_nssa_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_area_nssa_table_parser,
        }
    },
    {
        sizeof(tbl_ospf_area_vlink_key_t),
        sizeof(tbl_ospf_area_vlink_t),
        sizeof(tbl_ospf_area_vlink_fields)/sizeof(cdb_field_t),
        tbl_ospf_area_vlink_fields,
        {
            tbl_ospf_area_vlink_iterate,
            (TBL_DUMP_FUNC)tbl_ospf_area_vlink_dump_one,
            (TBL_KEY_V2S_FUNC)tbl_ospf_area_vlink_key_val2str,
            NULL
        },
        {
            (TBL_KEY_NAME_DUMP_FUNC)tbl_ospf_area_vlink_key_name_dump,
            (TBL_KEY_VALUE_DUMP_FUNC)tbl_ospf_area_vlink_key_value_dump,
            (TBL_FIELD_NAME_DUMP_FUNC)tbl_ospf_area_vlink_field_name_dump,
            (TBL_FIELD_VALUE_DUMP_FUNC)tbl_ospf_area_vlink_field_value_dump,
            (TBL_ALL_DUMP_FUNC)tbl_ospf_area_vlink_table_dump,
            (TBL_PARSER_FUNC)tbl_ospf_area_vlink_table_parser,
        }
    },
};

int32
cdb_tbl_pm_init(uint32 pm_id)
{
    cdb_node_t *p_tbl = NULL;
    uint32 i = 0;

    for (i = 0; i < TBL_MAX; i++)
    {
        p_tbl = cdb_get_tbl(i);

        if (0)
        {
        }
        else if (TBL_INTERFACE == i)
        {
            tbl_interface_init_interface();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ROUTE_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_AUTHD == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCLIENT == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_route_if_init_route_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_KERNEL_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_kernel_if_init_kernel_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_PORT_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_port_if_init_fea_port_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLAN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_AUTHD == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_vlan_init_vlan();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PVLAN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_pvlan_init_pvlan();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FDB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_fdb_init_fdb();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MCFDB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_mcfdb_init_mcfdb();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MACFILTER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_macfilter_init_macfilter();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PSFDB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_psfdb_init_psfdb();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSG_S_IP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_ipsg_s_ip_init_ipsg_s_ip();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSG_S_MAC == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id))
            {
                tbl_ipsg_s_mac_init_ipsg_s_mac();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSG_FIB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_ipsg_fib_init_ipsg_fib();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BRG_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_brg_global_init_brg_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MSTP_PORT == i)
        {
            tbl_mstp_port_init_mstp_port();
            p_tbl->inited = TRUE;
        }
        else if (TBL_MSTI_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_msti_port_init_msti_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MSTP_INSTANCE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_mstp_instance_init_mstp_instance();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MSTP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_mstp_global_init_mstp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LLDP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_lldp_global_init_lldp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LLDP_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_lldp_if_init_lldp_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MLAG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id))
            {
                tbl_mlag_init_mlag();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MLAG_PEER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id))
            {
                tbl_mlag_peer_init_mlag_peer();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MLAG_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id))
            {
                tbl_mlag_port_init_mlag_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ISOLATION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_isolation_init_isolation();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ROUTE_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_route_global_init_route_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BGP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_bgp_init_bgp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BGP_NETWORK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_bgp_network_init_bgp_network();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BGP_NEIGHBOR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_bgp_neighbor_init_bgp_neighbor();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_init_ospf();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF6 == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf6_init_ospf6();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF6_INTERFACE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf6_interface_init_ospf6_interface();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_NETWORK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_network_init_ospf_network();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_AREA_AUTH == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_area_auth_init_ospf_area_auth();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_AREA_RANGE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_area_range_init_ospf_area_range();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF6_AREA_RANGE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf6_area_range_init_ospf6_area_range();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPROUTE_NODE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_iproute_node_init_iproute_node();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPROUTE_NODE_ADD_FAIL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_iproute_node_add_fail_init_iproute_node_add_fail();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPROUTE_NODE_ADD_FAIL_COUNT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_iproute_node_add_fail_count_init_iproute_node_add_fail_count();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_STATIC_ROUTE_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_static_route_cfg_init_static_route_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_STATIC_RT_CNT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_static_rt_cnt_init_static_rt_cnt();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ARP_FIB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_arp_fib_init_arp_fib();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ARP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_PTP == pm_id))
            {
                tbl_arp_init_arp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NEXTHOP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nexthop_init_nexthop();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NEXTHOP_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nexthop_group_init_nexthop_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NEXTHOP_GROUP_ROUTED == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nexthop_group_routed_init_nexthop_group_routed();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_NEXTHOP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_nexthop_init_fea_nexthop();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MRT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_mrt_init_mrt();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPMC_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipmc_global_init_ipmc_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPMC_INTF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipmc_intf_init_ipmc_intf();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PIM_STATIC_RP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_static_rp_init_pim_static_rp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SYS_GLOBAL == i)
        {
            tbl_sys_global_init_sys_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_LAG_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_lag_global_init_lag_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ECMP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_ecmp_global_init_ecmp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RR_PREFIX == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_rr_prefix_init_rr_prefix();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MEM_SUMMARY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_mem_summary_init_mem_summary();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CPU_MEM_HISTORY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_cpu_mem_history_init_cpu_mem_history();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CHSM_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id))
            {
                tbl_chsm_debug_init_chsm_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SWITCH_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_switch_debug_init_switch_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ROUTE_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_route_debug_init_route_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SSM_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SSM == pm_id))
            {
                tbl_ssm_debug_init_ssm_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PTP_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id))
            {
                tbl_ptp_debug_init_ptp_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QUAGGA_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_quagga_debug_init_quagga_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LSRV_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_lsrv_debug_init_lsrv_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_HSRV_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_hsrv_debug_init_hsrv_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_APP_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_app_debug_init_app_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OPENFLOW_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_openflow_debug_init_openflow_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_rif_init_rif();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_LAG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_lag_init_fea_lag();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_global_init_fea_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_ACL_TABLE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_acl_table_init_fea_acl_table();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_ACL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_acl_init_fea_acl();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_FDB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_fdb_init_fea_fdb();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_BRG_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_brg_if_init_fea_brg_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_WORM_FILTER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_acl_worm_filter_init_acl_worm_filter();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_acl_config_init_acl_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACE_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_APPCFG == pm_id))
            {
                tbl_ace_config_init_ace_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_entry_init_acl_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_ENTRY_ACTION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_entry_action_init_acl_entry_action();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_NEXTHOP_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_nexthop_group_init_acl_nexthop_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_NEXTHOP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_nexthop_init_acl_nexthop();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TIME_RANGE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_time_range_init_time_range();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SSH_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_ssh_cfg_init_ssh_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_CFG == i)
        {
            tbl_snmp_cfg_init_snmp_cfg();
            p_tbl->inited = TRUE;
        }
        else if (TBL_SNMP_VIEW == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_view_init_snmp_view();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_COMMUNITY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_community_init_snmp_community();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_CONTEXT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_context_init_snmp_context();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_USM_USER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_usm_user_init_snmp_usm_user();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_GROUP_USM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_group_usm_init_snmp_group_usm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_ACCESS_USM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_access_usm_init_snmp_access_usm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_NOTIFY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_notify_init_snmp_notify();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_TARGET_ADDR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_target_addr_init_snmp_target_addr();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_TARGET_PARAMS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_target_params_init_snmp_target_params();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_TRAP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_trap_init_snmp_trap();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_INFORM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_snmp_inform_init_snmp_inform();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SNMP_TRAP_MSG == i)
        {
            tbl_snmp_trap_msg_init_snmp_trap_msg();
            p_tbl->inited = TRUE;
        }
        else if (TBL_RMON_EVENT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rmon_event_init_rmon_event();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RMON_ALARM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rmon_alarm_init_rmon_alarm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RMON_ETHER_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rmon_ether_stats_init_rmon_ether_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RMON_HISTORY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rmon_history_init_rmon_history();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RMON_STATS_HISTORY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rmon_stats_history_init_rmon_stats_history();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SYSLOG_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_syslog_cfg_init_syslog_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_SERVER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_server_init_ntp_server();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_ACE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_ace_init_ntp_ace();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_KEY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_key_init_ntp_key();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_cfg_init_ntp_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_if_init_ntp_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NTP_SYNCSTATUS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_ntp_syncstatus_init_ntp_syncstatus();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_STATIC_DNS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_static_dns_init_static_dns();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DYNAMIC_DNS_DOMAIN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_dynamic_dns_domain_init_dynamic_dns_domain();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DYNAMIC_DNS_SERVER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_dynamic_dns_server_init_dynamic_dns_server();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_DOMAIN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_domain_init_qos_domain();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_MPLS_DOMAIN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_mpls_domain_init_qos_mpls_domain();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_POLICER_RES == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_policer_res_init_qos_policer_res();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_POLICER_ACTION_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_policer_action_profile_init_qos_policer_action_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_POLICER_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_policer_profile_init_qos_policer_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_DROP_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_drop_profile_init_qos_drop_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_QUEUE_SHAPE_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_queue_shape_profile_init_qos_queue_shape_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_PORT_SHAPE_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_port_shape_profile_init_qos_port_shape_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_QUEUE_SMART_BUFFER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_queue_smart_buffer_init_qos_queue_smart_buffer();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_QOS_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_qos_global_init_qos_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MIRROR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_mirror_init_mirror();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MIRROR_MAC_ESCAPE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_mirror_mac_escape_init_mirror_mac_escape();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_GROUP_INGRESS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_group_ingress_init_tap_group_ingress();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_GROUP_INGRESS_FLOW == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_group_ingress_flow_init_tap_group_ingress_flow();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_GROUP_EGRESS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_group_egress_init_tap_group_egress();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_group_init_tap_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_FIVE_TUPLE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_five_tuple_init_tap_five_tuple();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_DPI_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_tap_dpi_global_init_tap_dpi_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_USER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_user_init_user();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LOGIN_RECORD == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_login_record_init_login_record();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LOGIN_SECURITY_GLB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_login_security_glb_init_login_security_glb();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VTY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_AUTHD == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_vty_init_vty();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CONSOLE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_console_init_console();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTHEN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_authen_init_authen();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LOGIN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_AUTHD == pm_id))
            {
                tbl_login_init_login();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RSA == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_rsa_init_rsa();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OPENFLOW == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_openflow_init_openflow();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CPU_TRAFFIC == i)
        {
            tbl_cpu_traffic_init_cpu_traffic();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CPU_TRAFFIC_GROUP == i)
        {
            tbl_cpu_traffic_group_init_cpu_traffic_group();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CPU_UTILIZATION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_cpu_utilization_init_cpu_utilization();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CPU_LIMIT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_cpu_limit_init_cpu_limit();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCRELAY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_dhcrelay_init_dhcrelay();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCSRVGRP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_dhcsrvgrp_init_dhcsrvgrp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCP_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCLIENT == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_dhcp_debug_init_dhcp_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCLIENT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCLIENT == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_dhclient_init_dhclient();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCSNOOPING == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_dhcsnooping_init_dhcsnooping();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCBINDING == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_dhcbinding_init_dhcbinding();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPTABLES_PREVENT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_iptables_prevent_init_iptables_prevent();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ERRDISABLE == i)
        {
            tbl_errdisable_init_errdisable();
            p_tbl->inited = TRUE;
        }
        else if (TBL_NS_PORT_FORWARDING == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_ns_port_forwarding_init_ns_port_forwarding();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LOG_GLOBAL == i)
        {
            tbl_log_global_init_log_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_LOG == i)
        {
            tbl_log_init_log();
            p_tbl->inited = TRUE;
        }
        else if (TBL_SYS_LOAD == i)
        {
            tbl_sys_load_init_sys_load();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CEM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_cem_init_cem();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CLOCK == i)
        {
            tbl_clock_init_clock();
            p_tbl->inited = TRUE;
        }
        else if (TBL_TAG_SUMMER_CLOCK == i)
        {
            tbl_tag_summer_clock_init_tag_summer_clock();
            p_tbl->inited = TRUE;
        }
        else if (TBL_PORT_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_OPENFLOW == pm_id) || (PM_ID_APPCFG == pm_id))
            {
                tbl_port_stats_init_port_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLAN_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_NCS == pm_id))
            {
                tbl_vlan_stats_init_vlan_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PORT_STATS_RATE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id) || (PM_ID_APPCFG == pm_id))
            {
                tbl_port_stats_rate_init_port_stats_rate();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACLQOS_IF == i)
        {
            tbl_aclqos_if_init_aclqos_if();
            p_tbl->inited = TRUE;
        }
        else if (TBL_L2_ACTION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_l2_action_init_l2_action();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_QOS_DROP_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_qos_drop_profile_init_fea_qos_drop_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_QOS_DOMAIN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_qos_domain_init_fea_qos_domain();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_G8032_RING == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_g8032_ring_init_fea_g8032_ring();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_G8032_RING_MC_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_g8032_ring_mc_group_init_fea_g8032_ring_mc_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_QOS_QUEUE_SHAPE_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_qos_queue_shape_profile_init_fea_qos_queue_shape_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_QOS_PORT_SHAPE_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_qos_port_shape_profile_init_fea_qos_port_shape_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_PORT_POLICER_APPLY == i)
        {
            tbl_fea_port_policer_apply_init_fea_port_policer_apply();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ACLQOS_IF_STATS == i)
        {
            tbl_aclqos_if_stats_init_aclqos_if_stats();
            p_tbl->inited = TRUE;
        }
        else if (TBL_VERSION == i)
        {
            tbl_version_init_version();
            p_tbl->inited = TRUE;
        }
        else if (TBL_MANAGE_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCLIENT == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_manage_if_init_manage_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BOOTIMAGE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id) || (PM_ID_BHM == pm_id))
            {
                tbl_bootimage_init_bootimage();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CHASSIS == i)
        {
            tbl_chassis_init_chassis();
            p_tbl->inited = TRUE;
        }
        else if (TBL_IFNAME_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_ifname_info_init_ifname_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CARD == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_OPENFLOW == pm_id) || (PM_ID_APPCFG == pm_id))
            {
                tbl_card_init_card();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_port_init_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FIBER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_fiber_init_fiber();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SYS_SPEC == i)
        {
            tbl_sys_spec_init_sys_spec();
            p_tbl->inited = TRUE;
        }
        else if (TBL_FAN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_fan_init_fan();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PSU == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_psu_init_psu();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LED == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_led_init_led();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SENSOR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_sensor_init_sensor();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VCM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_vcm_init_vcm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_POE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_poe_init_poe();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_REBOOT_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_CHSM == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_reboot_info_init_reboot_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ERRDISABLE_FLAP == i)
        {
            tbl_errdisable_flap_init_errdisable_flap();
            p_tbl->inited = TRUE;
        }
        else if (TBL_OPM_GLOBAL == i)
        {
            tbl_opm_global_init_opm_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ERPS_RING == i)
        {
            tbl_erps_ring_init_erps_ring();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ERPS_DOMAIN == i)
        {
            tbl_erps_domain_init_erps_domain();
            p_tbl->inited = TRUE;
        }
        else if (TBL_OPM_DEBUG == i)
        {
            tbl_opm_debug_init_opm_debug();
            p_tbl->inited = TRUE;
        }
        else if (TBL_POLICY_MAP_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_policy_map_config_init_policy_map_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CLASS_MAP_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_class_map_config_init_class_map_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CLASS_IN_POLICY_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_class_in_policy_config_init_class_in_policy_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_IN_CLASS_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_in_class_config_init_acl_in_class_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CLASS_MAP_ACTION_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_class_map_action_config_init_class_map_action_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_ACL_POLICY_ACTION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_acl_policy_action_init_fea_acl_policy_action();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IGSP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_igsp_global_init_igsp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IGSP_INTF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_igsp_intf_init_igsp_intf();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IGSP_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_igsp_group_init_igsp_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTH_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_AUTHD == pm_id))
            {
                tbl_auth_cfg_init_auth_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTH_SERVER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_AUTHD == pm_id))
            {
                tbl_auth_server_init_auth_server();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTH_SESSION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_AUTHD == pm_id))
            {
                tbl_auth_session_init_auth_session();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTHD_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_AUTHD == pm_id))
            {
                tbl_authd_debug_init_authd_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DOT1X_GLOBAL == i)
        {
            tbl_dot1x_global_init_dot1x_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_DOT1X_PORT == i)
        {
            tbl_dot1x_port_init_dot1x_port();
            p_tbl->inited = TRUE;
        }
        else if (TBL_DOT1X_RADIUS == i)
        {
            tbl_dot1x_radius_init_dot1x_radius();
            p_tbl->inited = TRUE;
        }
        else if (TBL_DOT1X_MAC == i)
        {
            tbl_dot1x_mac_init_dot1x_mac();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ENABLE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id))
            {
                tbl_enable_init_enable();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CHIP == i)
        {
            tbl_chip_init_chip();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CLEAR_ACL_POLICY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_clear_acl_policy_init_clear_acl_policy();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_AUTHOR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_AUTHD == pm_id))
            {
                tbl_author_init_author();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACCOUNT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_AUTHD == pm_id))
            {
                tbl_account_init_account();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACCOUNTCMD == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_APPCFG == pm_id) || (PM_ID_AUTHD == pm_id))
            {
                tbl_accountcmd_init_accountcmd();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLANCLASS_RULE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_vlanclass_rule_init_vlanclass_rule();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLANCLASS_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_vlanclass_group_init_vlanclass_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_L4_PORT_RANGE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_acl_l4_port_range_init_acl_l4_port_range();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_UDF_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_acl_udf_group_init_acl_udf_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ACL_UDF_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_acl_udf_entry_init_acl_udf_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_PCAP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_pcap_init_fea_pcap();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CONTROLLER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_controller_init_controller();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CPU_PACKETS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_cpu_packets_init_cpu_packets();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NS_ROUTE == i)
        {
            tbl_ns_route_init_ns_route();
            p_tbl->inited = TRUE;
        }
        else if (TBL_NS_ROUTE_IP == i)
        {
            tbl_ns_route_ip_init_ns_route_ip();
            p_tbl->inited = TRUE;
        }
        else if (TBL_OPENFLOW_INTERFACE == i)
        {
            tbl_openflow_interface_init_openflow_interface();
            p_tbl->inited = TRUE;
        }
        else if (TBL_OPENFLOW_MANAGER == i)
        {
            tbl_openflow_manager_init_openflow_manager();
            p_tbl->inited = TRUE;
        }
        else if (TBL_PTP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id))
            {
                tbl_ptp_global_init_ptp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PTP_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_ptp_port_init_ptp_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PTP_FOREIGN == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id))
            {
                tbl_ptp_foreign_init_ptp_foreign();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PTP_RESIDENCE_TIME_CACHE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id))
            {
                tbl_ptp_residence_time_cache_init_ptp_residence_time_cache();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_PTP_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_ptp_global_init_fea_ptp_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_PTP_TS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_PTP == pm_id))
            {
                tbl_fea_ptp_ts_init_fea_ptp_ts();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_TIME == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_fea_time_init_fea_time();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BHM_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_BHM == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_bhm_global_init_bhm_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_BHM_MODULE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_BHM == pm_id))
            {
                tbl_bhm_module_init_bhm_module();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OPENFLOW_TUNNEL_INTERFACE == i)
        {
            tbl_openflow_tunnel_interface_init_openflow_tunnel_interface();
            p_tbl->inited = TRUE;
        }
        else if (TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_openflow_tunnel_local_ip_cnt_init_openflow_tunnel_local_ip_cnt();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_INBAND_SNAT == i)
        {
            tbl_inband_snat_init_inband_snat();
            p_tbl->inited = TRUE;
        }
        else if (TBL_ARPINSP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_arpinsp_init_arpinsp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ARPACL_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_arpacl_config_init_arpacl_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ARPACE_CONFIG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_arpace_config_init_arpace_config();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_COPP_CFG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id) || (PM_ID_FEA == pm_id))
            {
                tbl_copp_cfg_init_copp_cfg();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_FEA_COPP_TRAP_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_fea_copp_trap_group_init_fea_copp_trap_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SFLOW_GLOBAL == i)
        {
            tbl_sflow_global_init_sflow_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_SFLOW_COLLECTOR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_sflow_collector_init_sflow_collector();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SFLOW_COUNTER_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_sflow_counter_port_init_sflow_counter_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VRRP_GLOBAL == i)
        {
            tbl_vrrp_global_init_vrrp_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_VRRP_VMAC == i)
        {
            tbl_vrrp_vmac_init_vrrp_vmac();
            p_tbl->inited = TRUE;
        }
        else if (TBL_VRRP_VIP == i)
        {
            tbl_vrrp_vip_init_vrrp_vip();
            p_tbl->inited = TRUE;
        }
        else if (TBL_VRRP_SESSION == i)
        {
            tbl_vrrp_session_init_vrrp_session();
            p_tbl->inited = TRUE;
        }
        else if (TBL_SSM_GLOBAL == i)
        {
            tbl_ssm_global_init_ssm_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_SSM_PORT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SSM == pm_id) || (PM_ID_SWITCH == pm_id) || (PM_ID_ROUTED == pm_id))
            {
                tbl_ssm_port_init_ssm_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSLA_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipsla_global_init_ipsla_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSLA_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipsla_entry_init_ipsla_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSLA_PACKET_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipsla_packet_stats_init_ipsla_packet_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IPSLA_TEST_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ipsla_test_stats_init_ipsla_test_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TRACK_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_track_global_init_track_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TRACK_OBJECT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_track_object_init_track_object();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CONTROLLER_ATTR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_NCS == pm_id))
            {
                tbl_controller_attr_init_controller_attr();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OAM_SESSION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_oam_session_init_oam_session();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OAM_SESSION_UPLOAD == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_oam_session_upload_init_oam_session_upload();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LSP_APS_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_NCS == pm_id))
            {
                tbl_lsp_aps_group_init_lsp_aps_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PW_APS_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id) || (PM_ID_NCS == pm_id))
            {
                tbl_pw_aps_group_init_pw_aps_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LSP_PE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_lsp_pe_init_lsp_pe();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_LSP_P == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_lsp_p_init_lsp_p();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PW_TPE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_pw_tpe_init_pw_tpe();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_SECTION_OAM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_section_oam_init_section_oam();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OAM_FLOW_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_OPM == pm_id))
            {
                tbl_oam_flow_info_init_oam_flow_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TPOAM_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPM == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_tpoam_global_init_tpoam_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TPOAM_EXEC_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPM == pm_id))
            {
                tbl_tpoam_exec_global_init_tpoam_exec_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_APS_GROUP_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPM == pm_id) || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_aps_group_info_init_aps_group_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OAM_INLABEL_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_oam_inlabel_info_init_oam_inlabel_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OAM_MPLS_APS_CHECK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id))
            {
                tbl_oam_mpls_aps_check_init_oam_mpls_aps_check();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLAN_MAPPING_TABLE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_vlan_mapping_table_init_vlan_mapping_table();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLAN_MAPPING_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_vlan_mapping_entry_init_vlan_mapping_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_EGRESS_MAPPING_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_egress_mapping_entry_init_egress_mapping_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_VLAN_GROUP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_vlan_group_init_vlan_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_EVC_ENTRY == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_evc_entry_init_evc_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_MAPPED_VLAN_INFO == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_mapped_vlan_info_init_mapped_vlan_info();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_L2_PROTOCOL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_l2_protocol_init_l2_protocol();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ND_FIB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nd_fib_init_nd_fib();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_ND_STATS == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nd_stats_init_nd_stats();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_CPU_MIRROR_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_cpu_mirror_global_init_cpu_mirror_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_TAP_SERVER == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_FEA == pm_id))
            {
                tbl_tap_server_init_tap_server();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_DHCPV6_DEBUG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_DHCRELAY == pm_id) || (PM_ID_DHCLIENT == pm_id) || (PM_ID_DHCSNOOPING == pm_id) || (PM_ID_DHCPD6 == pm_id) || (PM_ID_DHCLIENT6 == pm_id))
            {
                tbl_dhcpv6_debug_init_dhcpv6_debug();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_L2EDIT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_OPENFLOW == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_l2edit_init_l2edit();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_HASH_FIELD_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_hash_field_profile_init_hash_field_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_HASH_VALUE_PROFILE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_hash_value_profile_init_hash_value_profile();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_HASH_VALUE_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_SWITCH == pm_id))
            {
                tbl_hash_value_global_init_hash_value_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_GLOBAL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id) || (PM_ID_SWITCH == pm_id))
            {
                tbl_nat_global_init_nat_global();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_POOL == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_pool_init_nat_pool();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_RULE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_rule_init_nat_rule();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_SESSION == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_session_init_nat_session();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_SESSION_LIMIT == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_session_limit_init_nat_session_limit();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_PROXY_ARP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_proxy_arp_init_nat_proxy_arp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_RTIF_INSIDE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_rtif_inside_init_nat_rtif_inside();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_NAT_RTIF_OUTSIDE == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_nat_rtif_outside_init_nat_rtif_outside();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_G8032_RING == i)
        {
            tbl_g8032_ring_init_g8032_ring();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_GLOBAL == i)
        {
            tbl_cfm_global_init_cfm_global();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_MD == i)
        {
            tbl_cfm_md_init_cfm_md();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_MA == i)
        {
            tbl_cfm_ma_init_cfm_ma();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_MAID == i)
        {
            tbl_cfm_maid_init_cfm_maid();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_LMEP == i)
        {
            tbl_cfm_lmep_init_cfm_lmep();
            p_tbl->inited = TRUE;
        }
        else if (TBL_CFM_RMEP == i)
        {
            tbl_cfm_rmep_init_cfm_rmep();
            p_tbl->inited = TRUE;
        }
        else if (TBL_PIM_SM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_sm_init_pim_sm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PIM_SM_BSR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_sm_bsr_init_pim_sm_bsr();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PIM_SM_RP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_sm_rp_init_pim_sm_rp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PIM_SM_RP_GP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_sm_rp_gp_init_pim_sm_rp_gp();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_PIM_DM == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_pim_dm_init_pim_dm();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_IGMP_INTF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_igmp_intf_init_igmp_intf();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIP == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_rip_init_rip();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIPNG == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ripng_init_ripng();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIP_NETWORK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_rip_network_init_rip_network();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIPNG_NETWORK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ripng_network_init_ripng_network();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIP_NEIGHBOR == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_rip_neighbor_init_rip_neighbor();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIP_PASSIVE_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_rip_passive_if_init_rip_passive_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_RIPNG_PASSIVE_IF == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ripng_passive_if_init_ripng_passive_if();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_AREA_STUB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_area_stub_init_ospf_area_stub();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF6_AREA_STUB == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf6_area_stub_init_ospf6_area_stub();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_AREA_NSSA == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_area_nssa_init_ospf_area_nssa();
                p_tbl->inited = TRUE;
            }
        }
        else if (TBL_OSPF_AREA_VLINK == i)
        {
            if (CHECK_FULL_CDB_PMS || (PM_ID_ROUTED == pm_id))
            {
                tbl_ospf_area_vlink_init_ospf_area_vlink();
                p_tbl->inited = TRUE;
            }
        }
    }

    return PM_E_NONE;
}

#endif /* !__CDB_TBL_INFO_PRIV_H__ */

