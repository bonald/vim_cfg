
#ifndef __CDB_TBL_INFO_PRIV_H__
#define __CDB_TBL_INFO_PRIV_H__

static sdb_tbl_info_t g_sdb_tbl_info[STBL_MAX] =
{
    {
        0,
        sizeof(stbl_sai_glb_t),
        sizeof(stbl_sai_glb_fields)/sizeof(sdb_field_t),
        stbl_sai_glb_fields,
        {
            stbl_sai_glb_iterate,
            (STBL_DUMP_FUNC)stbl_sai_glb_dump_one,
            NULL,
            NULL
        },
    },
    {
        0,
        sizeof(stbl_port_glb_t),
        sizeof(stbl_port_glb_fields)/sizeof(sdb_field_t),
        stbl_port_glb_fields,
        {
            stbl_port_glb_iterate,
            (STBL_DUMP_FUNC)stbl_port_glb_dump_one,
            NULL,
            NULL
        },
    },
    {
        sizeof(stbl_port_key_t),
        sizeof(stbl_port_t),
        sizeof(stbl_port_fields)/sizeof(sdb_field_t),
        stbl_port_fields,
        {
            stbl_port_iterate,
            (STBL_DUMP_FUNC)stbl_port_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_port_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_hostif_key_t),
        sizeof(stbl_hostif_t),
        sizeof(stbl_hostif_fields)/sizeof(sdb_field_t),
        stbl_hostif_fields,
        {
            stbl_hostif_iterate,
            (STBL_DUMP_FUNC)stbl_hostif_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_hostif_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_lag_memport_key_t),
        sizeof(stbl_lag_memport_t),
        sizeof(stbl_lag_memport_fields)/sizeof(sdb_field_t),
        stbl_lag_memport_fields,
        {
            stbl_lag_memport_iterate,
            (STBL_DUMP_FUNC)stbl_lag_memport_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_lag_memport_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_vr_key_t),
        sizeof(stbl_vr_t),
        sizeof(stbl_vr_fields)/sizeof(sdb_field_t),
        stbl_vr_fields,
        {
            stbl_vr_iterate,
            (STBL_DUMP_FUNC)stbl_vr_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_vr_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_rif_key_t),
        sizeof(stbl_rif_t),
        sizeof(stbl_rif_fields)/sizeof(sdb_field_t),
        stbl_rif_fields,
        {
            stbl_rif_iterate,
            (STBL_DUMP_FUNC)stbl_rif_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_rif_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_nexthop_key_t),
        sizeof(stbl_nexthop_t),
        sizeof(stbl_nexthop_fields)/sizeof(sdb_field_t),
        stbl_nexthop_fields,
        {
            stbl_nexthop_iterate,
            (STBL_DUMP_FUNC)stbl_nexthop_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_nexthop_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_nexthop_group_key_t),
        sizeof(stbl_nexthop_group_t),
        sizeof(stbl_nexthop_group_fields)/sizeof(sdb_field_t),
        stbl_nexthop_group_fields,
        {
            stbl_nexthop_group_iterate,
            (STBL_DUMP_FUNC)stbl_nexthop_group_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_nexthop_group_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_neighbor_key_t),
        sizeof(stbl_neighbor_t),
        sizeof(stbl_neighbor_fields)/sizeof(sdb_field_t),
        stbl_neighbor_fields,
        {
            stbl_neighbor_iterate,
            (STBL_DUMP_FUNC)stbl_neighbor_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_neighbor_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_vlan_key_t),
        sizeof(stbl_vlan_t),
        sizeof(stbl_vlan_fields)/sizeof(sdb_field_t),
        stbl_vlan_fields,
        {
            stbl_vlan_iterate,
            (STBL_DUMP_FUNC)stbl_vlan_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_vlan_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_vlan_class_key_t),
        sizeof(stbl_vlan_class_t),
        sizeof(stbl_vlan_class_fields)/sizeof(sdb_field_t),
        stbl_vlan_class_fields,
        {
            stbl_vlan_class_iterate,
            (STBL_DUMP_FUNC)stbl_vlan_class_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_vlan_class_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_stp_key_t),
        sizeof(stbl_stp_t),
        sizeof(stbl_stp_fields)/sizeof(sdb_field_t),
        stbl_stp_fields,
        {
            stbl_stp_iterate,
            (STBL_DUMP_FUNC)stbl_stp_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_stp_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_l2mc_key_t),
        sizeof(stbl_l2mc_t),
        sizeof(stbl_l2mc_fields)/sizeof(sdb_field_t),
        stbl_l2mc_fields,
        {
            stbl_l2mc_iterate,
            (STBL_DUMP_FUNC)stbl_l2mc_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_l2mc_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_route_key_t),
        sizeof(stbl_route_t),
        sizeof(stbl_route_fields)/sizeof(sdb_field_t),
        stbl_route_fields,
        {
            stbl_route_iterate,
            (STBL_DUMP_FUNC)stbl_route_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_route_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_vrrp_vip_key_t),
        sizeof(stbl_vrrp_vip_t),
        sizeof(stbl_vrrp_vip_fields)/sizeof(sdb_field_t),
        stbl_vrrp_vip_fields,
        {
            stbl_vrrp_vip_iterate,
            (STBL_DUMP_FUNC)stbl_vrrp_vip_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_vrrp_vip_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_copp_trap_key_t),
        sizeof(stbl_copp_trap_t),
        sizeof(stbl_copp_trap_fields)/sizeof(sdb_field_t),
        stbl_copp_trap_fields,
        {
            stbl_copp_trap_iterate,
            (STBL_DUMP_FUNC)stbl_copp_trap_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_copp_trap_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_copp_group_key_t),
        sizeof(stbl_copp_group_t),
        sizeof(stbl_copp_group_fields)/sizeof(sdb_field_t),
        stbl_copp_group_fields,
        {
            stbl_copp_group_iterate,
            (STBL_DUMP_FUNC)stbl_copp_group_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_copp_group_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_copp_reason_key_t),
        sizeof(stbl_copp_reason_t),
        sizeof(stbl_copp_reason_fields)/sizeof(sdb_field_t),
        stbl_copp_reason_fields,
        {
            stbl_copp_reason_iterate,
            (STBL_DUMP_FUNC)stbl_copp_reason_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_copp_reason_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_copp_entry_key_t),
        sizeof(stbl_copp_entry_t),
        sizeof(stbl_copp_entry_fields)/sizeof(sdb_field_t),
        stbl_copp_entry_fields,
        {
            stbl_copp_entry_iterate,
            (STBL_DUMP_FUNC)stbl_copp_entry_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_copp_entry_key_val2str,
            NULL
        },
    },
    {
        0,
        sizeof(stbl_acl_glb_t),
        sizeof(stbl_acl_glb_fields)/sizeof(sdb_field_t),
        stbl_acl_glb_fields,
        {
            stbl_acl_glb_iterate,
            (STBL_DUMP_FUNC)stbl_acl_glb_dump_one,
            NULL,
            NULL
        },
    },
    {
        sizeof(stbl_acl_port_key_t),
        sizeof(stbl_acl_port_t),
        sizeof(stbl_acl_port_fields)/sizeof(sdb_field_t),
        stbl_acl_port_fields,
        {
            stbl_acl_port_iterate,
            (STBL_DUMP_FUNC)stbl_acl_port_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_acl_port_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_acl_tbl_key_t),
        sizeof(stbl_acl_tbl_t),
        sizeof(stbl_acl_tbl_fields)/sizeof(sdb_field_t),
        stbl_acl_tbl_fields,
        {
            stbl_acl_tbl_iterate,
            (STBL_DUMP_FUNC)stbl_acl_tbl_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_acl_tbl_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_acl_entry_key_t),
        sizeof(stbl_acl_entry_t),
        sizeof(stbl_acl_entry_fields)/sizeof(sdb_field_t),
        stbl_acl_entry_fields,
        {
            stbl_acl_entry_iterate,
            (STBL_DUMP_FUNC)stbl_acl_entry_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_acl_entry_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_acl_tunnel_key_t),
        sizeof(stbl_acl_tunnel_t),
        sizeof(stbl_acl_tunnel_fields)/sizeof(sdb_field_t),
        stbl_acl_tunnel_fields,
        {
            stbl_acl_tunnel_iterate,
            (STBL_DUMP_FUNC)stbl_acl_tunnel_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_acl_tunnel_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_acl_udf_entry_key_t),
        sizeof(stbl_acl_udf_entry_t),
        sizeof(stbl_acl_udf_entry_fields)/sizeof(sdb_field_t),
        stbl_acl_udf_entry_fields,
        {
            stbl_acl_udf_entry_iterate,
            (STBL_DUMP_FUNC)stbl_acl_udf_entry_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_acl_udf_entry_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_ipmc_key_t),
        sizeof(stbl_ipmc_t),
        sizeof(stbl_ipmc_fields)/sizeof(sdb_field_t),
        stbl_ipmc_fields,
        {
            stbl_ipmc_iterate,
            (STBL_DUMP_FUNC)stbl_ipmc_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_ipmc_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_nat_forward_key_t),
        sizeof(stbl_nat_forward_t),
        sizeof(stbl_nat_forward_fields)/sizeof(sdb_field_t),
        stbl_nat_forward_fields,
        {
            stbl_nat_forward_iterate,
            (STBL_DUMP_FUNC)stbl_nat_forward_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_nat_forward_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_qos_mpls_domain_key_t),
        sizeof(stbl_qos_mpls_domain_t),
        sizeof(stbl_qos_mpls_domain_fields)/sizeof(sdb_field_t),
        stbl_qos_mpls_domain_fields,
        {
            stbl_qos_mpls_domain_iterate,
            (STBL_DUMP_FUNC)stbl_qos_mpls_domain_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_qos_mpls_domain_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_qos_mpls_flow_key_t),
        sizeof(stbl_qos_mpls_flow_t),
        sizeof(stbl_qos_mpls_flow_fields)/sizeof(sdb_field_t),
        stbl_qos_mpls_flow_fields,
        {
            stbl_qos_mpls_flow_iterate,
            (STBL_DUMP_FUNC)stbl_qos_mpls_flow_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_qos_mpls_flow_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_cfm_lmep_key_t),
        sizeof(stbl_cfm_lmep_t),
        sizeof(stbl_cfm_lmep_fields)/sizeof(sdb_field_t),
        stbl_cfm_lmep_fields,
        {
            stbl_cfm_lmep_iterate,
            (STBL_DUMP_FUNC)stbl_cfm_lmep_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_cfm_lmep_key_val2str,
            NULL
        },
    },
    {
        sizeof(stbl_cfm_rmep_key_t),
        sizeof(stbl_cfm_rmep_t),
        sizeof(stbl_cfm_rmep_fields)/sizeof(sdb_field_t),
        stbl_cfm_rmep_fields,
        {
            stbl_cfm_rmep_iterate,
            (STBL_DUMP_FUNC)stbl_cfm_rmep_dump_one,
            (STBL_KEY_V2S_FUNC)stbl_cfm_rmep_key_val2str,
            NULL
        },
    },
};

