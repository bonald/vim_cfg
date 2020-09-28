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

#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>

#include "insight.h"
#include "bst.h"
#include "system.h"

#include "rest_api.h"
#include "log_api.h"

#define INSIGHT_MAX_REQUESTS  8
#define INSIGHT_BST_TIME_CONVERSION_FACTOR 1000

  typedef struct _bst_timer_context_s_ {
    uint8 unit;
    uint8 index;
    long cmd;
  }INSIGHT_BST_TIMER_CONTEXT_t;

  typedef struct _bst_timer_s_ {
    bool in_use;
    INSIGHT_BST_TIMER_CONTEXT_t context;
    timer_t bstTimer;
  }INSIGHT_BST_TIMER_t;


  typedef struct _bst_drop_ctrs_elem_s_ {
    bool in_use;
    unsigned int id;
    long type;
    BSTJSON_GET_BST_CGSN_DROP_CTRS_t req;
    INSIGHT_BST_TIMER_t cgsn_drop;
  }INSIGHT_BST_CGSN_DROP_ELEM_t;


typedef struct _bst_context_info__
{

  /* stats records */
  INSIGHT_BST_REPORT_SNAPSHOT_t *stats_active_record_ptr;
  INSIGHT_BST_REPORT_SNAPSHOT_t *stats_backup_record_ptr;
  INSIGHT_BST_REPORT_SNAPSHOT_t *stats_current_record_ptr;
  /* threshold records */
  INSIGHT_BST_REPORT_SNAPSHOT_t *threshold_record_ptr;

   INSIGHT_BST_TIMER_t   collection_timer;
   INSIGHT_BST_TIMER_t   trigger_timer;
   BSTJSON_CONFIGURE_BST_FEATURE_t  config;
   BSTJSON_CONFIGURE_BST_TRACKING_t track;

   /* asic capabilities */
   INSIGHT_ASIC_CAPABILITIES_t asic_capabilities;

   INSIGHT_BST_CGSN_DROP_ELEM_t drop_ctrs_db[INSIGHT_MAX_REQUESTS];

   /* place holder to store the bst max buffer settings */
   INSIGHT_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t bst_max_buffers;

  //INSIGHT_BST_UNIT_CXT_t unit[INSIGHT_BST_MAX_UNITS];
  /* BST Key to Queue Message*/
  key_t key1;
  key_t trigger_key;
  /* message queue id for bst */
  int recvMsgQid;
  int recvTriggerMsgQid;
    /* pthread ID*/
  pthread_t bst_thread;
  pthread_t bst_trigger_thread;


/* BST Mutex*/
pthread_mutex_t bst_mutex;
pthread_rwlock_t bst_configRWLock;
} INSIGHT_BST_CXT_t;

/* BST Context Info*/
 INSIGHT_BST_CXT_t *p_bst_info;



/* Macro to copy collect structures to response options structure */

#define  BST_COPY_COLLECT_TO_RESP(_collect_ptr, _resp_ptr)                                 \
            {                                                                              \
              if ((NULL == (_resp_ptr)) || (NULL == (_collect_ptr)))                     \
              {                                                                            \
                 return INSIGHT_STATUS_FAILURE;                                              \
              }                                                                            \
              (_resp_ptr)->includeDevice = (_collect_ptr)->includeDevice;                \
              (_resp_ptr)->includeIngressPortPriorityGroup =                          \
              (_collect_ptr)->includeIngressPortPriorityGroup;                                \
              (_resp_ptr)->includeIngressPortServicePool =                            \
              (_collect_ptr)->includeIngressPortServicePool;                                  \
              (_resp_ptr)->includeIngressServicePool =                                \
              (_collect_ptr)->includeIngressServicePool;                                      \
              (_resp_ptr)->includeEgressPortServicePool =                             \
              (_collect_ptr)->includeEgressPortServicePool;                                   \
              (_resp_ptr)->includeEgressServicePool =                                 \
              (_collect_ptr)->includeEgressServicePool;                                       \
              (_resp_ptr)->includeEgressUcQueue =                                     \
              (_collect_ptr)->includeEgressUcQueue;                                           \
              (_resp_ptr)->includeEgressUcQueueGroup =                                \
              (_collect_ptr)->includeEgressUcQueueGroup;                                      \
              (_resp_ptr)->includeEgressMcQueue =                                     \
              (_collect_ptr)->includeEgressMcQueue;                                           \
              (_resp_ptr)->includeEgressCpuQueue =                                    \
              (_collect_ptr)->includeEgressCpuQueue;                                          \
              (_resp_ptr)->includeEgressRqeQueue =                                    \
              (_collect_ptr)->includeEgressRqeQueue;                                          \
           }


/* Macro to copy track structures to collction structure */
#define BST_COPY_TRACK_TO_COLLECT(_track_ptr, _collect_ptr)                          \
            {                                                                        \
              if ((NULL == (_track_ptr)) || (NULL == (_collect_ptr)))                      \
              {                                                                      \
                return INSIGHT_STATUS_FAILURE;                                         \
              }                                                                      \
              (_collect_ptr)->includeDevice = (_track_ptr)->trackDevice;                   \
              (_collect_ptr)->includeIngressPortPriorityGroup =                         \
              (_track_ptr)->trackIngressPortPriorityGroup;                              \
              (_collect_ptr)->includeIngressPortServicePool =                           \
              (_track_ptr)->trackIngressPortServicePool;                                \
              (_collect_ptr)->includeIngressServicePool =                               \
              (_track_ptr)->trackIngressServicePool;                                    \
              (_collect_ptr)->includeEgressPortServicePool =                            \
              (_track_ptr)->trackEgressPortServicePool;                                 \
              (_collect_ptr)->includeEgressServicePool =                                \
              (_track_ptr)->trackEgressServicePool;                                     \
              (_collect_ptr)->includeEgressUcQueue =                                    \
              (_track_ptr)->trackEgressUcQueue;                                         \
              (_collect_ptr)->includeEgressUcQueueGroup =                               \
              (_track_ptr)->trackEgressUcQueueGroup;                                    \
              (_collect_ptr)->includeEgressMcQueue =                                    \
              (_track_ptr)->trackEgressMcQueue;                                         \
              (_collect_ptr)->includeEgressCpuQueue =                                   \
              (_track_ptr)->trackEgressCpuQueue;                                        \
              (_collect_ptr)->includeEgressRqeQueue =                                   \
              (_track_ptr)->trackEgressRqeQueue;                                        \
            }



/* Macro to copy response options structure with given value*/

#define  BST_COPY_TO_RESP(_resp_ptr, _val_)                                 \
            {                                                                              \
              if (NULL == (_resp_ptr))                                                  \
              {                                                                            \
                 LOG_POST (INSIGHT_LOG_ERROR, "Failed to copy c-json encoding options to response\r\n");\
                 return INSIGHT_STATUS_FAILURE;                                              \
              }                                                                            \
              (_resp_ptr)->includeDevice = _val_;                \
              (_resp_ptr)->includeIngressPortPriorityGroup = _val_;                         \
              (_resp_ptr)->includeIngressPortServicePool =  _val_;                          \
              (_resp_ptr)->includeIngressServicePool =  _val_;                              \
              (_resp_ptr)->includeEgressPortServicePool =    _val_;                         \
              (_resp_ptr)->includeEgressServicePool =       _val_;                          \
              (_resp_ptr)->includeEgressUcQueue =   _val_;                                  \
              (_resp_ptr)->includeEgressUcQueueGroup =   _val_;                             \
              (_resp_ptr)->includeEgressMcQueue =   _val_;                                  \
              (_resp_ptr)->includeEgressCpuQueue =  _val_;                                  \
              (_resp_ptr)->includeEgressRqeQueue =  _val_;                                  \
           }



/*********************************************************************
*  @brief:  function to set the given realm in the include trigger report.
*
* @param[in]   *realm : pointer to realm
* @param[in]   *realm : pointer to json encode options
*
* @retval  : none :
*
* @note :
*
*********************************************************************/

void bst_set_realm_to_collect(char *realm, BSTJSON_REPORT_OPTIONS_t *options)
{
    if (0 == sal_strcmp("device", realm))
    {
        options->includeDevice = true;
        return;
    }

    if (0 == sal_strcmp("ingress-service-pool", realm))
    {
        options->includeIngressServicePool = true;
        return;
    }

    if (0 == sal_strcmp("ingress-port-service-pool", realm))
    {
        options->includeIngressPortServicePool = true;
        return;
    }

    if (0 == sal_strcmp("ingress-port-priority-group", realm))
    {
        options->includeIngressPortPriorityGroup = true;
        return;
    }

    if (0 == sal_strcmp("egress-port-service-pool", realm))
    {
        options->includeEgressPortServicePool = true;
        return;
    }

    if (0 == sal_strcmp("egress-service-pool", realm))
    {
        options->includeEgressServicePool = true;
        return;
    }

    if (0 == sal_strcmp("egress-uc-queue", realm))
    {
        options->includeEgressUcQueue = true;
        return;
    }

    if (0 == sal_strcmp("egress-uc-queue-group", realm))
    {
        options->includeEgressUcQueueGroup = true;
        return;
    }

    if (0 == sal_strcmp("egress-mc-queue", realm))
    {
        options->includeEgressMcQueue = true;
        return;
    }

    if (0 == sal_strcmp("egress-cpu-queue", realm))
    {
        options->includeEgressCpuQueue = true;
        return;
    }

    if (0 == sal_strcmp("egress-rqe-queue", realm))
    {
        options->includeEgressRqeQueue = true;
        return;
    }

    return;
}


#define ______CALL_SDK_API________

INSIGHT_STATUS  sbplugin_ovsdb_system_time_get (INSIGHT_TIME_t *ptime)
{

  time (ptime);
  return INSIGHT_STATUS_SUCCESS;
}


int32 sbapi_bst_snapshot_async_get_cb(ctc_monitor_data_t *info, void* userdata)
{

    ctc_monitor_msg_t*  p_msg = NULL;
    ctc_monitor_buffer_stats_t *p_stats = NULL;
    INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *p_snapshot = NULL;
    uint16 i = 0;
    uint32 port = 0;

    BST_LOCK_TAKE (0);

    p_snapshot = &p_bst_info->stats_current_record_ptr->snapshot_data;

    for (i = 0; i < info->msg_num; i++)
    {
        p_msg = &info->p_msg[i];

        if (p_msg->monitor_type != CTC_MONITOR_BUFFER || p_msg->msg_type != CTC_MONITOR_MSG_STATS)
        {
            continue;
        }

        p_stats = &p_msg->u.buffer_stats;

        if (p_stats->buffer_type == CTC_MONITOR_BUFFER_TOTAL)
        {
            if ( p_stats->dir == CTC_EGRESS)
            {
                p_snapshot->device.bufferCount = p_stats->buffer_cnt;
            }
        }
        else if(p_stats->buffer_type == CTC_MONITOR_BUFFER_SC)
        {
            if ( p_stats->dir == CTC_INGRESS)
            {
                p_snapshot->iSp.data[p_stats->sc].umShareBufferCount = p_stats->buffer_cnt;
            }
            else
            {
               p_snapshot->eSp.data[p_stats->sc].umShareBufferCount = p_stats->buffer_cnt;
            }

        }
        else if(p_stats->buffer_type == CTC_MONITOR_BUFFER_PORT)
        {
            port =  CTC_MAP_GPORT_TO_LPORT(p_stats->gport);

            if ( p_stats->dir == CTC_INGRESS)
            {
                p_snapshot->iPortSp.data[port][0].umShareBufferCount = p_stats->buffer_cnt;
            }
            else
            {
                p_snapshot->ePortSp.data[port][0].umShareBufferCount = p_stats->buffer_cnt;
            }

        }

    }


    BST_LOCK_GIVE (0);

    return 0;
}



