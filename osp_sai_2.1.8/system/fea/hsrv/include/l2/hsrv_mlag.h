#ifndef __HSRV_SYNC_MLAG_H__
#define __HSRV_SYNC_MLAG_H__

#include "gen/tbl_mlag_port_define.h"

int32
hsrv_mlag_init_done_cb();

int32
hsrv_mlag_set_isolate_port(uint32 ifindex, uint32 is_set, sai_port_isolation_direction_t dir);

int32
hsrv_mlag_set_is_group(uint32 ifindex, uint32 is_group);

uint32
hsrv_mlag_is_peer_up(tbl_mlag_port_t *p_db_port);

int32
hsrv_mlag_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_mlag_peer_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_mlag_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_is_mlag_peer_fdb(tbl_fdb_t *p_fdb);

#endif /* !__HSRV_SYNC_MLAG_H__ */
