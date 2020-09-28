
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>

#include "cJSON.h"

#include "json.h"
#include "insight.h"
#include "modulemgr.h"
#include "sal.h"
#include "system.h"
#include "log_api.h"
#include "rest_api.h"

/* System application Context Info*/
insight_system_info_t     *pg_switch_info;



#define _INSIGNT_APP_TO_JSON_

INSIGHT_STATUS insight_system_json_encode_get_feature( int method,
    const insight_system_config_feature_params_t *pData,
    uint8 **pJsonBuffer
    )
{
  char *getSystemUtilsFeatureTemplate = " {\
                                 \"jsonrpc\": \"2.0\",\
                                 \"method\": \"get-system-feature\",\
                                 \"version\": \"%d\",\
                                 \"result\": {\
                                 \"heartbeat-enable\": %d,\
                                 \"msg-interval\": %d\
},\
  \"id\": %d\
  }";

char *jsonBuf;


LOG_POST (INSIGHT_LOG_DEBUG, "SYSTEM-UTIL-JSON-Encoder : Request for Get-System-Feature \n");


/* allocate memory for JSON */
jsonBuf = mem_malloc(MEM_APP_MODULE,INSIGHT_JSON_MEMSIZE_RESPONSE);
if(!jsonBuf)
{
  return INSIGHT_STATUS_INVALID_MEMORY;
}

/* clear the buffer */
sal_memset(jsonBuf, 0, INSIGHT_JSON_MEMSIZE_RESPONSE);

/* encode the JSON */
snprintf(jsonBuf, INSIGHT_JSON_MEMSIZE_RESPONSE, getSystemUtilsFeatureTemplate,
    INSIGHT_JSON_VERSION, pData->enable, pData->interval, method);

/* setup the return value */
*pJsonBuffer = (uint8 *) jsonBuf;

LOG_POST (INSIGHT_LOG_DEBUG, "SYSTEM-JSON-Encoder : [%d bytes] , %s \n",(int)sal_strlen(jsonBuf), jsonBuf);

return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-switch-properties" REST API.
 *
 * @param[in]   method      Method ID (from original request) that needs
 *                          to be encoded in JSON.
 * @param[in]   pData       Data structure holding the required parameters.
 * @param[out]  pJsonBuffer Filled-in JSON buffer
 *
 * @retval   INSIGHT_STATUS_SUCCESS  Data is encoded into JSON successfully
 * @retval   INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE  Internal Error
 * @retval   INSIGHT_STATUS_INVALID_PARAMETER  Invalid input parameter
 * @retval   INSIGHT_STATUS_OUTOFMEMORY  No available memory to create JSON buffer
 *
 * @note     The returned json-encoded-buffer should be freed using the
 *           system_utils_json_memory_free(). Failing to do so leads to memory leaks
 *********************************************************************/
INSIGHT_STATUS insight_system_json_encode_get_switch_properties (int method,
    insight_system_properties_t *pData,
    uint8 **pJsonBuffer
    )
{
  char *getSwitchPropTemplate = "{\
                                 \"jsonrpc\": \"2.0\",\
                                 \"method\": \"get-switch-properties\",\
                                 \"version\": \"%d\",\
                                 \"time-stamp\": \"%s\",\
                                 \"result\": {\"number-of-asics\": %d,\
                                 \"asic-info\":[%s],\
                                 \"supported-features\":[%s],\
                                 \"network-os\":\"%s\",\
                                 \"uid\":\"%s\",\
                                 \"agent-ip\":\"%s\",\
                                 \"agent-port\":\"%d\",\
                                 \"agent-sw-version\":\"%s\"},\
                                 \"id\":%d \
}";
  char *getSwitchPropAsyncTemplate = "{\
                                 \"jsonrpc\": \"2.0\",\
                                 \"method\": \"get-switch-properties\",\
                                 \"version\": \"%d\",\
                                 \"time-stamp\": \"%s\",\
                                 \"result\": {\"number-of-asics\": %d,\
                                 \"asic-info\":[%s],\
                                 \"supported-features\":[%s],\
                                 \"network-os\":\"%s\",\
                                 \"uid\":\"%s\",\
                                 \"agent-ip\":\"%s\",\
                                 \"agent-port\":\"%d\",\
                                 \"agent-sw-version\":\"%s\"}\
}";
  char *asicInfoTemplate = "[\"%s\", \"%s\", %d],";
  char *featureTemplate = "\"%s\"";
  char *jsonBuf;

  char asicInfoStr[JSON_MAX_NODE_LENGTH]={0};
  char featureStr[JSON_MAX_NODE_LENGTH]={0};
  int asic =0;
  int len = 0;
  int array_len = 0;
  int totalLen = 0;
  time_t report_time;
  struct tm *timeinfo;
  unsigned int  featureCount = 0;
  char timeString[INSIGHT_TIME_BUFFER_SIZE];
  char buf[JSON_MAX_NODE_LENGTH]={0};


  LOG_POST (INSIGHT_LOG_DEBUG, "SYSTEM_UTILS-JSON-Encoder : Request for Get-Switch-Properties \n");



/* allocate memory for JSON */
  jsonBuf = mem_malloc(MEM_APP_MODULE,INSIGHT_JSON_MEMSIZE_RESPONSE);
  if(!jsonBuf)
  {
    return INSIGHT_STATUS_INVALID_MEMORY;
  }

  /* clear the buffer */
  sal_memset(jsonBuf, 0, INSIGHT_JSON_MEMSIZE_RESPONSE);

    /* obtain the time */
    sal_memset(&timeString, 0, sizeof (timeString));
    report_time = *(time_t *) &pData->timeStamp;
    timeinfo = localtime(&report_time);
    strftime(timeString, 64, "%Y-%m-%d - %H:%M:%S", timeinfo);

 array_len = JSON_MAX_NODE_LENGTH-1;
  for (asic = 0; asic < pData->numAsics ; asic++)
  {
      if (pData->asicInfo[asic].asicType == 1)
    {
      sprintf (buf, "%s","BCM56850");
    }

    len  = snprintf (&asicInfoStr[totalLen], array_len, asicInfoTemplate,
        pData->asicInfo[asic].asic_notation,
        buf,
        pData->asicInfo[asic].numPorts);
    totalLen += len;
    array_len = array_len - totalLen;
  }
  /* Remove comma after last element */
  asicInfoStr[totalLen-1] = '\0';

  array_len = JSON_MAX_NODE_LENGTH -1;
  len = 0;
  totalLen = 0;
  if (pData->featureMask & INSIGHT_FEATURE_BST)
  {
    len = snprintf (&featureStr[totalLen], array_len,featureTemplate, "BST");
    totalLen += len;
    featureCount++;
    array_len = array_len - totalLen;
  }
  if (pData->featureMask & INSIGHT_FEATURE_PACKET_TRACE)
  {
    if (featureCount != 0)
    {
      len =  snprintf (&featureStr[totalLen], array_len, "%s", ",");
      totalLen += len;
    }
    len  = snprintf (&featureStr[totalLen], array_len, featureTemplate, "PT");
    totalLen += len;
    featureCount++;
    array_len = array_len - totalLen;
  }

  if (pData->featureMask & INSIGHT_FEATURE_LIVE_PT)
  {
    if (featureCount != 0)
    {
      len =  snprintf (&featureStr[totalLen], array_len, "%s", ",");
      totalLen += len;
    }
    len  = snprintf (&featureStr[totalLen], array_len, featureTemplate, "Live-PT");
    totalLen += len;
    featureCount++;
    array_len = array_len - totalLen;
  }

  if (pData->featureMask & INSIGHT_FEATURE_BHD)
  {
    if (featureCount != 0)
    {
      len =  snprintf (&featureStr[totalLen], array_len, "%s", ",");
      totalLen += len;
    }
    len  = snprintf (&featureStr[totalLen], array_len, featureTemplate, "BHD");
    totalLen += len;
    featureCount++;
    array_len = array_len - totalLen;
  }


  featureStr[totalLen] = '\0';

 if (0 == method)
 {
  /* encode the JSON */
  snprintf(jsonBuf, INSIGHT_JSON_MEMSIZE_RESPONSE, getSwitchPropAsyncTemplate,
      INSIGHT_JSON_VERSION, timeString, pData->numAsics, asicInfoStr, featureStr, pData->networkOs,
      pData->uid, pData->agent_ipaddr, pData->agent_port, pData->agent_sw_version);
 }
 else
 {
  /* encode the JSON */
  snprintf(jsonBuf, INSIGHT_JSON_MEMSIZE_RESPONSE, getSwitchPropTemplate,
      INSIGHT_JSON_VERSION, timeString, pData->numAsics, asicInfoStr, featureStr, pData->networkOs,
      pData->uid, pData->agent_ipaddr, pData->agent_port, pData->agent_sw_version, method);

 }


  /* setup the return value */
  *pJsonBuffer = (uint8 *) jsonBuf;


  LOG_POST (INSIGHT_LOG_DEBUG, "SYSTEM-JSON-Encoder : [%d bytes] , %s \n",(int)sal_strlen(jsonBuf), jsonBuf);


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
  * @note   : This function is invoked by the system_utils to send the report and
  *           requested get configure params. This function internally
  *           calls the encoding api to encode the data, and the memory
  *           for the data is allocated. In case of both successful and
  *           unsuccessful send of the data, the memory must be freed.
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_send_response (insight_system_response_msg_t * reply_data)
{
  INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;
  uint8 *pJsonBuffer = NULL;

  if (NULL == reply_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;

#if 0
  _SYSTEM_UTILS_LOG(_SYSTEM_UTILS_DEBUG_INFO, "Reply_data\n"
      "reply_data->msg_type = %ld\n"
      "reply_data->ret = %d\n"
      "reply_data->unit = %d\n"
      "reply_data->cookie = %d\n",
      reply_data->msg_type,reply_data->ret, reply_data->unit, (NULL == reply_data->cookie)? TRUE : FALSE);

#endif

  if (reply_data->ret != INSIGHT_STATUS_SUCCESS)
  {
    rest_response_send_error(reply_data->cookie, reply_data->ret, reply_data->id);
    return INSIGHT_STATUS_SUCCESS;
  }
  else
  {
    if ((INSIGHT_SYSTEM_CMD_API_SET_FEATURE == reply_data->msg_type) ||
        (INSIGHT_SYSTEM_CMD_API_CANCEL_REQUEST == reply_data->msg_type))
    {
      rest_response_send_ok (reply_data->cookie);
      return INSIGHT_STATUS_SUCCESS;
    }

  }

  /* Take lock*/
//  SYSTEM_UTILS_LOCK_TAKE (&pg_switch_info->mutex);
  switch (reply_data->msg_type)
  {
    case INSIGHT_SYSTEM_CMD_API_GET_FEATURE:
      /* call json encoder api for feature  */

      ret = insight_system_json_encode_get_feature (reply_data->id,
          &reply_data->config,
          &pJsonBuffer);
      break;
    case  INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES:
      /* call json encoder api for featute*/
      ret = insight_system_json_encode_get_switch_properties (reply_data->id,
          &reply_data->switch_properties,
          &pJsonBuffer);

      break;

    default:
      break;
  }

  if (NULL != pJsonBuffer && INSIGHT_STATUS_SUCCESS == ret)
  {
    ret = rest_response_send(reply_data->cookie, (char *)pJsonBuffer, sal_strlen((char *)pJsonBuffer));
    if (INSIGHT_STATUS_SUCCESS != ret)
    {

      LOG_POST (INSIGHT_LOG_ERROR,
          " sending response failed due to error = %d\r\n",ret);
    }
    else
    {

    }
    /* free the json buffer */
    if (NULL != pJsonBuffer)
    {
      mem_free(pJsonBuffer);
    }
  }
  else
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "encoding of system_utils response failed due to error = %d\r\n", ret);
    /* Can happen that memory is allocated,
       but the encoding failed.. in that case also
       free the json buffer.
     */
    if (NULL != pJsonBuffer)
    {
      mem_free(pJsonBuffer);
    }
  }
  /* release the lock for success and failed cases */
 // SYSTEM_UTILS_LOCK_GIVE(&pg_switch_info->mutex);
  return ret;
}


#define _INSIGNT_JSON_TO_APP_


INSIGHT_STATUS insight_system_send_request (insight_system_request_msg_t * msg_data)
{
  int ret = INSIGHT_STATUS_SUCCESS;
  struct mq_attr obuf; /* output attr struct for getattr */

  if (NULL == msg_data)
    return INSIGHT_STATUS_INVALID_PARAMETER;

  if (-1 == msgsnd (pg_switch_info->recvMsgQid, msg_data, sizeof (insight_system_request_msg_t), IPC_NOWAIT))
  {
    if ( ! mq_getattr(pg_switch_info->recvMsgQid,&obuf) )
    {
   //   _SYSTEM_UTILS_LOG(_SYSTEM_UTILS_DEBUG_TRACE, "flags: %ld  maxmsg: %ld  msgsize: %ld  curmsgs: %ld\n",
     //     obuf.mq_flags, obuf.mq_maxmsg, obuf.mq_msgsize, obuf.mq_curmsgs);
    }

  LOG_POST (INSIGHT_LOG_ERROR,
      "Failed to send message to system utility application,  msg_type  %ld, err = %d\r\n",
        msg_data->msg_type, errno);
    ret = INSIGHT_STATUS_FAILURE;
  }

  return ret;
}

INSIGHT_STATUS insight_system_json_get_switch_properties_impl (void *cookie,
                                                     int id,
                                                     void *pCommand)
{
  INSIGHT_STATUS ret;
  insight_system_request_msg_t msg_data;

  sal_memset (&msg_data, 0, sizeof (insight_system_request_msg_t));
  msg_data.cookie = cookie;
  msg_data.id = id;
  msg_data.msg_type = INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES;
  /* send message to system utils application */
  ret = insight_system_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "failed to post get switch properties to system utils queue. err = %d.\r\n",ret);
  }
  return ret;
}


