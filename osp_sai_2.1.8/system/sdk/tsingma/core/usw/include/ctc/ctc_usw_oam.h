#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_oam.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-1-19

 @version v2.0

Operations, Administration, and Maintenance (OAM) provide some mechanisms which can detect and recover
faults to keep the networks steady. The Centec OAM module includes CFM (802.1ag), ITU.Y.1731 and BFD function.

\b
\B CFM and Y.1731
\p
802.1ag and ITU.Y.1731 provide some methods to detect, verify and recover faults.
\p
Use Continuity Check (CC) protocol to detect connectivity failure and unintended connectivity
between service instances. Multicast Connectivity Check Messages (CCMs) are announced periodically to
all Maintain association End Points (MEPs) in a Maintain Association (MA) by each MEPs. Comparing the
received CCMs and configuration of remote MEPs, all connectivity faults can be detected.
\p
Use Loopback (LB) protocol to perform fault verification when detect connectivity failure. A MEP can
be ordered to send a Loopback Message (LBM) to a MEP or MIP in the MA. The destination MP responds
a Loopback Reply (LBR) message to the originating MEP if no connectivity failure, otherwise it is
disconnected between the two MPs.
\p
Use Linktrace (LT) protocol to determine the path to a target MEP. A Linktrace Message (LTM) is sent by
a MEP to a remote MEP, then all the MIPs in the path should reply a Link Trace Reply (LTR). So the
disconnected position can be determined if the target does not reply the LTR.
\p
Also, ITU.Y.1731 provides some methods to do performance measurement:
\p
Use Loss Measurement Message (LMM) and Loss Measurement Reply (LMR) to do on-demand Single-ended frame
 loss measurement, and use CCMs to do proactive Dual-ended frame loss measurement.
\p
Use One-way Delay Measurement (1DM) to do One-way frame delay measurement, and use Delay Measurement
Message (DMM) and Delay Measurement Reply (DMR) to do Two-way frame delay measurement.
\p
Use Synthetic Loss Message (SLM) and Synthetic Loss Reply (SLR) to do synthetic loss measurement which
is a mechanism to measure frame loss using synthetic frames rather than data traffic.
\p
Use LBM/LBR/TST to do Throughput measurement defined in RFC2544.

\b
The The Module OAM Engine provides control of the following OAM features for CFM and Y.1731:
\p
\d The processing of CFM based Ethernet encapsulation
\d The processing of Y.1731 based Ethernet and MPLS-TP encapsulation
\d The periodic transmission of outgoing CCMs by interval 0~7 defined by Y.1731
\d The reception and processing of incoming CCMs
\d The transmission of LBRs in response to LBMs
\d The forwarding LTM and LTR to CPU
\d The transmission of LMRs in response to LMMs
\d The time stamping and transmission of DMRs in response to DMMs
\d The periodic transmission of outgoing SLMs, the transmission of SLRs in response to SLMs
\d The periodic transmission of outgoing LBM/TST for Throughput measurement
\d The periodic transmission of outgoing Client Signal Fail (CSF) and the reception and processing of incoming CSF

\p
\B BFD
\p
Bidirectional Forwarding Detection (BFD) defined in RFC5880 is a simple mechanism which detects the existence
of a connection between adjacent systems, allowing it to quickly detect failure of any element in the
connection. It does not operate independently, but only as an adjunct to routing protocols. The routing
protocols are responsible for neighbor detection, and create BFD sessions with neighbors by requesting
failure monitoring from BFD.
\p
Once a BFD session is established with a neighbor, BFD exchanges control packets to verify connectivity
and informs the requesting protocol of failure if a specified number of successive packets are not
received. The requesting protocol is then responsible for responding to the loss of connectivity.
\p
Routing protocols using BFD for failure detection continue to operate normally when BFD is enabled,
including the exchange of hello packets.
\b
The Module OAM Engine provides control of the following OAM features for BFD:
\p
\d The processing of Single-Hop (RFC5881) and Multi-Hop (RFC5883) IP BFD
\d The processing of MPLS BFD defined in RFC5884
\d The processing of Virtual Circuit Connectivity Verification (VCCV) BFD defined in RFC5885
\d The processing of MPLS-TP BFD defined in RFC6428
\d The periodic transmission of outgoing CC/CV, the period is based 1ms or 3.33ms
\d The reception and processing of incoming CC/CV, and time negotiation on-chip
\d The periodic transmission of outgoing MPLS-TP CSF and the reception and processing of incoming CSF
\d Packet loss and delay measurement for MPLS networks defined in RFC6374, includes DM/DLM/DLMDM


