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
*    sail2mc.h
*
* Abstract:
*
*    This module defines SAI L2MC API
*
*/

#if !defined (__SAIL2MC_H_)
#define __SAIL2MC_H_

#include <saitypes.h>

/** \defgroup SAIL2MC SAI - L2MC specific API definitions.
 *
 *  \{
 */

/*
*  @brief L2 multicast entry key
*/
typedef struct _sai_l2mc_entry_t
{
    sai_mac_t mac_address;
    sai_vlan_id_t vlan_id;

} sai_l2mc_entry_t;

/*
*  @brief Attribute Id for L2 mutlicast entry
*/
typedef enum _sai_l2mc_entry_attr_t 
{
    /* READ-ONLY */

    /* READ-WRITE */

    /* SAI port list [sai_object_list_t] (CREATE_AND_SET) */
    SAI_L2MC_ATTR_PORT_LIST,

    /* -- */

    /* Custom range base value */
    SAI_L2MC_ENTRY_ATTR_CUSTOM_RANGE_BASE  = 0x10000000

} sai_l2mc_entry_attr_t;

/*
* Routine Description:
*    @brief Create L2 multicast entry
*
* Arguments:
*    @param[in] l2mc_entry - L2 multicast entry
*    @param[in] attr_count - number of attributes
*    @param[in] attr_list - array of attributes
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_create_l2mc_entry_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    );

/*
* Routine Description:
*    @brief Remove L2 multicast entry
*
* Arguments:
*    @param[in] l2mc_entry - L2 multicast entry
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_remove_l2mc_entry_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry
    );

/*
* Routine Description:
*    @brief add port list to L2 multicast
*
* Arguments:
*    @param[in] l2mc_entry - L2 multicast entry
*    @param[in] port_list  - port list
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_add_ports_to_l2mc_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ const sai_object_list_t *port_list
    );

/*
* Routine Description:
*    remove port list from L2 multicast
*
* Arguments:
*    @param[in] l2mc_entry - L2 multicast entry
*    @param[in] port_list  - port list
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_remove_ports_from_l2mc_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ const sai_object_list_t *port_list 
);

/*
* Routine Description:
*    @brief Set L2 multicast entry attribute value
*
* Arguments:
*    @param[in] L2 multicast - L2 multicast entry
*    @param[in] attr - attribute
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_set_l2mc_entry_attribute_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ const sai_attribute_t *attr
    );

/*
* Routine Description:
*    @brief Get L2 multicast entry attribute value
*
* Arguments:
*    @param[in] l2mc_entry - L2 multicast
*    @param[in] attr_count - number of attributes
*    @param[inout] attr_list - array of attributes
*
* Return Values:
*    @return SAI_STATUS_SUCCESS on success
*            Failure status code on error
*/
typedef sai_status_t (*sai_get_l2mc_entry_attribute_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );


typedef  sai_status_t(*sai_create_l2mc_entry_with_flag_fn)(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ uint32_t pkt_flag 
    );
/*
* @brief L2 multicast method table retrieved with sai_api_query()
*/
typedef struct _sai_l2mc_api_t
{
    sai_create_l2mc_entry_fn                     create_l2mc_entry;
    sai_remove_l2mc_entry_fn                     remove_l2mc_entry;
    sai_create_l2mc_entry_fn                     add_ports_to_l2mc;
    sai_create_l2mc_entry_fn                     remove_ports_from_l2mc;
    sai_set_l2mc_entry_attribute_fn              set_l2mc_entry_attribute;
    sai_get_l2mc_entry_attribute_fn              get_l2mc_entry_attribute;
    sai_create_l2mc_entry_with_flag_fn       create_l2mc_entry_with_flag;
} sai_l2mc_api_t;

/**
 * \}
 */
#endif // __SAIL2MC_H_

