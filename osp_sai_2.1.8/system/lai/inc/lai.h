/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-11-25
* Reason        : First Create.
****************************************************************************/
#if !defined (__LAI_H_)
#define __LAI_H_

#include "laitype.h"
#include "laifan.h"
#include "laithermal.h"
#include "laipsu.h"
#include "laivcm.h"
#include "laipoe.h"
#include "lailed.h"
#include "laiport.h"
#include "laifiber.h"
#include "laicard.h"
#include "laistatus.h"


typedef enum _lai_api_t
{    
    LAI_API_UNSPECIFIED      =  0, 
    LAI_API_CARD             =  1,  /**< lai_card_api_t */
    LAI_API_FAN              =  2,  /**< lai_fan_api_t */
    LAI_API_PSU              =  3,  /**< lai_psu_api_t */
    LAI_API_THERMAL          =  4,  /**< lai_thermal_api_t */
    LAI_API_LED              =  5,  /**< lai_led_api_t */
    LAI_API_PORT             =  6,  /**< lai_port_api_t */
    LAI_API_FIBER            =  7,  /**< lai_fiber_api_t */
    LAI_API_SYNCE            =  8,  /** added by yejl for LAI_API_SYNCE < lai_synce_api_t */
    LAI_API_VCM                = 9, /**< lai_vcm_api_t */
    LAI_API_POE                = 10, /**< lai_poe_api_t */
    LAI_API_MAX              =  11,
} lai_api_t;


/**
 * Routine Description:
 *     @brief Adapter module initialization call. This is NOT for SDK initialization.
 *
 * Arguments:
 *     @param[in] flags    - reserved for future use, must be zero
 *
 * Return Values:
 *    @return  LAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
lai_status_t
lai_api_initialize(
    _In_ uint64_t flags,
    _In_ lai_card_notification_t* noti_cb);


/**
 * Routine Description:
 *     Retrieve a pointer to the C-style method table for desired LAI 
 *     functionality as specified by the given lai_api_id.
 *
 * Arguments:
 *     @param[in]  lai_api_id       - LAI api ID
 *     @param[out] api_method_table - Caller allocated method table
 *           The table must remain valid until the lai_api_uninitialize() is called 
 *
 * Return Values:
 *    @return  LAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
lai_status_t
lai_api_query(
    _In_ lai_api_t lai_api_id,
    _Out_ void** api_method_table
    );


/**
 * Routine Description:
 *   Uninitialization of the adapter module. LAI functionalities, retrieved via 
 *   lai_api_query() cannot be used after this call.
 *
 * Arguments:
 *   None
 *
 * Return Values:
 *   @return LAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
lai_status_t
lai_api_uninitialize(
    void
    );

#endif  // __LAI_H_

