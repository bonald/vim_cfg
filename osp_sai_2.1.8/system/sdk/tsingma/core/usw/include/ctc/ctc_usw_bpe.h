#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_bpe.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2013-05-28

 @version v2.0


  The BPE module is a function module, which stands for Bridge Port Extension. The related API provided by this
  module is used for port extended, such as interlaken, Mux/DeMux etc.
 \p
  InterLaken Interface is a high-speed channelized interface, which used as interface with NP. When
  used in interlaken mode, it just works as MAC Aggregation. Mux/Demux Interface is another Port Extension function.
 \p


*/


#ifndef _CTC_USW_BPE_H
#define _CTC_USW_BPE_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_bpe.h"

/**
 @brief Initialize the bpe module

 @param[in] lchip    local chip id

 @param[in] bpe_global_cfg bpe global configure

 @remark[D2.TM]  Initialize bpe with global config

 @return CTC_E_XXX

*/
extern int32
ctc_usw_bpe_init(uint8 lchip, void* bpe_global_cfg);

/**
 @brief De-Initialize bpe module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the bpe configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_bpe_deinit(uint8 lchip);

/**
 @brief Set port extender function

 @param[in] lchip    local chip id

 @param[in] gport global port to enable/disable BPE function

 @param[in] p_extender port extend information for BPE function

 @remark[D2.TM] set port extender of global port
 @return CTC_E_XXX

*/
extern int32
ctc_usw_bpe_set_port_extender(uint8 lchip, uint32 gport, ctc_bpe_extender_t* p_extender);

/**
 @brief Get port extender enable status

 @param[in] lchip    local chip id

 @param[in] gport global port to enable/disable BPE function

 @param[out] enable enable or disable

 @remark[D2.TM] get port extender of global port
 @return CTC_E_XXX

*/
extern int32
ctc_usw_bpe_get_port_extender(uint8 lchip, uint32 gport, bool* enable);


/**
 @brief Set port extender function

 @param[in] lchip    local chip id

 @param[in] p_extender_fwd  point to extender forwad entry

 @remark[D2.TM] add extender forward entrey for pe uplink

 @return CTC_E_XXX

*/
extern int32
ctc_usw_bpe_add_8021br_forward_entry(uint8 lchip, ctc_bpe_8021br_forward_t*  p_extender_fwd);


/**
 @brief Set port extender function

 @param[in] lchip    local chip id


 @param[in] p_extender_fwd point to extender forwad entry

 @remark[D2.TM] remove extender forward entrey for pe uplink

 @return CTC_E_XXX

*/
extern int32
ctc_usw_bpe_remove_8021br_forward_entry(uint8 lchip,  ctc_bpe_8021br_forward_t*  p_extender_fwd);



#ifdef __cplusplus
}
#endif

#endif

#endif

