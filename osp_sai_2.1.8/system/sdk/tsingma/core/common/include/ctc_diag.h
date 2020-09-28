/**
 @file ctc_diag.h

 @author  Copyright (C) 2019 Centec Networks Inc.  All rights reserved.

 @date 2019-04-01

 @version v2.0

   This file contains all DIAG related data structure, enum, macro and proto.
*/

#ifndef _CTC_DIAG_H
#define _CTC_DIAG_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_field.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/


/**
 @defgroup diag DIAG
 @{
*/

#define CTC_DIAG_REASON_DESC_LEN 128 /**< [TM] drop reason description length maximum*/
#define CTC_DIAG_DROP_REASON_MAX 256 /**< [TM] drop reason maximum*/
#define CTC_DIAG_TRACE_STR_LEN 64    /**< [TM] packet trace string length maximum*/
#define CTC_DIAG_PACKET_HEAR_LEN 40  /**< [TM] packet header length in ASIC pipeline*/

/**
 @brief packet trace watch key mask enum
*/
typedef enum ctc_diag_trace_watch_key_mask_e
{
    CTC_DIAG_WATCH_KEY_MASK_CHANNEL,        /**< [TM] channel mask bit*/
    CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT,      /**< [TM] src_lport mask bit*/
    CTC_DIAG_WATCH_KEY_MASK_SRC_GPORT,      /**< [TM] src_gport mask bit*/
    CTC_DIAG_WATCH_KEY_MASK_DST_GPORT,      /**< [TM] dst_gport mask bit*/
    CTC_DIAG_WATCH_KEY_MASK_MC_GROUP,       /**< [TM] mc_group mask bit*/
    CTC_DIAG_WATCH_KEY_MASK_MEP_INDEX,      /**< [TM] mep_index mask bit*/

    CTC_DIAG_WATCH_KEY_MASK_MAX
}ctc_diag_trace_watch_key_mask_t;

/**
 @brief packet trace watch key mask bitmap length
*/
#define CTC_DIAG_WATCH_KEY_MASK_BMP_LEN    ((CTC_DIAG_WATCH_KEY_MASK_MAX + CTC_UINT32_BITS) / CTC_UINT32_BITS)


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                     *
 *                     Packet Trace Watch Point and Watch Key                          *
 *                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *       |   Watch Point  | IPE SCL | TM BS | TM MET | EPE ADJ | OAM ADJ | OAM TX |    *
 *   -----------------------------------------------------------------------------|    *
 *    W  | channel        |    *    |   *   |        |    *    |         |        |    *
 *    a  |------------------------------------------------------------------------|    *
 *    t  | lport          |    *    |   *   |        |    *    |         |        |    *
 *    c  |------------------------------------------------------------------------|    *
 *    h  | src_gport      |         |   *   |        |    *    |         |        |    *
 *       |------------------------------------------------------------------------|    *
 *       | dst_gport      |         |   *   |    *   |    *    |         |        |    *
 *       |------------------------------------------------------------------------|    *
 *    K  | mc_group       |         |   *   |    *   |    *    |         |        |    *
 *    e  |------------------------------------------------------------------------|    *
 *    y  | mep_index      |         |       |        |         |    *    |    *   |    *
 *       |------------------------------------------------------------------------|    *
 *                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/**
 @brief packet trace watch key structure
*/
struct ctc_diag_trace_watch_key_s
{
    uint16 channel;     /**< [TM] channel id*/
    uint16 mc_group;    /**< [TM] multicast group id, conflict with dst_gport*/
    uint16 src_lport;   /**< [TM] source local port*/
    uint32 src_gport;   /**< [TM] source global port*/
    uint32 dst_gport;   /**< [TM] destination global port, conflict with mc_group*/
    uint32 mep_index;   /**< [TM] MEP index*/
    uint32 key_mask_bmp[CTC_DIAG_WATCH_KEY_MASK_BMP_LEN];   /**< [TM] mask bitmap for watch key. refer to ctc_diag_trace_watch_key_mask_t*/
};
typedef struct ctc_diag_trace_watch_key_s ctc_diag_trace_watch_key_t;


