/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef __SAI_HANDLER_H__
#define __SAI_HANDLER_H__

#include <glib-object.h>
#include <stdio.h>

#include "gen-c_glib/switch_sai_rpc.h"

G_BEGIN_DECLS

/* A handler that implements the ThriftTestIf interface */

#define CTC_TYPE_SAI_RPC_HANDLER (ctc_sai_rpc_handler_get_type ())

#define CTC_THRIFT_TEST_HANDLER(obj)                                \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               CTC_TYPE_SAI_RPC_HANDLER,        \
                               ThriftTestHandler))
#define CTC_IS_SWITCH_SAI_RPC_HANDLER(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               CTC_TYPE_SAI_RPC_HANDLER))
#define CTC_SWITCH_SAI_RPC_HANDLER_CLASS(c)                    \
  (G_TYPE_CHECK_CLASS_CAST ((c),                        \
                            CTC_TYPE_SAI_RPC_HANDLER,   \
                            switch_sai_rpcHandlerClass))
#define CTC_IS_SWITCH_SAI_RPC_HANDLER_CLASS(c)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((c),                        \
                            CTC_TYPE_SAI_RPC_HANDLER))
#define CTC_SWITCH_SAI_RPC_HANDLER_GET_CLASS(obj)              \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              CTC_TYPE_SAI_RPC_HANDLER, \
                              switch_sai_rpcHandlerClass))

typedef struct _ctc_sai_rpcHandler ctc_sai_rpcHandler;
typedef struct _ctc_sai_rpcHandlerClass ctc_sai_rpcHandlerClass;

struct _ctc_sai_rpcHandler {
  switch_sai_rpcHandler parent;
};

struct _ctc_sai_rpcHandlerClass {
  switch_sai_rpcHandlerClass parent;

  gboolean (*sai_thrift_set_port_attribute) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t port_id, const sai_thrift_attribute_t * thrift_attr, GError **error);
  gboolean (*sai_thrift_create_fdb_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_fdb_entry_t * thrift_fdb_entry, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_delete_fdb_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_fdb_entry_t * thrift_fdb_entry, GError **error);
  gboolean (*sai_thrift_flush_fdb_entries) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_create_vlan) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_vlan_id_t vlan_id, GError **error);
  gboolean (*sai_thrift_delete_vlan) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_vlan_id_t vlan_id, GError **error);
  gboolean (*sai_thrift_add_ports_to_vlan) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_vlan_id_t vlan_id, const GPtrArray * thrift_port_list, GError **error);
  gboolean (*sai_thrift_remove_ports_from_vlan) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_vlan_id_t vlan_id, const GPtrArray * thrift_port_list, GError **error);
  gboolean (*sai_thrift_get_vlan_stats) (switch_sai_rpcIf *iface, GArray ** _return, const gint16 vlan_id, const GArray * counter_ids, const gint32 number_of_counters, GError **error);
  gboolean (*sai_thrift_create_virtual_router) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_virtual_router) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t vr_id, GError **error);
  gboolean (*sai_thrift_create_route) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_unicast_route_entry_t * thrift_unicast_route_entry, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_route) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_unicast_route_entry_t * thrift_unicast_route_entry, GError **error);
  gboolean (*sai_thrift_create_router_interface) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_router_interface) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t rif_id, GError **error);
  gboolean (*sai_thrift_create_next_hop) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_next_hop) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_id, GError **error);
  gboolean (*sai_thrift_create_next_hop_group) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_next_hop_group) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, GError **error);
  gboolean (*sai_thrift_add_next_hop_to_group) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, const GArray * thrift_nexthops, GError **error);
  gboolean (*sai_thrift_remove_next_hop_from_group) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, const GArray * thrift_nexthops, GError **error);
  gboolean (*sai_thrift_create_lag) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_lag) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t lag_id, GError **error);
  gboolean (*sai_thrift_create_lag_member) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_lag_member) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t lag_member_id, GError **error);
  gboolean (*sai_thrift_create_stp_entry) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_stp_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t stp_id, GError **error);
  gboolean (*sai_thrift_set_stp_port_state) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t stp_id, const sai_thrift_object_id_t port_id, const sai_thrift_port_stp_port_state_t stp_port_state, GError **error);
  gboolean (*sai_thrift_get_stp_port_state) (switch_sai_rpcIf *iface, sai_thrift_port_stp_port_state_t* _return, const sai_thrift_object_id_t stp_id, const sai_thrift_object_id_t port_id, GError **error);
  gboolean (*sai_thrift_create_neighbor_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_neighbor_entry_t * thrift_neighbor_entry, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_neighbor_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_neighbor_entry_t * thrift_neighbor_entry, GError **error);
  gboolean (*sai_thrift_get_switch_attribute) (switch_sai_rpcIf *iface, sai_thrift_attribute_list_t ** _return, GError **error);
  gboolean (*sai_thrift_set_switch_attribute) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_attribute_t * attribute, GError **error);
  gboolean (*sai_thrift_create_hostif) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_hostif) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t hif_id, GError **error);
  gboolean (*sai_thrift_create_hostif_trap_group) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_hostif_trap_group) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t trap_group_id, GError **error);
  gboolean (*sai_thrift_create_hostif_trap) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_hostif_trap) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_hostif_trap_id_t trap_id, GError **error);
  gboolean (*sai_thrift_set_hostif_trap) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_hostif_trap_id_t trap_id, const sai_thrift_attribute_t * thrift_attr, GError **error);
  gboolean (*sai_thrift_create_acl_table) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_delete_acl_table) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_table_id, GError **error);
  gboolean (*sai_thrift_create_acl_entry) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_delete_acl_entry) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_entry, GError **error);
  gboolean (*sai_thrift_create_acl_counter) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_delete_acl_counter) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_counter_id, GError **error);
  gboolean (*sai_thrift_get_acl_counter_attribute) (switch_sai_rpcIf *iface, GPtrArray ** _return, const sai_thrift_object_id_t acl_counter_id, const GArray * thrift_attr_ids, GError **error);
  gboolean (*sai_thrift_create_mirror_session) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_mirror_session) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t session_id, GError **error);
  gboolean (*sai_thrift_create_policer) (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error);
  gboolean (*sai_thrift_remove_policer) (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t policer_id, GError **error);
  gboolean (*sai_thrift_get_policer_stats) (switch_sai_rpcIf *iface, GArray ** _return, const sai_thrift_object_id_t policer_id, const GArray * counter_ids, GError **error);
};

/* Used by THRIFT_TEST_HANDLER_GET_TYPE */
GType ctc_sai_rpc_handler_get_type (void);
G_END_DECLS

#endif /* __SAI_HANDLER_H__ */
