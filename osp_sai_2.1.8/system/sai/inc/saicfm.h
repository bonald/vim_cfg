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
*    saineighbor.h
*
* Abstract:
*
*    This module defines SAI cfm table API
*
*/

#if !defined (__SAICFM_H_)
#define __SAICFM_H_

#include <saitypes.h>


/**
 * Routine Description:
 *    @brief Set cfm enable globally
 *
 * Arguments:
 *    @param[in] enable - cfm enable globally
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_enable_fn)(
     _In_ sai_object_id_t port_id,
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief add cfm maid
 *
 * Arguments:
 *    @param[in] attr_list - cfm maid
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_add_maid_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief delete cfm maid
 *
 * Arguments:
 *    @param[in] attr_list - cfm maid
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_del_maid_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief add cfm local mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_add_lmep_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief delete cfm local mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm del lmep
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_del_lmep_fn)(
     _Inout_ sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief set cfm cc enable
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_set_cc_enable_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief set cfm ccm vlan cos
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_set_ccm_vlan_cos_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief add cfm remote mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm remote mep
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_add_rmep_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief delete cfm remote mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm remote mep
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_del_rmep_fn)(
     _Inout_ sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief set cfm rdi
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_cfm_set_rdi_fn)(
     _In_ const sai_attribute_t *attr_list
    );

/**
 *  @brief neighbor table methods, retrieved via sai_api_query()
 */
typedef struct _sai_cfm_api_t
{
    sai_cfm_enable_fn        cfm_enable;
    sai_cfm_add_maid_fn   cfm_add_maid;
    sai_cfm_del_maid_fn    cfm_del_maid;
    sai_cfm_add_lmep_fn   cfm_add_lmep;
    sai_cfm_del_lmep_fn    cfm_del_lmep;
    sai_cfm_set_cc_enable_fn    cfm_set_cc_enable;
    sai_cfm_set_ccm_vlan_cos_fn  cfm_set_ccm_vlan_cos;
    sai_cfm_add_rmep_fn   cfm_add_rmep;
    sai_cfm_del_rmep_fn    cfm_del_rmep;
    sai_cfm_set_rdi_fn        cfm_set_rdi;
} sai_cfm_api_t;

/**
 * \}
 */
#endif // __SAICFM_H_