/*********************************************************************
 * @brief : REST API handler to configure the system_utils feature params
 *
 * @param[in] cookie : pointer to the cookie
 * @param[in] id     : unit id
 * @param[in] pCommand : pointer to the input command structure
 *
 * @retval  : INSIGHT_STATUS_SUCCESS : the message is successfully posted to system_utils queue.
 * @retval  : INSIGHT_STATUS_FAILURE : failed to post the message to system_utils.
 * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
 *
 * @note    : This api posts the request to system_utils application to configures system_utils feature params.
 *
 * @end
 *********************************************************************/
INSIGHT_STATUS insight_system_json_configure_feature_impl (void *cookie,
    int id,
    insight_system_config_feature_params_t* pCommand)
{
  insight_system_request_msg_t msg_data;
  INSIGHT_STATUS ret;

  if (NULL == pCommand)
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }


  sal_memset (&msg_data, 0, sizeof (insight_system_request_msg_t));
  msg_data.cookie = cookie;
  msg_data.msg_type = INSIGHT_SYSTEM_CMD_API_SET_FEATURE;
  msg_data.id = id;
  sal_memcpy(&msg_data.request.config,pCommand,sizeof(insight_system_config_feature_params_t));

  /* send message to system_utils application */
  ret = insight_system_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
     LOG_POST (INSIGHT_LOG_ERROR,
        "failed to post configure system_utils feature to system_utils queue. err = %d.\r\n",ret);
  }
  return ret;
}


