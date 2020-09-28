
#ifndef __PB_COMMON_H__
#define __PB_COMMON_H__

extern ProtobufCAllocator osp_pbc_allocator;

typedef struct
{
    uint8       type;       /* cdb_subscribe_type_t */
    uint32      pid;        /* client pid */
    uint32      src;        /* pm_id_t */
    uint32      format;     /* cdb_subscribe_format_t */

    struct
    {
        uint32  node_type;  /* cdb_node_type_t, dir/tbl */
        uint32  tbl_id;     /* cdb_tbl_t */
        uint32  field_id;   /* filter field */
        char *key;          /* filter tbl key */
    } cdb;
    
    struct
    {
        uint32 *tbl_id_list;/* cdb_tbl_t, tbl_id list */
        uint32  num;        /* count of tbl_id list */
        uint32 sync;        /* need sync */
    } tbl;
} pb_subscribe_request_t;

typedef struct
{
    uint8       oper;       /* cdb_oper_t, add/set/get/del */
    uint8       type;       /* cdb_node_type_t, tbl/ds */
    uint16      tbl_id;     /* cdb_tbl_t */
    uint16      ds_id;      /* cdb_ds_t */
    uint16      ds2_id;     /* cdb_ds_t */
    uint32      field_id;   /* field_id per tbl */
} pb_subscribe_response_t;

int32
pb_bitmap_array_dump(uint8 *array, uint32 len, char *out);

int32
pb_int8_array_dump(int8 *array, uint32 len, char *out);

int32
pb_uint8_array_dump(uint8 *array, uint32 len, char *out);

int32
pb_int32_array_dump(int32 *array, uint32 len, char *out);

int32
pb_uint32_array_dump(uint32 *array, uint32 len, char *out);

int32
pb_double_array_dump(double *array, uint32 len, char *out);

int32
pb_string_array_dump(char **array, uint32 len, char *out);

int32
pb_subscribe_response_to_pb(pb_subscribe_response_t *p_resp, Cdb__SubscribeResponse *p_pb);

int32
pb_subscribe_response_dump(Cdb__SubscribeResponse *p_pb, char *out);

int32
pb_subscribe_response_to_pb_free_packed(Cdb__SubscribeResponse *p_pb);

int32
pb_subscribe_response_pack(pb_subscribe_response_t *p_resp, uint8 *buf, uint32 buf_len);

int32
pb_subscribe_request_to_pb(pb_subscribe_request_t *p_req, Cdb__SubscribeRequest *p_pb);

int32
pb_subscribe_request_dump(Cdb__SubscribeRequest *p_pb, char *out);

int32
pb_subscribe_request_to_pb_free_packed(Cdb__SubscribeRequest *p_pb);

int32
pb_subscribe_request_pack(pb_subscribe_request_t *p_req, uint8 *buf, uint32 buf_len);

int32
pb_compose_fdb_key_t_to_pb(fdb_key_t *p_cdb, Cdb__ComposeFdbKeyT *p_pb);

int32
pb_compose_fdb_key_t_dump(Cdb__ComposeFdbKeyT *p_pb, char *out);

int32
pb_compose_fdb_key_t_to_pb_free_packed(Cdb__ComposeFdbKeyT *p_pb);

int32
pb_compose_mac_addr_t_to_pb(uint8 *p_cdb, Cdb__ComposeMacAddrT *p_pb);

int32
pb_compose_mac_addr_t_dump(Cdb__ComposeMacAddrT *p_pb, char *out);

int32
pb_compose_mac_addr_t_to_pb_free_packed(Cdb__ComposeMacAddrT *p_pb);

int32
pb_compose_msti_port_key_t_to_pb(msti_port_key_t *p_cdb, Cdb__ComposeMstiPortKeyT *p_pb);

int32
pb_compose_msti_port_key_t_dump(Cdb__ComposeMstiPortKeyT *p_pb, char *out);

int32
pb_compose_msti_port_key_t_to_pb_free_packed(Cdb__ComposeMstiPortKeyT *p_pb);

int32
pb_compose_route_node_key_t_to_pb(route_node_key_t *p_cdb, Cdb__ComposeRouteNodeKeyT *p_pb);

int32
pb_compose_route_node_key_t_dump(Cdb__ComposeRouteNodeKeyT *p_pb, char *out);

int32
pb_compose_route_node_key_t_to_pb_free_packed(Cdb__ComposeRouteNodeKeyT *p_pb);

int32
pb_compose_route_node_key_with_ad_t_to_pb(route_node_key_with_ad_t *p_cdb, Cdb__ComposeRouteNodeKeyWithAdT *p_pb);

int32
pb_compose_route_node_key_with_ad_t_dump(Cdb__ComposeRouteNodeKeyWithAdT *p_pb, char *out);

int32
pb_compose_route_node_key_with_ad_t_to_pb_free_packed(Cdb__ComposeRouteNodeKeyWithAdT *p_pb);

int32
pb_compose_ospf_area_range_key_t_to_pb(ospf_area_range_key_t *p_cdb, Cdb__ComposeOspfAreaRangeKeyT *p_pb);

int32
pb_compose_ospf_area_range_key_t_dump(Cdb__ComposeOspfAreaRangeKeyT *p_pb, char *out);

int32
pb_compose_ospf_area_range_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaRangeKeyT *p_pb);

int32
pb_compose_ospf6_area_range_key_t_to_pb(ospf6_area_range_key_t *p_cdb, Cdb__ComposeOspfAreaRangeKeyT *p_pb);

int32
pb_compose_ospf6_area_range_key_t_dump(Cdb__ComposeOspfAreaRangeKeyT *p_pb, char *out);

int32
pb_compose_ospf6_area_range_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaRangeKeyT *p_pb);

int32
pb_compose_ospf_area_vlink_key_t_to_pb(ospf_area_vlink_key_t *p_cdb, Cdb__ComposeOspfAreaVlinkKeyT *p_pb);

int32
pb_compose_ospf_area_vlink_key_t_dump(Cdb__ComposeOspfAreaVlinkKeyT *p_pb, char *out);

int32
pb_compose_ospf_area_vlink_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaVlinkKeyT *p_pb);

int32
pb_compose_rip_timers_t_to_pb(rip_timers_t *p_cdb, Cdb__ComposeRipTimersT *p_pb);

int32
pb_compose_rip_timers_t_dump(Cdb__ComposeRipTimersT *p_pb, char *out);

int32
pb_compose_rip_timers_t_to_pb_free_packed(Cdb__ComposeRipTimersT *p_pb);

int32
pb_compose_binding_key_t_to_pb(binding_key_t *p_cdb, Cdb__ComposeBindingKeyT *p_pb);

int32
pb_compose_binding_key_t_dump(Cdb__ComposeBindingKeyT *p_pb, char *out);

int32
pb_compose_binding_key_t_to_pb_free_packed(Cdb__ComposeBindingKeyT *p_pb);

int32
pb_compose_ns_port_forwarding_key_t_to_pb(ns_port_forwarding_key_t *p_cdb, Cdb__ComposeNsPortForwardingKeyT *p_pb);

