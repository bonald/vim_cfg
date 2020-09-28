#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_ipfix.h

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-17

 @version v3.0

\p
The IP Flow Information Export (IPFIX) provide a way to collect packet and byte counts (received and
transmitted) for traffic flows in the switch. A flow is identified by matching certain header fields from each
packet, which is per-port control. Every packet with the same set of header fields is considered part of a
given flow. IPFIX provides data to enable network and security monitoring, network planning, traffic analysis,
and IP accounting.

\b
IPFIX data for identified flow can be exported due to different reasons:
\d Exported due to packet count overflow
\d Exported due to packet bytes overflow
\d Exported due to timestamp overflow
\d Exported due to expired
\d Exported due to TCP closed/rst packet receive

\p
IPFIX have two mode, which is software learning/aging and hardware learning/aging
\p
1)software learning/aging
In this mode, IPFIX flow entry is added by CPU using API ctc_ipfix_add_entry, and deleted by CPU
using API ctc_ipfix_delete_entry. ASIC only do update operation for known flow entry, for unknown
flow do nothing.
\b

\p
2)hardware learning/aging
In this mode, IPFIX flow entry is added by ASIC, also updated by ASIC and aginged by ASIC. CPU cannot
do add and delete.
\b

\p
IPFIX have sampling function, which can maximal monitoring traffic based numbered resource. User can
configure sample_interval through API ctc_ipfix_set_port_cfg. For example, if config sample_interval as 10,
means  do one flow learning for every ten new flow detected.

\S ctc_ipfix.h:ctc_ipfix_data_t
\S ctc_ipfix.h:ctc_ipfix_hash_field_sel_t
\S ctc_ipfix.h:ctc_ipfix_port_cfg_t
\S ctc_ipfix.h:ctc_ipfix_global_cfg_t
\S ctc_ipfix.h:ctc_ipfix_hash_ipv4_field_sel_t
*/

#ifndef _CTC_USW_IPFIX_H
#define _CTC_USW_IPFIX_H
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
#include "ctc_ipfix.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @addtogroup ipfix IPFIX
 @{
*/

/**
 @brief Initialize the IPFIX module

 @param[in] lchip    local chip id

 @param[in]  p_global_cfg Ipfix global configuration

 @remark[D2.TM]    Initialize the ipfix module and set default config.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize ipfix module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the ipfix configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_ipfix_deinit(uint8 lchip);

/**
 @brief Set IPFIX hash select field

 @param[in] lchip    local chip id

 @param[in] field_sel   hash select field

 @remark[D2.TM] Set IPFIX hash select field

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_set_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel);

/**
 @brief Set global config for IPFIX

 @param[in] lchip    local chip id

 @param[in] ipfix_cfg   Ipfix global configuration

 @remark[D2.TM]   Before using this interface to set global configuration, need call ctc_ipfix_get_global_cfg
    to get current global configuration.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_set_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t* ipfix_cfg);

/**
 @brief Get global config for IPFIX

 @param[in] lchip    local chip id

 @param[out] ipfix_cfg   Current Ipfix global configuration

 @remark[D2.TM] Get global configuration

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_get_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t* ipfix_cfg);

/**
 @brief Set flow config for IPFIX

 @param[in] lchip   lchip

 @param[in] flow_cfg   Current Ipfix flow configuration

 @remark[D2.TM] Set flow configuration

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_set_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg);

/**
 @brief Get global config for IPFIX

 @param[in] lchip   lchip

 @param[out] flow_cfg   Current Ipfix flow configuration

 @remark[D2.TM] Get flow configuration

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_get_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg);
/**
 @brief Register Ipfix callback function

 @param[in] lchip    local chip id

 @param[in] callback   callback function

 @param[in] userdata   userdata need sdk to feed back

 @remark[D2.TM] Get global configuration

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_register_cb(uint8 lchip, ctc_ipfix_fn_t callback,void* userdata);

/**
 @brief Cpu add ipfix key

 @param[in] lchip    local chip id

 @param[in] p_data   ipfix key information

 @remark[D2.TM] This interface is only used in sw learning mode

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_add_entry(uint8 lchip, ctc_ipfix_data_t* p_data);

/**
 @brief Cpu delete ipfix key

 @param[in] lchip    local chip id

 @param[in] p_data   ipfix key information

 @remark[D2.TM] This interface is only used in sw learning mode

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_delete_entry(uint8 lchip, ctc_ipfix_data_t* p_data);

/**
 @brief Ipfix traverse interface

 @param[in] lchip    local chip id

 @param[in] fn   User callback function to process ipfix entry

 @param[in] p_data   traverse parameter

 @remark[D2.TM] This interface is used to traverse ipfix entry according given condition

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_traverse(uint8 lchip, ctc_ipfix_traverse_fn fn, ctc_ipfix_traverse_t* p_data);
/**
 @brief Ipfix traverse and remove interface

 @param[in] lchip    local chip id

 @param[in] fn   User callback function to decide to whether remove ipfix entry

 @param[in] p_data   traverse parameter

 @remark[D2.TM] This interface is used to traverse ipfix entry and remove a spcific entry according given codition

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipfix_traverse_remove(uint8 lchip, ctc_ipfix_traverse_remove_cmp fn, ctc_ipfix_traverse_t* p_data);
/**@} end of @addgroup   */
#ifdef __cplusplus
}
#endif

#endif

#endif

