
#ifndef __PB_TBL_H__
#define __PB_TBL_H__

typedef int32 (*PB_TBL_PACK_FUNC)(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
typedef void* (*PB_TBL_UNPACK_FUNC)(ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
typedef int32 (*PB_TBL_DUMP_FUNC)(void *object, uint32 only_key, char *out);
typedef void  (*PB_TBL_FREE_UNPACKED_FUNC)(void *object, ProtobufCAllocator *allocator);

#define Cdb__TblFdbKey                 Cdb__ComposeFdbKeyT
#define CDB__TBL_FDB_KEY__INIT         CDB__COMPOSE_FDB_KEY_T__INIT
#define Cdb__TblMcfdbKey               Cdb__ComposeFdbKeyT
#define CDB__TBL_MCFDB_KEY__INIT       CDB__COMPOSE_FDB_KEY_T__INIT
#define Cdb__TblPsfdbKey               Cdb__ComposeFdbKeyT
#define CDB__TBL_PSFDB_KEY__INIT       CDB__COMPOSE_FDB_KEY_T__INIT
#define Cdb__TblIpsgFibKey             Cdb__ComposeFdbKeyT
#define CDB__TBL_IPSG_FIB_KEY__INIT    CDB__COMPOSE_FDB_KEY_T__INIT
#define Cdb__TblMstiPortKey            Cdb__ComposeMstiPortKeyT
#define CDB__TBL_MSTI_PORT_KEY__INIT   CDB__COMPOSE_MSTI_PORT_KEY_T__INIT
#define Cdb__TblBgpNetworkKey          Cdb__ComposeAddrT
#define CDB__TBL_BGP_NETWORK_KEY__INIT CDB__COMPOSE_ADDR_T__INIT
#define Cdb__TblBgpNeighborKey         Cdb__ComposeAddrT
#define CDB__TBL_BGP_NEIGHBOR_KEY__INIT CDB__COMPOSE_ADDR_T__INIT
#define Cdb__TblOspfNetworkKey         Cdb__ComposePrefixT
#define CDB__TBL_OSPF_NETWORK_KEY__INIT CDB__COMPOSE_PREFIX_T__INIT
#define Cdb__TblOspfAreaRangeKey       Cdb__ComposeOspfAreaRangeKeyT
#define CDB__TBL_OSPF_AREA_RANGE_KEY__INIT CDB__COMPOSE_OSPF_AREA_RANGE_KEY_T__INIT
#define Cdb__TblOspf6AreaRangeKey      Cdb__ComposeOspf6AreaRangeKeyT
#define CDB__TBL_OSPF6_AREA_RANGE_KEY__INIT CDB__COMPOSE_OSPF6_AREA_RANGE_KEY_T__INIT
#define Cdb__TblIprouteNodeKey         Cdb__ComposeRouteNodeKeyT
#define CDB__TBL_IPROUTE_NODE_KEY__INIT CDB__COMPOSE_ROUTE_NODE_KEY_T__INIT
#define Cdb__TblIprouteNodeAddFailKey  Cdb__ComposeRouteNodeKeyT
#define CDB__TBL_IPROUTE_NODE_ADD_FAIL_KEY__INIT CDB__COMPOSE_ROUTE_NODE_KEY_T__INIT
#define Cdb__TblIprouteNodeAddFailCountKey Cdb__ComposeRouteNodeKeyT
#define CDB__TBL_IPROUTE_NODE_ADD_FAIL_COUNT_KEY__INIT CDB__COMPOSE_ROUTE_NODE_KEY_T__INIT
#define Cdb__TblStaticRouteCfgKey      Cdb__ComposeRouteNodeKeyT
#define CDB__TBL_STATIC_ROUTE_CFG_KEY__INIT CDB__COMPOSE_ROUTE_NODE_KEY_T__INIT
#define Cdb__TblStaticRtCntKey         Cdb__ComposeRouteNodeKeyWithAdT
#define CDB__TBL_STATIC_RT_CNT_KEY__INIT CDB__COMPOSE_ROUTE_NODE_KEY_WITH_AD_T__INIT
#define Cdb__TblNexthopKey             Cdb__ComposeNexthopKeyT
#define CDB__TBL_NEXTHOP_KEY__INIT     CDB__COMPOSE_NEXTHOP_KEY_T__INIT
#define Cdb__TblNexthopGroupKey        Cdb__ComposeNexthopEcmpT
#define CDB__TBL_NEXTHOP_GROUP_KEY__INIT CDB__COMPOSE_NEXTHOP_ECMP_T__INIT
#define Cdb__TblNexthopGroupRoutedKey  Cdb__ComposeNexthopEcmpRoutedT
#define CDB__TBL_NEXTHOP_GROUP_ROUTED_KEY__INIT CDB__COMPOSE_NEXTHOP_ECMP_ROUTED_T__INIT
#define Cdb__TblFeaNexthopKey          Cdb__ComposeNexthopKeyT
#define CDB__TBL_FEA_NEXTHOP_KEY__INIT CDB__COMPOSE_NEXTHOP_KEY_T__INIT
#define Cdb__TblMrtKey                 Cdb__ComposeMrtKeyT
#define CDB__TBL_MRT_KEY__INIT         CDB__COMPOSE_MRT_KEY_T__INIT
#define Cdb__TblFeaFdbKey              Cdb__ComposeFdbKeyT
#define CDB__TBL_FEA_FDB_KEY__INIT     CDB__COMPOSE_FDB_KEY_T__INIT
#define Cdb__TblAceConfigKey           Cdb__ComposeAceConfigKeyT
#define CDB__TBL_ACE_CONFIG_KEY__INIT  CDB__COMPOSE_ACE_CONFIG_KEY_T__INIT
#define Cdb__TblRmonStatsHistoryKey    Cdb__ComposeRmonStatsHistoryKeyT
#define CDB__TBL_RMON_STATS_HISTORY_KEY__INIT CDB__COMPOSE_RMON_STATS_HISTORY_KEY_T__INIT
#define Cdb__TblQosPolicerResKey       Cdb__ComposeQosPolicerResKeyT
#define CDB__TBL_QOS_POLICER_RES_KEY__INIT CDB__COMPOSE_QOS_POLICER_RES_KEY_T__INIT
#define Cdb__TblMirrorMacEscapeKey     Cdb__ComposeMirrorMacEscapeKeyT
#define CDB__TBL_MIRROR_MAC_ESCAPE_KEY__INIT CDB__COMPOSE_MIRROR_MAC_ESCAPE_KEY_T__INIT
#define Cdb__TblTapGroupIngressKey     Cdb__ComposeTapGroupIngressKeyT
#define CDB__TBL_TAP_GROUP_INGRESS_KEY__INIT CDB__COMPOSE_TAP_GROUP_INGRESS_KEY_T__INIT
#define Cdb__TblTapGroupIngressFlowKey Cdb__ComposeTapGroupIngressFlowKeyT
#define CDB__TBL_TAP_GROUP_INGRESS_FLOW_KEY__INIT CDB__COMPOSE_TAP_GROUP_INGRESS_FLOW_KEY_T__INIT
#define Cdb__TblTapGroupEgressKey      Cdb__ComposeTapGroupEgressKeyT
#define CDB__TBL_TAP_GROUP_EGRESS_KEY__INIT CDB__COMPOSE_TAP_GROUP_EGRESS_KEY_T__INIT
#define Cdb__TblTapFiveTupleKey        Cdb__ComposeTapFiveTupleKeyT
#define CDB__TBL_TAP_FIVE_TUPLE_KEY__INIT CDB__COMPOSE_TAP_FIVE_TUPLE_KEY_T__INIT
#define Cdb__TblDhcbindingKey          Cdb__ComposeBindingKeyT
#define CDB__TBL_DHCBINDING_KEY__INIT  CDB__COMPOSE_BINDING_KEY_T__INIT
#define Cdb__TblNsPortForwardingKey    Cdb__ComposeNsPortForwardingKeyT
#define CDB__TBL_NS_PORT_FORWARDING_KEY__INIT CDB__COMPOSE_NS_PORT_FORWARDING_KEY_T__INIT
#define Cdb__TblFeaG8032RingKey        Cdb__ComposeFeaG8032RingKeyT
#define CDB__TBL_FEA_G8032_RING_KEY__INIT CDB__COMPOSE_FEA_G8032_RING_KEY_T__INIT
#define Cdb__TblErpsRingKey            Cdb__ComposeErpsRingKeyT
#define CDB__TBL_ERPS_RING_KEY__INIT   CDB__COMPOSE_ERPS_RING_KEY_T__INIT
#define Cdb__TblClassInPolicyConfigKey Cdb__ComposeClassInPolicyConfigKeyT
#define CDB__TBL_CLASS_IN_POLICY_CONFIG_KEY__INIT CDB__COMPOSE_CLASS_IN_POLICY_CONFIG_KEY_T__INIT
#define Cdb__TblAclInClassConfigKey    Cdb__ComposeAclInClassConfigKeyT
#define CDB__TBL_ACL_IN_CLASS_CONFIG_KEY__INIT CDB__COMPOSE_ACL_IN_CLASS_CONFIG_KEY_T__INIT
#define Cdb__TblClassMapActionConfigKey Cdb__ComposeClassMapActionConfigKeyT
#define CDB__TBL_CLASS_MAP_ACTION_CONFIG_KEY__INIT CDB__COMPOSE_CLASS_MAP_ACTION_CONFIG_KEY_T__INIT
#define Cdb__TblIgspGroupKey           Cdb__ComposeIgspGroupKeyT
#define CDB__TBL_IGSP_GROUP_KEY__INIT  CDB__COMPOSE_IGSP_GROUP_KEY_T__INIT
#define Cdb__TblAuthServerKey          Cdb__ComposeAuthServerKeyT
#define CDB__TBL_AUTH_SERVER_KEY__INIT CDB__COMPOSE_AUTH_SERVER_KEY_T__INIT
#define Cdb__TblAuthSessionKey         Cdb__ComposeAuthSessionKeyT
#define CDB__TBL_AUTH_SESSION_KEY__INIT CDB__COMPOSE_AUTH_SESSION_KEY_T__INIT
#define Cdb__TblDot1xRadiusKey         Cdb__ComposeDot1xRadiusKeyT
#define CDB__TBL_DOT1X_RADIUS_KEY__INIT CDB__COMPOSE_DOT1X_RADIUS_KEY_T__INIT
#define Cdb__TblDot1xMacKey            Cdb__ComposeDot1xMacKeyT
#define CDB__TBL_DOT1X_MAC_KEY__INIT   CDB__COMPOSE_DOT1X_MAC_KEY_T__INIT
#define Cdb__TblNsRouteKey             Cdb__ComposeNsRouteKeyT
#define CDB__TBL_NS_ROUTE_KEY__INIT    CDB__COMPOSE_NS_ROUTE_KEY_T__INIT
#define Cdb__TblOpenflowManagerKey     Cdb__ComposeOpenflowManagerKeyT
#define CDB__TBL_OPENFLOW_MANAGER_KEY__INIT CDB__COMPOSE_OPENFLOW_MANAGER_KEY_T__INIT
#define Cdb__TblPtpForeignKey          Cdb__ComposePtpForeignMasterT
#define CDB__TBL_PTP_FOREIGN_KEY__INIT CDB__COMPOSE_PTP_FOREIGN_MASTER_T__INIT
#define Cdb__TblPtpResidenceTimeCacheKey Cdb__ComposePtpResidenceTimeCacheT
#define CDB__TBL_PTP_RESIDENCE_TIME_CACHE_KEY__INIT CDB__COMPOSE_PTP_RESIDENCE_TIME_CACHE_T__INIT
#define Cdb__TblInbandSnatKey          Cdb__ComposeInbandSnatKeyT
#define CDB__TBL_INBAND_SNAT_KEY__INIT CDB__COMPOSE_INBAND_SNAT_KEY_T__INIT
#define Cdb__TblArpaceConfigKey        Cdb__ComposeArpaceConfigKeyT
#define CDB__TBL_ARPACE_CONFIG_KEY__INIT CDB__COMPOSE_ARPACE_CONFIG_KEY_T__INIT
#define Cdb__TblSflowCollectorKey      Cdb__ComposeSflowCollectorKeyT
#define CDB__TBL_SFLOW_COLLECTOR_KEY__INIT CDB__COMPOSE_SFLOW_COLLECTOR_KEY_T__INIT
#define Cdb__TblVrrpVipKey             Cdb__ComposeAddrT
#define CDB__TBL_VRRP_VIP_KEY__INIT    CDB__COMPOSE_ADDR_T__INIT
#define Cdb__TblIpslaPacketStatsKey    Cdb__ComposeIpslaPacketStatsKeyT
#define CDB__TBL_IPSLA_PACKET_STATS_KEY__INIT CDB__COMPOSE_IPSLA_PACKET_STATS_KEY_T__INIT
#define Cdb__TblIpslaTestStatsKey      Cdb__ComposeIpslaTestStatsKeyT
#define CDB__TBL_IPSLA_TEST_STATS_KEY__INIT CDB__COMPOSE_IPSLA_TEST_STATS_KEY_T__INIT
#define Cdb__TblControllerAttrKey      Cdb__ComposeNsRouteKeyT
#define CDB__TBL_CONTROLLER_ATTR_KEY__INIT CDB__COMPOSE_NS_ROUTE_KEY_T__INIT
#define Cdb__TblVlanMappingEntryKey    Cdb__ComposeVlanMappingEntryKeyT
#define CDB__TBL_VLAN_MAPPING_ENTRY_KEY__INIT CDB__COMPOSE_VLAN_MAPPING_ENTRY_KEY_T__INIT
#define Cdb__TblEgressMappingEntryKey  Cdb__ComposeEgressMappingEntryKeyT
#define CDB__TBL_EGRESS_MAPPING_ENTRY_KEY__INIT CDB__COMPOSE_EGRESS_MAPPING_ENTRY_KEY_T__INIT
#define Cdb__TblMappedVlanInfoKey      Cdb__ComposeMappedVlanInfoKeyT
#define CDB__TBL_MAPPED_VLAN_INFO_KEY__INIT CDB__COMPOSE_MAPPED_VLAN_INFO_KEY_T__INIT
#define Cdb__TblNdFibKey               Cdb__ComposeNdPrefixKeyT
#define CDB__TBL_ND_FIB_KEY__INIT      CDB__COMPOSE_ND_PREFIX_KEY_T__INIT
#define Cdb__TblL2editKey              Cdb__ComposeL2editKeyT
#define CDB__TBL_L2EDIT_KEY__INIT      CDB__COMPOSE_L2EDIT_KEY_T__INIT
#define Cdb__TblNatRuleKey             Cdb__ComposeNatRuleKeyT
#define CDB__TBL_NAT_RULE_KEY__INIT    CDB__COMPOSE_NAT_RULE_KEY_T__INIT
#define Cdb__TblNatSessionKey          Cdb__ComposeNatSessionKeyT
#define CDB__TBL_NAT_SESSION_KEY__INIT CDB__COMPOSE_NAT_SESSION_KEY_T__INIT
#define Cdb__TblNatSessionLimitKey     Cdb__ComposeAddrT
#define CDB__TBL_NAT_SESSION_LIMIT_KEY__INIT CDB__COMPOSE_ADDR_T__INIT
#define Cdb__TblNatProxyArpKey         Cdb__ComposeNatProxyArpKeyT
#define CDB__TBL_NAT_PROXY_ARP_KEY__INIT CDB__COMPOSE_NAT_PROXY_ARP_KEY_T__INIT
#define Cdb__TblCfmMaKey               Cdb__ComposeCfmMaKeyT
#define CDB__TBL_CFM_MA_KEY__INIT      CDB__COMPOSE_CFM_MA_KEY_T__INIT
#define Cdb__TblCfmLmepKey             Cdb__ComposeCfmLmepKeyT
#define CDB__TBL_CFM_LMEP_KEY__INIT    CDB__COMPOSE_CFM_LMEP_KEY_T__INIT
#define Cdb__TblCfmRmepKey             Cdb__ComposeCfmRmepKeyT
#define CDB__TBL_CFM_RMEP_KEY__INIT    CDB__COMPOSE_CFM_RMEP_KEY_T__INIT
#define Cdb__TblRipNetworkKey          Cdb__ComposePrefixT
#define CDB__TBL_RIP_NETWORK_KEY__INIT CDB__COMPOSE_PREFIX_T__INIT
#define Cdb__TblRipngNetworkKey        Cdb__ComposePrefixT
#define CDB__TBL_RIPNG_NETWORK_KEY__INIT CDB__COMPOSE_PREFIX_T__INIT
#define Cdb__TblRipNeighborKey         Cdb__ComposeAddrT
#define CDB__TBL_RIP_NEIGHBOR_KEY__INIT CDB__COMPOSE_ADDR_T__INIT
#define Cdb__TblOspfAreaVlinkKey       Cdb__ComposeOspfAreaVlinkKeyT
#define CDB__TBL_OSPF_AREA_VLINK_KEY__INIT CDB__COMPOSE_OSPF_AREA_VLINK_KEY_T__INIT


/* pb functions for TBL_INTERFACE */
int32
pb_tbl_interface_to_pb(uint32 only_key, tbl_interface_t *p_if, Cdb__TblInterface *p_pb_if);
int32
pb_tbl_interface_to_pb_free_packed(Cdb__TblInterface *p_pb_if);
int32
pb_tbl_interface_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_interface_dump(Cdb__TblInterface *p_pb_if, uint32 only_key, char *out);

/* pb functions for TBL_ROUTE_IF */
int32
pb_tbl_route_if_to_pb(uint32 only_key, tbl_route_if_t *p_rtif, Cdb__TblRouteIf *p_pb_rtif);
int32
pb_tbl_route_if_to_pb_free_packed(Cdb__TblRouteIf *p_pb_rtif);
int32
pb_tbl_route_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_route_if_dump(Cdb__TblRouteIf *p_pb_rtif, uint32 only_key, char *out);

/* pb functions for TBL_KERNEL_IF */
int32
pb_tbl_kernel_if_to_pb(uint32 only_key, tbl_kernel_if_t *p_kernel_if, Cdb__TblKernelIf *p_pb_kernel_if);
int32
pb_tbl_kernel_if_to_pb_free_packed(Cdb__TblKernelIf *p_pb_kernel_if);
int32
pb_tbl_kernel_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_kernel_if_dump(Cdb__TblKernelIf *p_pb_kernel_if, uint32 only_key, char *out);

/* pb functions for TBL_FEA_PORT_IF */
int32
pb_tbl_fea_port_if_to_pb(uint32 only_key, tbl_fea_port_if_t *p_portif, Cdb__TblFeaPortIf *p_pb_portif);
int32
pb_tbl_fea_port_if_to_pb_free_packed(Cdb__TblFeaPortIf *p_pb_portif);
int32
pb_tbl_fea_port_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_port_if_dump(Cdb__TblFeaPortIf *p_pb_portif, uint32 only_key, char *out);

/* pb functions for TBL_VLAN */
int32
pb_tbl_vlan_to_pb(uint32 only_key, tbl_vlan_t *p_vlan, Cdb__TblVlan *p_pb_vlan);
int32
pb_tbl_vlan_to_pb_free_packed(Cdb__TblVlan *p_pb_vlan);
int32
pb_tbl_vlan_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlan_dump(Cdb__TblVlan *p_pb_vlan, uint32 only_key, char *out);

/* pb functions for TBL_PVLAN */
int32
pb_tbl_pvlan_to_pb(uint32 only_key, tbl_pvlan_t *p_pvlan, Cdb__TblPvlan *p_pb_pvlan);
int32
pb_tbl_pvlan_to_pb_free_packed(Cdb__TblPvlan *p_pb_pvlan);
int32
pb_tbl_pvlan_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pvlan_dump(Cdb__TblPvlan *p_pb_pvlan, uint32 only_key, char *out);

/* pb functions for TBL_FDB */
int32
pb_tbl_fdb_to_pb(uint32 only_key, tbl_fdb_t *p_fdb, Cdb__TblFdb *p_pb_fdb);
int32
pb_tbl_fdb_to_pb_free_packed(Cdb__TblFdb *p_pb_fdb);
int32
pb_tbl_fdb_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fdb_dump(Cdb__TblFdb *p_pb_fdb, uint32 only_key, char *out);

/* pb functions for TBL_MCFDB */
int32
pb_tbl_mcfdb_to_pb(uint32 only_key, tbl_mcfdb_t *p_mcfdb, Cdb__TblMcfdb *p_pb_mcfdb);
int32
pb_tbl_mcfdb_to_pb_free_packed(Cdb__TblMcfdb *p_pb_mcfdb);
int32
pb_tbl_mcfdb_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mcfdb_dump(Cdb__TblMcfdb *p_pb_mcfdb, uint32 only_key, char *out);

/* pb functions for TBL_MACFILTER */
int32
pb_tbl_macfilter_to_pb(uint32 only_key, tbl_macfilter_t *p_macflt, Cdb__TblMacfilter *p_pb_macflt);
int32
pb_tbl_macfilter_to_pb_free_packed(Cdb__TblMacfilter *p_pb_macflt);
int32
pb_tbl_macfilter_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_macfilter_dump(Cdb__TblMacfilter *p_pb_macflt, uint32 only_key, char *out);

/* pb functions for TBL_PSFDB */
int32
pb_tbl_psfdb_to_pb(uint32 only_key, tbl_psfdb_t *p_psfdb, Cdb__TblPsfdb *p_pb_psfdb);
int32
pb_tbl_psfdb_to_pb_free_packed(Cdb__TblPsfdb *p_pb_psfdb);
int32
pb_tbl_psfdb_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_psfdb_dump(Cdb__TblPsfdb *p_pb_psfdb, uint32 only_key, char *out);

/* pb functions for TBL_IPSG_S_IP */
int32
pb_tbl_ipsg_s_ip_to_pb(uint32 only_key, tbl_ipsg_s_ip_t *p_ipsg_s_ip, Cdb__TblIpsgSIp *p_pb_ipsg_s_ip);
int32
pb_tbl_ipsg_s_ip_to_pb_free_packed(Cdb__TblIpsgSIp *p_pb_ipsg_s_ip);
int32
pb_tbl_ipsg_s_ip_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsg_s_ip_dump(Cdb__TblIpsgSIp *p_pb_ipsg_s_ip, uint32 only_key, char *out);

/* pb functions for TBL_IPSG_S_MAC */
int32
pb_tbl_ipsg_s_mac_to_pb(uint32 only_key, tbl_ipsg_s_mac_t *p_ipsg_s_mac, Cdb__TblIpsgSMac *p_pb_ipsg_s_mac);
int32
pb_tbl_ipsg_s_mac_to_pb_free_packed(Cdb__TblIpsgSMac *p_pb_ipsg_s_mac);
int32
pb_tbl_ipsg_s_mac_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsg_s_mac_dump(Cdb__TblIpsgSMac *p_pb_ipsg_s_mac, uint32 only_key, char *out);

/* pb functions for TBL_IPSG_FIB */
int32
pb_tbl_ipsg_fib_to_pb(uint32 only_key, tbl_ipsg_fib_t *p_ipsg_fib, Cdb__TblIpsgFib *p_pb_ipsg_fib);
int32
pb_tbl_ipsg_fib_to_pb_free_packed(Cdb__TblIpsgFib *p_pb_ipsg_fib);
int32
pb_tbl_ipsg_fib_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsg_fib_dump(Cdb__TblIpsgFib *p_pb_ipsg_fib, uint32 only_key, char *out);

/* pb functions for TBL_BRG_GLOBAL */
int32
pb_tbl_brg_global_to_pb(uint32 only_key, tbl_brg_global_t *p_brg_glb, Cdb__TblBrgGlobal *p_pb_brg_glb);
int32
pb_tbl_brg_global_to_pb_free_packed(Cdb__TblBrgGlobal *p_pb_brg_glb);
int32
pb_tbl_brg_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_brg_global_dump(Cdb__TblBrgGlobal *p_pb_brg_glb, uint32 only_key, char *out);

/* pb functions for TBL_MSTP_PORT */
int32
pb_tbl_mstp_port_to_pb(uint32 only_key, tbl_mstp_port_t *p_mstp_port, Cdb__TblMstpPort *p_pb_mstp_port);
int32
pb_tbl_mstp_port_to_pb_free_packed(Cdb__TblMstpPort *p_pb_mstp_port);
int32
pb_tbl_mstp_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mstp_port_dump(Cdb__TblMstpPort *p_pb_mstp_port, uint32 only_key, char *out);

/* pb functions for TBL_MSTI_PORT */
int32
pb_tbl_msti_port_to_pb(uint32 only_key, tbl_msti_port_t *p_msti_port, Cdb__TblMstiPort *p_pb_msti_port);
int32
pb_tbl_msti_port_to_pb_free_packed(Cdb__TblMstiPort *p_pb_msti_port);
int32
pb_tbl_msti_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_msti_port_dump(Cdb__TblMstiPort *p_pb_msti_port, uint32 only_key, char *out);

/* pb functions for TBL_MSTP_INSTANCE */
int32
pb_tbl_mstp_instance_to_pb(uint32 only_key, tbl_mstp_instance_t *p_inst, Cdb__TblMstpInstance *p_pb_inst);
int32
pb_tbl_mstp_instance_to_pb_free_packed(Cdb__TblMstpInstance *p_pb_inst);
int32
pb_tbl_mstp_instance_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mstp_instance_dump(Cdb__TblMstpInstance *p_pb_inst, uint32 only_key, char *out);

/* pb functions for TBL_MSTP_GLOBAL */
int32
pb_tbl_mstp_global_to_pb(uint32 only_key, tbl_mstp_global_t *p_mstp_glb, Cdb__TblMstpGlobal *p_pb_mstp_glb);
int32
pb_tbl_mstp_global_to_pb_free_packed(Cdb__TblMstpGlobal *p_pb_mstp_glb);
int32
pb_tbl_mstp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mstp_global_dump(Cdb__TblMstpGlobal *p_pb_mstp_glb, uint32 only_key, char *out);

/* pb functions for TBL_LLDP_GLOBAL */
int32
pb_tbl_lldp_global_to_pb(uint32 only_key, tbl_lldp_global_t *p_lldp_glb, Cdb__TblLldpGlobal *p_pb_lldp_glb);
int32
pb_tbl_lldp_global_to_pb_free_packed(Cdb__TblLldpGlobal *p_pb_lldp_glb);
int32
pb_tbl_lldp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lldp_global_dump(Cdb__TblLldpGlobal *p_pb_lldp_glb, uint32 only_key, char *out);

/* pb functions for TBL_LLDP_IF */
int32
pb_tbl_lldp_if_to_pb(uint32 only_key, tbl_lldp_if_t *p_lldp_if, Cdb__TblLldpIf *p_pb_lldp_if);
int32
pb_tbl_lldp_if_to_pb_free_packed(Cdb__TblLldpIf *p_pb_lldp_if);
int32
pb_tbl_lldp_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lldp_if_dump(Cdb__TblLldpIf *p_pb_lldp_if, uint32 only_key, char *out);

/* pb functions for TBL_MLAG */
int32
pb_tbl_mlag_to_pb(uint32 only_key, tbl_mlag_t *p_mlag, Cdb__TblMlag *p_pb_mlag);
int32
pb_tbl_mlag_to_pb_free_packed(Cdb__TblMlag *p_pb_mlag);
int32
pb_tbl_mlag_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mlag_dump(Cdb__TblMlag *p_pb_mlag, uint32 only_key, char *out);

/* pb functions for TBL_MLAG_PEER */
int32
pb_tbl_mlag_peer_to_pb(uint32 only_key, tbl_mlag_peer_t *p_peer, Cdb__TblMlagPeer *p_pb_peer);
int32
pb_tbl_mlag_peer_to_pb_free_packed(Cdb__TblMlagPeer *p_pb_peer);
int32
pb_tbl_mlag_peer_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mlag_peer_dump(Cdb__TblMlagPeer *p_pb_peer, uint32 only_key, char *out);

/* pb functions for TBL_MLAG_PORT */
int32
pb_tbl_mlag_port_to_pb(uint32 only_key, tbl_mlag_port_t *p_port, Cdb__TblMlagPort *p_pb_port);
int32
pb_tbl_mlag_port_to_pb_free_packed(Cdb__TblMlagPort *p_pb_port);
int32
pb_tbl_mlag_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mlag_port_dump(Cdb__TblMlagPort *p_pb_port, uint32 only_key, char *out);

/* pb functions for TBL_ISOLATION */
int32
pb_tbl_isolation_to_pb(uint32 only_key, tbl_isolation_t *p_iso, Cdb__TblIsolation *p_pb_iso);
int32
pb_tbl_isolation_to_pb_free_packed(Cdb__TblIsolation *p_pb_iso);
int32
pb_tbl_isolation_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_isolation_dump(Cdb__TblIsolation *p_pb_iso, uint32 only_key, char *out);

/* pb functions for TBL_ROUTE_GLOBAL */
int32
pb_tbl_route_global_to_pb(uint32 only_key, tbl_route_global_t *p_rt_glb, Cdb__TblRouteGlobal *p_pb_rt_glb);
int32
pb_tbl_route_global_to_pb_free_packed(Cdb__TblRouteGlobal *p_pb_rt_glb);
int32
pb_tbl_route_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_route_global_dump(Cdb__TblRouteGlobal *p_pb_rt_glb, uint32 only_key, char *out);

/* pb functions for TBL_BGP */
int32
pb_tbl_bgp_to_pb(uint32 only_key, tbl_bgp_t *p_bgp, Cdb__TblBgp *p_pb_bgp);
int32
pb_tbl_bgp_to_pb_free_packed(Cdb__TblBgp *p_pb_bgp);
int32
pb_tbl_bgp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bgp_dump(Cdb__TblBgp *p_pb_bgp, uint32 only_key, char *out);

/* pb functions for TBL_BGP_NETWORK */
int32
pb_tbl_bgp_network_to_pb(uint32 only_key, tbl_bgp_network_t *p_bgp_network, Cdb__TblBgpNetwork *p_pb_bgp_network);
int32
pb_tbl_bgp_network_to_pb_free_packed(Cdb__TblBgpNetwork *p_pb_bgp_network);
int32
pb_tbl_bgp_network_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bgp_network_dump(Cdb__TblBgpNetwork *p_pb_bgp_network, uint32 only_key, char *out);

/* pb functions for TBL_BGP_NEIGHBOR */
int32
pb_tbl_bgp_neighbor_to_pb(uint32 only_key, tbl_bgp_neighbor_t *p_bgp_neighbor, Cdb__TblBgpNeighbor *p_pb_bgp_neighbor);
int32
pb_tbl_bgp_neighbor_to_pb_free_packed(Cdb__TblBgpNeighbor *p_pb_bgp_neighbor);
int32
pb_tbl_bgp_neighbor_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bgp_neighbor_dump(Cdb__TblBgpNeighbor *p_pb_bgp_neighbor, uint32 only_key, char *out);

/* pb functions for TBL_OSPF */
int32
pb_tbl_ospf_to_pb(uint32 only_key, tbl_ospf_t *p_ospf, Cdb__TblOspf *p_pb_ospf);
int32
pb_tbl_ospf_to_pb_free_packed(Cdb__TblOspf *p_pb_ospf);
int32
pb_tbl_ospf_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_dump(Cdb__TblOspf *p_pb_ospf, uint32 only_key, char *out);

/* pb functions for TBL_OSPF6 */
int32
pb_tbl_ospf6_to_pb(uint32 only_key, tbl_ospf6_t *p_ospf6, Cdb__TblOspf6 *p_pb_ospf6);
int32
pb_tbl_ospf6_to_pb_free_packed(Cdb__TblOspf6 *p_pb_ospf6);
int32
pb_tbl_ospf6_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf6_dump(Cdb__TblOspf6 *p_pb_ospf6, uint32 only_key, char *out);

/* pb functions for TBL_OSPF6_INTERFACE */
int32
pb_tbl_ospf6_interface_to_pb(uint32 only_key, tbl_ospf6_interface_t *p_ospf6_interface, Cdb__TblOspf6Interface *p_pb_ospf6_interface);
int32
pb_tbl_ospf6_interface_to_pb_free_packed(Cdb__TblOspf6Interface *p_pb_ospf6_interface);
int32
pb_tbl_ospf6_interface_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf6_interface_dump(Cdb__TblOspf6Interface *p_pb_ospf6_interface, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_NETWORK */
int32
pb_tbl_ospf_network_to_pb(uint32 only_key, tbl_ospf_network_t *p_ospf_network, Cdb__TblOspfNetwork *p_pb_ospf_network);
int32
pb_tbl_ospf_network_to_pb_free_packed(Cdb__TblOspfNetwork *p_pb_ospf_network);
int32
pb_tbl_ospf_network_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_network_dump(Cdb__TblOspfNetwork *p_pb_ospf_network, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_AREA_AUTH */
int32
pb_tbl_ospf_area_auth_to_pb(uint32 only_key, tbl_ospf_area_auth_t *p_ospf_area_auth, Cdb__TblOspfAreaAuth *p_pb_ospf_area_auth);
int32
pb_tbl_ospf_area_auth_to_pb_free_packed(Cdb__TblOspfAreaAuth *p_pb_ospf_area_auth);
int32
pb_tbl_ospf_area_auth_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_area_auth_dump(Cdb__TblOspfAreaAuth *p_pb_ospf_area_auth, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_AREA_RANGE */
int32
pb_tbl_ospf_area_range_to_pb(uint32 only_key, tbl_ospf_area_range_t *p_ospf_area_range, Cdb__TblOspfAreaRange *p_pb_ospf_area_range);
int32
pb_tbl_ospf_area_range_to_pb_free_packed(Cdb__TblOspfAreaRange *p_pb_ospf_area_range);
int32
pb_tbl_ospf_area_range_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_area_range_dump(Cdb__TblOspfAreaRange *p_pb_ospf_area_range, uint32 only_key, char *out);

/* pb functions for TBL_OSPF6_AREA_RANGE */
int32
pb_tbl_ospf6_area_range_to_pb(uint32 only_key, tbl_ospf6_area_range_t *p_ospf6_area_range, Cdb__TblOspf6AreaRange *p_pb_ospf6_area_range);
int32
pb_tbl_ospf6_area_range_to_pb_free_packed(Cdb__TblOspf6AreaRange *p_pb_ospf6_area_range);
int32
pb_tbl_ospf6_area_range_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf6_area_range_dump(Cdb__TblOspf6AreaRange *p_pb_ospf6_area_range, uint32 only_key, char *out);

/* pb functions for TBL_IPROUTE_NODE */
int32
pb_tbl_iproute_node_to_pb(uint32 only_key, tbl_iproute_node_t *p_rn, Cdb__TblIprouteNode *p_pb_rn);
int32
pb_tbl_iproute_node_to_pb_free_packed(Cdb__TblIprouteNode *p_pb_rn);
int32
pb_tbl_iproute_node_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_iproute_node_dump(Cdb__TblIprouteNode *p_pb_rn, uint32 only_key, char *out);

/* pb functions for TBL_IPROUTE_NODE_ADD_FAIL */
int32
pb_tbl_iproute_node_add_fail_to_pb(uint32 only_key, tbl_iproute_node_add_fail_t *p_rn, Cdb__TblIprouteNodeAddFail *p_pb_rn);
int32
pb_tbl_iproute_node_add_fail_to_pb_free_packed(Cdb__TblIprouteNodeAddFail *p_pb_rn);
int32
pb_tbl_iproute_node_add_fail_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_iproute_node_add_fail_dump(Cdb__TblIprouteNodeAddFail *p_pb_rn, uint32 only_key, char *out);

/* pb functions for TBL_IPROUTE_NODE_ADD_FAIL_COUNT */
int32
pb_tbl_iproute_node_add_fail_count_to_pb(uint32 only_key, tbl_iproute_node_add_fail_count_t *p_rn, Cdb__TblIprouteNodeAddFailCount *p_pb_rn);
int32
pb_tbl_iproute_node_add_fail_count_to_pb_free_packed(Cdb__TblIprouteNodeAddFailCount *p_pb_rn);
int32
pb_tbl_iproute_node_add_fail_count_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_iproute_node_add_fail_count_dump(Cdb__TblIprouteNodeAddFailCount *p_pb_rn, uint32 only_key, char *out);

/* pb functions for TBL_STATIC_ROUTE_CFG */
int32
pb_tbl_static_route_cfg_to_pb(uint32 only_key, tbl_static_route_cfg_t *p_rst_cfg, Cdb__TblStaticRouteCfg *p_pb_rst_cfg);
int32
pb_tbl_static_route_cfg_to_pb_free_packed(Cdb__TblStaticRouteCfg *p_pb_rst_cfg);
int32
pb_tbl_static_route_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_static_route_cfg_dump(Cdb__TblStaticRouteCfg *p_pb_rst_cfg, uint32 only_key, char *out);

/* pb functions for TBL_STATIC_RT_CNT */
int32
pb_tbl_static_rt_cnt_to_pb(uint32 only_key, tbl_static_rt_cnt_t *p_rt_cnt, Cdb__TblStaticRtCnt *p_pb_rt_cnt);
int32
pb_tbl_static_rt_cnt_to_pb_free_packed(Cdb__TblStaticRtCnt *p_pb_rt_cnt);
int32
pb_tbl_static_rt_cnt_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_static_rt_cnt_dump(Cdb__TblStaticRtCnt *p_pb_rt_cnt, uint32 only_key, char *out);

/* pb functions for TBL_ARP_FIB */
int32
pb_tbl_arp_fib_to_pb(uint32 only_key, tbl_arp_fib_t *p_arp_fib, Cdb__TblArpFib *p_pb_arp_fib);
int32
pb_tbl_arp_fib_to_pb_free_packed(Cdb__TblArpFib *p_pb_arp_fib);
int32
pb_tbl_arp_fib_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_arp_fib_dump(Cdb__TblArpFib *p_pb_arp_fib, uint32 only_key, char *out);

/* pb functions for TBL_ARP */
int32
pb_tbl_arp_to_pb(uint32 only_key, tbl_arp_t *p_arp, Cdb__TblArp *p_pb_arp);
int32
pb_tbl_arp_to_pb_free_packed(Cdb__TblArp *p_pb_arp);
int32
pb_tbl_arp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_arp_dump(Cdb__TblArp *p_pb_arp, uint32 only_key, char *out);

/* pb functions for TBL_NEXTHOP */
int32
pb_tbl_nexthop_to_pb(uint32 only_key, tbl_nexthop_t *p_nh, Cdb__TblNexthop *p_pb_nh);
int32
pb_tbl_nexthop_to_pb_free_packed(Cdb__TblNexthop *p_pb_nh);
int32
pb_tbl_nexthop_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nexthop_dump(Cdb__TblNexthop *p_pb_nh, uint32 only_key, char *out);

/* pb functions for TBL_NEXTHOP_GROUP */
int32
pb_tbl_nexthop_group_to_pb(uint32 only_key, tbl_nexthop_group_t *p_nhg, Cdb__TblNexthopGroup *p_pb_nhg);
int32
pb_tbl_nexthop_group_to_pb_free_packed(Cdb__TblNexthopGroup *p_pb_nhg);
int32
pb_tbl_nexthop_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nexthop_group_dump(Cdb__TblNexthopGroup *p_pb_nhg, uint32 only_key, char *out);

/* pb functions for TBL_NEXTHOP_GROUP_ROUTED */
int32
pb_tbl_nexthop_group_routed_to_pb(uint32 only_key, tbl_nexthop_group_routed_t *p_nhg_routed, Cdb__TblNexthopGroupRouted *p_pb_nhg_routed);
int32
pb_tbl_nexthop_group_routed_to_pb_free_packed(Cdb__TblNexthopGroupRouted *p_pb_nhg_routed);
int32
pb_tbl_nexthop_group_routed_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nexthop_group_routed_dump(Cdb__TblNexthopGroupRouted *p_pb_nhg_routed, uint32 only_key, char *out);

/* pb functions for TBL_FEA_NEXTHOP */
int32
pb_tbl_fea_nexthop_to_pb(uint32 only_key, tbl_fea_nexthop_t *p_nh, Cdb__TblFeaNexthop *p_pb_nh);
int32
pb_tbl_fea_nexthop_to_pb_free_packed(Cdb__TblFeaNexthop *p_pb_nh);
int32
pb_tbl_fea_nexthop_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_nexthop_dump(Cdb__TblFeaNexthop *p_pb_nh, uint32 only_key, char *out);

/* pb functions for TBL_MRT */
int32
pb_tbl_mrt_to_pb(uint32 only_key, tbl_mrt_t *p_mrt, Cdb__TblMrt *p_pb_mrt);
int32
pb_tbl_mrt_to_pb_free_packed(Cdb__TblMrt *p_pb_mrt);
int32
pb_tbl_mrt_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mrt_dump(Cdb__TblMrt *p_pb_mrt, uint32 only_key, char *out);

/* pb functions for TBL_IPMC_GLOBAL */
int32
pb_tbl_ipmc_global_to_pb(uint32 only_key, tbl_ipmc_global_t *p_pim_glb, Cdb__TblIpmcGlobal *p_pb_pim_glb);
int32
pb_tbl_ipmc_global_to_pb_free_packed(Cdb__TblIpmcGlobal *p_pb_pim_glb);
int32
pb_tbl_ipmc_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipmc_global_dump(Cdb__TblIpmcGlobal *p_pb_pim_glb, uint32 only_key, char *out);

/* pb functions for TBL_IPMC_INTF */
int32
pb_tbl_ipmc_intf_to_pb(uint32 only_key, tbl_ipmc_intf_t *p_pim_intf, Cdb__TblIpmcIntf *p_pb_pim_intf);
int32
pb_tbl_ipmc_intf_to_pb_free_packed(Cdb__TblIpmcIntf *p_pb_pim_intf);
int32
pb_tbl_ipmc_intf_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipmc_intf_dump(Cdb__TblIpmcIntf *p_pb_pim_intf, uint32 only_key, char *out);

/* pb functions for TBL_PIM_STATIC_RP */
int32
pb_tbl_pim_static_rp_to_pb(uint32 only_key, tbl_pim_static_rp_t *p_pim_static_rp, Cdb__TblPimStaticRp *p_pb_pim_static_rp);
int32
pb_tbl_pim_static_rp_to_pb_free_packed(Cdb__TblPimStaticRp *p_pb_pim_static_rp);
int32
pb_tbl_pim_static_rp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_static_rp_dump(Cdb__TblPimStaticRp *p_pb_pim_static_rp, uint32 only_key, char *out);

/* pb functions for TBL_SYS_GLOBAL */
int32
pb_tbl_sys_global_to_pb(uint32 only_key, tbl_sys_global_t *p_sys_glb, Cdb__TblSysGlobal *p_pb_sys_glb);
int32
pb_tbl_sys_global_to_pb_free_packed(Cdb__TblSysGlobal *p_pb_sys_glb);
int32
pb_tbl_sys_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sys_global_dump(Cdb__TblSysGlobal *p_pb_sys_glb, uint32 only_key, char *out);

/* pb functions for TBL_LAG_GLOBAL */
int32
pb_tbl_lag_global_to_pb(uint32 only_key, tbl_lag_global_t *p_lag_glb, Cdb__TblLagGlobal *p_pb_lag_glb);
int32
pb_tbl_lag_global_to_pb_free_packed(Cdb__TblLagGlobal *p_pb_lag_glb);
int32
pb_tbl_lag_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lag_global_dump(Cdb__TblLagGlobal *p_pb_lag_glb, uint32 only_key, char *out);

/* pb functions for TBL_ECMP_GLOBAL */
int32
pb_tbl_ecmp_global_to_pb(uint32 only_key, tbl_ecmp_global_t *p_ecmp_glb, Cdb__TblEcmpGlobal *p_pb_ecmp_glb);
int32
pb_tbl_ecmp_global_to_pb_free_packed(Cdb__TblEcmpGlobal *p_pb_ecmp_glb);
int32
pb_tbl_ecmp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ecmp_global_dump(Cdb__TblEcmpGlobal *p_pb_ecmp_glb, uint32 only_key, char *out);

/* pb functions for TBL_RR_PREFIX */
int32
pb_tbl_rr_prefix_to_pb(uint32 only_key, tbl_rr_prefix_t *p_rr_prefix, Cdb__TblRrPrefix *p_pb_rr_prefix);
int32
pb_tbl_rr_prefix_to_pb_free_packed(Cdb__TblRrPrefix *p_pb_rr_prefix);
int32
pb_tbl_rr_prefix_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rr_prefix_dump(Cdb__TblRrPrefix *p_pb_rr_prefix, uint32 only_key, char *out);

/* pb functions for TBL_MEM_SUMMARY */
int32
pb_tbl_mem_summary_to_pb(uint32 only_key, tbl_mem_summary_t *p_mem_info, Cdb__TblMemSummary *p_pb_mem_info);
int32
pb_tbl_mem_summary_to_pb_free_packed(Cdb__TblMemSummary *p_pb_mem_info);
int32
pb_tbl_mem_summary_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mem_summary_dump(Cdb__TblMemSummary *p_pb_mem_info, uint32 only_key, char *out);

/* pb functions for TBL_CPU_MEM_HISTORY */
int32
pb_tbl_cpu_mem_history_to_pb(uint32 only_key, tbl_cpu_mem_history_t *p_cpu_mem_history, Cdb__TblCpuMemHistory *p_pb_cpu_mem_history);
int32
pb_tbl_cpu_mem_history_to_pb_free_packed(Cdb__TblCpuMemHistory *p_pb_cpu_mem_history);
int32
pb_tbl_cpu_mem_history_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_mem_history_dump(Cdb__TblCpuMemHistory *p_pb_cpu_mem_history, uint32 only_key, char *out);

/* pb functions for TBL_CHSM_DEBUG */
int32
pb_tbl_chsm_debug_to_pb(uint32 only_key, tbl_chsm_debug_t *p_chsmdbg, Cdb__TblChsmDebug *p_pb_chsmdbg);
int32
pb_tbl_chsm_debug_to_pb_free_packed(Cdb__TblChsmDebug *p_pb_chsmdbg);
int32
pb_tbl_chsm_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_chsm_debug_dump(Cdb__TblChsmDebug *p_pb_chsmdbg, uint32 only_key, char *out);

/* pb functions for TBL_SWITCH_DEBUG */
int32
pb_tbl_switch_debug_to_pb(uint32 only_key, tbl_switch_debug_t *p_swthdbg, Cdb__TblSwitchDebug *p_pb_swthdbg);
int32
pb_tbl_switch_debug_to_pb_free_packed(Cdb__TblSwitchDebug *p_pb_swthdbg);
int32
pb_tbl_switch_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_switch_debug_dump(Cdb__TblSwitchDebug *p_pb_swthdbg, uint32 only_key, char *out);

/* pb functions for TBL_ROUTE_DEBUG */
int32
pb_tbl_route_debug_to_pb(uint32 only_key, tbl_route_debug_t *p_rtdbg, Cdb__TblRouteDebug *p_pb_rtdbg);
int32
pb_tbl_route_debug_to_pb_free_packed(Cdb__TblRouteDebug *p_pb_rtdbg);
int32
pb_tbl_route_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_route_debug_dump(Cdb__TblRouteDebug *p_pb_rtdbg, uint32 only_key, char *out);

/* pb functions for TBL_SSM_DEBUG */
int32
pb_tbl_ssm_debug_to_pb(uint32 only_key, tbl_ssm_debug_t *p_ssmdbg, Cdb__TblSsmDebug *p_pb_ssmdbg);
int32
pb_tbl_ssm_debug_to_pb_free_packed(Cdb__TblSsmDebug *p_pb_ssmdbg);
int32
pb_tbl_ssm_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ssm_debug_dump(Cdb__TblSsmDebug *p_pb_ssmdbg, uint32 only_key, char *out);

/* pb functions for TBL_PTP_DEBUG */
int32
pb_tbl_ptp_debug_to_pb(uint32 only_key, tbl_ptp_debug_t *p_ptpdbg, Cdb__TblPtpDebug *p_pb_ptpdbg);
int32
pb_tbl_ptp_debug_to_pb_free_packed(Cdb__TblPtpDebug *p_pb_ptpdbg);
int32
pb_tbl_ptp_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ptp_debug_dump(Cdb__TblPtpDebug *p_pb_ptpdbg, uint32 only_key, char *out);

/* pb functions for TBL_QUAGGA_DEBUG */
int32
pb_tbl_quagga_debug_to_pb(uint32 only_key, tbl_quagga_debug_t *p_dbg, Cdb__TblQuaggaDebug *p_pb_dbg);
int32
pb_tbl_quagga_debug_to_pb_free_packed(Cdb__TblQuaggaDebug *p_pb_dbg);
int32
pb_tbl_quagga_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_quagga_debug_dump(Cdb__TblQuaggaDebug *p_pb_dbg, uint32 only_key, char *out);

/* pb functions for TBL_LSRV_DEBUG */
int32
pb_tbl_lsrv_debug_to_pb(uint32 only_key, tbl_lsrv_debug_t *p_lsrvdbg, Cdb__TblLsrvDebug *p_pb_lsrvdbg);
int32
pb_tbl_lsrv_debug_to_pb_free_packed(Cdb__TblLsrvDebug *p_pb_lsrvdbg);
int32
pb_tbl_lsrv_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lsrv_debug_dump(Cdb__TblLsrvDebug *p_pb_lsrvdbg, uint32 only_key, char *out);

/* pb functions for TBL_HSRV_DEBUG */
int32
pb_tbl_hsrv_debug_to_pb(uint32 only_key, tbl_hsrv_debug_t *p_hsrvdbg, Cdb__TblHsrvDebug *p_pb_hsrvdbg);
int32
pb_tbl_hsrv_debug_to_pb_free_packed(Cdb__TblHsrvDebug *p_pb_hsrvdbg);
int32
pb_tbl_hsrv_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_hsrv_debug_dump(Cdb__TblHsrvDebug *p_pb_hsrvdbg, uint32 only_key, char *out);

/* pb functions for TBL_APP_DEBUG */
int32
pb_tbl_app_debug_to_pb(uint32 only_key, tbl_app_debug_t *p_appdbg, Cdb__TblAppDebug *p_pb_appdbg);
int32
pb_tbl_app_debug_to_pb_free_packed(Cdb__TblAppDebug *p_pb_appdbg);
int32
pb_tbl_app_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_app_debug_dump(Cdb__TblAppDebug *p_pb_appdbg, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW_DEBUG */
int32
pb_tbl_openflow_debug_to_pb(uint32 only_key, tbl_openflow_debug_t *p_openflowdbg, Cdb__TblOpenflowDebug *p_pb_openflowdbg);
int32
pb_tbl_openflow_debug_to_pb_free_packed(Cdb__TblOpenflowDebug *p_pb_openflowdbg);
int32
pb_tbl_openflow_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_debug_dump(Cdb__TblOpenflowDebug *p_pb_openflowdbg, uint32 only_key, char *out);

/* pb functions for TBL_RIF */
int32
pb_tbl_rif_to_pb(uint32 only_key, tbl_rif_t *p_rif, Cdb__TblRif *p_pb_rif);
int32
pb_tbl_rif_to_pb_free_packed(Cdb__TblRif *p_pb_rif);
int32
pb_tbl_rif_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rif_dump(Cdb__TblRif *p_pb_rif, uint32 only_key, char *out);

/* pb functions for TBL_FEA_LAG */
int32
pb_tbl_fea_lag_to_pb(uint32 only_key, tbl_fea_lag_t *p_fea_lag, Cdb__TblFeaLag *p_pb_fea_lag);
int32
pb_tbl_fea_lag_to_pb_free_packed(Cdb__TblFeaLag *p_pb_fea_lag);
int32
pb_tbl_fea_lag_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_lag_dump(Cdb__TblFeaLag *p_pb_fea_lag, uint32 only_key, char *out);

/* pb functions for TBL_FEA_GLOBAL */
int32
pb_tbl_fea_global_to_pb(uint32 only_key, tbl_fea_global_t *p_fea_global, Cdb__TblFeaGlobal *p_pb_fea_global);
int32
pb_tbl_fea_global_to_pb_free_packed(Cdb__TblFeaGlobal *p_pb_fea_global);
int32
pb_tbl_fea_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_global_dump(Cdb__TblFeaGlobal *p_pb_fea_global, uint32 only_key, char *out);

/* pb functions for TBL_FEA_ACL_TABLE */
int32
pb_tbl_fea_acl_table_to_pb(uint32 only_key, tbl_fea_acl_table_t *p_fea_acl_table, Cdb__TblFeaAclTable *p_pb_fea_acl_table);
int32
pb_tbl_fea_acl_table_to_pb_free_packed(Cdb__TblFeaAclTable *p_pb_fea_acl_table);
int32
pb_tbl_fea_acl_table_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_acl_table_dump(Cdb__TblFeaAclTable *p_pb_fea_acl_table, uint32 only_key, char *out);

/* pb functions for TBL_FEA_ACL */
int32
pb_tbl_fea_acl_to_pb(uint32 only_key, tbl_fea_acl_t *p_fea_acl, Cdb__TblFeaAcl *p_pb_fea_acl);
int32
pb_tbl_fea_acl_to_pb_free_packed(Cdb__TblFeaAcl *p_pb_fea_acl);
int32
pb_tbl_fea_acl_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_acl_dump(Cdb__TblFeaAcl *p_pb_fea_acl, uint32 only_key, char *out);

/* pb functions for TBL_FEA_FDB */
int32
pb_tbl_fea_fdb_to_pb(uint32 only_key, tbl_fea_fdb_t *p_fdb, Cdb__TblFeaFdb *p_pb_fdb);
int32
pb_tbl_fea_fdb_to_pb_free_packed(Cdb__TblFeaFdb *p_pb_fdb);
int32
pb_tbl_fea_fdb_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_fdb_dump(Cdb__TblFeaFdb *p_pb_fdb, uint32 only_key, char *out);

/* pb functions for TBL_FEA_BRG_IF */
int32
pb_tbl_fea_brg_if_to_pb(uint32 only_key, tbl_fea_brg_if_t *p_brgif, Cdb__TblFeaBrgIf *p_pb_brgif);
int32
pb_tbl_fea_brg_if_to_pb_free_packed(Cdb__TblFeaBrgIf *p_pb_brgif);
int32
pb_tbl_fea_brg_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_brg_if_dump(Cdb__TblFeaBrgIf *p_pb_brgif, uint32 only_key, char *out);

/* pb functions for TBL_ACL_WORM_FILTER */
int32
pb_tbl_acl_worm_filter_to_pb(uint32 only_key, tbl_acl_worm_filter_t *p_acl_worm_filter, Cdb__TblAclWormFilter *p_pb_acl_worm_filter);
int32
pb_tbl_acl_worm_filter_to_pb_free_packed(Cdb__TblAclWormFilter *p_pb_acl_worm_filter);
int32
pb_tbl_acl_worm_filter_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_worm_filter_dump(Cdb__TblAclWormFilter *p_pb_acl_worm_filter, uint32 only_key, char *out);

/* pb functions for TBL_ACL_CONFIG */
int32
pb_tbl_acl_config_to_pb(uint32 only_key, tbl_acl_config_t *p_acl_config, Cdb__TblAclConfig *p_pb_acl_config);
int32
pb_tbl_acl_config_to_pb_free_packed(Cdb__TblAclConfig *p_pb_acl_config);
int32
pb_tbl_acl_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_config_dump(Cdb__TblAclConfig *p_pb_acl_config, uint32 only_key, char *out);

/* pb functions for TBL_ACE_CONFIG */
int32
pb_tbl_ace_config_to_pb(uint32 only_key, tbl_ace_config_t *p_ace_config, Cdb__TblAceConfig *p_pb_ace_config);
int32
pb_tbl_ace_config_to_pb_free_packed(Cdb__TblAceConfig *p_pb_ace_config);
int32
pb_tbl_ace_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ace_config_dump(Cdb__TblAceConfig *p_pb_ace_config, uint32 only_key, char *out);

/* pb functions for TBL_ACL_ENTRY */
int32
pb_tbl_acl_entry_to_pb(uint32 only_key, tbl_acl_entry_t *p_acl_entry, Cdb__TblAclEntry *p_pb_acl_entry);
int32
pb_tbl_acl_entry_to_pb_free_packed(Cdb__TblAclEntry *p_pb_acl_entry);
int32
pb_tbl_acl_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_entry_dump(Cdb__TblAclEntry *p_pb_acl_entry, uint32 only_key, char *out);

/* pb functions for TBL_ACL_ENTRY_ACTION */
int32
pb_tbl_acl_entry_action_to_pb(uint32 only_key, tbl_acl_entry_action_t *p_acl_entry_action, Cdb__TblAclEntryAction *p_pb_acl_entry_action);
int32
pb_tbl_acl_entry_action_to_pb_free_packed(Cdb__TblAclEntryAction *p_pb_acl_entry_action);
int32
pb_tbl_acl_entry_action_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_entry_action_dump(Cdb__TblAclEntryAction *p_pb_acl_entry_action, uint32 only_key, char *out);

/* pb functions for TBL_ACL_NEXTHOP_GROUP */
int32
pb_tbl_acl_nexthop_group_to_pb(uint32 only_key, tbl_acl_nexthop_group_t *p_acl_nexthop_group, Cdb__TblAclNexthopGroup *p_pb_acl_nexthop_group);
int32
pb_tbl_acl_nexthop_group_to_pb_free_packed(Cdb__TblAclNexthopGroup *p_pb_acl_nexthop_group);
int32
pb_tbl_acl_nexthop_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_nexthop_group_dump(Cdb__TblAclNexthopGroup *p_pb_acl_nexthop_group, uint32 only_key, char *out);

/* pb functions for TBL_ACL_NEXTHOP */
int32
pb_tbl_acl_nexthop_to_pb(uint32 only_key, tbl_acl_nexthop_t *p_acl_nexthop, Cdb__TblAclNexthop *p_pb_acl_nexthop);
int32
pb_tbl_acl_nexthop_to_pb_free_packed(Cdb__TblAclNexthop *p_pb_acl_nexthop);
int32
pb_tbl_acl_nexthop_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_nexthop_dump(Cdb__TblAclNexthop *p_pb_acl_nexthop, uint32 only_key, char *out);

/* pb functions for TBL_TIME_RANGE */
int32
pb_tbl_time_range_to_pb(uint32 only_key, tbl_time_range_t *p_time_range, Cdb__TblTimeRange *p_pb_time_range);
int32
pb_tbl_time_range_to_pb_free_packed(Cdb__TblTimeRange *p_pb_time_range);
int32
pb_tbl_time_range_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_time_range_dump(Cdb__TblTimeRange *p_pb_time_range, uint32 only_key, char *out);

/* pb functions for TBL_SSH_CFG */
int32
pb_tbl_ssh_cfg_to_pb(uint32 only_key, tbl_ssh_cfg_t *p_ssh_cfg, Cdb__TblSshCfg *p_pb_ssh_cfg);
int32
pb_tbl_ssh_cfg_to_pb_free_packed(Cdb__TblSshCfg *p_pb_ssh_cfg);
int32
pb_tbl_ssh_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ssh_cfg_dump(Cdb__TblSshCfg *p_pb_ssh_cfg, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_CFG */
int32
pb_tbl_snmp_cfg_to_pb(uint32 only_key, tbl_snmp_cfg_t *p_snmp_cfg, Cdb__TblSnmpCfg *p_pb_snmp_cfg);
int32
pb_tbl_snmp_cfg_to_pb_free_packed(Cdb__TblSnmpCfg *p_pb_snmp_cfg);
int32
pb_tbl_snmp_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_cfg_dump(Cdb__TblSnmpCfg *p_pb_snmp_cfg, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_VIEW */
int32
pb_tbl_snmp_view_to_pb(uint32 only_key, tbl_snmp_view_t *p_snmp_view, Cdb__TblSnmpView *p_pb_snmp_view);
int32
pb_tbl_snmp_view_to_pb_free_packed(Cdb__TblSnmpView *p_pb_snmp_view);
int32
pb_tbl_snmp_view_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_view_dump(Cdb__TblSnmpView *p_pb_snmp_view, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_COMMUNITY */
int32
pb_tbl_snmp_community_to_pb(uint32 only_key, tbl_snmp_community_t *p_snmp_community, Cdb__TblSnmpCommunity *p_pb_snmp_community);
int32
pb_tbl_snmp_community_to_pb_free_packed(Cdb__TblSnmpCommunity *p_pb_snmp_community);
int32
pb_tbl_snmp_community_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_community_dump(Cdb__TblSnmpCommunity *p_pb_snmp_community, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_CONTEXT */
int32
pb_tbl_snmp_context_to_pb(uint32 only_key, tbl_snmp_context_t *p_snmp_context, Cdb__TblSnmpContext *p_pb_snmp_context);
int32
pb_tbl_snmp_context_to_pb_free_packed(Cdb__TblSnmpContext *p_pb_snmp_context);
int32
pb_tbl_snmp_context_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_context_dump(Cdb__TblSnmpContext *p_pb_snmp_context, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_USM_USER */
int32
pb_tbl_snmp_usm_user_to_pb(uint32 only_key, tbl_snmp_usm_user_t *p_snmp_usm_user, Cdb__TblSnmpUsmUser *p_pb_snmp_usm_user);
int32
pb_tbl_snmp_usm_user_to_pb_free_packed(Cdb__TblSnmpUsmUser *p_pb_snmp_usm_user);
int32
pb_tbl_snmp_usm_user_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_usm_user_dump(Cdb__TblSnmpUsmUser *p_pb_snmp_usm_user, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_GROUP_USM */
int32
pb_tbl_snmp_group_usm_to_pb(uint32 only_key, tbl_snmp_group_usm_t *p_snmp_group_usm, Cdb__TblSnmpGroupUsm *p_pb_snmp_group_usm);
int32
pb_tbl_snmp_group_usm_to_pb_free_packed(Cdb__TblSnmpGroupUsm *p_pb_snmp_group_usm);
int32
pb_tbl_snmp_group_usm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_group_usm_dump(Cdb__TblSnmpGroupUsm *p_pb_snmp_group_usm, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_ACCESS_USM */
int32
pb_tbl_snmp_access_usm_to_pb(uint32 only_key, tbl_snmp_access_usm_t *p_snmp_access_usm, Cdb__TblSnmpAccessUsm *p_pb_snmp_access_usm);
int32
pb_tbl_snmp_access_usm_to_pb_free_packed(Cdb__TblSnmpAccessUsm *p_pb_snmp_access_usm);
int32
pb_tbl_snmp_access_usm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_access_usm_dump(Cdb__TblSnmpAccessUsm *p_pb_snmp_access_usm, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_NOTIFY */
int32
pb_tbl_snmp_notify_to_pb(uint32 only_key, tbl_snmp_notify_t *p_snmp_notify, Cdb__TblSnmpNotify *p_pb_snmp_notify);
int32
pb_tbl_snmp_notify_to_pb_free_packed(Cdb__TblSnmpNotify *p_pb_snmp_notify);
int32
pb_tbl_snmp_notify_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_notify_dump(Cdb__TblSnmpNotify *p_pb_snmp_notify, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_TARGET_ADDR */
int32
pb_tbl_snmp_target_addr_to_pb(uint32 only_key, tbl_snmp_target_addr_t *p_snmp_target_addr, Cdb__TblSnmpTargetAddr *p_pb_snmp_target_addr);
int32
pb_tbl_snmp_target_addr_to_pb_free_packed(Cdb__TblSnmpTargetAddr *p_pb_snmp_target_addr);
int32
pb_tbl_snmp_target_addr_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_target_addr_dump(Cdb__TblSnmpTargetAddr *p_pb_snmp_target_addr, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_TARGET_PARAMS */
int32
pb_tbl_snmp_target_params_to_pb(uint32 only_key, tbl_snmp_target_params_t *p_snmp_target_params, Cdb__TblSnmpTargetParams *p_pb_snmp_target_params);
int32
pb_tbl_snmp_target_params_to_pb_free_packed(Cdb__TblSnmpTargetParams *p_pb_snmp_target_params);
int32
pb_tbl_snmp_target_params_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_target_params_dump(Cdb__TblSnmpTargetParams *p_pb_snmp_target_params, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_TRAP */
int32
pb_tbl_snmp_trap_to_pb(uint32 only_key, tbl_snmp_trap_t *p_snmp_trap, Cdb__TblSnmpTrap *p_pb_snmp_trap);
int32
pb_tbl_snmp_trap_to_pb_free_packed(Cdb__TblSnmpTrap *p_pb_snmp_trap);
int32
pb_tbl_snmp_trap_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_trap_dump(Cdb__TblSnmpTrap *p_pb_snmp_trap, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_INFORM */
int32
pb_tbl_snmp_inform_to_pb(uint32 only_key, tbl_snmp_inform_t *p_snmp_inform, Cdb__TblSnmpInform *p_pb_snmp_inform);
int32
pb_tbl_snmp_inform_to_pb_free_packed(Cdb__TblSnmpInform *p_pb_snmp_inform);
int32
pb_tbl_snmp_inform_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_inform_dump(Cdb__TblSnmpInform *p_pb_snmp_inform, uint32 only_key, char *out);

/* pb functions for TBL_SNMP_TRAP_MSG */
int32
pb_tbl_snmp_trap_msg_to_pb(uint32 only_key, tbl_snmp_trap_msg_t *p_trap, Cdb__TblSnmpTrapMsg *p_pb_trap);
int32
pb_tbl_snmp_trap_msg_to_pb_free_packed(Cdb__TblSnmpTrapMsg *p_pb_trap);
int32
pb_tbl_snmp_trap_msg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_snmp_trap_msg_dump(Cdb__TblSnmpTrapMsg *p_pb_trap, uint32 only_key, char *out);

/* pb functions for TBL_RMON_EVENT */
int32
pb_tbl_rmon_event_to_pb(uint32 only_key, tbl_rmon_event_t *p_rmon_event, Cdb__TblRmonEvent *p_pb_rmon_event);
int32
pb_tbl_rmon_event_to_pb_free_packed(Cdb__TblRmonEvent *p_pb_rmon_event);
int32
pb_tbl_rmon_event_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rmon_event_dump(Cdb__TblRmonEvent *p_pb_rmon_event, uint32 only_key, char *out);

/* pb functions for TBL_RMON_ALARM */
int32
pb_tbl_rmon_alarm_to_pb(uint32 only_key, tbl_rmon_alarm_t *p_rmon_alarm, Cdb__TblRmonAlarm *p_pb_rmon_alarm);
int32
pb_tbl_rmon_alarm_to_pb_free_packed(Cdb__TblRmonAlarm *p_pb_rmon_alarm);
int32
pb_tbl_rmon_alarm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rmon_alarm_dump(Cdb__TblRmonAlarm *p_pb_rmon_alarm, uint32 only_key, char *out);

/* pb functions for TBL_RMON_ETHER_STATS */
int32
pb_tbl_rmon_ether_stats_to_pb(uint32 only_key, tbl_rmon_ether_stats_t *p_rmon_ether_stats, Cdb__TblRmonEtherStats *p_pb_rmon_ether_stats);
int32
pb_tbl_rmon_ether_stats_to_pb_free_packed(Cdb__TblRmonEtherStats *p_pb_rmon_ether_stats);
int32
pb_tbl_rmon_ether_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rmon_ether_stats_dump(Cdb__TblRmonEtherStats *p_pb_rmon_ether_stats, uint32 only_key, char *out);

/* pb functions for TBL_RMON_HISTORY */
int32
pb_tbl_rmon_history_to_pb(uint32 only_key, tbl_rmon_history_t *p_rmon_history, Cdb__TblRmonHistory *p_pb_rmon_history);
int32
pb_tbl_rmon_history_to_pb_free_packed(Cdb__TblRmonHistory *p_pb_rmon_history);
int32
pb_tbl_rmon_history_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rmon_history_dump(Cdb__TblRmonHistory *p_pb_rmon_history, uint32 only_key, char *out);

/* pb functions for TBL_RMON_STATS_HISTORY */
int32
pb_tbl_rmon_stats_history_to_pb(uint32 only_key, tbl_rmon_stats_history_t *p_rmon_stats_history, Cdb__TblRmonStatsHistory *p_pb_rmon_stats_history);
int32
pb_tbl_rmon_stats_history_to_pb_free_packed(Cdb__TblRmonStatsHistory *p_pb_rmon_stats_history);
int32
pb_tbl_rmon_stats_history_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rmon_stats_history_dump(Cdb__TblRmonStatsHistory *p_pb_rmon_stats_history, uint32 only_key, char *out);

/* pb functions for TBL_SYSLOG_CFG */
int32
pb_tbl_syslog_cfg_to_pb(uint32 only_key, tbl_syslog_cfg_t *p_syslog_cfg, Cdb__TblSyslogCfg *p_pb_syslog_cfg);
int32
pb_tbl_syslog_cfg_to_pb_free_packed(Cdb__TblSyslogCfg *p_pb_syslog_cfg);
int32
pb_tbl_syslog_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_syslog_cfg_dump(Cdb__TblSyslogCfg *p_pb_syslog_cfg, uint32 only_key, char *out);

/* pb functions for TBL_NTP_SERVER */
int32
pb_tbl_ntp_server_to_pb(uint32 only_key, tbl_ntp_server_t *p_ntp_server, Cdb__TblNtpServer *p_pb_ntp_server);
int32
pb_tbl_ntp_server_to_pb_free_packed(Cdb__TblNtpServer *p_pb_ntp_server);
int32
pb_tbl_ntp_server_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_server_dump(Cdb__TblNtpServer *p_pb_ntp_server, uint32 only_key, char *out);

/* pb functions for TBL_NTP_ACE */
int32
pb_tbl_ntp_ace_to_pb(uint32 only_key, tbl_ntp_ace_t *p_ntp_ace, Cdb__TblNtpAce *p_pb_ntp_ace);
int32
pb_tbl_ntp_ace_to_pb_free_packed(Cdb__TblNtpAce *p_pb_ntp_ace);
int32
pb_tbl_ntp_ace_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_ace_dump(Cdb__TblNtpAce *p_pb_ntp_ace, uint32 only_key, char *out);

/* pb functions for TBL_NTP_KEY */
int32
pb_tbl_ntp_key_to_pb(uint32 only_key, tbl_ntp_key_t *p_ntp_key, Cdb__TblNtpKey *p_pb_ntp_key);
int32
pb_tbl_ntp_key_to_pb_free_packed(Cdb__TblNtpKey *p_pb_ntp_key);
int32
pb_tbl_ntp_key_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_key_dump(Cdb__TblNtpKey *p_pb_ntp_key, uint32 only_key, char *out);

/* pb functions for TBL_NTP_CFG */
int32
pb_tbl_ntp_cfg_to_pb(uint32 only_key, tbl_ntp_cfg_t *p_ntp_cfg, Cdb__TblNtpCfg *p_pb_ntp_cfg);
int32
pb_tbl_ntp_cfg_to_pb_free_packed(Cdb__TblNtpCfg *p_pb_ntp_cfg);
int32
pb_tbl_ntp_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_cfg_dump(Cdb__TblNtpCfg *p_pb_ntp_cfg, uint32 only_key, char *out);

/* pb functions for TBL_NTP_IF */
int32
pb_tbl_ntp_if_to_pb(uint32 only_key, tbl_ntp_if_t *p_ntp_if, Cdb__TblNtpIf *p_pb_ntp_if);
int32
pb_tbl_ntp_if_to_pb_free_packed(Cdb__TblNtpIf *p_pb_ntp_if);
int32
pb_tbl_ntp_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_if_dump(Cdb__TblNtpIf *p_pb_ntp_if, uint32 only_key, char *out);

/* pb functions for TBL_NTP_SYNCSTATUS */
int32
pb_tbl_ntp_syncstatus_to_pb(uint32 only_key, tbl_ntp_syncstatus_t *p_ntp_syncstatus, Cdb__TblNtpSyncstatus *p_pb_ntp_syncstatus);
int32
pb_tbl_ntp_syncstatus_to_pb_free_packed(Cdb__TblNtpSyncstatus *p_pb_ntp_syncstatus);
int32
pb_tbl_ntp_syncstatus_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ntp_syncstatus_dump(Cdb__TblNtpSyncstatus *p_pb_ntp_syncstatus, uint32 only_key, char *out);

/* pb functions for TBL_STATIC_DNS */
int32
pb_tbl_static_dns_to_pb(uint32 only_key, tbl_static_dns_t *p_static_cfg, Cdb__TblStaticDns *p_pb_static_cfg);
int32
pb_tbl_static_dns_to_pb_free_packed(Cdb__TblStaticDns *p_pb_static_cfg);
int32
pb_tbl_static_dns_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_static_dns_dump(Cdb__TblStaticDns *p_pb_static_cfg, uint32 only_key, char *out);

/* pb functions for TBL_DYNAMIC_DNS_DOMAIN */
int32
pb_tbl_dynamic_dns_domain_to_pb(uint32 only_key, tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, Cdb__TblDynamicDnsDomain *p_pb_dynamic_domain_cfg);
int32
pb_tbl_dynamic_dns_domain_to_pb_free_packed(Cdb__TblDynamicDnsDomain *p_pb_dynamic_domain_cfg);
int32
pb_tbl_dynamic_dns_domain_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dynamic_dns_domain_dump(Cdb__TblDynamicDnsDomain *p_pb_dynamic_domain_cfg, uint32 only_key, char *out);

/* pb functions for TBL_DYNAMIC_DNS_SERVER */
int32
pb_tbl_dynamic_dns_server_to_pb(uint32 only_key, tbl_dynamic_dns_server_t *p_dynamic_server_cfg, Cdb__TblDynamicDnsServer *p_pb_dynamic_server_cfg);
int32
pb_tbl_dynamic_dns_server_to_pb_free_packed(Cdb__TblDynamicDnsServer *p_pb_dynamic_server_cfg);
int32
pb_tbl_dynamic_dns_server_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dynamic_dns_server_dump(Cdb__TblDynamicDnsServer *p_pb_dynamic_server_cfg, uint32 only_key, char *out);

/* pb functions for TBL_QOS_DOMAIN */
int32
pb_tbl_qos_domain_to_pb(uint32 only_key, tbl_qos_domain_t *p_qos_domain, Cdb__TblQosDomain *p_pb_qos_domain);
int32
pb_tbl_qos_domain_to_pb_free_packed(Cdb__TblQosDomain *p_pb_qos_domain);
int32
pb_tbl_qos_domain_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_domain_dump(Cdb__TblQosDomain *p_pb_qos_domain, uint32 only_key, char *out);

/* pb functions for TBL_QOS_MPLS_DOMAIN */
int32
pb_tbl_qos_mpls_domain_to_pb(uint32 only_key, tbl_qos_mpls_domain_t *p_qos_mpls_domain, Cdb__TblQosMplsDomain *p_pb_qos_mpls_domain);
int32
pb_tbl_qos_mpls_domain_to_pb_free_packed(Cdb__TblQosMplsDomain *p_pb_qos_mpls_domain);
int32
pb_tbl_qos_mpls_domain_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_mpls_domain_dump(Cdb__TblQosMplsDomain *p_pb_qos_mpls_domain, uint32 only_key, char *out);

/* pb functions for TBL_QOS_POLICER_RES */
int32
pb_tbl_qos_policer_res_to_pb(uint32 only_key, tbl_qos_policer_res_t *p_qos_policer_res, Cdb__TblQosPolicerRes *p_pb_qos_policer_res);
int32
pb_tbl_qos_policer_res_to_pb_free_packed(Cdb__TblQosPolicerRes *p_pb_qos_policer_res);
int32
pb_tbl_qos_policer_res_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_policer_res_dump(Cdb__TblQosPolicerRes *p_pb_qos_policer_res, uint32 only_key, char *out);

/* pb functions for TBL_QOS_POLICER_ACTION_PROFILE */
int32
pb_tbl_qos_policer_action_profile_to_pb(uint32 only_key, tbl_qos_policer_action_profile_t *p_qos_policer_action_profile, Cdb__TblQosPolicerActionProfile *p_pb_qos_policer_action_profile);
int32
pb_tbl_qos_policer_action_profile_to_pb_free_packed(Cdb__TblQosPolicerActionProfile *p_pb_qos_policer_action_profile);
int32
pb_tbl_qos_policer_action_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_policer_action_profile_dump(Cdb__TblQosPolicerActionProfile *p_pb_qos_policer_action_profile, uint32 only_key, char *out);

/* pb functions for TBL_QOS_POLICER_PROFILE */
int32
pb_tbl_qos_policer_profile_to_pb(uint32 only_key, tbl_qos_policer_profile_t *p_qos_policer_profile, Cdb__TblQosPolicerProfile *p_pb_qos_policer_profile);
int32
pb_tbl_qos_policer_profile_to_pb_free_packed(Cdb__TblQosPolicerProfile *p_pb_qos_policer_profile);
int32
pb_tbl_qos_policer_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_policer_profile_dump(Cdb__TblQosPolicerProfile *p_pb_qos_policer_profile, uint32 only_key, char *out);

/* pb functions for TBL_QOS_DROP_PROFILE */
int32
pb_tbl_qos_drop_profile_to_pb(uint32 only_key, tbl_qos_drop_profile_t *p_qos_drop_profile, Cdb__TblQosDropProfile *p_pb_qos_drop_profile);
int32
pb_tbl_qos_drop_profile_to_pb_free_packed(Cdb__TblQosDropProfile *p_pb_qos_drop_profile);
int32
pb_tbl_qos_drop_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_drop_profile_dump(Cdb__TblQosDropProfile *p_pb_qos_drop_profile, uint32 only_key, char *out);

/* pb functions for TBL_QOS_QUEUE_SHAPE_PROFILE */
int32
pb_tbl_qos_queue_shape_profile_to_pb(uint32 only_key, tbl_qos_queue_shape_profile_t *p_qos_queue_shape_profile, Cdb__TblQosQueueShapeProfile *p_pb_qos_queue_shape_profile);
int32
pb_tbl_qos_queue_shape_profile_to_pb_free_packed(Cdb__TblQosQueueShapeProfile *p_pb_qos_queue_shape_profile);
int32
pb_tbl_qos_queue_shape_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_queue_shape_profile_dump(Cdb__TblQosQueueShapeProfile *p_pb_qos_queue_shape_profile, uint32 only_key, char *out);

/* pb functions for TBL_QOS_PORT_SHAPE_PROFILE */
int32
pb_tbl_qos_port_shape_profile_to_pb(uint32 only_key, tbl_qos_port_shape_profile_t *p_qos_port_shape_profile, Cdb__TblQosPortShapeProfile *p_pb_qos_port_shape_profile);
int32
pb_tbl_qos_port_shape_profile_to_pb_free_packed(Cdb__TblQosPortShapeProfile *p_pb_qos_port_shape_profile);
int32
pb_tbl_qos_port_shape_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_port_shape_profile_dump(Cdb__TblQosPortShapeProfile *p_pb_qos_port_shape_profile, uint32 only_key, char *out);

/* pb functions for TBL_QOS_QUEUE_SMART_BUFFER */
int32
pb_tbl_qos_queue_smart_buffer_to_pb(uint32 only_key, tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, Cdb__TblQosQueueSmartBuffer *p_pb_qos_queue_smart_buffer);
int32
pb_tbl_qos_queue_smart_buffer_to_pb_free_packed(Cdb__TblQosQueueSmartBuffer *p_pb_qos_queue_smart_buffer);
int32
pb_tbl_qos_queue_smart_buffer_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_queue_smart_buffer_dump(Cdb__TblQosQueueSmartBuffer *p_pb_qos_queue_smart_buffer, uint32 only_key, char *out);

/* pb functions for TBL_QOS_GLOBAL */
int32
pb_tbl_qos_global_to_pb(uint32 only_key, tbl_qos_global_t *p_qos_glb, Cdb__TblQosGlobal *p_pb_qos_glb);
int32
pb_tbl_qos_global_to_pb_free_packed(Cdb__TblQosGlobal *p_pb_qos_glb);
int32
pb_tbl_qos_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_qos_global_dump(Cdb__TblQosGlobal *p_pb_qos_glb, uint32 only_key, char *out);

/* pb functions for TBL_MIRROR */
int32
pb_tbl_mirror_to_pb(uint32 only_key, tbl_mirror_t *p_mirror, Cdb__TblMirror *p_pb_mirror);
int32
pb_tbl_mirror_to_pb_free_packed(Cdb__TblMirror *p_pb_mirror);
int32
pb_tbl_mirror_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mirror_dump(Cdb__TblMirror *p_pb_mirror, uint32 only_key, char *out);

/* pb functions for TBL_MIRROR_MAC_ESCAPE */
int32
pb_tbl_mirror_mac_escape_to_pb(uint32 only_key, tbl_mirror_mac_escape_t *p_mirror_mac_escape, Cdb__TblMirrorMacEscape *p_pb_mirror_mac_escape);
int32
pb_tbl_mirror_mac_escape_to_pb_free_packed(Cdb__TblMirrorMacEscape *p_pb_mirror_mac_escape);
int32
pb_tbl_mirror_mac_escape_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mirror_mac_escape_dump(Cdb__TblMirrorMacEscape *p_pb_mirror_mac_escape, uint32 only_key, char *out);

/* pb functions for TBL_TAP_GROUP_INGRESS */
int32
pb_tbl_tap_group_ingress_to_pb(uint32 only_key, tbl_tap_group_ingress_t *p_tap_group_ingress, Cdb__TblTapGroupIngress *p_pb_tap_group_ingress);
int32
pb_tbl_tap_group_ingress_to_pb_free_packed(Cdb__TblTapGroupIngress *p_pb_tap_group_ingress);
int32
pb_tbl_tap_group_ingress_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_group_ingress_dump(Cdb__TblTapGroupIngress *p_pb_tap_group_ingress, uint32 only_key, char *out);

/* pb functions for TBL_TAP_GROUP_INGRESS_FLOW */
int32
pb_tbl_tap_group_ingress_flow_to_pb(uint32 only_key, tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, Cdb__TblTapGroupIngressFlow *p_pb_tap_group_ingress_flow);
int32
pb_tbl_tap_group_ingress_flow_to_pb_free_packed(Cdb__TblTapGroupIngressFlow *p_pb_tap_group_ingress_flow);
int32
pb_tbl_tap_group_ingress_flow_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_group_ingress_flow_dump(Cdb__TblTapGroupIngressFlow *p_pb_tap_group_ingress_flow, uint32 only_key, char *out);

/* pb functions for TBL_TAP_GROUP_EGRESS */
int32
pb_tbl_tap_group_egress_to_pb(uint32 only_key, tbl_tap_group_egress_t *p_tap_group_egress, Cdb__TblTapGroupEgress *p_pb_tap_group_egress);
int32
pb_tbl_tap_group_egress_to_pb_free_packed(Cdb__TblTapGroupEgress *p_pb_tap_group_egress);
int32
pb_tbl_tap_group_egress_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_group_egress_dump(Cdb__TblTapGroupEgress *p_pb_tap_group_egress, uint32 only_key, char *out);

/* pb functions for TBL_TAP_GROUP */
int32
pb_tbl_tap_group_to_pb(uint32 only_key, tbl_tap_group_t *p_tap_group, Cdb__TblTapGroup *p_pb_tap_group);
int32
pb_tbl_tap_group_to_pb_free_packed(Cdb__TblTapGroup *p_pb_tap_group);
int32
pb_tbl_tap_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_group_dump(Cdb__TblTapGroup *p_pb_tap_group, uint32 only_key, char *out);

/* pb functions for TBL_TAP_FIVE_TUPLE */
int32
pb_tbl_tap_five_tuple_to_pb(uint32 only_key, tbl_tap_five_tuple_t *p_tap_five_tuple, Cdb__TblTapFiveTuple *p_pb_tap_five_tuple);
int32
pb_tbl_tap_five_tuple_to_pb_free_packed(Cdb__TblTapFiveTuple *p_pb_tap_five_tuple);
int32
pb_tbl_tap_five_tuple_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_five_tuple_dump(Cdb__TblTapFiveTuple *p_pb_tap_five_tuple, uint32 only_key, char *out);

/* pb functions for TBL_TAP_DPI_GLOBAL */
int32
pb_tbl_tap_dpi_global_to_pb(uint32 only_key, tbl_tap_dpi_global_t *p_tap_dpi_global, Cdb__TblTapDpiGlobal *p_pb_tap_dpi_global);
int32
pb_tbl_tap_dpi_global_to_pb_free_packed(Cdb__TblTapDpiGlobal *p_pb_tap_dpi_global);
int32
pb_tbl_tap_dpi_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_dpi_global_dump(Cdb__TblTapDpiGlobal *p_pb_tap_dpi_global, uint32 only_key, char *out);

/* pb functions for TBL_USER */
int32
pb_tbl_user_to_pb(uint32 only_key, tbl_user_t *p_user, Cdb__TblUser *p_pb_user);
int32
pb_tbl_user_to_pb_free_packed(Cdb__TblUser *p_pb_user);
int32
pb_tbl_user_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_user_dump(Cdb__TblUser *p_pb_user, uint32 only_key, char *out);

/* pb functions for TBL_LOGIN_RECORD */
int32
pb_tbl_login_record_to_pb(uint32 only_key, tbl_login_record_t *p_login_record, Cdb__TblLoginRecord *p_pb_login_record);
int32
pb_tbl_login_record_to_pb_free_packed(Cdb__TblLoginRecord *p_pb_login_record);
int32
pb_tbl_login_record_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_login_record_dump(Cdb__TblLoginRecord *p_pb_login_record, uint32 only_key, char *out);

/* pb functions for TBL_LOGIN_SECURITY_GLB */
int32
pb_tbl_login_security_glb_to_pb(uint32 only_key, tbl_login_security_glb_t *p_login_security_glb, Cdb__TblLoginSecurityGlb *p_pb_login_security_glb);
int32
pb_tbl_login_security_glb_to_pb_free_packed(Cdb__TblLoginSecurityGlb *p_pb_login_security_glb);
int32
pb_tbl_login_security_glb_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_login_security_glb_dump(Cdb__TblLoginSecurityGlb *p_pb_login_security_glb, uint32 only_key, char *out);

/* pb functions for TBL_VTY */
int32
pb_tbl_vty_to_pb(uint32 only_key, tbl_vty_t *p_vty, Cdb__TblVty *p_pb_vty);
int32
pb_tbl_vty_to_pb_free_packed(Cdb__TblVty *p_pb_vty);
int32
pb_tbl_vty_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vty_dump(Cdb__TblVty *p_pb_vty, uint32 only_key, char *out);

/* pb functions for TBL_CONSOLE */
int32
pb_tbl_console_to_pb(uint32 only_key, tbl_console_t *p_console, Cdb__TblConsole *p_pb_console);
int32
pb_tbl_console_to_pb_free_packed(Cdb__TblConsole *p_pb_console);
int32
pb_tbl_console_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_console_dump(Cdb__TblConsole *p_pb_console, uint32 only_key, char *out);

/* pb functions for TBL_AUTHEN */
int32
pb_tbl_authen_to_pb(uint32 only_key, tbl_authen_t *p_authen, Cdb__TblAuthen *p_pb_authen);
int32
pb_tbl_authen_to_pb_free_packed(Cdb__TblAuthen *p_pb_authen);
int32
pb_tbl_authen_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_authen_dump(Cdb__TblAuthen *p_pb_authen, uint32 only_key, char *out);

/* pb functions for TBL_LOGIN */
int32
pb_tbl_login_to_pb(uint32 only_key, tbl_login_t *p_login, Cdb__TblLogin *p_pb_login);
int32
pb_tbl_login_to_pb_free_packed(Cdb__TblLogin *p_pb_login);
int32
pb_tbl_login_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_login_dump(Cdb__TblLogin *p_pb_login, uint32 only_key, char *out);

/* pb functions for TBL_RSA */
int32
pb_tbl_rsa_to_pb(uint32 only_key, tbl_rsa_t *p_rsa, Cdb__TblRsa *p_pb_rsa);
int32
pb_tbl_rsa_to_pb_free_packed(Cdb__TblRsa *p_pb_rsa);
int32
pb_tbl_rsa_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rsa_dump(Cdb__TblRsa *p_pb_rsa, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW */
int32
pb_tbl_openflow_to_pb(uint32 only_key, tbl_openflow_t *p_openflow, Cdb__TblOpenflow *p_pb_openflow);
int32
pb_tbl_openflow_to_pb_free_packed(Cdb__TblOpenflow *p_pb_openflow);
int32
pb_tbl_openflow_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_dump(Cdb__TblOpenflow *p_pb_openflow, uint32 only_key, char *out);

/* pb functions for TBL_CPU_TRAFFIC */
int32
pb_tbl_cpu_traffic_to_pb(uint32 only_key, tbl_cpu_traffic_t *p_cpu_traffic, Cdb__TblCpuTraffic *p_pb_cpu_traffic);
int32
pb_tbl_cpu_traffic_to_pb_free_packed(Cdb__TblCpuTraffic *p_pb_cpu_traffic);
int32
pb_tbl_cpu_traffic_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_traffic_dump(Cdb__TblCpuTraffic *p_pb_cpu_traffic, uint32 only_key, char *out);

/* pb functions for TBL_CPU_TRAFFIC_GROUP */
int32
pb_tbl_cpu_traffic_group_to_pb(uint32 only_key, tbl_cpu_traffic_group_t *p_cpu_traffic_group, Cdb__TblCpuTrafficGroup *p_pb_cpu_traffic_group);
int32
pb_tbl_cpu_traffic_group_to_pb_free_packed(Cdb__TblCpuTrafficGroup *p_pb_cpu_traffic_group);
int32
pb_tbl_cpu_traffic_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_traffic_group_dump(Cdb__TblCpuTrafficGroup *p_pb_cpu_traffic_group, uint32 only_key, char *out);

/* pb functions for TBL_CPU_UTILIZATION */
int32
pb_tbl_cpu_utilization_to_pb(uint32 only_key, tbl_cpu_utilization_t *p_cpu_utilize, Cdb__TblCpuUtilization *p_pb_cpu_utilize);
int32
pb_tbl_cpu_utilization_to_pb_free_packed(Cdb__TblCpuUtilization *p_pb_cpu_utilize);
int32
pb_tbl_cpu_utilization_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_utilization_dump(Cdb__TblCpuUtilization *p_pb_cpu_utilize, uint32 only_key, char *out);

/* pb functions for TBL_CPU_LIMIT */
int32
pb_tbl_cpu_limit_to_pb(uint32 only_key, tbl_cpu_limit_t *p_cpu_limit, Cdb__TblCpuLimit *p_pb_cpu_limit);
int32
pb_tbl_cpu_limit_to_pb_free_packed(Cdb__TblCpuLimit *p_pb_cpu_limit);
int32
pb_tbl_cpu_limit_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_limit_dump(Cdb__TblCpuLimit *p_pb_cpu_limit, uint32 only_key, char *out);

/* pb functions for TBL_DHCRELAY */
int32
pb_tbl_dhcrelay_to_pb(uint32 only_key, tbl_dhcrelay_t *p_dhcrelay, Cdb__TblDhcrelay *p_pb_dhcrelay);
int32
pb_tbl_dhcrelay_to_pb_free_packed(Cdb__TblDhcrelay *p_pb_dhcrelay);
int32
pb_tbl_dhcrelay_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcrelay_dump(Cdb__TblDhcrelay *p_pb_dhcrelay, uint32 only_key, char *out);

/* pb functions for TBL_DHCSRVGRP */
int32
pb_tbl_dhcsrvgrp_to_pb(uint32 only_key, tbl_dhcsrvgrp_t *p_dhcsrvgrp, Cdb__TblDhcsrvgrp *p_pb_dhcsrvgrp);
int32
pb_tbl_dhcsrvgrp_to_pb_free_packed(Cdb__TblDhcsrvgrp *p_pb_dhcsrvgrp);
int32
pb_tbl_dhcsrvgrp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcsrvgrp_dump(Cdb__TblDhcsrvgrp *p_pb_dhcsrvgrp, uint32 only_key, char *out);

/* pb functions for TBL_DHCP_DEBUG */
int32
pb_tbl_dhcp_debug_to_pb(uint32 only_key, tbl_dhcp_debug_t *p_dhcp_debug, Cdb__TblDhcpDebug *p_pb_dhcp_debug);
int32
pb_tbl_dhcp_debug_to_pb_free_packed(Cdb__TblDhcpDebug *p_pb_dhcp_debug);
int32
pb_tbl_dhcp_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcp_debug_dump(Cdb__TblDhcpDebug *p_pb_dhcp_debug, uint32 only_key, char *out);

/* pb functions for TBL_DHCLIENT */
int32
pb_tbl_dhclient_to_pb(uint32 only_key, tbl_dhclient_t *p_dhclient, Cdb__TblDhclient *p_pb_dhclient);
int32
pb_tbl_dhclient_to_pb_free_packed(Cdb__TblDhclient *p_pb_dhclient);
int32
pb_tbl_dhclient_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhclient_dump(Cdb__TblDhclient *p_pb_dhclient, uint32 only_key, char *out);

/* pb functions for TBL_DHCSNOOPING */
int32
pb_tbl_dhcsnooping_to_pb(uint32 only_key, tbl_dhcsnooping_t *p_dhcsnooping, Cdb__TblDhcsnooping *p_pb_dhcsnooping);
int32
pb_tbl_dhcsnooping_to_pb_free_packed(Cdb__TblDhcsnooping *p_pb_dhcsnooping);
int32
pb_tbl_dhcsnooping_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcsnooping_dump(Cdb__TblDhcsnooping *p_pb_dhcsnooping, uint32 only_key, char *out);

/* pb functions for TBL_DHCBINDING */
int32
pb_tbl_dhcbinding_to_pb(uint32 only_key, tbl_dhcbinding_t *p_dhcbinding, Cdb__TblDhcbinding *p_pb_dhcbinding);
int32
pb_tbl_dhcbinding_to_pb_free_packed(Cdb__TblDhcbinding *p_pb_dhcbinding);
int32
pb_tbl_dhcbinding_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcbinding_dump(Cdb__TblDhcbinding *p_pb_dhcbinding, uint32 only_key, char *out);

/* pb functions for TBL_IPTABLES_PREVENT */
int32
pb_tbl_iptables_prevent_to_pb(uint32 only_key, tbl_iptables_prevent_t *p_iptables_prevent, Cdb__TblIptablesPrevent *p_pb_iptables_prevent);
int32
pb_tbl_iptables_prevent_to_pb_free_packed(Cdb__TblIptablesPrevent *p_pb_iptables_prevent);
int32
pb_tbl_iptables_prevent_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_iptables_prevent_dump(Cdb__TblIptablesPrevent *p_pb_iptables_prevent, uint32 only_key, char *out);

/* pb functions for TBL_ERRDISABLE */
int32
pb_tbl_errdisable_to_pb(uint32 only_key, tbl_errdisable_t *p_errdisable, Cdb__TblErrdisable *p_pb_errdisable);
int32
pb_tbl_errdisable_to_pb_free_packed(Cdb__TblErrdisable *p_pb_errdisable);
int32
pb_tbl_errdisable_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_errdisable_dump(Cdb__TblErrdisable *p_pb_errdisable, uint32 only_key, char *out);

/* pb functions for TBL_NS_PORT_FORWARDING */
int32
pb_tbl_ns_port_forwarding_to_pb(uint32 only_key, tbl_ns_port_forwarding_t *p_ns_port_forwarding, Cdb__TblNsPortForwarding *p_pb_ns_port_forwarding);
int32
pb_tbl_ns_port_forwarding_to_pb_free_packed(Cdb__TblNsPortForwarding *p_pb_ns_port_forwarding);
int32
pb_tbl_ns_port_forwarding_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ns_port_forwarding_dump(Cdb__TblNsPortForwarding *p_pb_ns_port_forwarding, uint32 only_key, char *out);

/* pb functions for TBL_LOG_GLOBAL */
int32
pb_tbl_log_global_to_pb(uint32 only_key, tbl_log_global_t *p_log_glb, Cdb__TblLogGlobal *p_pb_log_glb);
int32
pb_tbl_log_global_to_pb_free_packed(Cdb__TblLogGlobal *p_pb_log_glb);
int32
pb_tbl_log_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_log_global_dump(Cdb__TblLogGlobal *p_pb_log_glb, uint32 only_key, char *out);

/* pb functions for TBL_LOG */
int32
pb_tbl_log_to_pb(uint32 only_key, tbl_log_t *p_log, Cdb__TblLog *p_pb_log);
int32
pb_tbl_log_to_pb_free_packed(Cdb__TblLog *p_pb_log);
int32
pb_tbl_log_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_log_dump(Cdb__TblLog *p_pb_log, uint32 only_key, char *out);

/* pb functions for TBL_SYS_LOAD */
int32
pb_tbl_sys_load_to_pb(uint32 only_key, tbl_sys_load_t *p_sys_load, Cdb__TblSysLoad *p_pb_sys_load);
int32
pb_tbl_sys_load_to_pb_free_packed(Cdb__TblSysLoad *p_pb_sys_load);
int32
pb_tbl_sys_load_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sys_load_dump(Cdb__TblSysLoad *p_pb_sys_load, uint32 only_key, char *out);

/* pb functions for TBL_CEM */
int32
pb_tbl_cem_to_pb(uint32 only_key, tbl_cem_t *p_cem, Cdb__TblCem *p_pb_cem);
int32
pb_tbl_cem_to_pb_free_packed(Cdb__TblCem *p_pb_cem);
int32
pb_tbl_cem_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cem_dump(Cdb__TblCem *p_pb_cem, uint32 only_key, char *out);

/* pb functions for TBL_CLOCK */
int32
pb_tbl_clock_to_pb(uint32 only_key, tbl_clock_t *p_clk, Cdb__TblClock *p_pb_clk);
int32
pb_tbl_clock_to_pb_free_packed(Cdb__TblClock *p_pb_clk);
int32
pb_tbl_clock_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_clock_dump(Cdb__TblClock *p_pb_clk, uint32 only_key, char *out);

/* pb functions for TBL_TAG_SUMMER_CLOCK */
int32
pb_tbl_tag_summer_clock_to_pb(uint32 only_key, tbl_tag_summer_clock_t *p_tag_summer_clk, Cdb__TblTagSummerClock *p_pb_tag_summer_clk);
int32
pb_tbl_tag_summer_clock_to_pb_free_packed(Cdb__TblTagSummerClock *p_pb_tag_summer_clk);
int32
pb_tbl_tag_summer_clock_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tag_summer_clock_dump(Cdb__TblTagSummerClock *p_pb_tag_summer_clk, uint32 only_key, char *out);

/* pb functions for TBL_PORT_STATS */
int32
pb_tbl_port_stats_to_pb(uint32 only_key, tbl_port_stats_t *p_port_stats, Cdb__TblPortStats *p_pb_port_stats);
int32
pb_tbl_port_stats_to_pb_free_packed(Cdb__TblPortStats *p_pb_port_stats);
int32
pb_tbl_port_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_port_stats_dump(Cdb__TblPortStats *p_pb_port_stats, uint32 only_key, char *out);

/* pb functions for TBL_VLAN_STATS */
int32
pb_tbl_vlan_stats_to_pb(uint32 only_key, tbl_vlan_stats_t *p_vlan_stats, Cdb__TblVlanStats *p_pb_vlan_stats);
int32
pb_tbl_vlan_stats_to_pb_free_packed(Cdb__TblVlanStats *p_pb_vlan_stats);
int32
pb_tbl_vlan_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlan_stats_dump(Cdb__TblVlanStats *p_pb_vlan_stats, uint32 only_key, char *out);

/* pb functions for TBL_PORT_STATS_RATE */
int32
pb_tbl_port_stats_rate_to_pb(uint32 only_key, tbl_port_stats_rate_t *p_port_stats_rate, Cdb__TblPortStatsRate *p_pb_port_stats_rate);
int32
pb_tbl_port_stats_rate_to_pb_free_packed(Cdb__TblPortStatsRate *p_pb_port_stats_rate);
int32
pb_tbl_port_stats_rate_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_port_stats_rate_dump(Cdb__TblPortStatsRate *p_pb_port_stats_rate, uint32 only_key, char *out);

/* pb functions for TBL_ACLQOS_IF */
int32
pb_tbl_aclqos_if_to_pb(uint32 only_key, tbl_aclqos_if_t *p_aclqos_if, Cdb__TblAclqosIf *p_pb_aclqos_if);
int32
pb_tbl_aclqos_if_to_pb_free_packed(Cdb__TblAclqosIf *p_pb_aclqos_if);
int32
pb_tbl_aclqos_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_aclqos_if_dump(Cdb__TblAclqosIf *p_pb_aclqos_if, uint32 only_key, char *out);

/* pb functions for TBL_L2_ACTION */
int32
pb_tbl_l2_action_to_pb(uint32 only_key, tbl_l2_action_t *p_l2_act, Cdb__TblL2Action *p_pb_l2_act);
int32
pb_tbl_l2_action_to_pb_free_packed(Cdb__TblL2Action *p_pb_l2_act);
int32
pb_tbl_l2_action_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_l2_action_dump(Cdb__TblL2Action *p_pb_l2_act, uint32 only_key, char *out);

/* pb functions for TBL_FEA_QOS_DROP_PROFILE */
int32
pb_tbl_fea_qos_drop_profile_to_pb(uint32 only_key, tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, Cdb__TblFeaQosDropProfile *p_pb_fea_qos_drop_profile);
int32
pb_tbl_fea_qos_drop_profile_to_pb_free_packed(Cdb__TblFeaQosDropProfile *p_pb_fea_qos_drop_profile);
int32
pb_tbl_fea_qos_drop_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_qos_drop_profile_dump(Cdb__TblFeaQosDropProfile *p_pb_fea_qos_drop_profile, uint32 only_key, char *out);

/* pb functions for TBL_FEA_QOS_DOMAIN */
int32
pb_tbl_fea_qos_domain_to_pb(uint32 only_key, tbl_fea_qos_domain_t *p_fea_qos_domain, Cdb__TblFeaQosDomain *p_pb_fea_qos_domain);
int32
pb_tbl_fea_qos_domain_to_pb_free_packed(Cdb__TblFeaQosDomain *p_pb_fea_qos_domain);
int32
pb_tbl_fea_qos_domain_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_qos_domain_dump(Cdb__TblFeaQosDomain *p_pb_fea_qos_domain, uint32 only_key, char *out);

/* pb functions for TBL_FEA_G8032_RING */
int32
pb_tbl_fea_g8032_ring_to_pb(uint32 only_key, tbl_fea_g8032_ring_t *p_fea_g8032_ring, Cdb__TblFeaG8032Ring *p_pb_fea_g8032_ring);
int32
pb_tbl_fea_g8032_ring_to_pb_free_packed(Cdb__TblFeaG8032Ring *p_pb_fea_g8032_ring);
int32
pb_tbl_fea_g8032_ring_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_g8032_ring_dump(Cdb__TblFeaG8032Ring *p_pb_fea_g8032_ring, uint32 only_key, char *out);

/* pb functions for TBL_FEA_G8032_RING_MC_GROUP */
int32
pb_tbl_fea_g8032_ring_mc_group_to_pb(uint32 only_key, tbl_fea_g8032_ring_mc_group_t *p_fea_g8032_ring_mc_group, Cdb__TblFeaG8032RingMcGroup *p_pb_fea_g8032_ring_mc_group);
int32
pb_tbl_fea_g8032_ring_mc_group_to_pb_free_packed(Cdb__TblFeaG8032RingMcGroup *p_pb_fea_g8032_ring_mc_group);
int32
pb_tbl_fea_g8032_ring_mc_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_g8032_ring_mc_group_dump(Cdb__TblFeaG8032RingMcGroup *p_pb_fea_g8032_ring_mc_group, uint32 only_key, char *out);

/* pb functions for TBL_FEA_QOS_QUEUE_SHAPE_PROFILE */
int32
pb_tbl_fea_qos_queue_shape_profile_to_pb(uint32 only_key, tbl_fea_qos_queue_shape_profile_t *p_fea_qos_queue_shape_profile, Cdb__TblFeaQosQueueShapeProfile *p_pb_fea_qos_queue_shape_profile);
int32
pb_tbl_fea_qos_queue_shape_profile_to_pb_free_packed(Cdb__TblFeaQosQueueShapeProfile *p_pb_fea_qos_queue_shape_profile);
int32
pb_tbl_fea_qos_queue_shape_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_qos_queue_shape_profile_dump(Cdb__TblFeaQosQueueShapeProfile *p_pb_fea_qos_queue_shape_profile, uint32 only_key, char *out);

/* pb functions for TBL_FEA_QOS_PORT_SHAPE_PROFILE */
int32
pb_tbl_fea_qos_port_shape_profile_to_pb(uint32 only_key, tbl_fea_qos_port_shape_profile_t *p_fea_qos_port_shape_profile, Cdb__TblFeaQosPortShapeProfile *p_pb_fea_qos_port_shape_profile);
int32
pb_tbl_fea_qos_port_shape_profile_to_pb_free_packed(Cdb__TblFeaQosPortShapeProfile *p_pb_fea_qos_port_shape_profile);
int32
pb_tbl_fea_qos_port_shape_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_qos_port_shape_profile_dump(Cdb__TblFeaQosPortShapeProfile *p_pb_fea_qos_port_shape_profile, uint32 only_key, char *out);

/* pb functions for TBL_FEA_PORT_POLICER_APPLY */
int32
pb_tbl_fea_port_policer_apply_to_pb(uint32 only_key, tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, Cdb__TblFeaPortPolicerApply *p_pb_fea_port_policer_apply);
int32
pb_tbl_fea_port_policer_apply_to_pb_free_packed(Cdb__TblFeaPortPolicerApply *p_pb_fea_port_policer_apply);
int32
pb_tbl_fea_port_policer_apply_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_port_policer_apply_dump(Cdb__TblFeaPortPolicerApply *p_pb_fea_port_policer_apply, uint32 only_key, char *out);

/* pb functions for TBL_ACLQOS_IF_STATS */
int32
pb_tbl_aclqos_if_stats_to_pb(uint32 only_key, tbl_aclqos_if_stats_t *p_aclqos_if_stats, Cdb__TblAclqosIfStats *p_pb_aclqos_if_stats);
int32
pb_tbl_aclqos_if_stats_to_pb_free_packed(Cdb__TblAclqosIfStats *p_pb_aclqos_if_stats);
int32
pb_tbl_aclqos_if_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_aclqos_if_stats_dump(Cdb__TblAclqosIfStats *p_pb_aclqos_if_stats, uint32 only_key, char *out);

/* pb functions for TBL_VERSION */
int32
pb_tbl_version_to_pb(uint32 only_key, tbl_version_t *p_ver, Cdb__TblVersion *p_pb_ver);
int32
pb_tbl_version_to_pb_free_packed(Cdb__TblVersion *p_pb_ver);
int32
pb_tbl_version_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_version_dump(Cdb__TblVersion *p_pb_ver, uint32 only_key, char *out);

/* pb functions for TBL_MANAGE_IF */
int32
pb_tbl_manage_if_to_pb(uint32 only_key, tbl_manage_if_t *p_mng_if, Cdb__TblManageIf *p_pb_mng_if);
int32
pb_tbl_manage_if_to_pb_free_packed(Cdb__TblManageIf *p_pb_mng_if);
int32
pb_tbl_manage_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_manage_if_dump(Cdb__TblManageIf *p_pb_mng_if, uint32 only_key, char *out);

/* pb functions for TBL_BOOTIMAGE */
int32
pb_tbl_bootimage_to_pb(uint32 only_key, tbl_bootimage_t *p_boot, Cdb__TblBootimage *p_pb_boot);
int32
pb_tbl_bootimage_to_pb_free_packed(Cdb__TblBootimage *p_pb_boot);
int32
pb_tbl_bootimage_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bootimage_dump(Cdb__TblBootimage *p_pb_boot, uint32 only_key, char *out);

/* pb functions for TBL_CHASSIS */
int32
pb_tbl_chassis_to_pb(uint32 only_key, tbl_chassis_t *p_chassis, Cdb__TblChassis *p_pb_chassis);
int32
pb_tbl_chassis_to_pb_free_packed(Cdb__TblChassis *p_pb_chassis);
int32
pb_tbl_chassis_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_chassis_dump(Cdb__TblChassis *p_pb_chassis, uint32 only_key, char *out);

/* pb functions for TBL_IFNAME_INFO */
int32
pb_tbl_ifname_info_to_pb(uint32 only_key, tbl_ifname_info_t *p_ifname_info, Cdb__TblIfnameInfo *p_pb_ifname_info);
int32
pb_tbl_ifname_info_to_pb_free_packed(Cdb__TblIfnameInfo *p_pb_ifname_info);
int32
pb_tbl_ifname_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ifname_info_dump(Cdb__TblIfnameInfo *p_pb_ifname_info, uint32 only_key, char *out);

/* pb functions for TBL_CARD */
int32
pb_tbl_card_to_pb(uint32 only_key, tbl_card_t *p_card, Cdb__TblCard *p_pb_card);
int32
pb_tbl_card_to_pb_free_packed(Cdb__TblCard *p_pb_card);
int32
pb_tbl_card_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_card_dump(Cdb__TblCard *p_pb_card, uint32 only_key, char *out);

/* pb functions for TBL_PORT */
int32
pb_tbl_port_to_pb(uint32 only_key, tbl_port_t *p_port, Cdb__TblPort *p_pb_port);
int32
pb_tbl_port_to_pb_free_packed(Cdb__TblPort *p_pb_port);
int32
pb_tbl_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_port_dump(Cdb__TblPort *p_pb_port, uint32 only_key, char *out);

/* pb functions for TBL_FIBER */
int32
pb_tbl_fiber_to_pb(uint32 only_key, tbl_fiber_t *p_fiber, Cdb__TblFiber *p_pb_fiber);
int32
pb_tbl_fiber_to_pb_free_packed(Cdb__TblFiber *p_pb_fiber);
int32
pb_tbl_fiber_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fiber_dump(Cdb__TblFiber *p_pb_fiber, uint32 only_key, char *out);

/* pb functions for TBL_SYS_SPEC */
int32
pb_tbl_sys_spec_to_pb(uint32 only_key, tbl_sys_spec_t *p_sys_spec, Cdb__TblSysSpec *p_pb_sys_spec);
int32
pb_tbl_sys_spec_to_pb_free_packed(Cdb__TblSysSpec *p_pb_sys_spec);
int32
pb_tbl_sys_spec_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sys_spec_dump(Cdb__TblSysSpec *p_pb_sys_spec, uint32 only_key, char *out);

/* pb functions for TBL_FAN */
int32
pb_tbl_fan_to_pb(uint32 only_key, tbl_fan_t *p_fan, Cdb__TblFan *p_pb_fan);
int32
pb_tbl_fan_to_pb_free_packed(Cdb__TblFan *p_pb_fan);
int32
pb_tbl_fan_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fan_dump(Cdb__TblFan *p_pb_fan, uint32 only_key, char *out);

/* pb functions for TBL_PSU */
int32
pb_tbl_psu_to_pb(uint32 only_key, tbl_psu_t *p_psu, Cdb__TblPsu *p_pb_psu);
int32
pb_tbl_psu_to_pb_free_packed(Cdb__TblPsu *p_pb_psu);
int32
pb_tbl_psu_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_psu_dump(Cdb__TblPsu *p_pb_psu, uint32 only_key, char *out);

/* pb functions for TBL_LED */
int32
pb_tbl_led_to_pb(uint32 only_key, tbl_led_t *p_led, Cdb__TblLed *p_pb_led);
int32
pb_tbl_led_to_pb_free_packed(Cdb__TblLed *p_pb_led);
int32
pb_tbl_led_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_led_dump(Cdb__TblLed *p_pb_led, uint32 only_key, char *out);

/* pb functions for TBL_SENSOR */
int32
pb_tbl_sensor_to_pb(uint32 only_key, tbl_sensor_t *p_sensor, Cdb__TblSensor *p_pb_sensor);
int32
pb_tbl_sensor_to_pb_free_packed(Cdb__TblSensor *p_pb_sensor);
int32
pb_tbl_sensor_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sensor_dump(Cdb__TblSensor *p_pb_sensor, uint32 only_key, char *out);

/* pb functions for TBL_VCM */
int32
pb_tbl_vcm_to_pb(uint32 only_key, tbl_vcm_t *p_vcm, Cdb__TblVcm *p_pb_vcm);
int32
pb_tbl_vcm_to_pb_free_packed(Cdb__TblVcm *p_pb_vcm);
int32
pb_tbl_vcm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vcm_dump(Cdb__TblVcm *p_pb_vcm, uint32 only_key, char *out);

/* pb functions for TBL_POE */
int32
pb_tbl_poe_to_pb(uint32 only_key, tbl_poe_t *p_poe, Cdb__TblPoe *p_pb_poe);
int32
pb_tbl_poe_to_pb_free_packed(Cdb__TblPoe *p_pb_poe);
int32
pb_tbl_poe_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_poe_dump(Cdb__TblPoe *p_pb_poe, uint32 only_key, char *out);

/* pb functions for TBL_REBOOT_INFO */
int32
pb_tbl_reboot_info_to_pb(uint32 only_key, tbl_reboot_info_t *p_reboot_info, Cdb__TblRebootInfo *p_pb_reboot_info);
int32
pb_tbl_reboot_info_to_pb_free_packed(Cdb__TblRebootInfo *p_pb_reboot_info);
int32
pb_tbl_reboot_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_reboot_info_dump(Cdb__TblRebootInfo *p_pb_reboot_info, uint32 only_key, char *out);

/* pb functions for TBL_ERRDISABLE_FLAP */
int32
pb_tbl_errdisable_flap_to_pb(uint32 only_key, tbl_errdisable_flap_t *p_errdisable_flap, Cdb__TblErrdisableFlap *p_pb_errdisable_flap);
int32
pb_tbl_errdisable_flap_to_pb_free_packed(Cdb__TblErrdisableFlap *p_pb_errdisable_flap);
int32
pb_tbl_errdisable_flap_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_errdisable_flap_dump(Cdb__TblErrdisableFlap *p_pb_errdisable_flap, uint32 only_key, char *out);

/* pb functions for TBL_OPM_GLOBAL */
int32
pb_tbl_opm_global_to_pb(uint32 only_key, tbl_opm_global_t *p_opmglb, Cdb__TblOpmGlobal *p_pb_opmglb);
int32
pb_tbl_opm_global_to_pb_free_packed(Cdb__TblOpmGlobal *p_pb_opmglb);
int32
pb_tbl_opm_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_opm_global_dump(Cdb__TblOpmGlobal *p_pb_opmglb, uint32 only_key, char *out);

/* pb functions for TBL_ERPS_RING */
int32
pb_tbl_erps_ring_to_pb(uint32 only_key, tbl_erps_ring_t *p_erps_ring, Cdb__TblErpsRing *p_pb_erps_ring);
int32
pb_tbl_erps_ring_to_pb_free_packed(Cdb__TblErpsRing *p_pb_erps_ring);
int32
pb_tbl_erps_ring_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_erps_ring_dump(Cdb__TblErpsRing *p_pb_erps_ring, uint32 only_key, char *out);

/* pb functions for TBL_ERPS_DOMAIN */
int32
pb_tbl_erps_domain_to_pb(uint32 only_key, tbl_erps_domain_t *p_erps_domain, Cdb__TblErpsDomain *p_pb_erps_domain);
int32
pb_tbl_erps_domain_to_pb_free_packed(Cdb__TblErpsDomain *p_pb_erps_domain);
int32
pb_tbl_erps_domain_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_erps_domain_dump(Cdb__TblErpsDomain *p_pb_erps_domain, uint32 only_key, char *out);

/* pb functions for TBL_OPM_DEBUG */
int32
pb_tbl_opm_debug_to_pb(uint32 only_key, tbl_opm_debug_t *p_opmdbg, Cdb__TblOpmDebug *p_pb_opmdbg);
int32
pb_tbl_opm_debug_to_pb_free_packed(Cdb__TblOpmDebug *p_pb_opmdbg);
int32
pb_tbl_opm_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_opm_debug_dump(Cdb__TblOpmDebug *p_pb_opmdbg, uint32 only_key, char *out);

/* pb functions for TBL_POLICY_MAP_CONFIG */
int32
pb_tbl_policy_map_config_to_pb(uint32 only_key, tbl_policy_map_config_t *p_policy_map_config, Cdb__TblPolicyMapConfig *p_pb_policy_map_config);
int32
pb_tbl_policy_map_config_to_pb_free_packed(Cdb__TblPolicyMapConfig *p_pb_policy_map_config);
int32
pb_tbl_policy_map_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_policy_map_config_dump(Cdb__TblPolicyMapConfig *p_pb_policy_map_config, uint32 only_key, char *out);

/* pb functions for TBL_CLASS_MAP_CONFIG */
int32
pb_tbl_class_map_config_to_pb(uint32 only_key, tbl_class_map_config_t *p_class_map_config, Cdb__TblClassMapConfig *p_pb_class_map_config);
int32
pb_tbl_class_map_config_to_pb_free_packed(Cdb__TblClassMapConfig *p_pb_class_map_config);
int32
pb_tbl_class_map_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_class_map_config_dump(Cdb__TblClassMapConfig *p_pb_class_map_config, uint32 only_key, char *out);

/* pb functions for TBL_CLASS_IN_POLICY_CONFIG */
int32
pb_tbl_class_in_policy_config_to_pb(uint32 only_key, tbl_class_in_policy_config_t *p_class_in_policy_config, Cdb__TblClassInPolicyConfig *p_pb_class_in_policy_config);
int32
pb_tbl_class_in_policy_config_to_pb_free_packed(Cdb__TblClassInPolicyConfig *p_pb_class_in_policy_config);
int32
pb_tbl_class_in_policy_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_class_in_policy_config_dump(Cdb__TblClassInPolicyConfig *p_pb_class_in_policy_config, uint32 only_key, char *out);

/* pb functions for TBL_ACL_IN_CLASS_CONFIG */
int32
pb_tbl_acl_in_class_config_to_pb(uint32 only_key, tbl_acl_in_class_config_t *p_acl_in_class_config, Cdb__TblAclInClassConfig *p_pb_acl_in_class_config);
int32
pb_tbl_acl_in_class_config_to_pb_free_packed(Cdb__TblAclInClassConfig *p_pb_acl_in_class_config);
int32
pb_tbl_acl_in_class_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_in_class_config_dump(Cdb__TblAclInClassConfig *p_pb_acl_in_class_config, uint32 only_key, char *out);

/* pb functions for TBL_CLASS_MAP_ACTION_CONFIG */
int32
pb_tbl_class_map_action_config_to_pb(uint32 only_key, tbl_class_map_action_config_t *p_class_map_action_config, Cdb__TblClassMapActionConfig *p_pb_class_map_action_config);
int32
pb_tbl_class_map_action_config_to_pb_free_packed(Cdb__TblClassMapActionConfig *p_pb_class_map_action_config);
int32
pb_tbl_class_map_action_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_class_map_action_config_dump(Cdb__TblClassMapActionConfig *p_pb_class_map_action_config, uint32 only_key, char *out);

/* pb functions for TBL_FEA_ACL_POLICY_ACTION */
int32
pb_tbl_fea_acl_policy_action_to_pb(uint32 only_key, tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, Cdb__TblFeaAclPolicyAction *p_pb_fea_acl_policy_action);
int32
pb_tbl_fea_acl_policy_action_to_pb_free_packed(Cdb__TblFeaAclPolicyAction *p_pb_fea_acl_policy_action);
int32
pb_tbl_fea_acl_policy_action_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_acl_policy_action_dump(Cdb__TblFeaAclPolicyAction *p_pb_fea_acl_policy_action, uint32 only_key, char *out);

/* pb functions for TBL_IGSP_GLOBAL */
int32
pb_tbl_igsp_global_to_pb(uint32 only_key, tbl_igsp_global_t *p_glb, Cdb__TblIgspGlobal *p_pb_glb);
int32
pb_tbl_igsp_global_to_pb_free_packed(Cdb__TblIgspGlobal *p_pb_glb);
int32
pb_tbl_igsp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_igsp_global_dump(Cdb__TblIgspGlobal *p_pb_glb, uint32 only_key, char *out);

/* pb functions for TBL_IGSP_INTF */
int32
pb_tbl_igsp_intf_to_pb(uint32 only_key, tbl_igsp_intf_t *p_if, Cdb__TblIgspIntf *p_pb_if);
int32
pb_tbl_igsp_intf_to_pb_free_packed(Cdb__TblIgspIntf *p_pb_if);
int32
pb_tbl_igsp_intf_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_igsp_intf_dump(Cdb__TblIgspIntf *p_pb_if, uint32 only_key, char *out);

/* pb functions for TBL_IGSP_GROUP */
int32
pb_tbl_igsp_group_to_pb(uint32 only_key, tbl_igsp_group_t *p_group, Cdb__TblIgspGroup *p_pb_group);
int32
pb_tbl_igsp_group_to_pb_free_packed(Cdb__TblIgspGroup *p_pb_group);
int32
pb_tbl_igsp_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_igsp_group_dump(Cdb__TblIgspGroup *p_pb_group, uint32 only_key, char *out);

/* pb functions for TBL_AUTH_CFG */
int32
pb_tbl_auth_cfg_to_pb(uint32 only_key, tbl_auth_cfg_t *p_auth_cfg, Cdb__TblAuthCfg *p_pb_auth_cfg);
int32
pb_tbl_auth_cfg_to_pb_free_packed(Cdb__TblAuthCfg *p_pb_auth_cfg);
int32
pb_tbl_auth_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_auth_cfg_dump(Cdb__TblAuthCfg *p_pb_auth_cfg, uint32 only_key, char *out);

/* pb functions for TBL_AUTH_SERVER */
int32
pb_tbl_auth_server_to_pb(uint32 only_key, tbl_auth_server_t *p_auth_server, Cdb__TblAuthServer *p_pb_auth_server);
int32
pb_tbl_auth_server_to_pb_free_packed(Cdb__TblAuthServer *p_pb_auth_server);
int32
pb_tbl_auth_server_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_auth_server_dump(Cdb__TblAuthServer *p_pb_auth_server, uint32 only_key, char *out);

/* pb functions for TBL_AUTH_SESSION */
int32
pb_tbl_auth_session_to_pb(uint32 only_key, tbl_auth_session_t *p_auth_session, Cdb__TblAuthSession *p_pb_auth_session);
int32
pb_tbl_auth_session_to_pb_free_packed(Cdb__TblAuthSession *p_pb_auth_session);
int32
pb_tbl_auth_session_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_auth_session_dump(Cdb__TblAuthSession *p_pb_auth_session, uint32 only_key, char *out);

/* pb functions for TBL_AUTHD_DEBUG */
int32
pb_tbl_authd_debug_to_pb(uint32 only_key, tbl_authd_debug_t *p_authd_debug, Cdb__TblAuthdDebug *p_pb_authd_debug);
int32
pb_tbl_authd_debug_to_pb_free_packed(Cdb__TblAuthdDebug *p_pb_authd_debug);
int32
pb_tbl_authd_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_authd_debug_dump(Cdb__TblAuthdDebug *p_pb_authd_debug, uint32 only_key, char *out);

/* pb functions for TBL_DOT1X_GLOBAL */
int32
pb_tbl_dot1x_global_to_pb(uint32 only_key, tbl_dot1x_global_t *p_dot1x_global, Cdb__TblDot1xGlobal *p_pb_dot1x_global);
int32
pb_tbl_dot1x_global_to_pb_free_packed(Cdb__TblDot1xGlobal *p_pb_dot1x_global);
int32
pb_tbl_dot1x_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dot1x_global_dump(Cdb__TblDot1xGlobal *p_pb_dot1x_global, uint32 only_key, char *out);

/* pb functions for TBL_DOT1X_PORT */
int32
pb_tbl_dot1x_port_to_pb(uint32 only_key, tbl_dot1x_port_t *p_dot1x_port, Cdb__TblDot1xPort *p_pb_dot1x_port);
int32
pb_tbl_dot1x_port_to_pb_free_packed(Cdb__TblDot1xPort *p_pb_dot1x_port);
int32
pb_tbl_dot1x_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dot1x_port_dump(Cdb__TblDot1xPort *p_pb_dot1x_port, uint32 only_key, char *out);

/* pb functions for TBL_DOT1X_RADIUS */
int32
pb_tbl_dot1x_radius_to_pb(uint32 only_key, tbl_dot1x_radius_t *p_dot1x_radius, Cdb__TblDot1xRadius *p_pb_dot1x_radius);
int32
pb_tbl_dot1x_radius_to_pb_free_packed(Cdb__TblDot1xRadius *p_pb_dot1x_radius);
int32
pb_tbl_dot1x_radius_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dot1x_radius_dump(Cdb__TblDot1xRadius *p_pb_dot1x_radius, uint32 only_key, char *out);

/* pb functions for TBL_DOT1X_MAC */
int32
pb_tbl_dot1x_mac_to_pb(uint32 only_key, tbl_dot1x_mac_t *p_dot1x_mac, Cdb__TblDot1xMac *p_pb_dot1x_mac);
int32
pb_tbl_dot1x_mac_to_pb_free_packed(Cdb__TblDot1xMac *p_pb_dot1x_mac);
int32
pb_tbl_dot1x_mac_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dot1x_mac_dump(Cdb__TblDot1xMac *p_pb_dot1x_mac, uint32 only_key, char *out);

/* pb functions for TBL_ENABLE */
int32
pb_tbl_enable_to_pb(uint32 only_key, tbl_enable_t *p_enable, Cdb__TblEnable *p_pb_enable);
int32
pb_tbl_enable_to_pb_free_packed(Cdb__TblEnable *p_pb_enable);
int32
pb_tbl_enable_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_enable_dump(Cdb__TblEnable *p_pb_enable, uint32 only_key, char *out);

/* pb functions for TBL_CHIP */
int32
pb_tbl_chip_to_pb(uint32 only_key, tbl_chip_t *p_chip, Cdb__TblChip *p_pb_chip);
int32
pb_tbl_chip_to_pb_free_packed(Cdb__TblChip *p_pb_chip);
int32
pb_tbl_chip_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_chip_dump(Cdb__TblChip *p_pb_chip, uint32 only_key, char *out);

/* pb functions for TBL_CLEAR_ACL_POLICY */
int32
pb_tbl_clear_acl_policy_to_pb(uint32 only_key, tbl_clear_acl_policy_t *p_clear_acl_policy, Cdb__TblClearAclPolicy *p_pb_clear_acl_policy);
int32
pb_tbl_clear_acl_policy_to_pb_free_packed(Cdb__TblClearAclPolicy *p_pb_clear_acl_policy);
int32
pb_tbl_clear_acl_policy_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_clear_acl_policy_dump(Cdb__TblClearAclPolicy *p_pb_clear_acl_policy, uint32 only_key, char *out);

/* pb functions for TBL_AUTHOR */
int32
pb_tbl_author_to_pb(uint32 only_key, tbl_author_t *p_author, Cdb__TblAuthor *p_pb_author);
int32
pb_tbl_author_to_pb_free_packed(Cdb__TblAuthor *p_pb_author);
int32
pb_tbl_author_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_author_dump(Cdb__TblAuthor *p_pb_author, uint32 only_key, char *out);

/* pb functions for TBL_ACCOUNT */
int32
pb_tbl_account_to_pb(uint32 only_key, tbl_account_t *p_account, Cdb__TblAccount *p_pb_account);
int32
pb_tbl_account_to_pb_free_packed(Cdb__TblAccount *p_pb_account);
int32
pb_tbl_account_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_account_dump(Cdb__TblAccount *p_pb_account, uint32 only_key, char *out);

/* pb functions for TBL_ACCOUNTCMD */
int32
pb_tbl_accountcmd_to_pb(uint32 only_key, tbl_accountcmd_t *p_accountcmd, Cdb__TblAccountcmd *p_pb_accountcmd);
int32
pb_tbl_accountcmd_to_pb_free_packed(Cdb__TblAccountcmd *p_pb_accountcmd);
int32
pb_tbl_accountcmd_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_accountcmd_dump(Cdb__TblAccountcmd *p_pb_accountcmd, uint32 only_key, char *out);

/* pb functions for TBL_VLANCLASS_RULE */
int32
pb_tbl_vlanclass_rule_to_pb(uint32 only_key, tbl_vlanclass_rule_t *p_vlanclass_rule, Cdb__TblVlanclassRule *p_pb_vlanclass_rule);
int32
pb_tbl_vlanclass_rule_to_pb_free_packed(Cdb__TblVlanclassRule *p_pb_vlanclass_rule);
int32
pb_tbl_vlanclass_rule_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlanclass_rule_dump(Cdb__TblVlanclassRule *p_pb_vlanclass_rule, uint32 only_key, char *out);

/* pb functions for TBL_VLANCLASS_GROUP */
int32
pb_tbl_vlanclass_group_to_pb(uint32 only_key, tbl_vlanclass_group_t *p_vlanclass_group, Cdb__TblVlanclassGroup *p_pb_vlanclass_group);
int32
pb_tbl_vlanclass_group_to_pb_free_packed(Cdb__TblVlanclassGroup *p_pb_vlanclass_group);
int32
pb_tbl_vlanclass_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlanclass_group_dump(Cdb__TblVlanclassGroup *p_pb_vlanclass_group, uint32 only_key, char *out);

/* pb functions for TBL_ACL_L4_PORT_RANGE */
int32
pb_tbl_acl_l4_port_range_to_pb(uint32 only_key, tbl_acl_l4_port_range_t *p_acl_l4_port_range, Cdb__TblAclL4PortRange *p_pb_acl_l4_port_range);
int32
pb_tbl_acl_l4_port_range_to_pb_free_packed(Cdb__TblAclL4PortRange *p_pb_acl_l4_port_range);
int32
pb_tbl_acl_l4_port_range_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_l4_port_range_dump(Cdb__TblAclL4PortRange *p_pb_acl_l4_port_range, uint32 only_key, char *out);

/* pb functions for TBL_ACL_UDF_GROUP */
int32
pb_tbl_acl_udf_group_to_pb(uint32 only_key, tbl_acl_udf_group_t *p_acl_udf_group, Cdb__TblAclUdfGroup *p_pb_acl_udf_group);
int32
pb_tbl_acl_udf_group_to_pb_free_packed(Cdb__TblAclUdfGroup *p_pb_acl_udf_group);
int32
pb_tbl_acl_udf_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_udf_group_dump(Cdb__TblAclUdfGroup *p_pb_acl_udf_group, uint32 only_key, char *out);

/* pb functions for TBL_ACL_UDF_ENTRY */
int32
pb_tbl_acl_udf_entry_to_pb(uint32 only_key, tbl_acl_udf_entry_t *p_acl_udf_entry, Cdb__TblAclUdfEntry *p_pb_acl_udf_entry);
int32
pb_tbl_acl_udf_entry_to_pb_free_packed(Cdb__TblAclUdfEntry *p_pb_acl_udf_entry);
int32
pb_tbl_acl_udf_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_acl_udf_entry_dump(Cdb__TblAclUdfEntry *p_pb_acl_udf_entry, uint32 only_key, char *out);

/* pb functions for TBL_FEA_PCAP */
int32
pb_tbl_fea_pcap_to_pb(uint32 only_key, tbl_fea_pcap_t *p_pcap, Cdb__TblFeaPcap *p_pb_pcap);
int32
pb_tbl_fea_pcap_to_pb_free_packed(Cdb__TblFeaPcap *p_pb_pcap);
int32
pb_tbl_fea_pcap_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_pcap_dump(Cdb__TblFeaPcap *p_pb_pcap, uint32 only_key, char *out);

/* pb functions for TBL_CONTROLLER */
int32
pb_tbl_controller_to_pb(uint32 only_key, tbl_controller_t *p_controller, Cdb__TblController *p_pb_controller);
int32
pb_tbl_controller_to_pb_free_packed(Cdb__TblController *p_pb_controller);
int32
pb_tbl_controller_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_controller_dump(Cdb__TblController *p_pb_controller, uint32 only_key, char *out);

/* pb functions for TBL_CPU_PACKETS */
int32
pb_tbl_cpu_packets_to_pb(uint32 only_key, tbl_cpu_packets_t *p_cpu_packets, Cdb__TblCpuPackets *p_pb_cpu_packets);
int32
pb_tbl_cpu_packets_to_pb_free_packed(Cdb__TblCpuPackets *p_pb_cpu_packets);
int32
pb_tbl_cpu_packets_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_packets_dump(Cdb__TblCpuPackets *p_pb_cpu_packets, uint32 only_key, char *out);

/* pb functions for TBL_NS_ROUTE */
int32
pb_tbl_ns_route_to_pb(uint32 only_key, tbl_ns_route_t *p_ns_route, Cdb__TblNsRoute *p_pb_ns_route);
int32
pb_tbl_ns_route_to_pb_free_packed(Cdb__TblNsRoute *p_pb_ns_route);
int32
pb_tbl_ns_route_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ns_route_dump(Cdb__TblNsRoute *p_pb_ns_route, uint32 only_key, char *out);

/* pb functions for TBL_NS_ROUTE_IP */
int32
pb_tbl_ns_route_ip_to_pb(uint32 only_key, tbl_ns_route_ip_t *p_ns_route_ip, Cdb__TblNsRouteIp *p_pb_ns_route_ip);
int32
pb_tbl_ns_route_ip_to_pb_free_packed(Cdb__TblNsRouteIp *p_pb_ns_route_ip);
int32
pb_tbl_ns_route_ip_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ns_route_ip_dump(Cdb__TblNsRouteIp *p_pb_ns_route_ip, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW_INTERFACE */
int32
pb_tbl_openflow_interface_to_pb(uint32 only_key, tbl_openflow_interface_t *p_if, Cdb__TblOpenflowInterface *p_pb_if);
int32
pb_tbl_openflow_interface_to_pb_free_packed(Cdb__TblOpenflowInterface *p_pb_if);
int32
pb_tbl_openflow_interface_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_interface_dump(Cdb__TblOpenflowInterface *p_pb_if, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW_MANAGER */
int32
pb_tbl_openflow_manager_to_pb(uint32 only_key, tbl_openflow_manager_t *p_openflow_manager, Cdb__TblOpenflowManager *p_pb_openflow_manager);
int32
pb_tbl_openflow_manager_to_pb_free_packed(Cdb__TblOpenflowManager *p_pb_openflow_manager);
int32
pb_tbl_openflow_manager_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_manager_dump(Cdb__TblOpenflowManager *p_pb_openflow_manager, uint32 only_key, char *out);

/* pb functions for TBL_PTP_GLOBAL */
int32
pb_tbl_ptp_global_to_pb(uint32 only_key, tbl_ptp_global_t *p_global, Cdb__TblPtpGlobal *p_pb_global);
int32
pb_tbl_ptp_global_to_pb_free_packed(Cdb__TblPtpGlobal *p_pb_global);
int32
pb_tbl_ptp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ptp_global_dump(Cdb__TblPtpGlobal *p_pb_global, uint32 only_key, char *out);

/* pb functions for TBL_PTP_PORT */
int32
pb_tbl_ptp_port_to_pb(uint32 only_key, tbl_ptp_port_t *p_port, Cdb__TblPtpPort *p_pb_port);
int32
pb_tbl_ptp_port_to_pb_free_packed(Cdb__TblPtpPort *p_pb_port);
int32
pb_tbl_ptp_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ptp_port_dump(Cdb__TblPtpPort *p_pb_port, uint32 only_key, char *out);

/* pb functions for TBL_PTP_FOREIGN */
int32
pb_tbl_ptp_foreign_to_pb(uint32 only_key, tbl_ptp_foreign_t *p_foreign, Cdb__TblPtpForeign *p_pb_foreign);
int32
pb_tbl_ptp_foreign_to_pb_free_packed(Cdb__TblPtpForeign *p_pb_foreign);
int32
pb_tbl_ptp_foreign_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ptp_foreign_dump(Cdb__TblPtpForeign *p_pb_foreign, uint32 only_key, char *out);

/* pb functions for TBL_PTP_RESIDENCE_TIME_CACHE */
int32
pb_tbl_ptp_residence_time_cache_to_pb(uint32 only_key, tbl_ptp_residence_time_cache_t *p_residence_time_cache, Cdb__TblPtpResidenceTimeCache *p_pb_residence_time_cache);
int32
pb_tbl_ptp_residence_time_cache_to_pb_free_packed(Cdb__TblPtpResidenceTimeCache *p_pb_residence_time_cache);
int32
pb_tbl_ptp_residence_time_cache_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ptp_residence_time_cache_dump(Cdb__TblPtpResidenceTimeCache *p_pb_residence_time_cache, uint32 only_key, char *out);

/* pb functions for TBL_FEA_PTP_GLOBAL */
int32
pb_tbl_fea_ptp_global_to_pb(uint32 only_key, tbl_fea_ptp_global_t *p_fea_ptp_global, Cdb__TblFeaPtpGlobal *p_pb_fea_ptp_global);
int32
pb_tbl_fea_ptp_global_to_pb_free_packed(Cdb__TblFeaPtpGlobal *p_pb_fea_ptp_global);
int32
pb_tbl_fea_ptp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_ptp_global_dump(Cdb__TblFeaPtpGlobal *p_pb_fea_ptp_global, uint32 only_key, char *out);

/* pb functions for TBL_FEA_PTP_TS */
int32
pb_tbl_fea_ptp_ts_to_pb(uint32 only_key, tbl_fea_ptp_ts_t *p_fea_ptp_ts, Cdb__TblFeaPtpTs *p_pb_fea_ptp_ts);
int32
pb_tbl_fea_ptp_ts_to_pb_free_packed(Cdb__TblFeaPtpTs *p_pb_fea_ptp_ts);
int32
pb_tbl_fea_ptp_ts_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_ptp_ts_dump(Cdb__TblFeaPtpTs *p_pb_fea_ptp_ts, uint32 only_key, char *out);

/* pb functions for TBL_FEA_TIME */
int32
pb_tbl_fea_time_to_pb(uint32 only_key, tbl_fea_time_t *p_fea_time, Cdb__TblFeaTime *p_pb_fea_time);
int32
pb_tbl_fea_time_to_pb_free_packed(Cdb__TblFeaTime *p_pb_fea_time);
int32
pb_tbl_fea_time_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_time_dump(Cdb__TblFeaTime *p_pb_fea_time, uint32 only_key, char *out);

/* pb functions for TBL_BHM_GLOBAL */
int32
pb_tbl_bhm_global_to_pb(uint32 only_key, tbl_bhm_global_t *p_bhm_glb, Cdb__TblBhmGlobal *p_pb_bhm_glb);
int32
pb_tbl_bhm_global_to_pb_free_packed(Cdb__TblBhmGlobal *p_pb_bhm_glb);
int32
pb_tbl_bhm_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bhm_global_dump(Cdb__TblBhmGlobal *p_pb_bhm_glb, uint32 only_key, char *out);

/* pb functions for TBL_BHM_MODULE */
int32
pb_tbl_bhm_module_to_pb(uint32 only_key, tbl_bhm_module_t *p_bhm_module, Cdb__TblBhmModule *p_pb_bhm_module);
int32
pb_tbl_bhm_module_to_pb_free_packed(Cdb__TblBhmModule *p_pb_bhm_module);
int32
pb_tbl_bhm_module_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_bhm_module_dump(Cdb__TblBhmModule *p_pb_bhm_module, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW_TUNNEL_INTERFACE */
int32
pb_tbl_openflow_tunnel_interface_to_pb(uint32 only_key, tbl_openflow_tunnel_interface_t *p_tunnel_if, Cdb__TblOpenflowTunnelInterface *p_pb_tunnel_if);
int32
pb_tbl_openflow_tunnel_interface_to_pb_free_packed(Cdb__TblOpenflowTunnelInterface *p_pb_tunnel_if);
int32
pb_tbl_openflow_tunnel_interface_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_tunnel_interface_dump(Cdb__TblOpenflowTunnelInterface *p_pb_tunnel_if, uint32 only_key, char *out);

/* pb functions for TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT */
int32
pb_tbl_openflow_tunnel_local_ip_cnt_to_pb(uint32 only_key, tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, Cdb__TblOpenflowTunnelLocalIpCnt *p_pb_openflow_tunnel_local_ip);
int32
pb_tbl_openflow_tunnel_local_ip_cnt_to_pb_free_packed(Cdb__TblOpenflowTunnelLocalIpCnt *p_pb_openflow_tunnel_local_ip);
int32
pb_tbl_openflow_tunnel_local_ip_cnt_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_openflow_tunnel_local_ip_cnt_dump(Cdb__TblOpenflowTunnelLocalIpCnt *p_pb_openflow_tunnel_local_ip, uint32 only_key, char *out);

/* pb functions for TBL_INBAND_SNAT */
int32
pb_tbl_inband_snat_to_pb(uint32 only_key, tbl_inband_snat_t *p_inband_snat, Cdb__TblInbandSnat *p_pb_inband_snat);
int32
pb_tbl_inband_snat_to_pb_free_packed(Cdb__TblInbandSnat *p_pb_inband_snat);
int32
pb_tbl_inband_snat_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_inband_snat_dump(Cdb__TblInbandSnat *p_pb_inband_snat, uint32 only_key, char *out);

/* pb functions for TBL_ARPINSP */
int32
pb_tbl_arpinsp_to_pb(uint32 only_key, tbl_arpinsp_t *p_arpinsp, Cdb__TblArpinsp *p_pb_arpinsp);
int32
pb_tbl_arpinsp_to_pb_free_packed(Cdb__TblArpinsp *p_pb_arpinsp);
int32
pb_tbl_arpinsp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_arpinsp_dump(Cdb__TblArpinsp *p_pb_arpinsp, uint32 only_key, char *out);

/* pb functions for TBL_ARPACL_CONFIG */
int32
pb_tbl_arpacl_config_to_pb(uint32 only_key, tbl_arpacl_config_t *p_arpacl_config, Cdb__TblArpaclConfig *p_pb_arpacl_config);
int32
pb_tbl_arpacl_config_to_pb_free_packed(Cdb__TblArpaclConfig *p_pb_arpacl_config);
int32
pb_tbl_arpacl_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_arpacl_config_dump(Cdb__TblArpaclConfig *p_pb_arpacl_config, uint32 only_key, char *out);

/* pb functions for TBL_ARPACE_CONFIG */
int32
pb_tbl_arpace_config_to_pb(uint32 only_key, tbl_arpace_config_t *p_arpace_config, Cdb__TblArpaceConfig *p_pb_arpace_config);
int32
pb_tbl_arpace_config_to_pb_free_packed(Cdb__TblArpaceConfig *p_pb_arpace_config);
int32
pb_tbl_arpace_config_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_arpace_config_dump(Cdb__TblArpaceConfig *p_pb_arpace_config, uint32 only_key, char *out);

/* pb functions for TBL_COPP_CFG */
int32
pb_tbl_copp_cfg_to_pb(uint32 only_key, tbl_copp_cfg_t *p_copp_cfg, Cdb__TblCoppCfg *p_pb_copp_cfg);
int32
pb_tbl_copp_cfg_to_pb_free_packed(Cdb__TblCoppCfg *p_pb_copp_cfg);
int32
pb_tbl_copp_cfg_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_copp_cfg_dump(Cdb__TblCoppCfg *p_pb_copp_cfg, uint32 only_key, char *out);

/* pb functions for TBL_FEA_COPP_TRAP_GROUP */
int32
pb_tbl_fea_copp_trap_group_to_pb(uint32 only_key, tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, Cdb__TblFeaCoppTrapGroup *p_pb_fea_copp_trap_group);
int32
pb_tbl_fea_copp_trap_group_to_pb_free_packed(Cdb__TblFeaCoppTrapGroup *p_pb_fea_copp_trap_group);
int32
pb_tbl_fea_copp_trap_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_fea_copp_trap_group_dump(Cdb__TblFeaCoppTrapGroup *p_pb_fea_copp_trap_group, uint32 only_key, char *out);

/* pb functions for TBL_SFLOW_GLOBAL */
int32
pb_tbl_sflow_global_to_pb(uint32 only_key, tbl_sflow_global_t *p_sflow_glb, Cdb__TblSflowGlobal *p_pb_sflow_glb);
int32
pb_tbl_sflow_global_to_pb_free_packed(Cdb__TblSflowGlobal *p_pb_sflow_glb);
int32
pb_tbl_sflow_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sflow_global_dump(Cdb__TblSflowGlobal *p_pb_sflow_glb, uint32 only_key, char *out);

/* pb functions for TBL_SFLOW_COLLECTOR */
int32
pb_tbl_sflow_collector_to_pb(uint32 only_key, tbl_sflow_collector_t *p_sflow_collector, Cdb__TblSflowCollector *p_pb_sflow_collector);
int32
pb_tbl_sflow_collector_to_pb_free_packed(Cdb__TblSflowCollector *p_pb_sflow_collector);
int32
pb_tbl_sflow_collector_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sflow_collector_dump(Cdb__TblSflowCollector *p_pb_sflow_collector, uint32 only_key, char *out);

/* pb functions for TBL_SFLOW_COUNTER_PORT */
int32
pb_tbl_sflow_counter_port_to_pb(uint32 only_key, tbl_sflow_counter_port_t *p_sflow_counter_port, Cdb__TblSflowCounterPort *p_pb_sflow_counter_port);
int32
pb_tbl_sflow_counter_port_to_pb_free_packed(Cdb__TblSflowCounterPort *p_pb_sflow_counter_port);
int32
pb_tbl_sflow_counter_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_sflow_counter_port_dump(Cdb__TblSflowCounterPort *p_pb_sflow_counter_port, uint32 only_key, char *out);

/* pb functions for TBL_VRRP_GLOBAL */
int32
pb_tbl_vrrp_global_to_pb(uint32 only_key, tbl_vrrp_global_t *p_vrrp_glb, Cdb__TblVrrpGlobal *p_pb_vrrp_glb);
int32
pb_tbl_vrrp_global_to_pb_free_packed(Cdb__TblVrrpGlobal *p_pb_vrrp_glb);
int32
pb_tbl_vrrp_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vrrp_global_dump(Cdb__TblVrrpGlobal *p_pb_vrrp_glb, uint32 only_key, char *out);

/* pb functions for TBL_VRRP_VMAC */
int32
pb_tbl_vrrp_vmac_to_pb(uint32 only_key, tbl_vrrp_vmac_t *p_vrrp_vmac, Cdb__TblVrrpVmac *p_pb_vrrp_vmac);
int32
pb_tbl_vrrp_vmac_to_pb_free_packed(Cdb__TblVrrpVmac *p_pb_vrrp_vmac);
int32
pb_tbl_vrrp_vmac_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vrrp_vmac_dump(Cdb__TblVrrpVmac *p_pb_vrrp_vmac, uint32 only_key, char *out);

/* pb functions for TBL_VRRP_VIP */
int32
pb_tbl_vrrp_vip_to_pb(uint32 only_key, tbl_vrrp_vip_t *p_vrrp_vip, Cdb__TblVrrpVip *p_pb_vrrp_vip);
int32
pb_tbl_vrrp_vip_to_pb_free_packed(Cdb__TblVrrpVip *p_pb_vrrp_vip);
int32
pb_tbl_vrrp_vip_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vrrp_vip_dump(Cdb__TblVrrpVip *p_pb_vrrp_vip, uint32 only_key, char *out);

/* pb functions for TBL_VRRP_SESSION */
int32
pb_tbl_vrrp_session_to_pb(uint32 only_key, tbl_vrrp_session_t *p_vrrp_sess, Cdb__TblVrrpSession *p_pb_vrrp_sess);
int32
pb_tbl_vrrp_session_to_pb_free_packed(Cdb__TblVrrpSession *p_pb_vrrp_sess);
int32
pb_tbl_vrrp_session_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vrrp_session_dump(Cdb__TblVrrpSession *p_pb_vrrp_sess, uint32 only_key, char *out);

/* pb functions for TBL_SSM_GLOBAL */
int32
pb_tbl_ssm_global_to_pb(uint32 only_key, tbl_ssm_global_t *p_ssm_glb, Cdb__TblSsmGlobal *p_pb_ssm_glb);
int32
pb_tbl_ssm_global_to_pb_free_packed(Cdb__TblSsmGlobal *p_pb_ssm_glb);
int32
pb_tbl_ssm_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ssm_global_dump(Cdb__TblSsmGlobal *p_pb_ssm_glb, uint32 only_key, char *out);

/* pb functions for TBL_SSM_PORT */
int32
pb_tbl_ssm_port_to_pb(uint32 only_key, tbl_ssm_port_t *p_ssm_port, Cdb__TblSsmPort *p_pb_ssm_port);
int32
pb_tbl_ssm_port_to_pb_free_packed(Cdb__TblSsmPort *p_pb_ssm_port);
int32
pb_tbl_ssm_port_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ssm_port_dump(Cdb__TblSsmPort *p_pb_ssm_port, uint32 only_key, char *out);

/* pb functions for TBL_IPSLA_GLOBAL */
int32
pb_tbl_ipsla_global_to_pb(uint32 only_key, tbl_ipsla_global_t *p_ipsla_glb, Cdb__TblIpslaGlobal *p_pb_ipsla_glb);
int32
pb_tbl_ipsla_global_to_pb_free_packed(Cdb__TblIpslaGlobal *p_pb_ipsla_glb);
int32
pb_tbl_ipsla_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsla_global_dump(Cdb__TblIpslaGlobal *p_pb_ipsla_glb, uint32 only_key, char *out);

/* pb functions for TBL_IPSLA_ENTRY */
int32
pb_tbl_ipsla_entry_to_pb(uint32 only_key, tbl_ipsla_entry_t *p_ipsla_entry, Cdb__TblIpslaEntry *p_pb_ipsla_entry);
int32
pb_tbl_ipsla_entry_to_pb_free_packed(Cdb__TblIpslaEntry *p_pb_ipsla_entry);
int32
pb_tbl_ipsla_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsla_entry_dump(Cdb__TblIpslaEntry *p_pb_ipsla_entry, uint32 only_key, char *out);

/* pb functions for TBL_IPSLA_PACKET_STATS */
int32
pb_tbl_ipsla_packet_stats_to_pb(uint32 only_key, tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, Cdb__TblIpslaPacketStats *p_pb_ipsla_packet_stats);
int32
pb_tbl_ipsla_packet_stats_to_pb_free_packed(Cdb__TblIpslaPacketStats *p_pb_ipsla_packet_stats);
int32
pb_tbl_ipsla_packet_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsla_packet_stats_dump(Cdb__TblIpslaPacketStats *p_pb_ipsla_packet_stats, uint32 only_key, char *out);

/* pb functions for TBL_IPSLA_TEST_STATS */
int32
pb_tbl_ipsla_test_stats_to_pb(uint32 only_key, tbl_ipsla_test_stats_t *p_ipsla_test_stats, Cdb__TblIpslaTestStats *p_pb_ipsla_test_stats);
int32
pb_tbl_ipsla_test_stats_to_pb_free_packed(Cdb__TblIpslaTestStats *p_pb_ipsla_test_stats);
int32
pb_tbl_ipsla_test_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ipsla_test_stats_dump(Cdb__TblIpslaTestStats *p_pb_ipsla_test_stats, uint32 only_key, char *out);

/* pb functions for TBL_TRACK_GLOBAL */
int32
pb_tbl_track_global_to_pb(uint32 only_key, tbl_track_global_t *p_track_glb, Cdb__TblTrackGlobal *p_pb_track_glb);
int32
pb_tbl_track_global_to_pb_free_packed(Cdb__TblTrackGlobal *p_pb_track_glb);
int32
pb_tbl_track_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_track_global_dump(Cdb__TblTrackGlobal *p_pb_track_glb, uint32 only_key, char *out);

/* pb functions for TBL_TRACK_OBJECT */
int32
pb_tbl_track_object_to_pb(uint32 only_key, tbl_track_object_t *p_track_object, Cdb__TblTrackObject *p_pb_track_object);
int32
pb_tbl_track_object_to_pb_free_packed(Cdb__TblTrackObject *p_pb_track_object);
int32
pb_tbl_track_object_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_track_object_dump(Cdb__TblTrackObject *p_pb_track_object, uint32 only_key, char *out);

/* pb functions for TBL_CONTROLLER_ATTR */
int32
pb_tbl_controller_attr_to_pb(uint32 only_key, tbl_controller_attr_t *p_controller_attr, Cdb__TblControllerAttr *p_pb_controller_attr);
int32
pb_tbl_controller_attr_to_pb_free_packed(Cdb__TblControllerAttr *p_pb_controller_attr);
int32
pb_tbl_controller_attr_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_controller_attr_dump(Cdb__TblControllerAttr *p_pb_controller_attr, uint32 only_key, char *out);

/* pb functions for TBL_OAM_SESSION */
int32
pb_tbl_oam_session_to_pb(uint32 only_key, tbl_oam_session_t *p_oam_session, Cdb__TblOamSession *p_pb_oam_session);
int32
pb_tbl_oam_session_to_pb_free_packed(Cdb__TblOamSession *p_pb_oam_session);
int32
pb_tbl_oam_session_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_oam_session_dump(Cdb__TblOamSession *p_pb_oam_session, uint32 only_key, char *out);

/* pb functions for TBL_OAM_SESSION_UPLOAD */
int32
pb_tbl_oam_session_upload_to_pb(uint32 only_key, tbl_oam_session_upload_t *p_oam_session_upload, Cdb__TblOamSessionUpload *p_pb_oam_session_upload);
int32
pb_tbl_oam_session_upload_to_pb_free_packed(Cdb__TblOamSessionUpload *p_pb_oam_session_upload);
int32
pb_tbl_oam_session_upload_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_oam_session_upload_dump(Cdb__TblOamSessionUpload *p_pb_oam_session_upload, uint32 only_key, char *out);

/* pb functions for TBL_LSP_APS_GROUP */
int32
pb_tbl_lsp_aps_group_to_pb(uint32 only_key, tbl_lsp_aps_group_t *p_lsp_aps_group, Cdb__TblLspApsGroup *p_pb_lsp_aps_group);
int32
pb_tbl_lsp_aps_group_to_pb_free_packed(Cdb__TblLspApsGroup *p_pb_lsp_aps_group);
int32
pb_tbl_lsp_aps_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lsp_aps_group_dump(Cdb__TblLspApsGroup *p_pb_lsp_aps_group, uint32 only_key, char *out);

/* pb functions for TBL_PW_APS_GROUP */
int32
pb_tbl_pw_aps_group_to_pb(uint32 only_key, tbl_pw_aps_group_t *p_pw_aps_group, Cdb__TblPwApsGroup *p_pb_pw_aps_group);
int32
pb_tbl_pw_aps_group_to_pb_free_packed(Cdb__TblPwApsGroup *p_pb_pw_aps_group);
int32
pb_tbl_pw_aps_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pw_aps_group_dump(Cdb__TblPwApsGroup *p_pb_pw_aps_group, uint32 only_key, char *out);

/* pb functions for TBL_LSP_PE */
int32
pb_tbl_lsp_pe_to_pb(uint32 only_key, tbl_lsp_pe_t *p_lsp_pe, Cdb__TblLspPe *p_pb_lsp_pe);
int32
pb_tbl_lsp_pe_to_pb_free_packed(Cdb__TblLspPe *p_pb_lsp_pe);
int32
pb_tbl_lsp_pe_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lsp_pe_dump(Cdb__TblLspPe *p_pb_lsp_pe, uint32 only_key, char *out);

/* pb functions for TBL_LSP_P */
int32
pb_tbl_lsp_p_to_pb(uint32 only_key, tbl_lsp_p_t *p_lsp_p, Cdb__TblLspP *p_pb_lsp_p);
int32
pb_tbl_lsp_p_to_pb_free_packed(Cdb__TblLspP *p_pb_lsp_p);
int32
pb_tbl_lsp_p_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_lsp_p_dump(Cdb__TblLspP *p_pb_lsp_p, uint32 only_key, char *out);

/* pb functions for TBL_PW_TPE */
int32
pb_tbl_pw_tpe_to_pb(uint32 only_key, tbl_pw_tpe_t *p_pw_tpe, Cdb__TblPwTpe *p_pb_pw_tpe);
int32
pb_tbl_pw_tpe_to_pb_free_packed(Cdb__TblPwTpe *p_pb_pw_tpe);
int32
pb_tbl_pw_tpe_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pw_tpe_dump(Cdb__TblPwTpe *p_pb_pw_tpe, uint32 only_key, char *out);

/* pb functions for TBL_SECTION_OAM */
int32
pb_tbl_section_oam_to_pb(uint32 only_key, tbl_section_oam_t *p_section_oam, Cdb__TblSectionOam *p_pb_section_oam);
int32
pb_tbl_section_oam_to_pb_free_packed(Cdb__TblSectionOam *p_pb_section_oam);
int32
pb_tbl_section_oam_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_section_oam_dump(Cdb__TblSectionOam *p_pb_section_oam, uint32 only_key, char *out);

/* pb functions for TBL_OAM_FLOW_INFO */
int32
pb_tbl_oam_flow_info_to_pb(uint32 only_key, tbl_oam_flow_info_t *p_oam_flow_info, Cdb__TblOamFlowInfo *p_pb_oam_flow_info);
int32
pb_tbl_oam_flow_info_to_pb_free_packed(Cdb__TblOamFlowInfo *p_pb_oam_flow_info);
int32
pb_tbl_oam_flow_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_oam_flow_info_dump(Cdb__TblOamFlowInfo *p_pb_oam_flow_info, uint32 only_key, char *out);

/* pb functions for TBL_TPOAM_GLOBAL */
int32
pb_tbl_tpoam_global_to_pb(uint32 only_key, tbl_tpoam_global_t *p_tpoam_global, Cdb__TblTpoamGlobal *p_pb_tpoam_global);
int32
pb_tbl_tpoam_global_to_pb_free_packed(Cdb__TblTpoamGlobal *p_pb_tpoam_global);
int32
pb_tbl_tpoam_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tpoam_global_dump(Cdb__TblTpoamGlobal *p_pb_tpoam_global, uint32 only_key, char *out);

/* pb functions for TBL_TPOAM_EXEC_GLOBAL */
int32
pb_tbl_tpoam_exec_global_to_pb(uint32 only_key, tbl_tpoam_exec_global_t *p_tpoam_exec_global, Cdb__TblTpoamExecGlobal *p_pb_tpoam_exec_global);
int32
pb_tbl_tpoam_exec_global_to_pb_free_packed(Cdb__TblTpoamExecGlobal *p_pb_tpoam_exec_global);
int32
pb_tbl_tpoam_exec_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tpoam_exec_global_dump(Cdb__TblTpoamExecGlobal *p_pb_tpoam_exec_global, uint32 only_key, char *out);

/* pb functions for TBL_APS_GROUP_INFO */
int32
pb_tbl_aps_group_info_to_pb(uint32 only_key, tbl_aps_group_info_t *p_aps_group_info, Cdb__TblApsGroupInfo *p_pb_aps_group_info);
int32
pb_tbl_aps_group_info_to_pb_free_packed(Cdb__TblApsGroupInfo *p_pb_aps_group_info);
int32
pb_tbl_aps_group_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_aps_group_info_dump(Cdb__TblApsGroupInfo *p_pb_aps_group_info, uint32 only_key, char *out);

/* pb functions for TBL_OAM_INLABEL_INFO */
int32
pb_tbl_oam_inlabel_info_to_pb(uint32 only_key, tbl_oam_inlabel_info_t *p_oam_inlabel_info, Cdb__TblOamInlabelInfo *p_pb_oam_inlabel_info);
int32
pb_tbl_oam_inlabel_info_to_pb_free_packed(Cdb__TblOamInlabelInfo *p_pb_oam_inlabel_info);
int32
pb_tbl_oam_inlabel_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_oam_inlabel_info_dump(Cdb__TblOamInlabelInfo *p_pb_oam_inlabel_info, uint32 only_key, char *out);

/* pb functions for TBL_OAM_MPLS_APS_CHECK */
int32
pb_tbl_oam_mpls_aps_check_to_pb(uint32 only_key, tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, Cdb__TblOamMplsApsCheck *p_pb_oam_mpls_aps_check);
int32
pb_tbl_oam_mpls_aps_check_to_pb_free_packed(Cdb__TblOamMplsApsCheck *p_pb_oam_mpls_aps_check);
int32
pb_tbl_oam_mpls_aps_check_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_oam_mpls_aps_check_dump(Cdb__TblOamMplsApsCheck *p_pb_oam_mpls_aps_check, uint32 only_key, char *out);

/* pb functions for TBL_VLAN_MAPPING_TABLE */
int32
pb_tbl_vlan_mapping_table_to_pb(uint32 only_key, tbl_vlan_mapping_table_t *p_vlan_mapping_table, Cdb__TblVlanMappingTable *p_pb_vlan_mapping_table);
int32
pb_tbl_vlan_mapping_table_to_pb_free_packed(Cdb__TblVlanMappingTable *p_pb_vlan_mapping_table);
int32
pb_tbl_vlan_mapping_table_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlan_mapping_table_dump(Cdb__TblVlanMappingTable *p_pb_vlan_mapping_table, uint32 only_key, char *out);

/* pb functions for TBL_VLAN_MAPPING_ENTRY */
int32
pb_tbl_vlan_mapping_entry_to_pb(uint32 only_key, tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, Cdb__TblVlanMappingEntry *p_pb_vlan_mapping_entry);
int32
pb_tbl_vlan_mapping_entry_to_pb_free_packed(Cdb__TblVlanMappingEntry *p_pb_vlan_mapping_entry);
int32
pb_tbl_vlan_mapping_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlan_mapping_entry_dump(Cdb__TblVlanMappingEntry *p_pb_vlan_mapping_entry, uint32 only_key, char *out);

/* pb functions for TBL_EGRESS_MAPPING_ENTRY */
int32
pb_tbl_egress_mapping_entry_to_pb(uint32 only_key, tbl_egress_mapping_entry_t *p_egress_mapping_entry, Cdb__TblEgressMappingEntry *p_pb_egress_mapping_entry);
int32
pb_tbl_egress_mapping_entry_to_pb_free_packed(Cdb__TblEgressMappingEntry *p_pb_egress_mapping_entry);
int32
pb_tbl_egress_mapping_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_egress_mapping_entry_dump(Cdb__TblEgressMappingEntry *p_pb_egress_mapping_entry, uint32 only_key, char *out);

/* pb functions for TBL_VLAN_GROUP */
int32
pb_tbl_vlan_group_to_pb(uint32 only_key, tbl_vlan_group_t *p_vlan_group, Cdb__TblVlanGroup *p_pb_vlan_group);
int32
pb_tbl_vlan_group_to_pb_free_packed(Cdb__TblVlanGroup *p_pb_vlan_group);
int32
pb_tbl_vlan_group_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_vlan_group_dump(Cdb__TblVlanGroup *p_pb_vlan_group, uint32 only_key, char *out);

/* pb functions for TBL_EVC_ENTRY */
int32
pb_tbl_evc_entry_to_pb(uint32 only_key, tbl_evc_entry_t *p_evc_entry, Cdb__TblEvcEntry *p_pb_evc_entry);
int32
pb_tbl_evc_entry_to_pb_free_packed(Cdb__TblEvcEntry *p_pb_evc_entry);
int32
pb_tbl_evc_entry_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_evc_entry_dump(Cdb__TblEvcEntry *p_pb_evc_entry, uint32 only_key, char *out);

/* pb functions for TBL_MAPPED_VLAN_INFO */
int32
pb_tbl_mapped_vlan_info_to_pb(uint32 only_key, tbl_mapped_vlan_info_t *p_mapped_vlan_info, Cdb__TblMappedVlanInfo *p_pb_mapped_vlan_info);
int32
pb_tbl_mapped_vlan_info_to_pb_free_packed(Cdb__TblMappedVlanInfo *p_pb_mapped_vlan_info);
int32
pb_tbl_mapped_vlan_info_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_mapped_vlan_info_dump(Cdb__TblMappedVlanInfo *p_pb_mapped_vlan_info, uint32 only_key, char *out);

/* pb functions for TBL_L2_PROTOCOL */
int32
pb_tbl_l2_protocol_to_pb(uint32 only_key, tbl_l2_protocol_t *p_l2_protocol, Cdb__TblL2Protocol *p_pb_l2_protocol);
int32
pb_tbl_l2_protocol_to_pb_free_packed(Cdb__TblL2Protocol *p_pb_l2_protocol);
int32
pb_tbl_l2_protocol_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_l2_protocol_dump(Cdb__TblL2Protocol *p_pb_l2_protocol, uint32 only_key, char *out);

/* pb functions for TBL_ND_FIB */
int32
pb_tbl_nd_fib_to_pb(uint32 only_key, tbl_nd_fib_t *p_nd_fib, Cdb__TblNdFib *p_pb_nd_fib);
int32
pb_tbl_nd_fib_to_pb_free_packed(Cdb__TblNdFib *p_pb_nd_fib);
int32
pb_tbl_nd_fib_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nd_fib_dump(Cdb__TblNdFib *p_pb_nd_fib, uint32 only_key, char *out);

/* pb functions for TBL_ND_STATS */
int32
pb_tbl_nd_stats_to_pb(uint32 only_key, tbl_nd_stats_t *p_nd_stats, Cdb__TblNdStats *p_pb_nd_stats);
int32
pb_tbl_nd_stats_to_pb_free_packed(Cdb__TblNdStats *p_pb_nd_stats);
int32
pb_tbl_nd_stats_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nd_stats_dump(Cdb__TblNdStats *p_pb_nd_stats, uint32 only_key, char *out);

/* pb functions for TBL_CPU_MIRROR_GLOBAL */
int32
pb_tbl_cpu_mirror_global_to_pb(uint32 only_key, tbl_cpu_mirror_global_t *p_cpu_mirror, Cdb__TblCpuMirrorGlobal *p_pb_cpu_mirror);
int32
pb_tbl_cpu_mirror_global_to_pb_free_packed(Cdb__TblCpuMirrorGlobal *p_pb_cpu_mirror);
int32
pb_tbl_cpu_mirror_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cpu_mirror_global_dump(Cdb__TblCpuMirrorGlobal *p_pb_cpu_mirror, uint32 only_key, char *out);

/* pb functions for TBL_TAP_SERVER */
int32
pb_tbl_tap_server_to_pb(uint32 only_key, tbl_tap_server_t *p_tap_srv, Cdb__TblTapServer *p_pb_tap_srv);
int32
pb_tbl_tap_server_to_pb_free_packed(Cdb__TblTapServer *p_pb_tap_srv);
int32
pb_tbl_tap_server_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_tap_server_dump(Cdb__TblTapServer *p_pb_tap_srv, uint32 only_key, char *out);

/* pb functions for TBL_DHCPV6_DEBUG */
int32
pb_tbl_dhcpv6_debug_to_pb(uint32 only_key, tbl_dhcpv6_debug_t *p_dhcpv6_debug, Cdb__TblDhcpv6Debug *p_pb_dhcpv6_debug);
int32
pb_tbl_dhcpv6_debug_to_pb_free_packed(Cdb__TblDhcpv6Debug *p_pb_dhcpv6_debug);
int32
pb_tbl_dhcpv6_debug_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_dhcpv6_debug_dump(Cdb__TblDhcpv6Debug *p_pb_dhcpv6_debug, uint32 only_key, char *out);

/* pb functions for TBL_L2EDIT */
int32
pb_tbl_l2edit_to_pb(uint32 only_key, tbl_l2edit_t *p_l2edit, Cdb__TblL2edit *p_pb_l2edit);
int32
pb_tbl_l2edit_to_pb_free_packed(Cdb__TblL2edit *p_pb_l2edit);
int32
pb_tbl_l2edit_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_l2edit_dump(Cdb__TblL2edit *p_pb_l2edit, uint32 only_key, char *out);

/* pb functions for TBL_HASH_FIELD_PROFILE */
int32
pb_tbl_hash_field_profile_to_pb(uint32 only_key, tbl_hash_field_profile_t *p_hash_field_profile, Cdb__TblHashFieldProfile *p_pb_hash_field_profile);
int32
pb_tbl_hash_field_profile_to_pb_free_packed(Cdb__TblHashFieldProfile *p_pb_hash_field_profile);
int32
pb_tbl_hash_field_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_hash_field_profile_dump(Cdb__TblHashFieldProfile *p_pb_hash_field_profile, uint32 only_key, char *out);

/* pb functions for TBL_HASH_VALUE_PROFILE */
int32
pb_tbl_hash_value_profile_to_pb(uint32 only_key, tbl_hash_value_profile_t *p_hash_value_profile, Cdb__TblHashValueProfile *p_pb_hash_value_profile);
int32
pb_tbl_hash_value_profile_to_pb_free_packed(Cdb__TblHashValueProfile *p_pb_hash_value_profile);
int32
pb_tbl_hash_value_profile_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_hash_value_profile_dump(Cdb__TblHashValueProfile *p_pb_hash_value_profile, uint32 only_key, char *out);

/* pb functions for TBL_HASH_VALUE_GLOBAL */
int32
pb_tbl_hash_value_global_to_pb(uint32 only_key, tbl_hash_value_global_t *p_hash_value_global, Cdb__TblHashValueGlobal *p_pb_hash_value_global);
int32
pb_tbl_hash_value_global_to_pb_free_packed(Cdb__TblHashValueGlobal *p_pb_hash_value_global);
int32
pb_tbl_hash_value_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_hash_value_global_dump(Cdb__TblHashValueGlobal *p_pb_hash_value_global, uint32 only_key, char *out);

/* pb functions for TBL_NAT_GLOBAL */
int32
pb_tbl_nat_global_to_pb(uint32 only_key, tbl_nat_global_t *p_nat_glb, Cdb__TblNatGlobal *p_pb_nat_glb);
int32
pb_tbl_nat_global_to_pb_free_packed(Cdb__TblNatGlobal *p_pb_nat_glb);
int32
pb_tbl_nat_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_global_dump(Cdb__TblNatGlobal *p_pb_nat_glb, uint32 only_key, char *out);

/* pb functions for TBL_NAT_POOL */
int32
pb_tbl_nat_pool_to_pb(uint32 only_key, tbl_nat_pool_t *p_nat_pool, Cdb__TblNatPool *p_pb_nat_pool);
int32
pb_tbl_nat_pool_to_pb_free_packed(Cdb__TblNatPool *p_pb_nat_pool);
int32
pb_tbl_nat_pool_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_pool_dump(Cdb__TblNatPool *p_pb_nat_pool, uint32 only_key, char *out);

/* pb functions for TBL_NAT_RULE */
int32
pb_tbl_nat_rule_to_pb(uint32 only_key, tbl_nat_rule_t *p_nat_rule, Cdb__TblNatRule *p_pb_nat_rule);
int32
pb_tbl_nat_rule_to_pb_free_packed(Cdb__TblNatRule *p_pb_nat_rule);
int32
pb_tbl_nat_rule_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_rule_dump(Cdb__TblNatRule *p_pb_nat_rule, uint32 only_key, char *out);

/* pb functions for TBL_NAT_SESSION */
int32
pb_tbl_nat_session_to_pb(uint32 only_key, tbl_nat_session_t *p_nat_session, Cdb__TblNatSession *p_pb_nat_session);
int32
pb_tbl_nat_session_to_pb_free_packed(Cdb__TblNatSession *p_pb_nat_session);
int32
pb_tbl_nat_session_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_session_dump(Cdb__TblNatSession *p_pb_nat_session, uint32 only_key, char *out);

/* pb functions for TBL_NAT_SESSION_LIMIT */
int32
pb_tbl_nat_session_limit_to_pb(uint32 only_key, tbl_nat_session_limit_t *p_nat_session_limit, Cdb__TblNatSessionLimit *p_pb_nat_session_limit);
int32
pb_tbl_nat_session_limit_to_pb_free_packed(Cdb__TblNatSessionLimit *p_pb_nat_session_limit);
int32
pb_tbl_nat_session_limit_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_session_limit_dump(Cdb__TblNatSessionLimit *p_pb_nat_session_limit, uint32 only_key, char *out);

/* pb functions for TBL_NAT_PROXY_ARP */
int32
pb_tbl_nat_proxy_arp_to_pb(uint32 only_key, tbl_nat_proxy_arp_t *p_nat_proxy_arp, Cdb__TblNatProxyArp *p_pb_nat_proxy_arp);
int32
pb_tbl_nat_proxy_arp_to_pb_free_packed(Cdb__TblNatProxyArp *p_pb_nat_proxy_arp);
int32
pb_tbl_nat_proxy_arp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_proxy_arp_dump(Cdb__TblNatProxyArp *p_pb_nat_proxy_arp, uint32 only_key, char *out);

/* pb functions for TBL_NAT_RTIF_INSIDE */
int32
pb_tbl_nat_rtif_inside_to_pb(uint32 only_key, tbl_nat_rtif_inside_t *p_nat_rtif_inside, Cdb__TblNatRtifInside *p_pb_nat_rtif_inside);
int32
pb_tbl_nat_rtif_inside_to_pb_free_packed(Cdb__TblNatRtifInside *p_pb_nat_rtif_inside);
int32
pb_tbl_nat_rtif_inside_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_rtif_inside_dump(Cdb__TblNatRtifInside *p_pb_nat_rtif_inside, uint32 only_key, char *out);

/* pb functions for TBL_NAT_RTIF_OUTSIDE */
int32
pb_tbl_nat_rtif_outside_to_pb(uint32 only_key, tbl_nat_rtif_outside_t *p_nat_rtif_outside, Cdb__TblNatRtifOutside *p_pb_nat_rtif_outside);
int32
pb_tbl_nat_rtif_outside_to_pb_free_packed(Cdb__TblNatRtifOutside *p_pb_nat_rtif_outside);
int32
pb_tbl_nat_rtif_outside_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_nat_rtif_outside_dump(Cdb__TblNatRtifOutside *p_pb_nat_rtif_outside, uint32 only_key, char *out);

/* pb functions for TBL_G8032_RING */
int32
pb_tbl_g8032_ring_to_pb(uint32 only_key, tbl_g8032_ring_t *p_g8032_ring, Cdb__TblG8032Ring *p_pb_g8032_ring);
int32
pb_tbl_g8032_ring_to_pb_free_packed(Cdb__TblG8032Ring *p_pb_g8032_ring);
int32
pb_tbl_g8032_ring_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_g8032_ring_dump(Cdb__TblG8032Ring *p_pb_g8032_ring, uint32 only_key, char *out);

/* pb functions for TBL_CFM_GLOBAL */
int32
pb_tbl_cfm_global_to_pb(uint32 only_key, tbl_cfm_global_t *p_cfm_glb, Cdb__TblCfmGlobal *p_pb_cfm_glb);
int32
pb_tbl_cfm_global_to_pb_free_packed(Cdb__TblCfmGlobal *p_pb_cfm_glb);
int32
pb_tbl_cfm_global_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_global_dump(Cdb__TblCfmGlobal *p_pb_cfm_glb, uint32 only_key, char *out);

/* pb functions for TBL_CFM_MD */
int32
pb_tbl_cfm_md_to_pb(uint32 only_key, tbl_cfm_md_t *p_md, Cdb__TblCfmMd *p_pb_md);
int32
pb_tbl_cfm_md_to_pb_free_packed(Cdb__TblCfmMd *p_pb_md);
int32
pb_tbl_cfm_md_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_md_dump(Cdb__TblCfmMd *p_pb_md, uint32 only_key, char *out);

/* pb functions for TBL_CFM_MA */
int32
pb_tbl_cfm_ma_to_pb(uint32 only_key, tbl_cfm_ma_t *p_ma, Cdb__TblCfmMa *p_pb_ma);
int32
pb_tbl_cfm_ma_to_pb_free_packed(Cdb__TblCfmMa *p_pb_ma);
int32
pb_tbl_cfm_ma_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_ma_dump(Cdb__TblCfmMa *p_pb_ma, uint32 only_key, char *out);

/* pb functions for TBL_CFM_MAID */
int32
pb_tbl_cfm_maid_to_pb(uint32 only_key, tbl_cfm_maid_t *p_maid, Cdb__TblCfmMaid *p_pb_maid);
int32
pb_tbl_cfm_maid_to_pb_free_packed(Cdb__TblCfmMaid *p_pb_maid);
int32
pb_tbl_cfm_maid_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_maid_dump(Cdb__TblCfmMaid *p_pb_maid, uint32 only_key, char *out);

/* pb functions for TBL_CFM_LMEP */
int32
pb_tbl_cfm_lmep_to_pb(uint32 only_key, tbl_cfm_lmep_t *p_lmep, Cdb__TblCfmLmep *p_pb_lmep);
int32
pb_tbl_cfm_lmep_to_pb_free_packed(Cdb__TblCfmLmep *p_pb_lmep);
int32
pb_tbl_cfm_lmep_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_lmep_dump(Cdb__TblCfmLmep *p_pb_lmep, uint32 only_key, char *out);

/* pb functions for TBL_CFM_RMEP */
int32
pb_tbl_cfm_rmep_to_pb(uint32 only_key, tbl_cfm_rmep_t *p_rmep, Cdb__TblCfmRmep *p_pb_rmep);
int32
pb_tbl_cfm_rmep_to_pb_free_packed(Cdb__TblCfmRmep *p_pb_rmep);
int32
pb_tbl_cfm_rmep_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_cfm_rmep_dump(Cdb__TblCfmRmep *p_pb_rmep, uint32 only_key, char *out);

/* pb functions for TBL_PIM_SM */
int32
pb_tbl_pim_sm_to_pb(uint32 only_key, tbl_pim_sm_t *p_pim_sm, Cdb__TblPimSm *p_pb_pim_sm);
int32
pb_tbl_pim_sm_to_pb_free_packed(Cdb__TblPimSm *p_pb_pim_sm);
int32
pb_tbl_pim_sm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_sm_dump(Cdb__TblPimSm *p_pb_pim_sm, uint32 only_key, char *out);

/* pb functions for TBL_PIM_SM_BSR */
int32
pb_tbl_pim_sm_bsr_to_pb(uint32 only_key, tbl_pim_sm_bsr_t *p_pim_sm_bsr, Cdb__TblPimSmBsr *p_pb_pim_sm_bsr);
int32
pb_tbl_pim_sm_bsr_to_pb_free_packed(Cdb__TblPimSmBsr *p_pb_pim_sm_bsr);
int32
pb_tbl_pim_sm_bsr_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_sm_bsr_dump(Cdb__TblPimSmBsr *p_pb_pim_sm_bsr, uint32 only_key, char *out);

/* pb functions for TBL_PIM_SM_RP */
int32
pb_tbl_pim_sm_rp_to_pb(uint32 only_key, tbl_pim_sm_rp_t *p_pim_sm_rp, Cdb__TblPimSmRp *p_pb_pim_sm_rp);
int32
pb_tbl_pim_sm_rp_to_pb_free_packed(Cdb__TblPimSmRp *p_pb_pim_sm_rp);
int32
pb_tbl_pim_sm_rp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_sm_rp_dump(Cdb__TblPimSmRp *p_pb_pim_sm_rp, uint32 only_key, char *out);

/* pb functions for TBL_PIM_SM_RP_GP */
int32
pb_tbl_pim_sm_rp_gp_to_pb(uint32 only_key, tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, Cdb__TblPimSmRpGp *p_pb_pim_sm_rp_gp);
int32
pb_tbl_pim_sm_rp_gp_to_pb_free_packed(Cdb__TblPimSmRpGp *p_pb_pim_sm_rp_gp);
int32
pb_tbl_pim_sm_rp_gp_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_sm_rp_gp_dump(Cdb__TblPimSmRpGp *p_pb_pim_sm_rp_gp, uint32 only_key, char *out);

/* pb functions for TBL_PIM_DM */
int32
pb_tbl_pim_dm_to_pb(uint32 only_key, tbl_pim_dm_t *p_pim_dm, Cdb__TblPimDm *p_pb_pim_dm);
int32
pb_tbl_pim_dm_to_pb_free_packed(Cdb__TblPimDm *p_pb_pim_dm);
int32
pb_tbl_pim_dm_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_pim_dm_dump(Cdb__TblPimDm *p_pb_pim_dm, uint32 only_key, char *out);

/* pb functions for TBL_IGMP_INTF */
int32
pb_tbl_igmp_intf_to_pb(uint32 only_key, tbl_igmp_intf_t *p_igmp_intf, Cdb__TblIgmpIntf *p_pb_igmp_intf);
int32
pb_tbl_igmp_intf_to_pb_free_packed(Cdb__TblIgmpIntf *p_pb_igmp_intf);
int32
pb_tbl_igmp_intf_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_igmp_intf_dump(Cdb__TblIgmpIntf *p_pb_igmp_intf, uint32 only_key, char *out);

/* pb functions for TBL_RIP */
int32
pb_tbl_rip_to_pb(uint32 only_key, tbl_rip_t *p_rip, Cdb__TblRip *p_pb_rip);
int32
pb_tbl_rip_to_pb_free_packed(Cdb__TblRip *p_pb_rip);
int32
pb_tbl_rip_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rip_dump(Cdb__TblRip *p_pb_rip, uint32 only_key, char *out);

/* pb functions for TBL_RIPNG */
int32
pb_tbl_ripng_to_pb(uint32 only_key, tbl_ripng_t *p_ripng, Cdb__TblRipng *p_pb_ripng);
int32
pb_tbl_ripng_to_pb_free_packed(Cdb__TblRipng *p_pb_ripng);
int32
pb_tbl_ripng_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ripng_dump(Cdb__TblRipng *p_pb_ripng, uint32 only_key, char *out);

/* pb functions for TBL_RIP_NETWORK */
int32
pb_tbl_rip_network_to_pb(uint32 only_key, tbl_rip_network_t *p_rip_network, Cdb__TblRipNetwork *p_pb_rip_network);
int32
pb_tbl_rip_network_to_pb_free_packed(Cdb__TblRipNetwork *p_pb_rip_network);
int32
pb_tbl_rip_network_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rip_network_dump(Cdb__TblRipNetwork *p_pb_rip_network, uint32 only_key, char *out);

/* pb functions for TBL_RIPNG_NETWORK */
int32
pb_tbl_ripng_network_to_pb(uint32 only_key, tbl_ripng_network_t *p_ripng_network, Cdb__TblRipngNetwork *p_pb_ripng_network);
int32
pb_tbl_ripng_network_to_pb_free_packed(Cdb__TblRipngNetwork *p_pb_ripng_network);
int32
pb_tbl_ripng_network_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ripng_network_dump(Cdb__TblRipngNetwork *p_pb_ripng_network, uint32 only_key, char *out);

/* pb functions for TBL_RIP_NEIGHBOR */
int32
pb_tbl_rip_neighbor_to_pb(uint32 only_key, tbl_rip_neighbor_t *p_rip_neighbor, Cdb__TblRipNeighbor *p_pb_rip_neighbor);
int32
pb_tbl_rip_neighbor_to_pb_free_packed(Cdb__TblRipNeighbor *p_pb_rip_neighbor);
int32
pb_tbl_rip_neighbor_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rip_neighbor_dump(Cdb__TblRipNeighbor *p_pb_rip_neighbor, uint32 only_key, char *out);

/* pb functions for TBL_RIP_PASSIVE_IF */
int32
pb_tbl_rip_passive_if_to_pb(uint32 only_key, tbl_rip_passive_if_t *p_rip_passive_if, Cdb__TblRipPassiveIf *p_pb_rip_passive_if);
int32
pb_tbl_rip_passive_if_to_pb_free_packed(Cdb__TblRipPassiveIf *p_pb_rip_passive_if);
int32
pb_tbl_rip_passive_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_rip_passive_if_dump(Cdb__TblRipPassiveIf *p_pb_rip_passive_if, uint32 only_key, char *out);

/* pb functions for TBL_RIPNG_PASSIVE_IF */
int32
pb_tbl_ripng_passive_if_to_pb(uint32 only_key, tbl_ripng_passive_if_t *p_ripng_passive_if, Cdb__TblRipngPassiveIf *p_pb_ripng_passive_if);
int32
pb_tbl_ripng_passive_if_to_pb_free_packed(Cdb__TblRipngPassiveIf *p_pb_ripng_passive_if);
int32
pb_tbl_ripng_passive_if_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ripng_passive_if_dump(Cdb__TblRipngPassiveIf *p_pb_ripng_passive_if, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_AREA_STUB */
int32
pb_tbl_ospf_area_stub_to_pb(uint32 only_key, tbl_ospf_area_stub_t *p_ospf_area_stub, Cdb__TblOspfAreaStub *p_pb_ospf_area_stub);
int32
pb_tbl_ospf_area_stub_to_pb_free_packed(Cdb__TblOspfAreaStub *p_pb_ospf_area_stub);
int32
pb_tbl_ospf_area_stub_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_area_stub_dump(Cdb__TblOspfAreaStub *p_pb_ospf_area_stub, uint32 only_key, char *out);

/* pb functions for TBL_OSPF6_AREA_STUB */
int32
pb_tbl_ospf6_area_stub_to_pb(uint32 only_key, tbl_ospf6_area_stub_t *p_ospf6_area_stub, Cdb__TblOspf6AreaStub *p_pb_ospf6_area_stub);
int32
pb_tbl_ospf6_area_stub_to_pb_free_packed(Cdb__TblOspf6AreaStub *p_pb_ospf6_area_stub);
int32
pb_tbl_ospf6_area_stub_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf6_area_stub_dump(Cdb__TblOspf6AreaStub *p_pb_ospf6_area_stub, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_AREA_NSSA */
int32
pb_tbl_ospf_area_nssa_to_pb(uint32 only_key, tbl_ospf_area_nssa_t *p_ospf_area_nssa, Cdb__TblOspfAreaNssa *p_pb_ospf_area_nssa);
int32
pb_tbl_ospf_area_nssa_to_pb_free_packed(Cdb__TblOspfAreaNssa *p_pb_ospf_area_nssa);
int32
pb_tbl_ospf_area_nssa_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_area_nssa_dump(Cdb__TblOspfAreaNssa *p_pb_ospf_area_nssa, uint32 only_key, char *out);

/* pb functions for TBL_OSPF_AREA_VLINK */
int32
pb_tbl_ospf_area_vlink_to_pb(uint32 only_key, tbl_ospf_area_vlink_t *p_ospf_area_vlink, Cdb__TblOspfAreaVlink *p_pb_ospf_area_vlink);
int32
pb_tbl_ospf_area_vlink_to_pb_free_packed(Cdb__TblOspfAreaVlink *p_pb_ospf_area_vlink);
int32
pb_tbl_ospf_area_vlink_pack(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
int32
pb_tbl_ospf_area_vlink_dump(Cdb__TblOspfAreaVlink *p_pb_ospf_area_vlink, uint32 only_key, char *out);

int32
pb_tbl_pack(uint32 tbl_id, void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);
void*
pb_tbl_unpack(uint32 tbl_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data);
int32
pb_tbl_dump(uint32 tbl_id, void *object, uint32 only_key, char *out);
int32
pb_tbl_free_unpacked(uint32 tbl_id, void *object, ProtobufCAllocator *allocator);

#endif  /* __PB_TBL_H__ */