int32
pb_compose_ns_port_forwarding_key_t_dump(Cdb__ComposeNsPortForwardingKeyT *p_pb, char *out);

int32
pb_compose_ns_port_forwarding_key_t_to_pb_free_packed(Cdb__ComposeNsPortForwardingKeyT *p_pb);

int32
pb_compose_ns_route_key_t_to_pb(ns_route_key_t *p_cdb, Cdb__ComposeNsRouteKeyT *p_pb);

int32
pb_compose_ns_route_key_t_dump(Cdb__ComposeNsRouteKeyT *p_pb, char *out);

int32
pb_compose_ns_route_key_t_to_pb_free_packed(Cdb__ComposeNsRouteKeyT *p_pb);



int32
pb_compose_summer_time_t_to_pb(summer_time_t * p_tm,Cdb__ComposeSummerTimeT * tm);

int32
pb_compose_which_summer_time_t_to_pb(which_summer_time_t * p_wtm,Cdb__ComposeWhichSummerTimeT * wtm);

int32
pb_compose_summer_time_t_to_pb_free_packed(Cdb__ComposeSummerTimeT *p_tm);

int32
pb_compose_which_summer_time_t_to_pb_free_packed(Cdb__ComposeWhichSummerTimeT  *p_wtm);

int32
pb_compose_summer_time_t_dump(Cdb__ComposeSummerTimeT*p_tm,char* out);

int32
pb_compose_which_summer_time_t_dump(Cdb__ComposeWhichSummerTimeT *p_wtm,char* out);




int32
pb_compose_openflow_manager_key_t_to_pb(openflow_manager_key_t *p_cdb, Cdb__ComposeOpenflowManagerKeyT *p_pb);

int32
pb_compose_openflow_manager_key_t_dump(Cdb__ComposeOpenflowManagerKeyT *p_pb, char *out);

int32
pb_compose_openflow_manager_key_t_to_pb_free_packed(Cdb__ComposeOpenflowManagerKeyT *p_pb);

int32
pb_compose_mstp_digest_t_to_pb(mstp_digest_t *p_cdb, Cdb__ComposeMstpDigestT *p_pb);

int32
pb_compose_mstp_digest_t_dump(Cdb__ComposeMstpDigestT *p_pb, char *out);

int32
pb_compose_mstp_digest_t_to_pb_free_packed(Cdb__ComposeMstpDigestT *p_pb);

int32
pb_compose_ptp_default_t_to_pb(ptp_default_t *p_cdb, Cdb__ComposePtpDefaultT *p_pb);

int32
pb_compose_ptp_default_t_dump(Cdb__ComposePtpDefaultT *p_pb, char *out);

int32
pb_compose_ptp_default_t_to_pb_free_packed(Cdb__ComposePtpDefaultT *p_pb);

int32
pb_compose_ptp_current_t_to_pb(ptp_current_t *p_cdb, Cdb__ComposePtpCurrentT *p_pb);

int32
pb_compose_ptp_current_t_dump(Cdb__ComposePtpCurrentT *p_pb, char *out);

int32
pb_compose_ptp_current_t_to_pb_free_packed(Cdb__ComposePtpCurrentT *p_pb);

int32
pb_compose_ptp_parent_t_to_pb(ptp_parent_t *p_cdb, Cdb__ComposePtpParentT *p_pb);

int32
pb_compose_ptp_parent_t_dump(Cdb__ComposePtpParentT *p_pb, char *out);

int32
pb_compose_ptp_parent_t_to_pb_free_packed(Cdb__ComposePtpParentT *p_pb);

int32
pb_compose_ptp_time_properties_t_to_pb(ptp_time_properties_t *p_cdb, Cdb__ComposePtpTimePropertiesT *p_pb);

int32
pb_compose_ptp_time_properties_t_dump(Cdb__ComposePtpTimePropertiesT *p_pb, char *out);

int32
pb_compose_ptp_time_properties_t_to_pb_free_packed(Cdb__ComposePtpTimePropertiesT *p_pb);

int32
pb_compose_ptp_global_c_t_to_pb(ptp_global_c_t *p_cdb, Cdb__ComposePtpGlobalCT *p_pb);

int32
pb_compose_ptp_global_c_t_dump(Cdb__ComposePtpGlobalCT *p_pb, char *out);

int32
pb_compose_ptp_global_c_t_to_pb_free_packed(Cdb__ComposePtpGlobalCT *p_pb);

int32
pb_compose_ptp_port_t_to_pb(ptp_port_t *p_cdb, Cdb__ComposePtpPortT *p_pb);

int32
pb_compose_ptp_port_t_dump(Cdb__ComposePtpPortT *p_pb, char *out);

int32
pb_compose_ptp_port_t_to_pb_free_packed(Cdb__ComposePtpPortT *p_pb);

int32
pb_compose_glb_ptp_port_address_t_to_pb(glb_ptp_port_address_t *p_cdb, Cdb__ComposeGlbPtpPortAddressT *p_pb);

int32
pb_compose_glb_ptp_port_address_t_dump(Cdb__ComposeGlbPtpPortAddressT *p_pb, char *out);

int32
pb_compose_glb_ptp_port_address_t_to_pb_free_packed(Cdb__ComposeGlbPtpPortAddressT *p_pb);

int32
pb_compose_glb_ptp_timeinterval_t_to_pb(glb_ptp_timeinterval_t *p_cdb, Cdb__ComposeGlbPtpTimeintervalT *p_pb);

int32
pb_compose_glb_ptp_timeinterval_t_dump(Cdb__ComposeGlbPtpTimeintervalT *p_pb, char *out);

int32
pb_compose_glb_ptp_timeinterval_t_to_pb_free_packed(Cdb__ComposeGlbPtpTimeintervalT *p_pb);

int32
pb_compose_ptp_vlan_info_t_to_pb(ptp_vlan_info_t *p_cdb, Cdb__ComposePtpVlanInfoT *p_pb);

int32
pb_compose_ptp_vlan_info_t_dump(Cdb__ComposePtpVlanInfoT *p_pb, char *out);

int32
pb_compose_ptp_vlan_info_t_to_pb_free_packed(Cdb__ComposePtpVlanInfoT *p_pb);

int32
pb_compose_ptp_dump_flag_t_to_pb(ptp_dump_flag_t *p_cdb, Cdb__ComposePtpDumpFlagT *p_pb);

int32
pb_compose_ptp_dump_flag_t_dump(Cdb__ComposePtpDumpFlagT *p_pb, char *out);

int32
pb_compose_ptp_dump_flag_t_to_pb_free_packed(Cdb__ComposePtpDumpFlagT *p_pb);

int32
pb_compose_ptp_port_p_t_to_pb(ptp_port_p_t *p_cdb, Cdb__ComposePtpPortPT *p_pb);

int32
pb_compose_ptp_port_p_t_dump(Cdb__ComposePtpPortPT *p_pb, char *out);

int32
pb_compose_ptp_port_p_t_to_pb_free_packed(Cdb__ComposePtpPortPT *p_pb);

int32
pb_compose_ptp_port_c_t_to_pb(ptp_port_c_t *p_cdb, Cdb__ComposePtpPortCT *p_pb);

