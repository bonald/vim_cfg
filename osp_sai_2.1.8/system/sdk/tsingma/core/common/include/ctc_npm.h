#if (FEATURE_MODE == 0)
/**
 @file ctc_npm.h

 @date 2016-4-20

 @version v1.0

*/

#ifndef _CTC_NPM_H
#define _CTC_NPM_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_NPM_MAX_EMIX_NUM 16

/*
 @brief   Define npm cfg flags
*/
enum ctc_npm_cfg_flag_e
{
    CTC_NPM_CFG_FLAG_TS_EN             = 1U << 0,  /**< [D2.TM] Enable insert transmit timestamp, for delay measurement */
    CTC_NPM_CFG_FLAG_SEQ_EN            = 1U << 1,  /**< [D2.TM] Enable insert sequence number */
    CTC_NPM_CFG_FLAG_ILOOP             = 1U << 2,  /**< [D2.TM] Indicate key for up MEP */
    CTC_NPM_CFG_FLAG_BURST_EN          = 1U << 3,  /**< [D2.TM] Enable generator traffic in burst mode */
    CTC_NPM_CFG_FLAG_MCAST             = 1U << 4,  /**< [D2.TM] Indicate Destination global port is mcast dest-group-id */
    CTC_NPM_CFG_FLAG_DMR_NOT_TO_CPU    = 1U << 5,  /**< [D2.TM] Indicate session use DMM/DMR for NPM test, DMR not to CPU*/
    CTC_NPM_CFG_FLAG_RX_ROLE_EN        = 1U << 6,  /**< [D2.TM] Only enable rx role */
    CTC_NPM_CFG_FLAG_NTP_TS            = 1U << 7,  /**< [TM] If set, indicate timestamp use NTP, else PTPv2*/
    CTC_NPM_CFG_FLAG_MAX
};
typedef enum ctc_npm_cfg_flag_e ctc_npm_cfg_flag_t;

/**
 @brief  Define npm packet format
 Note1: Support below packet type
   |   Packet Type    |                       Descroption                |                                   Remark                                |
   |------------------|--------------------------------------------------|-------------------------------------------------------------------------|
   |    DMM/DMR       |ITU.T Y.1731,    Frame Delay metrics              |Test based on MEP, config MEP to identify test pkt and send to OAM engine|
   |    SLM/SLR       |Based on Y.1731, Frame Delay and Loss metrics     |Test based on MEP, config MEP to identify test pkt and send to OAM engine|
   |    LBM/LBR       |Based on Y.1731, Frame Loss metrics               |Test based on MEP, config MEP to identify test pkt and send to OAM engine|
   |    ETH TST       |Based on Y.1731, Frame Loss metrics               |Test based on MEP, config MEP to identify test pkt and send to OAM engine|
   |    OWAMP         |RFC4656 , One-Way Frame Delay and Loss metrics    |must use ACL to identify as CTC_ACL_OAM_TYPE_FLEX, and send to NPM engine|
   |    TWAMP         |RFC5357 , Two-Way Frame Delay and Loss metrics    |must use ACL to identify as CTC_ACL_OAM_TYPE_FLEX, and send to NPM engine|
   |    UDP-ECHO      |Frame Delay and Loss metrics                      |must use ACL to identify as CTC_ACL_OAM_TYPE_FLEX, and send to NPM engine|
   |    FL-PDU        |MEF 49, One-Way Frame Loss metrics                |must use ACL to identify as CTC_ACL_OAM_TYPE_FLEX, and send to NPM engine|
   |    FLEX          |User defined packet type, Frame Loss metrics      |must use ACL to identify as CTC_ACL_OAM_TYPE_FLEX, and send to NPM engine|
 Note2: When Tx NPM test PDUs, encape different NPM test PDUs to ctc_npm_pkt_format_t.pkt_header,(DMM/DMR,SLM/SLR,OWAMP..)
        use ctc_npm_cfg_t to set features of test PDU. For example, ts_offset and seq_num_offset are
        used to add timestamp and seq num in test PDU.(such as OWAMP/TWAMP, seq_num_offset=42 ts_offset=46)
 Note3: When Rx NPM test PDUs and using OWAMP/TWAMP/UDP Echo/FL-PDU/FLEX to do KPI metrics.
        If doing two-way test such as TWAMP/UDP Echo, receiever device just reflect receieved PDUs to sender device.
        If doing one-way test such as OWAMP/FLEX, reveiver device need set CTC_NPM_CFG_FLAG_RX_ROLE_EN and config ts_offset/seq_num_offset
        to get timestamp and seq num from receieved PDUs.
 Note4: When using FL-PDU and FD-PDU to do KPI metrics, need creat two test sessions at the same time (refer to MEF 49)
 Note5: Detail config please refer to <<SDKTypicalConfiguration_APP>>
*/
struct ctc_npm_pkt_format_s
{
    void*  pkt_header;         /**< [D2.TM] User-defined data, used to encape NPM Test PDU, Max size refer to ctc_npm_session_mode_t */
    uint16 header_len;         /**< [D2.TM] User-defined data length, not include crc */
    uint8  pattern_type;        /**< [D2.TM] Payload pattern type, refer to ctc_npm_pattern_type_t */
    uint8  ipg;                /**< [D2.TM] Inter-packet gap, should be same as mac config */