INSIGHT_STATUS
sbapi_bst_config_set (int asic, BVIEW_BST_CONFIG_t* p_bst_config)
{
  uint16 i = 0;
  uint8 lchip = 0;
  ctc_monitor_config_t config;

  /*By DRV IO read*/
  return 0;

  sal_memset(&config, 0, sizeof(config));

  if (p_bst_config->enablePeriodicCollection)
  {
      config.monitor_type = CTC_MONITOR_BUFFER;
      config.cfg_type = CTC_MONITOR_CONFIG_MON_SCAN_EN;
      config.value = 1;

      /*Totoal*/
      config.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
      config.dir = CTC_EGRESS;
      ctcs_monitor_set_config(lchip, &config);

      /*SC*/
      config.buffer_type = CTC_MONITOR_BUFFER_SC;
      for (i = 0; i < 4; i++)
      {
          config.sc = i;
          config.dir = CTC_INGRESS;
          ctcs_monitor_set_config(lchip, &config);
          config.dir = CTC_EGRESS;
          ctcs_monitor_set_config(lchip, &config);
      }

      /*PORT*/
      config.buffer_type = CTC_MONITOR_BUFFER_PORT;
      for (i = 0; i < 64; i++)
      {
          config.gport = i;
          config.dir = CTC_INGRESS;
          ctcs_monitor_set_config(lchip, &config);
          config.dir = CTC_EGRESS;
          ctcs_monitor_set_config(lchip, &config);
      }


      config.monitor_type = CTC_MONITOR_BUFFER;
      config.cfg_type = CTC_MONITOR_CONFIG_MON_INTERVAL;
      // config.value = p_bst_config->collectionPeriod*1000;
      config.value = 300;
      ctcs_monitor_set_config(lchip, &config);
      ctcs_monitor_register_cb(lchip, (ctc_monitor_fn_t)sbapi_bst_snapshot_async_get_cb, NULL);


  }

  return 0;
}


INSIGHT_STATUS
sbapi_bst_threshold_get (int asic,
                         INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *data,
                         INSIGHT_TIME_t * time)
{
    unsigned int           index = 0;
    INSIGHT_STATUS           rv = INSIGHT_STATUS_SUCCESS;


    /* Update current local time*/
    sbplugin_ovsdb_system_time_get (time);


    data->device.bufferCount =  10000;

    /* Get threshold configuration for The BST_Threshold for the Egress UC Queues in units of 8 buffers.*/
    for ((index) = 0; (index) < INSIGHT_ASIC_MAX_UC_QUEUES;  (index)++)
    {
        rv = rv/*API*/;

        data->eUcQ.data[index].ucBufferCount = 4096;
        if (INSIGHT_STATUS_SUCCESS != rv)
        {
            return INSIGHT_STATUS_FAILURE;
        }
        data->eUcQ.data[index].port = 0;
    }


    return INSIGHT_STATUS_SUCCESS;
}


INSIGHT_STATUS
sbapi_bst_snapshot_get (int asic,
                        INSIGHT_BST_ASIC_SNAPSHOT_DATA_t * data,
                        INSIGHT_TIME_t * time)
{
    uint8 lchip = 0;
    unsigned int           index = 0;
    ctc_qos_resrc_pool_stats_t stat;

    sal_memset(&stat, 0, sizeof(stat));

    /* Update current local time*/
    sbplugin_ovsdb_system_time_get (time);

    /* Get BST cnt of Device*/
    stat.type  = CTC_QOS_RESRC_STATS_EGS_TOTAL_COUNT;
    ctcs_qos_query_pool_stats(lchip, &stat);
    data->device.bufferCount = stat.count;

    /*  Get BST cnt of the Ingress sc*/
    for ((index) = 0; (index) < INSIGHT_ASIC_MAX_SERVICE_POOLS;  (index)++)
    {
        stat.type  = CTC_QOS_RESRC_STATS_IGS_POOL_COUNT;
        stat.pool = index;
        ctcs_qos_query_pool_stats(lchip, &stat);
        data->iSp.data[index].umShareBufferCount = stat.count;
    }


    /*  Get BST cnt of the Egress sc*/
    for ((index) = 0; (index) < INSIGHT_ASIC_MAX_SERVICE_POOLS;  (index)++)
    {
        stat.type  = CTC_QOS_RESRC_STATS_EGS_POOL_COUNT;
        stat.pool = index;
        ctcs_qos_query_pool_stats(lchip, &stat);
        data->iSp.data[index].umShareBufferCount = stat.count;
    }

#if 0
    /*  Get BST cnt of the Ingress port*/
    for ((index) = 0; (index) < 64;  (index)++)
    {
        stat.type  = CTC_QOS_RESRC_STATS_IGS_PORT_COUNT;
        stat.gport = index;
        ctcs_qos_query_pool_stats(lchip, &stat);
        data->iPortPg.data[index][0].umShareBufferCount = stat.count;
        data->iPortSp.data[index][0].umShareBufferCount = stat.count;
    }


    /*  Get BST cnt of the Egress port*/
    for ((index) = 0; (index) < 64;  (index)++)
    {
        stat.type  = CTC_QOS_RESRC_STATS_EGS_PORT_COUNT;
        stat.pool = index;
        ctcs_qos_query_pool_stats(lchip, &stat);
        data->ePortSp.data[index][0].umShareBufferCount = stat.count;
    }
#endif

    /*  Get BST cnt of the Egress UC Queues*/
    for ((index) = 0; (index) < INSIGHT_ASIC_MAX_UC_QUEUES;  (index)++)
    {
        stat.type  = CTC_QOS_RESRC_STATS_QUEUE_COUNT;
        stat.queue.gport = index / 8;
        stat.queue.queue_id = index % 8;
        stat.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        ctcs_qos_query_pool_stats(lchip, &stat);
        data->eUcQ.data[index].ucBufferCount = stat.count;
        data->eUcQ.data[index].port = index / 8;
    }

    return INSIGHT_STATUS_SUCCESS;
}


INSIGHT_STATUS sbapi_system_max_buf_snapshot_get (int asic,
                                                  INSIGHT_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t *data,
                                                  INSIGHT_TIME_t * time)
{

  unsigned int           index =0;
  INSIGHT_STATUS           rv = INSIGHT_STATUS_SUCCESS;


   /* Update current local time*/
  sbplugin_ovsdb_system_time_get (time);

   data->device.data.maxBuf = 10000;

  /* Get threshold configuration for The BST_Threshold for the Egress UC Queues in units of 8 buffers.*/
  for ((index) = 0; (index) < INSIGHT_ASIC_MAX_UC_QUEUES;  (index)++)
  {
       rv = rv/*API*/;

      data->eUcQ.data[index].ucMaxBuf = 8192;;
      if (INSIGHT_STATUS_SUCCESS != rv)
      {
          return INSIGHT_STATUS_FAILURE;
      }
  }

    return INSIGHT_STATUS_SUCCESS;
}



#define ______BST_MSG________


/*********************************************************************
* @brief : copies the h/w retrevied to the current record.
*
* @param[in] type : Type of the record , i.e. stats or threshold
* @param[in] unit : unit number for which the data is collected.
*
* @retval  : INSIGHT_STATUS_SUCCESS : successfully updates the records
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
*
* @note : Once the collection of the stats is completeed, the existing active
*        record becomes back up. The newly collected record become the new active
*        record. Make sure to take the lock while updating..
*
*********************************************************************/
INSIGHT_STATUS bst_update_data (INSIGHT_BST_REPORT_TYPE_t type, unsigned int unit)
{
    INSIGHT_BST_REPORT_SNAPSHOT_t *temp;

    switch (type)
    {
        case INSIGHT_BST_STATS:
              BST_LOCK_TAKE (unit);
            /* take the lock */
            /* copy the backup pointer in a temporary variable */
            temp = p_bst_info->stats_backup_record_ptr;
            /* make the active  as backup */
            p_bst_info->stats_backup_record_ptr = p_bst_info->stats_active_record_ptr;

            /* make the current as active */
            p_bst_info->stats_active_record_ptr = p_bst_info->stats_current_record_ptr;
            /* now make the old backup as current */
            p_bst_info->stats_current_record_ptr = temp;
            /* release the lock */
             BST_LOCK_GIVE (unit);
            break;

        default:
            break;
    }

    /* log an information that the records are updated */
    LOG_POST (INSIGHT_LOG_INFO,  "stat records are updated \r\n");

    return INSIGHT_STATUS_SUCCESS;
}


