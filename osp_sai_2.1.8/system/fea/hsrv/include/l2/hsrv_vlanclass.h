#ifndef __HSRV_SYNC_VLANCLASS_H__
#define __HSRV_SYNC_VLANCLASS_H__


int32
hsrv_vlanclass_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_vlanclass_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_VLANCLASS_H__ */


