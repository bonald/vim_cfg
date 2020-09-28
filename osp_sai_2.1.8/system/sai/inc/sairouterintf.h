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
*    sairouterintf.h
*
* Abstract:
*
*    This module defines SAI Router Interface
*
*/

#if !defined (__SAIROUTERINTF_H_)
#define __SAIROUTERINTF_H_

#include <saitypes.h>

/** \defgroup SAIROUTERINTF SAI - Router interface specific API definitions.
 *
 *  \{
 */
 
/**
 *  @brief Attribute data for SAI_ROUTER_INTERFACE_ATTR_TYPE
 */
typedef enum _sai_router_interface_type_t 
{
    /** Port or Lag Router Interface Type */
    SAI_ROUTER_INTERFACE_TYPE_PORT,

    /** VLAN Router Interface Type */
    SAI_ROUTER_INTERFACE_TYPE_VLAN

} sai_router_interface_type_t;


/**
 *  @brief Routing interface attribute IDs 
 */
typedef enum _sai_router_interface_attr_t
{
    /** READ-ONLY */

    /** Virtual router id [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
    SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,

    /** Type [sai_router_interface_type_t]  (MANDATORY_ON_CREATE|CREATE_ONLY) */
    SAI_ROUTER_INTERFACE_ATTR_TYPE,

    /** Assosiated Port or Lag object id [sai_object_id_t] 
    *  (MANDATORY_ON_CREATE when SAI_ROUTER_INTERFACE_ATTR_TYPE == SAI_ROUTER_INTERFACE_TYPE_PORT | CREATE_ONLY) 
    */
    SAI_ROUTER_INTERFACE_ATTR_PORT_ID,

    /** Assosiated Vlan [sai_vlan_id_t] 
    *  (MANDATORY_ON_CREATE when SAI_ROUTER_INTERFACE_ATTR_TYPE == SAI_ROUTER_INTERFACE_TYPE_VLAN | CREATE_ONLY)
    */
    SAI_ROUTER_INTERFACE_ATTR_VLAN_ID,

    /** READ-WRITE */

    /** MAC Address [sai_mac_t] (CREATE_AND_SET)
        (default to SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS if not set on create) */
    SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,

    /** Admin V4 state [bool] (CREATE_AND_SET) (default to TRUE) */
    SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,

    /** Admin V4Mcast state [bool] (CREATE_AND_SET) (default to FALSE) */
    SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE,

    /** Admin V6 state [bool] (CREATE_AND_SET) (default to TRUE) */
    SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE,

    /** IGMP SNOOP ENABLE state [bool] (CREATE_AND_SET) (default to FALSE) */
    SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN,

    /** MTU [uint32_t] (CREATE_AND_SET) (default to 1514 bytes) */
    SAI_ROUTER_INTERFACE_ATTR_MTU,

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    /** add VMAC Address [sai_mac_t] (SET) */
    SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS,
	
	/*add by zhw for mc*/
	/** add multicase */
	SAI_ROUTER_INTERFACE_ATTR_MULTICAST,
	
	/*add igmp(leave message) copy to cpu*/
	SAI_ROUTER_INTERFACE_ATTR_IGMP,
	/*end add zhw*/

    /* added by hansf for support l3if set trust 2018/09/20*/
    /** Trust_dscp [bool] (CREATE_AND_SET) (default to FALSE) */
    SAI_ROUTER_INTERFACE_ATTR_TRUST_DSCP,

    /* added by hansf for support l3if set qos domain 2018/09/20*/
    /** Qos map [[sai_object_id_t]] */
    SAI_ROUTER_INTERFACE_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP,

    /* added by hansf for support l3if set qos domain 2018/09/20*/
    /** Qos map [[sai_object_id_t]] */
    SAI_ROUTER_INTERFACE_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,

    /* added by taocy for NAT, 2019-06-03*/
    SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID,

    SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE,
    /* -- */

    /* Custom range base value */
    SAI_ROUTER_INTERFACE_ATTR_CUSTOM_RANGE_BASE  = 0x10000000,

    SAI_ROUTER_INTERFACE_ATTR_URPF,
    
    SAI_ROUTER_INTERFACE_ATTR_URPF_V6,

} sai_router_interface_attr_t;

typedef enum _sai_router_interface_l3_type_t
{ 
  SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL = 0,
  SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL = 1
}sai_router_interface_l3_type_t;

#define SAI_ROUTER_INTERFACE_NAT_DISABLE           0
#define SAI_ROUTER_INTERFACE_NAT_INSIDE           (1 << 0)
#define SAI_ROUTER_INTERFACE_NAT_OUTSIDE          (1 << 1)

/**
 * Routine Description:
 *    @brief Create router interface. 
 *
 * Arguments:
 *    @param[out] rif_id - router interface id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t(*sai_create_router_interface_fn)(
    _Out_ sai_object_id_t* rif_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief Remove router interface
 *
 * Arguments:
 *    @param[in] rif_id - router interface id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t(*sai_remove_router_interface_fn)(
    _In_ sai_object_id_t rif_id
    );

/**
 * Routine Description:
 *    @brief Set router interface attribute
 *
 * Arguments:
 *    @param[in] rif_id - router interface id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_set_router_interface_attribute_fn)(
    _In_ sai_object_id_t rif_id,
    _In_ const sai_attribute_t *attr
    );


/**
 * Routine Description:
 *    @brief Get router interface attribute
 *
 * Arguments:
 *    @param[in] rif_id - router interface id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_get_router_interface_attribute_fn)(
    _In_ sai_object_id_t rif_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );

/**
 *  @brief Routing interface methods table retrieved with sai_api_query()
 */
typedef struct _sai_router_interface_api_t
{
    sai_create_router_interface_fn          create_router_interface;
    sai_remove_router_interface_fn          remove_router_interface;
    sai_set_router_interface_attribute_fn   set_router_interface_attribute;
    sai_get_router_interface_attribute_fn   get_router_interface_attribute;

} sai_router_interface_api_t;

/**
 * \}
 */
#endif // __SAIROUTERINTF_H_

