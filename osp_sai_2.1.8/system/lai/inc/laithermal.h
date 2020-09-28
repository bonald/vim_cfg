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

#if !defined (__LAITHERMAL_H_)
#define __LAITHERMAL_H_

#include "laitype.h"


typedef enum _lai_thermal_event_t
{
    /** Create a new thermal*/
    LAI_THERMAL_EVENT_ADD,

    /** Delete/Invalidate an existing thermal */
    LAI_THERMAL_EVENT_DELETE,

} lai_thermal_event_t;


/**
 * Routine Description:
 *   @brief thermal event notification
 *
 * Arguments:
 *    @param[in] info- thermal information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_thermal_event_notification_fn)(
    _In_ lai_thermal_event_t  event,
    _In_ lai_thermal_info_t* thermal
    );

/**
 * Routine Description:
 *    @brief get thermal information
 *
 * Arguments:
 *    @param[in] oid - thermal oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_thermal_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_thermal_info_t* thermal
    );

/**
 * Routine Description:
 *    @brief set thermal information
 *
 * Arguments:
 *    @param[in] oid - thermal oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_thermal_info_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_thermal_info_t* thermal
    );

/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_thermal_api_t
{
    lai_thermal_info_get_fn              get_thermal_info;
    lai_thermal_info_set_fn              set_thermal_info;
} lai_thermal_api_t;

#endif // __LAITHERMAL_H_



