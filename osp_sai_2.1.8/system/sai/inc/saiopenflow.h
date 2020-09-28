/*
* Copyright (c) 2014 Microsoft Open Technologies, Inc.
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
*    Microsoft would like to thank the following companies for their review and
*    assistance with these files: Intel Corporation, Mellanox Technologies Ltd,
*    Dell Products, L.P., Facebook, Inc
*
* Module Name:
*
*    saiisolation.h
*
* Abstract:
*
*    This module defines SAI isolation API
*
*/

#if !defined (__SAIOPENFLOW_H_)
#define __SAIOPENFLOW_H_

#include <saitypes.h>

/** \defgroup SAIOPENFLOW SAI - openflow specific API definitions.
 *
 *  \{
 */

/**
 * @brief Create openflow port.
 *
 * @param[out] of_port_id Openflow Port id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_create_openflow_port_fn)(
        _In_  sai_object_id_t port_id,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list);

/**
 * @brief Remove openflow port.
 *
 * @param[in] of_port_id Openflow Port id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_remove_openflow_port_fn)(
        _In_ sai_object_id_t port_id);

/**
 * @brief OPENFLOW method table retrieved with sai_api_query()
 */
typedef struct _sai_openflow_api_t
{
    sai_create_openflow_port_fn create_openflow_port;
    sai_remove_openflow_port_fn remove_openflow_port;
} sai_openflow_api_t;

/**
 *\}
 */
#endif // __SAIOPENFLOW_H_

