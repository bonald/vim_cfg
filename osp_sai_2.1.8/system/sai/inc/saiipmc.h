/**
 * Copyright (c) 2014 Microsoft Open Technologies, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *    THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *    FOR A PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 *    See the Apache Version 2.0 License for specific language governing
 *    permissions and limitations under the License.
 *
 *    Microsoft would like to thank the following companies for their review and
 *    assistance with these files: Intel Corporation, Mellanox Technologies Ltd,
 *    Dell Products, L.P., Facebook, Inc., Marvell International Ltd.
 *
 * @file    saiipmc.h
 *
 * @brief   This module defines SAI IPMC interface
 */

#if !defined (__SAIIPMC_H_)
#define __SAIIPMC_H_

#include <saitypes.h>

/**
 * @brief Enum defining ipmc entry types.
 */
typedef enum _sai_ipmc_entry_type_t
{
    /** IPMC entry with type (S,G) */
    SAI_IPMC_TYPE_SG = 0x00000001,

    /** IPMC entry with type (*,G) */
	SAI_IPMC_TYPE_XG = 0x00000002,

} sai_ipmc_entry_type_t;

/*
*  IP multicast entry key
*/
typedef struct _sai_ipmc_entry_t
{
	sai_object_id_t vrf_id;
	sai_ipmc_entry_type_t type;
	sai_ip_address_t destination;
    sai_ip_address_t source;
} sai_ipmc_entry_t;

/*
*  Attribute Id for IP multicast entry
*/
typedef enum _sai_ipmc_entry_attr_t 
{
    SAI_IPMC_ENTRY_ATTR_START,
    /** READ-ONLY */

    /** READ-WRITE */
    /** Packet action [sai_packet_action_t]
       (default to SAI_PACKET_ACTION_FORWARD) */
    SAI_IPMC_ATTR_PACKET_ACTION = SAI_IPMC_ENTRY_ATTR_START,

    /** Packet priority for trap/log actions [uint8_t]
       (default to 0) */
    SAI_IPMC_ATTR_TRAP_PRIORITY,

	/** Next hop group id for the packet [sai_object_id_t]	
		* The next hop group type should be SAI_NEXT_HOP_GROUP_IPMC 
		* (MANDATORY_ON_CREATE when SAI_IPMC_ATTR_PACKET_ACTION == SAI_PACKET_ACTION_FORWARD)
		* (CREATE_AND_SET) */
	SAI_IPMC_ATTR_NEXT_HOP_GROUP_ID,
	SAI_IPMC_ATTR_RPF_ROUTER_INTERFACE,

    /* Custom range base value */
    SAI_IPMC_ATTR_CUSTOM_RANGE_BASE  = 0x10000000,

    SAI_IPMC_ATTR_IPMC_MEMBER,

	/* IPMC stats defined here*/
	/*READ ONLY*/
	/*sai_uint64_t*/
	SAI_IPMC_COUNTER_ATTR_COUNTER_PACKETS,

	/*READ ONLY*/
	/*sai_uint64_t*/
	SAI_IPMC_COUNTER_ATTR_COUNTER_BYTES
} sai_ipmc_entry_attr_t;

/*
* Routine Description:
*    Create IP multicast entry
*
* Arguments:
*    [in] ipmc_entry - IP multicast entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
typedef sai_status_t (*sai_create_ipmc_entry_fn)(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
);

/*
* Routine Description:
*    Remove IP multicast entry
*
* Arguments:
*    [in] ipmc_entry - IP multicast entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
typedef sai_status_t (*sai_remove_ipmc_entry_fn)(
    _In_ const sai_ipmc_entry_t* ipmc_entry
    );

/*
* Routine Description:
*    Set IP multicast entry attribute value
*
* Arguments:
*    [in] IP multicast - IP multicast entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
typedef sai_status_t (*sai_set_ipmc_entry_attribute_fn)(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ const sai_attribute_t *attr
    );

/*
* Routine Description:
*    Get IP multicast entry attribute value
*
* Arguments:
*    [in] ipmc_entry - IP multicast entry
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
typedef sai_status_t (*sai_get_ipmc_entry_attribute_fn)(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );

/*
* IP multicast method table retrieved with sai_api_query()
*/
typedef struct _sai_ipmc_api_t
{
    sai_create_ipmc_entry_fn                     create_ipmc_entry;
    sai_remove_ipmc_entry_fn                     remove_ipmc_entry;
    sai_set_ipmc_entry_attribute_fn              set_ipmc_entry_attribute;
    sai_get_ipmc_entry_attribute_fn              get_ipmc_entry_attribute;
} sai_ipmc_api_t;


#endif /** __SAIIPMC_H_ */
