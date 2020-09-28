/**
 @file ctc_usw_linkagg.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-10-19

 @version v2.0


 Link Aggregation is a method by which many Ethernet ports can be bundled together to form a linkagg port. The linkagg
 is regarded as one logical link, and is useful when a high bandwidth and/ or redundancy between switches are required.
 All ports in a linkagg must be configured with the same speed and the transmission mode for each linkagg member port
 must be set to full-duplex.

 \b
 \p
 Linkagg  provide features below:
 \d Static Load balance.
 \d Dynamic Load balance.
 \d Round robin Load balance.
 \d Failover.

\b
\p
\B Static Load balance.
\p
 Supports up to 64 link aggregation group and 6 linkagg mode:
\p
 64[G]*16[M], 32[G]*32[M], 16[G]*64[M], 8[G]*128[M], 4[G]*256[M], 8[G]*32[M] + 48[G]*16[M].[G] denotes how many
 linkagg groups, [M] denotes how many linkagg member ports per group.
\p
 Port Selection Criteria (PSC) are used to spread the traffic across the linkagg's member ports by different hash value,
 hash key can be macsa, macda, vlan, ipsa, ipda and others, you can use ctc_linkagg_set_psc() to set port selection criteria.

\b
\p
\B Dynamic Load balance(DLB).
\p
 Dynamic Load Balance(DLB) is used to choose the minimum load of bandwidth among linkagg members as the
 egress. DLB member port selection mechanism is as follows:
 \p
 The traffic status of each member port is monitored in the traffic management module.
 A timer uses a specific discounting algorithm to evaluate each port's output bandwidth.
 At the same time, If some link is down and consider member port's output bandwidth and status,
 the mechanism will elect a least load and active member, which will be used to do rebalance.


\b
\p
\B Round robin Load balance(RR)
\p
The traditional hash mechanism will introduce uneven link bandwidth usage because the same flow selects a only link.
If considering the end points can resolve TCP reorder problem,
then per-packet round robin forwarding mechanism has very good bandwidth utilization.
\pp
RR  Load balance support two mode:
\pp
1)Regular mode: This mode distribute each packet to each member from member0 to member2 according to the order.
\pp
2)Random mode: The above regular mode can not consider a flow's packet length variability.
 The mode shows that start point of each round uses random mechanism to select,
 and the following of the round is same with regular mode.

\b
\p
\B Failover
\p
During system running, if one member port of a linkagg goes down,
the traffic will still be hashed into this member port before CPU detects the Link down event
and remove it from the member list.
To reduce the whole process duration, Supports hardware base Failover convergence upon one member down.


 \S ctc_linkagg.h:ctc_linkagg_mode_t
 \S ctc_linkagg.h:ctc_linkagg_global_cfg_t
 \S ctc_linkagg.h:ctc_linkagg_group_flag_t
 \S ctc_linkagg.h:ctc_linkagg_group_t
 \S ctc_linkagg.h:ctc_linkagg_group_mode_t
 \S ctc_linkagg.h:ctc_linkagg_psc_l2_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_pbb_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_ip_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_l4_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_fcoe_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_trill_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_mpls_header_t
 \S ctc_linkagg.h:ctc_linkagg_psc_type_t
 \S ctc_linkagg.h:ctc_linkagg_psc_t


*/

#ifndef _CTC_USW_LINKAGG_H
#define _CTC_USW_LINKAGG_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_linkagg.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
/**
 @addtogroup linkagg LINKAGG
 @{
*/

