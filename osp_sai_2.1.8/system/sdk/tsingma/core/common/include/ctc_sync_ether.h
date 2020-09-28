/**
 @file ctc_sync_ether.h

 @author  Copyright(C) 2005-2010 Centec Networks Inc.  All rights reserved.

 @date 2009-12-15

 @version v2.0

   This file contains all stats related data structure, enum, macro and proto.
*/

#ifndef _CTC_SYNC_ETHER_H
#define _CTC_SYNC_ETHER_H
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
/**
 @defgroup sync_ether SYNC_ETHER
 @{
*/

/**
 @brief define sync ether configurations
*/
struct ctc_sync_ether_cfg_s
{
    uint16  divider;                    /**< [GB.GG.D2.TM] clock divider, GB: <0-63>, GG: <0-31>, D2: <0-31>*/
    uint8  link_status_detect_en;    /**< [GB.GG.D2.TM] indicate whether detect link status, and the clock will not output
                                             when link down if enable detect link status*/
    uint8  clock_output_en;           /**< [GB.GG.D2.TM] 1: output enable, 0: output disable */
    uint16  recovered_clock_lport;    /**< [GB.GG.D2.TM] local port ID, clock recovered from the port */
    uint8  rsv1[2];
};
typedef struct ctc_sync_ether_cfg_s ctc_sync_ether_cfg_t;

/**@} end of @defgroup  sync_ether SYNC_ETHER */

#ifdef __cplusplus
}
#endif

#endif