/*********************************************************************
* @brief : function to send reponse for encoding to cjson and sending
*          using rest API
*
* @param[in] reply_data : pointer to the response message
*
* @retval  : INSIGHT_STATUS_SUCCESS : message is successfully using rest API.
* @retval  : INSIGHT_STATUS_FAILURE : message is unable to deliver using rest API.
* @retval  : INSIGHT_STATUS_OUTOFMEMORY : Unable to allocate json buffer.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter to function.
*
* @note   : This function is invoked by the bst to send the report and
*           requested get configure params. This function internally
*           calls the encoding api to encode the data, and the memory
*           for the data is allocated. In case of both successful and
*           unsuccessful send of the data, the memory must be freed.
*
*********************************************************************/
INSIGHT_STATUS insight_bst_send_response (INSIGHT_BST_RESPONSE_MSG_t * reply_data)
{
  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
  uint8 *pJsonBuffer = NULL;

  if (NULL == reply_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;

LOG_POST (INSIGHT_LOG_INFO, "Reply_data: reply_data->msg_type = %u reply_data->rv = %d\n" "reply_data->unit = %d\n" "reply_data->cookie = %d\n",
            reply_data->msg_type,reply_data->rv, reply_data->unit, (NULL == reply_data->cookie)? true : false);
  if (reply_data->rv != INSIGHT_STATUS_SUCCESS)
  {
    rest_response_send_error(reply_data->cookie, reply_data->rv, reply_data->id);
    return INSIGHT_STATUS_SUCCESS;
  }
  else
  {
    if ((INSIGHT_BST_CMD_API_SET_TRACK == reply_data->msg_type) ||
        (INSIGHT_BST_CMD_API_SET_FEATURE == reply_data->msg_type) ||
        (INSIGHT_BST_CMD_API_SET_THRESHOLD == reply_data->msg_type) ||
        (INSIGHT_BST_CMD_API_CLEAR_STATS == reply_data->msg_type) ||
        (INSIGHT_BST_CMD_API_CLEAR_THRESHOLD == reply_data->msg_type))
    {
      rest_response_send_ok (reply_data->cookie);
      return INSIGHT_STATUS_SUCCESS;
    }

  }

  /* Take lock*/
  BST_LOCK_TAKE (reply_data->unit);
  switch (reply_data->msg_type)
  {
    case INSIGHT_BST_CMD_API_GET_TRACK:

      /* call json encoder api for tracking  */
      rv = bstjson_encode_get_bst_tracking (reply_data->unit, reply_data->id,
          reply_data->response.track,
          &pJsonBuffer);
      break;

    case INSIGHT_BST_CMD_API_GET_FEATURE:
      /* call json encoder api for feature  */

      rv = bstjson_encode_get_bst_feature (reply_data->unit, reply_data->id,
         &reply_data->response.feature,
          &pJsonBuffer);
      break;


     case INSIGHT_BST_CMD_API_GET_REPORT:
     case INSIGHT_BST_CMD_API_TRIGGER_REPORT:
     case INSIGHT_BST_CMD_API_GET_THRESHOLD:
      /*  call json encoder api for report  */

      /* if this is a periodic report, the back up pointer is
         a non null poiner, other wise , the backup
         pointer would be NULL pointer. so call
         the encoder function accordingly */


      if (NULL == reply_data->response.report.backup)
      {
      rv = bstjson_encode_get_bst_report (reply_data->unit, reply_data->msg_type,
                                          NULL,
                                          &reply_data->response.report.active->snapshot_data,
                                          &reply_data->options,
                                          reply_data->asic_capabilities,
                                          &reply_data->response.report.active->tv,
                                          &pJsonBuffer);
      }
      else
      {
      rv = bstjson_encode_get_bst_report (reply_data->unit, reply_data->msg_type,
                                          &reply_data->response.report.backup->snapshot_data,
                                          &reply_data->response.report.active->snapshot_data,
                                          &reply_data->options,
                                          reply_data->asic_capabilities,
                                          &reply_data->response.report.active->tv,
                                          &pJsonBuffer);
      }

        break;


    case INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS:
        /* call json encoder api for feature  */
#if 0

        rv = bstjson_encode_get_cgsn_drop_ctrs (reply_data->unit,
                                                &reply_data->response.drp_ctr_report,
                                                &reply_data->asic_capabilities,
                                                &pJsonBuffer);
#endif
        break;


    default:
      break;
  }

  if (rv != INSIGHT_STATUS_SUCCESS)
  {
    /* free the allocated memory */
    if (NULL != pJsonBuffer)
    {
      mem_free(pJsonBuffer);
    }
    BST_LOCK_GIVE(reply_data->unit);
    return rv;
  }
  if (NULL != pJsonBuffer)
  {
    #ifdef BST_DEBUG_METRICS
    if (reply_data->msg_type == INSIGHT_BST_CMD_API_TRIGGER_REPORT)
    {
      char buf[INSIGHT_TIME_BUFFER_SIZE];
      system_dispaly_local_time_get (buf);
      printf ("\r\n%s: Sending trigger report for realm (%s) counter (%s) index1 (%d) index2 (%d)\r\n",
             buf,reply_data->options.triggerInfo.realm,
             reply_data->options.triggerInfo.counter,
             reply_data->options.triggerInfo.port,
             reply_data->options.triggerInfo.queue);
    }
    #endif
    rv = rest_response_send(reply_data->cookie, (char *)pJsonBuffer, strlen((char *)pJsonBuffer));

    if (INSIGHT_STATUS_SUCCESS != rv)
    {
      LOG_POST (INSIGHT_LOG_ERROR, "sending response failed due to error = %d\r\n",rv);
      LOG_POST (INSIGHT_LOG_ERROR,
          " sending response failed due to error = %d\r\n",rv);
    }
    /* free the json buffer */
    mem_free(pJsonBuffer);
  }
  else
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "encoding of bst response failed due failure of memory allocation\r\n");
  }
  /* release the lock for success and failed cases */
  BST_LOCK_GIVE(reply_data->unit);
  return rv;
}

/*********************************************************************
* @brief : function to prepare the response to the request message
*
* @param[in] msg_data : pointer to the request message
* @param[out] reply_data : pointer to the response message
*
* @retval  : INSIGHT_STATUS_FAILURE : if the copy to response message fails
* @retval  : INSIGHT_STATUS_SUCCESS :  response message is successfully prepred.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
*                                           or null pointers to function
*
* @note  : This api copies the required information from input request
*          message to response message. In case of report, it copies the
*          pointers into the structure The data need to be protected till
*          the memory is allocated for the report to be copied.
*          If the report is periodic, then we need the references of both
*          active and back up records. If the report is non periodic, i.e
*          trigger or get report, then only the active record is required.
*          incse of periodic report, the report contains
*          incremental/differential changes.. Wher as for get report
*          the report is complete. Ensure that the lock is taken at this function
*          is released when the buffer for the data is allocated.
*
*********************************************************************/
INSIGHT_STATUS bst_copy_reply_params (INSIGHT_BST_REQUEST_MSG_t * msg_data,
                            INSIGHT_BST_RESPONSE_MSG_t * reply_data)
{
#if 0
  INSIGHT_BST_UNIT_CXT_t *ptr;

  INSIGHT_BST_CGSN_DROPS_t *tmp = NULL;


  if ((NULL == msg_data) || (NULL == reply_data))
    return INSIGHT_STATUS_INVALID_PARAMETER;

  ptr = BST_UNIT_PTR_GET (msg_data->unit);
#endif

  BSTJSON_GET_BST_REPORT_t *pCollect = &msg_data->request.collect;
  BSTJSON_REPORT_OPTIONS_t  *pResp;

  pCollect = &msg_data->request.collect;
  pResp = &reply_data->options;

  /* copy the request type */
  reply_data->msg_type = msg_data->msg_type;
  /* copy the unit */
  reply_data->unit = msg_data->unit;
  /* copy the asic type */
  reply_data->id = msg_data->id;
  /* copy the cookie ..  */
  reply_data->cookie = msg_data->cookie;
  /* copy the asic capabilities into the response */
  reply_data->asic_capabilities = &p_bst_info->asic_capabilities;

  reply_data->options.statUnitsInCells =   p_bst_info->config.statUnitsInCells;

  reply_data->options.statsInPercentage = false;

  /* copy the address pointer of the default values */
  reply_data->options.bst_max_buffers_ptr = &p_bst_info->bst_max_buffers;


   /* copy the collect params into options fields of the request */
   BST_COPY_COLLECT_TO_RESP (pCollect, pResp);


  switch (msg_data->msg_type)
  {
    case INSIGHT_BST_CMD_API_GET_REPORT:
    case INSIGHT_BST_CMD_API_TRIGGER_REPORT:
    {
        /* copy stats_units and trigger into  options */
        if (INSIGHT_BST_STATS_TRIGGER == msg_data->report_type)
        {
            /* this report is of type trigger */
            reply_data->options.reportTrigger = true;
            reply_data->options.reportThreshold = false;
            reply_data->cookie = NULL;
            reply_data->options.triggerInfo = msg_data->triggerInfo;
            reply_data->options.sendSnapShotOnTrigger = p_bst_info->config.sendSnapshotOnTrigger;
            if (false == reply_data->options.sendSnapShotOnTrigger)
            {
                BST_COPY_TO_RESP(pResp, false);
                /* Set the only the desired realm to true */
                bst_set_realm_to_collect(msg_data->triggerInfo.realm, pResp);
            }
        }

        /* update the data, i.e make the active record as new backup
        and current record as new active */
        bst_update_data(INSIGHT_BST_STATS, msg_data->unit);

        /* assign the active records */
        reply_data->response.report.active = p_bst_info->stats_active_record_ptr;

        /* copy the backup record ptr if and only if the report is periodic */

        if (INSIGHT_BST_PERIODIC == msg_data->report_type)
        {
            reply_data->options.sendIncrementalReport = p_bst_info->config.sendIncrementalReport;

            /* check if complete report is configured */
            if (false == p_bst_info->config.sendIncrementalReport)
            {
                reply_data->response.report.backup = NULL;
            }
            else
            {
                reply_data->response.report.backup = p_bst_info->stats_backup_record_ptr;
            }
            reply_data->cookie = NULL;
        }
        else
        {
            /* copy null as the encoder function expects the null for non-periodic cases */
            reply_data->response.report.backup = NULL;

            /* no need to send huge data when get-bst-report is requested.
            Send only the counters which contain non zero values */
            reply_data->options.sendIncrementalReport =  true;
        }

        reply_data->options.statsInPercentage = p_bst_info->config.statsInPercentage;
    }

      break;

    case INSIGHT_BST_CMD_API_GET_THRESHOLD:
      {
          reply_data->options.reportThreshold = true;
          reply_data->response.report.active = p_bst_info->threshold_record_ptr;
          /* copy null as the encoder function expects the null  */
          reply_data->response.report.backup = NULL;
          reply_data->options.sendIncrementalReport =  false;
          reply_data->options.statsInPercentage = p_bst_info->config.statsInPercentage;
      }
      break;

    case INSIGHT_BST_CMD_API_GET_FEATURE:
      reply_data->response.config = &p_bst_info->config;
      break;

    case INSIGHT_BST_CMD_API_GET_TRACK:
          reply_data->response.track = &p_bst_info->track;
      break;

    case INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS:
     BST_LOCK_TAKE (msg_data->unit);
  //  tmp = ptr->cgsn_drp_curr;
   // ptr->cgsn_drp_curr = ptr->cgsn_drp_active;
  //  ptr->cgsn_drp_active = tmp;
   // reply_data->response.drp_ctr_report = ptr->cgsn_drp_active;
    if (INSIGHT_BST_PERIODIC == msg_data->report_type)
    {
      reply_data->cookie = NULL;
    }
    BST_LOCK_GIVE (msg_data->unit);
      break;

    default:
      break;
  }
  /* release the lock for success and failed cases */

  return insight_bst_send_response(reply_data);
}

/*********************************************************************
* @brief   :  function to post message to the bst application
*
* @param[in]  msg_data : pointer to the message request
*
* @retval  : INSIGHT_STATUS_SUCCESS : if the message is successfully posted to BST queue.
* @retval  : INSIGHT_STATUS_FAILURE : if the message is failed to send to bst
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
*
* @note  : all the apis and call back functions should use this api
*          to post the message to bst application.
*
*********************************************************************/
INSIGHT_STATUS insight_bst_send_request (INSIGHT_BST_REQUEST_MSG_t * msg_data)
{
  int rv = INSIGHT_STATUS_SUCCESS;
  struct mq_attr obuf; /* output attr struct for getattr */

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;

  if (-1 == msgsnd (p_bst_info->recvMsgQid, msg_data, sizeof (INSIGHT_BST_REQUEST_MSG_t), IPC_NOWAIT))
  {
    if ( ! mq_getattr(p_bst_info->recvMsgQid,&obuf) )
    {
        LOG_POST (INSIGHT_LOG_ERROR,"flags: %ld  maxmsg: %ld  msgsize: %ld  curmsgs: %ld\n",
          obuf.mq_flags, obuf.mq_maxmsg, obuf.mq_msgsize, obuf.mq_curmsgs);
    }

    LOG_POST (INSIGHT_LOG_ERROR,
              "Failed to send message to bst application,  msg_type  %ld, err = %d\r\n",
              msg_data->msg_type, errno);
    rv = INSIGHT_STATUS_FAILURE;
  }

  return rv;
}

