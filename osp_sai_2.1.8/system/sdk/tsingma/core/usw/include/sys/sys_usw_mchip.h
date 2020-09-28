/**
 @file sys_usw_mchip.h

 @date 2009-10-19

 @version v2.0

 The file contains chip independent related APIs of sys layer
*/

#ifndef _SYS_USW_MCHIP_H
#define _SYS_USW_MCHIP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_parser.h"
#include "sys_usw_mac.h"
#include "sys_usw_datapath.h"
#include "sys_usw_ftm.h"

enum sys_mchip_capability_e
{
    SYS_CAP_SPEC_LOGIC_PORT_NUM = 0,
    SYS_CAP_SPEC_MAX_FID,
    SYS_CAP_SPEC_MAX_VRFID,
    SYS_CAP_SPEC_MCAST_GROUP_NUM,
    SYS_CAP_SPEC_VLAN_NUM,
    SYS_CAP_SPEC_VLAN_RANGE_GROUP_NUM,
    SYS_CAP_SPEC_STP_INSTANCE_NUM,
    SYS_CAP_SPEC_LINKAGG_GROUP_NUM,
    SYS_CAP_SPEC_LINKAGG_MEMBER_NUM,
    SYS_CAP_SPEC_LINKAGG_DLB_FLOW_NUM,
    SYS_CAP_SPEC_LINKAGG_DLB_MEMBER_NUM,
    SYS_CAP_SPEC_LINKAGG_DLB_GROUP_NUM,
    SYS_CAP_SPEC_ECMP_GROUP_NUM,
    SYS_CAP_SPEC_ECMP_MEMBER_NUM,
    SYS_CAP_SPEC_ECMP_DLB_FLOW_NUM,
    SYS_CAP_SPEC_EXTERNAL_NEXTHOP_NUM,
    SYS_CAP_SPEC_GLOBAL_DSNH_NUM,
    SYS_CAP_SPEC_MPLS_TUNNEL_NUM,
    SYS_CAP_SPEC_ARP_ID_NUM,
    SYS_CAP_SPEC_L3IF_NUM,
    SYS_CAP_SPEC_OAM_SESSION_NUM,
    SYS_CAP_SPEC_NPM_SESSION_NUM,
    SYS_CAP_SPEC_APS_GROUP_NUM,
    SYS_CAP_SPEC_TOTAL_POLICER_NUM,
    SYS_CAP_SPEC_POLICER_NUM,
    SYS_CAP_SPEC_TOTAL_STATS_NUM,
    SYS_CAP_SPEC_QUEUE_STATS_NUM,
    SYS_CAP_SPEC_POLICER_STATS_NUM,
    SYS_CAP_SPEC_SHARE1_STATS_NUM,
    SYS_CAP_SPEC_SHARE2_STATS_NUM,
    SYS_CAP_SPEC_SHARE3_STATS_NUM,
    SYS_CAP_SPEC_SHARE4_STATS_NUM,
    SYS_CAP_SPEC_ACL0_IGS_STATS_NUM,
    SYS_CAP_SPEC_ACL1_IGS_STATS_NUM,
    SYS_CAP_SPEC_ACL2_IGS_STATS_NUM,
    SYS_CAP_SPEC_ACL3_IGS_STATS_NUM,
    SYS_CAP_SPEC_ACL0_EGS_STATS_NUM,
    SYS_CAP_SPEC_ECMP_STATS_NUM,
    SYS_CAP_SPEC_ROUTE_MAC_ENTRY_NUM,
    SYS_CAP_SPEC_MAC_ENTRY_NUM,
     /*SYS_CAP_SPEC_BLACK_HOLE_ENTRY_NUM,*/
    SYS_CAP_SPEC_HOST_ROUTE_ENTRY_NUM,
    SYS_CAP_SPEC_LPM_ROUTE_ENTRY_NUM,
    SYS_CAP_SPEC_IPMC_ENTRY_NUM,
    SYS_CAP_SPEC_MPLS_ENTRY_NUM,
    SYS_CAP_SPEC_TUNNEL_ENTRY_NUM,
     /*SYS_CAP_SPEC_L2PDU_L2HDR_PROTO_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L2PDU_MACDA_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L2PDU_MACDA_LOW24_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L2PDU_L2CP_MAX_ACTION_INDEX,*/
     /*SYS_CAP_SPEC_L3PDU_L3HDR_PROTO_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L3PDU_L4PORT_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L3PDU_IPDA_ENTRY_NUM,*/
     /*SYS_CAP_SPEC_L3PDU_MAX_ACTION_INDEX,*/
    SYS_CAP_SPEC_SCL_HASH_ENTRY_NUM,
    SYS_CAP_SPEC_SCL1_HASH_ENTRY_NUM,
    SYS_CAP_SPEC_SCL0_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_SCL1_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_SCL2_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_SCL3_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL_HASH_ENTRY_NUM,
    SYS_CAP_SPEC_ACL0_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL1_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL2_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL3_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL4_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL5_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL6_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL7_IGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL0_EGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL1_EGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_ACL2_EGS_TCAM_ENTRY_NUM,
    SYS_CAP_SPEC_IPFIX_ENTRY_NUM,
    SYS_CAP_SPEC_EFD_FLOW_ENTRY_NUM,
    SYS_CAP_SPEC_MAX_LCHIP_NUM,
    SYS_CAP_SPEC_MAX_PHY_PORT_NUM,
    SYS_CAP_SPEC_MAX_PORT_NUM,
    SYS_CAP_SPEC_MAX_CHIP_NUM,
    SYS_CAP_ACL_COMPRESS_ETHER_TYPE_NUM,
    SYS_CAP_ACL_COPP_THRD,
    SYS_CAP_ACL_EGRESS_LKUP,
    SYS_CAP_ACL_EGRESS_LKUP_NUM,
    SYS_CAP_ACL_FIELD_RANGE,
    SYS_CAP_ACL_HASH_AD_INDEX_OFFSET,
    SYS_CAP_ACL_HASH_CID_KEY,
    SYS_CAP_ACL_HASH_SEL_PROFILE,
    SYS_CAP_ACL_INGRESS_ACL_LKUP,
    SYS_CAP_ACL_INGRESS_LKUP_NUM,
    SYS_CAP_ACL_LABEL_NUM,
    SYS_CAP_ACL_MAX_CID_VALUE,
    SYS_CAP_ACL_MAX_SESSION,
    SYS_CAP_ACL_PORT_CLASS_ID_NUM,
    SYS_CAP_ACL_SERVICE_ID_NUM,
    SYS_CAP_ACL_TCAM_CID_PAIR,
    SYS_CAP_ACL_UDF_ENTRY_NUM,
    SYS_CAP_ACL_UDF_OFFSET_MAX,
    SYS_CAP_ACL_VLAN_ACTION_SIZE_PER_BUCKET,
    SYS_CAP_ACL_VLAN_CLASS_ID_NUM,
    SYS_CAP_APS_GROUP_NUM,
    SYS_CAP_AQM_PORT_THRD_HIGN,
    SYS_CAP_AQM_PORT_THRD_LOW,
    SYS_CAP_BFD_INTERVAL_CAM_NUM,
    SYS_CAP_CHANID_DMA_RX0,
    SYS_CAP_CHANID_DMA_RX1,
    SYS_CAP_CHANID_DMA_RX2,
    SYS_CAP_CHANID_DMA_RX3,
    SYS_CAP_CHANID_DROP,
    SYS_CAP_CHANID_ELOG,
    SYS_CAP_CHANID_ELOOP,
    SYS_CAP_CHANID_GMAC_MAX,
    SYS_CAP_CHANID_ILOOP,
    SYS_CAP_CHANID_MAC_DECRYPT,
    SYS_CAP_CHANID_MAC_ENCRYPT,
    SYS_CAP_CHANID_MAX,
    SYS_CAP_CHANID_OAM,
    SYS_CAP_CHANID_QCN,
    SYS_CAP_CHANID_RSV,
    SYS_CAP_CHANID_WLAN_DECRYPT0,
    SYS_CAP_CHANID_WLAN_DECRYPT1,
    SYS_CAP_CHANID_WLAN_DECRYPT2,
    SYS_CAP_CHANID_WLAN_DECRYPT3,
    SYS_CAP_CHANID_WLAN_ENCRYPT0,
    SYS_CAP_CHANID_WLAN_ENCRYPT1,
    SYS_CAP_CHANID_WLAN_ENCRYPT2,
    SYS_CAP_CHANID_WLAN_ENCRYPT3,
    SYS_CAP_CHANID_WLAN_REASSEMBLE,
    SYS_CAP_CHANNEL_NUM,
    SYS_CAP_CPU_OAM_EXCP_NUM,
    SYS_CAP_CPU_REASON_OAM_DEFECT_MESSAGE_BASE,
    SYS_CAP_DLB_MEMBER_NUM,
    SYS_CAP_DOT1AE_AN_NUM,
    SYS_CAP_DOT1AE_DIVISION_WIDE,
    SYS_CAP_DOT1AE_RX_CHAN_NUM,
    SYS_CAP_DOT1AE_SEC_CHAN_NUM,
    SYS_CAP_DOT1AE_SHIFT_WIDE,
    SYS_CAP_DOT1AE_TX_CHAN_NUM,
    SYS_CAP_EFD_FLOW_DETECT_MAX,
    SYS_CAP_EFD_FLOW_STATS,
    SYS_CAP_EPEDISCARDTYPE_DS_ACL_DIS,
    SYS_CAP_EPEDISCARDTYPE_DS_PLC_DIS,
    SYS_CAP_FID_NUM,
    SYS_CAP_FLOW_HASH_LEVEL_NUM,
    SYS_CAP_GCHIP_CHIP_ID,
    SYS_CAP_GLB_DEST_PORT_NUM_PER_CHIP,
    SYS_CAP_INTR_NUM,
    SYS_CAP_IP_HASH_LEVEL_NUM,
    SYS_CAP_IPFIX_MAX_HASH_SEL_ID,
    SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE,
    SYS_CAP_IPFIX_MIN_AGING_INTERVAL,
    SYS_CAP_IPFIX_SAMPPLING_PKT_INTERVAL,
    SYS_CAP_ISOLATION_GROUP_NUM,
    SYS_CAP_ISOLATION_ID_MAX,
    SYS_CAP_ISOLATION_ID_NUM,
    SYS_CAP_L2_BLACK_HOLE_ENTRY,
    SYS_CAP_L2_FDB_CAM_NUM,
    SYS_CAP_L2PDU_BASED_BPDU_ENTRY,
    SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY,
    SYS_CAP_L2PDU_BASED_L3TYPE_ENTRY,
    SYS_CAP_L2PDU_BASED_MACDA_ENTRY,
    SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY,
    SYS_CAP_L2PDU_PER_PORT_ACTION_INDEX,
    SYS_CAP_L2PDU_PORT_ACTION_INDEX,
    SYS_CAP_L3IF_ECMP_GROUP_NUM,
    SYS_CAP_L3IF_INNER_ROUTE_MAC_NUM,
    SYS_CAP_L3IF_ROUTER_MAC_ENTRY_NUM,
    SYS_CAP_L3IF_ROUTER_MAC_NUM,
    SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY,
    SYS_CAP_L3IF_RSV_L3IF_ID,
    SYS_CAP_L3PDU_ACTION_INDEX,
    SYS_CAP_L3PDU_BASED_IPDA,
    SYS_CAP_L3PDU_BASED_L3HDR_PROTO,
    SYS_CAP_L3PDU_BASED_PORT,
    SYS_CAP_L3PDU_L3IF_ACTION_INDEX,
    SYS_CAP_L4_USER_UDP_TYPE_VXLAN,
    SYS_CAP_LEARN_LIMIT_MAX,
    SYS_CAP_LEARN_LIMIT_PROFILE_NUM,
    SYS_CAP_LEARNING_CACHE_MAX_INDEX,
    SYS_CAP_LINKAGG_ALL_MEM_NUM,
    SYS_CAP_LINKAGG_CHAN_ALL_MEM_NUM,
    SYS_CAP_LINKAGG_CHAN_GRP_MAX,
    SYS_CAP_LINKAGG_CHAN_PER_GRP_MEM,
    SYS_CAP_LINKAGG_DYNAMIC_INTERVAL,
    SYS_CAP_LINKAGG_FRAGMENT_SIZE,
    SYS_CAP_LINKAGG_GROUP_NUM,
    SYS_CAP_LINKAGG_MEM_NUM,
    SYS_CAP_LINKAGG_MODE56_DLB_MEM_MAX,
    SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX,
    SYS_CAP_LINKAGG_RR_TID_MAX,
    SYS_CAP_LOCAL_SLICE_NUM,
    SYS_CAP_MAC_HASH_LEVEL_NUM,
    SYS_CAP_MIRROR_ACL_ID,
    SYS_CAP_MIRROR_ACL_LOG_ID,
    SYS_CAP_MIRROR_CPU_RX_SPAN_INDEX,
    SYS_CAP_MIRROR_CPU_TX_SPAN_INDEX,
    SYS_CAP_MIRROR_EGRESS_ACL_LOG_INDEX_BASE,
    SYS_CAP_MIRROR_EGRESS_ACL_LOG_PRIORITY,
    SYS_CAP_MIRROR_EGRESS_IPFIX_LOG_INDEX,
    SYS_CAP_MIRROR_EGRESS_L2_SPAN_INDEX_BASE,
    SYS_CAP_MIRROR_EGRESS_L3_SPAN_INDEX_BASE,
    SYS_CAP_MIRROR_INGRESS_ACL_LOG_INDEX_BASE,
    SYS_CAP_MIRROR_INGRESS_ACL_LOG_PRIORITY,
    SYS_CAP_MIRROR_INGRESS_IPFIX_LOG_INDEX,
    SYS_CAP_MIRROR_INGRESS_L2_SPAN_INDEX_BASE,
    SYS_CAP_MIRROR_INGRESS_L3_SPAN_INDEX_BASE,
    SYS_CAP_MONITOR_BUFFER_RSV_PROF,
    SYS_CAP_MONITOR_DIVISION_WIDE,
    SYS_CAP_MONITOR_LATENCY_MAX_LEVEL,
    SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE,
    SYS_CAP_MONITOR_MAX_CHANNEL,
    SYS_CAP_MONITOR_SHIFT_WIDE,
    SYS_CAP_MONITOR_SYNC_CNT,
    SYS_CAP_MONITOR_BUFFER_MAX_THRD,
    SYS_CAP_MPLS_INNER_ROUTE_MAC_NUM,
    SYS_CAP_MPLS_MAX_LABEL,
    SYS_CAP_MPLS_MAX_LABEL_SPACE,
    SYS_CAP_MPLS_MAX_OAM_CHK_TYPE,
    SYS_CAP_MPLS_MAX_TPID_INDEX,
    SYS_CAP_MPLS_VPLS_SRC_PORT_NUM,
    SYS_CAP_NETWORK_CHANNEL_NUM,
    SYS_CAP_NEXTHOP_MAX_CHIP_NUM,
    SYS_CAP_NEXTHOP_PORT_NUM_PER_CHIP,
    SYS_CAP_NH_CW_NUM,
    SYS_CAP_NH_DROP_DESTMAP,
    SYS_CAP_NH_DSMET_BITMAP_MAX_PORT_ID,
    SYS_CAP_NH_ECMP_GROUP_ID_NUM,
    SYS_CAP_NH_ECMP_MEMBER_NUM,
    SYS_CAP_NH_ECMP_RR_GROUP_NUM,
    SYS_CAP_NH_ILOOP_MAX_REMOVE_WORDS,
    SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM,
    SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM,
    SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM,
    SYS_CAP_NH_L2EDIT_VLAN_PROFILE_NUM,
    SYS_CAP_NPM_SESSION_NUM,
    SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM,
    SYS_CAP_OAM_DEFECT_NUM,
    SYS_CAP_OAM_MEP_ID,
    SYS_CAP_OAM_MEP_NUM_PER_CHAN,
    SYS_CAP_OVERLAY_TUNNEL_IP_INDEX,
    SYS_CAP_PARSER_L2_PROTOCOL_USER_ENTRY,
    SYS_CAP_PARSER_L3_PROTOCOL_USER_ENTRY,
    SYS_CAP_PARSER_L3FLEX_BYTE_SEL,
    SYS_CAP_PARSER_L4_APP_DATA_CTL_ENTRY,
    SYS_CAP_PARSER_UPF_OFFSET,
    SYS_CAP_PER_SLICE_PORT_NUM,
    SYS_CAP_PHY_PORT_NUM_PER_SLICE,
    SYS_CAP_PKT_CPU_QDEST_BY_DMA,
    SYS_CAP_PKT_STRIP_PKT_LEN,
    SYS_CAP_PKT_TRUNCATED_LEN,
    SYS_CAP_PORT_NUM,
    SYS_CAP_PORT_NUM_GLOBAL,
    SYS_CAP_PORT_NUM_PER_CHIP,
    SYS_CAP_PORT_TCAM_TYPE_NUM,
    SYS_CAP_PTP_CAPTURED_RX_SEQ_ID,
    SYS_CAP_PTP_CAPTURED_RX_SOURCE,
    SYS_CAP_PTP_CAPTURED_TX_SEQ_ID,
    SYS_CAP_PTP_FRC_VALUE_SECOND,
    SYS_CAP_PTP_MAX_PTP_ID,
    SYS_CAP_PTP_NS_OR_NNS_VALUE,
    SYS_CAP_PTP_RC_QUANTA,
    SYS_CAP_PTP_SECONDS_OF_EACH_WEEK,
    SYS_CAP_PTP_SYNC_CODE_BIT,
    SYS_CAP_PTP_SYNC_PULSE_FREQUENCY_HZ,
    SYS_CAP_PTP_TAI_TO_GPS_SECONDS,
    SYS_CAP_PTP_TOD_1PPS_DELAY,
    SYS_CAP_PTP_TOD_ADJUSTING_THRESHOLD,
    SYS_CAP_PTP_TOD_ADJUSTING_TOGGLE_STEP,
    SYS_CAP_PTP_TOD_PULSE_HIGH_LEVEL,
    SYS_CAP_PVLAN_COMMUNITY_ID_NUM,
    SYS_CAP_QOS_BASE_QUEUE_GRP_NUM,
    SYS_CAP_QOS_BASE_QUEUE_NUM,
    SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX,
    SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX,
    SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX,
    SYS_CAP_QOS_CLASS_PRIORITY_MAX,
    SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX,
    SYS_CAP_QOS_CONGEST_LEVEL_NUM,
    SYS_CAP_QOS_EXT_GRP_BASE_NUM,
    SYS_CAP_QOS_EXT_QUEUE_GRP_NUM,
    SYS_CAP_QOS_GROUP_NUM,
    SYS_CAP_QOS_GROUP_SHAPE_PROFILE,
    SYS_CAP_QOS_GRP_SHP_CBUCKET_NUM,
    SYS_CAP_QOS_MAX_SHAPE_BURST,
    SYS_CAP_QOS_MIN_SHAPE_BURST,
    SYS_CAP_QOS_MISC_QUEUE_NUM,
    SYS_CAP_QOS_NORMAL_QUEUE_NUM,
    SYS_CAP_QOS_OAM_QUEUE_NUM,
    SYS_CAP_QOS_PHB_OFFSET_NUM,
    SYS_CAP_QOS_POLICER_ACTION_PROFILE_NUM,
    SYS_CAP_QOS_POLICER_CBS,
    SYS_CAP_QOS_POLICER_COPP_PROFILE_NUM,
    SYS_CAP_QOS_POLICER_MFP_PROFILE_NUM,
    SYS_CAP_QOS_POLICER_POLICER_NUM,
    SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES,
    SYS_CAP_QOS_POLICER_PROF_TBL_NUM,
    SYS_CAP_QOS_POLICER_PROFILE_NUM,
    SYS_CAP_QOS_POLICER_RATE_KBPS,
    SYS_CAP_QOS_POLICER_RATE_PPS,
    SYS_CAP_QOS_POLICER_SUPPORTED_FREQ_NUM,
    SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH,
    SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH,
    SYS_CAP_QOS_POLICER_MAX_COS_LEVEL,
    SYS_CAP_QOS_PORT_AQM_FREQ,
    SYS_CAP_QOS_PORT_POLICER_NUM,
    SYS_CAP_QOS_PORT_POLICER_NUM_4Q,
    SYS_CAP_QOS_PORT_POLICER_NUM_8Q,
    SYS_CAP_QOS_QUEUE_BASE_EXCP,
    SYS_CAP_QOS_QUEUE_BASE_EXT,
    SYS_CAP_QOS_QUEUE_BASE_MISC,
    SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC,
    SYS_CAP_QOS_QUEUE_BASE_NORMAL,
    SYS_CAP_QOS_QUEUE_BASE_TYPE_EXCP,
    SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT,
    SYS_CAP_QOS_QUEUE_BASE_TYPE_MISC_CHANNEL,
    SYS_CAP_QOS_QUEUE_BASE_TYPE_NETWORK_MISC,
    SYS_CAP_QOS_QUEUE_BASE_TYPE_NORMAL,
    SYS_CAP_QOS_QUEUE_BUCKET,
    SYS_CAP_QOS_QUEUE_GRP_NUM_FOR_CPU_REASON,
    SYS_CAP_QOS_QUEUE_INVALID_GROUP,
    SYS_CAP_QOS_QUEUE_METER_PROFILE,
    SYS_CAP_QOS_QUEUE_NUM,
    SYS_CAP_QOS_QUEUE_NUM_PER_CHAN,
    SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP,
    SYS_CAP_QOS_QUEUE_NUM_PER_GRP,
    SYS_CAP_QOS_QUEUE_OFFSET_EXT,
    SYS_CAP_QOS_QUEUE_OFFSET_NETWORK,
    SYS_CAP_QOS_QUEUE_PIR_BUCKET,
    SYS_CAP_QOS_QUEUE_SHAPE_PROFILE,
    SYS_CAP_QOS_QUEUE_SHAPE_PIR_PROFILE,
    SYS_CAP_QOS_QUEUE_WEIGHT_BASE,
    SYS_CAP_QOS_QUEUE_MAX_DROP_THRD,
    SYS_CAP_QOS_QUEUE_DROP_WTD_PROFILE_NUM,
    SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM,
    SYS_CAP_QOS_REASON_C2C_MAX_QUEUE_NUM,
    SYS_CAP_QOS_SCHED_MAX_QUE_WEITGHT,
    SYS_CAP_QOS_SCHED_WEIGHT_BASE,
    SYS_CAP_QOS_SCHED_MAX_EXT_QUE_CLASS,
    SYS_CAP_QOS_SHP_BUCKET_CIR_PASS0,
    SYS_CAP_QOS_SHP_BUCKET_CIR_PASS1,
    SYS_CAP_QOS_SHP_BUCKET_PIR,
    SYS_CAP_QOS_SHP_BUCKET_PIR_PASS0,
    SYS_CAP_QOS_SHP_BUCKET_PIR_PASS1,
    SYS_CAP_QOS_SHP_FULL_TOKENS,
    SYS_CAP_QOS_SHP_PPS_PACKET_BYTES,
    SYS_CAP_QOS_SHP_PPS_SHIFT,
    SYS_CAP_QOS_SHP_RATE,
    SYS_CAP_QOS_SHP_RATE_PPS,
    SYS_CAP_QOS_SHP_TOKEN_RATE,
    SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH,
    SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC,
    SYS_CAP_QOS_SHP_TOKEN_THRD,
    SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT,
    SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH,
    SYS_CAP_QOS_SHP_UPDATE_UNIT,
    SYS_CAP_QOS_SHP_WEIGHT_BASE,
    SYS_CAP_QOS_SUB_GRP_NUM_PER_GRP,
    SYS_CAP_QOS_VLAN_POLICER_NUM,
    SYS_CAP_RANDOM_LOG_RATE,
    SYS_CAP_RANDOM_LOG_THRESHOD,
    SYS_CAP_RANDOM_LOG_THRESHOD_PER,
    SYS_CAP_REASON_GRP_QUEUE_NUM,
    SYS_CAP_RPF_IF_NUM,
    SYS_CAP_RPF_PROFILE_NUM,
    SYS_CAP_SCL_ACL_CONTROL_PROFILE,
    SYS_CAP_SCL_AD_FIELD_POS_IS_HALF,
    SYS_CAP_SCL_BY_PASS_VLAN_PTR,
    SYS_CAP_SCL_DS_AD_FIELD_PO_AD_INDEX,
    SYS_CAP_SCL_HASH_SEL_ID,
    SYS_CAP_SCL_LABEL_FOR_IPSG,
    SYS_CAP_SCL_LABEL_FOR_VLAN_CLASS,
    SYS_CAP_SCL_LABEL_NUM,
    SYS_CAP_SCL_TCAM_NUM,
    SYS_CAP_SCL_VLAN_ACTION_RESERVE_NUM,
    SYS_CAP_SCL_FWD_TYPE,
    SYS_CAP_SCL_SERVICE_ID_NUM,
    SYS_CAP_SCL_HASH_NUM,
    SYS_CAP_STATS_ACL0_SIZE,
    SYS_CAP_STATS_ACL1_SIZE,
    SYS_CAP_STATS_ACL2_SIZE,
    SYS_CAP_STATS_ACL3_SIZE,
    SYS_CAP_STATS_CGMAC_RAM_MAX,
    SYS_CAP_STATS_DEQUEUE_SIZE,
    SYS_CAP_STATS_ECMP_RESERVE_SIZE,
    SYS_CAP_STATS_EGS_ACL0_SIZE,
    SYS_CAP_STATS_ENQUEUE_SIZE,
    SYS_CAP_STATS_IPE_FWD_SIZE,
    SYS_CAP_STATS_IPE_IF_SIZE,
    SYS_CAP_STATS_POLICER_SIZE,
    SYS_CAP_STATS_RAM_GLOBAL_SIZE,
    SYS_CAP_STATS_RAM_PRIVATE_SIZE,
    SYS_CAP_STATS_XQMAC_PORT_NUM,
    SYS_CAP_STATS_XQMAC_RAM_NUM,
    SYS_CAP_STATS_DMA_BLOCK_SIZE,
    SYS_CAP_STATS_DMA_BLOCK_NUM,
    SYS_CAP_STK_GLB_DEST_PORT_NUM,
    SYS_CAP_STK_MAX_GCHIP,
    SYS_CAP_STK_MAX_LPORT,
    SYS_CAP_STK_PORT_FWD_PROFILE_NUM,
    SYS_CAP_STK_SGMAC_GROUP_NUM,
    SYS_CAP_STK_TRUNK_DLB_MAX_MEMBERS,
    SYS_CAP_STK_TRUNK_STATIC_MAX_MEMBERS,
    SYS_CAP_STK_TRUNK_MAX_ID,
    SYS_CAP_STK_TRUNK_MAX_NUM,
    SYS_CAP_STK_TRUNK_MEMBERS,
    SYS_CAP_STMCTL_DEFAULT_THRD,
    SYS_CAP_STMCTL_DIV_PULSE,
    SYS_CAP_STMCTL_MAC_COUNT,
    SYS_CAP_STMCTL_MAX_KBPS,
    SYS_CAP_STMCTL_MAX_PPS,
    SYS_CAP_STMCTL_UPD_FREQ,
    SYS_CAP_STP_STATE_ENTRY_NUM,
    SYS_CAP_SYNC_ETHER_CLOCK,
    SYS_CAP_SYNC_ETHER_DIVIDER,
    SYS_CAP_VLAN_BITMAP_NUM,
    SYS_CAP_VLAN_PROFILE_ID,
    SYS_CAP_VLAN_RANGE_EN_BIT_POS,
    SYS_CAP_VLAN_RANGE_ENTRY_NUM,
    SYS_CAP_VLAN_RANGE_TYPE_BIT_POS,
    SYS_CAP_VLAN_STATUS_ENTRY_BITS,
    SYS_CAP_WLAN_PER_SSI_NUM,
    SYS_CAP_WLAN_PER_TUNNEL_NUM,
    SYS_CAP_SERVICE_ID_NUM,
    SYS_CAP_NPM_MAX_TS_OFFSET,

