
#ifndef __PB_DS_H__
#define __PB_DS_H__

typedef int32 (*PB_DS_PACK_FUNC)(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
typedef void* (*PB_DS_UNPACK_FUNC)(ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
typedef int32 (*PB_DS_DUMP_FUNC)(void *object, char *out);
typedef void  (*PB_DS_FREE_UNPACKED_FUNC)(void *object, ProtobufCAllocator *allocator);

#define Cdb__DsLldpNeighbourKey        Cdb__ComposeLldpMsapIdT
#define CDB__DS_LLDP_NEIGHBOUR_KEY__INIT CDB__COMPOSE_LLDP_MSAP_ID_T__INIT
#define Cdb__DsNdPrefixKey             Cdb__ComposeNdPrefixKeyT
#define CDB__DS_ND_PREFIX_KEY__INIT    CDB__COMPOSE_ND_PREFIX_KEY_T__INIT
#define Cdb__DsCfmErrorKey             Cdb__ComposeCfmTimeKeyT
#define CDB__DS_CFM_ERROR_KEY__INIT    CDB__COMPOSE_CFM_TIME_KEY_T__INIT


/* pb functions for TBL_INTERFACE DS_BRGIF */
int32
pb_ds_brgif_to_pb(tbl_interface_key_t *p_if_key, ds_brgif_t *p_brgif, Cdb__DsBrgif *p_pb_brgif);
int32
pb_ds_brgif_to_pb_free_packed(Cdb__DsBrgif *p_pb_brgif);
int32
pb_ds_brgif_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_brgif_dump(Cdb__DsBrgif *p_pb_brgif, char *out);

/* pb functions for TBL_INTERFACE DS_ACLQOS_IF */
int32
pb_ds_aclqos_if_to_pb(tbl_interface_key_t *p_if_key, ds_aclqos_if_t *p_aclqos_if, Cdb__DsAclqosIf *p_pb_aclqos_if);
int32
pb_ds_aclqos_if_to_pb_free_packed(Cdb__DsAclqosIf *p_pb_aclqos_if);
int32
pb_ds_aclqos_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_aclqos_if_dump(Cdb__DsAclqosIf *p_pb_aclqos_if, char *out);

/* pb functions for TBL_INTERFACE DS_LAG */
int32
pb_ds_lag_to_pb(tbl_interface_key_t *p_if_key, ds_lag_t *p_lag, Cdb__DsLag *p_pb_lag);
int32
pb_ds_lag_to_pb_free_packed(Cdb__DsLag *p_pb_lag);
int32
pb_ds_lag_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_lag_dump(Cdb__DsLag *p_pb_lag, char *out);

/* pb functions for TBL_INTERFACE DS_LACP */
int32
pb_ds_lacp_to_pb(tbl_interface_key_t *p_if_key, ds_lacp_t *p_lacp, Cdb__DsLacp *p_pb_lacp);
int32
pb_ds_lacp_to_pb_free_packed(Cdb__DsLacp *p_pb_lacp);
int32
pb_ds_lacp_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_lacp_dump(Cdb__DsLacp *p_pb_lacp, char *out);

/* pb functions for TBL_ROUTE_IF DS_CONNECTED */
int32
pb_ds_connected_to_pb(tbl_route_if_key_t *p_rtif_key, ds_connected_t *p_connect, Cdb__DsConnected *p_pb_connect);
int32
pb_ds_connected_to_pb_free_packed(Cdb__DsConnected *p_pb_connect);
int32
pb_ds_connected_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_connected_dump(Cdb__DsConnected *p_pb_connect, char *out);

/* pb functions for TBL_ROUTE_IF DS_CONNECTED_V6 */
int32
pb_ds_connected_v6_to_pb(tbl_route_if_key_t *p_rtif_key, ds_connected_v6_t *p_connect_v6, Cdb__DsConnectedV6 *p_pb_connect_v6);
int32
pb_ds_connected_v6_to_pb_free_packed(Cdb__DsConnectedV6 *p_pb_connect_v6);
int32
pb_ds_connected_v6_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_connected_v6_dump(Cdb__DsConnectedV6 *p_pb_connect_v6, char *out);

/* pb functions for TBL_ARP DS_STATIC_ROUTE_DUP_IPV4_NH */
int32
pb_ds_static_route_dup_ipv4_nh_to_pb(tbl_arp_key_t *p_arp_key, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh);
int32
pb_ds_static_route_dup_ipv4_nh_to_pb_free_packed(Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh);
int32
pb_ds_static_route_dup_ipv4_nh_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_static_route_dup_ipv4_nh_dump(Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh, char *out);

/* pb functions for TBL_ROUTE_IF DS_OSPF_AUTH */
int32
pb_ds_ospf_auth_to_pb(tbl_route_if_key_t *p_rtif_key, ds_ospf_auth_t *p_ospf_auth, Cdb__DsOspfAuth *p_pb_ospf_auth);
int32
pb_ds_ospf_auth_to_pb_free_packed(Cdb__DsOspfAuth *p_pb_ospf_auth);
int32
pb_ds_ospf_auth_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_ospf_auth_dump(Cdb__DsOspfAuth *p_pb_ospf_auth, char *out);

/* pb functions for TBL_INTERFACE DS_STORM_CONTROL */
int32
pb_ds_storm_control_to_pb(tbl_interface_key_t *p_if_key, ds_storm_control_t *p_storm_control, Cdb__DsStormControl *p_pb_storm_control);
int32
pb_ds_storm_control_to_pb_free_packed(Cdb__DsStormControl *p_pb_storm_control);
int32
pb_ds_storm_control_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_storm_control_dump(Cdb__DsStormControl *p_pb_storm_control, char *out);

/* pb functions for TBL_INTERFACE DS_OPENFLOW_IF */
int32
pb_ds_openflow_if_to_pb(tbl_interface_key_t *p_if_key, ds_openflow_if_t *p_openflow, Cdb__DsOpenflowIf *p_pb_openflow);
int32
pb_ds_openflow_if_to_pb_free_packed(Cdb__DsOpenflowIf *p_pb_openflow);
int32
pb_ds_openflow_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_openflow_if_dump(Cdb__DsOpenflowIf *p_pb_openflow, char *out);

/* pb functions for TBL_INTERFACE DS_DHCLIENT_IF */
int32
pb_ds_dhclient_if_to_pb(tbl_interface_key_t *p_if_key, ds_dhclient_if_t *p_dhclient, Cdb__DsDhclientIf *p_pb_dhclient);
int32
pb_ds_dhclient_if_to_pb_free_packed(Cdb__DsDhclientIf *p_pb_dhclient);
int32
pb_ds_dhclient_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_dhclient_if_dump(Cdb__DsDhclientIf *p_pb_dhclient, char *out);

/* pb functions for TBL_PVLAN DS_PVLAN_COMMUNITY */
int32
pb_ds_pvlan_community_to_pb(tbl_pvlan_key_t *p_pvlan_key, ds_pvlan_community_t *p_community, Cdb__DsPvlanCommunity *p_pb_community);
int32
pb_ds_pvlan_community_to_pb_free_packed(Cdb__DsPvlanCommunity *p_pb_community);
int32
pb_ds_pvlan_community_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_pvlan_community_dump(Cdb__DsPvlanCommunity *p_pb_community, char *out);

/* pb functions for TBL_INTERFACE DS_CIRCUIT_ID */
int32
pb_ds_circuit_id_to_pb(tbl_interface_key_t *p_if_key, ds_circuit_id_t *p_circuit_id, Cdb__DsCircuitId *p_pb_circuit_id);
int32
pb_ds_circuit_id_to_pb_free_packed(Cdb__DsCircuitId *p_pb_circuit_id);
int32
pb_ds_circuit_id_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_circuit_id_dump(Cdb__DsCircuitId *p_pb_circuit_id, char *out);

/* pb functions for TBL_L2_ACTION DS_FLUSH_FDB */
int32
pb_ds_flush_fdb_to_pb(ds_flush_fdb_t *p_flush_fdb, Cdb__DsFlushFdb *p_pb_flush_fdb);
int32
pb_ds_flush_fdb_to_pb_free_packed(Cdb__DsFlushFdb *p_pb_flush_fdb);
int32
pb_ds_flush_fdb_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_flush_fdb_dump(Cdb__DsFlushFdb *p_pb_flush_fdb, char *out);

/* pb functions for TBL_LLDP_IF DS_LLDP_NEIGHBOUR */
int32
pb_ds_lldp_neighbour_to_pb(tbl_lldp_if_key_t *p_lldp_if_key, ds_lldp_neighbour_t *p_neigh, Cdb__DsLldpNeighbour *p_pb_neigh);
int32
pb_ds_lldp_neighbour_to_pb_free_packed(Cdb__DsLldpNeighbour *p_pb_neigh);
int32
pb_ds_lldp_neighbour_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_lldp_neighbour_dump(Cdb__DsLldpNeighbour *p_pb_neigh, char *out);

/* pb functions for TBL_IGSP_INTF DS_MROUTER_PORT */
int32
pb_ds_mrouter_port_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_mrouter_port_t *p_mr_port, Cdb__DsMrouterPort *p_pb_mr_port);
int32
pb_ds_mrouter_port_to_pb_free_packed(Cdb__DsMrouterPort *p_pb_mr_port);
int32
pb_ds_mrouter_port_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_mrouter_port_dump(Cdb__DsMrouterPort *p_pb_mr_port, char *out);

/* pb functions for TBL_IGSP_INTF DS_QUERY */
int32
pb_ds_query_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_query_t *p_query, Cdb__DsQuery *p_pb_query);
int32
pb_ds_query_to_pb_free_packed(Cdb__DsQuery *p_pb_query);
int32
pb_ds_query_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_query_dump(Cdb__DsQuery *p_pb_query, char *out);