\p
\b
This module contains OAM associated APIs, apply APIs can initialize, add/remove maid/local MEP/remote MEP/MIP etc.
All API use MEP type(ctc_oam_mep_type_t) to indicate which OAM type to support.

\p
Callback mechanisms are also provided for notifying the application of OAM-related events, such as the setting
and clearing of defect.

\p
The module provides APIs to support OAM:
\p
\d Add/remove maid for OAM by ctc_oam_add_maid()/ctc_oam_remove_maid()
\d Add/remove/update local mep by ctc_oam_add_lmep()/ctc_oam_remove_lmep()/ctc_oam_update_lmep()
\d Add/remove/update remote mep by ctc_oam_add_rmep()/ctc_oam_remove_rmep()/ctc_oam_update_rmep()
\d Add/remove mip by ctc_oam_add_mip()/ctc_oam_remove_mip()
\d Configure global property for OAM by ctc_oam_set_property()
\d Get defect MEP by ctc_oam_get_defect_info()
\d Get mep information by ctc_oam_get_mep_info()
\d Set throughput config by ctc_oam_set_trpt_cfg()
\d Enable/disable throughput measurement by ctc_oam_set_trpt_en()
\d Get and clear throughput statistics by ctc_oam_get_trpt_stats()/ctc_oam_clear_trpt_stats()

\S ctc_oam.h:ctc_oam_global_t
\S ctc_oam.h:ctc_oam_mep_type_t
\S ctc_oam.h:ctc_oam_eth_key_t
\S ctc_oam.h:ctc_oam_tp_key_t
\S ctc_oam.h:ctc_oam_bfd_key_t
\S ctc_oam.h:ctc_oam_key_t
\S ctc_oam.h:ctc_oam_maid_t
\S ctc_oam.h:ctc_oam_lmep_t
\S ctc_oam.h:ctc_oam_rmep_t
\S ctc_oam.h:ctc_oam_mip_t
\S ctc_oam.h:ctc_oam_property_type_t
\S ctc_oam.h:ctc_oam_com_property_t
\S ctc_oam.h:ctc_oam_property_t
\S ctc_oam.h:ctc_oam_defect_info_t
\S ctc_oam.h:ctc_oam_mep_info_t
\S ctc_oam.h:ctc_oam_trpt_t
\S ctc_oam.h:ctc_oam_trpt_stats_t

\S ctc_oam_y1731.h:ctc_oam_y1731_lmep_t
\S ctc_oam_y1731.h:ctc_oam_y1731_rmep_t
\S ctc_oam_y1731.h:ctc_oam_y1731_prop_t

\S ctc_oam_bfd.h:ctc_oam_bfd_lmep_t
\S ctc_oam_bfd.h:ctc_oam_bfd_rmep_t

*/

#ifndef _CTC_USW_OAM_H
#define _CTC_USW_OAM_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "ctc_oam.h"
#include "ctc_parser.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

/****************************************************************************
 *
* Function
*
****************************************************************************/
/**
 @addtogroup oam OAM
 @{
 */

