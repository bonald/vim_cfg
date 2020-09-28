#ifndef __HSRV_SYNC_VLANMAP_H__
#define __HSRV_SYNC_VLANMAP_H__


int32
hsrv_basic_qinq_add_delete_default_entry(tbl_interface_t *p_if, vid_t pvid, 
        uint32 add, uint32 replace);

int32
hsrv_vlanmap_apply_vm_entry_for_defvid(tbl_interface_t *p_if, vid_t pvid, uint32 add);

int32
hsrv_vlanmap_if_set_pvid(tbl_interface_t *p_if, vid_t pvid);

int32
hsrv_vlanmap_if_set_untag_pvid(tbl_interface_t *p_if, bool untag_pvid);

int32
hsrv_vlanmap_entry_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_vlanmap_entry_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_vlanmap_egress_entry_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_vlanmap_egress_entry_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_vlanmap_table_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_vlan_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_VLANMAP_H__ */


