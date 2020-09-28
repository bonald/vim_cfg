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
*    saivlanmap.h
*
* Abstract:
*
*    This module defines SAI VLAN MAPPING API
*
*/

#if !defined (__SAIVLANMAP_H_)
#define __SAIVLANMAP_H_

#include <saitypes.h>


/*
*  vlan mapping entry data struction
*/
typedef struct _sai_vlanmap_entry_t
{
    bool default_entry;
    bool replace;
    bool egress;
    bool untag_pvid;
    bool change_defvid;
    bool default_fwd;
    sai_object_id_t port_id;
    sai_vlan_id_t   raw_vid;
    sai_vlan_id_t   raw_vid_max;
    sai_vlan_id_t   def_vid;
    sai_vlan_id_t   mapped_svlan;
    sai_vlan_id_t   mapped_cvlan;
    sai_uint32_t    group_id;
} sai_vlanmap_entry_t;

/*
*  vlan mapping entry data struction
*/
typedef struct _sai_vlan_group_t
{
    sai_uint32_t    group_id;
    sai_vlan_id_t   vlanmin[8];
    sai_vlan_id_t   vlanmax[8]; 
} sai_vlan_group_t;


/*
    \brief Create VLANMAP GROUP
    \param[in] sai_vlanmap_entry_t vlan mapping entry
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t(*sai_create_vlanmap_entry_fn)(
    _In_ sai_vlanmap_entry_t* vm_entry,
    _In_ sai_attribute_t *attr_list
    );

/*
    \brief Remove VLANMAP GROUP ID
    \param[in] group_id VLANMAP GROUP ID
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t(*sai_delete_vlanmap_entry_fn)(
    _In_ sai_vlanmap_entry_t* vm_entry,
    _In_ sai_attribute_t *attr_list
    );

#if 0
/**
 *  @brief List of VLANMAP entry attributes
*/
typedef enum _sai_vlanmap_entry_attr_t
{
    /** READ_WRITE */

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** GROUP ID [sai_object_id_t] */
    //SAI_VLAN_MAP_ATTR_GROUP_ID,

    /** Custom range base value */
    SAI_VLAN_MAP_ATTR_CUSTOM_RANGE_BASE  = 0x10000000

} sai_vlanmap_entry_attr_t;
#endif

/**
 * @brief Create vlanmap Group Member id.
 *
 * @param[out] group_member_id 
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t(*sai_create_vlan_group_fn)(
        _In_ sai_vlan_group_t *vlan_group);


/**
 * @brief Remove vlanmap Group Member id.
 *
 * @param[in] vlanmap rule id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_delete_vlan_group_fn)(
        _In_ sai_object_id_t group_id);


typedef struct _sai_vlanmap_api_t
{
    sai_create_vlanmap_entry_fn        create_vlanmap_entry;
    sai_delete_vlanmap_entry_fn        delete_vlanmap_entry;
    sai_create_vlan_group_fn           create_vlan_group;
    sai_delete_vlan_group_fn           delete_vlan_group;
} sai_vlanmap_api_t;

/**
 *\}
 */
#endif // __SAIVLANMAP_H_