/*********************************************************************
*  @brief:  callback function to send periodic reports
*
* @param[in]   sigval : Data passed with notification after timer expires
*
* @retval  : INSIGHT_STATUS_SUCCESS : message is successfully posted to bst.
* @retval  : INSIGHT_STATUS_FAILURE : failed to post message to bst.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
*
* @note : when the periodic collection timer expires, this api is invoked in
*         the timer context. We just post the request for the report, while
*        posting we mark the report type as periodic. The parameter returned
*       from the callback is the reference to unit id.
*
*********************************************************************/
INSIGHT_STATUS bst_periodic_collection_cb (union sigval sigval)
{
  INSIGHT_BST_REQUEST_MSG_t msg_data;
  INSIGHT_STATUS rv;
  INSIGHT_BST_TIMER_CONTEXT_t context;

   memset(&context, 0, sizeof(INSIGHT_BST_TIMER_CONTEXT_t));

  msg_data.report_type = INSIGHT_BST_PERIODIC;
 context = (*(INSIGHT_BST_TIMER_CONTEXT_t *)sigval.sival_ptr);
 msg_data.msg_type = context.cmd;
 msg_data.unit = context.unit;
  msg_data.id = context.index;
  /* Send the message to the bst application */
  rv = insight_bst_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != rv)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "Failed to send periodic collection message to bst application. err = %d\r\n", rv);
       return INSIGHT_STATUS_FAILURE;
  }
    LOG_POST (INSIGHT_LOG_INFO,
        "timer call back received for unit = %d\r\n", *(int *)sigval.sival_ptr);
  return INSIGHT_STATUS_SUCCESS;
}

INSIGHT_STATUS bst_periodic_collection_timer_delete (int unit,
                                                   INSIGHT_FEATURE_BST_CMD_API_t cmd,
                                                   unsigned int id)
{

  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
  INSIGHT_BST_TIMER_t *timer_ptr;


  /* get the appropriate timer context */
  switch (cmd)
  {
    case INSIGHT_BST_CMD_API_GET_REPORT:
    timer_ptr = &p_bst_info->collection_timer;
    break;

    case INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS:
    timer_ptr =  &p_bst_info->drop_ctrs_db[id].cgsn_drop;
    break;

    default:
    return INSIGHT_STATUS_SUCCESS;
  }

  if (true == timer_ptr->in_use)
  {
    rv = system_timer_delete (timer_ptr->bstTimer);
    if (INSIGHT_STATUS_SUCCESS == rv)
    {
      timer_ptr->in_use = false;
      /* Clear the context */
      memset(&timer_ptr->context, 0, sizeof(INSIGHT_BST_TIMER_CONTEXT_t));
        LOG_POST (INSIGHT_LOG_INFO,
              "bst application: successfully deleted timer for unit %d , timer id %d.\r\n", unit, timer_ptr->bstTimer);
    }
    else
    {
      /* timer node add has failed. log the same */
      LOG_POST (INSIGHT_LOG_ERROR,
           "Failed to delete periodic collection time for unit %d, err %d \r\n", unit, rv);
    }
  }

  return rv;
}


INSIGHT_STATUS bst_periodic_collection_timer_add (uint8  unit,
                                                void * handler,
                                                INSIGHT_FEATURE_BST_CMD_API_t cmd,
                                                uint8 id)
{
//  BSTJSON_CONFIGURE_BST_FEATURE_t *ptr;

  INSIGHT_BST_TIMER_t *timer_ptr;
  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
  uint8 interval = 0;

  if (NULL == handler)
    return INSIGHT_STATUS_INVALID_PARAMETER;

  /* get the appropriate timer context */
  switch (cmd)
  {
    case INSIGHT_BST_CMD_API_GET_REPORT:
    timer_ptr = &p_bst_info->collection_timer;
    /* fill the context */
    timer_ptr->context.unit = unit;
    timer_ptr->context.cmd = cmd;
    interval = p_bst_info->config.collectionInterval;
    break;

    case INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS:
    timer_ptr =  &p_bst_info->drop_ctrs_db[id].cgsn_drop;
    /* fill the context */
    timer_ptr->context.index = p_bst_info->drop_ctrs_db[id].id;
    timer_ptr->context.unit = unit;
    timer_ptr->context.cmd = cmd;
    interval = p_bst_info->drop_ctrs_db[id].req.intrvl;
    break;

    default:
    return INSIGHT_STATUS_SUCCESS;
  }

  /* check if the timer node is already in use.
  */
  if (true == timer_ptr->in_use)
  {
    /* the timer is in use. The requester has asked
       to add the timer again.. Remove the old entru
       and add it again.. Reasosn could be that config
       interval would have been changed, In such case,
       delete the one with previous collection timer
       interval and add the new one */
    rv =  bst_periodic_collection_timer_delete(unit,
                                           cmd, id);
    if (INSIGHT_STATUS_SUCCESS != rv)
  {
      /* timer node add has failed. log the same */
      LOG_POST (INSIGHT_LOG_ERROR,
          "%s Failed to delete periodic collection time for unit %d, err %d \r\n", __func__, unit, rv);
    }
  }

  /* The timer add function expects the time in milli seconds..
     so convert the time into milli seconds. , before adding
     the timer node */
    rv =  system_timer_add (handler,
                  &timer_ptr->bstTimer, interval*INSIGHT_BST_TIME_CONVERSION_FACTOR,
                  1, &timer_ptr->context);

    if (INSIGHT_STATUS_SUCCESS == rv)
    {
      timer_ptr->in_use = true;
       LOG_POST (INSIGHT_LOG_INFO,
              "bst application: timer is successfully started for unit %d.\r\n", unit);
    }
    else
    {
      /* timer node add has failed. log the same */
      LOG_POST (INSIGHT_LOG_ERROR,
         "Failed to add periodic collection time for unit %d, err %d \r\n", unit, rv);
    }
  return rv;
}


/*********************************************************************
*  @brief:  function to clean up  bst application
*
* @param[in]  none
*
* @retval  : none
*
* @note  When in case of any error event, this api cleans up all the resources.
*         i.e it deletes the timer notes, if any are present.
*         destroy the mutexes. clears the memory and destroys the message queue.
*
*********************************************************************/
void bst_app_uninit ()
{

  p_bst_info->key1 = INSIGHT_MSG_QUEUE_ID_TO_BST;
  p_bst_info->trigger_key =  INSIGHT_MSG_QUEUE_ID_TO_BST_TRIGGER;


  /* if periodic collection is enabled
  delete the timer.
  loop through all the units and close
  */
  //  bst_periodic_collection_timer_delete (id, INSIGHT_BST_CMD_API_GET_REPORT, 0);
  /* Destroy mutex */

  pthread_mutex_destroy (&p_bst_info->bst_mutex);
  pthread_rwlock_destroy (&p_bst_info->bst_configRWLock);



  /* check if the message queue already exists.
     If yes, we should delete the same */
  if (-1 == (p_bst_info->recvMsgQid = msgget(p_bst_info->key1, IPC_EXCL | IPC_CREAT | 0777)))
  {
    if (EEXIST == errno)
    {
      /* delete the message queue  */
      if (msgctl(p_bst_info->recvMsgQid, IPC_RMID, NULL) == -1)
      {
        LOG_POST (INSIGHT_LOG_ERROR,
            "Failed to destroy bst msgQ id, err = %d\n", errno);
      }
    }
  }
}


static INSIGHT_REST_API_t bst_cmd_api_list[] = {

  {"configure-bst-tracking", insight_bst_json_configure_bst_tracking},
  {"configure-bst-feature", insight_bst_json_configure_bst_feature},
  {"configure-bst-thresholds", insight_bst_json_configure_bst_thresholds},
  {"get-bst-report", insight_bst_json_get_bst_report},
  {"get-bst-feature", insight_bst_json_get_bst_feature},
  {"get-bst-tracking", insight_bst_json_get_bst_tracking},
  {"get-bst-thresholds", insight_bst_json_get_bst_thresholds},
  {"clear-bst-thresholds", insight_bst_json_clear_bst_thresholds},
  {"clear-bst-statistics", insight_bst_json_clear_bst_statistics},
  {"get-bst-congestion-drop-counters", insight_bst_json_get_bst_congestion_drop_counters}
};

INSIGHT_STATUS insight_bst_module_register ()
{
  insight_module_feature_info_t bstInfo;
  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

  memset (&bstInfo, 0, sizeof (insight_module_feature_info_t));

  bstInfo.featureId = INSIGHT_FEATURE_BST;
  strncpy (&bstInfo.featureName[0], "bst", strlen("bst"));
  memcpy (bstInfo.restApiList, bst_cmd_api_list,  sizeof(bst_cmd_api_list));

  /* Register with module manager. */
  rv = modulemgr_register (&bstInfo);
  if (INSIGHT_STATUS_SUCCESS != rv)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
              "bst application failed to register with module mgr\r\n");
  }
  else
  {
      LOG_POST (INSIGHT_LOG_INFO,
           "module mgr registration for bst successful \r\n");
  }
  return rv;
}

