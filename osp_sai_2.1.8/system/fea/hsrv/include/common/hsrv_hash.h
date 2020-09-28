#ifndef __HSRV_SYNC_HASH_H__
#define __HSRV_SYNC_HASH_H__


int32
hsrv_lag_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);


int32
hsrv_ecmp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_hash_init();
#endif /* !__HSRV_SYNC_HASH_H__ */

