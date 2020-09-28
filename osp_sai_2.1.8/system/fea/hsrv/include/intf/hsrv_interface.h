#ifndef __HSRV_SYNC_INTERFACE_H__
#define __HSRV_SYNC_INTERFACE_H__

#include "sdb.h"
#include "gen/stbl_port_define.h"

extern sal_mutex_t *g_hsrv_brgif_mutex;

int32
hsrv_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_interface_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_openflow_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_interface_get_objectid(tbl_interface_t *p_db_if, sai_object_id_t *port_oid);

int32
hsrv_interface_ifindex_to_objectid(uint32 ifindex, sai_object_id_t *port_oid);

int32
hsrv_interface_set_port_security(tbl_interface_t *p_if, ds_brgif_t* p_brgif, bool is_max);

int32
hsrv_interface_set_port_security_mac_learning(tbl_interface_t *p_if, uint32 enable);

int32
hsrv_interface_set_vlanclass_type_and_group_id(tbl_interface_t* p_if);

int32
hsrv_interface_set_vlanclass_default_action(tbl_interface_t* p_if);

int32
hsrv_interface_set_vlan_range_en(tbl_interface_t *p_if, uint32 group_id, uint32 enable);

int32
hsrv_interface_set_flow_control(tbl_interface_t* p_if);

int32
hsrv_interface_get_if_by_oid(sai_object_id_t oid, tbl_interface_t** pp_if);

int32
hsrv_interface_hash_value_profile_select(tbl_interface_t* p_if);

int32
hsrv_interface_start();

/* system modified by liwh for bug 53010, 2019-07-29 */
#ifdef GREATBELT
int32
hsrv_interface_set_port_mac(tbl_interface_t* p_if);
#endif

#endif /* !__HSRV_SYNC_INTERFACE_H__ */