int32
sdb_tbl_pm_init(uint32 pm_id)
{
    sdb_node_t *p_tbl = NULL;
    uint32 i = 0;

    for (i = 0; i < STBL_MAX; i++)
    {
        p_tbl = sdb_get_tbl(i);

        if (0)
        {
        }
        else if (STBL_SAI_GLB == i)
        {
            {
                stbl_sai_glb_init_sai_glb();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_PORT_GLB == i)
        {
            {
                stbl_port_glb_init_port_glb();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_PORT == i)
        {
            {
                stbl_port_init_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_HOSTIF == i)
        {
            {
                stbl_hostif_init_hostif();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_LAG_MEMPORT == i)
        {
            {
                stbl_lag_memport_init_lag_memport();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_VR == i)
        {
            {
                stbl_vr_init_vr();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_RIF == i)
        {
            {
                stbl_rif_init_rif();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_NEXTHOP == i)
        {
            {
                stbl_nexthop_init_nexthop();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_NEXTHOP_GROUP == i)
        {
            {
                stbl_nexthop_group_init_nexthop_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_NEIGHBOR == i)
        {
            {
                stbl_neighbor_init_neighbor();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_VLAN == i)
        {
            {
                stbl_vlan_init_vlan();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_VLAN_CLASS == i)
        {
            {
                stbl_vlan_class_init_vlan_class();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_STP == i)
        {
            {
                stbl_stp_init_stp();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_L2MC == i)
        {
            {
                stbl_l2mc_init_l2mc();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ROUTE == i)
        {
            {
                stbl_route_init_route();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_VRRP_VIP == i)
        {
            {
                stbl_vrrp_vip_init_vrrp_vip();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_COPP_TRAP == i)
        {
            {
                stbl_copp_trap_init_copp_trap();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_COPP_GROUP == i)
        {
            {
                stbl_copp_group_init_copp_group();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_COPP_REASON == i)
        {
            {
                stbl_copp_reason_init_copp_reason();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_COPP_ENTRY == i)
        {
            {
                stbl_copp_entry_init_copp_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_GLB == i)
        {
            {
                stbl_acl_glb_init_acl_glb();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_PORT == i)
        {
            {
                stbl_acl_port_init_acl_port();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_TBL == i)
        {
            {
                stbl_acl_tbl_init_acl_tbl();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_ENTRY == i)
        {
            {
                stbl_acl_entry_init_acl_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_TUNNEL == i)
        {
            {
                stbl_acl_tunnel_init_acl_tunnel();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_ACL_UDF_ENTRY == i)
        {
            {
                stbl_acl_udf_entry_init_acl_udf_entry();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_IPMC == i)
        {
            {
                stbl_ipmc_init_ipmc();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_NAT_FORWARD == i)
        {
            {
                stbl_nat_forward_init_nat_forward();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_QOS_MPLS_DOMAIN == i)
        {
            {
                stbl_qos_mpls_domain_init_qos_mpls_domain();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_QOS_MPLS_FLOW == i)
        {
            {
                stbl_qos_mpls_flow_init_qos_mpls_flow();
                p_tbl->inited = TRUE;
            }
        }
        else if (STBL_CFM_LMEP == i)
        {
            stbl_cfm_lmep_init_cfm_lmep();
            p_tbl->inited = TRUE;
        }
        else if (STBL_CFM_RMEP == i)
        {
            stbl_cfm_rmep_init_cfm_rmep();
            p_tbl->inited = TRUE;
        }
    }

    return SDB_E_NONE;
}

#endif /* !__CDB_TBL_INFO_PRIV_H__ */

