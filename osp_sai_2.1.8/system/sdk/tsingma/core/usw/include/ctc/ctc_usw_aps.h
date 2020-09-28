/**
 @file ctc_usw_aps.h

 @date 2010-3-15

 @version v2.0

 Traditional transport network requires sub 50ms automatic protection switching upon working path failure.
 SDH/SONET network has provided mature technology to quickly detect the failure and perform rapid protection switch
 to meet the sub 50ms switch-over requirement.
 \b
 Carrier Ethernet and MPLS-TP network must meet the same requirement for high service availability.
 This requires the capability to detect the connectivity fault via Ethernet CC/MPLS-TP CC and to react rapidly to
 switch the working path to the provisioned protection path upon working path failure via APS. APS doesn't refer to
 a specific protocol. Instead it's common concept about quick switch between working path and protection path.
 \b
 ITU has developed G.8031 (for linear protection) and G.8032 (for ring protection) for Ethernet APS, and G.8131
 (for linear protection) and G.8132 (for ring protection) for MPLS-TP APS.
 \b
 Dedicated APS protocols are designed to coordinate the protection switching between two endpoints,
 such protocols can normally run on a CPU. But to speed up the switching process, the forwarding plane must provide
 some special mechanism to facilitate the switch instead of flushing out all the forwarding entries.
 Depending on the topology, there are ring protection and linear protection. For linear protection,
 there are 1:1 protection and 1+1 protection. These are applicable to both Ethernet APS and MPLS-TP APS.
 \b
 For MPLS-TP, there are also two protection topologies, linear protection and ring protection.
 And for linear protection, there are also 1:1 and 1+1 modes. ITU-T G.8131 defines MPLS-TP Linear protection
 and ITU-T G.8132 defines MPLS-TP Ring protection. RFC 6378 also defines the linear protection mechanism.
 On control plane, it's different from ITU-T G.8131. But the data plane is very similar. Even for control plane,
 the working theory is similar too. The ring protection hasn't been standardized by IETF yet.
 There are a few of different drafts. One of them suggests using linear protection mechanism to support ring protection.
 Different from Ethernet APS, for MPLS-TP APS, there may be hierarchy protection level.
 E.g. protection applied on PW/ LSP/Tunnel respectively.
 \b
 For linear protection, the architecture is similar as Ethernet linear protection.
 PW/LSP/Tunnel instead of Ethernet VLAN is used.
 \b
 The Module implements quick switching in forwarding data path while the APS protocol needs to run in a separate
 CPU if necessary. Once a failure is detected on the data path by OAM or other mechanisms,
 the CPU will be notified and it will trigger The Module to perform the quick switching.
 The Module implements a protocol/topology independent APS mechanism to perform quick protection switching.
 Only one bit flip is needed to finish the switching between working path and protection path with no need to flush
 the forwarding table. For MPLS, there may be hierarchy OAM level running on PW/LSP/Tunnel.
 The Module APS can support up to 2 level protections switching, with each level works independently.
 This is mainly used for MPLS-TP APS.
 \d   For APS Bridge and selector, the table DsApsBridge is used to make the switching.

\b
\p
The module provides APIs to support APS:
\p
\d Create aps group by ctc_aps_create_aps_bridge_group()
\d Update aps group info by ctc_aps_set_aps_bridge_group()
\d Destroy aps group by ctc_aps_destroy_aps_bridge_group()
\d Set port as aps group working port by ctc_aps_set_aps_bridge_working_path()
\d Get port as aps group working port ctc_aps_get_aps_bridge_working_path()
\d Set port as aps group protection port by ctc_aps_set_aps_bridge_protection_path()
\d Get port as aps group protection port by ctc_aps_get_aps_bridge_protection_path()
\d Set aps protection enable by ctc_aps_set_aps_bridge()
\d Set aps protection enable info by ctc_aps_get_aps_bridge()
\d Set aps select group enable by ctc_aps_set_aps_select()
\d Create raps multicast group by ctc_aps_create_raps_mcast_group()
\d Destroy raps multicast group by ctc_aps_destroy_raps_mcast_group()
\d Update raps multicast member by ctc_aps_update_raps_mcast_member()

\S ctc_aps.h:ctc_aps_flag_t
\S ctc_aps.h:ctc_aps_type_t
\S ctc_aps.h:ctc_aps_bridge_group_t
\S ctc_aps.h:ctc_aps_next_aps_t
\S ctc_aps.h:ctc_raps_member_t

*/