    SYS_CAP_LINKAGG_RR_MAX_MEM_NUM,
    SYS_CAP_VLAN_STATUS_NUM,
    SYS_CAP_IPFIX_MEMORY_SHARE,
    SYS_CAP_NH_MAX_LOGIC_DEST_PORT,

    SYS_CAP_DMPS_SERDES_NUM_PER_SLICE,
    SYS_CAP_DMPS_HSS28G_NUM_PER_SLICE,
    SYS_CAP_DMPS_HSS15G_NUM_PER_SLICE,
    SYS_CAP_DMPS_HSS_NUM,
    SYS_CAP_DMPS_CALENDAR_CYCLE,

    SYS_CAP_DMA_MAX_CHAN_ID,
    SYS_CAP_MAX
};

struct sys_usw_mchip_api_s
{
    int32 (*mchip_cap_init)(uint8 lchip);
    int32 (*ipuc_tcam_init)(uint8 lchip, sys_ipuc_route_mode_t route_mode, void* ofb_cb_fn);
    int32 (*ipuc_tcam_deinit)(uint8 lchip, sys_ipuc_route_mode_t route_mode);
    int32 (*ipuc_tcam_get_blockid)(uint8 lchip, sys_ipuc_tcam_data_t *p_data, uint8 *block_id);
    int32 (*ipuc_tcam_write_key)(uint8 lchip, sys_ipuc_tcam_data_t *p_data);
    int32 (*ipuc_tcam_write_ad)(uint8 lchip, sys_ipuc_tcam_data_t *p_data);
    int32 (*ipuc_tcam_move)(uint8 lchip, uint32 new_index, uint32 old_index, void *p_ofb_cb);
    int32 (*ipuc_show_tcam_key)(uint8 lchip, sys_ipuc_tcam_data_t *p_data);
    int32 (*ipuc_show_tcam_status)(uint8 lchip);
    int32 (*ipuc_show_sram_usage)(uint8 lchip);
    int32 (*ipuc_alpm_init)(uint8 lchip, uint8 calpm_prefix8, uint8 ipsa_enable);
    int32 (*ipuc_alpm_deinit)(uint8 lchip);
    int32 (*ipuc_alpm_add)(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index, void *wb_alpm_info);
    int32 (*ipuc_alpm_del)(uint8 lchip, sys_ipuc_param_t* p_ipuc_param);
    int32 (*ipuc_alpm_update)(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index);
    int32 (*ipuc_alpm_arrange_fragment)(uint8 lchip, sys_ipuc_param_list_t *p_info_list);
    int32 (*ipuc_alpm_show_alpm_key)(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);
    int32 (*ipuc_alpm_show_status)(uint8 lchip);
    int32 (*ipuc_alpm_mapping_wb_master)(uint8 lchip, uint8 sync);
    int32 (*ipuc_alpm_get_wb_info)(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, void* p_alpm_info);
    int32 (*ipuc_alpm_dump_db)(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param);
    int32 (*ipuc_alpm_merge)(uint8 lchip, uint32 vrf_id, uint8 ip_ver);
    int32 (*ipuc_alpm_set_fragment_status)(uint8 lchip, uint8 ip_ver, uint8 status);
    int32 (*ipuc_alpm_get_fragment_status)(uint8 lchip, uint8 ip_ver, uint8* status);
    int32 (*ipuc_alpm_get_fragment_auto_enable)(uint8 lchip, uint8* enable);
    int32 (*ipuc_alpm_get_real_tcam_index)(uint8 lchip, uint16 soft_tcam_index, uint8 *real_lchip, uint16* real_tcam_index);
    int32 (*mem_bist_set)(uint8 lchip, void* p_val);
    int32 (*parser_set_hash_field)(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage);
    int32 (*parser_get_hash_field)(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage);
    int32 (*parser_hash_init)(uint8 lchip);
    int32 (*hash_set_cfg)(uint8 lchip, void* p_hash_cfg);
    int32 (*hash_get_cfg)(uint8 lchip, void* p_hash_cfg);
    int32 (*hash_set_offset)(uint8 lchip, void* p_hash_offset);
    int32 (*hash_get_offset)(uint8 lchip, void* p_hash_offset);