    uint16 seq_num_offset;     /**< [D2.TM] Offset sequence number in user-defined packet when CTC_NPM_CFG_FLAG_SEQ_EN flag is set*/
    uint16 ts_offset;          /**< [D2.TM] Offset in bytes of timestamp when CTC_NPM_CFG_FLAG_TS_EN flag is set */
    uint32 repeat_pattern;     /**< [D2.TM] Used for repeat payload type */

    uint8  frame_size_mode;           /**< [D2.TM] Packet size mode, 0: Fixed, 1: Increased, 2:Emix*/
    uint8  emix_size_num;             /**< [D2.TM] Used when size_mode is emix, means emix size num*/
    uint16 emix_size[CTC_NPM_MAX_EMIX_NUM];  /**< [D2.TM] emix size, emix size refer to global cfg for emix size*/
    uint32 frame_size;               /**< [D2.TM] Total packet size, include header_len and crc, must >= 64 && >= header_len + 4 bytes,
                                 when size_mode is fixed, means packet size; when size_mode is Increased, means max packet size*/
    uint32 min_frame_size;           /**< [D2.TM] Min packet size when size_mode is Increased */
};
typedef struct ctc_npm_pkt_format_s ctc_npm_pkt_format_t;

/**
 @brief  Define npm cfg parameter, determine features of NPM test PDUs.
*/
struct ctc_npm_cfg_s
{
    uint32 flag;                /**< [D2.TM] refer to ctc_npm_cfg_flag_t */

    uint8 session_id;           /**< [D2.TM] NPM session id */
    uint8 tx_mode;              /**< [D2.TM] Tx mode, refer to ctc_npm_tx_mode_t */
    uint8 timeout;              /**< [D2.TM] Enable timeout for On-demand test, 0 means disable, unit:s */
    uint8 rx_role_en;           /**< [D2.TM] If set, indicate rx roler enable, when CTC_NPM_CFG_FLAG_RX_ROLE_EN flag is set */

    uint32 rate;                /**< [D2.TM] Generate traffic speed(Kbps) */
    uint32 tx_period;           /**< [D2.TM] Used for generate pkts when in tx CTC_NPM_TX_MODE_PERIOD mode, unit: 1s */
    uint32 packet_num;          /**< [D2.TM] Used for generate pkts when in tx CTC_NPM_TX_MODE_PACKET mode */

    uint32 burst_cnt;           /**< [D2.TM] burst cnt when burst enable*/
    uint32 ibg;                 /**< [D2.TM] Inter-Burst Gap, used for burst packet, uint:us, 0: using default*/

    uint32 dest_gport;          /**< [D2.TM] Destination global port ID */
    uint16 vlan_id;             /**< [D2.TM] Vlan id */
    uint8  vlan_domain;         /**< [D2.TM] Vlan domain, when set 1, indicate cvlan-doamin,else svlan-domain */
    uint8  dm_stats_mode;       /**< [TM] Only valid for two-way delay measure (such as DMM/DMR,...), 0 store two way delay stats;
                                      1: store far end delay stats; 2.store near end delay stats */

    ctc_npm_pkt_format_t pkt_format;  /**< [D2.TM] npm packet format */

};
typedef struct ctc_npm_cfg_s ctc_npm_cfg_t;


enum ctc_npm_tx_mode_e
{
    CTC_NPM_TX_MODE_CONTINUOUS = 0,  /**< [D2.TM] Transmit continuously */
    CTC_NPM_TX_MODE_PACKET_NUM,      /**< [D2.TM] Stop packets transmission after the number of packet transmission */
    CTC_NPM_TX_MODE_PERIOD,          /**< [D2.TM] Stop packets transmission after a period of packet transmission */

