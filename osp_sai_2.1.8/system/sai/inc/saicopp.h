/**
* Copyright (c) 2015 Dell Inc and cente.
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
* saicopp.h
*
* Abstract:
*
* This module defines SAI COPP API
*
*/

#if !defined (__SAICOPP_H_)
#define __SAICOPP_H_

#include "saitypes.h"
#include "saistatus.h"

/** \defgroup SAICOPP  SAI - STP specific public APIs and datastructures.
 *
 *  \{
 */


/**
 * @brief Enum defining queue attributes.
 */
typedef enum _sai_copp_queue_attr_t
{

    /* READ-WRITE */

    /** Attach scheduler to Queue [sai_object_id_t]*/
    SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID = 0x00000001,

    /** Attach shape to CPU port [uint64_t]*/
    SAI_COPP_QUEUE_ATTR_TOTAL_RATE = 0x00000002,

    /* -- */
    /* Custom range base value */
    SAI_COPP_QUEUE_QUEUE_ATTR_CUSTOM_RANGE_BASE = 0x10000000,
} sai_copp_queue_attr_t;



/**
 * @brief Enum defining statistics for Queue.
 */

typedef enum _sai_copp_queue_stat_counter_t
{
    /** get/set tx packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_PACKETS = 0x00000000,

    /** get/set tx bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_BYTES = 0x00000001,

    /** get/set dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_DROPPED_PACKETS = 0x00000002,

    /** get/set dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_DROPPED_BYTES = 0x00000003,

    /** get/set green color tx packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_PACKETS = 0x00000004,

    /** get/set green color tx bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_BYTES = 0x00000005,

    /** get/set green color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_DROPPED_PACKETS = 0x00000006,

    /**  get/set green color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_DROPPED_BYTES = 0x00000007,

    /** get/set yellow color tx packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_PACKETS = 0x00000008,

    /** get/set yellow color tx bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_BYTES = 0x00000009,

    /** get/set yellow color drooped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_DROPPED_PACKETS = 0x0000000a,

    /** get/set yellow color dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_DROPPED_BYTES = 0x0000000b,

    /** get/set red color tx packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_PACKETS = 0x0000000c,

    /** get/set red color tx bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_BYTES = 0x0000000d,

    /** get/set red color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_DROPPED_PACKETS = 0x0000000e,

    /** get/set red color drooped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_DROPPED_BYTES = 0x0000000f,

    /** get/set WRED green color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS = 0x00000010,

    /** get/set WRED green color dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES = 0x00000011,

    /** get/set WRED yellow color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS = 0x00000012,

    /** get/set WRED yellow color dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES = 0x00000013,

    /** get/set WRED red color dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS = 0x00000014,

    /** get/set WRED red color dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES = 0x00000015,

    /** get/set WRED dropped packets count [uint64_t] */
    SAI_COPP_QUEUE_STAT_DISCARD_DROPPED_PACKETS = 0x00000016,

    /** get/set WRED red dropped bytes count [uint64_t] */
    SAI_COPP_QUEUE_STAT_DISCARD_DROPPED_BYTES = 0x00000017,

    /* -- */
    /* Custom range base value */
    SAI_COPP_QUEUE_STAT_CUSTOM_RANGE_BASE = 0x10000000

} sai_copp_queue_stat_counter_t;

/**
 *  @brief Attribute Id for sai_copp_entry
 */
typedef enum _sai_copp_entry_attr_t
{
    /** READ-ONLY */

    /** SAI copp entry port object id [sai_object_id_t]
     * (MANDATORY_ON_CREATE) */
    SAI_COPP_ENTRY_ATTR_PORT_ID,

    /** READ-WRITE */
    /** TYPE ssh/telnet [sai_uint8_t] */
    SAI_COPP_ENTRY_ATTR_FIELD_TYPE,

    /** L4 Src Port [sai_uint16_t] */
    SAI_COPP_ENTRY_ATTR_FIELD_L4_SRC_PORT,

    /** L4 Dst Port [sai_uint16_t] */
    SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT,

    /** IPv4/v6 [sai_uint8_t] */
    SAI_COPP_ENTRY_ATTR_FIELD_IP_TYPE,

    /** IP Protocol [sai_uint8_t] */
    SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL,

} sai_copp_entry_attr_t;

/**
 * Routine Description:
 *   @brief Create an COPP entry
 *
 * Arguments:
 *   @param[out] copp_entry_id - the acl entry id
 *   @param[in] attr_count - number of attributes
 *   @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
typedef sai_status_t (*sai_create_copp_entry_fn)(
    _Out_ sai_object_id_t *copp_entry_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *   @brief Delete an COPP entry
 *
 * Arguments:
 *  @param[in] copp_entry_id - the copp entry id
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
typedef sai_status_t (*sai_delete_copp_entry_fn)(
    _In_ sai_object_id_t copp_entry_id
    );

/**
 * @brief Set attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
typedef sai_status_t (*sai_set_copp_queue_attribute_fn)(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
    );

/**
 * @brief Get attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
typedef sai_status_t (*sai_get_copp_queue_attribute_fn)(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t        attr_count,
    _Inout_ sai_attribute_t *attr_list
    );


/**
 * @brief   Get queue statistics counters.
 *
 * @param[in] queue_id Queue id
 * @param[in] counter_ids specifies the array of counter ids
 * @param[in] number_of_counters number of counters in the array
 * @param[out] counters array of resulting counter values.
 *
 * @return SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
typedef sai_status_t (*sai_get_copp_queue_stats_fn)(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_copp_queue_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    );


/* SYSTEM MODIFIED BEGIN: add clear qos stats interface by wangqj at 2016/8/8 */
/**
 * @brief   clear queue statistics counters.
 *
 * @param[in] queue_id Queue id
 *
 * @return SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
typedef sai_status_t (*sai_clear_copp_queue_stats_fn)(
    _In_ sai_object_id_t queue_id
    );
/* END: add clear qos stats interface by wangqj at 2016/8/8 */

/**
 *  @brief Qos methods table retrieved with sai_api_query()
 */
typedef struct _sai_copp_api_t
{
    sai_set_copp_queue_attribute_fn   set_copp_queue_attribute;
    sai_get_copp_queue_attribute_fn   get_copp_queue_attribute;
    sai_get_copp_queue_stats_fn       get_copp_queue_stats;
    sai_clear_copp_queue_stats_fn     clear_copp_queue_stats;
    sai_create_copp_entry_fn          create_copp_entry;
    sai_delete_copp_entry_fn          delete_copp_entry;
} sai_copp_api_t;

/**
 *\}
 */

/**
 * \}
 */
#endif // __SAISTP_H_

