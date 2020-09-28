/**
 @file ctc_usw_pdu.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-31

 @version v2.0

This file contains pdu function interfaces.
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_usw_pdu.h"
#include "sys_usw_pdu.h"
#include "sys_usw_common.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @brief  Classify layer2 pdu based on macda,macda-low24 bit, layer2 header protocol

 @param[in] l2pdu_type  CTC_PDU_L2PDU_TYPE_XXX

 @param[in] index  classify layer2 pdu index,
                               based on macda,max index num is 4,
                               based on macda low24 bit, max index num is 10,
                               based on layer2 header protocol, max index num is 16,
                               based on layer3 type, the index  is equal to l3type (CTC_PARSER_L3_TYPE_XXX),
                               based on bpdu,index num is 1

 @param[in] key  layer2 pdu action key

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_classify_l2pdu(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                   ctc_pdu_l2pdu_key_t* key)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_l2pdu_classify_l2pdu(lchip, l2pdu_type, index, key));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get layer2 pdu key

 @param[in] l2pdu_type  CTC_PDU_L2PDU_TYPE_XXX

 @param[in] index  classify layer2 pdu index,
                               based on macda,max index num is 4,
                               based on macda low24 bit, max index num is 10,
                               based on layer2 header protocol, max index num is 16,
                               based on layer3 type, the index  is equal to l3type (CTC_PARSER_L3_TYPE_XXX),
                               based on bpdu,index num is 1

 @param[out] key  layer2 pdu action key

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_get_classified_key(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                       ctc_pdu_l2pdu_key_t* key)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_l2pdu_get_classified_key(lchip, l2pdu_type, index, key));

    return CTC_E_NONE;
}

/**
 @brief  Set layer2 pdu global property

 @param[in] l2pdu_type  CTC_PDU_L2PDU_TYPE_XXX

 @param[in] index  classify layer2 pdu index,
                               based on macda,max index num is 4,
                               based on macda low24 bit, max index num is 10,
                               based on layer2 header protocol, max index num is 16,
                               based on layer3 type, the index  is equal to l3type (CTC_PARSER_L3_TYPE_XXX),
                               based on bpdu,index num is 1
 @param[in] action  layer2 pdu global property filed

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_set_global_action(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                      ctc_pdu_global_l2pdu_action_t* action)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_l2pdu_set_global_action(lchip, l2pdu_type, index, action));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get layer2 pdu global property

 @param[in] l2pdu_type  CTC_PDU_L2PDU_TYPE_XXX

 @param[in] index  classify layer2 pdu index,
                               based on macda,max index num is 4,
                               based on macda low24 bit, max index num is 10,
                               based on layer2 header protocol, max index num is 16,
                               based on layer3 type, the index  is equal to l3type (CTC_PARSER_L3_TYPE_XXX),
                               based on bpdu,index num is 1
 @param[out] action  layer2 pdu global property filed

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_get_global_action(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                      ctc_pdu_global_l2pdu_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_l2pdu_get_global_action(lchip, l2pdu_type, index, action));

    return CTC_E_NONE;
}

/**
 @brief  Per port control layer2 pdu action

 @param[in] gport  global port

 @param[in] action_index  per port control action index,it's from action_index from ctc_pdu_global_l2pdu_action_t

 @param[in] action  layer2 pdu action type

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_set_port_action(uint8 lchip, uint32 gport, uint8 action_index,
                                    ctc_pdu_port_l2pdu_action_t action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_l2pdu_set_port_action(lchip, gport, action_index, action));

    return CTC_E_NONE;
}

/**
 @brief  Per port control layer2 pdu action

 @param[in] gport  global port

 @param[in] action_index  per port control action index,it's from action_index from ctc_pdu_global_l2pdu_action_t

 @param[out] action  layer2 pdu action type

 @return SDK_E_XXX

*/
int32
ctc_usw_l2pdu_get_port_action(uint8 lchip, uint32 gport, uint8 action_index,
                                    ctc_pdu_port_l2pdu_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_l2pdu_get_port_action(lchip, gport, action_index, action));

    return CTC_E_NONE;
}

/**@}*/ /*end of @addtogroup  l2pdu L2PDU */

/**
 @addtogroup  l3pdu L3PDU
 @{
*/

