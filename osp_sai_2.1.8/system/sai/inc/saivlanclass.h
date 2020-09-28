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
*    saivlan.h
*
* Abstract:
*
*    This module defines SAI VLAN API
*
*/

#if !defined (__SAIVLANCLASS_H_)
#define __SAIVLANCLASS_H_

#include <saitypes.h>

/** \defgroup SAIVLANCLASS SAI - VLANCLASS specific API definitions.
 *
 *  \{
 */

/**
 *  @brief VLANCLASS Entry type
 */
typedef enum _sai_vlanclass_entry_type_t
{
    SAI_VLAN_CLASS_ENTRY_TYPE_MAC,
    SAI_VLAN_CLASS_ENTRY_TYPE_IPV4,
    SAI_VLAN_CLASS_ENTRY_TYPE_IPV6,
    SAI_VLAN_CLASS_ENTRY_TYPE_PROTOCOL,
} sai_vlanclass_entry_type_t;

/**
 *  @brief VLANCLASS Protocol type
 */
typedef enum _sai_vlanclass_protocol_type_t
{
    SAI_VLAN_CLASS_PROTOCOL_TYPE_NONE,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV4,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV6,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS_MCAST,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_ARP,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_RARP,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_PPPOE,
    SAI_VLAN_CLASS_PROTOCOL_TYPE_MAX,
} sai_vlanclass_protocol_type_t;


/**
 *  @brief List of VLANCLASS GROUP attributes
 */
typedef enum _sai_vlanclass_group_attr_t {

    /** READ-ONLY */

    /* SAI rule list ID [sai_object_list_t] */
    SAI_VLANCLASS_GROUP_ATTR_RULE_LIST,

    /** Custom range base value */
    SAI_VLANCLASS_GROUP_ATTR_CUSTOM_RANGE_BASE  = 0x10000000

} sai_vlanclass_group_attr_t;


/*
    \brief Create VLANCLASS GROUP
    \param[out] group_id VLANCLASS GROUP ID
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t(*sai_create_vlanclass_group_fn)(
    _Out_ sai_object_id_t* group_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list
    );

/*
    \brief Remove VLANCLASS GROUP ID
    \param[in] group_id VLANCLASS GROUP ID
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t(*sai_delete_vlanclass_group_fn)(
    _In_ sai_object_id_t  group_id
    );


/*
    \brief Set VLANCLASS GROUP Attribute
    \param[in] group_id VLANCLASS GROUP ID
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t (*sai_set_vlanclass_group_attribute_fn)(
    _In_ sai_object_id_t  group_id,
    _In_ const sai_attribute_t *attr
    );

/*
    \brief Get VLANCLASS GROUP Attribute
    \param[in] group_id VLANCLASS GROUP ID
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/

typedef sai_status_t (*sai_get_vlanclass_group_attribute_fn)(
    _In_ sai_object_id_t group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );



/**
 *  @brief List of VLANCLASS GROUP member attributes
*/
typedef enum _sai_vlanclass_group_member_attr_t
{
    /** READ_WRITE */

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** GROUP ID [sai_object_id_t] */
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** RULE ID [uint32_t] */
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID,
    
    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Vlanclass Entry type MAC/IPV4/IPV6/PROTOTOL [sai_vlanclass_entry_type_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Valid for Vlanclass Entry VlanId - [sai_vlan_id_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Valid for Vlanclass Entry MAC [sai_mac_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS,
    
    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Valid for Vlanclass Entry IP [sai_ip4_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Valid for Vlanclass Entry IP [sai_ip6_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** Vlanclass Entry Protocol type [sai_vlanclass_protocol_type_t]*/
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE,
    
    /** Custom range base value */
    SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_CUSTOM_RANGE_BASE  = 0x10000000

} sai_vlanclass_rule_attr_t;


/**
 * @brief Create vlanclass Group Member id.
 *
 * @param[out] group_member_id 
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_creat_vlanclass_group_member_fn)(
        _Out_ sai_object_id_t *group_member_id,
        _In_  uint32_t attr_count,
        _In_  sai_attribute_t *attr_list);


/**
 * @brief Remove vlanclass Group Member id.
 *
 * @param[in] vlanclass rule id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_delete_vlanclass_group_member_fn)(
        _In_ sai_object_id_t group_member_id);


/*
    \brief Set Vlanclass Group Member Attribute
    \param[in] rule_id RULE ID
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
typedef sai_status_t (*sai_set_vlanclass_group_member_attribute_fn)(
    _In_ sai_object_id_t  group_member_id,
    _In_ const sai_attribute_t *attr
    );

/**
 * Routine Description:
 *    @brief Get Vlanclass Group Member Attribute
 *
 * Arguments:
 *    @param[in] group_member_id 
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_get_vlanclass_group_member_attribute_fn)(
        _In_ sai_object_id_t group_member_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);


/**
 * @brief VLANCLASS methods table retrieved with sai_api_query()
 */

typedef struct _sai_vlanclass_api_t
{
    sai_create_vlanclass_group_fn                  create_vlanclass_group;
    sai_delete_vlanclass_group_fn                  delete_vlanclass_group;
    sai_set_vlanclass_group_attribute_fn           set_vlanclass_group_attribute;
    sai_get_vlanclass_group_attribute_fn           get_vlanclass_group_attribute;
    sai_creat_vlanclass_group_member_fn            create_vlanclass_group_member;
    sai_delete_vlanclass_group_member_fn           delete_vlanclass_group_member;
    sai_set_vlanclass_group_member_attribute_fn    set_vlanclass_group_member_attribute;
    sai_get_vlanclass_group_member_attribute_fn    get_vlanclass_group_member_attribute;
} sai_vlanclass_api_t;

/**
 *\}
 */
#endif // __SAIVLANCLASS_H_

