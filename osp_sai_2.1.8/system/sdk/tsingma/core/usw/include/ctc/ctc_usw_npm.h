#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_npm.h

 @author Copyright (C) 2014 Centec Networks Inc.  All rights reserved.

 @date 2014-10-28

 @version v3.0

 The Module support the new feature NPM(Network Performance Metrics). The NPM function is useful to do Network
 Performance Metrics for RFC2544 and Y.1564. Based on NPM function, and associated with QoS function, user
 can do Ethernet Service Activation Test to verify the correct configuration and performance of Carrier Ethernet
 services at the time of service activation. Also can do throughput test based on RFC2544. NPM module support
 using much types test packets to do Network Performance Metrics.
\p

\t   |   Packet Type    |                       Description                                                    |
\t   |------------------|--------------------------------------------------------------------------------------|
\t   |    DMM/DMR       |Based on Y.1731, used for L2 network Frame Delay metrics, need support OAM function  |
\t   |    SLM/SLR       |Based on Y.1731, used for L2 network Frame Delay and Loss metrics, need support OAM function  |
\t   |    LBM/LBR       |Based on Y.1731, used for L2 network Frame Loss metrics, need support OAM function  |
\t   |    TST           |Based on Y.1731, used for L2 network Frame Loss metrics, need support OAM function  |
\t   |    OWAMP         |used for L3 network, used for One-Way Frame Delay and Loss metrics   |
\t   |    TWAMP         |used for L3 network, used for Two-Way Frame Delay and Loss metrics   |
\t   |    UDP-ECHO      |used for L3 network, used for Two-Way Frame Delay and Loss metrics   |
\t   |    FL-PDU        |Based on MEF 49, used for L2 network, used for One-Way Frame Loss metrics   |
\t   |    FLEX          |User defined packet type, used for Frame Loss metrics   |

\p
The following Key Performance Indicators (KPI) metrics are collected to ensure that the configured SLAs is met for the
service/stream in NPM Module.
\p

\t   |        KPI       |                       Descroption                                                    |
\t   |------------------|--------------------------------------------------------------------------------------|
\t   | Frame Transfer Delay (FTD) or latency|Measures the round-trip or one-way time (RTT) taken by a test frame to travel through a network device or across the network and back to the test port|
\t   | Frame Loss Ratio (FLR£©|expressed as a percentage, of the number of frames not delivered divided by the total number of frames during time interval T|
\t   | Frame Delay Variation (FDV) or jitter|Measures the variations in the time delay between packet deliveries|
\t   | IR or ThroughPut |Measures the maximum rate at which none of the offered frames are dropped by the device under test. translates into the available bandwidth of the Ethernet virtual connection (EVC)|

\p
 NPM Module can support max 8 sessions, 6 sessions or 4 sessions, which can be configured through API ctc_npm_set_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm).
 How to select max session mode is depended on test packet header length.
\d        For 8 session mode, max header_len per session is 96bytes, which is NPM default mode
\d        For 6 session mode, max header_len per session is 128bytes
\d        For 4 session mode, max header_len per session is 192bytes

\p
 Notice: NPM Module If using TWAMP/OWAMP/UDP Echo and other Ip packet to do KPI metrics, pay attention to Ip Header checksum and layer4 checksum, these checksum should be calculated by user,
only packet header should be calculated, not include payload. Also during test checksum will keep no change, even for random packet size mode. Total length in Ip Header have the same attention.

\p
The module provides APIs to support ACL:
\p
\d Set global configure by ctc_npm_set_global_config()
\d Get global configure by ctc_npm_get_global_config()
\d Set transmit packet parameter by ctc_npm_set_config()
\d Set transmit packet enable by ctc_npm_set_transmit_en()
\d Get test result by ctc_npm_get_stats()

\S ctc_npm.h:ctc_npm_pkt_format_t
\S ctc_npm.h:ctc_npm_cfg_t
\S ctc_npm.h:ctc_npm_stats_t
\S ctc_npm.h:ctc_npm_global_cfg_t

*/

#ifndef _CTC_USW_NPM_H
#define _CTC_USW_NPM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_npm.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/****************************************************************
 *
* Function
*
****************************************************************/
/**
 @addtogroup npm
 @{
*/

/**
 @brief  Set npm parameter info

 @param[in] lchip_id    local chip id

 @param[in] p_cfg    npm parameter include packet format and generator parameter

 @remark[D2.TM] This function set the npm parameter.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_set_config(uint8 lchip_id, ctc_npm_cfg_t* p_cfg);

/**
 @brief  Set npm transmit enable

 @param[in] lchip_id    local chip id

 @param[in] session_id    npm session id

 @param[in] enable   enable or disable transmit

 @remark[D2.TM] This function set packet transmit enable or disable.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_set_transmit_en(uint8 lchip_id, uint8 session_id, uint8 enable);

/**
 @brief  Get npm stats

 @param[in] lchip_id    local chip id

 @param[in] session_id    npm session id

 @param[out] p_stats   npm stats

 @remark[D2.TM] This function get stats of session npm.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_get_stats(uint8 lchip_id, uint8 session_id, ctc_npm_stats_t* p_stats);


/**
 @brief  Clear npm stats

 @param[in] lchip_id    local chip id

 @param[in] session_id    npm session id

 @remark[D2.TM] This function get stats of session npm.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_clear_stats(uint8 lchip_id, uint8 session_id);


/**
 @brief  Set global config of npm

 @param[in] lchip_id    local chip id

 @param[in] p_npm    global config

 @remark[D2.TM] This function set the global config of npm.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_set_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm);

/**
 @brief  Get global config of npm

 @param[in] lchip_id    local chip id

 @param[in] p_npm    global parameter

 @remark[D2.TM] This function get the global config of npm.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_get_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm);


/**
 @brief  Npm module init

 @param[in] lchip    local chip id

 @param[in] npm_global_cfg    global parameter

 @remark[D2.TM] This function init npm module.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_init(uint8 lchip, void* npm_global_cfg);


/**
 @brief  Npm module deinit

 @param[in] lchip    local chip id

 @remark[D2.TM] This function deinit npm module.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_npm_deinit(uint8 lchip);

/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

