
#ifdef TSINGMA

#include "ctc_error.h"
#include "sys_usw_mchip.h"
#include "sys_usw_ftm.h"
#include "drv_api.h"


int32 sys_tsingma_capability_init(uint8 lchip)
{
    /*uint32 entry_num = 0;*/

    /* set chip_capability */
    MCHIP_CAP(SYS_CAP_ACL_COMPRESS_ETHER_TYPE_NUM)         = 64;
    MCHIP_CAP(SYS_CAP_ACL_COPP_THRD)                       = 0x10000;
    MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)                     = 3;
    MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP_NUM)                 = 3;
    MCHIP_CAP(SYS_CAP_ACL_FIELD_RANGE)                     = 12;
    MCHIP_CAP(SYS_CAP_ACL_HASH_AD_INDEX_OFFSET)            = 31;
    MCHIP_CAP(SYS_CAP_ACL_HASH_CID_KEY)                    = 256;
    MCHIP_CAP(SYS_CAP_ACL_HASH_SEL_PROFILE)                = 16;
    MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)                = 8;
    MCHIP_CAP(SYS_CAP_ACL_INGRESS_LKUP_NUM)                = 8;
    MCHIP_CAP(SYS_CAP_ACL_LABEL_NUM)                       = 9;
    MCHIP_CAP(SYS_CAP_ACL_MAX_CID_VALUE)                   = 255;
    MCHIP_CAP(SYS_CAP_ACL_MAX_SESSION)                     = 4;
    MCHIP_CAP(SYS_CAP_ACL_PORT_CLASS_ID_NUM)               = 1024;
    MCHIP_CAP(SYS_CAP_ACL_SERVICE_ID_NUM)                  = 0xFFFF;
    MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR)                   = 128;
    MCHIP_CAP(SYS_CAP_ACL_UDF_ENTRY_NUM)                   = 16;
    MCHIP_CAP(SYS_CAP_ACL_UDF_OFFSET_MAX)                  = 0x1F;    /*udf offset max value in word*/
    MCHIP_CAP(SYS_CAP_ACL_VLAN_ACTION_SIZE_PER_BUCKET)     = 16;
    MCHIP_CAP(SYS_CAP_ACL_VLAN_CLASS_ID_NUM)               = 1024;
    MCHIP_CAP(SYS_CAP_APS_GROUP_NUM)                       = 1024;
    MCHIP_CAP(SYS_CAP_AQM_PORT_THRD_HIGN)                  = 1024;
    MCHIP_CAP(SYS_CAP_AQM_PORT_THRD_LOW)                   = 256;
    MCHIP_CAP(SYS_CAP_BFD_INTERVAL_CAM_NUM)                = 32;
    MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0)                      = 79;
    MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1)                      = 80;
    MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2)                      = 81;
    MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)                      = 82;
    MCHIP_CAP(SYS_CAP_CHANID_DROP)                         = 90;
    MCHIP_CAP(SYS_CAP_CHANID_ELOG)                         = 76;
    MCHIP_CAP(SYS_CAP_CHANID_ELOOP)                        = 75;
    MCHIP_CAP(SYS_CAP_CHANID_GMAC_MAX)                     = 63;
    MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)                  = 74;
    MCHIP_CAP(SYS_CAP_CHANID_ILOOP)                        = 77;
    MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)                  = 73;
    MCHIP_CAP(SYS_CAP_CHANID_MAX)                          = 84;
    MCHIP_CAP(SYS_CAP_CHANID_OAM)                          = 78;
    MCHIP_CAP(SYS_CAP_CHANID_QCN)                          = 83;
    MCHIP_CAP(SYS_CAP_CHANID_RSV)                          = 127;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0)                = 68;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1)                = 69;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2)                = 70;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3)                = 71;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)                = 64;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1)                = 65;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2)                = 66;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3)                = 67;
    MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)              = 72;
    MCHIP_CAP(SYS_CAP_CHANNEL_NUM)                         = 84;
    MCHIP_CAP(SYS_CAP_CPU_OAM_EXCP_NUM)                    = 37;
    MCHIP_CAP(SYS_CAP_CPU_REASON_OAM_DEFECT_MESSAGE_BASE)  = 128;
    MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM)                      = 16;
    MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM)                       = 2;
    MCHIP_CAP(SYS_CAP_DOT1AE_DIVISION_WIDE)                = 8;
    MCHIP_CAP(SYS_CAP_DOT1AE_RX_CHAN_NUM)                  = 32;
    MCHIP_CAP(SYS_CAP_DOT1AE_SEC_CHAN_NUM)                 = 64;
    MCHIP_CAP(SYS_CAP_DOT1AE_SHIFT_WIDE)                   = 5;
    MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM)                  = 32;
    MCHIP_CAP(SYS_CAP_EFD_FLOW_DETECT_MAX)                 = 512;
    MCHIP_CAP(SYS_CAP_EFD_FLOW_STATS)                      = 1024;
    MCHIP_CAP(SYS_CAP_EPEDISCARDTYPE_DS_ACL_DIS)           = 0x13;
    MCHIP_CAP(SYS_CAP_EPEDISCARDTYPE_DS_PLC_DIS)           = 0x15;
    MCHIP_CAP(SYS_CAP_FID_NUM)                             = 0x3FFF;
    MCHIP_CAP(SYS_CAP_FLOW_HASH_LEVEL_NUM)                 = 4;
    MCHIP_CAP(SYS_CAP_GCHIP_CHIP_ID)                       = 0x7E;
    MCHIP_CAP(SYS_CAP_GLB_DEST_PORT_NUM_PER_CHIP)          = 128;
    MCHIP_CAP(SYS_CAP_INTR_NUM)                            = 32;
    MCHIP_CAP(SYS_CAP_IP_HASH_LEVEL_NUM)                   = 3;
    MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID)               = 15;
    MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)            = 4;
    MCHIP_CAP(SYS_CAP_IPFIX_MIN_AGING_INTERVAL)            = 1;
    MCHIP_CAP(SYS_CAP_IPFIX_SAMPPLING_PKT_INTERVAL)        = 4;
    MCHIP_CAP(SYS_CAP_ISOLATION_GROUP_NUM)                 = 32;
    MCHIP_CAP(SYS_CAP_ISOLATION_ID_MAX)                    = 31;
    MCHIP_CAP(SYS_CAP_ISOLATION_ID_NUM)                    = 32;
    MCHIP_CAP(SYS_CAP_L2_BLACK_HOLE_ENTRY)                 = 128;
    MCHIP_CAP(SYS_CAP_L2_FDB_CAM_NUM)                      = 32;
    MCHIP_CAP(SYS_CAP_L2PDU_BASED_BPDU_ENTRY)              = 1;
    MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY)         = 16;
    MCHIP_CAP(SYS_CAP_L2PDU_BASED_L3TYPE_ENTRY)            = 15;
    MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY)             = 8;
    MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY)       = 10;
    MCHIP_CAP(SYS_CAP_L2PDU_PER_PORT_ACTION_INDEX)         = 31;
    MCHIP_CAP(SYS_CAP_L2PDU_PORT_ACTION_INDEX)             = 31;
    MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM)                 = 1024;
    MCHIP_CAP(SYS_CAP_L3IF_INNER_ROUTE_MAC_NUM)            = 64;
    MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_ENTRY_NUM)           = 64;
    MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM)                 = 512;
    MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY)       = 8;
    MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID)                    = 4095;
    MCHIP_CAP(SYS_CAP_L3PDU_ACTION_INDEX)                  = 31;
    MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA)                    = 4;
    MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO)             = 8;
    MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT)                    = 8;
    MCHIP_CAP(SYS_CAP_L3PDU_L3IF_ACTION_INDEX)             = 15;
    MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN)              = 5;
    MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX)                     = 0x3FFFF;
    MCHIP_CAP(SYS_CAP_LEARN_LIMIT_PROFILE_NUM)             = 256;
    MCHIP_CAP(SYS_CAP_LEARNING_CACHE_MAX_INDEX)            = 32;
    MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM)                 = 2048;
	MCHIP_CAP(SYS_CAP_LINKAGG_RR_MAX_MEM_NUM)              = 255;
    MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_ALL_MEM_NUM)            = 512;
    MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_GRP_MAX)                = 64;
    MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_PER_GRP_MEM)            = 8;
    MCHIP_CAP(SYS_CAP_LINKAGG_DYNAMIC_INTERVAL)            = 5;
    MCHIP_CAP(SYS_CAP_LINKAGG_FRAGMENT_SIZE)               = 1;
    MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)                   = 256;
    MCHIP_CAP(SYS_CAP_LINKAGG_MEM_NUM)                     = 255;
    MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_MEM_MAX)          = 256;
    MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX)          = 7;
    MCHIP_CAP(SYS_CAP_LINKAGG_RR_TID_MAX)                  = 255;
    MCHIP_CAP(SYS_CAP_LOCAL_SLICE_NUM)                     = 1;
    MCHIP_CAP(SYS_CAP_MAC_HASH_LEVEL_NUM)                  = 5;
    MCHIP_CAP(SYS_CAP_MIRROR_ACL_ID)                       = 8;
    MCHIP_CAP(SYS_CAP_MIRROR_ACL_LOG_ID)                   = 4;
    MCHIP_CAP(SYS_CAP_MIRROR_CPU_RX_SPAN_INDEX)            = 0x013D;
    MCHIP_CAP(SYS_CAP_MIRROR_CPU_TX_SPAN_INDEX)            = 0x013E;
    MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_INDEX_BASE)    = 0xE8;
    MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_PRIORITY)      = 3;
    MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_IPFIX_LOG_INDEX)       = 0xF5;
    MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L2_SPAN_INDEX_BASE)    = 0xE0;
    MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L3_SPAN_INDEX_BASE)    = 0xE4;
    MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_INDEX_BASE)   = 0x0108;
    MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_PRIORITY)     = 8;
    MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_IPFIX_LOG_INDEX)      = 0x129;
    MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L2_SPAN_INDEX_BASE)   = 0x100;
    MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L3_SPAN_INDEX_BASE)   = 0x104;
    MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_RSV_PROF)             = 7;
    MCHIP_CAP(SYS_CAP_MONITOR_DIVISION_WIDE)               = 7;
    MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL)           = 7;
    MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE)          = 64;
    MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHANNEL)                 = 64;
    MCHIP_CAP(SYS_CAP_MONITOR_SHIFT_WIDE)                  = 5;
    MCHIP_CAP(SYS_CAP_MONITOR_SYNC_CNT)                    = 64;
    MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD)             = 32767;
    MCHIP_CAP(SYS_CAP_MPLS_INNER_ROUTE_MAC_NUM)            = 64;
    MCHIP_CAP(SYS_CAP_MPLS_MAX_LABEL)                      = 0xFFFFF;
    MCHIP_CAP(SYS_CAP_MPLS_MAX_LABEL_SPACE)                = 0xFF;
    MCHIP_CAP(SYS_CAP_MPLS_MAX_OAM_CHK_TYPE)               = 0x5;
    MCHIP_CAP(SYS_CAP_MPLS_MAX_TPID_INDEX)                 = 3;
    MCHIP_CAP(SYS_CAP_MPLS_VPLS_SRC_PORT_NUM)              = 0xFFF;
    MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM)                 = 64;
    MCHIP_CAP(SYS_CAP_NEXTHOP_MAX_CHIP_NUM)                = 31;
    MCHIP_CAP(SYS_CAP_NEXTHOP_PORT_NUM_PER_CHIP)           = 512;
    MCHIP_CAP(SYS_CAP_NH_CW_NUM)                           = 16;
    MCHIP_CAP(SYS_CAP_NH_DROP_DESTMAP)                     = 0xFFFE;
    MCHIP_CAP(SYS_CAP_NH_DSMET_BITMAP_MAX_PORT_ID)         = 0x1FF;
    MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM)                = 1023;
    MCHIP_CAP(SYS_CAP_NH_ECMP_MEMBER_NUM)                  = 16384;
    MCHIP_CAP(SYS_CAP_NH_ECMP_RR_GROUP_NUM)                = 16;
    MCHIP_CAP(SYS_CAP_NH_ILOOP_MAX_REMOVE_WORDS)           = 3;
    MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_INVALID_IPSA_NUM)       = 0xFF;
    MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM)          = 16;
    MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM)          = 16;
    MCHIP_CAP(SYS_CAP_NH_L2EDIT_VLAN_PROFILE_NUM)          = 64;
    MCHIP_CAP(SYS_CAP_NPM_SESSION_NUM)                     = 8;
    MCHIP_CAP(SYS_CAP_NPM_MAX_TS_OFFSET)                   = 80;
    MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM)           = 128;
    MCHIP_CAP(SYS_CAP_OAM_DEFECT_NUM)                      = 20;
    MCHIP_CAP(SYS_CAP_OAM_MEP_ID)                          = 8191;
    MCHIP_CAP(SYS_CAP_OAM_MEP_NUM_PER_CHAN)                = 4;
    MCHIP_CAP(SYS_CAP_OVERLAY_TUNNEL_IP_INDEX)             = 4;
    MCHIP_CAP(SYS_CAP_PARSER_L2_PROTOCOL_USER_ENTRY)       = 4;
    MCHIP_CAP(SYS_CAP_PARSER_L3_PROTOCOL_USER_ENTRY)       = 4;
    MCHIP_CAP(SYS_CAP_PARSER_L3FLEX_BYTE_SEL)              = 8;
    MCHIP_CAP(SYS_CAP_PARSER_L4_APP_DATA_CTL_ENTRY)        = 4;
    MCHIP_CAP(SYS_CAP_PARSER_UPF_OFFSET)                   = 64;
    MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)                  = 256;
    MCHIP_CAP(SYS_CAP_PHY_PORT_NUM_PER_SLICE)              = 64;
    MCHIP_CAP(SYS_CAP_PKT_CPU_QDEST_BY_DMA)                = 104;
    MCHIP_CAP(SYS_CAP_PKT_STRIP_PKT_LEN)                   = 128;
    MCHIP_CAP(SYS_CAP_PKT_TRUNCATED_LEN)                   = 2040;
    MCHIP_CAP(SYS_CAP_PORT_NUM)                            = 256;
    MCHIP_CAP(SYS_CAP_PORT_NUM_GLOBAL)                     = 512;
    MCHIP_CAP(SYS_CAP_PORT_NUM_PER_CHIP)                   = 256;
    MCHIP_CAP(SYS_CAP_PORT_TCAM_TYPE_NUM)               = 6;
    MCHIP_CAP(SYS_CAP_PTP_CAPTURED_RX_SEQ_ID)              = 16;
    MCHIP_CAP(SYS_CAP_PTP_CAPTURED_RX_SOURCE)              = 9;
    MCHIP_CAP(SYS_CAP_PTP_CAPTURED_TX_SEQ_ID)              = 4;
    MCHIP_CAP(SYS_CAP_PTP_FRC_VALUE_SECOND)                = 0xFFFFFFFF;
    MCHIP_CAP(SYS_CAP_PTP_MAX_PTP_ID)                      = 7;
    MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE)                 = 1000000000;
    MCHIP_CAP(SYS_CAP_PTP_RC_QUANTA)                       = 4;
    MCHIP_CAP(SYS_CAP_PTP_SECONDS_OF_EACH_WEEK)            = 604800;
    MCHIP_CAP(SYS_CAP_PTP_SYNC_CODE_BIT)                   = 97;
    MCHIP_CAP(SYS_CAP_PTP_SYNC_PULSE_FREQUENCY_HZ)         = 250000;
    MCHIP_CAP(SYS_CAP_PTP_TAI_TO_GPS_SECONDS)              = 315964819;
    MCHIP_CAP(SYS_CAP_PTP_TOD_1PPS_DELAY)                  = 0x80000000;
    MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_THRESHOLD)         = 250;
    MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_TOGGLE_STEP)       = 250000000;
    MCHIP_CAP(SYS_CAP_PTP_TOD_PULSE_HIGH_LEVEL)            = 0x3FFFF;
    MCHIP_CAP(SYS_CAP_PVLAN_COMMUNITY_ID_NUM)              = 15;
    MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)              = 64;
    MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_NUM)                  = 1024;
    MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX)            = 7;
    MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX)           = 15;
    MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX)            = 15;
    MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX)              = 15;
    MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX)          = 15;
    MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)               = 4;
    MCHIP_CAP(SYS_CAP_QOS_EXT_GRP_BASE_NUM)                = 128;
    MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)               = 384;
    MCHIP_CAP(SYS_CAP_QOS_GROUP_NUM)                       = 448;
    MCHIP_CAP(SYS_CAP_QOS_GROUP_SHAPE_PROFILE)             = 64;
    MCHIP_CAP(SYS_CAP_QOS_GRP_SHP_CBUCKET_NUM)             = 4;
    MCHIP_CAP(SYS_CAP_QOS_MAX_SHAPE_BURST)                 = 4194300;
    MCHIP_CAP(SYS_CAP_QOS_MIN_SHAPE_BURST)                 = 16000;
    MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM)                  = 4;
    MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)                = 8;
    MCHIP_CAP(SYS_CAP_QOS_OAM_QUEUE_NUM)                   = 16;
    MCHIP_CAP(SYS_CAP_QOS_PHB_OFFSET_NUM)                  = 8;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_ACTION_PROFILE_NUM)      = 128;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_CBS)                     = 5120;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_COPP_PROFILE_NUM)        = 32;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_MFP_PROFILE_NUM)         = 64;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_POLICER_NUM)             = 12544;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES)        = 125;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_PROF_TBL_NUM)            = 4;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_PROFILE_NUM)             = 800;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_KBPS)               = 100000000;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_PPS)                = 148809523;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_SUPPORTED_FREQ_NUM)      = 20;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH)    = 256;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH) = 4;
    MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL)           = 8;
    MCHIP_CAP(SYS_CAP_QOS_PORT_AQM_FREQ)                   = 25000;
    MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM)                = 64;
    MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM_4Q)             = 480;
    MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM_8Q)             = 240;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP)                 = 640;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)                  = 1024;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)                 = 512;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)         = 768;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL)               = 0;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXCP)            = 640;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT)             = 1024;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_MISC_CHANNEL)    = 512;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_NETWORK_MISC)    = 768;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_NORMAL)          = 0;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_BUCKET)                    = 16;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_GRP_NUM_FOR_CPU_REASON)    = 16;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_INVALID_GROUP)             = 0x1FF;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_METER_PROFILE)             = 64;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM)                       = 4096;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN)              = 8;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)           = 8;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_GRP)               = 8;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_OFFSET_EXT)                = 8;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_OFFSET_NETWORK)            = 12;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)                = 8;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_SHAPE_PROFILE)             = 264;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_SHAPE_PIR_PROFILE)         = 264;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_WEIGHT_BASE)               = 1024;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD)             = 32767;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WTD_PROFILE_NUM)      = 0x3F;
    MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM)     = 31;
    MCHIP_CAP(SYS_CAP_QOS_REASON_C2C_MAX_QUEUE_NUM)        = 16;
    MCHIP_CAP(SYS_CAP_QOS_SCHED_MAX_QUE_WEITGHT)           = 0x1FFF;
    MCHIP_CAP(SYS_CAP_QOS_SCHED_WEIGHT_BASE)               = 1024;
    MCHIP_CAP(SYS_CAP_QOS_SCHED_MAX_EXT_QUE_CLASS)         = 7;
    MCHIP_CAP(SYS_CAP_QOS_SHP_BUCKET_CIR_PASS0)            = 4;
    MCHIP_CAP(SYS_CAP_QOS_SHP_BUCKET_CIR_PASS1)            = 16;
    MCHIP_CAP(SYS_CAP_QOS_SHP_BUCKET_PIR)                  = 8;
    MCHIP_CAP(SYS_CAP_QOS_SHP_BUCKET_PIR_PASS0)            = 7;
    MCHIP_CAP(SYS_CAP_QOS_SHP_BUCKET_PIR_PASS1)            = 15;
    MCHIP_CAP(SYS_CAP_QOS_SHP_FULL_TOKENS)                 = 0xFFFFFF;
    MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)            = 2000;
    MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_SHIFT)                   = 5;
    MCHIP_CAP(SYS_CAP_QOS_SHP_RATE)                        = 100000000;
    MCHIP_CAP(SYS_CAP_QOS_SHP_RATE_PPS)                    = 148809523;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE)                  = 0x3FFFFF;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH)        = 256;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC)             = 0xFF;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD)                  = 0xFF;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT)            = 0x11;
    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH)     = 5;
    MCHIP_CAP(SYS_CAP_QOS_SHP_UPDATE_UNIT)                 = 4;
    MCHIP_CAP(SYS_CAP_QOS_SHP_WEIGHT_BASE)                 = 256;
    MCHIP_CAP(SYS_CAP_QOS_SUB_GRP_NUM_PER_GRP)             = 2;
    MCHIP_CAP(SYS_CAP_QOS_VLAN_POLICER_NUM)                = 512;
    MCHIP_CAP(SYS_CAP_RANDOM_LOG_RATE)                     = 0x8000;
    MCHIP_CAP(SYS_CAP_RANDOM_LOG_THRESHOD)                 = 0x100000;
    MCHIP_CAP(SYS_CAP_RANDOM_LOG_THRESHOD_PER)             = 330;
    MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM)                = 8;
    MCHIP_CAP(SYS_CAP_RPF_IF_NUM)                          = 16;
    MCHIP_CAP(SYS_CAP_RPF_PROFILE_NUM)                     = 512;
    MCHIP_CAP(SYS_CAP_SCL_ACL_CONTROL_PROFILE)             = 16;
    MCHIP_CAP(SYS_CAP_SCL_AD_FIELD_POS_IS_HALF)            = 31;
    MCHIP_CAP(SYS_CAP_SCL_BY_PASS_VLAN_PTR)                = 0x1000;
    MCHIP_CAP(SYS_CAP_SCL_DS_AD_FIELD_PO_AD_INDEX)         = 30;
    MCHIP_CAP(SYS_CAP_SCL_HASH_SEL_ID)                     = 4;
    MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG)                  = 63;
    MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_VLAN_CLASS)            = 62;
    MCHIP_CAP(SYS_CAP_SCL_LABEL_NUM)                       = 64;
    MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)                        = 4;
    MCHIP_CAP(SYS_CAP_SCL_VLAN_ACTION_RESERVE_NUM)         = 64;
    MCHIP_CAP(SYS_CAP_SCL_SERVICE_ID_NUM)               = 512;
    MCHIP_CAP(SYS_CAP_SCL_HASH_NUM)                        = 2;
    MCHIP_CAP(SYS_CAP_STATS_ACL0_SIZE)                     = 4096;
    MCHIP_CAP(SYS_CAP_STATS_ACL1_SIZE)                     = 512;
    MCHIP_CAP(SYS_CAP_STATS_ACL2_SIZE)                     = 256;
    MCHIP_CAP(SYS_CAP_STATS_ACL3_SIZE)                     = 256;
    MCHIP_CAP(SYS_CAP_STATS_CGMAC_RAM_MAX)                 = 4;
    MCHIP_CAP(SYS_CAP_STATS_DEQUEUE_SIZE)                  = 2048;
    MCHIP_CAP(SYS_CAP_STATS_ECMP_RESERVE_SIZE)             = 7168;
    MCHIP_CAP(SYS_CAP_STATS_EGS_ACL0_SIZE)                 = 1024;
    MCHIP_CAP(SYS_CAP_STATS_ENQUEUE_SIZE)                  = 2048;
    MCHIP_CAP(SYS_CAP_STATS_IPE_FWD_SIZE)                  = 8192;
    MCHIP_CAP(SYS_CAP_STATS_IPE_IF_SIZE)                   = 8192;
    MCHIP_CAP(SYS_CAP_STATS_POLICER_SIZE)                  = 4096;
    MCHIP_CAP(SYS_CAP_STATS_RAM_GLOBAL_SIZE)               = 4096;
    MCHIP_CAP(SYS_CAP_STATS_RAM_PRIVATE_SIZE)              = 512;
    MCHIP_CAP(SYS_CAP_STATS_XQMAC_PORT_NUM)                = 4;
    MCHIP_CAP(SYS_CAP_STATS_XQMAC_RAM_NUM)                 = 18;
    MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE)                = 4096;
    MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM)                 = 8;
    MCHIP_CAP(SYS_CAP_STK_GLB_DEST_PORT_NUM)               = 8192;
    MCHIP_CAP(SYS_CAP_STK_MAX_GCHIP)                       = 128;
    MCHIP_CAP(SYS_CAP_STK_MAX_LPORT)                       = 64;
    MCHIP_CAP(SYS_CAP_STK_PORT_FWD_PROFILE_NUM)            = 8;
    MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM)                 = 128;
    MCHIP_CAP(SYS_CAP_STK_TRUNK_DLB_MAX_MEMBERS)           = 16;
    MCHIP_CAP(SYS_CAP_STK_TRUNK_STATIC_MAX_MEMBERS)        = 31;
    MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_ID)                    = 63;
    MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_NUM)                   = 64;
    MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS)                   = 512;
    MCHIP_CAP(SYS_CAP_STMCTL_DEFAULT_THRD)                 = 0x10000;
    MCHIP_CAP(SYS_CAP_STMCTL_DIV_PULSE)                    = 0x1311d0;
    MCHIP_CAP(SYS_CAP_STMCTL_MAC_COUNT)                    = 128;
    MCHIP_CAP(SYS_CAP_STMCTL_MAX_KBPS)                     = 100000000;
    MCHIP_CAP(SYS_CAP_STMCTL_MAX_PPS)                      = 148809523;
    MCHIP_CAP(SYS_CAP_STMCTL_UPD_FREQ)                     = 32000;
    MCHIP_CAP(SYS_CAP_STP_STATE_ENTRY_NUM)                 = 1024;
    MCHIP_CAP(SYS_CAP_SYNC_ETHER_CLOCK)                    = 3;
    MCHIP_CAP(SYS_CAP_SYNC_ETHER_DIVIDER)                  = 1024;
    MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)                     = 256;       /**< [TM] VLAN_BITMAP_NUM -> VLAN_STATUS_NUM, 4096 -> 256*/
    MCHIP_CAP(SYS_CAP_VLAN_PROFILE_ID)                     = 511;
    MCHIP_CAP(SYS_CAP_VLAN_RANGE_EN_BIT_POS)               = 15;
    MCHIP_CAP(SYS_CAP_VLAN_RANGE_ENTRY_NUM)                = 64;
    MCHIP_CAP(SYS_CAP_VLAN_RANGE_TYPE_BIT_POS)             = 14;
    MCHIP_CAP(SYS_CAP_VLAN_STATUS_ENTRY_BITS)              = 128;
    MCHIP_CAP(SYS_CAP_WLAN_PER_SSI_NUM)                    = 16384;
    MCHIP_CAP(SYS_CAP_WLAN_PER_TUNNEL_NUM)                 = 2048;
    MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT)              = 0x7FFF;

    MCHIP_CAP(SYS_CAP_SERVICE_ID_NUM)                      = 512;

    /*Spec*/
    MCHIP_CAP(SYS_CAP_SPEC_MAX_LCHIP_NUM)                  = 30;
    MCHIP_CAP(SYS_CAP_SPEC_MAX_CHIP_NUM)                   = 128;
    MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM)               = 64;
    MCHIP_CAP(SYS_CAP_SPEC_MAX_PORT_NUM)                   = 256;
    MCHIP_CAP(SYS_CAP_SPEC_ARP_ID_NUM)                     = 1024;
    MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_MEMBER_NUM)         = 16;
    MCHIP_CAP(SYS_CAP_SPEC_VLAN_NUM)                       = 4096;
    MCHIP_CAP(SYS_CAP_SPEC_NPM_SESSION_NUM)                = 4;
    MCHIP_CAP(SYS_CAP_SPEC_ROUTE_MAC_ENTRY_NUM)            = 512;
    MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)                       = 4095;

    MCHIP_CAP(SYS_CAP_SPEC_QUEUE_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_ENQUEUE_SIZE)+MCHIP_CAP(SYS_CAP_STATS_DEQUEUE_SIZE);
    MCHIP_CAP(SYS_CAP_SPEC_POLICER_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_POLICER_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_SHARE2_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_IPE_IF_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_ACL0_IGS_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_ACL0_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_ACL1_IGS_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_ACL1_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_ACL2_IGS_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_ACL2_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_ACL3_IGS_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_ACL3_SIZE)-1;
    MCHIP_CAP(SYS_CAP_SPEC_ACL0_EGS_STATS_NUM) = MCHIP_CAP(SYS_CAP_STATS_EGS_ACL0_SIZE)-1;

    MCHIP_CAP(SYS_CAP_DMPS_SERDES_NUM_PER_SLICE)           = 32;
    MCHIP_CAP(SYS_CAP_DMPS_HSS28G_NUM_PER_SLICE)           = 2;
    MCHIP_CAP(SYS_CAP_DMPS_HSS15G_NUM_PER_SLICE)           = 3;
    MCHIP_CAP(SYS_CAP_DMPS_HSS_NUM)                        = 5;
    MCHIP_CAP(SYS_CAP_DMPS_CALENDAR_CYCLE)                 = 128;

    MCHIP_CAP(SYS_CAP_DMA_MAX_CHAN_ID) = 19;
    return CTC_E_NONE;
}


#endif