/*********************************************************************
 * @brief : REST API handler to get the system utils feature params
 *
 * @param[in] cookie : pointer to the cookie
 * @param[in] id     : unit id
 * @param[in] pCommand : pointer to the input command structure
 *
 * @retval  : INSIGHT_STATUS_SUCCESS : the message is successfully posted to system utils queue.
 * @retval  : INSIGHT_STATUS_FAILURE : failed to post the message to system utils.
 * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
 *
 * @note    : This api posts the request to system utils application to get the system utils feature params.
 *
 *********************************************************************/
INSIGHT_STATUS insight_system_json_get_feature_impl (void *cookie, int id,
    void * pCommand)
{
  insight_system_request_msg_t msg_data;
  INSIGHT_STATUS ret;

  sal_memset (&msg_data, 0, sizeof (insight_system_request_msg_t));
  msg_data.cookie = cookie;
  msg_data.msg_type = INSIGHT_SYSTEM_CMD_API_GET_FEATURE;
  msg_data.id = id;
  /* send message to system utils application */
  ret = insight_system_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "failed to post get system utils feature to system utils queue. err = %d.\r\n",ret);
  }
  return ret;
}

/*********************************************************************
  * @brief : REST API handler to cancel request
  *
  * @param[in] cookie : pointer to the cookie
  * @param[in] id     : unit id
  * @param[in] pCommand : pointer to the input command structure
  *
  * @retval  : INSIGHT_STATUS_SUCCESS : the message is successfully posted
  *            to sys utility queue.
  * @retval  : INSIGHT_STATUS_FAILURE : failed to post the message to application.
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameter.
  *
  * @note    : This api posts the request to application to get
  *            switch properties params.
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_json_cancel_request_impl (void *cookie,
                                                     int asicId,
                                                     int id,
                                                     insight_system_cancel_request_t *pCommand)
{
  INSIGHT_STATUS ret;
  insight_system_request_msg_t msg_data;

  sal_memset (&msg_data, 0, sizeof (insight_system_request_msg_t));
  msg_data.cookie = cookie;
  msg_data.unit = asicId;
  msg_data.id = id;
  msg_data.msg_type = INSIGHT_SYSTEM_CMD_API_CANCEL_REQUEST;
  sal_memcpy(&msg_data.request.cancel ,pCommand,sizeof(insight_system_cancel_request_t));
  /* send message to system utils application */
  ret = insight_system_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "failed to post get system utils feature to system utils queue. err = %d.\r\n",ret);
  }
  return ret;


  return INSIGHT_STATUS_SUCCESS;
}




