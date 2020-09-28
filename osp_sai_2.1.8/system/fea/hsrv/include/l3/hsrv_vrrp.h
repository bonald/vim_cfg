#ifndef __HSRV_VRRP_H__
#define __HSRV_VRRP_H__

int32
hsrv_vrrp_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

#endif /* !__HSRV_VRRP_H__ */