/**
 @brief packet trace mode enum
*/
typedef enum ctc_diag_pkt_trace_mode_e
{
    CTC_DIAG_TRACE_MODE_USER,       /**< [TM] user mode, trace the specified packet which input by user with data format ctc_diag_trace_user_pkt_t*/
    CTC_DIAG_TRACE_MODE_NETWORK     /**< [TM] network mode, trace the online packet from network port with data format ctc_diag_trace_network_pkt_t*/
}ctc_diag_pkt_trace_mode_t;

/**
 @brief packet trace watch point enum
*/
enum ctc_diag_pkt_trace_watch_point_e
{
    CTC_DIAG_TRACE_POINT_IPE_SCL = 0,   /**< [TM] trace watch point start from IPE_SCL*/
    CTC_DIAG_TRACE_POINT_TM_BS,         /**< [TM] trace watch point start from TM_BS*/
    CTC_DIAG_TRACE_POINT_TM_MET,        /**< [TM] trace watch point start from TM_MET*/
    CTC_DIAG_TRACE_POINT_EPE_ADJ,       /**< [TM] trace watch point start from EPE_ADJ*/
    CTC_DIAG_TRACE_POINT_OAM_ADJ,       /**< [TM] trace watch point start from OAM_ADJ*/
    CTC_DIAG_TRACE_POINT_OAM_TX,        /**< [TM] trace watch point start from OAM_TX*/

    CTC_DIAG_TRACE_POINT_MAX
};
typedef enum ctc_diag_pkt_trace_watch_point_e ctc_diag_pkt_trace_watch_point_t;


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                             *
 *       Packet Trace Network Mode Supportable Key Field       *
 *                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *            CTC_FIELD_KEY_L2_TYPE                  *
 *         *            CTC_FIELD_KEY_ETHER_TYPE               *
 *         *            CTC_FIELD_KEY_MAC_SA                   *
 * Layer2  *            CTC_FIELD_KEY_MAC_DA                   *
 *         *            CTC_FIELD_KEY_SVLAN_ID                 *
 *         *            CTC_FIELD_KEY_CVLAN_ID                 *
 *         *            CTC_FIELD_KEY_VLAN_NUM                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *            CTC_FIELD_KEY_L3_TYPE                  *
 *         *            CTC_FIELD_KEY_IP_TTL                   *
 *         *            CTC_FIELD_KEY_IPV6_FLOW_LABEL          *
 *         *            CTC_FIELD_KEY_IP_PROTOCOL              *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_IP_SA                    *
 *         *  IPv4   *  CTC_FIELD_KEY_IP_DA                    *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_IPV6_SA                  *
 *         *  IPv6   *  CTC_FIELD_KEY_IPV6_DA                  *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_MPLS_LABEL0              *
 *         *  MPLS   *  CTC_FIELD_KEY_MPLS_LABEL1              *
 *         *         *  CTC_FIELD_KEY_MPLS_LABEL2              *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_ARP_OP_CODE              *
 *         *         *  CTC_FIELD_KEY_ARP_PROTOCOL_TYPE        *
 *         *         *  CTC_FIELD_KEY_ARP_SENDER_IP            *
 * Layer3  *  ARP    *  CTC_FIELD_KEY_ARP_TARGET_IP            *
 *         *         *  CTC_FIELD_KEY_SENDER_MAC               *
 *         *         *  CTC_FIELD_KEY_TARGET_MAC               *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_ETHER_OAM_LEVEL          *
 *         *  OAM    *  CTC_FIELD_KEY_ETHER_OAM_OP_CODE        *
 *         *         *  CTC_FIELD_KEY_ETHER_OAM_VERSION        *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_FCOE_DST_FCID            *
 *         *  FCoE   *  CTC_FIELD_KEY_FCOE_SRC_FCID            *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_SLOW_PROTOCOL_CODE       *
 *         *SLOW PRO *  CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS      *
 *         *         *  CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE   *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_INGRESS_NICKNAME         *
 *         *         *  CTC_FIELD_KEY_EGRESS_NICKNAME          *
 *         *  TRILL  *  CTC_FIELD_KEY_TRILL_INNER_VLANID       *
 *         *         *  CTC_FIELD_KEY_TRILL_VERSION            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *            CTC_FIELD_KEY_L4_TYPE                  *
 *         *            CTC_FIELD_KEY_L4_USER_TYPE             *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_L4_DST_PORT              *
 *         * L4 Port *  CTC_FIELD_KEY_L4_SRC_PORT              *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_TCP_ECN                  *
 *         *  TCP    *  CTC_FIELD_KEY_TCP_FLAGS                *
 * Layer4  * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_VN_ID                    *
 *         *         *  CTC_FIELD_KEY_VXLAN_FLAGS              *
 *         *  VxLAN  *  CTC_FIELD_KEY_VXLAN_RSV1               *
 *         *         *  CTC_FIELD_KEY_VXLAN_RSV2               *
 *         * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *         *         *  CTC_FIELD_KEY_GRE_KEY                  *
 *         *  GRE    *  CTC_FIELD_KEY_GRE_FLAGS                *
 *         *         *  CTC_FIELD_KEY_GRE_PROTOCOL_TYPE        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/**
 @brief Define packet trace network packet key structure
*/
struct ctc_diag_trace_network_pkt_s
{
    uint32            count;        /**< [TM] field count*/
    ctc_field_key_t*  field;        /**< [TM] network mode key field, refer to the support table*/
};
typedef struct ctc_diag_trace_network_pkt_s ctc_diag_trace_network_pkt_t;

