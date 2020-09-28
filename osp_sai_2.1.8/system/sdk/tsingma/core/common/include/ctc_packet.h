/**
 @file ctc_packet.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0

   This file contains all packet related data structure, enum, macro and proto.
*/

#ifndef _CTC_PACKET_H
#define _CTC_PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_chip.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup packet PACKET
 @{
*/

/**
 @brief Define packet related MACROs
*/
#define CTC_PKT_HDR_ROOM    64                          /**< [GB.GG.D2.TM] Header room length in bytes */
#define CTC_PKT_MTU         (9600 + CTC_PKT_HDR_ROOM)   /**< [GB.GG.D2.TM] Max packet length in bytes, include header room */

#define CTC_PKT_MAX_NETIF_NAME_LEN        16
/**
 @brief Define packet transport mode
*/
enum ctc_pkt_mode_e
{
    CTC_PKT_MODE_DMA,           /**< [GB.GG.D2.TM] Packet is transported by DMA */
    CTC_PKT_MODE_ETH            /**< [GB.GG.D2.TM] Packet is transported by Ethernet */
};
typedef enum ctc_pkt_mode_e ctc_pkt_mode_t;

/**
 @brief Define packet RX/TX flag
*/
enum ctc_pkt_flag_e
{
    CTC_PKT_FLAG_MCAST              = 0x0001,  /**< [GB.GG.D2.TM] [RX,TX] Is multicast packet or not */
    CTC_PKT_FLAG_SRC_SVID_VALID     = 0x0002,  /**< [GB.GG.D2.TM] [RX,TX] Source svlan id is set */
    CTC_PKT_FLAG_SRC_CVID_VALID     = 0x0004,  /**< [GB.GG.D2.TM] [RX,TX] Source cvlan id is set */
    CTC_PKT_FLAG_PRIORITY           = 0x0008,  /**< [GB.GG.D2.TM] [RX,TX] Priority is set */
    CTC_PKT_FLAG_COLOR              = 0x0010,  /**< [GB.GG] [RX,TX] Color is set */
    CTC_PKT_FLAG_NH_OFFSET_BYPASS   = 0x0020,  /**< [GB.GG.D2.TM] [TX] Use bypass NH offset, higher than NH_OFFSET_VALID */
    CTC_PKT_FLAG_NH_OFFSET_VALID    = 0x0040,  /**< [GB.GG.D2.TM] [TX] Use field nh_offset, otherwise use bridge NH offset, lower than NH_OFFSET_BYPASS */
    CTC_PKT_FLAG_NH_OFFSET_IS_8W    = 0x0080,  /**< [GB.GG.D2.TM] [TX] Indicate field nh_offset is 8W, otherwise is 4W */
    CTC_PKT_FLAG_HASH_VALID         = 0x0100,  /**< [GB.GG.D2.TM] [TX] Use field hash, otherwise generated by SDK */
    CTC_PKT_FLAG_SRC_PORT_VALID     = 0x0200,  /**< [GB.GG.D2.TM] [TX] Use field src_port, otherwise use CTC_LPORT_CPU */
    CTC_PKT_FLAG_PKT_TYPE_VALID     = 0x0400,  /**< [GB.GG] [TX] Use field packet_type, otherwise use CTC_PARSER_PKT_TYPE_ETHERNET */
    CTC_PKT_FLAG_INGRESS_MODE       = 0x0800,  /**< [GB.GG.D2.TM] [TX] Indicate field will do IPE processing though I-LOOP channel,
                                                                 for GB, if loop packet do bridge need set CTC_PKT_FLAG_NH_OFFSET_VALID */
    CTC_PKT_FLAG_BUFFER_VICTIM_PKT  = 0x1000,   /**< [GG.D2.TM] [RX] Indicate the packet is the victim packet of buffer monitor log*/
    CTC_PKT_FLAG_UNKOWN_MACDA       = 0x2000,   /**< [GG.D2.TM] [RX] Indicate the packet macda is unknown*/
    CTC_PKT_FLAG_OAM_USE_FID        = 0x4000,   /**< [GG.D2.TM] [TX] Indicate Oam Using fid lookup*/
    CTC_PKT_FLAG_VRFID_VALID        = 0x8000,  /**< [GG.D2.TM] [RX] Indicate vrfid is valid */
    CTC_PKT_FLAG_METADATA_VALID     = 0x10000,  /**< [GG.D2.TM] [RX] Indicate metadata is valid */
    CTC_PKT_FLAG_CANCEL_DOT1AE      = 0x20000,   /**< [D2.TM] [TX] Indicate the pakcet won't be encrypted */
    CTC_PKT_FLAG_ENCRYPTED          = 0x40000,   /**< [D2.TM] [RX] Indicate the pakcet to cpu will be encrypted */
    CTC_PKT_FLAG_ASSIGN_DEST_PORT   = 0x80000,   /**< [D2.TM] [TX] In stacking situation,cpu assign the out port, ignore stacking trunk*/
    CTC_PKT_FLAG_INTERNAl_PORT      = 0x100000,  /**< [D2.TM] [RX] Indicate source port is internal port */
    CTC_PKT_FLAG_NHID_VALID         = 0x200000,   /**< [GB.GG.D2.TM] [TX] Indicate  Use field nhid*/
    CTC_PKT_FLAG_BPE_TRANSPARENT    = 0x400000,  /**< [D2.TM] [TX] packet will transmit transparent with not encap ecid from extender port */
    CTC_PKT_FLAG_PTP_TS_PRECISION   = 0x800000,   /**< [D2.TM] [TX] Indicate the ts set by system is precision time */
    CTC_PKT_FLAG_SESSION_ID_EN      = 0x1000000,   /**< [D2.TM] [TX] Indicate enable session id for packet tx*/
    CTC_PKT_FLAG_SESSION_PENDING_EN      = 0x2000000,   /**< [D2.TM] [TX] Indicate packet for this session will stop transmit in tx timer*/
    CTC_PKT_FLAG_APS_PATH_EN         = 0x4000000,   /**< [GG.D2.TM] [TX] Indicate oam packet select aps path, not from aps bridge*/
    CTC_PKT_FLAG_DIAG_PKT            = 0x8000000,   /**< [TM] [TX] Indicate the pakcet is a diagnostic packet*/
    /*SYSTEM MODIFY, Added by weij for sdk bug 105728, 2017-09-11*/
    CTC_PKT_FLAG_LPORT_VALID = 0x10000000,
    /* SYSTEM MODIFIED by liwh for merge bug 100413 & 100427 to support 1br, this flag is not exist in http://10.10.25.60/svn/ctcsdk/tags/duet2_sdk_release_v3.9.1.2_2017-04-17_12_17_17 */
    CTC_PKT_FLAG_NH_OFFSET_BYPASS_ECID = 0x20000000,
    /* system modified by liwh & lil for bug 46591, 2018-05-15 */
    CTC_PKT_FLAG_APS_W_PATH = 0x40000000,   /**< [GG.D2] [TX] Indicate packet use working path of aps nexthop*/
    CTC_PKT_FLAG_APS_P_PATH      = 0x80000000   /**< [GG.D2] [TX] Indicate packet use protection path of aps nexthop*/
    /* liwh end */
};
typedef enum ctc_pkt_flag_e ctc_pkt_flag_t;

/**
 @brief Define packet OAM flags
*/
enum ctc_pkt_oam_flag_e
{
    CTC_PKT_OAM_FLAG_LINK_OAM       = 0x0001,   /**< [GB.GG.D2.TM] [ETH,MPLSTP] Indicate is link-level */
    CTC_PKT_OAM_FLAG_IS_DM          = 0x0002,   /**< [GB.GG.D2.TM] [ETH,MPLSTP] Indicate is DM */
    CTC_PKT_OAM_FLAG_IS_LM          = 0x0004,   /**< [GB.GG.D2.TM] [ETH,MPLSTP] Indicate is LM */
    CTC_PKT_OAM_FLAG_IS_MIP         = 0x0008,   /**< [GB.GG.D2.TM] [ETH] Indicate is MIP */
    CTC_PKT_OAM_FLAG_TX_MIP_TUNNEL  = 0x0010,   /**< [GB.GG.D2.TM] [ETH] Indicate bypass MIP configured port when TX */
    CTC_PKT_OAM_FLAG_IS_UP_MEP      = 0x0020,   /**< [GB.GG.D2.TM] [ETH] Indicate is UP MEP */
    CTC_PKT_OAM_FLAG_HAS_MPLS_GAL   = 0x0040,   /**< [GB.GG.D2.TM] [MPLSTP] Indicate has GAL */
    CTC_PKT_OAM_FLAG_HAS_MPLS_CW    = 0x0080    /**< [GB.GG] [MPLSTP] Indicate has CW */
};
typedef enum ctc_pkt_oam_flag_e ctc_pkt_oam_flag_t;

/**
 @brief Define packet type
*/
enum ctc_pkt_oper_type_s
{
    CTC_PKT_OPER_NORMAL = 0,    /**< [GB.GG.D2.TM] Normal Packet */
    CTC_PKT_OPER_C2C    = 4,    /**< [GB.GG.D2.TM] C2C for stacking */
    CTC_PKT_OPER_PTP    = 5,    /**< [GB.GG.D2.TM]PTP Packet */
    CTC_PKT_OPER_OAM    = 7     /**< [GB.GG.D2.TM] OAM Packet */
};
typedef enum ctc_pkt_oper_type_s ctc_pkt_oper_type_t;

/**
 @brief Define OAM type
*/
enum ctc_oam_type_e
{
    CTC_OAM_TYPE_NONE           = 0,    /**< [GB.GG.D2.TM] Invalid Type */
    CTC_OAM_TYPE_ETH            = 1,    /**< [GB.GG.D2.TM] Ethernet OAM */
    CTC_OAM_TYPE_IP_BFD         = 2,    /**< [GB.GG.D2.TM] IP BFD OAM */
    CTC_OAM_TYPE_MPLS           = 6,    /**< [GB.GG] MPLS OAM */
    CTC_OAM_TYPE_MPLS_BFD       = 7,    /**< [GB.GG.D2.TM] MPLS BFD OAM */
    CTC_OAM_TYPE_ACH            = 8,    /**< [GB.GG.D2.TM] MPLSTP OAM */
    CTC_OAM_TYPE_TRILL_BFD      = 10,   /**< [GB.GG] TRILL BFD OAM */
    CTC_OAM_TYPE_TRILL_BFD_ECHO = 11    /**< [GB.GG.D2.TM] TRILL BFD Echo OAM */
};
typedef enum ctc_oam_type_e ctc_oam_type_t;

/*
  define forwarded packet to cpu nexthopptr
   combine:
        |3bit    |      6bit    |        6bit      | 3bit   |   = 18 bit
        [modid][exception index][exception sub index|[cpu mac]
 In humber:
        |3bit    |      6bit    |      5bit  |       4 bit     |    = 18 bit
        [modid][exception index][ Reserved][exception sub index/cpu mac]
       1) if cpu destination  is single cpu , the low 4 bits indicate exception sub index;
       2) if cpu destination  is multiple cpu ,the low 3 bits indicate cpu mac index;
 In Greatbelt/GoldenGate
       |   6bit     | 8bit     |       4 bit     |    = 18 bit
       [    Rsv    ][reason-id     ][     cpu mac    ]
        1) the low 4 bits indicate cpu mac index
       2) the mid 8 bits indicate cpu reason
*/


#define CTC_PKT_CPU_REASON_BUILD_NHPTR(reason, cpu_mac)  ((reason) << 4 | (cpu_mac & 0xF))  /**< [GB.GG] Build nexthop_ptr by reason and cpu mac index */

#define CTC_PKT_CPU_REASON_GET_BY_NHPTR(nexthop_ptr)     ((nexthop_ptr >> 4) & 0x3FF)       /**< [GB.GG] Get cpu reason by nexthop_ptr */


#define CTC_PKT_CPU_REASON_L2_PDU_RESV  32  /* [GB.GG.D2.TM] max l2 pdu action index is 32 */
#define CTC_PKT_CPU_REASON_L3_PDU_RESV  32  /* [GB.GG.D2.TM] max l3 pdu action index is 32 */
#define CTC_PKT_CPU_REASON_OAM_PDU_RESV 32  /* [GB.GG.D2.TM] max oam pdu action index is 32 */

/*
 @brief Define packet cpu reason
*/
enum ctc_pkt_cpu_reason_e
{
    CTC_PKT_CPU_REASON_DROP        = 0,     /**< [GB.GG.D2.TM] Drop packet */

    /*refert to ctc_l2pdu_action_index_t, ActionIndex is used to map Resson = (CTC_PKT_CPU_REASON_L2_PDU + ActionIndex)*/
    CTC_PKT_CPU_REASON_L2_PDU         = 1,  /**< [GB.GG.D2.TM] L2 pdu start, */

    /*refert to pdu module, refert to ctc_l3pdu_action_index_t, ActionIndex is used to map Resson = (CTC_PKT_CPU_REASON_L3_PDU + ActionIndex)*/
    CTC_PKT_CPU_REASON_L3_PDU         = CTC_PKT_CPU_REASON_L2_PDU + CTC_PKT_CPU_REASON_L2_PDU_RESV, /**<  [GB.GG.D2.TM] L3 pdu start*/

    CTC_PKT_CPU_REASON_IGMP_SNOOPING  = CTC_PKT_CPU_REASON_L3_PDU + CTC_PKT_CPU_REASON_L3_PDU_RESV, /**<  [GB.GG.D2.TM] IGMP-Snooping*/
    CTC_PKT_CPU_REASON_PTP,                 /**< [GB.GG.D2.TM] PTP */
    CTC_PKT_CPU_REASON_MPLS_OAM,            /**< [GB.GG.D2.TM] TMPLS or MPLS protocol OAM */
    CTC_PKT_CPU_REASON_DCN,                 /**< [GB.GG.D2.TM] DCN */
    CTC_PKT_CPU_REASON_SCL_MATCH,           /**< [GB.GG.D2.TM] SCL entry match */
    CTC_PKT_CPU_REASON_ACL_MATCH,           /**< [GB.GG.D2.TM] ACL entry match */
    CTC_PKT_CPU_REASON_SFLOW_SOURCE,        /**< [GB.GG.D2.TM] SFlow source */
    CTC_PKT_CPU_REASON_SFLOW_DEST,          /**< [GB.GG.D2.TM] SFlow destination */

    CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT,  /**< [GB.GG.D2.TM] MAC learn limit base on port */
    CTC_PKT_CPU_REASON_L2_VLAN_LEARN_LIMIT,  /**< [GB.GG.D2.TM] MAC learn limit base on vlan/vsi */
    CTC_PKT_CPU_REASON_L2_SYSTEM_LEARN_LIMIT,/**< [GB.GG.D2.TM] MAC learn limit base on system */
    CTC_PKT_CPU_REASON_L2_MOVE,             /**< [GB.GG.D2.TM] MAC SA station move */
    CTC_PKT_CPU_REASON_L2_CPU_LEARN,        /**< [GB.GG.D2.TM] MAC SA CPU learning */
    CTC_PKT_CPU_REASON_L2_COPY_CPU,         /**< [GB.GG.D2.TM] MAC DA copy to CPU */
    CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL,   /**< [GB.GG.D2.TM] IP TTL check fail */
    CTC_PKT_CPU_REASON_L3_MTU_FAIL,         /**< [GB.GG.D2.TM] IP MTU check fail */
    CTC_PKT_CPU_REASON_L3_MC_RPF_FAIL,      /**< [GB.GG.D2.TM] IP multicast RPF check fail */
    CTC_PKT_CPU_REASON_L3_MC_MORE_RPF,      /**< [GB.GG.D2.TM] IP multicast more RPF */
    CTC_PKT_CPU_REASON_TUNNEL_MORE_RPF = CTC_PKT_CPU_REASON_L3_MC_MORE_RPF, /**< [GG.D2.TM] L3 tunnel more RPF */
    CTC_PKT_CPU_REASON_L3_IP_OPTION,        /**< [GB.GG.D2.TM] IP option */
    CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT,    /**< [GB.GG.D2.TM] IP ICMP redirect */
    CTC_PKT_CPU_REASON_L3_COPY_CPU,         /**< [GB.GG.D2.TM] IP DA copy to CPU */

    CTC_PKT_CPU_REASON_L3_MARTIAN_ADDR,     /**< [GB.GG.D2.TM] IP martian address */
    CTC_PKT_CPU_REASON_L3_URPF_FAIL,        /**< [GB.GG.D2.TM] IP URPF fail */
    CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL, /**< [GB.GG.D2.TM] IP multicast ttl check fail */
    CTC_PKT_CPU_REASON_EPE_TTL_CHECK_FAIL = CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL, /**< [D2.TM] Packet TTL check fail at EPE */
    CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL, /**< [GB.GG.D2.TM] MPLS ttl check fail */
    CTC_PKT_CPU_REASON_GRE_UNKNOWN,         /**< [GB.GG.D2.TM] GRE unknown */
    CTC_PKT_CPU_REASON_LABEL_MISS,          /**< [GB.GG.D2.TM] MPLS label out of range */
    CTC_PKT_CPU_REASON_LINK_ID_FAIL,        /**< [GB.GG.D2.TM] IP Link id check fail */
    CTC_PKT_CPU_REASON_OAM_HASH_CONFLICT,   /**< [GB.GG.D2.TM] OAM hash conflict */
    CTC_PKT_CPU_REASON_IPFIX_HASH_CONFLICT,     /**< [GG.D2.TM] Ipfix hash conflict */
    CTC_PKT_CPU_REASON_VXLAN_NVGRE_CHK_FAIL,   /**< [GG.D2.TM] Vxlan or NvGre check fail */

    CTC_PKT_CPU_REASON_OAM,                   /**< [GB.GG.D2.TM] OAM PDU from oam engine, refer to ctc_oam_exceptions_t, ExcpIndex is used to map Reason (CTC_PKT_CPU_REASON_OAM + ExcpIndex) */
    CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE = CTC_PKT_CPU_REASON_OAM + CTC_PKT_CPU_REASON_OAM_PDU_RESV,  /**< [GB.GG.D2.TM] OAM Defect message from oam engine */

    CTC_PKT_CPU_REASON_FWD_CPU ,              /**< [GB.GG.D2.TM] packet redirect forward to CPU */
    CTC_PKT_CPU_REASON_NEW_ELEPHANT_FLOW,     /**< [GG.D2.TM] New elephant flow */
    CTC_PKT_CPU_REASON_L2_STORM_CTL,          /**< [GB.GG.D2.TM] Storm control*/
    CTC_PKT_CPU_REASON_MONITOR_BUFFER_LOG,    /**< [GG.D2.TM]  Monitor Buffer log*/
    CTC_PKT_CPU_REASON_MONITOR_LATENCY_LOG,   /**< [GG.D2.TM]  Monitor Latency log*/
    CTC_PKT_CPU_REASON_QUEUE_DROP_PKT,        /**< [GG.D2.TM] Monitor Queue Drop packet to cpu*/
    CTC_PKT_CPU_REASON_GENEVE_PKT,            /**< [GG.D2.TM] Geneve packet*/
    CTC_PKT_CPU_REASON_MIRRORED_TO_CPU,       /**< [GB.GG.D2.TM] Mirrored packet to cpu*/
    CTC_PKT_CPU_REASON_L2_VSI_LEARN_LIMIT,    /**< [GB] MAC learn limit base on VSI */

    CTC_PKT_CPU_REASON_ARP_MISS,              /**< [GB.D2.TM] ARP lookup miss */

    CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT,      /**< [GG.D2.TM] Packets discarded by chip for diagnostic*/
    CTC_PKT_CPU_REASON_WLAN_KEPP_ALIVE,       /**< [D2.TM] WLAN keep alive*/
    CTC_PKT_CPU_REASON_WLAN_DTLS,             /**< [D2.TM] WLAN Dtls*/
    CTC_PKT_CPU_REASON_WLAN_CTL,              /**< [D2.TM] WLAN Control*/
    CTC_PKT_CPU_REASON_WLAN_DOT11_MGR,        /**< [D2.TM] WLAN Dot11 manager*/

    CTC_PKT_CPU_REASON_WLAN_CHECK_ERR,        /**< [D2.TM] WLAN error, include client error,cipher mismatch*/

    CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR,      /**< [D2.TM] DOT1AE check error*/
    CTC_PKT_CPU_REASON_DOT1AE_REACH_PN_THRD,  /**< [D2.TM] DOT1AE decrypt/encrypt reach PN thrd*/
    CTC_PKT_CPU_REASON_DOT1AE_DECRYPT_ICV_CHK_FAIL,    /**< [D2.TM] DOT1AE ICV check fail*/
    CTC_PKT_CPU_REASON_MCAST_TUNNEL_FRAG,              /**< [D2.TM] Mcast tunnel frag*/
    CTC_PKT_CPU_REASON_TUNNEL_ECN_MIS,                 /**< [D2.TM] Tunnel ecn mismatch*/
    CTC_PKT_CPU_REASON_ICMP_ERR_MSG,                   /**< [D2.TM] ICMP error message*/
    CTC_PKT_CPU_REASON_C2C_PKT ,                       /**< [GB.GG.D2.TM] CPU-to-CPU packet */
    CTC_PKT_CPU_REASON_PIM_SNOOPING,                   /**< [D2.TM] PIM-Snooping*/
    CTC_PKT_CPU_REASON_DKITS_CAPTURE_PKT,              /**< [D2.TM] dkits captured pkt to cpu*/
    CTC_PKT_CPU_REASON_ELEPHANT_FLOW_LOG,              /**< [D2.TM] elephant flow log */
    CTC_PKT_CPU_REASON_UNKNOWN_GEM_PORT,               /**< [TM] unknown gem port pkt */
    CTC_PKT_CPU_REASON_PON_PT_PKT,                     /**< [TM] pon passthrough packet to cpu */
    /* ----- User define CPU Reason ---- */
    CTC_PKT_CPU_REASON_CUSTOM_BASE = 256,
    CTC_PKT_CPU_REASON_MAX_COUNT = 1024
};
typedef enum ctc_pkt_cpu_reason_e ctc_pkt_cpu_reason_t;

/**
 @brief Define CPU reason destination
*/
enum ctc_pkt_cpu_reason_dest_e
{
    CTC_PKT_CPU_REASON_TO_LOCAL_CPU = 0, /**< [GB.GG.D2.TM] cpu reason to local CPU by DMA */
    CTC_PKT_CPU_REASON_TO_LOCAL_PORT,    /**< [GB.GG.D2.TM] cpu reason to local/remote port */
    CTC_PKT_CPU_REASON_TO_REMOTE_CPU,    /**< [GB.GG.D2.TM] cpu reason to remote cpu */
    CTC_PKT_CPU_REASON_TO_DROP,           /**< [GB.GG.D2.TM] cpu reason to drop */
    CTC_PKT_CPU_REASON_TO_NHID,         /**< [GB.GG.D2.TM] get destination by nexthop*/
    CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH, /**< [GB.GG.D2.TM] cpu reason to local CPU by network port */
    CTC_PKT_CPU_REASON_DISCARD_CANCEL_TO_CPU, /**< [D2.TM] Cancel switch logic discard's packet exception to CPU */

	CTC_PKT_CPU_REASON_TO_MAX_COUNT

};
typedef enum ctc_pkt_cpu_reason_dest_e ctc_pkt_cpu_reason_dest_t;

/**
 @brief Define CPU reason destination
*/
enum ctc_pkt_cpu_reason_group_type_e
{
    CTC_PKT_CPU_REASON_GROUP_EXCEPTION = 0,  /**< [GB] qos group for packet exception to cpu */
    CTC_PKT_CPU_REASON_GROUP_OAM,            /**< [GB] qos group for packet from OAM engine to cpu */
    CTC_PKT_CPU_REASON_GROUP_FORWARD,        /**< [GB] qos group for packet forward to cpu */
    CTC_PKT_CPU_REASON_GROUP_REMOTE_FORWARD  /**< [GB] qos group for packet from remote chip forward to cpu */
};
typedef enum ctc_pkt_cpu_reason_group_type_e ctc_pkt_cpu_reason_group_type_t;

/**
 @brief Define packet buffer for RX
*/
struct ctc_pkt_buf_s
{
    uint8* data;                /**< [GB.GG.D2.TM] pointer to packet data */
    uint32 len;                 /**< [GB.GG.D2.TM] length of data in bytes */
};
typedef struct ctc_pkt_buf_s ctc_pkt_buf_t;

/**
 @brief Define packet buffer for TX
*/
struct ctc_pkt_skb_s
{
    uint8* head;                /**< [GB.GG.D2.TM] head pointer of the total buffer */
    uint8* data;                /**< [GB.GG.D2.TM] head pointer of the data */
    uint8* tail;                /**< [GB.GG.D2.TM] end pointer of the data */
    uint8* end;                 /**< [GB.GG.D2.TM] end pointer of the total buffer */
    uint32 len;                 /**< [GB.GG.D2.TM] Packet length in bytes */
    uint8  buf[CTC_PKT_MTU];    /**< [GB.GG.D2.TM] Buffer for packet, if buf unused, head/tail/end = NULL, data point to userdate */
};
typedef struct ctc_pkt_skb_s ctc_pkt_skb_t;

/**
 @brief Init packet skb with default length

 @param[in/out] skb     Pointer to packet buffer

 @remark

 @return CTC_E_XXX
*/
static INLINE int32
ctc_packet_skb_init(ctc_pkt_skb_t* skb)
{
    skb->head = skb->buf;
    skb->end = skb->buf + CTC_PKT_MTU;
    skb->data = skb->buf + CTC_PKT_HDR_ROOM;
    skb->tail = skb->data;
    skb->len = 0;
    return 0;
}

/**
 @brief Init packet skb with length of header room

 @param[in/out] skb     Pointer to packet buffer

 @param[in] skb         Length of header room

 @remark

 @return CTC_E_XXX
*/
static INLINE int32
ctc_packet_skb_init_with_len(ctc_pkt_skb_t* skb, uint32 len)
{
    skb->head = skb->buf;
    skb->end = skb->buf + CTC_PKT_MTU;
    skb->data = skb->buf + len;
    skb->tail = skb->data;
    skb->len = 0;
    return 0;
}

/**
 @brief Push packet skb, expand room forwards

 @param[in/out] skb     Pointer to packet buffer

 @param[in]     len     Pushed length

 @remark

 @return CTC_E_XXX
*/
static INLINE uint8*
ctc_packet_skb_push(ctc_pkt_skb_t* skb, uint32 len)
{
    skb->data -= len;
    skb->len += len;
    return skb->data;
}

/**
 @brief Pull packet skb, strip room has processed or need ignored

 @param[in/out] skb     Pointer to packet buffer

 @param[in]     len     Pulled length

 @remark

 @return CTC_E_XXX
*/
static INLINE uint8*
ctc_packet_skb_pull(ctc_pkt_skb_t* skb, uint32 len)
{
    skb->data += len;
    skb->len -= len;
    return skb->data;
}

/**
 @brief Put packet skb, expand room backwards

 @param[in/out] skb     Pointer to packet buffer

 @param[in]     len     Puted length

 @remark

 @return CTC_E_XXX
*/
static INLINE uint8*
ctc_packet_skb_put(ctc_pkt_skb_t* skb, uint32 len)
{
    uint8* tmp = skb->tail;

    skb->tail += len;
    skb->len += len;
    return tmp;
}

/**
 @brief Define timestamp format in packet TX/RX information, used by DM and PTP
*/
struct ctc_packet_ts_s
{
    uint32  seconds;            /**< [GB.GG.D2.TM] seconds */
    uint32  nanoseconds;        /**< [GB.GG.D2.TM] nano seconds */
};
typedef struct ctc_packet_ts_s ctc_packet_ts_t;

/**
 @brief Define packet OAM data-structure
*/
struct ctc_pkt_oam_info_s
{
    uint32  type;               /**< [GB.GG.D2.TM] [RX,TX] OAM type, refer to ctc_oam_type_t */
    uint32  flags;              /**< [GB.GG.D2.TM]  [RX,TX] OAM flags, refer to ctc_pkt_oam_flag_t */
    uint32  dm_ts_offset;       /**< [GB.GG.D2.TM] [TX] [DM] Offset in bytes of timestamp */
    uint16  vid;                /**< [GB.GG.D2.TM] [TX] VLAN ID, valid for UP MEP */
    uint16  mep_index;          /**< [GB.GG.D2.TM] [RX] index of table MEP/RMEP */
    ctc_packet_ts_t dm_ts;      /**< [GB.GG.D2.TM] [RX] [DM] timestamp */
    uint32  lm_fcl;             /**< [GB.GG.D2.TM] [RX] [LM] RxFCl value at the time LMR frame was received */
};
typedef struct ctc_pkt_oam_info_s ctc_pkt_oam_info_t;

/**
 @brief Define PTP opertation type in packet header
*/
enum ctc_ptp_oper_type_e
{
    CTC_PTP_CAPTURE_ONLY,       /**< [GB.GG] Get timestamp and store in TX Capture FIFO */
    CTC_PTP_REPLACE_ONLY,       /**< [GG.D2.TM] Get timestamp and replace into packet */
    CTC_PTP_NULL_OPERATION,     /**< [GG.D2.TM] Do not take any operation */
    CTC_PTP_CORRECTION          /**< [GG.D2.TM] Get timestamp and calculate correction field value into packet */
};
typedef enum ctc_ptp_oper_type_e ctc_ptp_oper_type_t;

enum ctc_pkt_netif_type_e
{
    CTC_PKT_NETIF_T_PORT,
    CTC_PKT_NETIF_T_VLAN,
    CTC_PKT_NETIF_MAX_TYPE,
};
typedef enum ctc_pkt_netif_type_e ctc_pkt_netif_type_t;

/**
 @brief Define packet PTP data-structure
*/
struct ctc_pkt_ptp_info_s
{
    ctc_packet_ts_t     ts;         /**< [GB.GG.D2.TM] [RX,TX] PTP timestamp */
    ctc_ptp_oper_type_t oper;       /**< [GB.GG.D2.TM] [TX] PTP operation type */
    uint8               seq_id;     /**< [GB.GG.D2.TM] [TX] PTP capture sequence Id, only for oper equal to CAPTURE_ONLY */
    uint8               ts_offset;  /**< [GG.D2.TM]  [TX] PTP timestamp offset */
    uint8               resv0[2];   /**< [GB.GG.D2.TM] Reserved for alignment */
};
typedef struct ctc_pkt_ptp_info_s ctc_pkt_ptp_info_t;

/**
 @brief Define packet TX/RX information
*/
struct ctc_pkt_info_s
{
    uint32  flags;               /**< [GB.GG.D2.TM] [RX,TX] flags of the packet, refer to ctc_pkt_flag_t */
    uint32  dest_gport;          /**< [GB.GG.D2.TM] [TX] destination global port ID for unicast, include LinkAgg, valid if CTC_PKT_FLAG_MCAST is not set
                                               [RX] packet to queue id*/
    uint16  src_svid;            /**< [GB.GG.D2.TM] [RX,TX] source S-VLAN ID */
    uint16  src_cvid;            /**< [GB.GG.D2.TM] [RX,TX] source C-VLAN ID */
    uint32  src_port;            /**< [GB.GG.D2.TM] [RX,TX] source port, valid for TX if CTC_PKT_FLAG_SRC_PORT_VALID is set, or stackingTrunkPort */
    uint32  stacking_src_port;   /**< [GB.GG.D2.TM] [RX] stacking source port, valid for RX if stackingTrunkPort */
    uint8   oper_type;           /**< [GB.GG.D2.TM] [RX,TX] operation type, refer to ctc_pkt_oper_type_t */
    uint8   priority;            /**< [GB.GG.D2.TM] [RX,TX] priority of the packet,range is [0, 15]  */
    uint16  dest_group_id;       /**< [GB.GG.D2.TM] [TX] destination group ID for multicast, valid if CTC_PKT_FLAG_MCAST is set */
    uint8   color;               /**< [GB.GG.D2.TM] [RX,TX] color of the packet, refer to ctc_qos_color_t */
    uint8   src_cos;             /**< [GB.GG.D2.TM] [RX,TX] COS of the packet, range is [0, 7] */
    uint8   ttl;                 /**< [GB.GG.D2.TM] [RX,TX] TTL of the packet */
    uint8   is_critical;         /**< [GB.GG.D2.TM] [RX,TX] If set, indicate that the packet should not be droped in queue */
    ctc_pkt_oam_info_t oam;      /**< [GB.GG.D2.TM] [RX,TX] store OAM information, valid if oper_type is CTC_PKT_OPER_OAM */
    ctc_pkt_ptp_info_t ptp;      /**< [GB.GG.D2.TM] [RX,TX] store PTP information, valid if oper_type is CTC_PKT_OPER_PTP */
    uint32  nh_offset;           /**< [GB.GG.D2.TM] [TX] nexthop offset, valid if CTC_PKT_FLAG_NH_OFFSET_VALID flags is set */
    uint8   hash;                /**< [GB.GG.D2.TM] [TX] hash of LAG for load balance, valid if CTC_PKT_FLAG_HASH_VALID flags is set */
    uint8   payload_offset;      /**< [GB.GG.D2.TM] [RX] offset into the packet for start of payload */
    uint8   vlan_domain;         /**< [GG.GB.D2.TM] [TX] refer to ctc_port_vlan_domain_type_t */
    uint8   resv0;               /**< [GB.GG.D2.TM] Reserved for alignment */
    ctc_pkt_cpu_reason_t reason; /**< [GB.GG.D2.TM] [RX] packet to CPU reason */
    uint16  vrfid;               /**< [GB.GG.D2.TM] [RX] VRFID */
    uint8   packet_type;         /**< [GB.GG.D2.TM] [RX] packet type, refer to ctc_parser_pkt_type_t */
    uint8   src_chip;            /**< [GB.GG.D2.TM] [RX] source chip ID */
    uint16 fid;                  /**< [GG.D2.TM] [RX,TX] Forwarding Id */
    uint16 logic_src_port;   /**<[GB.GG.D2.TM] [RX] logic source port*/
    uint16 meta_data;       /**<[GG.D2.TM] [RX] meta data*/
    uint16 svlan_tpid;              /**<[GB.GG.D2.TM] [TX] svlan tpid*/
    uint8 stag_op;               /**< [GB.GG.D2.TM] [RX] operation type of stag, refer to ctc_vlan_tag_op_t*/
    uint8 ctag_op;               /**< [GB.GG.D2.TM] [RX] operation type of ctag, refer to ctc_vlan_tag_op_t*/
    uint8 logic_port_type;       /**< [GB.GG.D2.TM] [TX]logic-port-type, used for VPLS horizon split discard */
    uint8 aps_p_path;            /**< [GG.D2.TM] [TX] If set, indicate oam packet select protection path, else select working path, valid if CTC_PKT_FLAG_APS_PATH_EN is set */
    uint16 trucated_len;        /**< [D2.TM] [RX] trucate len of pkt*/
    uint16 cid;                 /**< [D2.TM] [RX] categories id*/
    uint16 isolated_id;          /**< [GB.GG] [TX]Port isolated id*/
    uint16 lport;                /**< [D2.TM] [RX] localPhyPort, for TX only valid if CTC_PKT_FLAG_INGRESS_MODE is set, indicate local output port*/
    uint32 nhid;                /**< [GB.GG.D2.TM] [TX] nexthop id*/
    uint16 session_id;          /**< [D2.TM] [TX] used for packet tx on timer*/

    /*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*//* SYSTEM MODIFIED by liuyang for system bug 36347 2015-12-4*/
    uint8   src_leaf;            /**< [GB.GG] [RX] source chip ID */
    /*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*//* SYSTEM MODIFIED by liuyang for system bug 38251 2015-4-30*/
    uint16   efd_id;            /**< [GB.GG] [RX] EFD FlowID*/
};
typedef struct ctc_pkt_info_s ctc_pkt_info_t;

typedef struct ctc_pkt_tx_s ctc_pkt_tx_t;

typedef void (*ctc_pkt_callback)(ctc_pkt_tx_t* pkt, void* user_data);

/**
 @brief Define packet TX data-structure
*/
struct ctc_pkt_tx_s
{
    ctc_pkt_mode_t  mode;       /**< [GB.GG.D2.TM] Packet Transport mode */
    ctc_pkt_skb_t   skb;        /**< [GB.GG.D2.TM] Packet TX buffer */
    ctc_pkt_info_t  tx_info;    /**< [GB.GG.D2.TM] Packet TX information needed to encoded into packet header */
    ctc_pkt_callback callback;  /**< [GB.GG.D2.TM] Packet Tx async callback function */
    void*           user_data;  /**< [GB.GG.D2.TM] User data for callback */
    uint8           lchip;      /**< [GB.GG.D2.TM] Local chip ID the packet expected transmit to */
};


#define CTC_PKT_TOTAL_HDR_LEN(pkt_rx) \
    (((pkt_rx)->eth_hdr_len + (pkt_rx)->pkt_hdr_len) + (pkt_rx)->stk_hdr_len)

/**
 @brief Define packet RX data-structure
*/
struct ctc_pkt_rx_s
{
    ctc_pkt_mode_t  mode;        /**< [GB.GG.D2.TM] Packet Transport mode */
    uint16          pkt_len;     /**< [GB.GG.D2.TM] Packet length in bytes */
    uint8           dma_chan;    /**< [GB.GG.D2.TM] DMA Channel ID of packet RX */
    uint8           eth_hdr_len; /**< [GB.GG.D2.TM] CPUMAC header length in bytes, CTC_PKT_CPUMAC_LEN for ETH, 0 for DMA */
    uint8           pkt_hdr_len; /**< [GB.GG.D2.TM] Packet header length in bytes, CTC_PKT_HEADER_LEN */
    uint8           stk_hdr_len; /**< [GB.GG.D2.TM] Stacking header length in bytes, 0 for packet from local chip */
    uint8           lchip;       /**< [GB.GG.D2.TM] Local chip ID the packet receive from */
    uint8           resv0[1];    /**< [GB.GG.D2.TM] Reserved for alignment */
    uint32          buf_count;   /**< [GB.GG.D2.TM] Count of packet buffer array */
    ctc_pkt_buf_t*  pkt_buf;     /**< [GB.GG.D2.TM] Packet buffer array */
    ctc_pkt_info_t  rx_info;     /**< [GB.GG.D2.TM] Packet RX information decoded from packet header */
};
typedef struct ctc_pkt_rx_s ctc_pkt_rx_t;

/**
 @brief Define packet receive callback function prototype
*/
typedef int32 (* CTC_PKT_RX_CALLBACK)(ctc_pkt_rx_t* p_pkt_rx);

/**
 @brief Define packet transmit callback function prototype
*/
typedef int32 (* CTC_PKT_TX_CALLBACK)(ctc_pkt_tx_t* p_pkt_tx);

/**
 @brief Packet global config information
*/
struct ctc_pkt_global_cfg_s
{
    CTC_PKT_RX_CALLBACK     rx_cb;      /**< [GB.GG.D2.TM] Callback function for packet RX */
    CTC_PKT_TX_CALLBACK     socket_tx_cb;      /**< [GB.GG.D2.TM] Callback function for packet TX by Socket*/
    CTC_PKT_TX_CALLBACK     dma_tx_cb;      /**< [GB.GG.D2.TM] Callback function for packet TX by Dma, used for EADP*/
};
typedef struct ctc_pkt_global_cfg_s ctc_pkt_global_cfg_t;

struct ctc_pkt_tx_timer_s
{
    uint16 session_num;   /**<[D2.TM] support max session num, used to cfg timer when state is pending*/
    uint16 interval;      /**<[D2.TM] unit is ms, used to cfg timer when state is pending */
    uint16 pkt_len;       /**<[D2.TM] max packet length, all session packet cannot exceed the length*/
    uint8  timer_state;   /**<[D2.TM] 0:pending, 1:start, 2��stop, 3:destroy*/
    uint8  rsv;
};
typedef struct ctc_pkt_tx_timer_s ctc_pkt_tx_timer_t;

struct ctc_pkt_netif_s
{
    uint8 type;     /**<[TM] Network interface type, 0: associated with physical switch port, 1: associated with specified vlan id */
    uint16 vlan;    /**<[TM] Vlan id associated with this interface */
    uint32 gport;  /**<[TM] Gport associated with this interface */
    mac_addr_t mac; /**<[TM] MAC address associated with this interface */
    char name[CTC_PKT_MAX_NETIF_NAME_LEN]; /**<[TM] Network interface name */
};
typedef struct ctc_pkt_netif_s ctc_pkt_netif_t;

/**@} end of @defgroup packet PACKET  */

#ifdef __cplusplus
}
#endif

#endif
