#if (FEATURE_MODE == 0)
/*
 @file ctc_usw_trill.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-07

 @version v2.0

TRILL(Transparent Interconnection of Lots of Links) is a L2 tunnel technology that overcomes the problems
associated with spanning tree, while retaining the beneficial aspects of Ethernet. With TRILL, all links
in the network are simultaneously active. Traffic travels along the shortest path between source and
destination, and can be load balanced across multiple equal cost paths. In the event of a failure, the
TRILL network can stay up because the effect of any temporary loops is mitigated by the use of a HopCount
in the TRILL header.

\b
\p
The following is associate API supported TRILL features.
\p
1) Add or remove TRILL route entry for transmit RBridge by ctc_trill_add_route()/ctc_trill_remove_route();
\p
2) Add or remove some checks like RPF check, adjacency check etc by ctc_trill_add_route()/ctc_trill_remove_route();
\p
3) Add or remove TRILL tunnel decapsulation entry for egress RBridge by ctc_trill_add_tunnel()/ctc_trill_remove_tunnel();

\S ctc_trill.h:ctc_trill_route_t
\S ctc_trill.h:ctc_trill_tunnel_t

*/

#ifndef _CTC_USW_TRILL_H
#define _CTC_USW_TRILL_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_trill.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/****************************************************************
 *
* Function
*
****************************************************************/

/**
 @brief Init TRILL module

 @param[in] lchip    local chip id

 @param[in] trill_global_cfg  Point to init paramete

 @remark[D2.TM]  N/A

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_init(uint8 lchip, void* trill_global_cfg);

/**
 @brief Deinit TRILL module

 @param[in] lchip    local chip id

 @remark[D2.TM]  N/A

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_deinit(uint8 lchip);

/**
 @brief Add TRILL route entry

 @param[in] lchip    local chip id

 @param[in] p_trill_route  Data of the trill entry

 @remark[D2.TM]  This function is used to add ucast or mcast trill entry
\p
 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_add_route(uint8 lchip, ctc_trill_route_t* p_trill_route);

/**
 @brief Remove TRILL route entry

 @param[in] lchip    local chip id

 @param[in] p_trill_route  Data of the trill entry

 @remark[D2.TM]  This function is used to remove ucast or mcast trill entry

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_remove_route(uint8 lchip, ctc_trill_route_t* p_trill_route);

/**
 @brief Add TRILL tunnel decapsulation entry

 @param[in] lchip    local chip id

 @param[in] p_trill_tunnel  Data of the trill entry

 @remark[D2.TM]  This function is used to add ucast or mcast trill tunnel entry for decapsulation

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_add_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);

/**
 @brief Remove TRILL tunnel decapsulation entry

 @param[in] lchip    local chip id

 @param[in] p_trill_tunnel  Data of the trill entry

 @remark[D2.TM]  This function is used to remove ucast or mcast trill tunnel entry for decapsulation

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_remove_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);

/**
 @brief Update TRILL tunnel decapsulation entry

 @param[in] lchip    local chip id

 @param[in] p_trill_tunnel  Data of the trill entry

 @remark[D2.TM]  This function is used to update ucast or mcast trill tunnel entry for decapsulation

 @return CTC_E_XXX

*/
extern int32
ctc_usw_trill_update_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);
#ifdef __cplusplus
}
#endif

#endif

#endif