INSIGHT_STATUS insight_system_json_configure_feature (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method;
    cJSON *json_id, *json_heartbeatEnable, *json_interval;
    cJSON *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[JSON_MAX_NODE_LENGTH] = {0};
    char method[JSON_MAX_NODE_LENGTH] = {0};
    int  id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    insight_system_config_feature_params_t command;

    sal_memset(&command, 0, sizeof (command));

    /* Validating input parameters */

    /* Validating 'cookie' */
    JSON_VALIDATE_POINTER(cookie, "cookie", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'jsonBuffer' */
    JSON_VALIDATE_POINTER(jsonBuffer, "jsonBuffer", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'bufLength' */
    if (bufLength > sal_strlen(jsonBuffer))
    {
        _jsonlog("Invalid value for parameter bufLength %d ", bufLength );
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    /* Parse JSON to a C-JSON root */
    root = cJSON_Parse(jsonBuffer);
    JSON_VALIDATE_JSON_POINTER(root, "root", INSIGHT_STATUS_INVALID_JSON);

    /* Obtain command parameters */
    params = cJSON_GetObjectItem(root, "params");
    JSON_VALIDATE_JSON_POINTER(params, "params", INSIGHT_STATUS_INVALID_JSON);

    /* Parsing and Validating 'jsonrpc' from JSON buffer */
    json_jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    JSON_VALIDATE_JSON_POINTER(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "configure-bst-feature" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "configure-system-feature");


    /* Parsing and Validating 'id' from JSON buffer */
    json_id = cJSON_GetObjectItem(root, "id");
    JSON_VALIDATE_JSON_POINTER(json_id, "id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_id, "id");
    /* Copy the value */
    id = json_id->valueint;
    /* Ensure  that the number 'id' is within range of [1,100000] */
    JSON_CHECK_VALUE_AND_CLEANUP (id, 1, 100000);

    /* Parsing and Validating 'heartbeat-enable' from JSON buffer */
    json_heartbeatEnable = cJSON_GetObjectItem(params, "heartbeat-enable");
    JSON_VALIDATE_JSON_POINTER(json_heartbeatEnable, "heartbeat-enable", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_heartbeatEnable, "heartbeat-enable");
    /* Copy the value */
    command.enable = json_heartbeatEnable->valueint;
    /* Ensure  that the number 'bst-enable' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.enable, 0, 1);
    command.configMask = (command.configMask | (1 << 0));

    /* Parsing and Validating 'msg-interval' from JSON buffer */
    json_interval = cJSON_GetObjectItem(params, "msg-interval");
    JSON_VALIDATE_JSON_POINTER(json_interval, "msg-interval", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_interval, "msg-interval");
    /* Copy the value */
    command.interval = json_interval->valueint;
    /* Ensure  that the number 'collection-interval' is within range of [0,600] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.interval, 0, 600);
    command.configMask = (command.configMask | (1 << 1));

    /* Send the 'command' along with 'cookie' to the Application thread. */
    status = insight_system_json_configure_feature_impl (cookie, id, &command);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

INSIGHT_STATUS insight_system_json_get_feature (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[JSON_MAX_NODE_LENGTH] = {0};
    char method[JSON_MAX_NODE_LENGTH] = {0};
    int id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;


    /*sal_memset commented since the structure is empty*/
    /* sal_memset(&command, 0, sizeof (command));*/

    /* Validating input parameters */

    /* Validating 'cookie' */
    JSON_VALIDATE_POINTER(cookie, "cookie", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'jsonBuffer' */
    JSON_VALIDATE_POINTER(jsonBuffer, "jsonBuffer", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'bufLength' */
    if (bufLength > sal_strlen(jsonBuffer))
    {
        _jsonlog("Invalid value for parameter bufLength %d ", bufLength );
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    /* Parse JSON to a C-JSON root */
    root = cJSON_Parse(jsonBuffer);
    JSON_VALIDATE_JSON_POINTER(root, "root", INSIGHT_STATUS_INVALID_JSON);

    /* Obtain command parameters */
    params = cJSON_GetObjectItem(root, "params");
    JSON_VALIDATE_JSON_POINTER(params, "params", INSIGHT_STATUS_INVALID_JSON);

    /* Parsing and Validating 'jsonrpc' from JSON buffer */
    json_jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    JSON_VALIDATE_JSON_POINTER(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-feature" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-system-feature");


    /* Parsing and Validating 'id' from JSON buffer */
    json_id = cJSON_GetObjectItem(root, "id");
    JSON_VALIDATE_JSON_POINTER(json_id, "id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_id, "id");
    /* Copy the value */
    id = json_id->valueint;
    /* Ensure  that the number 'id' is within range of [1,100000] */
    JSON_CHECK_VALUE_AND_CLEANUP (id, 1, 100000);


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = insight_system_json_get_feature_impl (cookie, id, NULL);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}


INSIGHT_STATUS insight_system_json_get_switch_properties (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[JSON_MAX_NODE_LENGTH] = {0};
    char method[JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;


    /*sal_memset commented since the structure is empty*/
    /* sal_memset(&command, 0, sizeof (command));*/

    /* Validating input parameters */

    /* Validating 'cookie' */
    JSON_VALIDATE_POINTER(cookie, "cookie", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'jsonBuffer' */
    JSON_VALIDATE_POINTER(jsonBuffer, "jsonBuffer", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'bufLength' */
    if (bufLength > sal_strlen(jsonBuffer))
    {
        _jsonlog("Invalid value for parameter bufLength %d ", bufLength );
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    /* Parse JSON to a C-JSON root */
    root = cJSON_Parse(jsonBuffer);
    JSON_VALIDATE_JSON_POINTER(root, "root", INSIGHT_STATUS_INVALID_JSON);

    /* Obtain command parameters */
    params = cJSON_GetObjectItem(root, "params");
    JSON_VALIDATE_JSON_POINTER(params, "params", INSIGHT_STATUS_INVALID_JSON);

    /* Parsing and Validating 'jsonrpc' from JSON buffer */
    json_jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    JSON_VALIDATE_JSON_POINTER(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-switch-properties" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-switch-properties");

    /* Parsing and Validating 'asic-id' from JSON buffer */
    json_asicId = cJSON_GetObjectItem(root, "asic-id");
   if (NULL != json_asicId)
   {
     JSON_VALIDATE_JSON_POINTER(json_asicId, "asic-id", INSIGHT_STATUS_INVALID_JSON);
     JSON_VALIDATE_JSON_AS_STRING(json_asicId, "asic-id", INSIGHT_STATUS_INVALID_JSON);
     /* Copy the 'asic-id' in external notation to our internal representation */
     JSON_ASIC_ID_MAP_FROM_NOTATION(asicId, json_asicId->valuestring);

     if (0 > asicId)
     {
       return INSIGHT_STATUS_INVALID_PARAMETER;
     }
   }

    /* Parsing and Validating 'id' from JSON buffer */
    json_id = cJSON_GetObjectItem(root, "id");
    JSON_VALIDATE_JSON_POINTER(json_id, "id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_id, "id");
    /* Copy the value */
    id = json_id->valueint;
    /* Ensure  that the number 'id' is within range of [1,100000] */
    JSON_CHECK_VALUE_AND_CLEANUP (id, 1, 100000);


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
   status = insight_system_json_get_switch_properties_impl (cookie, id, NULL);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

INSIGHT_STATUS insight_system_json_cancel_request (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method;
    cJSON *json_id, *json_req_id, *json_asicId;
    cJSON *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[JSON_MAX_NODE_LENGTH] = {0};
    char method[JSON_MAX_NODE_LENGTH] = {0};
    int  id = 0, asicId =0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    insight_system_cancel_request_t command;

    sal_memset(&command, 0, sizeof (command));

    /* Validating input parameters */

    /* Validating 'cookie' */
    JSON_VALIDATE_POINTER(cookie, "cookie", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'jsonBuffer' */
    JSON_VALIDATE_POINTER(jsonBuffer, "jsonBuffer", INSIGHT_STATUS_INVALID_PARAMETER);

    /* Validating 'bufLength' */
    if (bufLength > sal_strlen(jsonBuffer))
    {
        _jsonlog("Invalid value for parameter bufLength %d ", bufLength );
        return INSIGHT_STATUS_INVALID_PARAMETER;
    }

    /* Parse JSON to a C-JSON root */
    root = cJSON_Parse(jsonBuffer);
    JSON_VALIDATE_JSON_POINTER(root, "root", INSIGHT_STATUS_INVALID_JSON);

    /* Obtain command parameters */
    params = cJSON_GetObjectItem(root, "params");
    JSON_VALIDATE_JSON_POINTER(params, "params", INSIGHT_STATUS_INVALID_JSON);

    /* Parsing and Validating 'jsonrpc' from JSON buffer */
    json_jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    JSON_VALIDATE_JSON_POINTER(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_jsonrpc, "jsonrpc", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "configure-bst-feature" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "cancel-request");

    /* Parsing and Validating 'asic-id' from JSON buffer */
    json_asicId = cJSON_GetObjectItem(root, "asic-id");
    JSON_VALIDATE_JSON_POINTER(json_asicId, "asic-id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_asicId, "asic-id", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the 'asic-id' in external notation to our internal representation */
    JSON_ASIC_ID_MAP_FROM_NOTATION(asicId, json_asicId->valuestring);



    /* Parsing and Validating 'id' from JSON buffer */
    json_id = cJSON_GetObjectItem(root, "id");
    JSON_VALIDATE_JSON_POINTER(json_id, "id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_id, "id");
    /* Copy the value */
    id = json_id->valueint;
    /* Ensure  that the number 'id' is within range of [1,100000] */
    JSON_CHECK_VALUE_AND_CLEANUP (id, 1, 100000);

    /* Parsing and Validating 'heartbeat-enable' from JSON buffer */
    json_req_id = cJSON_GetObjectItem(params, "request-id");
    JSON_VALIDATE_JSON_POINTER(json_req_id, "request-id", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_req_id, "request-id");
    /* Copy the value */
    command.id = json_req_id->valueint;
    status = insight_system_json_cancel_request_impl (cookie, asicId, id, &command);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

/*********************************************************************
  * @brief   :  function to post message to the system utility application
  *
  * @param[in]  msg_data : pointer to the message request
  *
  * @retval  : INSIGHT_STATUS_SUCCESS : if the message is successfully posted to queue.
  * @retval  : INSIGHT_STATUS_FAILURE : if the message is failed to send to application
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
  *
  * @note  : all the apis and call back functions should use this api
  *          to post the message to application.
  *
  *********************************************************************/

#define _INSIGHT_APP_PROC_



/*********************************************************************
  *  @brief:  timer callback function to send heart beat messages
  *
  * @param[in]   sigval : Data passed with notification after timer expires
  *
  * @retval  : INSIGHT_STATUS_SUCCESS : message is successfully posted to system_utils.
  * @retval  : INSIGHT_STATUS_FAILURE : failed to post message to system_utils.
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : invalid parameters to function.
  *
  * @note : when the periodic collection timer expires, this api is invoked in
  *         the timer context. We just post the request for the report, while
  *        posting we mark the report type as periodic. The parameter returned
  *       from the callback is the reference to unit id.
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_timer_cb (union sigval sigval)
{
  insight_system_request_msg_t msg_data;
  INSIGHT_STATUS ret;

  sal_memset(&msg_data, 0, sizeof(insight_system_request_msg_t));
  msg_data.report_type = 1;
  msg_data.msg_type = INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES;
  /* Send the message to the system utility application */
  ret = insight_system_send_request (&msg_data);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "Failed to send heart beat timer expiry message to application. err = %d\r\n", ret);
    return INSIGHT_STATUS_FAILURE;
  }
  return INSIGHT_STATUS_SUCCESS;
}


/*********************************************************************
  * @brief : Deletes the timer node for the given unit
  *
  * @param[in] unit : unit id for which  the timer needs to be deleted.
  *
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER -- Inpput paramerts are invalid.
  * @retval  : INSIGHT_STATUS_FAILURE -- timer is successfully deleted
  * @retval  : INSIGHT_STATUS_SUCCESS -- failed to delete the timer
  *
  * @note  : The heart beat timer is deleted when send feature is turned off
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_stop_heartbeat_timer (void)
{

  INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;



  if (TRUE == pg_switch_info->hb_inuse)
  {
    ret = system_timer_delete (pg_switch_info->hb_timer);
    if (INSIGHT_STATUS_SUCCESS == ret)
    {
    pg_switch_info->hb_inuse = FALSE;
      LOG_POST (INSIGHT_LOG_INFO,
          "Registration and heart beat: successfully deleted heart  beat timer \r\n");
    }
    else
    {
      /* timer node add has failed. log the same */
      LOG_POST (INSIGHT_LOG_ERROR,
          "Failed to delete heart beat timer for err %d \r\n", ret);
    }
  }

  return ret;
}


/*********************************************************************
  * @brief : function to add timer for the periodic heartbeat
  *
  * @param[in] unit : unit for which the heart beat need to be advertized.
  *
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER -- Inpput paramerts are invalid.
  * @retval  : INSIGHT_STATUS_FAILURE -- failed to add the timer
  * @retval  : INSIGHT_STATUS_SUCCESS -- timer is successfully added
  *
  * @note : this api adds the timer to the linux timer thread, so when the timer
  *         expires, we receive the callback and post message to the application.
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_start_heartbeat_timer (void)
{

  INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;


  if (TRUE == pg_switch_info->hb_inuse)
  {
    /* the timer is in use. The requester has asked
       to add the timer again.. Remove the old entru
       and add it again.. Reasosn could be that config
       interval would have been changed, In such case,
       delete the one with previous collection timer
       interval and add the new one */
    ret =  insight_system_stop_heartbeat_timer();
    if (INSIGHT_STATUS_SUCCESS != ret)
    {
      /* timer node add has failed. log the same */
      LOG_POST (INSIGHT_LOG_ERROR,
          "%s Failed to delete heart beat timer err %d \r\n", __func__, ret);
    }
  }

  /* The timer add function expects the time in milli seconds..
     so convert the time into milli seconds. , before adding
     the timer node */
  ret =  system_timer_add (insight_system_timer_cb,
      &pg_switch_info->hb_timer,pg_switch_info->hb_interval*1000000,  1, 0);

  if (INSIGHT_STATUS_SUCCESS == ret)
  {
     pg_switch_info->hb_inuse = TRUE;
      LOG_POST (INSIGHT_LOG_INFO,
        "registration and heart beat: heart beat timer is successfully started.\r\n");
  }
  else
  {
    /* timer node add has failed. log the same */
    LOG_POST (INSIGHT_LOG_ERROR,
        "Failed to add heart beat timer. err %d \r\n", ret);
  }
  return ret;
}


INSIGHT_STATUS insight_system_set_config_feature (void * data)
{
  INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;
  insight_system_request_msg_t *msg_data = (insight_system_request_msg_t*) data;
  int tmpMask = 0;
  bool modifyTimer = FALSE;

  /* check for the null of the input pointer */
  if (NULL == msg_data)
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }

  /* collection interval is maintained in seconds in application.
     while adding the timer, the same should be converted into
     milli seconds and added as the timer api expects
     interval in milli seconds. */
  tmpMask = msg_data->request.config.configMask;

  if ((tmpMask & (1 << 0)) &&
      (pg_switch_info->hb_enable != msg_data->request.config.enable))
  {
    pg_switch_info->hb_enable = msg_data->request.config.enable;
     modifyTimer = TRUE;
  }

  if ((tmpMask & (1 << 1)) &&
      (pg_switch_info->hb_interval != msg_data->request.config.interval))
  {
    pg_switch_info->hb_interval  = msg_data->request.config.interval;

    if (0 != pg_switch_info->hb_interval)
    {
      modifyTimer = TRUE;
    }
  }

  /* stop timer when the feature is disabled or when the timer is modified */

  if ((FALSE == pg_switch_info->hb_enable) || (0 == pg_switch_info->hb_interval))
  {
    insight_system_stop_heartbeat_timer();
  }

  /* restart only if the timer is modified and the feature is enabled */
   if ((TRUE == modifyTimer ) && ((TRUE ==pg_switch_info->hb_enable) &&  (0 != pg_switch_info->hb_interval)))
   {
     insight_system_start_heartbeat_timer();
   }

  return ret;
}


/*********************************************************************
  * @brief : application function to get switch properties
  *
  * @param[in] msg_data : pointer to the message request.
  *
  * @retval  : INSIGHT_STATUS_INVALID_PARAMETER : Inpput paramerts are invalid.
  * @retval  : INSIGHT_STATUS_SUCCESS  : successfully retrieved the switch
  *                                    properties.
  * @note
  *
  *********************************************************************/
INSIGHT_STATUS  insight_system_get_func (void * msg_data)
{

  if (NULL == msg_data)
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }

  return INSIGHT_STATUS_SUCCESS;
}


INSIGHT_STATUS insight_system_set_cancel_request(void * data)
{
 insight_system_request_msg_t *msg_data = (insight_system_request_msg_t*) data;

  if (NULL == msg_data)
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }

  unsigned int i = 0;
  INSIGHT_STATUS ret = INSIGHT_STATUS_NOT_CONFIGURED;
  unsigned int numOfEle;

  numOfEle = sizeof(pg_switch_info->cancel_api_list)/sizeof(insight_system_cancel_api_t);

  //SYSTEM_UTILS_LOCK_TAKE (&pg_switch_info->mutex);
  for (i = 0; i < numOfEle; i++)
  {
    if ((pg_switch_info->cancel_api_list[i].present == TRUE) &&
        (NULL != pg_switch_info->cancel_api_list[i].handler))
    {
      ret = pg_switch_info->cancel_api_list[i].handler(msg_data->unit,  msg_data->request.cancel.id);
      if (ret != INSIGHT_STATUS_NOT_CONFIGURED)
      {
        /* Id cancel is handled by handler, return value is in ret*/
        break;
      }
    }
  }
 // SYSTEM_UTILS_LOCK_GIVE (&pg_switch_info->mutex);
  return ret;

}


INSIGHT_STATUS insight_system_reply_params (insight_system_request_msg_t * msg_data,
                                insight_system_response_msg_t * reply_data)
{

  if ((NULL == msg_data) || (NULL == reply_data))
    return INSIGHT_STATUS_INVALID_PARAMETER;

  /* copy the request type */
  reply_data->msg_type = msg_data->msg_type;
  /* copy the unit */
  reply_data->unit = msg_data->unit;
  /* copy the asic type */
  reply_data->id = msg_data->id;
  /* copy the cookie ..  */
  reply_data->cookie = msg_data->cookie;

  switch (msg_data->msg_type)
  {
    case INSIGHT_SYSTEM_CMD_API_GET_FEATURE:
      reply_data->config.enable = pg_switch_info->hb_enable;
	reply_data->config.interval =  pg_switch_info->hb_interval;
	break;

    case INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES:
      sal_memcpy(&reply_data->switch_properties, &pg_switch_info->switch_properties,sizeof(insight_system_properties_t));

      break;
    default:
      break;
  }
  /* release the lock for success and failed cases */

  return insight_system_send_response(reply_data);
}



static INSIGHT_REST_API_t switch_api_list[] = {

  {"configure-system-feature", insight_system_json_configure_feature},
  {"get-system-feature", insight_system_json_get_feature},
  {"get-switch-properties", insight_system_json_get_switch_properties},
  {"cancel-request", insight_system_json_cancel_request}
};

/*********************************************************************
 * @brief : function to return the api handler for the system command type
 *
 * @param[in] int : request type
 *
 * @retval    : function pointer to the corresponding api
 *
 * @note :
 *
 *********************************************************************/
INSIGHT_STATUS insight_system_type_api_get (int type, INSIGHT_APP_API_HANDLER_t *handler)
{
  unsigned int i = 0;

  static const insight_app_api_t insight_system_cmd_api_list[] = {
    {INSIGHT_SYSTEM_CMD_API_GET_FEATURE, insight_system_get_func},
    {INSIGHT_SYSTEM_CMD_API_SET_FEATURE, insight_system_set_config_feature},
    {INSIGHT_SYSTEM_CMD_API_GET_SWITCH_PROPERTIES, insight_system_get_func},
    {INSIGHT_SYSTEM_CMD_API_CANCEL_REQUEST, insight_system_set_cancel_request}
  };

  for (i = 0; i < (sizeof(insight_system_cmd_api_list)/sizeof(insight_system_api_t)); i++)
  {
    if (type == insight_system_cmd_api_list[i].command)
    {
      *handler = insight_system_cmd_api_list[i].handler;
      return INSIGHT_STATUS_SUCCESS;
    }
  }
  return INSIGHT_STATUS_INVALID_PARAMETER;
}




/*********************************************************************
  * @brief :  function to register with module mgr
  *
  * @param[in] : none
  *
  * @retval  : INSIGHT_STATUS_SUCCESS : registration of application with module manager is successful.
  * @retval  : INSIGHT_STATUS_FAILURE : Application failed to register with module manager.
  *
  * @note : The utility application need to register with module manager for the below purpose.
  *         When the REST API is invoked, rest queries the module manager for
  *         the suitable function api  for the corresponding request. Once the
  *         api is retieved , posts the request using the retrieved api.
  *
  * @end
  *********************************************************************/
INSIGHT_STATUS insight_system_module_register ()
{
  insight_module_feature_info_t systemUtilsInfo;
  INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;

  sal_memset (&systemUtilsInfo, 0, sizeof (insight_module_feature_info_t));


   systemUtilsInfo.featureName[0] = '\0';
  systemUtilsInfo.featureId = INSIGHT_FEATURE_SYSTEM;
  sal_memcpy (systemUtilsInfo.restApiList, switch_api_list,  sizeof(switch_api_list));

  /* Register with module manager. */
  ret = modulemgr_register (&systemUtilsInfo);
  if (INSIGHT_STATUS_SUCCESS != ret)
  {
    LOG_POST (INSIGHT_LOG_ERROR,
        "system utility application failed to register with module mgr\r\n");
  }
  else
  {
    LOG_POST (INSIGHT_LOG_INFO,
        "module mgr registration for system utility successful \r\n");
  }
  return ret;
}

INSIGHT_STATUS insight_system_thread_handler (void)
{
  insight_system_request_msg_t msg_data;
  insight_system_response_msg_t reply_data;
  INSIGHT_STATUS ret = INSIGHT_STATUS_FAILURE;
  unsigned int rcvd_err = 0;
 // unsigned int id = 0, num_units = 0;
  INSIGHT_APP_API_HANDLER_t handler;
  //insight_system_properties_t  *pswitchProp = &pg_switch_info->switch_properties;
//  struct sockaddr_in agent_ip;
 // uint8 num_units = 1;

 if (INSIGHT_STATUS_SUCCESS != insight_system_module_register ())
 {
     /* registration with module mgr has failed.
     return failure. so that the caller can clean the resources */
     LOG_POST (INSIGHT_LOG_EMERGENCY,  "Registration with module mgr failed \r\n");

     return INSIGHT_STATUS_FAILURE;
 }

  if ((TRUE == pg_switch_info->hb_enable) && (0 != pg_switch_info->hb_interval))
  {
      /* start the heart beat timer */
      insight_system_start_heartbeat_timer();
  }

  while (1)
  {
    if (-1 != (msgrcv (pg_switch_info->recvMsgQid, &msg_data, sizeof (msg_data), 0, 0)))
    {
      LOG_POST(INSIGHT_LOG_INFO, "msg_data info\n"
          "msg_data.msg_type = %ld\n"
          "msg_data.unit = %d\n"
          "msg_data.cookie = %d\n",
          msg_data.msg_type, msg_data.unit, (NULL == msg_data.cookie)? TRUE: FALSE);


      if (msg_data.unit >= 1)
      {
        /* requested id is greater than the number of units.
           send failure message and continue */
        sal_memset (&reply_data, 0, sizeof (insight_system_response_msg_t));
        reply_data.ret = INSIGHT_STATUS_INVALID_PARAMETER;


        /* copy the request type */
        reply_data.msg_type = msg_data.msg_type;
        /* copy the unit */
        reply_data.unit = msg_data.unit;
        /* copy the asic type */
        reply_data.id = msg_data.id;
        /* copy the cookie ..  */
        reply_data.cookie = msg_data.cookie;

        ret = insight_system_send_response(&reply_data);
        if (INSIGHT_STATUS_SUCCESS != ret)
        {
          LOG_POST (INSIGHT_LOG_ERROR,
              "failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, ret);
        }
        continue;
      }
      /* Counter to check for read errors..
         successfully read the message. clear this to 0*/
      rcvd_err = 0;
      /* Memset the response message */
      sal_memset (&reply_data, 0, sizeof (insight_system_response_msg_t));

      /* get the api function for the method type */
      if (INSIGHT_STATUS_SUCCESS != insight_system_type_api_get (msg_data.msg_type, &handler))
      {
        continue;
      }
      ret = handler(&msg_data);

      reply_data.ret = ret;
      ret = insight_system_reply_params (&msg_data, &reply_data);
      if (INSIGHT_STATUS_SUCCESS != ret)
      {

        LOG_POST (INSIGHT_LOG_ERROR,
            "failed to send response for command %ld , err = %d. \r\n", msg_data.msg_type, ret);
      }
    }
    else
    {
      LOG_POST (INSIGHT_LOG_ERROR,
          "Failed to read message from the system utility application queue. err = %d\r\n", errno);
      /* increment the error counter.
         If this counter increments continously exit and return failure
         so that corrective action can be taken */
      rcvd_err++;

      /* wait till 10 consective messages */
      if(100 < rcvd_err)
      {
        break;
      }
      continue;
    }
  }                             /* while (1) */

  pthread_exit(&pg_switch_info->rx_thread);
  return INSIGHT_STATUS_FAILURE;
}

#define _INSIGHT_API_

/*********************************************************************
 * @brief : Register a feature's cancel api handler for the system
 *                   cancel command type
 *
 * @param[in] int : feature id
 * @param[in] INSIGHT_SYSTEM_UTILS_CANCEL_API_HANDLER_t : Feature's Handler
 *                                        function for cancel command
 *
 * @retval  : INSIGHT_STATUS_SUCCESS : when the function is successfully registered
 * @retval  : INSIGHT_STATUS_ALREADY_CONFIGURED: when the hanlder is already configured
 *                    for the feature
 * @retval  : INSIGHT_STATUS_FAILURE : when failed to register the handler.
 *
 * @note :
 *
 *********************************************************************/
INSIGHT_STATUS insight_system_register_cancel_request(unsigned int featId,
                                                INSIGHT_CANCEL_API_HANDLER_t handler)
{
   unsigned int numOfEle, i = 0;
   INSIGHT_STATUS ret = INSIGHT_STATUS_SUCCESS;
   unsigned int freeIndex = 0;
   bool freeIndexFound =  FALSE;

   numOfEle =
         sizeof(pg_switch_info->cancel_api_list)/sizeof(insight_system_cancel_api_t);

   //SYSTEM_UTILS_LOCK_TAKE (&pg_switch_info->mutex);
   for(i = 0; i < numOfEle; i++)
   {
     if ((pg_switch_info->cancel_api_list[i].present == FALSE) &&
         (FALSE == freeIndexFound))
     {
        freeIndex = i;
        freeIndexFound = TRUE;
     }
     if (pg_switch_info->cancel_api_list[i].module == featId)
     {
        ret = INSIGHT_STATUS_ALREADY_CONFIGURED;
        break;
     }
   }
   if (ret != INSIGHT_STATUS_ALREADY_CONFIGURED)
   {
      if (freeIndexFound == TRUE)
      {
        pg_switch_info->cancel_api_list[freeIndex].module = featId;
        pg_switch_info->cancel_api_list[freeIndex].handler = handler;
        pg_switch_info->cancel_api_list[freeIndex].present = TRUE;
        ret = INSIGHT_STATUS_SUCCESS;
      }
      else
      {
        ret = INSIGHT_STATUS_TABLE_FULL;
      }
   }
   //SYSTEM_UTILS_LOCK_GIVE (&pg_switch_info->mutex);
   return ret;
}


/*********************************************************************
  * @brief : function to create the system utility application thread.
  *
  * @param[in]  none
  *
  * @retval  : INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE : Returns when memory is not available
  * @retval  : INSIGHT_STATUS_FAILURE : failed to create the message queue or thread.
  * @retval  : INSIGHT_STATUS_SUCCESS : successful completion of task creation.
  * @retval  : INSIGHT_STATUS_INIT_FAILED : application initialization failed.
  *
  * @note : Main function to invoke the  application thread. This function allocates
  *         the required memory for application.
  *         Creates the mutexes to protect data. Creates the message queue
  *         It assumes that the system is initialized before the application
  *         thread is invoked. This api depends on the system variables
  *         such as number of units per device. Incase of no such
  *         need, the number of units can be #defined.
  *
  *********************************************************************/
INSIGHT_STATUS insight_system_init ()
{
  int ret = INSIGHT_STATUS_SUCCESS;
  int recvMsgQid;
  //int num_units = 0;


  /* allocate memory for switch properties*/
  pg_switch_info =  (insight_system_info_t *) mem_malloc (MEM_APP_MODULE, sizeof (insight_system_info_t));
  if (pg_switch_info == NULL)
  {
      LOG_POST (INSIGHT_LOG_EMERGENCY,
                "Failed to allocate memory for system utility application\r\n");
      return INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE;
  }

  pg_switch_info->key1 = INSIGHT_MSG_QUEUE_ID_TO_SYSTEM;
  pthread_mutex_init (&pg_switch_info->mutex, NULL);

//  num_units = 1;
  pg_switch_info->hb_enable = 1;
  ;
  pg_switch_info->hb_interval = 5;

  //ctc_sal_time(&pg_switch_info->switch_properties.timeStamp);
  pg_switch_info->switch_properties.numAsics = 1;
  pg_switch_info->switch_properties.uid[0] = 0xFF;
  pg_switch_info->switch_properties.uid[127] = 0xFe;
  pg_switch_info->switch_properties.asicInfo[0].numPorts = 48;
  pg_switch_info->switch_properties.asicInfo[0].asicType = 1;
  sal_strcpy(&pg_switch_info->switch_properties.asicInfo[0].asic_notation[0], "1");




  sal_memcpy(&pg_switch_info->switch_properties.networkOs, "openNSL", sizeof("openNSL"));

  sal_memcpy(&pg_switch_info->switch_properties.uid, "0000d80bb99bbbbb", sizeof("0000d80bb99bbbbb"));
  sal_memcpy(&pg_switch_info->switch_properties.agent_sw_version, "V3.2.1", sizeof("V3.2.1"));

  sal_memcpy(&pg_switch_info->switch_properties.agent_ipaddr, "10.10.25.29", sizeof("10.10.25.29"));

  pg_switch_info->switch_properties.featureMask = INSIGHT_FEATURE_SYSTEM | INSIGHT_FEATURE_BST | INSIGHT_FEATURE_PACKET_TRACE;
  //sal_strcpy(pg_switch_info->switch_properties .agent_ipaddr, "10.1025.29");
  pg_switch_info->switch_properties.agent_port = SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT;

  /* Check if message queue is already exitsing */
  if (0 > (recvMsgQid = msgget(pg_switch_info->key1, IPC_EXCL | IPC_CREAT | 0777)))
  {
    if (EEXIST == errno)
    {

      /* get the already existing  message queue id for application */
      if ((recvMsgQid = msgget (pg_switch_info->key1, IPC_CREAT | 0777)) < 0)
      {
        /* Free the resources allocated so far */

        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "Failed to create  msgQ for system utility application. err = %d\n", errno);
        return  INSIGHT_STATUS_FAILURE;
      }

      /* message queue exits.. Delete the message queue */
      if  (msgctl(recvMsgQid, IPC_RMID, (struct msqid_ds *)NULL) <0)
      {
        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "Failed to destroy system_utils msgQ id, err = %d\n", errno);
      }


      /* create the message queue for application */
      if ((recvMsgQid = msgget (pg_switch_info->key1, IPC_CREAT | 0777)) < 0)
      {
        /* Free the resources allocated so far */
      //  insight_system_deinit ();

        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "Failed to create  msgQ for system_utils application. err = %d\n", errno);
        return  INSIGHT_STATUS_FAILURE;
      }
    }
  }
  pg_switch_info->recvMsgQid = recvMsgQid;


  /* create pthread for system_utils application */
  if (0 != pthread_create (&pg_switch_info->rx_thread, NULL, (void *) &insight_system_thread_handler, NULL))
  {
    LOG_POST (INSIGHT_LOG_EMERGENCY, "SYSTEM_UTILS  thread creation failed %d\r\n", errno);
    return  INSIGHT_STATUS_FAILURE;
  }
  LOG_POST (INSIGHT_LOG_INFO,
      "system_utils application: system_utils pthread created\r\n");

  return ret;
}

/*********************************************************************
  *  @brief:  function to clean up  system utility application
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
void insight_system_deinit ()
{
  pg_switch_info->key1 =INSIGHT_MSG_QUEUE_ID_TO_SYSTEM;

  /*
     delete the timer.
   */
 // reg_hb_heartbeat_timer_stop ();


  pthread_mutex_destroy (&pg_switch_info->mutex);


  /* check if the message queue already exists.
     If yes, we should delete the same */
  if (-1 == (pg_switch_info->recvMsgQid = msgget(pg_switch_info->key1, IPC_EXCL | IPC_CREAT | 0777)))
  {
    if (EEXIST == errno)
    {
      /* delete the message queue  */
      if (msgctl(pg_switch_info->recvMsgQid, IPC_RMID, NULL) == -1)
      {
        LOG_POST (INSIGHT_LOG_ERROR,
            "Failed to destroy system utility msgQ id, err = %d\n", errno);
      }
    }
  }
      mem_free (pg_switch_info);
}


