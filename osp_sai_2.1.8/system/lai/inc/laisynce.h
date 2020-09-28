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

#if !defined (__LAISYNCE_H_)
#define __LAISYNCE_H_

#include "laitype.h"

typedef enum _lai_synce_event_t
{
    /** synce update DPLL State*/
    LAI_SYNCE_UPDATE_DPLL_STATE,

} lai_synce_event_t;

/**
 * Routine Description:
 *   @brief synce event notification
 *
 * Arguments:
 *    @param[in] event - event information
 *    @param[in] synce_info  - synce information 

 * Return Values:
 *    None
 */
typedef void (*lai_synce_dpll_state_notification_fn)(
    lai_synce_event_t event,
    lai_synce_info_t* synce_info
    );
/**
 * Routine Description:
 *    @brief set synce attribute
 *
 * Arguments:
 *    @param[in] synce_info - synce information 
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_synce_set_attribute_fn)(
    _In_ lai_synce_info_t* synce_info
    );

/**
 * @brief FAN methods table retrieved with lai_api_query()
 */
typedef struct _lai_synce_api_t
{
    lai_synce_set_attribute_fn                set_synce_attribute; 
} lai_synce_api_t;

#endif // __LAISYNCE_H_

