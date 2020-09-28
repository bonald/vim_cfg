#ifndef __HSRV_SYNC_SWITCH_H__
#define __HSRV_SYNC_SWITCH_H__
int32
hsrv_dhcprelay_set_excp_type(uint32 ifindex, int32 enable);

int32
hsrv_switch_get_router_id(sai_object_id_t* routerid);

int32
hsrv_switch_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_switch_bhm_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_sys_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_openflow_set_stpid(uint32 svlan_tpid);

int32
hsrv_openflow_set_ctpid(uint32 cvlan_tpid);

int32
hsrv_openflow_set_fdb_flow_en();

int32
hsrv_openflow_set_ffgroup_switchback_en(uint8 enable);

int32
hsrv_switch_init();

void 
hsrv_switch_monitor_event_notification_cb(
    _In_ sai_object_id_t  port_oid,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list    
    );

int32
hsrv_dhcp_relay_update_service_when_add_delete_rtif(tbl_route_if_t *p_router_if, bool add);

#endif /* !__HSRV_SYNC_SWITCH_H__ */