/**
 @brief Define packet trace user packet structure
*/
typedef struct ctc_diag_trace_user_pkt_s
{
    uint32 src_port;        /**< [TM] source port*/
    uint32 len;             /**< [TM] buffer length, units:byte*/
    uint8* pkt_buf;             /**< [TM] user packet buffer pointer*/
}ctc_diag_trace_user_pkt_t;

/**
 @brief Define packet trace structure
*/
struct ctc_diag_pkt_trace_s
{
    ctc_diag_pkt_trace_mode_t          mode;        /**< [TM] packet trace mode*/
    union
    {
        ctc_diag_trace_user_pkt_t      user;        /**< [TM] user packet*/
        ctc_diag_trace_network_pkt_t   network;     /**< [TM] network packet key*/
    }pkt;

    ctc_diag_pkt_trace_watch_point_t   watch_point; /**< [TM] watch start point*/
    ctc_diag_trace_watch_key_t         watch_key;   /**< [TM] watch key*/
};
typedef struct ctc_diag_pkt_trace_s ctc_diag_pkt_trace_t;

/**
 @brief packet trace value type enum
*/
typedef enum ctc_diag_value_type_e
{
    CTC_DIAG_VAL_NONE,      /**< [TM] none type*/
    CTC_DIAG_VAL_UINT32,    /**< [TM] uint32 type*/
    CTC_DIAG_VAL_U32_HEX,   /**< [TM] uint32 hex type*/
    CTC_DIAG_VAL_UINT64,    /**< [TM] uint64 type*/
    CTC_DIAG_VAL_U64_HEX,   /**< [TM] uint64 hex type*/
    CTC_DIAG_VAL_CHAR,      /**< [TM] char type*/
    CTC_DIAG_VAL_MAC,       /**< [TM] mac type*/
    CTC_DIAG_VAL_IPv4,      /**< [TM] ipv4 type*/
    CTC_DIAG_VAL_IPv6,      /**< [TM] ipv6 type*/
    CTC_DIAG_VAL_PKT_HDR,   /**< [TM] packet header hex type*/

    CTC_DIAG_VAL_MAX
}ctc_diag_value_type_t;

/**
 @brief Define packet trace result info structure
*/
struct ctc_diag_pkt_trace_result_info_s
{
    char  str[CTC_DIAG_TRACE_STR_LEN];      /**< [TM] key indicate the reslut info node key*/
    uint8 val_type;                         /**< [TM] data type*/
    uint8 position;                         /**< [TM] the reslut info node position*/
    union
    {
        uint32      u32;                    /**< [TM] uint32 value*/
        uint64      u64;                    /**< [TM] uint64 value*/
        mac_addr_t  mac;                    /**< [TM] mac address*/
        ip_addr_t   ipv4;                   /**< [TM] ipv4 address*/
        ipv6_addr_t ipv6;                   /**< [TM] ipv4 address*/
        char        str[CTC_DIAG_TRACE_STR_LEN];    /**< [TM] string value*/
        uint8       pkt_hdr[CTC_DIAG_PACKET_HEAR_LEN];           /**< [TM] packet header*/
    }val;
};
typedef struct ctc_diag_pkt_trace_result_info_s ctc_diag_pkt_trace_result_info_t;

