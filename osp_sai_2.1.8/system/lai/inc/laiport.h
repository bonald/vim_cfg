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

#if !defined (__LAIPORT_H_)
#define __LAIPORT_H_

#include "laitype.h"


typedef enum _lai_port_event_t
{
    /** Create a new port*/
    LAI_PORT_EVENT_ADD,

    /** Delete/Invalidate an existing port */
    LAI_PORT_EVENT_DELETE,

    /** Port status to Link-UP*/
    LAI_PORT_EVENT_UP,

    /**Port status to Link-down */
    LAI_PORT_EVENT_DOWN,

} lai_port_event_t;


/**
 * Routine Description:
 *   @brief port event notification
 *
 * Arguments:
 *    @param[in] info- port information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_port_event_notification_fn)(
    _In_ lai_port_event_t  event,
    _In_ lai_port_info_t* port
    );

/**
 * Routine Description:
 *    @brief get port information
 *
 * Arguments:
 *    @param[in] oid - port oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_port_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_port_info_t* port
    );


/**
 * @brief PORT methods table retrieved with lai_api_query()
 */
typedef struct _lai_port_api_t
{
    lai_port_info_get_fn              get_port_info;
} lai_port_api_t;


#endif // __LAIPORT_H_


