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

#if !defined (__LAIFIBER_H_)
#define __LAIFIBER_H_

#include "laitype.h"


typedef enum _lai_fiber_event_t
{
    /** Create a new fiber*/
    LAI_FIBER_EVENT_ADD,

    /** Delete/Invalidate an existing fiber */
    LAI_FIBER_EVENT_DELETE,

} lai_fiber_event_t;


/**
 * Routine Description:
 *   @brief fiber event notification
 *
 * Arguments:
 *    @param[in] info- fiber information
 *    @param[in] data  - array of fiber events

 * Return Values:
 *    None
 */
typedef void (*lai_fiber_event_notification_fn)(
    _In_ lai_fiber_event_t  event,
    _In_ lai_fiber_info_t* fiber
    );

/**
 * Routine Description:
 *    @brief get fiber information
 *
 * Arguments:
 *    @param[in] oid - fiber oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_fiber_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_fiber_info_t* fiber
    );


/**
 * @brief FIBER methods table retrieved with lai_api_query()
 */
typedef struct _lai_fiber_api_t
{
    lai_fiber_info_get_fn              get_fiber_info;
} lai_fiber_api_t;


#endif // __LAIFIBER_H_



