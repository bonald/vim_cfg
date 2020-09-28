#ifndef __HSRV_TUNNELDB_CDB_H__
#define __HSRV_TUNNELDB_CDB_H__

int32 
hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(uint32 remote_ip, glb_openflow_tunnel_info_t* p_tunnel_info);

typedef enum
{
    HSRV_TUNNEL_CDB_FLOW_ADD_SINGLE_DYNAMIC_TUNNEL,
    HSRV_TUNNEL_CDB_FLOW_ADD_MULTI_DYNAMIC_TUNNEL,
    HSRV_TUNNEL_CDB_FLOW_DEL_UNMAP_ACTION,
    HSRV_TUNNEL_CDB_OP_MAX
} hsrv_tunnel_cdb_oper_t;

typedef enum
{
    HSRV_TUNNEL_CDB_ARP_ADD,
    HSRV_TUNNEL_CDB_ARP_DEL,
    HSRV_TUNNEL_CDB_ARP_OP_MAX
} hsrv_tunnel_cdb_arp_oper_t;


typedef enum
{
    HSRV_TUNNEL_CDB_ROUTE_ADD,
    HSRV_TUNNEL_CDB_ROUTE_DEL,
    HSRV_TUNNEL_CDB_ROUTE_OP_MAX
} hsrv_tunnel_cdb_route_oper_t;

typedef enum
{
    HSRV_TUNNEL_CDB_FDB_ADD,
    HSRV_TUNNEL_CDB_FDB_DEL,
    HSRV_TUNNEL_CDB_FDB_UPDATE,
    HSRV_TUNNEL_CDB_FDB_OP_MAX
} hsrv_tunnel_cdb_fdb_oper_t;

int32 
hsrv_tunneldb_cdb_oper_function(hsrv_tunnel_cdb_oper_t op, hsrv_openflow_flow_t *flow);

#endif