/**
 @brief packet trace position enum
*/
enum ctc_diag_pkt_trace_position_e
{
    CTC_DIAG_TRACE_POS_IPE = 0,     /**< [TM] IPE trace position*/
    CTC_DIAG_TRACE_POS_TM,          /**< [TM] TM trace position*/
    CTC_DIAG_TRACE_POS_EPE,         /**< [TM] EPE trace position*/
    CTC_DIAG_TRACE_POS_OAM,         /**< [TM] OAM trace position*/
    CTC_DIAG_TRACE_POS_MAX
};
typedef enum ctc_diag_pkt_trace_position_e ctc_diag_pkt_trace_position_t;

/**
 @brief packet trace dest type
*/
enum ctc_diag_dest_type_e
{
    CTC_DIAG_DEST_NONE = 0,     /**< [TM] none destination*/
    CTC_DIAG_DEST_NETWORK,      /**< [TM] packet destination is network port*/
    CTC_DIAG_DEST_CPU,          /**< [TM] packet destination is CPU port*/
    CTC_DIAG_DEST_DROP,         /**< [TM] packet droped*/
    CTC_DIAG_DEST_OAM           /**< [TM] packet destination is OAM engine*/
};
typedef enum ctc_diag_dest_type_e ctc_diag_dest_type_t;

/**
 @brief packet trace loop flag
*/
enum ctc_diag_loop_flag_e
{
    CTC_DIAG_LOOP_FLAG_ILOOP = 0x1,     /**< [TM] iloop flag*/
    CTC_DIAG_LOOP_FLAG_ELOOP = 0x2,     /**< [TM] eloop flag*/

    CTC_DIAG_LOOP_FLAG_MAX
};
typedef enum ctc_diag_loop_flag_e ctc_diag_loop_flag_t;

/**
 @brief packet trace result valid flag
*/
typedef enum ctc_diag_trace_result_flag_e
{
    CTC_DIAG_TRACE_RSTL_FLAG_PORT        = 0x00000001,  /**< [TM] port valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_CHANNLE     = 0x00000002,  /**< [TM] channel valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP    = 0x00000004,  /**< [TM] mc_group valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_TID         = 0x00000008,  /**< [TM] tid valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_ECMP_GROUP  = 0x00000010,  /**< [TM] ecmp_group valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_L3IF_ID     = 0x00000020,  /**< [TM] l3if_id valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_DROP_REASON = 0x00000040,  /**< [TM] drop_reason valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_DROP_DESC   = 0x00000080,  /**< [TM] drop_desc valid flag*/
    CTC_DIAG_TRACE_RSTL_FLAG_EXCP_VECTOR = 0x00000100   /**< [TM] excp_vector valid flag*/

}ctc_diag_trace_result_flag_t;

/**
 @brief packet trace result exception vector bit
*/
typedef enum ctc_diag_trace_exception_vector_e
{
    CTC_DIAG_EXCP_VEC_EXCEPTION = 0,    /**< [TM] exception bit*/
    CTC_DIAG_EXCP_VEC_L2_SPAN,      /**< [TM] l2 span bit*/
    CTC_DIAG_EXCP_VEC_L3_SPAN,      /**< [TM] l3 span bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG0,     /**< [TM] acl log0 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG1,     /**< [TM] acl log1 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG2,     /**< [TM] acl log2 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG3,     /**< [TM] acl log3 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG4,     /**< [TM] acl log4 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG5,     /**< [TM] acl log5 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG6,     /**< [TM] acl log6 bit*/
    CTC_DIAG_EXCP_VEC_ACL_LOG7,     /**< [TM] acl log7 bit*/
    CTC_DIAG_EXCP_VEC_PORT_LOG,     /**< [TM] port log bit*/
    CTC_DIAG_EXCP_VEC_IPFIX_MIRROR, /**< [TM] ipfix mirror bit*/
    CTC_DIAG_EXCP_VEC_IPFIX_CRITICAL_PKT_LOG,   /**< [TM] ipfix critical packet log bit*/
    CTC_DIAG_EXCP_VEC_POSTCARD_LOG, /**< [TM] postcard log bit*/
    CTC_DIAG_EXCP_VEC_DEBUGSESSION_LOG, /**< [TM] debug session log bit*/
    CTC_DIAG_EXCP_VEC_ENQDROPSPANDEST,  /**< [TM] enqueue drop span destination bit*/
    CTC_DIAG_EXCP_VEC_DISCARD_PACKET_LOG,   /**< [TM] drop packet log bit*/
    CTC_DIAG_EXCP_VEC_ELEPHANTFLOW_LOG,     /**< [TM] elephant flow log bit*/
    CTC_DIAG_EXCP_VEC_CPU_RX_LOG,           /**< [TM] cpu rx log bit*/
    CTC_DIAG_EXCP_VEC_CPU_TX_LOG            /**< [TM] cpu tx log bit*/
}ctc_diag_trace_exception_vector_t;

