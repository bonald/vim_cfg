/**
 @file

 @date 2009-10-20

 @version v2.0

 The file contains all stp APIs for the requirement of multiple supporting spanning tree(MSTP).
 Assumption devices not supporting multiple STP instance, a single STP instance is utilized,
 otherwise MSTP instances identified by both port and vlan indicated.

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_const.h"

#include "ctc_usw_stp.h"
#include "sys_usw_stp.h"
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
 @brief the function is to clear all instances blonged to one port

 @param[in] gport  global port

 @return CTC_E_XXX

 @remark Destroy all STP instances and initialize the STP module to its initial configuration.

*/
int32
ctc_usw_stp_clear_all_inst_state(uint8 lchip, uint32 gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_L2);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
    lchip_end = lchip_start + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_stp_clear_all_inst_state(lchip, gport));
    }

    return CTC_E_NONE;
}

/**
 @brief the function is to set stp stp id (MSTI instance) for vlan

 @param[in] vlan_id      802.1q vlan id

 @param[in] stpid        a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @return CTC_E_XXX

 @remark Set the STP instance id to the VLAN specified by vlan id.
         The current stp id overwrite the previous vlan's stp id value after configuration implicitly.

*/
int32
ctc_usw_stp_set_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8 stpid)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_L2);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_stp_set_vlan_stpid(lchip, vlan_id, stpid));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get vlan's stp id

 @param[in] vlan_id 802.1q vlan id

 @param[in] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @return CTC_E_XXX

 @remark Retrieve the stp id of the VLAN specified by vlan id.

*/
int32
ctc_usw_stp_get_vlan_stpid(uint8 lchip, uint16 vlan_id, uint8* stpid)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_L2);
    LCHIP_CHECK(lchip);

    lchip_start = lchip;
    lchip_end = lchip + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_stp_get_vlan_stpid(lchip, vlan_id, stpid));
    }

    return CTC_E_NONE;
}

/**
 @brief the function is to update the state

 @param[in] gport global port

 @param[in] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @param[in] state a value denoted the instance

 @return CTC_E_XXX

 @remark Set stp state to the instance of STP state indicates by both global port and stp id.
         The stp state should use the value 0:forward, 1:blocking, and 2:learning.

*/
int32
ctc_usw_stp_set_state(uint8 lchip, uint32 gport, uint8 stpid, uint8 state)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_L2);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
    lchip_end = lchip_start + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_stp_set_state(lchip, gport,   stpid,  state));
    }

    return CTC_E_NONE;
};

/**
 @brief the function is to get the instance of the port

 @param[in] gport  global port

 @param[in] stpid a value denoted the index of the stp instance, its value is between 0 and 0x7F.

 @param[out] state  the pointer point to the getted state, will be NULL if none

 @return CTC_E_XXX

 @remark Retrieve the STP state indicated by both global port and stp id.

*/
int32
ctc_usw_stp_get_state(uint8 lchip, uint32 gport, uint8 stpid, uint8* state)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_L2);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
    lchip_end = lchip_start + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_stp_get_state(lchip, gport, stpid, state));
    }

    return CTC_E_NONE;
};

/**
 @brief the function is to get the instance of the port

 @param[in] NULL

 @param[out] NULL

 @return CTC_E_XXX

 @remark Initialize the STP module to its initial configuration.

*/
int32
ctc_usw_stp_init(uint8 lchip, void* stp_global_cfg)
{
    return CTC_E_NONE;
}

int32
ctc_usw_stp_deinit(uint8 lchip)
{
    return CTC_E_NONE;
};

