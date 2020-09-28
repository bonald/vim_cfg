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

#if !defined (__LAILED_H_)
#define __LAILED_H_

#include "laitype.h"


typedef enum _lai_led_event_t
{
    /** Create a new led*/
    LAI_LED_EVENT_ADD,

    /** Delete/Invalidate an existing led */
    LAI_LED_EVENT_DELETE,

} lai_led_event_t;


/**
 * Routine Description:
 *   @brief led event notification
 *
 * Arguments:
 *    @param[in] info- led information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_led_event_notification_fn)(
    _In_ lai_led_event_t  event,
    _In_ lai_led_info_t* led
    );

/**
 * Routine Description:
 *    @brief get led information
 *
 * Arguments:
 *    @param[in] oid - led oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_led_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_led_info_t* led
    );

/**
 * Routine Description:
 *    @brief set led mode on/off
 *
 * Arguments:
 *    @param[in] oid - led oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_led_mode_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_led_info_t* led
    );

/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_led_api_t
{
    lai_led_info_get_fn              get_led_info;
    lai_led_mode_set_fn              set_led_mode;
} lai_led_api_t;

#endif // __LAILED_H_




