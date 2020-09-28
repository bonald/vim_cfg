
#include "proto.h"
#include "cdb_store.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"

static int32
_cdb_store_ds_brgif(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_brgif_t *p_brgif = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_brgif = (ds_brgif_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_brgif_add_brgif_sync(p_if, p_brgif, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_brgif_del_brgif_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_brgif_set_brgif_field_sync(p_if, p_brgif, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_aclqos_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_aclqos_if_t *p_aclqos_if = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_aclqos_if = (ds_aclqos_if_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_aclqos_if_add_aclqos_if_sync(p_if, p_aclqos_if, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_aclqos_if_del_aclqos_if_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_aclqos_if_set_aclqos_if_field_sync(p_if, p_aclqos_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_lag(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_lag_t *p_lag = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_lag = (ds_lag_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_lag_add_lag_sync(p_if, p_lag, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_lag_del_lag_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_lag_set_lag_field_sync(p_if, p_lag, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_lacp(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_lacp_t *p_lacp = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_lacp = (ds_lacp_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_lacp_add_lacp_sync(p_if, p_lacp, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_lacp_del_lacp_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_lacp_set_lacp_field_sync(p_if, p_lacp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_connected(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_route_if_t *p_rtif = NULL;
    ds_connected_t *p_connect = NULL;
    int32 rc = PM_E_NONE;

    p_rtif = (tbl_route_if_t*)p_tbl;
    p_connect = (ds_connected_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_connected_add_connected_sync(p_rtif, p_connect, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_connected_del_connected_sync(p_rtif, p_connect, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_connected_set_connected_field_sync(p_rtif, p_connect, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_connected_v6(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_route_if_t *p_rtif = NULL;
    ds_connected_v6_t *p_connect_v6 = NULL;
    int32 rc = PM_E_NONE;

    p_rtif = (tbl_route_if_t*)p_tbl;
    p_connect_v6 = (ds_connected_v6_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_connected_v6_add_connected_v6_sync(p_rtif, p_connect_v6, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_connected_v6_del_connected_v6_sync(p_rtif, p_connect_v6, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_connected_v6_set_connected_v6_field_sync(p_rtif, p_connect_v6, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_static_route_dup_ipv4_nh(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_arp_t *p_arp = NULL;
    ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh = NULL;
    int32 rc = PM_E_NONE;

    p_arp = (tbl_arp_t*)p_tbl;
    p_static_route_dup_ipv4_nh = (ds_static_route_dup_ipv4_nh_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh_sync(p_arp, p_static_route_dup_ipv4_nh, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh_sync(p_arp, p_static_route_dup_ipv4_nh, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field_sync(p_arp, p_static_route_dup_ipv4_nh, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_ospf_auth(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_route_if_t *p_rtif = NULL;
    ds_ospf_auth_t *p_ospf_auth = NULL;
    int32 rc = PM_E_NONE;

    p_rtif = (tbl_route_if_t*)p_tbl;
    p_ospf_auth = (ds_ospf_auth_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_ospf_auth_add_ospf_auth_sync(p_rtif, p_ospf_auth, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_ospf_auth_del_ospf_auth_sync(p_rtif, p_ospf_auth, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_ospf_auth_set_ospf_auth_field_sync(p_rtif, p_ospf_auth, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_storm_control(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_storm_control_t *p_storm_control = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_storm_control = (ds_storm_control_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_storm_control_add_storm_control_sync(p_if, p_storm_control, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_storm_control_del_storm_control_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_storm_control_set_storm_control_field_sync(p_if, p_storm_control, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_openflow_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_openflow_if_t *p_openflow = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_openflow = (ds_openflow_if_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_openflow_if_add_openflow_if_sync(p_if, p_openflow, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_openflow_if_del_openflow_if_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_openflow_if_set_openflow_if_field_sync(p_if, p_openflow, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_dhclient_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_dhclient_if_t *p_dhclient = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_dhclient = (ds_dhclient_if_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_dhclient_if_add_dhclient_if_sync(p_if, p_dhclient, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_dhclient_if_del_dhclient_if_sync(p_if, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_dhclient_if_set_dhclient_if_field_sync(p_if, p_dhclient, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_pvlan_community(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_pvlan_t *p_pvlan = NULL;
    ds_pvlan_community_t *p_community = NULL;
    int32 rc = PM_E_NONE;

    p_pvlan = (tbl_pvlan_t*)p_tbl;
    p_community = (ds_pvlan_community_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_pvlan_community_add_pvlan_community_sync(p_pvlan, p_community, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_pvlan_community_del_pvlan_community_sync(p_pvlan, p_community, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_pvlan_community_set_pvlan_community_field_sync(p_pvlan, p_community, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_circuit_id(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_circuit_id_t *p_circuit_id = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_circuit_id = (ds_circuit_id_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_circuit_id_add_circuit_id_sync(p_if, p_circuit_id, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_circuit_id_del_circuit_id_sync(p_if, p_circuit_id, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_circuit_id_set_circuit_id_field_sync(p_if, p_circuit_id, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_flush_fdb(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_l2_action_t *p_l2_act = NULL;
    ds_flush_fdb_t *p_flush_fdb = NULL;
    int32 rc = PM_E_NONE;

    p_l2_act = (tbl_l2_action_t*)p_tbl;
    p_flush_fdb = (ds_flush_fdb_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_flush_fdb_add_flush_fdb_sync(p_l2_act, p_flush_fdb, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_flush_fdb_del_flush_fdb_sync(p_l2_act, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_flush_fdb_set_flush_fdb_field_sync(p_l2_act, p_flush_fdb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_lldp_neighbour(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_lldp_if_t *p_lldp_if = NULL;
    ds_lldp_neighbour_t *p_neigh = NULL;
    int32 rc = PM_E_NONE;

    p_lldp_if = (tbl_lldp_if_t*)p_tbl;
    p_neigh = (ds_lldp_neighbour_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_lldp_neighbour_add_lldp_neighbour_sync(p_lldp_if, p_neigh, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_lldp_neighbour_del_lldp_neighbour_sync(p_lldp_if, p_neigh, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_lldp_neighbour_set_lldp_neighbour_field_sync(p_lldp_if, p_neigh, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_mrouter_port(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_igsp_intf_t *p_if = NULL;
    ds_mrouter_port_t *p_mr_port = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_igsp_intf_t*)p_tbl;
    p_mr_port = (ds_mrouter_port_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_mrouter_port_add_mrouter_port_sync(p_if, p_mr_port, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_mrouter_port_del_mrouter_port_sync(p_if, p_mr_port, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_mrouter_port_set_mrouter_port_field_sync(p_if, p_mr_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_query(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_igsp_intf_t *p_if = NULL;
    ds_query_t *p_query = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_igsp_intf_t*)p_tbl;
    p_query = (ds_query_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_query_add_query_sync(p_if, p_query, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_query_del_query_sync(p_if, p_query, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_query_set_query_field_sync(p_if, p_query, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_l2protocol(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_interface_t *p_if = NULL;
    ds_l2protocol_t *p_l2protocol = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_interface_t*)p_tbl;
    p_l2protocol = (ds_l2protocol_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_l2protocol_add_l2protocol_sync(p_if, p_l2protocol, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_l2protocol_del_l2protocol_sync(p_if, p_l2protocol, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_l2protocol_set_l2protocol_field_sync(p_if, p_l2protocol, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_nd_prefix(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_route_if_t *p_rtif = NULL;
    ds_nd_prefix_t *p_nd_prefix = NULL;
    int32 rc = PM_E_NONE;

    p_rtif = (tbl_route_if_t*)p_tbl;
    p_nd_prefix = (ds_nd_prefix_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_nd_prefix_add_nd_prefix_sync(p_rtif, p_nd_prefix, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_nd_prefix_del_nd_prefix_sync(p_rtif, p_nd_prefix, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_nd_prefix_set_nd_prefix_field_sync(p_rtif, p_nd_prefix, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_join_group(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_igsp_intf_t *p_if = NULL;
    ds_join_group_t *p_join = NULL;
    int32 rc = PM_E_NONE;

    p_if = (tbl_igsp_intf_t*)p_tbl;
    p_join = (ds_join_group_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_join_group_add_join_group_sync(p_if, p_join, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_join_group_del_join_group_sync(p_if, p_join, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_join_group_set_join_group_field_sync(p_if, p_join, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_ds_cfm_error(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    tbl_cfm_md_t *p_md = NULL;
    ds_cfm_error_t *p_cfm_error = NULL;
    int32 rc = PM_E_NONE;

    p_md = (tbl_cfm_md_t*)p_tbl;
    p_cfm_error = (ds_cfm_error_t*)p_ds;

    switch (oper)
    {
    case CDB_OPER_ADD:
        rc = ds_cfm_error_add_cfm_error_sync(p_md, p_cfm_error, FALSE);
        break;

    case CDB_OPER_DEL:
        rc = ds_cfm_error_del_cfm_error_sync(p_md, p_cfm_error, FALSE);
        break;

    case CDB_OPER_SET:
        rc = ds_cfm_error_set_cfm_error_field_sync(p_md, p_cfm_error, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

CDB_STORE_DS_FUNC g_cdb_store_ds_func[DS_MAX] = 
{
    _cdb_store_ds_brgif,
    _cdb_store_ds_aclqos_if,
    _cdb_store_ds_lag,
    _cdb_store_ds_lacp,
    _cdb_store_ds_connected,
    _cdb_store_ds_connected_v6,
    _cdb_store_ds_static_route_dup_ipv4_nh,
    _cdb_store_ds_ospf_auth,
    _cdb_store_ds_storm_control,
    _cdb_store_ds_openflow_if,
    _cdb_store_ds_dhclient_if,
    _cdb_store_ds_pvlan_community,
    _cdb_store_ds_circuit_id,
    _cdb_store_ds_flush_fdb,
    _cdb_store_ds_lldp_neighbour,
    _cdb_store_ds_mrouter_port,
    _cdb_store_ds_query,
    _cdb_store_ds_l2protocol,
    _cdb_store_ds_nd_prefix,
    _cdb_store_ds_join_group,
    _cdb_store_ds_cfm_error,
};

int32
cdb_store_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
                void *p_data, uint32 data_len)
{
    void *p_tbl = NULL;
    void *p_ds = NULL;
    uint32 ds2_data_size = 0;

    if (p_ds2_node)
    {
        ds2_data_size = p_ds2_node->tbl_info->data_size;
    }
    SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size + ds2_data_size == data_len);
    p_tbl = p_data;
    p_ds = p_data + p_tbl_node->tbl_info->key_size;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    if (NULL == g_cdb_store_ds_func[p_ds_node->id])
    {
        return PM_E_NOT_SUPPORT;

    }

    if (p_ds2_node)
    {
        return g_cdb_store_ds_func[p_ds2_node->id](oper, fields, p_tbl_node, p_ds_node,
                p_tbl, p_tbl_node->tbl_info->key_size, p_ds, p_ds_node->tbl_info->data_size);
    }
    else
    {
        return g_cdb_store_ds_func[p_ds_node->id](oper, fields, p_tbl_node, p_ds_node,
                p_tbl, p_tbl_node->tbl_info->key_size, p_ds, p_ds_node->tbl_info->data_size);
    }
}

int32
cdb_store_ds_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
                void *p_data, uint32 data_len)
{
    void *p_tbl = NULL;
    void *p_ds = NULL;
    void *p_left = NULL;
    cdb_sync_refer_t *p_refer = NULL;
    void *p_refer_obj = NULL;
    uint32 ds2_data_size = 0;
    uint32 add = FALSE;
    int32 rc = PM_E_NONE;

    if (p_ds2_node)
    {
        ds2_data_size = p_ds2_node->tbl_info->key_size;
    }
    p_tbl = p_data;
    p_ds = p_data + p_tbl_node->tbl_info->key_size;
    p_left = p_data + p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->key_size + ds2_data_size;
    p_refer = p_left;
    p_refer_obj = &(p_refer->object);
    SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->key_size + ds2_data_size \
        + sizeof(cdb_sync_refer_hdr_t) + p_refer->hdr.size == data_len);
    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;

    if (CDB_REFER_TBL == p_refer->hdr.type)
    {
        switch (p_ds_node->id)
        {
        case DS_LAG:
            switch (fields[0])
            {
            case DS_LAG_FLD_MEMBER_PORTS:
                if (tbl_interface_get_master())
                {
                    p_refer_obj = tbl_interface_get_interface(p_refer_obj);
                    PM_E_RETURN(ds_lag_set_member_ports_sync(p_tbl, p_ds, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        case DS_PVLAN_COMMUNITY:
            switch (fields[0])
            {
            case DS_PVLAN_COMMUNITY_FLD_PORTS:
                if (tbl_interface_get_master())
                {
                    p_refer_obj = tbl_interface_get_interface(p_refer_obj);
                    PM_E_RETURN(ds_pvlan_community_set_ports_sync(p_tbl, p_ds, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        default:
            return PM_E_INVALID_PARAM;
        }
    }
    else if (CDB_REFER_TIMER == p_refer->hdr.type)
    {
    }

    return PM_E_NONE;
}

