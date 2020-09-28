/*****************************************************************************
  *
  * Copyright © 2016 Broadcom.  The term "Broadcom" refers
  * to Broadcom Limited and/or its subsidiaries.
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  *
  * You may obtain a copy of the License at
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ***************************************************************************/

#ifndef INCLUDE_BST_APP_H
#define INCLUDE_BST_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include "modulemgr.h"
#include "insight.h"

#include "bst_json.h"

typedef enum _bst_report_type_ {
  INSIGHT_BST_STATS = 1,
  INSIGHT_BST_THRESHOLD,
  INSIGHT_BST_PERIODIC,
  INSIGHT_BST_STATS_TRIGGER
}INSIGHT_BST_REPORT_TYPE_t;



/* BST command enums */
typedef enum _bst_cmd_
{
  /* Set group */
  INSIGHT_BST_CMD_API_SET_TRACK = 1,
  INSIGHT_BST_CMD_API_SET_FEATURE = 2,
  INSIGHT_BST_CMD_API_SET_THRESHOLD = 3,
  INSIGHT_BST_CMD_API_CLEAR_THRESHOLD = 4,
  INSIGHT_BST_CMD_API_CLEAR_STATS = 5,
  /* get group */
  INSIGHT_BST_CMD_API_GET_REPORT = 6,
  INSIGHT_BST_CMD_API_GET_FEATURE = 7,
  INSIGHT_BST_CMD_API_GET_TRACK = 8,
  INSIGHT_BST_CMD_API_GET_THRESHOLD = 9,
  INSIGHT_BST_CMD_API_TRIGGER_REPORT = 10,
  INSIGHT_BST_CMD_API_CLEAR_TRIGGER_COUNT = 11,
  INSIGHT_BST_CMD_API_ENABLE_BST_ON_TRIGGER = 12,
  INSIGHT_BST_CMD_API_TRIGGER_COLLECT = 13,
  INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS =  14,
  INSIGHT_BST_CMD_API_MAX
}
INSIGHT_FEATURE_BST_CMD_API_t;

#define INSIGHT_BST_MAX_QUEUE_SEND_FAILS  512

typedef struct _bst_request_msg_
{
    uint32 msg_type; /* message type */
    int unit; /* variable to hold the asic type */
    void *cookie;
    int id; /* id passed from the request */
    int version; /* json version */

    char realm[INSIGHT_JSON_MAX_NODE_LENGTH];
    uint32 threshold_type;
    uint32 report_type;
    //  INSIGHT_BST_THRESHOLD_CONFIG_t threshold;
    /* trigger info */
    INSIGHT_BST_TRIGGER_INFO_t triggerInfo;
    union
    {
        /* feature params */
        BSTJSON_CONFIGURE_BST_FEATURE_t feature;
        /* params configured to track */
        BSTJSON_CONFIGURE_BST_TRACKING_t  track;

        /* params requsted to collect in report */
        BSTJSON_GET_BST_REPORT_t   collect;

        BSTJSON_GET_BST_CGSN_DROP_CTRS_t drp_ctrs;

        BSTJSON_CONFIGURE_BST_THRESHOLDS_t  threshold;

    }
    request;

    unsigned int threshold_config_mask;

}
INSIGHT_BST_REQUEST_MSG_t;


/** type define time_t to BVIEW_TIME_t */
typedef time_t INSIGHT_TIME_t;

typedef struct _bst_report_snapshot_data_
{
    INSIGHT_TIME_t tv;
    INSIGHT_BST_ASIC_SNAPSHOT_DATA_t snapshot_data;
}
INSIGHT_BST_REPORT_SNAPSHOT_t;

typedef struct _bst_report_respose_
{
    INSIGHT_BST_REPORT_SNAPSHOT_t *active;
    INSIGHT_BST_REPORT_SNAPSHOT_t *backup;
}
INSIGHT_BST_REPORT_RESP_t;


typedef struct _bst_response_msg_
{
    uint32 msg_type;
    int unit;
    void *cookie;
    int id;
    INSIGHT_BST_REPORT_TYPE_t report_type;
    INSIGHT_ASIC_CAPABILITIES_t*  asic_capabilities;
    BSTJSON_REPORT_OPTIONS_t options;

    INSIGHT_STATUS rv; /* return value for set request */
    union
    {
        BSTJSON_CONFIGURE_BST_FEATURE_t  feature;
        BSTJSON_CONFIGURE_BST_TRACKING_t  *track;
        INSIGHT_BST_REPORT_RESP_t   report;
        BSTJSON_CONFIGURE_BST_FEATURE_t *config;

    }
    response;
}
INSIGHT_BST_RESPONSE_MSG_t;


/* Macro to acquire lock */
#define BST_LOCK_TAKE(_unit)                                                        \
        {                                                                           \
           if (0 != pthread_mutex_lock (&p_bst_info->bst_mutex))                          \
           {                                                                        \
              LOG_POST (INSIGHT_LOG_ERROR,                                            \
                  "Failed to take the lock for unit %d.\r\n",_unit);                \
              return INSIGHT_STATUS_FAILURE;                                          \
           }                                                                        \
         }
/*  to release lock*/
#define BST_LOCK_GIVE(_unit)                                                        \
         {                                                                          \
           if (0 != pthread_mutex_unlock(&p_bst_info->bst_mutex))                         \
           {                                                                        \
              LOG_POST (INSIGHT_LOG_ERROR,                                            \
              "Failed to Release the lock for unit %d.\r\n",_unit);                 \
               return INSIGHT_STATUS_FAILURE;                                         \
            }                                                                       \
          }



/* Macro to acquire read lock */
#define BST_RWLOCK_RD_LOCK(_unit)                             \
{                                                                          \
  if (pthread_rwlock_rdlock(&p_bst_info->bst_configRWLock) != 0)                    \
  {                                                         \
    LOG_POST (INSIGHT_LOG_ERROR,                                            \
        "Failed to take the rw lock for unit %d.\r\n",_unit);                 \
    return INSIGHT_STATUS_FAILURE;                                         \
  }                                                                       \
}


/* Macro to acquire write lock */
#define BST_RWLOCK_WR_LOCK(_unit)                             \
{                                                                          \
  if (pthread_rwlock_wrlock(&p_bst_info->bst_configRWLock) != 0)                    \
  {                                                         \
    LOG_POST (INSIGHT_LOG_ERROR,                                            \
        "Failed to take the write lock for unit %d.\r\n",_unit);                 \
    return INSIGHT_STATUS_FAILURE;                          \
  }                                                                       \
}

/* Macro to release RW lock */
#define BST_RWLOCK_UNLOCK(_unit)                              \
{                                                                          \
  if (pthread_rwlock_unlock(&p_bst_info->bst_configRWLock) != 0)                    \
  {                                                         \
    LOG_POST (INSIGHT_LOG_ERROR,                                            \
        "Failed to release the write lock for unit %d.\r\n",_unit);                 \
    return INSIGHT_STATUS_FAILURE;                          \
  }                                                                       \
}




extern INSIGHT_STATUS
insight_bst_send_request (INSIGHT_BST_REQUEST_MSG_t * msg_data);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BST_APP_H */