/**
 @brief   Add ma id for oam

 @param[in] lchip    local chip id

 @param[in]  p_maid    maid data structure pointer

 @remark[D2.TM]  Add ma id , refer to ctc_oam_maid_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_add_maid(uint8 lchip, ctc_oam_maid_t* p_maid);

/**
 @brief   Remove ma id for oam

 @param[in] lchip    local chip id

 @param[in]  p_maid   maid data sturcture pointer

 @remark[D2.TM]  Remove ma id , refer to ctc_oam_maid_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_remove_maid(uint8 lchip, ctc_oam_maid_t* p_maid);

/**
 @brief   Add local mep

 @param[in] lchip    local chip id

 @param[in]  p_lmep   local mep data sturcture pointer

 @remark[D2.TM]  Add local mep, refer to ctc_oam_lmep_t.
          Use flag can set MEP to P2MP mode, up/down mep.
          For MEP on linkagg port cross chip, should use local MEP index allocate by system,
          and should set master_gchip to indicate which chip OAM engine to MEP resident.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_add_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep);

/**
 @brief   Remove local mep

 @param[in] lchip    local chip id

 @param[in]  p_lmep   local mep data sturcture pointer

 @remark[D2.TM]  Remove local mep, refer to ctc_oam_lmep_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_remove_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep);

/**
 @brief   Update local mep

 @param[in] lchip    local chip id

 @param[in]  p_lmep   local mep data sturcture pointer

 @remark[D2.TM]  Update local mep config , refer to ctc_oam_update_t.
          for local MEP, should set is_local to 1.
          For example, If MEP type is CTC_OAM_MEP_TYPE_ETH_Y1731,
          refer ctc_eth_oam_lmep_update_type_t can update local mep config.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_update_lmep(uint8 lchip, ctc_oam_update_t* p_lmep);

/**
 @brief   Add remote mep

 @param[in] lchip    local chip id

 @param[in]  p_rmep   remote mep data sturcture pointer

 @remark[D2.TM]  Add remote mep, refer to ctc_oam_rmep_t.
          For P2MP mode, use different remote mep id to add remote mep,
          but should set P2MP when Add local MEP.
          For MEP on linkagg port cross chip, should use remote MEP index allocate by system.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_add_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep);

/**
 @brief   Remove remote mep

 @param[in] lchip    local chip id

 @param[in]  p_rmep   remote mep data sturcture pointer

 @remark[D2.TM]  Remove remote mep, refer to ctc_oam_rmep_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_remove_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep);

/**
 @brief   Update remote mep

 @param[in] lchip    local chip id

 @param[in]  p_rmep   remote mep data sturcture pointer

 @remark[D2.TM]  Update remote mep config , refer to ctc_oam_update_t.
          For remote MEP, should set is_local to 0 and rmep_id indicate which remote mep to update.
          For example, If MEP type is CTC_OAM_MEP_TYPE_ETH_Y1731,
          refer ctc_eth_oam_rmep_update_type_t can update remote mep config.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_update_rmep(uint8 lchip, ctc_oam_update_t* p_rmep);

/**
 @brief   Add mip

 @param[in] lchip    local chip id

 @param[in]  p_mip   mip data sturcture pointer

 @remark[D2.TM]  Add mip, refer to ctc_oam_mip_t.
          For MIP on linkagg port cross chip, should set master_gchip
          indicated which chip to process OAM PDU.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_add_mip(uint8 lchip, ctc_oam_mip_t* p_mip);

/**
 @brief   Remove mip

 @param[in] lchip    local chip id

 @param[in]  p_mip   mip data sturcture pointer

 @remark[D2.TM]  Remove mip, refer to ctc_oam_mip_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_remove_mip(uint8 lchip, ctc_oam_mip_t* p_mip);

/**
 @brief   Set oam property of oam enable and others

 @param[in] lchip    local chip id

 @param[in] p_prop property sturcture pointer

 @remark[D2.TM]  Use MEP type to set Common OAM/ETH-OAM/TP-OAM global property, refer to ctc_oam_defect_info_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_set_property(uint8 lchip, ctc_oam_property_t* p_prop);

/**
 @brief   Read Oam defect cache which keep up defect info

 @param[in] lchip_id    local chip id

 @param[out] p_defect_info    point to defect cache info

 @remark[D2.TM]  p_defect_info refer to ctc_oam_defect_info_t.
          Get defect MEP's index from error cache.
          How to use p_defect_info to get MEP index :
\p
          for(mep_bit_index = 0; mep_bit_index < 32 , mep_bit_index++)
\p
          {
\p
            if(CTC_IS_BIT_SET(mep_index_bitmap, mep_bit_index))
\p
            {
\p
                mep_index = mep_index_base*32 + mep_bit_index;
\p
            }
\p
          }
\p
 @return CTC_E_XXX
*/
extern int32
ctc_usw_oam_get_defect_info(uint8 lchip_id, void* p_defect_info);