int32
pb_compose_ptp_port_c_t_dump(Cdb__ComposePtpPortCT *p_pb, char *out);

int32
pb_compose_ptp_port_c_t_to_pb_free_packed(Cdb__ComposePtpPortCT *p_pb);

int32
pb_compose_ptp_port_stats_t_to_pb(ptp_port_stats_t *p_cdb, Cdb__ComposePtpPortStatsT *p_pb);

int32
pb_compose_ptp_port_stats_t_dump(Cdb__ComposePtpPortStatsT *p_pb, char *out);

int32
pb_compose_ptp_port_stats_t_to_pb_free_packed(Cdb__ComposePtpPortStatsT *p_pb);

int32
pb_compose_ptp_msg_proc_ops_t_to_pb(ptp_msg_proc_ops_t *p_cdb, Cdb__ComposePtpMsgProcOpsT *p_pb);

int32
pb_compose_ptp_msg_proc_ops_t_dump(Cdb__ComposePtpMsgProcOpsT *p_pb, char *out);

int32
pb_compose_ptp_msg_proc_ops_t_to_pb_free_packed(Cdb__ComposePtpMsgProcOpsT *p_pb);

int32
pb_compose_kernel_if_ipv6_addr_t_to_pb(kernel_if_ipv6_addr_t *p_cdb, Cdb__ComposeKernelIfIpv6AddrT *p_pb);

int32
pb_compose_kernel_if_ipv6_addr_t_dump(Cdb__ComposeKernelIfIpv6AddrT *p_pb, char *out);

int32
pb_compose_kernel_if_ipv6_addr_t_to_pb_free_packed(Cdb__ComposeKernelIfIpv6AddrT *p_pb);

int32
pb_compose_brg_allowed_vlan_t_to_pb(brg_allowed_vlan_t *p_cdb, Cdb__ComposeBrgAllowedVlanT *p_pb);

int32
pb_compose_brg_allowed_vlan_t_dump(Cdb__ComposeBrgAllowedVlanT *p_pb, char *out);

int32
pb_compose_brg_allowed_vlan_t_to_pb_free_packed(Cdb__ComposeBrgAllowedVlanT *p_pb);

int32
pb_compose_ptp_common_message_header_t_to_pb(ptp_common_message_header_t *p_cdb, Cdb__ComposePtpCommonMessageHeaderT *p_pb);

int32
pb_compose_ptp_common_message_header_t_dump(Cdb__ComposePtpCommonMessageHeaderT *p_pb, char *out);

int32
pb_compose_ptp_common_message_header_t_to_pb_free_packed(Cdb__ComposePtpCommonMessageHeaderT *p_pb);

int32
pb_compose_ptp_msg_announce_t_to_pb(ptp_msg_announce_t *p_cdb, Cdb__ComposePtpMsgAnnounceT *p_pb);

int32
pb_compose_ptp_msg_announce_t_dump(Cdb__ComposePtpMsgAnnounceT *p_pb, char *out);

int32
pb_compose_ptp_msg_announce_t_to_pb_free_packed(Cdb__ComposePtpMsgAnnounceT *p_pb);

int32
pb_compose_tpoam_dm_t_to_pb(tpoam_dm_t *p_cdb, Cdb__ComposeTpoamDmT *p_pb);

int32
pb_compose_tpoam_dm_t_dump(Cdb__ComposeTpoamDmT *p_pb, char *out);

int32
pb_compose_tpoam_dm_t_to_pb_free_packed(Cdb__ComposeTpoamDmT *p_pb);

int32
pb_compose_tpoam_lm_t_to_pb(tpoam_lm_t *p_cdb, Cdb__ComposeTpoamLmT *p_pb);

int32
pb_compose_tpoam_lm_t_dump(Cdb__ComposeTpoamLmT *p_pb, char *out);

int32
pb_compose_tpoam_lm_t_to_pb_free_packed(Cdb__ComposeTpoamLmT *p_pb);

int32
pb_compose_tpoam_lb_t_to_pb(tpoam_lb_t *p_cdb, Cdb__ComposeTpoamLbT *p_pb);

int32
pb_compose_tpoam_lb_t_dump(Cdb__ComposeTpoamLbT *p_pb, char *out);

int32
pb_compose_tpoam_lb_t_to_pb_free_packed(Cdb__ComposeTpoamLbT *p_pb);

int32
pb_compose_tpoam_event_t_to_pb(tpoam_event_t *p_cdb, Cdb__ComposeTpoamEventT *p_pb);

int32
pb_compose_tpoam_event_t_dump(Cdb__ComposeTpoamEventT *p_pb, char *out);

int32
pb_compose_tpoam_event_t_to_pb_free_packed(Cdb__ComposeTpoamEventT *p_pb);

int32
pb_compose_tpoam_stats_t_to_pb(tpoam_stats_t *p_cdb, Cdb__ComposeTpoamStatsT *p_pb);

int32
pb_compose_tpoam_stats_t_dump(Cdb__ComposeTpoamStatsT *p_pb, char *out);

int32
pb_compose_tpoam_stats_t_to_pb_free_packed(Cdb__ComposeTpoamStatsT *p_pb);

int32
pb_compose_tpoam_list_t_to_pb(tpoam_list_t *p_cdb, Cdb__ComposeTpoamListT *p_pb);

int32
pb_compose_tpoam_list_t_dump(Cdb__ComposeTpoamListT *p_pb, char *out);

int32
pb_compose_tpoam_list_t_to_pb_free_packed(Cdb__ComposeTpoamListT *p_pb);

int32
pb_compose_tpoam_rc_count_t_to_pb(tpoam_rc_count_t *p_cdb, Cdb__ComposeTpoamRcCountT *p_pb);

int32
pb_compose_tpoam_rc_count_t_dump(Cdb__ComposeTpoamRcCountT *p_pb, char *out);

int32
pb_compose_tpoam_rc_count_t_to_pb_free_packed(Cdb__ComposeTpoamRcCountT *p_pb);

int32
pb_compose_tpoam_exec_t_to_pb(tpoam_exec_t *p_cdb, Cdb__ComposeTpoamExecT *p_pb);

int32
pb_compose_tpoam_exec_t_dump(Cdb__ComposeTpoamExecT *p_pb, char *out);

int32
pb_compose_tpoam_exec_t_to_pb_free_packed(Cdb__ComposeTpoamExecT *p_pb);

int32
pb_compose_tpoam_trap_cache_t_to_pb(tpoam_trap_cache_t *p_cdb, Cdb__ComposeTpoamTrapCacheT *p_pb);

int32
pb_compose_tpoam_trap_cache_t_dump(Cdb__ComposeTpoamTrapCacheT *p_pb, char *out);

int32
pb_compose_tpoam_trap_cache_t_to_pb_free_packed(Cdb__ComposeTpoamTrapCacheT *p_pb);

int32
pb_compose_tpoam_ratelimit_t_to_pb(tpoam_ratelimit_t *p_cdb, Cdb__ComposeTpoamRatelimitT *p_pb);

int32
pb_compose_tpoam_ratelimit_t_dump(Cdb__ComposeTpoamRatelimitT *p_pb, char *out);

