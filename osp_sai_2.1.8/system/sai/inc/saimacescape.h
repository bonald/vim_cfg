/*
* Copyright (c) 2015 Dell Inc.
*
*    Licensed under the Apache License, Version 2.0 (the "License"); you may
*    not use this file except in compliance with the License. You may obtain
*    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
*    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
*    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
*    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
*    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
*    See the Apache Version 2.0 License for specific language governing
*    permissions and limitations under the License.
*
*/
/**
* Module Name:
*
* saimacescape.h
*
* Abstract:
*
* This module defines SAI mac escape
*
*/

#if !defined (__SAIMACESCAPE_H_)
#define __SAIMACESCAPE_H_

#include "saitypes.h"
#include "saistatus.h"


/**
 *  @brief mace_scape entry key
 */
typedef struct _sai_mac_escape_entry_t
{
    sai_mac_t mac;
    sai_mac_t mask;

} sai_mac_escape_entry_t;

/**
 * @brief Create mac escape.
 *
 * @param[in] macescape_entry mac escape entry
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_create_mac_escape_fn)(
        _In_ const sai_mac_escape_entry_t *macescape_entry);


/**
 * @brief delete mac escape.
 *
 * @param[in] macescape_entry mac escape entry
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_delete_mac_escape_fn)(
         _In_ const sai_mac_escape_entry_t *macescape_entry);


/**
 * @brief MACESCAPE method table retrieved with sai_api_query()
 */
typedef struct _sai_mac_escape_api_t
{
    sai_create_mac_escape_fn create_mac_escape_entry;
    sai_delete_mac_escape_fn delete_mac_escape_entry;
} sai_mac_escape_api_t;

/**
 * \}
 */

#endif // __SAIMACESCAPE_H_