/**
 @brief  Classify layer3 pdu based on layer3 header protocol, layer4 dest port

 @param[in] l3pdu_type  CTC_PDU_L3PDU_TYPE_XXX

 @param[in] index  classify layer3 pdu index,
                               based on layer3 header protocol, max index num is 8
                               based on layer4 dest port, max index num is 8

 @param[in] key  layer3 pdu action key

 @return SDK_E_XXX

*/
int32
ctc_usw_l3pdu_classify_l3pdu(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index,
                                   ctc_pdu_l3pdu_key_t* key)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_l3pdu_classify_l3pdu(lchip, l3pdu_type, index, key));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get layer3 pdu classified key

 @param[in] l3pdu_type  CTC_PDU_L3PDU_TYPE_XXX

 @param[in] index  classify layer3 pdu index,
                               based on layer3 header protocol, max index num is 8
                               based on layer4 dest port, max index num is 8

 @param[out] key  layer3 pdu action key

 @return SDK_E_XXX

*/
int32
ctc_usw_l3pdu_get_classified_key(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index,
                                       ctc_pdu_l3pdu_key_t* key)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_l3pdu_get_classified_key(lchip, l3pdu_type, index, key));

    return CTC_E_NONE;
}

/**
 @brief  Set layer3 pdu global property

 @param[in] l3pdu_type  CTC_PDU_L3PDU_TYPE_XXX

 @param[in] index  classify layer3 pdu index,
                               based on layer3 header protocol, max index num is 8
                               based on layer4 dest port, max index num is 8

 @param[in] action  layer3 pdu global property filed

 @return SDK_E_XXX

*/
int32
ctc_usw_l3pdu_set_global_action(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index,
                                      ctc_pdu_global_l3pdu_action_t* action)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_l3pdu_set_global_action(lchip, l3pdu_type, index, action));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get layer3 pdu global property

 @param[in] l3pdu_type  CTC_PDU_L3PDU_TYPE_XXX

 @param[in] index  classify layer3 pdu index,
                               based on layer3 header protocol, max index num is 8
                               based on layer4 dest port, max index num is 8

 @param[out] action  layer3 pdu global property filed

 @return SDK_E_XXX

*/
int32
ctc_usw_l3pdu_get_global_action(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index,
                                      ctc_pdu_global_l3pdu_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_l3pdu_get_global_action(lchip, l3pdu_type, index, action));

    return CTC_E_NONE;
}

/**
 @brief  Per layer3 interface get layer3 pdu action

 @param[in] l3ifid  layer3 interface id

 @param[in] action_index  per layer3 interface control action index,it's from action_index from ctc_pdu_global_l3pdu_action_t ds

 @param[out] action  layer3 pdu action type

 @remark Get the action of layer3 interface's exception.
         \p
         input action_index:
         \p
         max index num is 16;
         \p
         output action:
         \p
         CTC_PDU_L3PDU_ACTION_TYPE_FWD indicate the action of layer3 exception is forward packet and not send to CPU.
         \p
         CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU indicate the action of layer3 exception is forward packet and send to CPU.

 @return CTC_E_XXX

*/
int32
ctc_usw_l3pdu_set_l3if_action(uint8 lchip, uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t action)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_l3pdu_set_l3if_action(lchip, l3ifid, action_index, action));
    }

    return CTC_E_NONE;
}

/**
 @brief  Per layer3 interface get layer3 pdu action

 @param[in] l3ifid  layer3 interface id

 @param[in] action_index  per layer3 interface control action index,it's from action_index from ctc_pdu_global_l3pdu_action_t ds

 @param[out] action  layer3 pdu action type

 @remark Get the action of layer3 interface's exception.
         \p
         input action_index:
         \p
         max index num is 16;
         \p
         output action:
         \p
         CTC_PDU_L3PDU_ACTION_TYPE_FWD indicate the action of layer3 exception is forward packet and not send to CPU.
         \p
         CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU indicate the action of layer3 exception is forward packet and send to CPU.

 @return CTC_E_XXX

*/
int32
ctc_usw_l3pdu_get_l3if_action(uint8 lchip, uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PDU);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_l3pdu_get_l3if_action(lchip, l3ifid, action_index, action));

    return CTC_E_NONE;
}

/**
 @brief init pdu module

 @return CTC_E_XXX

*/
int32
ctc_usw_pdu_init(uint8 lchip, void* pdu_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_pdu_init(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief deinit pdu module

 @return CTC_E_XXX

*/
int32
ctc_usw_pdu_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_pdu_deinit(lchip));
    }

    return CTC_E_NONE;
}