/**
 @brief   Get MEP Information from both SDK database and ASIC dataset

 @param[in] lchip_id    local chip id

 @param[in|out] p_mep_info    MEP info pointer

 @remark[D2.TM]  Input MEP index get by ctc_oam_get_defect_info().
          According MEP index, can get MEP's defect information. Such as Loc, Mismerge,
          UnexpectedPeriod, UnexpectedMEP, UnexpectedMEGlevel.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_oam_get_mep_info(uint8 lchip_id, ctc_oam_mep_info_t* p_mep_info);



/**
 @brief   Get MEP Information from both SDK database and ASIC dataset according OAM key

 @param[in] lchip    local chip id

 @param[in|out] p_mep_info    MEP info pointer

 @remark[D2.TM]  Input OAM key information.
          According OAM Key, can get MEP's defect information. Such as Loc, Mismerge,
          UnexpectedPeriod, UnexpectedMEP, UnexpectedMEGlevel, dMisc, BFD interval, BFD discr.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_oam_get_mep_info_with_key(uint8 lchip, ctc_oam_mep_info_with_key_t* p_mep_info);

/**
 @brief   Get OAM MEP Stats from ASIC dataset

 @param[in] lchip    local chip id

 @param[in|out] p_stat_info    MEP stats information pointer

 @remark[D2.TM]  Input key for Local MEP, can get MEP's LM counter for dual-ended LM, LM type and LM cos type.
          For single-ended LM , not need to get LM counter use this API.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_oam_get_stats(uint8 lchip, ctc_oam_stats_info_t* p_stat_info);


/**
 @brief  Set throughput session parameter

 @param[in] lchip    local chip id

 @param[in] p_throughput config for throughput

 @remark[D2.TM]  set throughput config
 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_set_trpt_cfg(uint8 lchip, ctc_oam_trpt_t* p_throughput);

/**
 @brief   Set throughput session enable

 @param[in] lchip    local chip id

 @param[in] gchip global chip
 @param[in] session_id session if for throughput

 @param[in] enable enable throughput or disable

 @remark[D2.TM]  Must set throughput parameter by ctc_oam_set_trpt_para first, then enable throughput session.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_set_trpt_en(uint8 lchip, uint8 gchip, uint8 session_id, uint8 enable);

/**
 @brief  Get throughput stats

 @param[in] lchip    local chip id

 @param[in] gchip global chip

 @param[in] session_id session id if for throughput

 @param[out] p_trpt_stats the statistics for throughput

 @remark[D2.TM]  get throughput statistics
 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_get_trpt_stats(uint8 lchip, uint8 gchip, uint8 session_id,  ctc_oam_trpt_stats_t* p_trpt_stats);

/**
 @brief  Clear throughput stats

 @param[in] lchip    local chip id

 @param[in] gchip global chip

 @param[in] session_id session id if for throughput

 @remark[D2.TM]  clear throughput statistics

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_clear_trpt_stats(uint8 lchip, uint8 gchip, uint8 session_id);



/**
 @brief   Initialization for oam

 @param[in] lchip    local chip id

 @param[in] p_cfg global configuration for oam pointer

 @remark[D2.TM]  When p_cfg is NULL,OAM will use default global config to initialize sdk.
          \p
          By default: Enable MEP index allocated by SDK.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_init(uint8 lchip, ctc_oam_global_t* p_cfg);


/**
 @brief   deinit for oam module

 @param[in] lchip    local chip id

 @remark[D2.TM] local chip id

 @return CTC_E_XXX

*/
extern int32
ctc_usw_oam_deinit(uint8 lchip);

/**@} end of @addtogroup oam OAM*/
#ifdef __cplusplus
}
#endif

#endif

#endif

