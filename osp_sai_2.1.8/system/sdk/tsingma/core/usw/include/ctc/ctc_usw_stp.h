/**
 @file ctc_usw_stp.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-10-16

 @version v2.0

 Spanning Tree Protocols prevent bridging loops in Layer 2 Ethernet networks. STP module support
 Rapid Spanning Tree, Multiple Spanning Tree, and Rapid-Per VLAN Spanning Tree protocols.
 Multiple Spanning Tree can call ctc_stp_set_vlan_stpid() to set multiple instance.
 STP state can be set by ctc_stp_set_state()

*/

#ifndef _CTC_USW_STP_H
#define _CTC_USW_STP_H
#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************
              Define API function interfaces
***********************************************************************************/

/**
 @addtogroup stp STP
 @{
*/

/**
 @brief Clear all instances belong to one port

 @param[in] lchip    local chip id

 @param[in] gport  global port

 @remark[D2.TM] Destroy all STP instances and initialize the STP module to its initial configuration.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_clear_all_inst_state(uint8 lchip, uint32 gport);

/**
 @brief Set stp id (MSTI instance) for vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id      802.1q vlan id

 @param[in] stpid        a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @remark[D2.TM] Set the STP instance id to the VLAN specified by vlan id.
         The current stp id overwrite the previous vlan's stp id value after configuration implicitly.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_set_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8 stpid);

/**
 @brief Get vlan's stp id

 @param[in] lchip    local chip id

 @param[in] vlan_id 802.1q vlan id

 @param[out] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @remark[D2.TM] Retrieve the stp id of the VLAN specified by vlan id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_get_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8* stpid);

/**
 @brief Update the state

 @param[in] lchip    local chip id

 @param[in] gport global port

 @param[in] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @param[in] state a value denoted the instance

 @remark[D2.TM] Set stp state to the instance of STP state indicates by both global port and stp id.
         The stp state should use the value 0:forward, 1:blocking, and 2:learning.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_set_state(uint8 lchip, uint32 gport, uint8 stpid, uint8 state);

/**
 @brief Get the instance of the port

 @param[in] lchip    local chip id

 @param[in] gport  global port

 @param[in] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F,
            depend on instance mode.

 @param[out] state  the pointer point to the getted state, will be NULL if none

 @remark[D2.TM] Retrieve the STP state indicated by both global port and stp id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_get_state(uint8 lchip, uint32 gport, uint8 stpid, uint8* state);

/**
 @brief Initilize the stp module

 @param[in] lchip    local chip id

 @param[in] stp_global_cfg  stp global configuration

 @remark[D2.TM] Initialize the STP module to its initial configuration.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stp_init(uint8 lchip, void* stp_global_cfg);

/**
 @brief De-Initialize stp module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the stp configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stp_deinit(uint8 lchip);

/**@} end of @addtogroup stp STP*/

#ifdef __cplusplus
}
#endif

#endif

