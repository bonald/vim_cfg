#ifndef __HSRV_SYNC_NEIGHBOR_H__
#define __HSRV_SYNC_NEIGHBOR_H__

int32
hsrv_neighbor_fdb_age_cb_init();

/* modified by liwh for bug 48486, 2018-08-14 */
int32
hsrv_neighbor_nd_cb_init();
/* liwh end */

int32
hsrv_neighbor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32 
hsrv_neighbor_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_ipv6_neighbor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

/* modified by liwh for bug 48486, 2018-08-14 */
int32 
hsrv_neighbor_update_nd_nd_fib(uint8* saddr);
/* liwh end */

#endif /* !__HSRV_SYNC_NEIGHBOR_H__ */

