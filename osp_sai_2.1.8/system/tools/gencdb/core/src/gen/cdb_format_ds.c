
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
_cdb_format_ds_brgif(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_aclqos_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_lag(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_lag_t *p_lag = NULL;

    p_lag = (ds_lag_t*)p_ds;

    p_lag->member_ports.obj_list = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_lacp(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_lacp_t *p_lacp = NULL;

    p_lacp = (ds_lacp_t*)p_ds;

    p_lacp->pdu = NULL;
    p_lacp->current_while_timer = NULL;
    p_lacp->periodic_timer = NULL;
    p_lacp->wait_while_timer = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_connected(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_connected_v6(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_connected_v6_t *p_connect_v6 = NULL;

    p_connect_v6 = (ds_connected_v6_t*)p_ds;

    p_connect_v6->t_timer = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_static_route_dup_ipv4_nh(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_ospf_auth(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_storm_control(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_openflow_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_dhclient_if(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_pvlan_community(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_pvlan_community_t *p_community = NULL;

    p_community = (ds_pvlan_community_t*)p_ds;

    p_community->ports.obj_list = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_circuit_id(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_flush_fdb(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_lldp_neighbour(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_lldp_neighbour_t *p_neigh = NULL;

    p_neigh = (ds_lldp_neighbour_t*)p_ds;

    p_neigh->rx_ttl_timer = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_mrouter_port(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_mrouter_port_t *p_mr_port = NULL;

    p_mr_port = (ds_mrouter_port_t*)p_ds;

    p_mr_port->expire_timer = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_query(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    ds_query_t *p_query = NULL;

    p_query = (ds_query_t*)p_ds;

    p_query->response_timer = NULL;
    return PM_E_NONE;
}

static int32
_cdb_format_ds_l2protocol(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_nd_prefix(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_join_group(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_ds_cfm_error(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node,
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)
{
    return PM_E_NONE;
}

CDB_STORE_DS_FUNC g_cdb_format_ds_func[DS_MAX] = 
{
    _cdb_format_ds_brgif,
    _cdb_format_ds_aclqos_if,
    _cdb_format_ds_lag,
    _cdb_format_ds_lacp,
    _cdb_format_ds_connected,
    _cdb_format_ds_connected_v6,
    _cdb_format_ds_static_route_dup_ipv4_nh,
    _cdb_format_ds_ospf_auth,
    _cdb_format_ds_storm_control,
    _cdb_format_ds_openflow_if,
    _cdb_format_ds_dhclient_if,
    _cdb_format_ds_pvlan_community,
    _cdb_format_ds_circuit_id,
    _cdb_format_ds_flush_fdb,
    _cdb_format_ds_lldp_neighbour,
    _cdb_format_ds_mrouter_port,
    _cdb_format_ds_query,
    _cdb_format_ds_l2protocol,
    _cdb_format_ds_nd_prefix,
    _cdb_format_ds_join_group,
    _cdb_format_ds_cfm_error,
};

int32
cdb_format_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
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

    if (NULL == g_cdb_format_ds_func[p_ds_node->id])
    {
        return PM_E_NOT_SUPPORT;
    }

    return g_cdb_format_ds_func[p_ds_node->id](oper, fields, p_tbl_node, p_ds_node,
            p_tbl, p_tbl_node->tbl_info->key_size, p_ds, data_len);
}