    /* DMPS */
    int32 (*mac_set_property)(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);
    int32 (*mac_get_property)(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);
    int32 (*mac_set_interface_mode)(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);
    int32 (*mac_get_link_up)(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);
    int32 (*mac_get_capability)(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);
    int32 (*serdes_set_property)(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
    int32 (*serdes_get_property)(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
    int32 (*serdes_set_mode)(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);
    int32 (*serdes_set_link_training_en)(uint8 lchip, uint16 serdes_id, bool enable);
    int32 (*serdes_get_link_training_status)(uint8 lchip, uint16 serdes_id, uint16* p_value);
    int32 (*datapath_init)(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg);
    int32 (*mac_init)(uint8 lchip);
    int32 (*mcu_show_debug_info)(uint8 lchip);
    int32 (*mac_self_checking)(uint8 lchip, uint32 gport);
    int32 (*mac_link_up_event)(uint8 lchip, uint32 gport);

    /*FTM*/
    int32 (*ftm_mapping_ctc_to_drv_hash_poly)(uint8 lchip, uint8 sram_type, uint32 type, uint32* p_poly);
    int32 (*ftm_mapping_drv_to_ctc_hash_poly)(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 *poly_type);
    int32 (*ftm_get_hash_poly_cap)(uint8 lchip, uint8 sram_type, ctc_ftm_hash_poly_t* hash_poly);
    /*peri*/
    int32 (*peri_init)(uint8 lchip);
    int32 (*peri_mdio_init)(uint8 lchip);
    int32 (*peri_set_phy_scan_cfg)(uint8 lchip);
    int32 (*peri_set_phy_scan_para)(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);
    int32 (*peri_get_phy_scan_para)(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);
    int32 (*peri_read_phy_reg)(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);
    int32 (*peri_write_phy_reg)(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);
    int32 (*peri_set_mdio_clock)(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq);
    int32 (*peri_get_mdio_clock)(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq);
    int32 (*peri_set_mac_led_mode)(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner);
    int32 (*peri_set_mac_led_mapping)(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map);
    int32 (*peri_set_mac_led_en)(uint8 lchip, bool enable);
    int32 (*peri_get_mac_led_en)(uint8 lchip, bool* enable);
    int32 (*peri_set_gpio_mode)(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode);
    int32 (*peri_set_gpio_output)(uint8 lchip, uint8 gpio_id, uint8 out_para);
    int32 (*peri_get_gpio_input)(uint8 lchip, uint8 gpio_id, uint8* in_value);
    int32 (*peri_phy_link_change_isr)(uint8 lchip, uint32 intr, void* p_data);
    int32 (*peri_get_chip_sensor)(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value);

    /*diag*/
    int32 (*diag_get_pkt_trace)(uint8 lchip, void* p_value);
    int32 (*diag_get_drop_info)(uint8 lchip, void* p_value);
    int32 (*diag_set_dbg_pkt)(uint8 lchip, void* p_value);
    int32 (*diag_set_dbg_session)(uint8 lchip, void* p_value);
};
typedef struct sys_usw_mchip_api_s sys_usw_mchip_api_t;

struct sys_usw_mchip_s
{
    sys_usw_mchip_api_t *p_mchip_api;
    uint32 *p_capability;
};

typedef struct sys_usw_mchip_s sys_usw_mchip_t;

#define MCHIP_CAP(type) p_sys_mchip_master[lchip]->p_capability[type]

#define MCHIP_API(lchip) p_sys_mchip_master[lchip]->p_mchip_api

extern sys_usw_mchip_t  *p_sys_mchip_master[CTC_MAX_LOCAL_CHIP_NUM];



int32
sys_usw_mchip_init(uint8 lchip);

int32
sys_usw_mchip_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

