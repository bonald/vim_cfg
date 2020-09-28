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
*    sairoute.h
*
* Abstract:
*
*    This module defines SAI Route Entry API
*
*/

#if !defined (__SAIAPSGROUP_H_)
#define __SAIAPSGROUP_H_

#include <saitypes.h>

typedef enum _sai_aps_group_type_t
{
 
    SAI_APS_GROUP_MGROUP_TYPE_ADD,
    SAI_APS_GROUP_MGROUP_TYPE_RING_ID,
    SAI_APS_GROUP_MGROUP_TYPE_ID,
    SAI_APS_GROUP_TYPE_ID,
    SAI_APS_GROUP_TYPE_NH_ID,
    SAI_APS_GROUP_TYPE_NH_OFFSET, 
    SAI_APS_GROUP_TYPE_INSTANCE_ID,
    SAI_APS_GROUP_TYPE_PORT_ID,
    SAI_APS_GROUP_TYPE_PORT_STATE,
    
} sai_aps_group_type_t;

typedef enum _sai_port_raps_port_state_t
{
    /** Port is in Learning mode*/
    SAI_PORT_APS_STATE_LEARNING,

    /** Port is in Forwarding mode */
    SAI_PORT_APS_STATE_FORWARDING,

    /** Port is in Blocking mode */
    SAI_PORT_APS_STATE_BLOCKING,

} sai_port_raps_port_state_t;

typedef sai_status_t (*ctc_sai_create_raps_mc_group_fn)(
    _In_ uint16_t group_id
    );

typedef sai_status_t(*ctc_sai_destroy_raps_mc_group_fn)(
    _In_ uint16_t group_id);

/**
 * Routine Description:
 *    @brief Remove Route
 *
 * Arguments:
 *    @param[in] unicast_route_entry - route entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 */

/**
 * Routine Description:
 *    @brief Set route attribute value
 *
 * Arguments:
 *    @param[in] unicast_route_entry - route entry
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */

typedef sai_status_t (*ctc_sai_aps_mgroup_update_member_port_fn)(
	_In_ uint32_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list);

typedef sai_status_t (*ctc_sai_add_aps_group_fn)(
	_In_ uint32_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list);

typedef sai_status_t (*ctc_sai_aps_set_port_state_fn)(
	_In_ uint32_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list);

typedef sai_status_t (*ctc_sai_aps_set_protection_en_fn)(
	_In_ uint16_t aps_group_id,
	_In_ uint8_t enable);

typedef sai_status_t (*ctc_sai_aps_delete_aps_group_fn)(
	_In_ uint16_t aps_group_id,
	_In_ uint32_t nh_id);

typedef struct _sai_aps_group_api_t
{
    ctc_sai_create_raps_mc_group_fn            create_raps_mc_group;
    ctc_sai_destroy_raps_mc_group_fn         destroy_raps_mc_group;
    ctc_sai_aps_mgroup_update_member_port_fn   aps_mgroup_update_member_port;
    ctc_sai_add_aps_group_fn                   add_aps_group;
    ctc_sai_aps_set_port_state_fn              set_port_state;
    ctc_sai_aps_set_protection_en_fn     set_protection_en;
    ctc_sai_aps_delete_aps_group_fn     delete_aps_group;
    
} sai_aps_group_api_t;

/**
 * \}
 */
#endif // __SAIROUTE_H_