int32
pb_compose_tpoam_ratelimit_t_to_pb_free_packed(Cdb__ComposeTpoamRatelimitT *p_pb);

int32
pb_compose_tpoam_lmm_exec_t_to_pb(tpoam_lmm_exec_t*p_cdb, Cdb__ComposeTpoamLmmExecT *p_pb);

int32
pb_compose_tpoam_lmm_exec_t_dump(Cdb__ComposeTpoamLmmExecT *p_pb, char *out);

int32
pb_compose_tpoam_lmm_exec_t_to_pb_free_packed(Cdb__ComposeTpoamLmmExecT *p_pb);

int32
pb_compose_tpoam_lb_exec_t_to_pb(tpoam_lb_exec_t*p_cdb, Cdb__ComposeTpoamLbExecT *p_pb);

int32
pb_compose_tpoam_lb_exec_t_dump(Cdb__ComposeTpoamLbExecT *p_pb, char *out);

int32
pb_compose_tpoam_lb_exec_t_to_pb_free_packed(Cdb__ComposeTpoamLbExecT *p_pb);

int32
pb_compose_tpoam_1dm_dmm_exec_t_to_pb(tpoam_1dm_dmm_exec_t*p_cdb, Cdb__ComposeTpoam1dmDmmExecT *p_pb);

int32
pb_compose_tpoam_1dm_dmm_exec_t_dump(Cdb__ComposeTpoam1dmDmmExecT *p_pb, char *out);

int32
pb_compose_tpoam_1dm_dmm_exec_t_to_pb_free_packed(Cdb__ComposeTpoam1dmDmmExecT *p_pb);

int32
pb_compose_upload_tpoam_dm_t_to_pb(upload_tpoam_dm_t*p_cdb, Cdb__ComposeUploadTpoamDmT *p_pb);

int32
pb_compose_upload_tpoam_dm_t_dump(Cdb__ComposeUploadTpoamDmT *p_pb, char *out);

int32
pb_compose_upload_tpoam_dm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamDmT *p_pb);

int32
pb_compose_upload_tpoam_lm_t_to_pb(upload_tpoam_lm_t*p_cdb, Cdb__ComposeUploadTpoamLmT *p_pb);

int32
pb_compose_upload_tpoam_lm_t_dump(Cdb__ComposeUploadTpoamLmT *p_pb, char *out);

int32
pb_compose_upload_tpoam_lm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamLmT *p_pb);

int32
pb_compose_upload_tpoam_ccm_t_to_pb(upload_tpoam_ccm_t*p_cdb, Cdb__ComposeUploadTpoamCcmT *p_pb);

int32
pb_compose_upload_tpoam_ccm_t_dump(Cdb__ComposeUploadTpoamCcmT *p_pb, char *out);

int32
pb_compose_upload_tpoam_ccm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamCcmT *p_pb);

int32
pb_compose_glb_board_type_t_to_pb(glb_board_type_t *p_cdb, Cdb__ComposeGlbBoardTypeT *p_pb);

int32
pb_compose_glb_board_type_t_dump(Cdb__ComposeGlbBoardTypeT *p_pb, char *out);

int32
pb_compose_glb_board_type_t_to_pb_free_packed(Cdb__ComposeGlbBoardTypeT *p_pb);

int32
pb_compose_time_range_value_t_to_pb(time_range_value_t *p_cdb, Cdb__ComposeTimeRangeValueT *p_pb);

int32
pb_compose_time_range_value_t_dump(Cdb__ComposeTimeRangeValueT *p_pb, char *out);

int32
pb_compose_time_range_value_t_to_pb_free_packed(Cdb__ComposeTimeRangeValueT *p_pb);

int32
pb_compose_time_range_timer_t_to_pb(time_range_timer_t *p_cdb, Cdb__ComposeTimeRangeTimerT *p_pb);

int32
pb_compose_time_range_timer_t_dump(Cdb__ComposeTimeRangeTimerT *p_pb, char *out);

int32
pb_compose_time_range_timer_t_to_pb_free_packed(Cdb__ComposeTimeRangeTimerT *p_pb);

int32
pb_compose_snmp_trap_key_t_to_pb(snmp_trap_key_t *p_cdb, Cdb__ComposeSnmpTrapKeyT *p_pb);

int32
pb_compose_snmp_trap_key_t_dump(Cdb__ComposeSnmpTrapKeyT *p_pb, char *out);

int32
pb_compose_snmp_trap_key_t_to_pb_free_packed(Cdb__ComposeSnmpTrapKeyT *p_pb);

int32
pb_compose_snmp_view_key_t_to_pb(snmp_view_key_t *p_cdb, Cdb__ComposeSnmpViewKeyT *p_pb);

int32
pb_compose_snmp_view_key_t_dump(Cdb__ComposeSnmpViewKeyT *p_pb, char *out);

int32
pb_compose_snmp_view_key_t_to_pb_free_packed(Cdb__ComposeSnmpViewKeyT *p_pb);

int32
pb_compose_snmp_group_usm_key_t_to_pb(snmp_group_usm_key_t *p_cdb, Cdb__ComposeSnmpGroupUsmKeyT *p_pb);

int32
pb_compose_snmp_group_usm_key_t_dump(Cdb__ComposeSnmpGroupUsmKeyT *p_pb, char *out);

int32
pb_compose_snmp_group_usm_key_t_to_pb_free_packed(Cdb__ComposeSnmpGroupUsmKeyT *p_pb);

int32
pb_compose_snmp_inform_key_t_to_pb(snmp_inform_key_t *p_cdb, Cdb__ComposeSnmpInformKeyT *p_pb);

int32
pb_compose_snmp_inform_key_t_dump(Cdb__ComposeSnmpInformKeyT *p_pb, char *out);

int32
pb_compose_snmp_inform_key_t_to_pb_free_packed(Cdb__ComposeSnmpInformKeyT *p_pb);

int32
pb_compose_rsa_keystr_t_to_pb(rsa_keystr_t *p_cdb, Cdb__ComposeRsaKeystrT *p_pb);

int32
pb_compose_rsa_keystr_t_dump(Cdb__ComposeRsaKeystrT *p_pb, char *out);

int32
pb_compose_rsa_keystr_t_to_pb_free_packed(Cdb__ComposeRsaKeystrT *p_pb);

int32
pb_compose_timestamp_t_to_pb(timestamp_t *p_cdb, Cdb__ComposeTimestampT *p_pb);

int32
pb_compose_timestamp_t_dump(Cdb__ComposeTimestampT *p_pb, char *out);

int32
pb_compose_timestamp_t_to_pb_free_packed(Cdb__ComposeTimestampT *p_pb);

int32
pb_compose_mail_t_to_pb(mail_t *p_cdb, Cdb__ComposeMailT *p_pb);

int32
pb_compose_mail_t_dump(Cdb__ComposeMailT *p_pb, char *out);

int32
pb_compose_mail_t_to_pb_free_packed(Cdb__ComposeMailT *p_pb);

int32
pb_compose_mstp_brg_id_t_to_pb(mstp_brg_id_t *p_cdb, Cdb__ComposeMstpBrgIdT *p_pb);

