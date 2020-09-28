#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_sync_ether.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2012-10-18

 @version v2.0

Synchronous Ethernet (SyncE) is expected that synchronization networks will evolve as packet network technology
replaces TDM network technology. ITU-T has specified synchronous Ethernet that allows the distribution of frequency
to be provided over Ethernet physical links.
\p
The Synchronous Ethernet protocol defines ESMC (Ethernet Synchronization Messaging Channel) to carry the SSM
(Synchronization Status Message) message. ESMC does not real exist, but a mechanism to provide this communication
ability. This message channel for synchronous Ethernet SSM is an Ethernet protocol based on an IEEE Organizational
Specific Slow Protocol (OSSP).
\p
The Ethernet SSM is an ITU-T defined Ethernet slow protocol. The IEEE has provided ITU-T with an Organizationally
Unique Identifier (OUI) and a slow protocol subtype. These are used to distinguish the Ethernet SSM PDU.
\p
The module provides API to configure the clock recovered from physical links:
\p
\d  (1)There are several physical links on the chip, so you can select which one to recover clock;
\d  (2)The recovered clock can be dividing by internal divider, which has a range from 1 to 64;
\d  (3)It has a ability to detect the link status, when the link is down, it will disable clock output to avoid clock loop.

*/

#ifndef _CTC_USW_SYNC_ETHER_H
#define _CTC_USW_SYNC_ETHER_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_sync_ether.h"
/****************************************************************************
*
* Function
*
*****************************************************************************/
/**
 @addtogroup synce SYNC_ETHER
 @{
*/

/**
 @brief  Initialize SyncE module

 @param[in] lchip    local chip id

 @param[in]  sync_ether_global_cfg

 @remark[D2.TM]  Initialize SyncE module and set default config.
 \p
          Default config:
 \p
 \d           (1)Recovered clock divider by 1;
 \d           (2)Link status detect enable;
 \d           (3)SyncE clock output disable.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_sync_ether_init(uint8 lchip, void* sync_ether_global_cfg);

/**
 @brief De-Initialize sync_ether module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the sync_ether configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_sync_ether_deinit(uint8 lchip);

/**
 @brief Set SyncE module config

 @param[in] lchip_id    local chip id
 @param[in]  sync_ether_clock_id  <0-3>
 @param[in]  p_sync_ether_cfg  Configuration of SyncE

 @remark[D2.TM]  Set SyncE module works as you expect.
 \p
 \d           (1)use p_sync_ether_cfg->divider to config the recovered clock divider;
 \d           (2)use p_sync_ether_cfg->link_status_detect_en to enable/disable link status detect. if enable, the clock will
               not output to avoid loop when physical links down;
 \d           (3)use p_sync_ether_cfg->clock_output_en to enable/disable clock output;
 \d           (4)use p_sync_ether_cfg->recovered_clock_lport to select clock recovered from whick link.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_sync_ether_set_cfg(uint8 lchip_id, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg);

/**
 @brief Get SyncE module config

 @param[in] lchip_id    local chip id
 @param[in]  sync_ether_clock_id  <0-1>

 @param[out]  p_sync_ether_cfg  Configuration of SyncE

 @remark[D2.TM]  Get the config information of the SyncE module.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_sync_ether_get_cfg(uint8 lchip_id, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg);

/**@} end of @addgroup   */
#ifdef __cplusplus
}
#endif

#endif

#endif

