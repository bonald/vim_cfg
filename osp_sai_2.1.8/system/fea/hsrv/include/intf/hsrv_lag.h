#ifndef __HSRV_SYNC_LAG_H__
#define __HSRV_SYNC_LAG_H__


int32
hsrv_lag_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_lag_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
int32 
hsrv_lag_set_mem_port_security(tbl_interface_t *p_if,ds_brgif_t  *p_brgif);
int32
hsrv_lag_remove_member(tbl_interface_t* p_mif, ds_lag_t* p_lagif);

int32
hsrv_lag_add_member(tbl_interface_t* p_mif, ds_lag_t* p_lagif);

int32
hsrv_lag_start();

#endif /* !__HSRV_SYNC_LAG_H__ */