int32
pb_compose_mstp_brg_id_t_dump(Cdb__ComposeMstpBrgIdT *p_pb, char *out);

int32
pb_compose_mstp_brg_id_t_to_pb_free_packed(Cdb__ComposeMstpBrgIdT *p_pb);

int32
pb_compose_glb_stats_t_to_pb(glb_stats_t *p_cdb, Cdb__ComposeGlbStatsT *p_pb);

int32
pb_compose_glb_stats_t_dump(Cdb__ComposeGlbStatsT *p_pb, char *out);

int32
pb_compose_glb_stats_t_to_pb_free_packed(Cdb__ComposeGlbStatsT *p_pb);

int32
pb_compose_glb_flow_l4_port_t_to_pb(glb_flow_l4_port_t *p_cdb, Cdb__ComposeGlbFlowL4PortT *p_pb);

int32
pb_compose_glb_flow_l4_port_t_dump(Cdb__ComposeGlbFlowL4PortT *p_pb, char *out);

int32
pb_compose_glb_flow_l4_port_t_to_pb_free_packed(Cdb__ComposeGlbFlowL4PortT *p_pb);

int32
pb_compose_glb_flow_tcp_flag_t_to_pb(glb_flow_tcp_flag_t *p_cdb, Cdb__ComposeGlbFlowTcpFlagT *p_pb);

int32
pb_compose_glb_flow_tcp_flag_t_dump(Cdb__ComposeGlbFlowTcpFlagT *p_pb, char *out);

int32
pb_compose_glb_flow_tcp_flag_t_to_pb_free_packed(Cdb__ComposeGlbFlowTcpFlagT *p_pb);

int32
pb_compose_sal_time_t_to_pb(sal_time_t *p_cdb, Cdb__ComposeSalTimeT *p_pb);

int32
pb_compose_sal_time_t_dump(Cdb__ComposeSalTimeT *p_pb, char *out);

int32
pb_compose_sal_time_t_to_pb_free_packed(Cdb__ComposeSalTimeT *p_pb);

int32
pb_compose_prefix_t_to_pb(prefix_t *p_cdb, Cdb__ComposePrefixT *p_pb);

int32
pb_compose_prefix_t_dump(Cdb__ComposePrefixT *p_pb, char *out);

int32
pb_compose_prefix_t_to_pb_free_packed(Cdb__ComposePrefixT *p_pb);

int32
pb_compose_prefix_ipv4_t_to_pb(prefix_ipv4_t *p_cdb, Cdb__ComposePrefixIpv4T *p_pb);

int32
pb_compose_prefix_ipv4_t_dump(Cdb__ComposePrefixIpv4T *p_pb, char *out);

int32
pb_compose_prefix_ipv4_t_to_pb_free_packed(Cdb__ComposePrefixIpv4T *p_pb);

int32
pb_compose_addr_t_to_pb(addr_t *p_cdb, Cdb__ComposeAddrT *p_pb);

int32
pb_compose_addr_t_dump(Cdb__ComposeAddrT *p_pb, char *out);

int32
pb_compose_addr_t_to_pb_free_packed(Cdb__ComposeAddrT *p_pb);

int32
pb_compose_snmp_info_t_to_pb(snmp_info_t *p_cdb, Cdb__ComposeSnmpInfoT *p_pb);

int32
pb_compose_snmp_info_t_dump(Cdb__ComposeSnmpInfoT *p_pb, char *out);

int32
pb_compose_snmp_info_t_to_pb_free_packed(Cdb__ComposeSnmpInfoT *p_pb);

int32
pb_compose_user_t_to_pb(user_t *p_cdb, Cdb__ComposeUserT *p_pb);

int32
pb_compose_user_t_dump(Cdb__ComposeUserT *p_pb, char *out);

int32
pb_compose_user_t_to_pb_free_packed(Cdb__ComposeUserT *p_pb);


int32
pb_compose_addr_ipv4_t_to_pb(addr_ipv4_t *p_cdb, Cdb__ComposeAddrIpv4T *p_pb);

int32
pb_compose_addr_ipv4_t_dump(Cdb__ComposeAddrIpv4T *p_pb, char *out);

int32
pb_compose_addr_ipv4_t_to_pb_free_packed(Cdb__ComposeAddrIpv4T *p_pb);

int32
pb_compose_addr_ipv6_t_to_pb(addr_ipv6_t *p_cdb, Cdb__ComposeAddrIpv6T *p_pb);

int32
pb_compose_addr_ipv6_t_dump(Cdb__ComposeAddrIpv6T *p_pb, char *out);

int32
pb_compose_addr_ipv6_t_to_pb_free_packed(Cdb__ComposeAddrIpv6T *p_pb);

int32
pb_compose_nexthop_key_t_to_pb(nexthop_key_t *p_cdb, Cdb__ComposeNexthopKeyT *p_pb);

int32
pb_compose_nexthop_key_t_dump(Cdb__ComposeNexthopKeyT *p_pb, char *out);

int32
pb_compose_nexthop_key_t_to_pb_free_packed(Cdb__ComposeNexthopKeyT *p_pb);

int32
pb_compose_nexthop_ecmp_t_to_pb(nexthop_ecmp_t *p_cdb, Cdb__ComposeNexthopEcmpT *p_pb);

int32
pb_compose_nexthop_ecmp_t_dump(Cdb__ComposeNexthopEcmpT *p_pb, char *out);

int32
pb_compose_nexthop_ecmp_t_to_pb_free_packed(Cdb__ComposeNexthopEcmpT *p_pb);

int32
pb_compose_nexthop_ecmp_routed_t_to_pb(nexthop_ecmp_routed_t *p_cdb, Cdb__ComposeNexthopEcmpRoutedT *p_pb);

int32
pb_compose_nexthop_ecmp_routed_t_dump(Cdb__ComposeNexthopEcmpRoutedT *p_pb, char *out);

int32
pb_compose_nexthop_ecmp_routed_t_to_pb_free_packed(Cdb__ComposeNexthopEcmpRoutedT *p_pb);

int32
pb_compose_ace_config_key_t_to_pb(ace_config_key_t *p_cdb, Cdb__ComposeAceConfigKeyT *p_pb);

int32
pb_compose_ace_config_key_t_dump(Cdb__ComposeAceConfigKeyT *p_pb, char *out);

int32
pb_compose_ace_config_key_t_to_pb_free_packed(Cdb__ComposeAceConfigKeyT *p_pb);

int32
pb_compose_lldp_msap_id_t_to_pb(lldp_msap_id_t *p_cdb, Cdb__ComposeLldpMsapIdT *p_pb);

int32
pb_compose_lldp_msap_id_t_dump(Cdb__ComposeLldpMsapIdT *p_pb, char *out);

int32
pb_compose_lldp_msap_id_t_to_pb_free_packed(Cdb__ComposeLldpMsapIdT *p_pb);

int32
pb_compose_ptp_foreign_master_t_to_pb(ptp_foreign_master_t *p_cdb, Cdb__ComposePtpForeignMasterT *p_pb);

int32
pb_compose_ptp_foreign_master_t_dump(Cdb__ComposePtpForeignMasterT *p_pb, char *out);

