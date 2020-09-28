#ifndef __LSRV_CHASSIS_H__
#define __LSRV_CHASSIS_H__



int32
lsrv_chassis_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
lsrv_ifname_info_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

#endif /* !__LSRV_CHASSIS_H__ */


