/**
 @file ctc_efd.h

 @author  Copyright (C) 2014 Centec Networks Inc.  All rights reserved.

 @date 2014-10-28

 @version v3.0

   This file contains all efd related data structure, enum, macro and proto.
*/

#ifndef _CTC_EFD_H
#define _CTC_EFD_H
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

#define CTC_EFD_FLOW_ID_ARRAY_MAX       64

/**
 @brief  global control type
*/
enum ctc_efd_global_control_type_e
{
    CTC_EFD_GLOBAL_MODE_TCP,               /**< [GG.D2.TM]Only TCP packets do EFD. value: bool*, TRUE or FALSE */
    CTC_EFD_GLOBAL_PRIORITY_EN,            /**< [GG]When enable, the elephant flow will use the configed priority. value: bool*, TRUE or FALSE */
    CTC_EFD_GLOBAL_PRIORITY,                 /**< [GG]Config global priority for the elephant flow.value: <0-15> */
    CTC_EFD_GLOBAL_COLOR_EN,               /**< [GG]When enable, the elephant flow will use the configed color. value: bool*, TRUE or FALSE */
    CTC_EFD_GLOBAL_COLOR,                    /**< [GG]Config global color for the elephant flow. value: 1-red 2-yellow 3-green */
    CTC_EFD_GLOBAL_MIN_PKT_LEN,          /**< [GG.D2.TM]Only packet length > MIN_PKT_LEN value do EFD. value:  0-0x3fff, 0 means all packet length do EFD */
    CTC_EFD_GLOBAL_DETECT_RATE,          /**< [GG.D2.TM]Min flow rate to detect as Elephant Flow, unit:kbps. */
    CTC_EFD_GLOBAL_IPG_EN,                   /**< [GG.D2.TM]Ipg is considered in EFD. value: bool*, TRUE or FALSE*/
    CTC_EFD_GLOBAL_EFD_EN,                 /**< [D2.TM]Config global EFD eanble. value: bool*, TRUE or FALSE default is enable*/
    CTC_EFD_GLOBAL_PPS_EN,                 /**< [D2.TM]Do EFD by pps. value: bool*, TRUE:pps ,FALSE:bps*/
    CTC_EFD_GLOBAL_FUZZY_MATCH_EN,         /**< [D2.TM]Do EFD using fuzzy match mode. value: bool*, TRUE:fuzzy match mode£¬no limit to flow table ,FALSE:exact match mode£¬limit to flow table*/
    CTC_EFD_GLOBAL_RANDOM_LOG_EN,          /**< [D2.TM]When enable, the elephant flow will use the configed log to cpu. value: bool*, TRUE or FALSE */
    CTC_EFD_GLOBAL_RANDOM_LOG_RATE,        /**< [D2.TM]Config Threshold rate to cpu.percent:1/(2^(15-rate)) value: <0-15> */
    CTC_EFD_GLOBAL_DETECT_GRANU,            /**< [GG.D2.TM]Packet length update granularity for detect, value: <0-3>, 0:16Byte, 1:8Byte, 2:4Byte, 3:32Byte*/
    CTC_EFD_GLOBAL_DETECT_INTERVAL,           /**< [GG.D2.TM]Detect efd flow (learning) and aging time interval, unit:1ms*/
    CTC_EFD_GLOBAL_AGING_MULT,      /**< [GG.D2.TM]Aging multiplier, value: 0-7, aging time=DetectInterval*AgingMult */
    CTC_EFD_GLOBAL_REDIRECT,                /**< [GG.D2.TM] Config efd flow redirect nexthop id, value: uint32* array, array[0] is flowId, array[1] is nhId;d2/tm only support global redirect */
    /*SYSTEM MODIFIED by xgu, added SDK 3.5 in ggsys trunk code, 2016-6-29*/
    CTC_EFD_GLOBAL_REDIRECT_NHID,  /**< [GG]Redirect NHID*/
    /*SYSTEM MODIFIED by xgu, merged from SDK 3.5 in ggsys trunk code and add more function, system bug 38251, 2016-6-27, end*/
    CTC_EFD_GLOBAL_FLOW_ACTIVE_BMP,        /**< [GG.D2.TM] Get active elephant flow bitmap based on flow id, value: uint32* bitmap, should at least have 2K bits in GG, and 1k bits in D2 */

    CTC_EFD_GLOBAL_CONTROL_MAX
};
typedef enum ctc_efd_global_control_type_e ctc_efd_global_control_type_t;


/**
@brief  efd data of hardware
*/
struct ctc_efd_data_s
{
    uint16 count;                                                             /**<[GG.D2.TM]Flow id number*/
    uint16 rsv;
    uint32 flow_id_array[CTC_EFD_FLOW_ID_ARRAY_MAX];    /**<[GG.D2.TM]Use array to store flow id */
};
typedef struct ctc_efd_data_s ctc_efd_data_t;

/**
@brief  callback of efd for aging
*/
typedef void (* ctc_efd_fn_t)(ctc_efd_data_t* info, void* userdata) ;  /**< [GG.D2.TM]Callback function to get aging flowId from DMA */


#ifdef __cplusplus
}
#endif

#endif

