#ifndef __HSRV_L2PROTOCOL_H__
#define __HSRV_L2PROTOCOL_H__

int32
hsrv_l2protocol_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_l2protocol_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

#endif /* !__HSRV_L2PROTOCOL_H__ */


