#ifndef __HSRV_SYNC_L2_ACTION_H__
#define __HSRV_SYNC_L2_ACTION_H__


int32
hsrv_l2_action_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_erps_ring_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_fdb_flush_fdb_all();

int32
hsrv_fdb_flush_fdb_vlan(vid_t vid);

int32
hsrv_fdb_flush_fdb_port(uint32 ifindex);

int32
hsrv_fdb_flush_fdb_port_without_mlag_sync(uint32 ifindex);

int32
hsrv_fdb_flush_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
hsrv_fdb_flush_fdb_mac(mac_addr_t mac);

int32
hsrv_fdb_flush_fdb_pending(vid_t vid, mac_addr_t mac);

#endif /* !__HSRV_SYNC_VLAN_H__ */


