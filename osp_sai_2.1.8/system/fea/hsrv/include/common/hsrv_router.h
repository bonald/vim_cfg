#include "hsrv_inc.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"

int32
hsrv_router_nexthop_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_router_nexthop_group_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_nexthop_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_router_nexthop_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
fea_nexthop_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
fea_nhg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
hsrv_route_api_get_nexthop_by_addr4(addr_ipv4_t *pst_dest, uint32 *mask_len, addr_ipv4_t *pst_nexthop);