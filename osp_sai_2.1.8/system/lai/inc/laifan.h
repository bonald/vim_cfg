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

#if !defined (__LAIFAN_H_)
#define __LAIFAN_H_

#include "laitype.h"


typedef enum _lai_fan_event_t
{
    /** Create a new fan*/
    LAI_FAN_EVENT_ADD,

    /** Delete/Invalidate an existing fan */
    LAI_FAN_EVENT_DELETE,

} lai_fan_event_t;


/**
 * Routine Description:
 *   @brief fan event notification
 *
 * Arguments:
 *    @param[in] info- fan information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_fan_event_notification_fn)(
    _In_ lai_fan_event_t  event,
    _In_ lai_fan_info_t* fan
    );

/**
 * Routine Description:
 *    @brief get fan information
 *
 * Arguments:
 *    @param[in] oid - fan oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_fan_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_fan_info_t* fan
    );


/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_fan_api_t
{
    lai_fan_info_get_fn              get_fan_info;
} lai_fan_api_t;


#endif // __LAIFAN_H_