int32
pb_compose_ptp_foreign_master_t_to_pb_free_packed(Cdb__ComposePtpForeignMasterT *p_pb);

int32
pb_compose_ptp_residence_time_cache_t_to_pb(ptp_residence_time_cache_t *p_cdb, Cdb__ComposePtpResidenceTimeCacheT *p_pb);

int32
pb_compose_ptp_residence_time_cache_t_dump(Cdb__ComposePtpResidenceTimeCacheT *p_pb, char *out);

int32
pb_compose_ptp_residence_time_cache_t_to_pb_free_packed(Cdb__ComposePtpResidenceTimeCacheT *p_pb);

int32
pb_compose_erps_ring_key_t_to_pb(erps_ring_key_t *p_cdb, Cdb__ComposeErpsRingKeyT *p_pb);

int32
pb_compose_erps_ring_key_t_dump(Cdb__ComposeErpsRingKeyT *p_pb, char *out);

int32
pb_compose_erps_ring_key_t_to_pb_free_packed(Cdb__ComposeErpsRingKeyT *p_pb);

int32
pb_compose_class_in_policy_config_key_t_to_pb(class_in_policy_config_key_t *p_cdb, Cdb__ComposeClassInPolicyConfigKeyT *p_pb);

int32
pb_compose_class_in_policy_config_key_t_dump(Cdb__ComposeClassInPolicyConfigKeyT *p_pb, char *out);

int32
pb_compose_class_in_policy_config_key_t_to_pb_free_packed(Cdb__ComposeClassInPolicyConfigKeyT *p_pb);

int32
pb_compose_acl_in_class_config_key_t_to_pb(acl_in_class_config_key_t *p_cdb, Cdb__ComposeAclInClassConfigKeyT *p_pb);

int32
pb_compose_acl_in_class_config_key_t_dump(Cdb__ComposeAclInClassConfigKeyT *p_pb, char *out);

int32
pb_compose_acl_in_class_config_key_t_to_pb_free_packed(Cdb__ComposeAclInClassConfigKeyT *p_pb);

int32
pb_compose_class_map_action_config_key_t_to_pb(class_map_action_config_key_t *p_cdb, Cdb__ComposeClassMapActionConfigKeyT *p_pb);

int32
pb_compose_class_map_action_config_key_t_dump(Cdb__ComposeClassMapActionConfigKeyT *p_pb, char *out);

int32
pb_compose_class_map_action_config_key_t_to_pb_free_packed(Cdb__ComposeClassMapActionConfigKeyT *p_pb);

int32
pb_compose_auth_server_key_t_to_pb(auth_server_key_t *p_cdb, Cdb__ComposeAuthServerKeyT *p_pb);

int32
pb_compose_auth_server_key_t_dump(Cdb__ComposeAuthServerKeyT *p_pb, char *out);

int32
pb_compose_auth_server_key_t_to_pb_free_packed(Cdb__ComposeAuthServerKeyT *p_pb);

int32
pb_compose_auth_session_key_t_to_pb(auth_session_key_t *p_cdb, Cdb__ComposeAuthSessionKeyT *p_pb);

int32
pb_compose_auth_session_key_t_dump(Cdb__ComposeAuthSessionKeyT *p_pb, char *out);

int32
pb_compose_auth_session_key_t_to_pb_free_packed(Cdb__ComposeAuthSessionKeyT *p_pb);

int32
pb_compose_lldp_civic_address_t_to_pb(lldp_civic_address_t *p_cdb, Cdb__ComposeLldpCivicAddressT *p_pb);

int32
pb_compose_lldp_civic_address_t_dump(Cdb__ComposeLldpCivicAddressT *p_pb, char *out);

int32
pb_compose_lldp_civic_address_t_to_pb_free_packed(Cdb__ComposeLldpCivicAddressT *p_pb);

int32
pb_compose_igsp_group_key_t_to_pb(igsp_group_key_t *p_cdb, Cdb__ComposeIgspGroupKeyT *p_pb);

int32
pb_compose_igsp_group_key_t_dump(Cdb__ComposeIgspGroupKeyT *p_pb, char *out);

int32
pb_compose_igsp_group_key_t_to_pb_free_packed(Cdb__ComposeIgspGroupKeyT *p_pb);

int32
pb_compose_fea_g8032_ring_key_t_to_pb(fea_g8032_ring_key_t *p_cdb, Cdb__ComposeFeaG8032RingKeyT *p_pb);

int32
pb_compose_fea_g8032_ring_key_t_dump(Cdb__ComposeFeaG8032RingKeyT *p_pb, char *out);


int32
pb_compose_fea_g8032_ring_key_t_to_pb_free_packed(Cdb__ComposeFeaG8032RingKeyT *p_pb);

int32
pb_compose_dot1x_radius_key_t_to_pb(dot1x_radius_key_t *p_cdb, Cdb__ComposeDot1xRadiusKeyT *p_pb);

int32
pb_compose_dot1x_radius_key_t_dump(Cdb__ComposeDot1xRadiusKeyT *p_pb, char *out);

int32
pb_compose_dot1x_radius_key_t_to_pb_free_packed(Cdb__ComposeDot1xRadiusKeyT *p_pb);

int32
pb_compose_sflow_collector_key_t_to_pb(sflow_collector_key_t *p_cdb, Cdb__ComposeSflowCollectorKeyT *p_pb);

int32
pb_compose_sflow_collector_key_t_dump(Cdb__ComposeSflowCollectorKeyT *p_pb, char *out);

int32
pb_compose_sflow_collector_key_t_to_pb_free_packed(Cdb__ComposeSflowCollectorKeyT *p_pb);

int32
pb_compose_dot1x_mac_key_t_to_pb(dot1x_mac_key_t *p_cdb, Cdb__ComposeDot1xMacKeyT *p_pb);

int32
pb_compose_dot1x_mac_key_t_dump(Cdb__ComposeDot1xMacKeyT *p_pb, char *out);

int32
pb_compose_dot1x_mac_key_t_to_pb_free_packed(Cdb__ComposeDot1xMacKeyT *p_pb);

int32
pb_compose_mirror_mac_escape_key_t_to_pb(mirror_mac_escape_key_t *p_cdb, Cdb__ComposeMirrorMacEscapeKeyT *p_pb);

int32
pb_compose_mirror_mac_escape_key_t_dump(Cdb__ComposeMirrorMacEscapeKeyT *p_pb, char *out);

int32
pb_compose_mirror_mac_escape_key_t_to_pb_free_packed(Cdb__ComposeMirrorMacEscapeKeyT *p_pb);

int32
pb_compose_glb_ptp_port_id_t_to_pb(tbl_ptp_foreign_key_t *p_cdb, Cdb__ComposeGlbPtpPortIdT *p_pb);

int32
pb_compose_glb_ptp_port_id_t_dump(Cdb__ComposeGlbPtpPortIdT *p_pb, char *out);

int32
pb_compose_glb_ptp_port_id_t_to_pb_free_packed(Cdb__ComposeGlbPtpPortIdT *p_pb);