INSIGHT_STATUS bst_config_feature_set (void * msg_data)
{
   INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;
   BVIEW_BST_CONFIG_t bstMode;
   uint32 tmpMask;
   uint8 timerUpdateReqd = 0;
  if (NULL == p_req_msg)
    return INSIGHT_STATUS_INVALID_PARAMETER;

      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: setting bst feature for chip %d.\r\n", p_req_msg->unit);

  BST_RWLOCK_WR_LOCK(p_req_msg->unit);


  tmpMask = p_req_msg->request.feature.configMask;

  if ((tmpMask & (1 << BST_CONFIG_PARAMS_COLL_INTRVL)) &&
      (p_bst_info->config.collectionInterval != p_req_msg->request.feature.collectionInterval))
  {
    /* Collection interval has changed.
       so need to register the modified interval with the timer */
    p_bst_info->config.collectionInterval = p_req_msg->request.feature.collectionInterval;
    timerUpdateReqd = true;
  }

  if ((tmpMask & (1 << BST_CONFIG_PARAMS_SND_SNAP_TGR)) &&
      (p_bst_info->config.sendSnapshotOnTrigger != p_req_msg->request.feature.sendSnapshotOnTrigger))
  {
    p_bst_info->config.sendSnapshotOnTrigger = p_req_msg->request.feature.sendSnapshotOnTrigger;
  }

  if ((tmpMask & (1 << BST_CONFIG_PARAMS_TGR_RATE_LIMIT)) &&
       (p_bst_info->config.bstMaxTriggers != p_req_msg->request.feature.bstMaxTriggers))
  {
    p_bst_info->config.bstMaxTriggers = p_req_msg->request.feature.bstMaxTriggers;
  }

  if ((tmpMask & (1 << BST_CONFIG_PARAMS_TGR_RL_INTVL)) &&
      (p_bst_info->config.triggerTransmitInterval != p_req_msg->request.feature.triggerTransmitInterval))
  {
    p_bst_info->config.triggerTransmitInterval = p_req_msg->request.feature.triggerTransmitInterval;
  }

  /* request is always the negation of the variable. Hence checking
     for equality. If same then change the variable */
  if (tmpMask & (1 << BST_CONFIG_PARAMS_ASYNC_FULL_REP))
  {
    p_bst_info->config.sendIncrementalReport = (p_req_msg->request.feature.sendIncrementalReport == 0)?1:0;
  }

 if (tmpMask & (1 << BST_CONFIG_PARAMS_SND_ASYNC_REP))
 {
   if (true == p_req_msg->request.feature.sendAsyncReports)
   {
     /* request contains sendAsyncReports = true */
     if (true != p_bst_info->config.sendAsyncReports)
     {
       /* old config is not enabled for sending async reports.
          now it is enabled.. so need to register with timer*/
       p_bst_info->config.sendAsyncReports = true;
       timerUpdateReqd = true;
     }
     /*
        Register with the timer for periodic callbacks */
     if (true == timerUpdateReqd)
     {
       bst_periodic_collection_timer_add (p_req_msg->unit,
                                          bst_periodic_collection_cb,
                                          INSIGHT_BST_CMD_API_GET_REPORT, 0);
     }
   }
   else
   {
     p_bst_info->config.sendAsyncReports = false;
     /* Periodic report collection is turned off...
        so no need for  the timer.
        delete the timer */
     bst_periodic_collection_timer_delete (p_req_msg->unit,
                                           INSIGHT_BST_CMD_API_GET_REPORT, 0);
   }
 }

  if ((tmpMask & (1 << BST_CONFIG_PARAMS_STATS_UNITS)) &&
      (p_bst_info->config.statUnitsInCells != p_req_msg->request.feature.statUnitsInCells))
  {
    /* Store the data is desired in bytes or cells */
    p_bst_info->config.statUnitsInCells = p_req_msg->request.feature.statUnitsInCells;
  }


  if ((tmpMask & (1 << BST_CONFIG_PARAMS_STATS_IN_PERCENT)) &&
      (p_bst_info->config.statsInPercentage != p_req_msg->request.feature.statsInPercentage))
  {
    /* Store the data is desired in percentage */
    p_bst_info->config.statsInPercentage = p_req_msg->request.feature.statsInPercentage;
  }


  if (!(tmpMask & (1 << BST_CONFIG_PARAMS_ENABLE)))
  {
    /* bst not enabled in config.
       just return */
      return rv;
  }


  BST_RWLOCK_UNLOCK(p_req_msg->unit);
  sal_memset (&bstMode, 0, sizeof (bstMode));
 /* Set the asic with the desired config to control bst */
  bstMode.enableStatsMonitoring = p_req_msg->request.feature.bstEnable;
  bstMode.enablePeriodicCollection = p_bst_info->config.sendAsyncReports;
  bstMode.collectionPeriod = p_bst_info->config.collectionInterval;
  rv = sbapi_bst_config_set (p_req_msg->unit, &bstMode);
  if (INSIGHT_STATUS_SUCCESS == rv)
  {
    /* asic is successfully programmed.. now update the config.. */
    BST_RWLOCK_WR_LOCK(p_req_msg->unit);
    p_bst_info->config.bstEnable = p_req_msg->request.feature.bstEnable;
    BST_RWLOCK_UNLOCK(p_req_msg->unit);
    LOG_POST (INSIGHT_LOG_INFO,  "bst application: setting bst feature is successful for unit %d.\r\n", p_req_msg->unit);
  }


  return rv;
}


INSIGHT_STATUS bst_config_feature_get (void * msg_data)
{
   INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: get bst feature for chip %d.\r\n", p_req_msg->unit);


  return rv;
}

/* Track set and get apis */
/*********************************************************************
* @brief : application function to configure the bst tracking
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : Inpput paramerts are invalid.
* @retval  : INSIGHT_STATUS_FAILURE : unable to set the bst tracking params
* @retval  : INSIGHT_STATUS_SUCCESS : successfully configured bst params.
*
* @note
*
*********************************************************************/
INSIGHT_STATUS bst_config_track_set (void * msg_data)
{
  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;
  bool config_changed = false;
//  bool trackIngress = true;
//  bool trackEgress = true;
//  bool trackDevice = true;
//  int trackPeakStats = 0;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


   /* tracking for current or peak mode is changed.. mark the same */
  if (p_bst_info->track.trackPeakStats != p_req_msg->request.track.trackPeakStats)
  {
    config_changed = true;
  }

  if (p_bst_info->track.trackIngressPortPriorityGroup != p_req_msg->request.track.trackIngressPortPriorityGroup)
  {
   /* tracking for ingress port pri grp has  changed.. mark the same */
    config_changed = true;
  }

  if (p_bst_info->track.trackIngressPortServicePool != p_req_msg->request.track.trackIngressPortServicePool)
  {
   /* tracking for ingress port service pool has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackIngressServicePool != p_req_msg->request.track.trackIngressServicePool)
  {
   /* tracking for ingress service pool has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressPortServicePool != p_req_msg->request.track.trackEgressPortServicePool)
  {
   /* tracking for egress port service pool has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressServicePool != p_req_msg->request.track.trackEgressServicePool)
  {
   /* tracking for egress service pool has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressUcQueue != p_req_msg->request.track.trackEgressUcQueue)
  {
   /* tracking for egress unicat queue has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressUcQueueGroup != p_req_msg->request.track.trackEgressUcQueueGroup)
  {
   /* tracking for egress unicast queuei grp has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressMcQueue != p_req_msg->request.track.trackEgressMcQueue)
  {
   /* tracking for egress multicast queue has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressCpuQueue != p_req_msg->request.track.trackEgressCpuQueue)
  {
   /* tracking for egress cpu queue has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackEgressRqeQueue != p_req_msg->request.track.trackEgressRqeQueue)
  {
   /* tracking for egress rqe queue has  changed */
    config_changed = true;
  }

  if (p_bst_info->track.trackDevice != p_req_msg->request.track.trackDevice)
  {
   /* tracking for global data  changed */
    config_changed = true;
  }

  if (false == config_changed)
  {
    /*no config change. so just return.
      what ever is desired.. is already in place .. */
    return rv;
  }
  #if 0
 /* request has additonal than what is currently there.
    program the asic */
  memset (&bstMode, 0, sizeof (INSIGHT_BST_CONFIG_t));

 if (true == msg_data->request.track.trackPeakStats)
 {
   trackPeakStats = INSIGHT_BST_MODE_PEAK;
 }
 else
 {
   trackPeakStats = INSIGHT_BST_MODE_CURRENT;
 }
  bstMode.enableStatsMonitoring = config_ptr->bstEnable;
  bstMode.enableDeviceStatsMonitoring = trackDevice;
  bstMode.enableIngressStatsMonitoring = trackIngress;
  bstMode.enableEgressStatsMonitoring = trackEgress;
  bstMode.mode = trackPeakStats;
/* program the asic. */
  rv = sbapi_bst_config_set (msg_data->unit, &bstMode);
  if (INSIGHT_STATUS_SUCCESS == rv)
  {
      /*successfully programed the asic. store the config */
    *ptr = msg_data->request.track;
    LOG_POST (INSIGHT_LOG_INFO,
              "bst application: setting bst tracking is successful"
               " for unit %d.\r\n", msg_data->unit);
  }
  return rv;

#endif
      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: setting bst tracking for chip %d.\r\n", p_req_msg->unit);


  return rv;
}

/*********************************************************************
* @brief : application function to get the bst tracking
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : Inpput paramerts are invalid.
* @retval  : INSIGHT_STATUS_SUCCESS  : successfully retrieved the tracking params.
*
* @note
*
*********************************************************************/
INSIGHT_STATUS bst_config_track_get (void * msg_data)
{
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: get bst tracking for chip %d.\r\n", p_req_msg->unit);


  return rv;
}


