#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_dot1ae.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

\p
This chapter describes the MACsec(Media Access Control Security) defined by IEEE 802.1AE that used to provide
two-layer encryption technique for user and can be used in host oriented and device oriented two modes. Its greatest
advantages are full speed two-layer encryption and decryption, encryption and decryption by chip and hop by hop
deployment, and centec SDK provide various configurations, such as mirror plaintext or ciphertext from TX and RX,
flexible ICV check mode and transmit message without encryption controlled by ACL.
\p
Three important functions:
\d  Data encrypt and decrypt: If enable MACsec and use port protected by MACsec to transmit or receive message,
message can be encrypted or decrypted and use secret key by multi-parties negotiate session based on MKA protocol.

\d  Integrity check: MACsec can use secret key to calculate ICV(Integrity Check Value) for message and attach to the
end of message, provide various ICV check mode when RX get message.

\d  Replay protection mechanism: During data frame encapsulated by MACsec transmit, MACsec replay protection mechanism
allows disorder within the scope, the data frame out of range will be discard.
\p

*/

#ifndef _CTC_USW_DOT1AE_H
#define _CTC_USW_DOT1AE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_dot1ae.h"
#include "ctc_l2.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @addtogroup dot1ae DOT1AE
 @{
*/

/**
 @brief  Init Dot1ae module

 @param[in] lchip    local chip id

 @param[in] dot1ae_global_cfg   Dot1ae module global config

 @remark[D2.TM]
      Initialize Dot1ae feature module.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_dot1ae_init(uint8 lchip, void* dot1ae_global_cfg);

/**
 @brief  deInit Dot1ae module

 @param[in] lchip    local chip id

 @remark[D2.TM]
      Free resource

 @return CTC_E_XXX
*/
extern int32
ctc_usw_dot1ae_deinit(uint8 lchip);

/**
 @brief  Add Dot1ae Secure Channel

 @param[in] lchip    local chip id

 @param[in] p_chan   pointer to Secure Channel

 @remark[D2.TM]
     Add secure channel to port.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_add_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan);
/**
 @brief  Remove Dot1ae Secure Channel

 @param[in] lchip    local chip id

 @param[in] p_chan  Channel information

 @remark[D2.TM]
     Remove secure channel by chan id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_remove_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan);

/**
 @brief  Update Dot1ae Secure Channel

 @param[in] lchip    local chip id

 @param[in] p_chan  Channel information

 @remark[D2.TM]
     Update secure channel by chan id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_update_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan);
/**
 @brief  Get Dot1ae Secure Channel information

 @param[in] lchip    local chip id

 @param[in] p_chan  Channel information

 @remark[D2.TM]

   Get secure channel by chan id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_get_sec_chan(uint8 lchip_id,  ctc_dot1ae_sc_t* p_chan);

/**
 @brief  Set global configure of Dot1ae

 @param[in] lchip    local chip id

 @param[in] p_glb_cfg  global configure of Dot1ae

 @remark[D2.TM]
  Set configure of Dot1ae.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_set_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg);

/**
 @brief  Get global configure of Dot1ae

 @param[in] lchip    local chip id

 @param[in] p_glb_cfg  global configure of Dot1ae

 @remark[D2.TM]
  Set configure of Dot1ae.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_get_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg);

/**
 @brief  Get stats of Dot1ae

 @param[in] lchip    local chip id

 @param[in] chan_id  channel id

 @param[out] p_stats  stats count

 @remark[TM]

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_get_stats(uint8 lchip_id, uint32 chan_id, ctc_dot1ae_stats_t* p_stats);

/**
 @brief  clear stats of Dot1ae

 @param[in] lchip    local chip id

 @param[in] chan_id  channel id

 @param[in] an  association number, IF an equal 0xFF, clear all an stats

 @remark[TM]

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dot1ae_clear_stats(uint8 lchip_id, uint32 chan_id, uint8 an);

/**@} end of @addtogroup  dot1ae DOT1AE */

#ifdef __cplusplus
}
#endif

#endif

#endif