int32
pb_compose_tap_five_tuple_key_t_to_pb(tbl_tap_five_tuple_key_t *p_cdb, Cdb__ComposeTapFiveTupleKeyT *p_pb);

int32
pb_compose_tap_five_tuple_key_t_dump(Cdb__ComposeTapFiveTupleKeyT *p_pb, char *out);

int32
pb_compose_tap_five_tuple_key_t_to_pb_free_packed(Cdb__ComposeTapFiveTupleKeyT *p_pb);

int32
pb_compose_tap_tuple_acl_entry_id_t_to_pb(tap_tuple_acl_entry_id_t *p_cdb, Cdb__ComposeTapTupleAclEntryIdT *p_pb);

int32
pb_compose_tap_tuple_acl_entry_id_t_dump(Cdb__ComposeTapTupleAclEntryIdT *p_pb, char *out);

int32
pb_compose_tap_tuple_acl_entry_id_t_to_pb_free_packed(Cdb__ComposeTapTupleAclEntryIdT *p_pb);

int32
pb_compose_tap_group_ingress_key_t_to_pb(tbl_tap_group_ingress_key_t *p_cdb, Cdb__ComposeTapGroupIngressKeyT *p_pb);

int32
pb_compose_tap_group_ingress_key_t_dump(Cdb__ComposeTapGroupIngressKeyT *p_pb, char *out);

int32
pb_compose_tap_group_ingress_key_t_to_pb_free_packed(Cdb__ComposeTapGroupIngressKeyT *p_pb);

int32
pb_compose_tap_group_ingress_flow_key_t_to_pb(tbl_tap_group_ingress_flow_key_t *p_cdb, Cdb__ComposeTapGroupIngressFlowKeyT *p_pb);

int32
pb_compose_tap_group_ingress_flow_key_t_dump(Cdb__ComposeTapGroupIngressFlowKeyT *p_pb, char *out);

int32
pb_compose_tap_group_ingress_flow_key_t_to_pb_free_packed(Cdb__ComposeTapGroupIngressFlowKeyT *p_pb);

int32
pb_compose_tap_group_egress_key_t_to_pb(tbl_tap_group_egress_key_t *p_cdb, Cdb__ComposeTapGroupEgressKeyT *p_pb);

int32
pb_compose_tap_group_egress_key_t_dump(Cdb__ComposeTapGroupEgressKeyT *p_pb, char *out);

int32
pb_compose_tap_group_egress_key_t_to_pb_free_packed(Cdb__ComposeTapGroupEgressKeyT *p_pb);

int32
pb_compose_inband_snat_key_t_to_pb(inband_snat_key_t *p_cdb, Cdb__ComposeInbandSnatKeyT *p_pb);

int32
pb_compose_inband_snat_key_t_dump(Cdb__ComposeInbandSnatKeyT *p_pb, char *out);

int32
pb_compose_inband_snat_key_t_to_pb_free_packed(Cdb__ComposeInbandSnatKeyT *p_pb);

int32
pb_compose_arpace_config_key_t_to_pb(arpace_config_key_t *p_cdb, Cdb__ComposeArpaceConfigKeyT *p_pb);

int32
pb_compose_arpace_config_key_t_dump(Cdb__ComposeArpaceConfigKeyT *p_pb, char *out);

int32
pb_compose_arpace_config_key_t_to_pb_free_packed(Cdb__ComposeArpaceConfigKeyT *p_pb);

int32
pb_compose_ssm_clock_id_t_to_pb(ssm_clock_id_t *p_cdb, Cdb__ComposeSsmClockIdT *p_pb);

int32
pb_compose_ssm_clock_id_t_dump(Cdb__ComposeSsmClockIdT *p_pb, char *out);

int32
pb_compose_ssm_clock_id_t_to_pb_free_packed(Cdb__ComposeSsmClockIdT *p_pb);

int32
pb_compose_rmon_stats_history_key_t_to_pb(rmon_stats_history_key_t *p_cdb, Cdb__ComposeRmonStatsHistoryKeyT *p_pb);

int32
pb_compose_rmon_stats_history_key_t_dump(Cdb__ComposeRmonStatsHistoryKeyT *p_pb, char *out);

int32
pb_compose_rmon_stats_history_key_t_to_pb_free_packed(Cdb__ComposeRmonStatsHistoryKeyT *p_pb);

int32
pb_compose_ipsla_packet_stats_key_t_to_pb(ipsla_packet_stats_key_t *p_cdb, Cdb__ComposeIpslaPacketStatsKeyT *p_pb);

int32
pb_compose_ipsla_packet_stats_key_t_dump(Cdb__ComposeIpslaPacketStatsKeyT *p_pb, char *out);

int32
pb_compose_ipsla_packet_stats_key_t_to_pb_free_packed(Cdb__ComposeIpslaPacketStatsKeyT *p_pb);

int32
pb_compose_ipsla_test_stats_key_t_to_pb(ipsla_test_stats_key_t *p_cdb, Cdb__ComposeIpslaTestStatsKeyT *p_pb);

int32
pb_compose_ipsla_test_stats_key_t_dump(Cdb__ComposeIpslaTestStatsKeyT *p_pb, char *out);

int32
pb_compose_ipsla_test_stats_key_t_to_pb_free_packed(Cdb__ComposeIpslaTestStatsKeyT *p_pb);

int32
pb_compose_mrt_key_t_to_pb(mrt_key_t *p_cdb, Cdb__ComposeMrtKeyT *p_pb);

int32
pb_compose_mrt_key_t_dump(Cdb__ComposeMrtKeyT *p_pb, char *out);

int32
pb_compose_mrt_key_t_to_pb_free_packed(Cdb__ComposeMrtKeyT *p_pb);

int32
pb_compose_vlan_mapping_entry_key_t_to_pb(vlan_mapping_entry_key_t *p_cdb, Cdb__ComposeVlanMappingEntryKeyT *p_pb);

int32
pb_compose_vlan_mapping_entry_key_t_dump(Cdb__ComposeVlanMappingEntryKeyT *p_pb, char *out);

int32
pb_compose_vlan_mapping_entry_key_t_to_pb_free_packed(Cdb__ComposeVlanMappingEntryKeyT *p_pb);

int32
pb_compose_egress_mapping_entry_key_t_to_pb(egress_mapping_entry_key_t *p_cdb, Cdb__ComposeEgressMappingEntryKeyT *p_pb);

int32
pb_compose_egress_mapping_entry_key_t_dump(Cdb__ComposeEgressMappingEntryKeyT *p_pb, char *out);

int32
pb_compose_egress_mapping_entry_key_t_to_pb_free_packed(Cdb__ComposeEgressMappingEntryKeyT *p_pb);

int32
pb_compose_mapped_vlan_info_key_t_to_pb(mapped_vlan_info_key_t *p_cdb, Cdb__ComposeMappedVlanInfoKeyT *p_pb);

int32
pb_compose_mapped_vlan_info_key_t_dump(Cdb__ComposeMappedVlanInfoKeyT *p_pb, char *out);

int32
pb_compose_mapped_vlan_info_key_t_to_pb_free_packed(Cdb__ComposeMappedVlanInfoKeyT *p_pb);

