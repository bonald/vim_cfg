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

#if !defined (__LAIVCM_H_)
#define __LAIVCM_H_

#include "laitype.h"


typedef enum _lai_vcm_event_t
{
    /** Create a new vcm*/
    LAI_VCM_EVENT_ADD,

    /** Delete/Invalidate an existing vcm */
    LAI_VCM_EVENT_DELETE,

} lai_vcm_event_t;


/**
 * Routine Description:
 *   @brief vcm event notification
 *
 * Arguments:
 *    @param[in] info- vcm information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_vcm_event_notification_fn)(
    _In_ lai_vcm_event_t  event,
    _In_ lai_vcm_info_t* vcm
    );

/**
 * Routine Description:
 *    @brief get vcm information
 *
 * Arguments:
 *    @param[in] oid - vcm oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_vcm_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_vcm_info_t* vcm
    );


/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_vcm_api_t
{
    lai_vcm_info_get_fn              get_vcm_info;
} lai_vcm_api_t;

#endif // __LAIVCM_H_


