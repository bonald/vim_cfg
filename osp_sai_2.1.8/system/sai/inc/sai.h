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
*    sai.h
*
* Abstract:
*
*   This module defines an entry point into Switch Abstraction Interfrace (SAI)
*
*/
#if !defined (__SAI_H_)
#define __SAI_H_

#include <saitypes.h>
#include <saiexttypes.h>
#include <saistatus.h>
#include <saiswitch.h>
#include <saiport.h>
#include <saivlan.h>
#include <saifdb.h>
#include <sairouter.h>
#include <sairouterintf.h>
#include <saineighbor.h>
#include <sainexthop.h>
#include <sainexthopgroup.h>
#include <sairoute.h>
#include <saiacl.h>
#include <saihostintf.h>
#include <saimirror.h>
#include <saistp.h>
#include <saisamplepacket.h>
#include <sailag.h>
#include <saipolicer.h>
#include <saiwred.h>
#include <saiqosmaps.h>
#include <saiqueue.h>
#include <saischeduler.h>
#include <saischedulergroup.h>
#include <saiisolation.h>   /* added by kcao for SAI_EXT_ISOLATION 2015-12-01 */
#include <sail2mc.h>        /* added by kcao for SAI_EXT_L2MC 2016-04-11 */
#include <saiopenflow.h>    /* added by kcao for SAI_EXT_OPENFLOW 2016-08-28 */
#include <saivlanclass.h>   /* added by yejl for SAI_EXT_VLANCLASS 2016-10-10 */ 
#include <saimacescape.h>   /* added by yejl for SAI_EXT_MACESCAPE 2016-11-09 */
#include <saitapgroup.h>
#include <saiptp.h>
#include <saicopp.h>
#include <saiudf.h>         /* added by huwx for SAI_UDF 2017-07-19 */
#include <saivrrp.h>        /* support vrrp modified by liwh for bug 45215, 2017-09-18 */
#include <saiipmc.h>
#include <sairpfgroup.h>
//#include <saibuffer.h>
#include <sainat.h>

/* modified by wanjj for some test */
//#include <saibuffer.h>

/** \defgroup SAI SAI - Entry point specific API definitions.
 *
 *  \{
 */

/**
 *
 * Defined API sets have assigned ID's. If specific api method table changes
 * in any way (method signature, number of methods), a new ID needs to be 
 * created (e.g. VLAN2) and old API still may need to be supported for 
 * compatibility with older adapter hosts.
 *
 */
typedef enum _sai_api_t
{    
    SAI_API_UNSPECIFIED      =  0, 
    SAI_API_SWITCH           =  1,  /**< sai_switch_api_t */
    SAI_API_PORT             =  2,  /**< sai_port_api_t */
    SAI_API_FDB              =  3,  /**< sai_fdb_api_t */
    SAI_API_VLAN             =  4,  /**< sai_vlan_api_t */
    SAI_API_VIRTUAL_ROUTER   =  5,  /**< sai_virtual_router_api_t */
    SAI_API_ROUTE            =  6,  /**< sai_route_api_t */
    SAI_API_NEXT_HOP         =  7,  /**< sai_next_hop_api_t */
    SAI_API_NEXT_HOP_GROUP   =  8,  /**< sai_next_hop_group_api_t */
    SAI_API_ROUTER_INTERFACE =  9,  /**< sai_router_interface_api_t */
    SAI_API_NEIGHBOR         = 10,  /**< sai_neighbor_api_t */
    SAI_API_ACL              = 11,  /**< sai_acl_api_t */
    SAI_API_HOST_INTERFACE   = 12,  /**< sai_host_interface_api_t */
    SAI_API_MIRROR           = 13,  /**< sai_mirror_api_t */
    SAI_API_SAMPLEPACKET     = 14,  /**< sai_samplepacket_api_t */
    SAI_API_STP              = 15,  /**< sai_stp_api_t */
    SAI_API_LAG              = 16,  /**< sai_lag_api_t */
    SAI_API_POLICER          = 17,  /**< sai_policer_api_t */
    SAI_API_WRED             = 18,  /**< sai_wred_api_t*/
    SAI_API_QOS_MAPS         = 19,  /**< sai_qos_map_api_t*/
    SAI_API_QUEUE            = 20,  /**< sai_queue_api_t*/
    SAI_API_SCHEDULER        = 21,  /**< sai_scheduler_api_t*/
    SAI_API_SCHEDULER_GROUP  = 22,  /**< sai_scheduler_group_api_t*/
    SAI_API_BUFFERS          = 23,  /**< sai_buffer_api_t */
    SAI_API_HASH             = 24,  /**< sai_hash_api_t */
    SAI_API_UDF              = 25,  /**< sai_udf_api_t */
    SAI_API_ISOLATION        = 26,  /**< sai_isolation_api_t */ /* added by kcao for SAI_EXT_ISOLATION 2015-12-01 */
    SAI_API_L2MC             = 27,  /**< sai_l2mc_api_t */      /* added by kcao for SAI_EXT_L2MC 2016-04-05 */
    SAI_API_OPENFLOW         = 28,  /**< sai_openflow_api_t */  /* added by kcao for SAI_EXT_OPENFLOW 2016-08-28 */
    SAI_API_VLAN_CLASS       = 29,  /**< sai_vlan_class_api_t */ /* added by yejl for SAI_EXT_VLANCLASS 2016-10-10 */
    SAI_API_MAC_ESCAPE       = 30,  /**< sai_mac_escape_api_t */ /* added by yejl for SAI_EXT_MACESCAPE 2016-11-09 */
    SAI_API_TAP_GROUP        = 31,  /**< sai_tap_group_api_t */ /* added by xuwj for SAI_EXT_TAPGROUP 2017-02-15 */
    SAI_API_PTP              = 32,  /**< sai_ptp_api_t */       /* added by kcao for SAI_EXT_PTP 2017-02-15 */
    SAI_API_COPP             = 33,  /**< sai_copp_api_t */       /* added by chenyq for SAI_EXT_COPP 2017-05-11 */
    SAI_API_VRRP             = 34,  /**< sai_vrrp_api_t */       /* added by vrrp for SAI_EXT_VRRP 2017-09-18 */
    SAI_API_IPMC_GROUP       = 35,
    SAI_API_RPF_GROUP        = 36,
    SAI_API_IPMC             = 37,
    SAI_API_VLANMAP          = 38,
    SAI_API_NAT              = 39,  /**< sai_nat_api_t */       /* added by taocy for NAT 2019/06/04 */
     SAI_API_APS_GROUP        = 40,
    SAI_API_CFM          = 41,
    SAI_API_MAX
} sai_api_t;