/**
 @brief Init the linkagg module

 @param[in] lchip    local chip id

 @param[in]  linkagg_global_cfg linkagg global config information

 @remark  Initialize the linkagg module, including global variable, soft table, asic table, etc.
                This function has to be called before any other linkagg functions.
                LinkAgg group num can set by user according to the parameter linkagg_global_cfg
\p
                group num can be change :
\d              CTC_LINKAGG_MODE_4:  4 groups
\d              CTC_LINKAGG_MODE_8:  8 groups
\d              CTC_LINKAGG_MODE_16: 16 groups
\d              CTC_LINKAGG_MODE_32: 32 groups
\d              CTC_LINKAGG_MODE_64: 64 groups
\d              CTC_LINKAGG_MODE_56: 56 groups

\p
                Supports up to 64 link aggregation group. In one group, member number can be any one of
                1-16, 32, 64, 128 and 256. But totally, the memberNum * groupNum is up to 1024.
                When linkagg_global_cfg is NULL, linkagg will use default global config to initialize sdk.
                Notice: When config CTC_LINKAGG_MODE_64,  dlb is not support in this mode.
                When config CTC_LINKAGG_MODE_56, only group 0-7 can be config as dlb mode and every dlb
                group can only support 32 flow.
                If configure dlb mode, max member number is 16 regardless of group mode configured.
\p
                By default:
                LinkAgg group num is 32 (CTC_LINKAGG_MODE_32).

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_init(uint8 lchip, void* linkagg_global_cfg);

/**
 @brief De-Initialize linkagg module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the linkagg configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_linkagg_deinit(uint8 lchip);

/**
 @brief Create one linkagg

 @param[in] lchip    local chip id

 @param[in] p_linkagg_grp linkagg group information

 @remark[D2.TM]  Create one linkagg with linkagg mode and group id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_create(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp);

/**
 @brief Delete one linkagg

 @param[in] lchip    local chip id

 @param[in] tid the linkagg id which to be removed

 @remark[D2.TM]  Delete one linkagg with linkagg id is the parameter tid.
                Remove all current linkagg member ports from the linkagg group and marks
                the linkagg ID as invalid (available for reuse).

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_destroy(uint8 lchip, uint8 tid);

/**
 @brief The function is to set linkagg property

 @param[in] tid the linkagg id wanted to set property

 @param[in] linkagg_prop the property of linkagg

 @param[in] value the property value of linkagg

 @remark[D2.TM] The API provides a ability to control the various features on the linkagg, refer to ctc_linkagg_property_t

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_set_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32 value);

/**
 @brief The function is to get linkagg property

 @param[in] tid the linkagg id wanted to get property

 @param[in] linkagg_prop the property of linkagg

 @param[out] p_value pointer to the property value of linkagg

 @remark[D2.TM] Get the property of linkagg

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_get_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32* p_value);

/**
 @brief Add a port to linkagg

 @param[in] lchip    local chip id

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member port which to added

 @remark[D2.TM]  Add a member port to a exist linkagg group with the given tid and gport.
\p
          The specified linkagg group must be one returned from ctc_linkagg_create().
\d        If the port is already exist in linkagg group, add member port operation will fail.
\d        If the linkagg already has members, they will be retained.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_add_port(uint8 lchip, uint8 tid, uint32 gport);

/**
 @brief Remove the port from linkagg

 @param[in] lchip    local chip id

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member port which to be removed

 @remark[D2.TM]  Remove a member port from a exist linkagg group with the given tid and gport.
                The specified linkagg group must be one returned from ctc_linkagg_create().
\p
                The port wanted to remove must be a member of the linkagg.
\d        If the port is not exist in linkagg group, add member port operation will fail.
\d        If the linkagg has members, they will be remove.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_remove_port(uint8 lchip, uint8 tid, uint32 gport);

/**
 @brief The function is to replace ports for linkagg

 @param[in] lchip local chip id

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member ports which wanted to replace

 @param[in] mem_num number of the member ports wanted to replace

 @remark[D2.TM] Replace linkagg group's members

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_replace_ports(uint8 lchip, uint8 tid, uint32* gport, uint16 mem_num);

/**
 @brief Get the a local member port of linkagg

 @param[in] lchip_id    local chip id

 @param[in] tid the linkagg id wanted to operate

 @param[out] p_gport the pointer point to the local member port, will be NULL if none

 @param[out] local_cnt number of local port

 @remark[D2.TM]  Get a local member port of a exist linkagg group with the given tid.
                The specified linkagg group must be one returned from ctc_linkagg_create().

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_get_1st_local_port(uint8 lchip_id, uint8 tid, uint32* p_gport, uint16* local_cnt);

/**
 @brief Get the member ports from a linkagg

 @param[in] lchip    local chip id

 @param[in] tid the linkagg id wanted to operate

 @param[out] p_gports a global member ports list of linkagg group

 @param[out] cnt       the number of linkagg member

 @remark[D2.TM]  Get the member ports of a exist linkagg group with the given tid.
                The specified linkagg group must be one returned from ctc_linkagg_create().
                The member ports  list of linkagg group is stored in array pointer p_gports.
                The p_gports memory must be allocated by user and the array num is 1024/groupNum.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_get_member_ports(uint8 lchip, uint8 tid, uint32* p_gports, uint16* cnt);

/**
 @brief Set member of linkagg hash key

 @param[in] lchip    local chip id

 @param[in] psc the port selection criteria of linkagg hash

 @remark[D2.TM]  Set the global linkagg's Port Selection Criteria (PSC), members of linkagg hash key included.
                Assigns a particular load balancing hash algorithm.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_set_psc(uint8 lchip, ctc_linkagg_psc_t* psc);

/**
 @brief Get member of linkagg hash key

 @param[in] lchip    local chip id

 @param[in|out] psc the port selection criteria of linkagg hash

 @remark[D2.TM]  Get the global linkagg's Port Selection Criteria (PSC), members of linkagg hash key included.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_get_psc(uint8 lchip, ctc_linkagg_psc_t* psc);

/**
 @brief Get max member num of linkagg group

 @param[in] lchip    local chip id

 @param[out] max_num the max member num of linkagg group

 @remark[D2.TM]  Get the max member num of linkagg group

 @return CTC_E_XXX

*/
extern int32
ctc_usw_linkagg_get_max_mem_num(uint8 lchip, uint16* max_num);
/**@} end of @addgroup linkagg */

#ifdef __cplusplus
}
#endif

#endif

