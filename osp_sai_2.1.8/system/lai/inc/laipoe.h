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

#if !defined (__LAIPOE_H_)
#define __LAIPOE_H_

#include "laitype.h"


typedef enum _lai_poe_event_t
{
    /** Create a new poe*/
    LAI_POE_EVENT_ADD,

    /** Delete/Invalidate an existing poe */
    LAI_POE_EVENT_DELETE,

} lai_poe_event_t;


/**
 * Routine Description:
 *   @brief poe event notification
 *
 * Arguments:
 *    @param[in] info- poe information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_poe_event_notification_fn)(
    _In_ lai_poe_event_t  event,
    _In_ lai_poe_info_t* poe
    );

/**
 * Routine Description:
 *    @brief get poe information
 *
 * Arguments:
 *    @param[in] oid - poe oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_poe_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_poe_info_t* poe
    );


/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_poe_api_t
{
    lai_poe_info_get_fn              get_poe_info;
} lai_poe_api_t;

#endif // __LAIPOE_H_