typedef enum _sai_log_level_t
{
    SAI_LOG_DEBUG            = 1,
    SAI_LOG_INFO             = 2,
    SAI_LOG_NOTICE           = 3,
    SAI_LOG_WARN             = 4,
    SAI_LOG_ERROR            = 5,
    SAI_LOG_CRITICAL         = 6
} sai_log_level_t;

/**
 *   Method table that contains function pointers for services exposed by adapter
 *   host for adapter.
 */

typedef struct _service_method_table_t
{
    /** Get variable value given its name */
    const char* (*profile_get_value)(_In_ sai_switch_profile_id_t profile_id,
                                     _In_ const char* variable);

    /** Enumerate all the K/V pairs in a profile. 
     *  Pointer to NULL passed as variable restarts enumeration.
     *   Function returns 0 if next value exists, -1 at the end of the list. */
    int (*profile_get_next_value)(_In_ sai_switch_profile_id_t profile_id,
                                  _Out_ const char** variable,
                                  _Out_ const char** value);

} service_method_table_t;



/**
 * Routine Description:
 *     @brief Adapter module initialization call. This is NOT for SDK initialization.
 *
 * Arguments:
 *     @param[in] flags    - reserved for future use, must be zero
 *     @param[in] services - methods table with services provided by adapter host
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t
sai_api_initialize(
    _In_ uint64_t flags,
    _In_ const service_method_table_t* services
    );


/**
 * Routine Description:
 *     Retrieve a pointer to the C-style method table for desired SAI 
 *     functionality as specified by the given sai_api_id.
 *
 * Arguments:
 *     @param[in]  sai_api_id       - SAI api ID
 *     @param[out] api_method_table - Caller allocated method table
 *           The table must remain valid until the sai_api_uninitialize() is called 
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t
sai_api_query(
    _In_ sai_api_t sai_api_id,
    _Out_ void** api_method_table
    );


/**
 * Routine Description:
 *   Uninitialization of the adapter module. SAI functionalities, retrieved via 
 *   sai_api_query() cannot be used after this call.
 *
 * Arguments:
 *   None
 *
 * Return Values:
 *   @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t
sai_api_uninitialize(
    void
    );

/**
 * Routine Description:
 *     @brief Set log level for sai api module. The default log level is SAI_LOG_WARN.
 *
 * Arguments:
 *     @param[in] sai_api_id - SAI api ID
 *     @param[in] log_level  - log level
 *
 * Return Values:
 *     @return  SAI_STATUS_SUCCESS on success
 *              Failure status code on error
 */
sai_status_t 
sai_log_set(
    _In_ sai_api_t sai_api_id,
    _In_ sai_log_level_t log_level
    );

/**
 * Routine Description:
 *     @brief  Query sai object type.
 *
 * Arguments:
 *     @param[in] sai_object_id
 *
 * Return Values:
 *    @return  Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
 *             Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
 */
sai_object_type_t 
sai_object_type_query(
    _In_ sai_object_id_t sai_object_id
    );

/**
 * \}
 */
#endif  // __SAI_H_