/**
 @brief Define packet trace result structure
*/
struct ctc_diag_pkt_trace_result_s
{
    uint32    watch_point;      /**< [TM] [in]specified watch point and the result info will start from it, refer to ctc_diag_pkt_trace_watch_point_t*/
    uint32    port;             /**< [TM] [out]network port id*/
    uint16    channel;          /**< [TM] [out]network channel id*/
    uint32    mc_group;         /**< [TM] [out]multicast group id*/
    uint16    tid;              /**< [TM] [out]linkagg group id*/
    uint16    ecmp_group;       /**< [TM] [out]ecmp group id*/
    uint16    l3if_id;          /**< [TM] [out]l3 interface id*/
    uint32    excp_vector;      /**< [TM] [out]exception vector bitmap ,refer to ctc_diag_trace_exception_vector_t*/
    uint16    drop_reason;      /**< [TM] [out]drop reason*/
    char      drop_desc[CTC_DIAG_REASON_DESC_LEN];  /**< [TM] [out]drop reason description*/
    uint8     position;         /**< [TM] [out]the latest trace position, refer to ctc_diag_pkt_trace_position_t*/
    uint8     loop_flags;       /**< [TM] [out]trace packet loop flags, refer to ctc_diag_loop_flag_t*/
    uint32    dest_type;        /**< [TM] [out]trace packet destination in ASIC, refer to ctc_diag_dest_type_t*/
    uint32    flags;            /**< [TM] [out]structure's member valid flags, refer to ctc_diag_trace_result_flag_t*/
    uint32    count;            /**< [TM] [in]info count, input by user*/
    ctc_diag_pkt_trace_result_info_t* info; /**< [TM] [in/out]result info node which should malloc by user with size of count * sizeof(ctc_diag_pkt_trace_result_info_t)*/
    uint32    real_count;       /**< [TM] [out]output real info count*/
};
typedef struct ctc_diag_pkt_trace_result_s ctc_diag_pkt_trace_result_t;

/**
 @brief drop info operation type enum
*/
typedef enum ctc_diag_drop_oper_type_e
{
    CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP,    /**< [TM] get all droped packet network port bitmap*/
    CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP,  /**< [TM] get drop reason bitmap based on a drop port*/
    CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO  /**< [TM] get drop detail info based on drop port and reason*/
}ctc_diag_drop_oper_type_t;

/**
 @brief Define drop packet buffer structure
*/
typedef struct ctc_diag_drop_info_buffer_s
{
    uint32 len;             /**< [TM] [in]packet buffer length, units:byte*/
    sal_time_t timestamp;   /**< [TM] [out]timestamp of the drop packet*/
    uint8* pkt_buf;         /**< [TM] [in/out]packet buffer, should malloc by user*/
    uint32 real_len;        /**< [TM] [out]the drop packet real length*/
}ctc_diag_drop_info_buffer_t;

/**
 @brief Define drop info structure
*/
struct ctc_diag_drop_info_s
{
    uint32 count;           /**< [TM] [out]drop statistics, based on drop reason or port and drop reason*/
    char   reason_desc[CTC_DIAG_REASON_DESC_LEN];   /**< [TM] [out]drop reason description*/
    uint8  position;        /**< [TM] [out]drop position*/
    uint8  rsv[3];
    uint32 buffer_count;    /**< [TM] [in]buffer count*/
    ctc_diag_drop_info_buffer_t* buffer;    /**< [TM] [in/out]drop buffer, size = buffer_count * sizeof(ctc_diag_drop_info_buffer_t)*/
    uint32 real_buffer_count;/**< [TM] [out]real drop buffer count, indicate exist more than one difference droped packet*/
};
typedef struct ctc_diag_drop_info_s ctc_diag_drop_info_t;