/* pb functions for TBL_INTERFACE DS_L2PROTOCOL */
int32
pb_ds_l2protocol_to_pb(tbl_interface_key_t *p_if_key, ds_l2protocol_t *p_l2protocol, Cdb__DsL2protocol *p_pb_l2protocol);
int32
pb_ds_l2protocol_to_pb_free_packed(Cdb__DsL2protocol *p_pb_l2protocol);
int32
pb_ds_l2protocol_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_l2protocol_dump(Cdb__DsL2protocol *p_pb_l2protocol, char *out);

/* pb functions for TBL_ROUTE_IF DS_ND_PREFIX */
int32
pb_ds_nd_prefix_to_pb(tbl_route_if_key_t *p_rtif_key, ds_nd_prefix_t *p_nd_prefix, Cdb__DsNdPrefix *p_pb_nd_prefix);
int32
pb_ds_nd_prefix_to_pb_free_packed(Cdb__DsNdPrefix *p_pb_nd_prefix);
int32
pb_ds_nd_prefix_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_nd_prefix_dump(Cdb__DsNdPrefix *p_pb_nd_prefix, char *out);

/* pb functions for TBL_IGSP_INTF DS_JOIN_GROUP */
int32
pb_ds_join_group_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_join_group_t *p_join, Cdb__DsJoinGroup *p_pb_join);
int32
pb_ds_join_group_to_pb_free_packed(Cdb__DsJoinGroup *p_pb_join);
int32
pb_ds_join_group_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_join_group_dump(Cdb__DsJoinGroup *p_pb_join, char *out);

/* pb functions for TBL_CFM_MD DS_CFM_ERROR */
int32
pb_ds_cfm_error_to_pb(tbl_cfm_md_key_t *p_md_key, ds_cfm_error_t *p_cfm_error, Cdb__DsCfmError *p_pb_cfm_error);
int32
pb_ds_cfm_error_to_pb_free_packed(Cdb__DsCfmError *p_pb_cfm_error);
int32
pb_ds_cfm_error_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
int32
pb_ds_cfm_error_dump(Cdb__DsCfmError *p_pb_cfm_error, char *out);

int32
pb_ds_pack(uint32 ds_id, void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);
void*
pb_ds_unpack(uint32 ds_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data);
int32
pb_ds_dump(uint32 ds_id, void *object, char *out);
int32
pb_ds_free_unpacked(uint32 ds_id, void *object, ProtobufCAllocator *allocator);

#endif  /* __PB_DS_H__ */

