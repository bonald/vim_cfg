#ifndef __HSRV_SYNC_CPU_TRAFFIC_H__
#define __HSRV_SYNC_CPU_TRAFFIC_H__

int32
hsrv_cpu_traffic_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_cpu_traffic_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_VLAN_H__ */
