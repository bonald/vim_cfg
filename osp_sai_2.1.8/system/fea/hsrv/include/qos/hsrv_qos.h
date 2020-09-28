#ifndef __HSRV_SYNC_QOS_H__
#define __HSRV_SYNC_QOS_H__

typedef struct hsrv_qos_domain_port_bind_map_id_s
{
    uint32_t dot1p_to_tc_color_map_id;
    uint32_t dscp_to_tc_color_map_id;
    uint32_t tc_and_color_to_dot1p_map_id;
    uint32_t tc_and_color_to_dscp_map_id;
}hsrv_qos_domain_port_bind_map_id_t;

int32
hsrv_qos_domain_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_qos_policer_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_drop_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_scheduler_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
int32
hsrv_qos_port_shape_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_policer_action_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_policer_action_map_sai_type(sai_attribute_t *attr, uint32 *attr_count, tbl_qos_policer_action_profile_t *p_db_action_profile);

int32
hsrv_qos_mpls_domain_synce(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_qos_queue_smart_buffer(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_VLAN_H__ */