/**
 @brief Define drop reason bitmap length
*/
#define CTC_DIAG_DROP_REASON_BMP_LEN     ((CTC_DIAG_DROP_REASON_MAX + CTC_UINT32_BITS) / CTC_UINT32_BITS)


/**
 @brief Define drop structure
*/
struct ctc_diag_drop_s
{
    /*key*/
    uint8  with_clear;  /**< [TM] [in]clear drop packet buffer after obtained the whole packet, only valid when oper_type==CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO*/
    uint8  rsv[3];
    uint16 reason;      /**< [TM] [in]drop reason*/
    uint16 lport;       /**< [TM] [in]local port id, 0xFFFF means chip global level's drop info*/
    uint32 oper_type;   /**< [TM] [in]operation type, refer to ctc_diag_drop_oper_type_t*/

    /*data*/
    union
    {
        uint32                  reason_bmp[CTC_DIAG_DROP_REASON_BMP_LEN];   /**< [TM] drop reason bitmap, when oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP*/
        ctc_port_bitmap_t       port_bmp;   /**< [TM] drop port bitmap, when oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP*/
        ctc_diag_drop_info_t    info;       /**< [TM] drop detail info, when oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO*/
    }u;
};
typedef struct ctc_diag_drop_s ctc_diag_drop_t;

/**
 @brief Define drop packet active report callback function type
*/
typedef int32 (*ctc_diag_drop_pkt_report_cb)(uint32 gport, uint16 reason, ctc_diag_drop_info_buffer_t* pkt_buf); /**< [TM] active report drop packet callback function*/

typedef enum ctc_diag_drop_pkt_config_flag_e
{
    CTC_DIAG_DROP_PKT_HASH_CALC_LEN  = 0x1,   /**< [TM] hash_calc_len valid flag*/
    CTC_DIAG_DROP_PKT_STORE_EN = 0x2,    /**< [TM] storage_en valid flag*/
    CTC_DIAG_DROP_PKT_STORE_LEN = 0x4,   /**< [TM] storage_len valid flag*/
    CTC_DIAG_DROP_PKT_STORE_CNT = 0x8,   /**< [TM] storage_cnt valid flag*/
    CTC_DIAG_DROP_PKT_OVERWRITE = 0x10,    /**< [TM] overwrite valid flag*/
    CTC_DIAG_DROP_PKT_CLEAR = 0x20,       /**< [TM] if set, clear all stored packet*/
    CTC_DIAG_DROP_PKT_REPORT_CALLBACK = 0x40, /**< [TM] drop packet report callback function*/
}ctc_diag_drop_pkt_config_flag_t;


struct ctc_diag_drop_pkt_config_s
{

    uint32 flags;           /**< [TM] config flags, refer to ctc_diag_drop_pkt_config_flag_t*/
    uint32 hash_calc_len;   /**< [TM] drop packet length which part in hash calculation to distinguish the difference packet, default value is 32*/
    uint32 storage_len;     /**< [TM] drop packet storage length, default value is 256*/
    uint32 storage_cnt;     /**< [TM] drop packet storage count, default value is 128*/
    bool   storage_en;      /**< [TM] drop packet storage enable or disable*/
    bool   overwrite;       /**< [TM] if true, indicate it will ovewrite the oldest drop packet buffer when it's statistics exceed the storage count*/
    ctc_diag_drop_pkt_report_cb report_cb; /**< [TM] drop packet report callback function*/
};
typedef struct ctc_diag_drop_pkt_config_s ctc_diag_drop_pkt_config_t;


/**
 @brief diag module property enum
*/
typedef enum ctc_diag_property_e
{
    CTC_DIAG_PROP_DROP_PKT_CONFIG,      /**< [TM] drop packet global config, refer to ctc_diag_drop_pkt_config_t*/
    CTC_DIAG_PROP_MAX
}ctc_diag_property_t;

struct ctc_diag_global_cfg_s
{
};
typedef struct ctc_diag_global_cfg_s ctc_diag_global_cfg_t;

/**@} end of @defgroup  diag DIAG */

#ifdef __cplusplus
}
#endif

#endif  /*_CTC_DIAG_H*/