/*********************************************************************
* @brief : application function to get the bst report and thresholds
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER -- Inpput paramerts are invalid.
* @retval  : INSIGHT_STATUS_FAILURE -- report is successfully sent
* @retval  : INSIGHT_STATUS_SUCCESS -- failed to get the report
*
* @note : based on the message type the report is retrieved.
*
*********************************************************************/
INSIGHT_STATUS bst_get_report (void * p_msg_data)
{
   INSIGHT_BST_REQUEST_MSG_t   *msg_data = (INSIGHT_BST_REQUEST_MSG_t*) p_msg_data;

    INSIGHT_BST_REPORT_SNAPSHOT_t *ss;
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

    BSTJSON_CONFIGURE_BST_TRACKING_t *track_ptr;
    BSTJSON_CONFIGURE_BST_FEATURE_t *config_ptr;
    INSIGHT_TIME_t  curr_time;

    rv = rv;

    if (NULL == msg_data)
    {
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    config_ptr = &p_bst_info->config;
    track_ptr = &p_bst_info->track;

    if ((NULL == track_ptr) || (NULL == config_ptr))
    {
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    /* request is to get the report. */

    if (((INSIGHT_BST_PERIODIC == msg_data->report_type) ||
        (INSIGHT_BST_STATS_TRIGGER == msg_data->report_type)) &&
    (NULL != track_ptr))
    {
        BSTJSON_GET_BST_REPORT_t *pCollect = &msg_data->request.collect;

        /* copy the track params to the collection request */
        BST_COPY_TRACK_TO_COLLECT (track_ptr,  pCollect);
    }


    if ((INSIGHT_BST_CMD_API_GET_REPORT == msg_data->msg_type) ||
        (INSIGHT_BST_CMD_API_TRIGGER_REPORT == msg_data->msg_type))
    {
        /* collect data.. since the data is huge.. give the current record
        memory pointer directly so that we can avoid, copy */

        BST_LOCK_TAKE (msg_data->unit);
        ss = p_bst_info->stats_current_record_ptr;

        /* before we collect data..ensure there is no garbage..  */
        sal_memset (ss, 0,  sizeof (INSIGHT_BST_REPORT_SNAPSHOT_t));
        rv = sbapi_bst_snapshot_get (msg_data->unit, &ss->snapshot_data, &ss->tv);

        BST_LOCK_GIVE (msg_data->unit);

        if (INSIGHT_STATUS_SUCCESS != rv)
        {
            /* call failed..  log the reason code ..*/
            LOG_POST (INSIGHT_LOG_ERROR, "Failed to get bst stats, err %d \r\n", rv);
            /* Since stats collection has failed.. no need to do the rest.
            report the error to the calling function */
        }

        /* check if stats are requested in percentage format.
        if yes, then retrieve the default / max buffers allocated from ASIC */
        if (true == config_ptr->statsInPercentage)
        {
            sbapi_system_max_buf_snapshot_get (msg_data->unit, &p_bst_info->bst_max_buffers,  &curr_time);
        }

        if (INSIGHT_BST_CMD_API_TRIGGER_REPORT == msg_data->msg_type)
        {
            //   rv = bst_enable_on_trigger(msg_data, true);
        }
    }

    /* request is to get the thresholds..
    */
    if (INSIGHT_BST_CMD_API_GET_THRESHOLD == msg_data->msg_type)
    {
        /* here we need to take the lock, since we maintain only one record for
        thresholds. assumption is that threshold get is called very sparingly,
       where as report is quite often.. so one record is enough..
        since we have single record, don't want a read while collecting the  information.
        so protect the same */
         BST_LOCK_TAKE (msg_data->unit);
        /* make sure no garbage.. */
        sal_memset(&p_bst_info->threshold_record_ptr->snapshot_data, 0, sizeof(INSIGHT_BST_ASIC_SNAPSHOT_DATA_t));

        /* check if stats are requested in percentage format.
        if yes, then retrieve the default / max buffers allocated from ASIC */

        if (true == config_ptr->statsInPercentage)
        {
            sbapi_system_max_buf_snapshot_get (msg_data->unit, &p_bst_info->bst_max_buffers, &curr_time);
        }

        rv = sbapi_bst_threshold_get (msg_data->unit, &p_bst_info->threshold_record_ptr->snapshot_data, &p_bst_info->threshold_record_ptr->tv);

        /* Release  the lock . */
         BST_LOCK_GIVE (msg_data->unit);

        if (INSIGHT_STATUS_SUCCESS != rv)
        {
            /* call failed..  log the reason code ..*/
            LOG_POST (INSIGHT_LOG_ERROR, "Failed to get bst stats, err %d \r\n", rv);
            /* Since threshold get has failed.. no need to do the rest.
            report the error to the calling function */
        }
    }
    return rv;
}

/*********************************************************************
* @brief : set the threshold for the given realm.
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_SUCCESS - threshold is set successfully
* @retval  : INSIGHT_STATUS_FAILURE - failed to apply the inputted threshold
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note    : In case of any threshold set fail, the error is logged
*
*********************************************************************/
INSIGHT_STATUS bst_config_threshold_set (void * msg_data)
{
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
    INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

    BSTJSON_CONFIGURE_BST_THRESHOLDS_t *p_threshold = &p_req_msg->request.threshold ;
    if (NULL == msg_data)
        return INSIGHT_STATUS_INVALID_PARAMETER;

    switch(p_req_msg->threshold_type)
    {
    case INSIGHT_BST_DEVICE_THRESHOLD :
        if (!p_threshold->mask & INSIGHT_BST_DEVICE_MASK)
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }
        break;
    case INSIGHT_BST_EGRESS_SP_THRESHOLD:
        if (!((p_threshold->mask & INSIGHT_BST_UMSHARE_MASK) ||
            (p_threshold->mask & INSIGHT_BST_UCSHARE_MASK) ||
        (p_threshold->mask & INSIGHT_BST_MCSHARE_MASK) ||
        (p_threshold->mask & INSIGHT_BST_MCSHARE_QUEUE_MASK)))
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }
    //    p_threshold->umHeadroomThreshold = 0;
        break;
    case INSIGHT_BST_EGRESS_UC_QUEUE_THRESHOLD:
        if (!(p_threshold->mask & INSIGHT_BST_UC_MASK))
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }

        if (!(p_threshold->mask & INSIGHT_BST_QUEUE_MASK))
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }

        break;

    case INSIGHT_BST_EGRESS_CPU_QUEUE_THRESHOLD:
        if (!(p_threshold->mask & INSIGHT_BST_CPU_MASK))
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }
        if (!(p_threshold->mask & INSIGHT_BST_QUEUE_MASK))
        {
            return INSIGHT_STATUS_INVALID_PARAMETER;
        }
        break;
    case INSIGHT_BST_EGRESS_MC_QUEUE_THRESHOLD:
    case INSIGHT_BST_EGRESS_RQE_QUEUE_THRESHOLD:
    case INSIGHT_BST_EGRESS_UC_QUEUEGROUPS_THRESHOLD:
    case INSIGHT_BST_EGRESS_PORT_SP_THRESHOLD:
    case INSIGHT_BST_INGRESS_PORT_PG_THRESHOLD:
    case INSIGHT_BST_INGRESS_PORT_SP_THRESHOLD:
    case INSIGHT_BST_INGRESS_SP_THRESHOLD:
        return INSIGHT_STATUS_UNSUPPORTED;
        break;
    default:
        break;
    }

    LOG_POST (INSIGHT_LOG_INFO,
              "bst application: setting bst threshold for chip %d.\r\n", p_req_msg->unit);


    return rv;
}

/*********************************************************************
* @brief : function to clear the threshold set
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_SUCCESS : successfully cleared the threshold values
* @retval  : INSIGHT_STATUS_FAILURE : The clearing of thresholds has failed.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note :
*
*********************************************************************/
INSIGHT_STATUS bst_clear_threshold_set (void * msg_data)
{
   INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: clear bst threshold for chip %d.\r\n", p_req_msg->unit);


  return rv;
}

/*********************************************************************
* @brief : clear the stats
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_SUCCESS : successfully cleared the stats values
* @retval  : INSIGHT_STATUS_FAILURE : The clearing of stats has failed.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note
*
*********************************************************************/
INSIGHT_STATUS bst_clear_stats_set (void * msg_data)
{
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: clear bst stats for chip %d.\r\n", p_req_msg->unit);


  return rv;
}
INSIGHT_STATUS bst_get_cgsn_drp_ctrs(void * msg_data)
{
      INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: get bst cgsn_drp for chip %d.\r\n", p_req_msg->unit);


  return rv;
}


/*********************************************************************
* @brief : clear the trigger counters
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_SUCCESS : successfully cleared the trigger report count
* @retval  : INSIGHT_STATUS_FAILURE : The clearing of count has failed.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note
*
*********************************************************************/
INSIGHT_STATUS bst_clear_trigger_count (void * msg_data)
{
     INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;
   INSIGHT_BST_REQUEST_MSG_t   *p_req_msg = (INSIGHT_BST_REQUEST_MSG_t*) msg_data;

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;


      LOG_POST (INSIGHT_LOG_INFO,
              "bst application: clear bst trigger counters  for chip %d.\r\n", p_req_msg->unit);


  return rv;
}


/*********************************************************************
* @brief :  re-enable bst on trigger timer expiry
*
* @param[in] msg_data : pointer to the bst message request.
*
* @retval  : INSIGHT_STATUS_SUCCESS : successfully cleared the trigger report count
* @retval  : INSIGHT_STATUS_FAILURE : The re-enable has failed.
* @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note
*
*********************************************************************/
INSIGHT_STATUS bst_enable_on_trigger_timer_expiry (void * msg_data)
{
  INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

  return rv;
}
/*********************************************************************
 * @brief : function to return the api handler for the bst command type
 *
 * @param[in] int : request type
 *
 * @retval    : function pointer to the corresponding api
 *
 * @note :
 *
 *********************************************************************/
INSIGHT_STATUS insight_bst_type_get_api (int type, INSIGHT_APP_API_HANDLER_t *handler)
{
  uint8 i = 0;

  static const insight_app_api_t bst_api_list[] = {
    {INSIGHT_BST_CMD_API_GET_FEATURE, bst_config_feature_get},
    {INSIGHT_BST_CMD_API_GET_TRACK, bst_config_track_get},
    {INSIGHT_BST_CMD_API_GET_REPORT, bst_get_report},
    {INSIGHT_BST_CMD_API_GET_THRESHOLD, bst_get_report},
    {INSIGHT_BST_CMD_API_TRIGGER_REPORT, bst_get_report},
  //  {INSIGHT_BST_CMD_API_TRIGGER_COLLECT, bst_process_trigger},
    {INSIGHT_BST_CMD_API_SET_FEATURE, bst_config_feature_set},
    {INSIGHT_BST_CMD_API_SET_TRACK, bst_config_track_set},
    {INSIGHT_BST_CMD_API_SET_THRESHOLD, bst_config_threshold_set},
    {INSIGHT_BST_CMD_API_CLEAR_THRESHOLD, bst_clear_threshold_set},
    {INSIGHT_BST_CMD_API_CLEAR_STATS, bst_clear_stats_set},
    {INSIGHT_BST_CMD_API_CLEAR_TRIGGER_COUNT, bst_clear_trigger_count},
    {INSIGHT_BST_CMD_API_ENABLE_BST_ON_TRIGGER, bst_enable_on_trigger_timer_expiry},
    {INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS, bst_get_cgsn_drp_ctrs}
  };

  for (i = 0; i < INSIGHT_BST_CMD_API_MAX-1; i++)
  {
    if (type == bst_api_list[i].command)
    {
      *handler = bst_api_list[i].handler;
      return INSIGHT_STATUS_SUCCESS;
    }
  }
  return INSIGHT_STATUS_INVALID_PARAMETER;
}


INSIGHT_STATUS insight_bst_cancel_request(unsigned int unit, unsigned int id)
{
#if 0
    unsigned int index = 0;
    INSIGHT_STATUS rv;


    /* check if the record already exists */
    rv = bst_drop_ctrs_db_get_rec_index(bst_data_ptr, id,        &index);


    if (INSIGHT_STATUS_SUCCESS == rv)
    {
        /* take lock */
        BST_LOCK_TAKE (unit);

        /* clear the timer, if any */
        bst_periodic_collection_timer_delete (unit,
                                              INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS,
                                              index);

        memset(&bst_data_ptr->drop_ctrs_db[index], 0 , sizeof(INSIGHT_BST_CGSN_DROP_ELEM_t));

        /* release lock */
        BST_LOCK_GIVE (unit);
        rv = INSIGHT_STATUS_SUCCESS;
    }
    else
    {
        rv = INSIGHT_STATUS_NOT_CONFIGURED;
    }
#endif
    return INSIGHT_STATUS_SUCCESS;
}


