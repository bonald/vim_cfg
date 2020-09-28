/**
 @file ctc_usw_learning_aging.h

 @date 2010-3-16

 @version v2.0

\B MAC Learning
\p
MAC learning will use Fid + MacSa as lookup key for regular bridge and globalSrcPort
is in the associated data. To support more flexible advanced features, The Module provides
an option to use Fid + MacSa as the key and logicSrcPort is in the associated data.\p
The Module support two learning mode:
\pp
    1)Hardware learning
    \pp
    Hardware learning is controlled by both MAC and Port. Only if both DsMac.fastLearningEn
    and DsSrcPort.fastLeaningEn are set, hardware learning will be enabled.
\pp
    2)Hardware assist for software learning.
\pp
   The whole learning process can be divided into two stages.\p
\pp
     \dd  Decide if the MAC should be learnt, which is independent of whether hardware or software learning is used.
\pp
     \dd When Learn the MAC
\ppp
     1) For hardware learning, chip will directly write the data into hardware table. The associated DsMac should be programmed
     in advanced. Per globaSrcPort or logicSrcPort there is an adIndex indexed into DsMac table.
\ppp
      2) For hardware assisted software learning, chip will push lots of necessary information, such as MACSA, fid, port into
      a DMA FIFO. Then the DMA engine will send these message from FIFO to DMA memory and trigger interrupt to notify CPU that
      a new MAC needs to be learned.
\p
\B Aging
\p
The Module Aging table contains three different types: DsUserId(SCL)/DsIpDa(IPMC))/DsMac(L2Uc/L2MC).
The same as MAC learning, both hardware assisted software aging and hardware aging are supported.
DsUserId/DsIpDa corresponding Key will use hardware assisted software aging; For DsMac, if DsMAC
corresponding Key stored in TCAM, it must be hardware assisted software aging; else it can use
Hardware aging by DsMac.fastLearningEn.
\p

The Module supports Aging via DsAging table. Each entry of DsAging table is 32 bits. Each MAC entry has
a corresponding bit in this table. This bit is used to indicate if the entry should be aged, which
agingStatus is. Global aging timer is running infinitely until it¡¯s stopped by software. In each
aging period, the whole DsAging table will be scanned and each agingStatus bit will be checked.
In each aging scan period, the each agingStatus bit will be reset to 0 if it¡¯s 1 in this round.
So in next round, if this bit is still 0:\p

\pp
     1) Hardware aging:
\pp
   The corresponding entry will be removed and and pushed into FibLearnFifoInfo_t table is used
   to notify the CPU by DMA mechanism;

\pp
     2) Hardware assisted software aging: \pp
   The corresponding Aghing Ptr will be pushed into 16©\depth Aging Fifo table, when the number
   in the Aging Fifoexceeds a threshold, CPU will be interrupted. Then CPU  needs to read the
   Aging Fifo, By default, the threshold should be set to 1 so that chip will always notify CPU
   for each new entry.\p

\S ctc_learning_aging.h:ctc_aging_table_type_t
\S ctc_learning_aging.h:ctc_aging_status_t
\S ctc_learning_aging.h:ctc_learning_cache_entry_t
\S ctc_learning_aging.h:ctc_learning_cache_t
\S ctc_learning_aging.h:ctc_aging_info_entry_t
\S ctc_learning_aging.h:ctc_aging_fifo_info_t
\S ctc_learning_aging.h:ctc_learn_aging_info_t
\S ctc_learning_aging.h:ctc_hw_learn_aging_fifo_t
\S ctc_learning_aging.h:ctc_learning_action_t
\S ctc_learning_aging.h:ctc_learning_action_info_t
\S ctc_learning_aging.h:ctc_aging_prop_t
\S ctc_learning_aging.h:ctc_learn_aging_global_cfg_t
\S ctc_learning_aging.h:ctc_aging_type_t

*/

#ifndef CTC_USW_LEARNING_AGING_H_
#define CTC_USW_LEARNING_AGING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_learning_aging.h"

/**
 @addtogroup learning_aging  LEARNING_AGING
 @{
*/

/**
 @brief Set the learning action and cache threshold

 @param[in] lchip    local chip id

 @param[in] p_learning_action    learning action and cache threshold

 @remark[D2.TM]  Set Learning action and learning cache threshold.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_set_learning_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action);

/**
 @brief Get the learning action and cache threshold

 @param[in] lchip    local chip id

 @param[out] p_learning_action    learning action and cache threshold

 @remark[D2.TM]  Get Learning action and learning cache threshold.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_get_learning_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action);


/**
 @brief Set the aging properties

 @param[in] lchip    local chip id

 @param[in] tbl_type      aging table type
 @param[in] aging_prop    aging property type
 @param[in] value         aging action value

 @remark[D2.TM]  Set the aging properties

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aging_set_property(uint8 lchip, ctc_aging_table_type_t  tbl_type, ctc_aging_prop_t aging_prop, uint32 value);

/**
 @brief Get the aging properties

 @param[in] lchip    local chip id

 @param[in] tbl_type      aging table type
 @param[in] aging_prop    aging property type
 @param[out] value         point to aging action value

 @remark[D2.TM]  Get the aging properties

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aging_get_property(uint8 lchip, ctc_aging_table_type_t  tbl_type, ctc_aging_prop_t aging_prop, uint32* value);

/**
 @brief Initialize the learning and aging modules

 @param[in] lchip    local chip id

 @param[in]     global_cfg   point to global configure

 @remark[D2.TM]  Initialize the learning and aging modules.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_learning_aging_init(uint8 lchip, void* global_cfg);

/**
 @brief De-Initialize learning_aging module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the learning_aging configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_learning_aging_deinit(uint8 lchip);

/**@} end of @addtogroup learning_aging  LEARNING_AGING*/

#ifdef __cplusplus
}
#endif

#endif

