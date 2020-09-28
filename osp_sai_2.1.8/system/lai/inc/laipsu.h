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

#if !defined (__LAIPSU_H_)
#define __LAIPSU_H_

#include "laitype.h"


typedef enum _lai_psu_event_t
{
    /** Create a new psu*/
    LAI_PSU_EVENT_ADD,

    /** Delete/Invalidate an existing psu */
    LAI_PSU_EVENT_DELETE,

} lai_psu_event_t;


/**
 * Routine Description:
 *   @brief psu event notification
 *
 * Arguments:
 *    @param[in] info- psu information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_psu_event_notification_fn)(
    _In_ lai_psu_event_t  event,
    _In_ lai_psu_info_t* psu
    );

/**
 * Routine Description:
 *    @brief get psu information
 *
 * Arguments:
 *    @param[in] oid - psu oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_psu_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_psu_info_t* psu
    );


/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_psu_api_t
{
    lai_psu_info_get_fn              get_psu_info;
} lai_psu_api_t;

#endif // __LAIPSU_H_