    CTC_NPM_TX_MODE_MAX
};
typedef enum ctc_npm_tx_mode_e ctc_npm_tx_mode_t;

/*
 @brief   Define NPM generator pattern type
*/
enum ctc_npm_pattern_type_e
{
    CTC_NPM_PATTERN_TYPE_REPEAT,       /**< [D2.TM] Repeat mode */
    CTC_NPM_PATTERN_TYPE_PRBS,         /**< [D2.TM] Random mode */
    CTC_NPM_PATTERN_TYPE_INC_BYTE,     /**< [D2.TM] Increase by byte mode */
    CTC_NPM_PATTERN_TYPE_DEC_BYTE,     /**< [D2.TM] Decrease by byte mode */
    CTC_NPM_PATTERN_TYPE_INC_WORD,     /**< [D2.TM] Increase by word mode */
    CTC_NPM_PATTERN_TYPE_DEC_WORD,     /**< [D2.TM] Decrease by word mode */

    CTC_NPM_PATTERN_TYPE_MAX
};
typedef enum ctc_npm_pattern_type_e ctc_npm_pattern_type_t;

/*
 @brief   Define NPM stats structure, used to calculate KPI metrics
 Note1: When get autogen stats, if session transmit is enable(tx_en=1), num of tx_pkts or rx_pkts have deviation,
 If wnat to calulate frame lost, session transmit must be disable(tx_en=0).
*/
struct ctc_npm_stats_s
{
    uint64 total_delay;      /**< [D2.TM] total delay for test session, used to calculate average Frame Delay, uint:ns, for
                             DMR/SLR/TWAMP means two-way total delay, for 1DM/OWAMP means one-way total delay*/
    uint64 total_near_delay; /**< [D2.TM] only used for DMR/SLR/TWAMP to calculate near direction delay, uint:ns */
    uint64 total_far_delay;  /**< [D2.TM] only used for DMR/SLR/TWAMP to calculate far direction delay, uint:ns */
    uint64 first_ts;         /**< [D2.TM] 1st receive packet timestamp, uint:ns */
    uint64 last_ts;          /**< [D2.TM] last receive packet timestamp, uint:ns */

    uint64 tx_fcf;           /**< [D2.TM] Tx fcf, used for SLR/1SL */
    uint64 tx_fcb;           /**< [D2.TM] Tx fcb, used for SLR */
    uint64 rx_fcl;           /**< [D2.TM] Rx fcl, used for SLR/1SL */

    uint64 tx_pkts;          /**< [D2.TM] packet cnt stats in Tx Dir */
    uint64 rx_pkts;          /**< [D2.TM] packet cnt stats in Rx Dir */
    uint64 tx_bytes;         /**< [D2.TM] packet bytes stats in Tx Dir */
    uint64 rx_bytes;         /**< [D2.TM] packet bytes stats in Rx Dir */
    uint8  tx_en;            /**< [D2.TM] indicate autogen session transmit enable */
    uint64 max_delay;        /**< [TM] max delay stats, uint:ns*/
    uint64 min_delay;        /**< [TM] min delay stats, uint:ns*/
    uint32 max_jitter;       /**< [TM] max jitter stats, uint:ns*/
    uint32 min_jitter;       /**< [TM] min jitter stats, uint:ns*/
    uint64 total_jitter;     /**< [TM] total jitter stats, uint:ns*/
};
typedef struct ctc_npm_stats_s ctc_npm_stats_t;

enum ctc_npm_session_mode_e
{
   CTC_NPM_SESSION_MODE_8,   /**<[D2.TM] Support max session number is 8, max header_len per session is 96bytes*/
   CTC_NPM_SESSION_MODE_6,   /**<[D2.TM] Support max session number is 6, max header_len per session is 128bytes*/
   CTC_NPM_SESSION_MODE_4,   /**<[D2.TM] Support max session number is 4, max header_len per session is 192bytes*/

   CTC_NPM_SESSION_MODE_MAX
};
typedef enum ctc_npm_session_mode_e ctc_npm_session_mode_t;

/*
 @brief   Define NPM global config parameter
*/
struct ctc_npm_global_cfg_s
{
    uint8 session_mode;       /**< [D2.TM] refer to ctc_npm_session_mode_t*/
    uint8 rsv[3];

    uint16 emix_size[CTC_NPM_MAX_EMIX_NUM];  /**< [D2.TM] emix size array, can support max 16 frame size for emix mode*/
};
typedef struct ctc_npm_global_cfg_s ctc_npm_global_cfg_t;

#ifdef __cplusplus
}
#endif

#endif

#endif

