#include "hsrv_inc.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"

int32
hsrv_route_rt_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_route_rt_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
      void *p_tbl, void *p_ds);

int32
hsrv_route_global_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
      void *p_tbl, void *p_ds);

