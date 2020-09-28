/**
 @file

 @date 2009-10-20

 @version v2.0

*/

#ifndef _SYS_USW_STP_H
#define _SYS_USW_STP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"

#include "ctc_l2.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define SYS_STP_INIT_CHECK() \
    do \
    { \
        LCHIP_CHECK(lchip);\
        if (p_usw_stp_master[lchip] == NULL){ \
            return CTC_E_NOT_INIT; } \
    } \
    while (0)

/****************************************************************************
*
* Function
*
*****************************************************************************/

/**
 @brief the function is to clear all instances blonged to one port

*/
extern int32
sys_usw_stp_clear_all_inst_state(uint8 lchip, uint32 gport);

/**
 @brief the function is to stp id (MSTI instance) for vlan
*/
extern int32
sys_usw_stp_set_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8 stpid);

/**
 @brief the function is to stp id (MSTI instance) for vlan
*/
extern int32
sys_usw_stp_get_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8* stpid);

/**
 @brief the function is to update the state
*/
extern int32
sys_usw_stp_set_state(uint8 lchip, uint32 gport, uint8 stpid, uint8 state);

/**
 @brief the function is to get the instance of the port
*/
extern int32
sys_usw_stp_get_state(uint8 lchip, uint32 gport, uint8 stpid, uint8* state);

/**
 @brief the function is to init all port stp state
*/
extern int32
sys_usw_stp_init(uint8 lchip, uint8 stp_mode);

extern int32
sys_usw_stp_deinit(uint8 lchip);


#ifdef __cplusplus
}
#endif

#endif

