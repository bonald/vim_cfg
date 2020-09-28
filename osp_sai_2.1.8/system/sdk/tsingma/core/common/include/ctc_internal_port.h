/**
  @file ctc_internal_port.h

  @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

  @date 2010-03-29

  @version v2.0

    This file contains all internal port related data structure, enum, macro and proto.
 */

#ifndef _CTC_INTERNAL_PORT_H
#define _CTC_INTERNAL_PORT_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup internal_port INTERNAL_PORT
 @{
*/

/**
 @brief  Define internal port usage type
*/
enum ctc_internal_port_type_e
{
    CTC_INTERNAL_PORT_TYPE_ILOOP,      /**< [GB.GG.D2.TM] I-Loopback port */
    CTC_INTERNAL_PORT_TYPE_ELOOP,     /**< [GB.GG.D2.TM] E-Loopback port */
    CTC_INTERNAL_PORT_TYPE_DISCARD,  /**< [GB.GG.D2.TM] Discard port */
    CTC_INTERNAL_PORT_TYPE_FWD,        /**< [GB.GG.D2.TM] Forwarding port, used for BPE */
    CTC_INTERNAL_PORT_TYPE_WLAN       /**< [D2.TM] WLAN port*/
};
typedef enum ctc_internal_port_type_e ctc_internal_port_type_t;

/**
 @brief Internal port assign parameters.
*/
struct ctc_internal_port_assign_para_s
{
    ctc_internal_port_type_t type;                /**< [GB.GG.D2.TM] internal port type */
    uint8  gchip;                                        /**< [GB.GG.D2.TM] global chipId */
    uint8  slice_id;                                     /**< [GG.D2.TM] slice Id for alloc CTC_INTERNAL_PORT_TYPE_ILOOP and CTC_INTERNAL_PORT_TYPE_ELOOP port */
    uint16 inter_port;                                 /**< [GB.GG.D2.TM] the internal port which is assigned or setted */
    uint32 fwd_gport;                                 /**< [GB.GG.D2.TM] this is the dest port of FWD type */
    uint32 nhid;                                          /**< [GB.GG.D2.TM] Nexthop information for CTC_INTERNAL_PORT_TYPE_ELOOP */
};
typedef struct ctc_internal_port_assign_para_s ctc_internal_port_assign_para_t;

/**@} end of @defgroup internal_port INTERNAL_PORT  */

#ifdef __cplusplus
}
#endif

#endif