#ifndef _CTC_USW_APS_H
#define _CTC_USW_APS_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_aps.h"
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
 @addtogroup aps APS
 @{
*/

/**
 @brief Initialize APS module

 @param[in] lchip    local chip id

 @param[in] aps_global_cfg aps global config

 @remark[D2.TM] Initialize the The Module aps subsystem, including global variable, soft table, asic table, etc.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_init(uint8 lchip, void* aps_global_cfg);

/**
 @brief De-Initialize aps module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the aps configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_deinit(uint8 lchip);

/**
 @brief Create APS Bridge Group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge, value is 0~1023

 @param[in] aps_group               information of APS group, include working path and protection path

 @remark[D2.TM] Create APS bridge group by APS group id and configure information.
                set w_l3if_id/p_l3if_id when use vlan interface.
                set physical_isolated for layer 2 APS protection when don't edit packet info.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_create_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group);

/**
 @brief Set APS Bridge Group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id   group id of the APS bridge, value is 0~1023

 @param[in] aps_group               information of APS group, include working path and protection path

 @remark[D2.TM] Set APS bridge group by APS group id and configure information.
                set w_l3if_id/p_l3if_id when use vlan interface.
                set physical_isolated for layer 2 APS protection when don't edit packet info.
                this interface used for the translation between aps nexthop and normal nexthop,
                and don't support for spme aps nexthop.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_set_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group);

/**
 @brief Remove a APS Bridge Group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @remark[D2.TM] Create APS bridge group by APS group id.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_destroy_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id);

/**
 @brief Set working path of APS bridge group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @param[in] gport                   gport of working path

 @remark[D2.TM] Set working path of APS bridge group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_set_aps_bridge_working_path(uint8 lchip, uint16 aps_bridge_group_id, uint32 gport);

/**
 @brief Get working path of APS bridge group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @param[out] gport                   gport of working path

 @remark[D2.TM] Get working path of APS bridge group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_get_aps_bridge_working_path(uint8 lchip, uint16 aps_bridge_group_id, uint32* gport);

/**
 @brief Set protection path of APS bridge group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @param[in] gport                   gport of protection path

 @remark[D2.TM] Set protection path of APS bridge group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_set_aps_bridge_protection_path(uint8 lchip, uint16 aps_bridge_group_id, uint32 gport);

/**
 @brief Get protection path of APS bridge group

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @param[out] gport                   gport of protection path

 @remark[D2.TM] Get protection path of APS bridge group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_get_aps_bridge_protection_path(uint8 lchip, uint16 aps_bridge_group_id, uint32* gport);

/**
 @brief Set APS bridge to working path or protection path

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the APS bridge

 @param[in] protect_en              APS bridge to protection path if TURE, otherwise to working path

 @remark[D2.TM] Set APS bridge to working path or protection path.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_set_aps_bridge(uint8 lchip, uint16 aps_bridge_group_id, bool protect_en);

/**
 @brief Get APS bridge status

 @param[in] lchip    local chip id

 @param[in] aps_bridge_group_id     group id of the aps bridge

 @param[out] protect_en              APS bridge to protection path if TURE, otherwise to working path

 @remark[D2.TM] Get APS bridge status.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_get_aps_bridge(uint8 lchip, uint16 aps_bridge_group_id, bool* protect_en);

/**
 @brief Set APS selector from working path or protection path

 @param[in] lchip    local chip id

 @param[in] aps_select_group_id     group id of the aps selector

 @param[in] protect_en              APS selector from protection path if TRUE, otherwise working path

 @remark[D2.TM] Set APS selector from working path or protection path.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_set_aps_select(uint8 lchip, uint16 aps_select_group_id, bool protect_en);

/**
 @brief Get APS selector status

 @param[in] lchip    local chip id

 @param[in] aps_select_group_id     group id of the APS selector

 @param[out] protect_en              APS selector from protection path if TRUE, otherwise working path

 @remark[D2.TM] Get APS selector status.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_get_aps_select(uint8 lchip, uint16 aps_select_group_id, bool* protect_en);

/**
 @brief create R-APS mcast group

 @param[in] lchip    local chip id

 @param[in] group_id             raps group id

 @remark[D2.TM] create R-APS mcast group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_create_raps_mcast_group(uint8 lchip, uint16 group_id);

/**
 @brief remove R-APS mcast group

 @param[in] lchip    local chip id

 @param[in] group_id              raps group id

 @remark[D2.TM] remove R-APS mcast group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_destroy_raps_mcast_group(uint8 lchip, uint16 group_id);

/**
 @brief update R-APS group meber

 @param[in] lchip    local chip id

 @param[in] p_raps_mem            raps member info

 @remark[D2.TM] update R-APS group member.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_aps_update_raps_mcast_member(uint8 lchip, ctc_raps_member_t* p_raps_mem);

/**@}*/ /*end of @addtogroup APS*/

#ifdef __cplusplus
}
#endif

#endif