/*********************************************************************
* @brief : bst main application function which does processing of messages
*
* @param[in] : none
*
* @retval  : INSIGHT_STATUS_SUCCESS: Fails to register with module manager
*             or some error happened due to which the bst application is
*             unable to process incoming messages.
* @retval  : INSIGHT_STATUS_FAILURE: Fails to register with module manager
*
* @note  : This api is the processing thread of the bst application.
*          All the incoming requests are processed and the responses
*          are sent in the bst context. Currently the assumption
*          is made that if the thread fails to read continously 10 or
*          more messages, then there is some error and the thread exits.
*
*********************************************************************/
INSIGHT_STATUS insight_bst_handler_thread (void)
{
  INSIGHT_BST_REQUEST_MSG_t msg_data;
  INSIGHT_BST_RESPONSE_MSG_t reply_data;
  INSIGHT_STATUS rv = INSIGHT_STATUS_FAILURE;
  uint8 rcvd_err = 0;
  unsigned  num_units = 1;
  INSIGHT_APP_API_HANDLER_t handler;



  if (INSIGHT_STATUS_SUCCESS !=
      insight_system_register_cancel_request(INSIGHT_FEATURE_BST, insight_bst_cancel_request))
  {
    /* registration with system utils for cancel request has failed.
       return failure. so that the caller can clean the resources */
    LOG_POST (INSIGHT_LOG_ERROR,
              "Registration with system utils for cancel request failed \r\n");

    return INSIGHT_STATUS_FAILURE;
  }


  while (1)
  {
    if (-1 != (msgrcv (p_bst_info->recvMsgQid, &msg_data, sizeof (msg_data), 0, 0)))
    {
    num_units = num_units;

      LOG_POST (INSIGHT_LOG_INFO, "insight_bst_handler_thread msg_data info:msg_data.msg_type = %u, msg_data.unit = %d, num_units:%d, msg_data.cookie = %d\n",
          msg_data.msg_type, msg_data.unit, num_units, (NULL == msg_data.cookie)? true: false);

      LOG_POST (INSIGHT_LOG_INFO, "insight_bst_handler_thread msg_data info:msg_data.msg_type = %u, msg_data.unit = %d,msg_data.cookie = %d\n",
          msg_data.msg_type, msg_data.unit, (NULL == msg_data.cookie)? true: false);
      if (msg_data.unit >= 1)
      {
        /* requested id is greater than the number of units.
           send failure message and continue */
        memset (&reply_data, 0, sizeof (INSIGHT_BST_RESPONSE_MSG_t));
        reply_data.rv = INSIGHT_STATUS_INVALID_PARAMETER;

        /* copy the request type */
        reply_data.msg_type = msg_data.msg_type;
        /* copy the unit */
        reply_data.unit = msg_data.unit;
        /* copy the asic type */
        reply_data.id = msg_data.id;
        /* copy the cookie ..  */
        reply_data.cookie = msg_data.cookie;

        rv = insight_bst_send_response(&reply_data);
        if (INSIGHT_STATUS_SUCCESS != rv)
        {
          LOG_POST (INSIGHT_LOG_ERROR,
              "failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, rv);
        }
        continue;
      }
      /* Counter to check for read errors..
         successfully read the message. clear this to 0*/
      rcvd_err = 0;
      /* Memset the response message */
      memset (&reply_data, 0, sizeof (INSIGHT_BST_RESPONSE_MSG_t));

      /* get the api function for the method type */
      if (INSIGHT_STATUS_SUCCESS != insight_bst_type_get_api (msg_data.msg_type, &handler))
      {
        continue;
      }

      rv = handler(&msg_data);

      reply_data.rv = rv;

      rv = bst_copy_reply_params (&msg_data, &reply_data);

      if (INSIGHT_STATUS_SUCCESS != rv)
      {
        LOG_POST (INSIGHT_LOG_ERROR,
                "bst_main.c failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, rv);
        LOG_POST (INSIGHT_LOG_ERROR,
            "failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, rv);
      }
    }
    else
    {
      LOG_POST (INSIGHT_LOG_ERROR,
          "Failed to read message from the bst application queue. err = %d\r\n", errno);
      /* increment the error counter.
         If this counter increments continously exit and return failure
         so that corrective action can be taken */
       rcvd_err++;

        /* wait till 10 consective messages */
       if(INSIGHT_BST_MAX_QUEUE_SEND_FAILS < rcvd_err)
       {
         break;
       }
       continue;
    }
  }                             /* while (1) */
  LOG_POST (INSIGHT_LOG_EMERGENCY,
            "exiting from bst pthread ... \r\n");
  pthread_exit(&p_bst_info->bst_thread);
  return INSIGHT_STATUS_FAILURE;
}


INSIGHT_STATUS insight_bst_trigger_handler_thread(void)
{
  INSIGHT_BST_REQUEST_MSG_t msg_data;
  INSIGHT_STATUS rv = INSIGHT_STATUS_FAILURE;
  uint8 rcvd_err = 0;
  uint8 num_units = 0;
  INSIGHT_APP_API_HANDLER_t handler;

  while (1)
  {
      if (-1 != (msgrcv (p_bst_info->recvTriggerMsgQid, &msg_data, sizeof (msg_data), 0, 0)))
      {
          /* get num units */
          num_units = 0;
#if 0
          if (INSIGHT_STATUS_SUCCESS != sbapi_system_num_units_get ((int *) &num_units))
          {
              LOG_POST (INSIGHT_LOG_ERROR, "Failed to get num of units\r\n");
          }
#endif
          LOG_POST (INSIGHT_LOG_INFO, "trigger : msg_data info:msg_data.msg_type = %ld msg_data.unit = %d msg_data.cookie = %d num_units = %d\n",
                    msg_data.msg_type, msg_data.unit, (NULL == msg_data.cookie)? true: false, num_units);

          if (msg_data.unit >= num_units)
          {
              /* requested id is greater than the number of units.
              send failure message and continue */
              LOG_POST (INSIGHT_LOG_ERROR,
                        "failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, rv);
              continue;
          }
          /* Counter to check for read errors..
          successfully read the message. clear this to 0*/
          rcvd_err = 0;
          /* get the api function for the method type */
          if (INSIGHT_STATUS_SUCCESS != insight_bst_type_get_api(msg_data.msg_type, &handler))
          {
              continue;
          }

          handler(&msg_data);
      }
      else
      {
          LOG_POST (INSIGHT_LOG_ERROR,
                    "Failed to read message from the bst trigger application queue. err = %d\r\n", errno);
          /* increment the error counter.
          If this counter increments continously exit and return failure
          so that corrective action can be taken */
          rcvd_err++;

          /* wait till 10 consective messages */
          if (INSIGHT_BST_MAX_QUEUE_SEND_FAILS < rcvd_err)
          {
              break;
          }
          continue;
      }
  }                             /* while (1) */
             LOG_POST (INSIGHT_LOG_EMERGENCY,
                 "exiting from bst trigger pthread ... \r\n");
  pthread_exit(&p_bst_info->bst_trigger_thread);
  return INSIGHT_STATUS_FAILURE;
}

#define _INSIGHT_API_

INSIGHT_STATUS insight_bst_deinit ()
{
  return 0;
}


/*********************************************************************
* @brief : function to initializes bst structures to default values
*
* @param[in] : number of units on the agent
*
* @retval  : INSIGHT_STATUS_SUCCESS : configuration is successfully initialized.
* @retval  : INSIGHT_STATUS_FAILURE : Fail to initialize the BST appliation to default.
*
* @note  : Initialization functon to make sure both software and asic
*          are configured with same default values. We are not setting
*          the default values for threshold, but we are using
*          the same as that of asic.
*
*********************************************************************/

/* Default values for BST configurations */
  /* bst enable */
#define INSIGHT_BST_DEFAULT_ENABLE    false
#define INSIGHT_BST_PERIODIC_REPORT_DEFAULT false
#define INSIGHT_BST_DEFAULT_INTERVAL  60
#define INSIGHT_BST_DEFAULT_STATS_UNITS  false
#define INSIGHT_BST_DEFAULT_STATS_PERCENTAGE false
#define INSIGHT_BST_DEFAULT_TRACK_INGRESS   true
#define INSIGHT_BST_DEFAULT_TRACK_EGRESS    true
#define INSIGHT_BST_DEFAULT_TRACK_DEVICE    true
#define INSIGHT_BST_DEFAULT_TRACK_IN_P_PG      true
#define INSIGHT_BST_DEFAULT_TRACK_IN_P_SP      true
#define INSIGHT_BST_DEFAULT_TRACK_IN_SP        true
#define INSIGHT_BST_DEFAULT_TRACK_E_P_SP       true
#define INSIGHT_BST_DEFAULT_TRACK_E_SP         true
#define INSIGHT_BST_DEFAULT_TRACK_E_UC_Q       true
#define INSIGHT_BST_DEFAULT_TRACK_E_UC_QG      true
#define INSIGHT_BST_DEFAULT_TRACK_E_MC_Q       true
#define INSIGHT_BST_DEFAULT_TRACK_E_CPU_Q      true
#define INSIGHT_BST_DEFAULT_TRACK_E_RQE_Q      true
#define INSIGHT_BST_DEFAULT_TRACK_MODE         1
#define INSIGHT_BST_DEFAULT_MAX_TRIGGERS 1
#define INSIGHT_BST_DEFAULT_SNAPSHOT_TRIGGER true
#define INSIGHT_BST_DEFAULT_TRIGGER_INTERVAL 1
#define INSIGHT_BST_DEFAULT_SEND_INCR_REPORT  1
#define INSIGHT_BST_MAX_UNITS 8
#define INSIGHT_BST_TIME_CONVERSION_FACTOR 1000

INSIGHT_STATUS insight_app_config_init (unsigned int num_units)
{
    INSIGHT_BST_CXT_t *ptr;
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

    ptr = p_bst_info;


    /* bst enable */
    ptr->config.bstEnable = INSIGHT_BST_DEFAULT_ENABLE;
    /* send async reports  */
    ptr->config.sendAsyncReports = INSIGHT_BST_PERIODIC_REPORT_DEFAULT;
    /* collection interval  */
    ptr->config.collectionInterval = INSIGHT_BST_DEFAULT_INTERVAL;
    /* stats in cells or bytes.  */
    ptr->config.statUnitsInCells = INSIGHT_BST_DEFAULT_STATS_UNITS;
    ptr->config.statsInPercentage = INSIGHT_BST_DEFAULT_STATS_PERCENTAGE;
    ptr->config.bstMaxTriggers = INSIGHT_BST_DEFAULT_MAX_TRIGGERS;
    ptr->config.sendSnapshotOnTrigger = INSIGHT_BST_DEFAULT_SNAPSHOT_TRIGGER;
    ptr->config.triggerTransmitInterval = INSIGHT_BST_DEFAULT_TRIGGER_INTERVAL;
    ptr->config.sendIncrementalReport = INSIGHT_BST_DEFAULT_SEND_INCR_REPORT;

    /* enable device tracking   */
    ptr->track.trackDevice = INSIGHT_BST_DEFAULT_TRACK_DEVICE;
    /* enable ingress tracking p + pg   */
    ptr->track.trackIngressPortPriorityGroup = INSIGHT_BST_DEFAULT_TRACK_IN_P_PG;
    /* enable ingress tracking p + sp  */
    ptr->track.trackIngressPortServicePool = INSIGHT_BST_DEFAULT_TRACK_IN_P_SP;
    /* enable ingress tracking  sp  */
    ptr->track.trackIngressServicePool = INSIGHT_BST_DEFAULT_TRACK_IN_SP;
    /* enable egress tracking P+ sp  */
    ptr->track.trackEgressPortServicePool = INSIGHT_BST_DEFAULT_TRACK_E_P_SP;
    /* enable egress tracking sp  */
    ptr->track.trackEgressServicePool = INSIGHT_BST_DEFAULT_TRACK_E_SP;
    /* enable egress tracking uc queues  */
    ptr->track.trackEgressUcQueue = INSIGHT_BST_DEFAULT_TRACK_E_UC_Q;
    /* enable egress tracking uc queue grp */
    ptr->track.trackEgressUcQueueGroup = INSIGHT_BST_DEFAULT_TRACK_E_UC_QG;
    /* enable egress tracking mc queue grp */
    ptr->track.trackEgressMcQueue = INSIGHT_BST_DEFAULT_TRACK_E_MC_Q;
    /* enable egress tracking cpu queue grp */
    ptr->track.trackEgressCpuQueue = INSIGHT_BST_DEFAULT_TRACK_E_CPU_Q;
    /* enable egress tracking rqe queue grp */
    ptr->track.trackEgressRqeQueue = INSIGHT_BST_DEFAULT_TRACK_MODE;
    /* enable  tracking mode to current */
    ptr->track.trackPeakStats = false;

    /* Initialize the bst timer array */
    ptr->collection_timer.context.unit = 0;
    ptr->collection_timer.in_use = false;
    ptr->trigger_timer.in_use = false;
    ptr->trigger_timer.context.unit = 0;


#if 0
    /* push default values to asic */
    /* populate the default thereshold cache */
    memset(&bst_info.unit[0].bst_thresholds_cache, 0, sizeof(INSIGHT_BST_ASIC_SNAPSHOT_DATA_t));
    rv = sbapi_bst_threshold_get (unit_id,
                                  &bst_info.unit[0].bst_thresholds_cache,
                                  &tv);

    memset (&bstMode, 0, sizeof (INSIGHT_BST_CONFIG_t));
    bstMode.enableStatsMonitoring = INSIGHT_BST_DEFAULT_ENABLE;
    bstMode.enableDeviceStatsMonitoring = INSIGHT_BST_DEFAULT_TRACK_DEVICE;
    bstMode.enableIngressStatsMonitoring = INSIGHT_BST_DEFAULT_TRACK_INGRESS;
    bstMode.enableEgressStatsMonitoring = INSIGHT_BST_DEFAULT_TRACK_EGRESS;
    bstMode.mode = INSIGHT_BST_MODE_CURRENT;

    if (INSIGHT_STATUS_SUCCESS != sbapi_bst_config_set (unit_id, &bstMode))
    {
        LOG_POST (INSIGHT_LOG_ERROR,
                  "Failed to set bst config params for unit %d\r\n", unit_id);
        return INSIGHT_STATUS_FAILURE;
    }

#endif


    if ((false != ptr->config.sendAsyncReports) && (ptr->config.bstEnable != false))
    {
        /* register for timer callback only if reports need
        to be sent asyncronously */
        rv = bst_periodic_collection_timer_add (0,
                                                bst_periodic_collection_cb,
                                                INSIGHT_BST_CMD_API_GET_REPORT, 0);
        if (INSIGHT_STATUS_SUCCESS != rv)
        {
            LOG_POST (INSIGHT_LOG_ERROR,  "Failed to register with timer  for callbacks for  unit %d\r\n", 0);
            return INSIGHT_STATUS_FAILURE;
        }
    }


    LOG_POST (INSIGHT_LOG_INFO,  "bst default initialization successful\r\n");
    return rv;
}



INSIGHT_STATUS insight_asic_capabilities_get (uint8 asicType,
                                              INSIGHT_ASIC_CAPABILITIES_t*asicCapPtr)
{
    asicCapPtr->numPorts = 64;
    asicCapPtr->numUnicastQueues = INSIGHT_ASIC_MAX_UC_QUEUES;
    asicCapPtr->numUnicastQueueGroups = INSIGHT_ASIC_MAX_UC_QUEUE_GROUPS;
    asicCapPtr->numMulticastQueues = INSIGHT_ASIC_MAX_MC_QUEUES;
    asicCapPtr->numServicePools = INSIGHT_ASIC_MAX_SERVICE_POOLS;
    asicCapPtr->numCommonPools = INSIGHT_ASIC_MAX_COMMON_POOLS;
    asicCapPtr->numCpuQueues = INSIGHT_ASIC_MAX_CPU_QUEUES;
    asicCapPtr->numRqeQueues = INSIGHT_ASIC_MAX_RQE_QUEUES;
    asicCapPtr->numRqeQueuePools = INSIGHT_ASIC_MAX_RQE_QUEUE_POOLS;
    asicCapPtr->numPriorityGroups = INSIGHT_ASIC_MAX_PRIORITY_GROUPS;
    asicCapPtr->support1588 = 0;
    asicCapPtr->cellToByteConv = 0;
    return 0;
}

INSIGHT_STATUS insight_bst_init ()
{

    int rv = INSIGHT_STATUS_SUCCESS;
    int recvMsgQid;


    p_bst_info = (INSIGHT_BST_CXT_t *) malloc (sizeof (INSIGHT_BST_CXT_t));

    if (NULL == p_bst_info)
    {
        return INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE;
    }

    p_bst_info->key1 = INSIGHT_MSG_QUEUE_ID_TO_BST;
    p_bst_info->trigger_key = INSIGHT_MSG_QUEUE_ID_TO_BST_TRIGGER;



    /* stats records */
    p_bst_info->stats_active_record_ptr =  (INSIGHT_BST_REPORT_SNAPSHOT_t *)
    sal_malloc (sizeof (INSIGHT_BST_REPORT_SNAPSHOT_t));

    p_bst_info->stats_backup_record_ptr =  (INSIGHT_BST_REPORT_SNAPSHOT_t *)
    sal_malloc (sizeof (INSIGHT_BST_REPORT_SNAPSHOT_t));

    p_bst_info->stats_current_record_ptr = (INSIGHT_BST_REPORT_SNAPSHOT_t *)
    sal_malloc (sizeof (INSIGHT_BST_REPORT_SNAPSHOT_t));

    /* threshold records */
    p_bst_info->threshold_record_ptr = (INSIGHT_BST_REPORT_SNAPSHOT_t *)
    sal_malloc (sizeof (INSIGHT_BST_REPORT_SNAPSHOT_t));


    if (INSIGHT_STATUS_SUCCESS != insight_bst_module_register ())
    {
        /* registration with module mgr has failed.
        return failure. so that the caller can clean the resources */
        LOG_POST (INSIGHT_LOG_EMERGENCY, "Registration with module mgr failed \r\n");
        return INSIGHT_STATUS_FAILURE;
    }


    if (INSIGHT_STATUS_SUCCESS != insight_app_config_init (0))
    {
        return INSIGHT_STATUS_FAILURE;
    }

    /* get the asic capabilities of the system
    * save the same so that the same can be reused
    */
    if (INSIGHT_STATUS_SUCCESS != insight_asic_capabilities_get(0,
                                                                &p_bst_info->asic_capabilities))
    {
        /* unable to get the asic capabilities
        log error and return */
        LOG_POST (INSIGHT_LOG_ERROR,
                  "Failed to Get Asic capabilities for unit %d. \r\n", 0);
        return INSIGHT_STATUS_FAILURE;
    }
    else
    {

        sal_printf("supported capabilities are\n"
        "num ports = %d, numUnicastQueues = %d\n"
        "numUnicastQueueGroups = %d, numMulticastQueues = %d\n"
        "numServicePools = %d, numCommonPools = %d\n"
        "numCpuQueues = %d, numRqeQueues = %d\n"
        "numRqeQueuePools = %d, numPriorityGroups = %d\n",
           p_bst_info->asic_capabilities.numPorts,
           p_bst_info->asic_capabilities.numUnicastQueues,
           p_bst_info->asic_capabilities.numUnicastQueueGroups,
           p_bst_info->asic_capabilities.numMulticastQueues,
           p_bst_info->asic_capabilities.numServicePools,
           p_bst_info->asic_capabilities.numCommonPools,
           p_bst_info->asic_capabilities.numCpuQueues,
           p_bst_info->asic_capabilities.numRqeQueues,
           p_bst_info->asic_capabilities.numRqeQueuePools,
           p_bst_info->asic_capabilities.numPriorityGroups
        );
    }


    pthread_mutex_init (&p_bst_info->bst_mutex, NULL);
    /* Initialize Read Write lock with default attributes */
    pthread_rwlock_init (&p_bst_info->bst_configRWLock, NULL);


    /* Check if message queue is already exitsing */
    if (0 > (recvMsgQid = msgget(p_bst_info->key1, IPC_EXCL | IPC_CREAT | 0777)))
    {
        if (EEXIST == errno)
        {

            /* get the already existing  message queue id for bst */
            if ((recvMsgQid = msgget (p_bst_info->key1, IPC_CREAT | 0777)) < 0)
            {
                /* Free the resources allocated so far */
                //bst_app_uninit ();

                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to create  msgQ for bst application. err = %d\n", errno);
                return  INSIGHT_STATUS_FAILURE;
            }

            /* message queue exits.. Delete the message queue */
            if  (msgctl(recvMsgQid, IPC_RMID, (struct msqid_ds *)NULL) < 0)
            {
                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to destroy bst msgQ id, err = %d\n", errno);
            }


            /* create the message queue for bst */
            if ((recvMsgQid = msgget (p_bst_info->key1, IPC_CREAT | 0777)) < 0)
            {
                /* Free the resources allocated so far */
                insight_bst_deinit ();

                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to create  msgQ for bst application. err = %d\n", errno);
                return  INSIGHT_STATUS_FAILURE;
            }
        }
    }
    p_bst_info->recvMsgQid = recvMsgQid;

    /* create pthread for bst application */
    if (0 != pthread_create (&p_bst_info->bst_thread, NULL, (void *) &insight_bst_handler_thread, NULL))
    {
        LOG_POST (INSIGHT_LOG_EMERGENCY, "BST  thread creation failed %d\r\n", errno);
        return  INSIGHT_STATUS_FAILURE;
    }
    LOG_POST (INSIGHT_LOG_INFO,
              "bst application: bst pthread created\r\n");


    /* Check if message queue is already exitsing */
    if (0 > (recvMsgQid = msgget(p_bst_info->trigger_key, IPC_EXCL | IPC_CREAT | 0777)))
    {
        if (EEXIST == errno)
        {

            /* get the already existing  message queue id for bst */
            if ((recvMsgQid = msgget (p_bst_info->trigger_key, IPC_CREAT | 0777)) < 0)
            {
                /* Free the resources allocated so far */
                //bst_app_uninit ();

                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to create  msgQ for bst trigger. err = %d\n", errno);
                return  INSIGHT_STATUS_FAILURE;
            }

            /* message queue exits..
            Delete the message queue */
            if  (msgctl(recvMsgQid, IPC_RMID, (struct msqid_ds *)NULL) < 0)
            {
                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to destroy bst msgQ id, err = %d\n", errno);
            }


            /* create the message queue for bst */
            if ((recvMsgQid = msgget (p_bst_info->trigger_key, IPC_CREAT | 0777)) < 0)
            {
                /* Free the resources allocated so far */
                insight_bst_deinit();

                LOG_POST (INSIGHT_LOG_EMERGENCY,
                          "Failed to create  msgQ for bst application. err = %d\n", errno);
                return  INSIGHT_STATUS_FAILURE;
            }

        }
    }

    p_bst_info->recvTriggerMsgQid = recvMsgQid;

    /* create pthread for bst application */
    if (0 != pthread_create (&p_bst_info->bst_trigger_thread, NULL, (void *) &insight_bst_trigger_handler_thread, NULL))
    {
        LOG_POST (INSIGHT_LOG_EMERGENCY, "BST  trigger processing thread creation failed %d\r\n", errno);
        return  INSIGHT_STATUS_FAILURE;
    }
    LOG_POST (INSIGHT_LOG_INFO,
              "bst application: bst trigger process pthread created\r\n");

    return rv;
}