int32
pb_compose_nd_prefix_key_t_to_pb(nd_prefix_key_t *p_cdb, Cdb__ComposeNdPrefixKeyT *p_pb);

int32
pb_compose_nd_prefix_key_t_dump(Cdb__ComposeNdPrefixKeyT *p_pb, char *out);

int32
pb_compose_nd_prefix_key_t_to_pb_free_packed(Cdb__ComposeNdPrefixKeyT *p_pb);

int32
pb_compose_l2pro_if_action_vlan_t_to_pb(l2pro_if_action_vlan_t *p_cdb, Cdb__ComposeL2proIfActionVlanT *p_pb);

int32
pb_compose_l2pro_if_action_vlan_t_dump(Cdb__ComposeL2proIfActionVlanT *p_pb, char *out);

int32
pb_compose_l2pro_if_action_vlan_t_to_pb_free_packed(Cdb__ComposeL2proIfActionVlanT *p_pb);

int32
pb_compose_glb_qos_color_action_t_to_pb(glb_qos_color_action_t *p_cdb, Cdb__ComposeGlbQosColorActionT *p_pb);

int32
pb_compose_glb_qos_color_action_t_dump(Cdb__ComposeGlbQosColorActionT *p_pb, char *out);

int32
pb_compose_glb_qos_color_action_t_to_pb_free_packed(Cdb__ComposeGlbQosColorActionT *p_pb);

int32
pb_compose_qos_policer_res_key_t_to_pb(qos_policer_res_key_t *p_cdb, Cdb__ComposeQosPolicerResKeyT *p_pb);

int32
pb_compose_qos_policer_res_key_t_dump(Cdb__ComposeQosPolicerResKeyT *p_pb, char *out);

int32
pb_compose_qos_policer_res_key_t_to_pb_free_packed(Cdb__ComposeQosPolicerResKeyT *p_pb);

int32
pb_compose_l2edit_key_t_to_pb(l2edit_key_t *p_cdb, Cdb__ComposeL2editKeyT *p_pb);

int32
pb_compose_l2edit_key_t_dump(Cdb__ComposeL2editKeyT *p_pb, char *out);

int32
pb_compose_l2edit_key_t_to_pb_free_packed(Cdb__ComposeL2editKeyT *p_pb);

int32
pb_compose_nat_session_key_t_to_pb(nat_session_key_t *p_cdb, Cdb__ComposeNatSessionKeyT *p_pb);

int32
pb_compose_nat_session_key_t_dump(Cdb__ComposeNatSessionKeyT *p_pb, char *out);

int32
pb_compose_nat_session_key_t_to_pb_free_packed(Cdb__ComposeNatSessionKeyT *p_pb);

int32
pb_compose_glb_poe_cfg_t_to_pb(glb_poe_cfg_t *p_cdb, Cdb__ComposeGlbPoeCfgT *p_pb);

int32
pb_compose_glb_poe_cfg_t_dump(Cdb__ComposeGlbPoeCfgT *p_pb, char *out);

int32
pb_compose_glb_poe_cfg_t_to_pb_free_packed(Cdb__ComposeGlbPoeCfgT *p_pb);

int32
pb_compose_nat_rule_key_t_to_pb(nat_rule_key_t *p_cdb, Cdb__ComposeNatRuleKeyT *p_pb);

int32
pb_compose_nat_rule_key_t_dump(Cdb__ComposeNatRuleKeyT *p_pb, char *out);

int32
pb_compose_nat_rule_key_t_to_pb_free_packed(Cdb__ComposeNatRuleKeyT *p_pb);

int32
pb_compose_ctclib_opb_t_to_pb(ctclib_opb_t *p_cdb, Cdb__ComposeCtclibOpbT *p_pb);

int32
pb_compose_ctclib_opb_t_dump(Cdb__ComposeCtclibOpbT *p_pb, char *out);

int32
pb_compose_ctclib_opb_t_to_pb_free_packed(Cdb__ComposeCtclibOpbT *p_pb);

int32
pb_compose_nat_proxy_arp_key_t_to_pb(nat_proxy_arp_key_t *p_cdb, Cdb__ComposeNatProxyArpKeyT *p_pb);

int32
pb_compose_nat_proxy_arp_key_t_dump(Cdb__ComposeNatProxyArpKeyT *p_pb, char *out);

int32
pb_compose_nat_proxy_arp_key_t_to_pb_free_packed(Cdb__ComposeNatProxyArpKeyT *p_pb);

int32
pb_compose_cfm_ma_key_t_to_pb(cfm_ma_key_t *p_cdb, Cdb__ComposeCfmMaKeyT *p_pb);

int32
pb_compose_cfm_ma_key_t_dump(Cdb__ComposeCfmMaKeyT *p_pb, char *out);

int32
pb_compose_cfm_ma_key_t_to_pb_free_packed(Cdb__ComposeCfmMaKeyT *p_pb);

int32
pb_compose_cfm_lmep_key_t_to_pb(cfm_lmep_key_t *p_cdb, Cdb__ComposeCfmLmepKeyT *p_pb);

int32
pb_compose_cfm_lmep_key_t_dump(Cdb__ComposeCfmLmepKeyT *p_pb, char *out);

int32
pb_compose_cfm_lmep_key_t_to_pb_free_packed(Cdb__ComposeCfmLmepKeyT *p_pb);

int32
pb_compose_cfm_rmep_key_t_to_pb(cfm_rmep_key_t *p_cdb, Cdb__ComposeCfmRmepKeyT *p_pb);

int32
pb_compose_cfm_rmep_key_t_dump(Cdb__ComposeCfmRmepKeyT *p_pb, char *out);

int32
pb_compose_cfm_rmep_key_t_to_pb_free_packed(Cdb__ComposeCfmRmepKeyT *p_pb);

int32
pb_compose_cfm_time_key_t_to_pb(cfm_time_key_t *p_cdb, Cdb__ComposeCfmTimeKeyT *p_pb);

int32
pb_compose_cfm_time_key_t_dump(Cdb__ComposeCfmTimeKeyT *p_pb, char *out);

int32
pb_compose_cfm_time_key_t_to_pb_free_packed(Cdb__ComposeCfmTimeKeyT *p_pb);

int32
pb_compose_pim_sm_gprefix_t_to_pb(pim_sm_gprefix_t *p_cdb, Cdb__ComposePimSmGprefixT *p_pb);

int32
pb_compose_pim_sm_gprefix_t_dump(Cdb__ComposePimSmGprefixT *p_pb, char *out);

int32
pb_compose_pim_sm_gprefix_t_to_pb_free_packed(Cdb__ComposePimSmGprefixT *p_pb);

int32
pb_compose_pim_static_rp_addr_t_to_pb(pim_static_rp_addr_t *p_cdb, Cdb__ComposePimStaticRpAddrT *p_pb);

int32
pb_compose_pim_static_rp_addr_t_dump(Cdb__ComposePimStaticRpAddrT *p_pb, char *out);

int32
pb_compose_pim_static_rp_addr_t_to_pb_free_packed(Cdb__ComposePimStaticRpAddrT *p_pb);
#endif  /* __PB_COMMON_H__ */

