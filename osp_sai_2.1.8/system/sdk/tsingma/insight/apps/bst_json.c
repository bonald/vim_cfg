#include "sal.h"

#include "cJSON.h"

#include "json.h"
#include "modulemgr.h"
#include "log_api.h"
#include "rest_api.h"

#include "insight.h"
#include "system.h"
#include "bst.h"
#include "bst_json.h"



#define INSIGHT_DECODE_JSON_TO_APP



#define BVIEW_ASIC_MAX_PORTS            130
#define BVIEW_ASIC_MAX_UC_QUEUES        4096
#define BVIEW_ASIC_MAX_UC_QUEUE_GROUPS  128
#define BVIEW_ASIC_MAX_MC_QUEUES        1040
#define BVIEW_ASIC_MAX_SERVICE_POOLS    4
#define BVIEW_ASIC_MAX_COMMON_POOLS     1
#define BVIEW_ASIC_MAX_CPU_QUEUES       8
#define BVIEW_ASIC_MAX_RQE_QUEUES       11
#define BVIEW_ASIC_MAX_RQE_QUEUE_POOLS  4
#define BVIEW_ASIC_MAX_PRIORITY_GROUPS  8


typedef struct _bst_realm_index_ {
  char *realm;
  char *index1;
  char *index2;
}BSTJSON_REALM_INDEX_t;


#define _JSONENCODE_ASSERT(op)
#define   BSTJSON_MEMSIZE_REPORT  (sizeof(INSIGHT_BST_ASIC_SNAPSHOT_DATA_t)+ (32*2048))


/* Utility Macros for parameter validation */
#define _JSONENCODE_ASSERT_ERROR(condition, errcode) do { \
    if (!(condition)) { \
        LOG_POST(INSIGHT_LOG_ERROR, \
                    "BST JSON Encoder (%s:%d) Invalid Input Parameter  \n", \
                    __func__, __LINE__); \
        return (errcode); \
    } \
} while(0)


#define _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actLen, dst, len, lenptr, format, args...) \
    do { \
        int xtemp = *(lenptr); \
        (actLen) = snprintf((dst), (len), format, ##args); \
        *(lenptr) = (xtemp) + (actLen); \
        if ( (len) == (actLen)) { \
            /* Out of buffer here */ \
            LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (%s:%d) Out of Json memory while encoding \n", __func__, __LINE__); \
            return INSIGHT_STATUS_OUTOFMEMORY; \
        } \
        (dst) += (actLen); \
        (len) -= (actLen); \
    } while(0)



/* structure to map realm to threshold type */
typedef struct _bst_json_string_value_ {
  /* req string */
  char *req_str;
  /* req  type*/
  uint32 value;
}bst_json_string_value_t;



uint64_t round_int( double r ) {
      return (r > 0.0) ? (r + 0.5) : (r - 0.5);
}


static INSIGHT_STATUS _insight_bst_get_realm_type (char *str, uint32 *type)
{
  unsigned int i = 0;

  const bst_json_string_value_t realm_threshold_map[] = {
    {"device", INSIGHT_BST_DEVICE_THRESHOLD},
    {"ingress-port-priority-group", INSIGHT_BST_INGRESS_PORT_PG_THRESHOLD},
    {"ingress-port-service-pool", INSIGHT_BST_INGRESS_PORT_SP_THRESHOLD},
    {"ingress-service-pool", INSIGHT_BST_INGRESS_SP_THRESHOLD},
    {"egress-port-service-pool", INSIGHT_BST_EGRESS_PORT_SP_THRESHOLD},
    {"egress-service-pool", INSIGHT_BST_EGRESS_SP_THRESHOLD},
    {"egress-uc-queue", INSIGHT_BST_EGRESS_UC_QUEUE_THRESHOLD},
    {"egress-uc-queue-group", INSIGHT_BST_EGRESS_UC_QUEUEGROUPS_THRESHOLD},
    {"egress-mc-queue", INSIGHT_BST_EGRESS_MC_QUEUE_THRESHOLD},
    {"egress-cpu-queue", INSIGHT_BST_EGRESS_CPU_QUEUE_THRESHOLD},
    {"egress-rqe-queue", INSIGHT_BST_EGRESS_RQE_QUEUE_THRESHOLD}
  };
  for (i = INSIGHT_BST_DEVICE_THRESHOLD; i <= INSIGHT_BST_DEVICE_THRESHOLD; i++)
  {
    if (0 == strcmp (str, realm_threshold_map[i-1].req_str))
    {
       *type = realm_threshold_map[i-1].value;
      return INSIGHT_STATUS_SUCCESS;
    }
  }

  return INSIGHT_STATUS_FAILURE;
}

#define CHK_PTR_NULL(ptr) if(NULL == ptr) {return INSIGHT_STATUS_INVALID_PARAMETER;}

INSIGHT_STATUS bstjson_send_msg_to_app (void *cookie, int asicId, int id,  void * pCommand,uint8 msg_type)
{
    INSIGHT_BST_REQUEST_MSG_t msg_data;
    INSIGHT_STATUS rv;


    sal_memset (&msg_data, 0, sizeof (INSIGHT_BST_REQUEST_MSG_t));
    msg_data.unit = asicId;
    msg_data.cookie = cookie;
    msg_data.msg_type = msg_type;
    msg_data.id = id;
    switch(msg_type)
    {
    case INSIGHT_BST_CMD_API_SET_TRACK:
        CHK_PTR_NULL(pCommand);
        sal_memcpy(&msg_data.request.track , pCommand, sizeof(BSTJSON_CONFIGURE_BST_TRACKING_t));
        break;
    case INSIGHT_BST_CMD_API_SET_FEATURE:
        CHK_PTR_NULL(pCommand);
        sal_memcpy(&msg_data.request.feature , pCommand, sizeof(BSTJSON_CONFIGURE_BST_FEATURE_t));
        break;
    case INSIGHT_BST_CMD_API_SET_THRESHOLD:
    {
        BSTJSON_CONFIGURE_BST_THRESHOLDS_t *p_threshold = (BSTJSON_CONFIGURE_BST_THRESHOLDS_t*)pCommand;

        CHK_PTR_NULL(pCommand);
        sal_memcpy (msg_data.realm, p_threshold->realm, INSIGHT_JSON_MAX_NODE_LENGTH);
        _insight_bst_get_realm_type (p_threshold->realm,  &msg_data.threshold_type);
        msg_data.threshold_config_mask = p_threshold->mask;
        sal_memcpy(&msg_data.request.threshold , p_threshold, sizeof(BSTJSON_CONFIGURE_BST_THRESHOLDS_t));
    }
    break;

    case INSIGHT_BST_CMD_API_GET_THRESHOLD:
    case INSIGHT_BST_CMD_API_GET_REPORT:
        CHK_PTR_NULL(pCommand);
        sal_memcpy(&msg_data.request.collect , pCommand, sizeof(BSTJSON_GET_BST_REPORT_t));

        break;
    case INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS:
        CHK_PTR_NULL(pCommand);
        sal_memcpy(&msg_data.request.drp_ctrs , pCommand, sizeof(BSTJSON_GET_BST_CGSN_DROP_CTRS_t));
        break;

    case INSIGHT_BST_CMD_API_GET_FEATURE:
    case INSIGHT_BST_CMD_API_GET_TRACK:
    case INSIGHT_BST_CMD_API_CLEAR_THRESHOLD:
    case INSIGHT_BST_CMD_API_CLEAR_STATS:
        break;
    default :
       return INSIGHT_STATUS_INVALID_PARAMETER;

    }
    /* send message to bst application */
    rv = insight_bst_send_request (&msg_data);
    if (INSIGHT_STATUS_SUCCESS != rv)
    {
        LOG_POST (INSIGHT_LOG_ERROR,
                  "failed to post get bst report to bst queue. err = %d.\r\n", rv);
    }
    return rv;
}


 /* Decode Json */

INSIGHT_STATUS insight_bst_json_configure_bst_tracking (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id, *json_trackPeakStats, *json_trackIngressPortPriorityGroup;
    cJSON *json_trackIngressPortServicePool, *json_trackIngressServicePool, *json_trackEgressPortServicePool;
    cJSON *json_trackEgressServicePool, *json_trackEgressUcQueue, *json_trackEgressUcQueueGroup;
    cJSON *json_trackEgressMcQueue, *json_trackEgressCpuQueue, *json_trackEgressRqeQueue;
    cJSON *json_trackDevice,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_CONFIGURE_BST_TRACKING_t command;

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
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "configure-bst-tracking" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "configure-bst-tracking");


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


    /* Parsing and Validating 'track-peak-stats' from JSON buffer */
    json_trackPeakStats = cJSON_GetObjectItem(params, "track-peak-stats");
    JSON_VALIDATE_JSON_POINTER(json_trackPeakStats, "track-peak-stats", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackPeakStats, "track-peak-stats");
    /* Copy the value */
    command.trackPeakStats = json_trackPeakStats->valueint;
    /* Ensure  that the number 'track-peak-stats' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackPeakStats, 0, 1);


    /* Parsing and Validating 'track-ingress-port-priority-group' from JSON buffer */
    json_trackIngressPortPriorityGroup = cJSON_GetObjectItem(params, "track-ingress-port-priority-group");
    JSON_VALIDATE_JSON_POINTER(json_trackIngressPortPriorityGroup, "track-ingress-port-priority-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackIngressPortPriorityGroup, "track-ingress-port-priority-group");
    /* Copy the value */
    command.trackIngressPortPriorityGroup = json_trackIngressPortPriorityGroup->valueint;
    /* Ensure  that the number 'track-ingress-port-priority-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackIngressPortPriorityGroup, 0, 1);


    /* Parsing and Validating 'track-ingress-port-service-pool' from JSON buffer */
    json_trackIngressPortServicePool = cJSON_GetObjectItem(params, "track-ingress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_trackIngressPortServicePool, "track-ingress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackIngressPortServicePool, "track-ingress-port-service-pool");
    /* Copy the value */
    command.trackIngressPortServicePool = json_trackIngressPortServicePool->valueint;
    /* Ensure  that the number 'track-ingress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackIngressPortServicePool, 0, 1);


    /* Parsing and Validating 'track-ingress-service-pool' from JSON buffer */
    json_trackIngressServicePool = cJSON_GetObjectItem(params, "track-ingress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_trackIngressServicePool, "track-ingress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackIngressServicePool, "track-ingress-service-pool");
    /* Copy the value */
    command.trackIngressServicePool = json_trackIngressServicePool->valueint;
    /* Ensure  that the number 'track-ingress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackIngressServicePool, 0, 1);


    /* Parsing and Validating 'track-egress-port-service-pool' from JSON buffer */
    json_trackEgressPortServicePool = cJSON_GetObjectItem(params, "track-egress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressPortServicePool, "track-egress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressPortServicePool, "track-egress-port-service-pool");
    /* Copy the value */
    command.trackEgressPortServicePool = json_trackEgressPortServicePool->valueint;
    /* Ensure  that the number 'track-egress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressPortServicePool, 0, 1);


    /* Parsing and Validating 'track-egress-service-pool' from JSON buffer */
    json_trackEgressServicePool = cJSON_GetObjectItem(params, "track-egress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressServicePool, "track-egress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressServicePool, "track-egress-service-pool");
    /* Copy the value */
    command.trackEgressServicePool = json_trackEgressServicePool->valueint;
    /* Ensure  that the number 'track-egress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressServicePool, 0, 1);


    /* Parsing and Validating 'track-egress-uc-queue' from JSON buffer */
    json_trackEgressUcQueue = cJSON_GetObjectItem(params, "track-egress-uc-queue");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressUcQueue, "track-egress-uc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressUcQueue, "track-egress-uc-queue");
    /* Copy the value */
    command.trackEgressUcQueue = json_trackEgressUcQueue->valueint;
    /* Ensure  that the number 'track-egress-uc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressUcQueue, 0, 1);


    /* Parsing and Validating 'track-egress-uc-queue-group' from JSON buffer */
    json_trackEgressUcQueueGroup = cJSON_GetObjectItem(params, "track-egress-uc-queue-group");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressUcQueueGroup, "track-egress-uc-queue-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressUcQueueGroup, "track-egress-uc-queue-group");
    /* Copy the value */
    command.trackEgressUcQueueGroup = json_trackEgressUcQueueGroup->valueint;
    /* Ensure  that the number 'track-egress-uc-queue-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressUcQueueGroup, 0, 1);


    /* Parsing and Validating 'track-egress-mc-queue' from JSON buffer */
    json_trackEgressMcQueue = cJSON_GetObjectItem(params, "track-egress-mc-queue");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressMcQueue, "track-egress-mc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressMcQueue, "track-egress-mc-queue");
    /* Copy the value */
    command.trackEgressMcQueue = json_trackEgressMcQueue->valueint;
    /* Ensure  that the number 'track-egress-mc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressMcQueue, 0, 1);


    /* Parsing and Validating 'track-egress-cpu-queue' from JSON buffer */
    json_trackEgressCpuQueue = cJSON_GetObjectItem(params, "track-egress-cpu-queue");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressCpuQueue, "track-egress-cpu-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressCpuQueue, "track-egress-cpu-queue");
    /* Copy the value */
    command.trackEgressCpuQueue = json_trackEgressCpuQueue->valueint;
    /* Ensure  that the number 'track-egress-cpu-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressCpuQueue, 0, 1);


    /* Parsing and Validating 'track-egress-rqe-queue' from JSON buffer */
    json_trackEgressRqeQueue = cJSON_GetObjectItem(params, "track-egress-rqe-queue");
    JSON_VALIDATE_JSON_POINTER(json_trackEgressRqeQueue, "track-egress-rqe-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackEgressRqeQueue, "track-egress-rqe-queue");
    /* Copy the value */
    command.trackEgressRqeQueue = json_trackEgressRqeQueue->valueint;
    /* Ensure  that the number 'track-egress-rqe-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackEgressRqeQueue, 0, 1);


    /* Parsing and Validating 'track-device' from JSON buffer */
    json_trackDevice = cJSON_GetObjectItem(params, "track-device");
    JSON_VALIDATE_JSON_POINTER(json_trackDevice, "track-device", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_trackDevice, "track-device");
    /* Copy the value */
    command.trackDevice = json_trackDevice->valueint;
    /* Ensure  that the number 'track-device' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.trackDevice, 0, 1);


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, &command,INSIGHT_BST_CMD_API_SET_TRACK);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

extern INSIGHT_STATUS
insight_bst_json_configure_bst_feature (void *cookie, char *jsonBuffer, int bufLength)
{
 /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id, *json_bstEnable, *json_sendAsyncReports;
    cJSON *json_collectionInterval, *json_statUnitsInCells,  *root, *params;
    cJSON *json_maxTriggerReports, *json_sendSnapshotTrigger,  *json_triggerTransmitInterval, *json_sendIncrementalReport;
    cJSON *json_statsInPercentage;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_CONFIGURE_BST_FEATURE_t command;

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
    sal_strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    sal_strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "configure-bst-feature" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "configure-bst-feature");


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

    /* Parsing and Validating 'bst-enable' from JSON buffer */
    json_bstEnable = cJSON_GetObjectItem(params, "bst-enable");
    JSON_VALIDATE_JSON_POINTER(json_bstEnable, "bst-enable", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_bstEnable, "bst-enable");
    /* Copy the value */
    command.bstEnable = json_bstEnable->valueint;
    /* Ensure  that the number 'bst-enable' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.bstEnable, 0, 1);
    command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_ENABLE));


    /* Parsing and Validating 'send-async-reports' from JSON buffer */
    json_sendAsyncReports = cJSON_GetObjectItem(params, "send-async-reports");
    JSON_VALIDATE_JSON_POINTER(json_sendAsyncReports, "send-async-reports", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_sendAsyncReports, "send-async-reports");
    /* Copy the value */
    command.sendAsyncReports = json_sendAsyncReports->valueint;
    /* Ensure  that the number 'send-async-reports' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.sendAsyncReports, 0, 1);
    command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_SND_ASYNC_REP));


    /* Parsing and Validating 'collection-interval' from JSON buffer */
    json_collectionInterval = cJSON_GetObjectItem(params, "collection-interval");
    JSON_VALIDATE_JSON_POINTER(json_collectionInterval, "collection-interval", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_collectionInterval, "collection-interval");
    /* Copy the value */
    command.collectionInterval = json_collectionInterval->valueint;
    /* Ensure  that the number 'collection-interval' is within range of [0,600] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.collectionInterval, 0, 600);
    command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_COLL_INTRVL));


    /* Parsing and Validating 'stat-units-in-cells' from JSON buffer */
    json_statUnitsInCells = cJSON_GetObjectItem(params, "stat-units-in-cells");
    JSON_VALIDATE_JSON_POINTER(json_statUnitsInCells, "stat-units-in-cells", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_statUnitsInCells, "stat-units-in-cells");
    /* Copy the value */
    command.statUnitsInCells = json_statUnitsInCells->valueint;
    /* Ensure  that the number 'stat-units-in-cells' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.statUnitsInCells, 0, 1);
    command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_STATS_UNITS));

    /* Parsing and Validating 'max-trigger-reports' from JSON buffer */
    json_maxTriggerReports = cJSON_GetObjectItem(params, "trigger-rate-limit");
    if (NULL != json_maxTriggerReports)
    {
      JSON_VALIDATE_JSON_POINTER(json_maxTriggerReports, "trigger-rate-limit", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_maxTriggerReports, "trigger-rate-limit");
      /* Copy the value */
      command.bstMaxTriggers = json_maxTriggerReports->valueint;
      JSON_CHECK_VALUE_AND_CLEANUP (command.bstMaxTriggers, 1, 30);
      command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_TGR_RATE_LIMIT));
    }

    /* Parsing and Validating 'send-snapshot-on-trigger' from JSON buffer */
    json_sendSnapshotTrigger = cJSON_GetObjectItem(params, "send-snapshot-on-trigger");
    if (NULL != json_sendSnapshotTrigger)
    {
      JSON_VALIDATE_JSON_POINTER(json_sendSnapshotTrigger, "send-snapshot-on-trigger", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_sendSnapshotTrigger, "send-snapshot-on-trigger");
      /* Copy the value */
      command.sendSnapshotOnTrigger = json_sendSnapshotTrigger->valueint;
      JSON_CHECK_VALUE_AND_CLEANUP (command.sendSnapshotOnTrigger, 0, 1);
      command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_SND_SNAP_TGR));
    }

    /* Parsing and Validating 'trigger-rate-limit-interval' from JSON buffer */
    json_triggerTransmitInterval = cJSON_GetObjectItem(params, "trigger-rate-limit-interval");
    if (NULL != json_triggerTransmitInterval)
    {
      JSON_VALIDATE_JSON_POINTER(json_triggerTransmitInterval, "trigger-rate-limit-interval", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_triggerTransmitInterval, "trigger-rate-limit-interval");
      /* Copy the value */
      command.triggerTransmitInterval = json_triggerTransmitInterval->valueint;
      JSON_CHECK_VALUE_AND_CLEANUP (command.triggerTransmitInterval, 1, 60);
      command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_TGR_RL_INTVL));
    }

    /* Parsing and Validating 'aync-full-reports' from JSON buffer */
    json_sendIncrementalReport = cJSON_GetObjectItem(params, "async-full-reports");
    if (NULL != json_sendIncrementalReport)
    {
      JSON_VALIDATE_JSON_POINTER(json_sendIncrementalReport, "async-full-reports", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_sendIncrementalReport, "async-full-reports");
      /* Copy the value */
      command.sendIncrementalReport = json_sendIncrementalReport->valueint;
      JSON_CHECK_VALUE_AND_CLEANUP (command.sendIncrementalReport, 0, 1);
      command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_ASYNC_FULL_REP));
    }

    /* Parsing and Validating 'stats-in-percentage' from JSON buffer */
    json_statsInPercentage = cJSON_GetObjectItem(params, "stats-in-percentage");
    if (NULL != json_statsInPercentage)
    {
      JSON_VALIDATE_JSON_POINTER(json_statsInPercentage, "stats-in-percentage", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_statsInPercentage, "stats-in-percentage");
      /* Copy the value */
      command.statsInPercentage = json_statsInPercentage->valueint;
      /* Ensure  that the number 'stats-in-percentage' is within range of [0,1] */
      JSON_CHECK_VALUE_AND_CLEANUP (command.statsInPercentage, 0, 1);
      command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_STATS_IN_PERCENT));
    }
    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, &command,INSIGHT_BST_CMD_API_SET_FEATURE);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}
extern INSIGHT_STATUS
insight_bst_json_configure_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength)
{

    /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id, *json_realm, *json_port;
    cJSON *json_priorityGroup, *json_servicePool, *json_queue;
    cJSON *json_queueGroup, *json_threshold, *json_umShareThreshold;
    cJSON *json_umHeadroomThreshold, *json_ucShareThreshold, *json_mcShareThreshold;
    cJSON *json_mcShareQueueEntriesThreshold, *json_ucThreshold, *json_mcThreshold;
    cJSON *json_mcQueueEntriesThreshold, *json_cpuThreshold, *json_rqeThreshold;
    cJSON  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_CONFIGURE_BST_THRESHOLDS_t command;

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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "configure-bst-thresholds" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "configure-bst-thresholds");


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


    /* Parsing and Validating 'realm' from JSON buffer */
    json_realm = cJSON_GetObjectItem(params, "realm");
    JSON_VALIDATE_JSON_POINTER(json_realm, "realm", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_realm, "realm", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&command.realm[0], json_realm->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);


    /* Parsing and Validating 'port' from JSON buffer */
    json_port = cJSON_GetObjectItem(params, "port");
    /* The node 'port' is an optioanl one, ignore if not present in JSON */
    if (json_port != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_port, "port", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_STRING(json_port, "port", INSIGHT_STATUS_INVALID_JSON);
        /* Copy the 'port' in external notation to our internal representation */
        JSON_PORT_MAP_FROM_NOTATION(command.port, json_port->valuestring);
        command.mask = command.mask | INSIGHT_BST_PORT_MASK;
    } /* if optional */


    /* Parsing and Validating 'priority-group' from JSON buffer */
    json_priorityGroup = cJSON_GetObjectItem(params, "priority-group");
    /* The node 'priority-group' is an optioanl one, ignore if not present in JSON */
    if (json_priorityGroup != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_priorityGroup, "priority-group", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_priorityGroup, "priority-group");
        /* Copy the value */
        command.priorityGroup = json_priorityGroup->valueint;
        /* Ensure  that the number 'priority-group' is within range of [0,7] */
        JSON_CHECK_VALUE_AND_CLEANUP (command.priorityGroup, 0, 7);
        command.mask = command.mask | INSIGHT_BST_PG_MASK;
    } /* if optional */


    /* Parsing and Validating 'service-pool' from JSON buffer */
    json_servicePool = cJSON_GetObjectItem(params, "service-pool");
    /* The node 'service-pool' is an optioanl one, ignore if not present in JSON */
    if (json_servicePool != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_servicePool, "service-pool", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_servicePool, "service-pool");
        /* Copy the value */
        command.servicePool = json_servicePool->valueint;
        /* Ensure  that the number 'service-pool' is within range of [0,3] */
        JSON_CHECK_VALUE_AND_CLEANUP (command.servicePool, 0, 3);
        command.mask = command.mask | INSIGHT_BST_SP_MASK;
    } /* if optional */


    /* Parsing and Validating 'queue' from JSON buffer */
    json_queue = cJSON_GetObjectItem(params, "queue");
    /* The node 'queue' is an optioanl one, ignore if not present in JSON */
    if (json_queue != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_queue, "queue", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_queue, "queue");
        /* Copy the value */
        command.queue = json_queue->valueint;
        /* Ensure  that the number 'queue' is within range of [0,4095] */
        JSON_CHECK_VALUE_AND_CLEANUP (command.queue, 0, 4095);
        command.mask = command.mask | INSIGHT_BST_QUEUE_MASK;
    } /* if optional */


    /* Parsing and Validating 'queue-group' from JSON buffer */
    json_queueGroup = cJSON_GetObjectItem(params, "queue-group");
    /* The node 'queue-group' is an optioanl one, ignore if not present in JSON */
    if (json_queueGroup != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_queueGroup, "queue-group", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_queueGroup, "queue-group");
        /* Copy the value */
        command.queueGroup = json_queueGroup->valueint;
        /* Ensure  that the number 'queue-group' is within range of [0,7] */
        JSON_CHECK_VALUE_AND_CLEANUP (command.queueGroup, 0, 127);
        command.mask = command.mask | INSIGHT_BST_QUEUE_GRP_MASK;
    } /* if optional */


    /* Parsing and Validating 'threshold' from JSON buffer */
    json_threshold = cJSON_GetObjectItem(params, "threshold");
    /* The node 'threshold' is an optioanl one, ignore if not present in JSON */
    if (json_threshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_threshold, "threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_threshold, "threshold");
        /* Copy the value */
        command.threshold = json_threshold->valueint;
        command.mask = command.mask | INSIGHT_BST_DEVICE_MASK;
    } /* if optional */


    /* Parsing and Validating 'um-share-threshold' from JSON buffer */
    json_umShareThreshold = cJSON_GetObjectItem(params, "um-share-threshold");
    /* The node 'um-share-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_umShareThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_umShareThreshold, "um-share-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_umShareThreshold, "um-share-threshold");
        /* Copy the value */
        command.umShareThreshold = json_umShareThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_UMSHARE_MASK;
    } /* if optional */


    /* Parsing and Validating 'um-headroom-threshold' from JSON buffer */
    json_umHeadroomThreshold = cJSON_GetObjectItem(params, "um-headroom-threshold");
    /* The node 'um-headroom-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_umHeadroomThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_umHeadroomThreshold, "um-headroom-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_umHeadroomThreshold, "um-headroom-threshold");
        /* Copy the value */
        command.umHeadroomThreshold = json_umHeadroomThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_UMHEADROOM_MASK;
    } /* if optional */


    /* Parsing and Validating 'uc-share-threshold' from JSON buffer */
    json_ucShareThreshold = cJSON_GetObjectItem(params, "uc-share-threshold");
    /* The node 'uc-share-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_ucShareThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_ucShareThreshold, "uc-share-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_ucShareThreshold, "uc-share-threshold");
        /* Copy the value */
        command.ucShareThreshold = json_ucShareThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_UCSHARE_MASK;
    } /* if optional */


    /* Parsing and Validating 'mc-share-threshold' from JSON buffer */
    json_mcShareThreshold = cJSON_GetObjectItem(params, "mc-share-threshold");
    /* The node 'mc-share-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_mcShareThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_mcShareThreshold, "mc-share-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_mcShareThreshold, "mc-share-threshold");
        /* Copy the value */
        command.mcShareThreshold = json_mcShareThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_MCSHARE_MASK;
    } /* if optional */


    /* Parsing and Validating 'mc-share-queue-entries-threshold' from JSON buffer */
    json_mcShareQueueEntriesThreshold = cJSON_GetObjectItem(params, "mc-share-queue-entries-threshold");
    /* The node 'mc-share-queue-entries-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_mcShareQueueEntriesThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_mcShareQueueEntriesThreshold, "mc-share-queue-entries-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_mcShareQueueEntriesThreshold, "mc-share-queue-entries-threshold");
        /* Copy the value */
        command.mcShareQueueEntriesThreshold = json_mcShareQueueEntriesThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_MCSHARE_QUEUE_MASK;
    } /* if optional */


    /* Parsing and Validating 'uc-threshold' from JSON buffer */
    json_ucThreshold = cJSON_GetObjectItem(params, "uc-threshold");
    /* The node 'uc-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_ucThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_ucThreshold, "uc-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_ucThreshold, "uc-threshold");
        /* Copy the value */
        command.ucThreshold = json_ucThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_UC_MASK;
    } /* if optional */


    /* Parsing and Validating 'mc-threshold' from JSON buffer */
    json_mcThreshold = cJSON_GetObjectItem(params, "mc-threshold");
    /* The node 'mc-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_mcThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_mcThreshold, "mc-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_mcThreshold, "mc-threshold");
        /* Copy the value */
        command.mcThreshold = json_mcThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_MC_MASK;
    } /* if optional */


    /* Parsing and Validating 'mc-queue-entries-threshold' from JSON buffer */
    json_mcQueueEntriesThreshold = cJSON_GetObjectItem(params, "mc-queue-entries-threshold");
    /* The node 'mc-queue-entries-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_mcQueueEntriesThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_mcQueueEntriesThreshold, "mc-queue-entries-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_mcQueueEntriesThreshold, "mc-queue-entries-threshold");
        /* Copy the value */
        command.mcQueueEntriesThreshold = json_mcQueueEntriesThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_MCQUEUE_MASK;
    } /* if optional */


    /* Parsing and Validating 'cpu-threshold' from JSON buffer */
    json_cpuThreshold = cJSON_GetObjectItem(params, "cpu-threshold");
    /* The node 'cpu-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_cpuThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_cpuThreshold, "cpu-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_cpuThreshold, "cpu-threshold");
        /* Copy the value */
        command.cpuThreshold = json_cpuThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_CPU_MASK;
    } /* if optional */


    /* Parsing and Validating 'rqe-threshold' from JSON buffer */
    json_rqeThreshold = cJSON_GetObjectItem(params, "rqe-threshold");
    /* The node 'rqe-threshold' is an optioanl one, ignore if not present in JSON */
    if (json_rqeThreshold != NULL)
    {
        JSON_VALIDATE_JSON_POINTER(json_rqeThreshold, "rqe-threshold", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_NUMBER(json_rqeThreshold, "rqe-threshold");
        /* Copy the value */
        command.rqeThreshold = json_rqeThreshold->valueint;
        command.mask = command.mask | INSIGHT_BST_RQE_MASK;
    } /* if optional */


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, &command,INSIGHT_BST_CMD_API_SET_THRESHOLD);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;

}

extern INSIGHT_STATUS
insight_bst_json_get_bst_report (void *cookie, char *jsonBuffer, int bufLength)
{
  /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id, *json_includeIngressPortPriorityGroup, *json_includeIngressPortServicePool;
    cJSON *json_includeIngressServicePool, *json_includeEgressPortServicePool, *json_includeEgressServicePool;
    cJSON *json_includeEgressUcQueue, *json_includeEgressUcQueueGroup, *json_includeEgressMcQueue;
    cJSON *json_includeEgressCpuQueue, *json_includeEgressRqeQueue, *json_includeDevice;
    cJSON  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_GET_BST_REPORT_t command;

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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-report" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-bst-report");


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


    /* Parsing and Validating 'include-ingress-port-priority-group' from JSON buffer */
    json_includeIngressPortPriorityGroup = cJSON_GetObjectItem(params, "include-ingress-port-priority-group");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressPortPriorityGroup, "include-ingress-port-priority-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressPortPriorityGroup, "include-ingress-port-priority-group");
    /* Copy the value */
    command.includeIngressPortPriorityGroup = json_includeIngressPortPriorityGroup->valueint;
    /* Ensure  that the number 'include-ingress-port-priority-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressPortPriorityGroup, 0, 1);


    /* Parsing and Validating 'include-ingress-port-service-pool' from JSON buffer */
    json_includeIngressPortServicePool = cJSON_GetObjectItem(params, "include-ingress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressPortServicePool, "include-ingress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressPortServicePool, "include-ingress-port-service-pool");
    /* Copy the value */
    command.includeIngressPortServicePool = json_includeIngressPortServicePool->valueint;
    /* Ensure  that the number 'include-ingress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressPortServicePool, 0, 1);


    /* Parsing and Validating 'include-ingress-service-pool' from JSON buffer */
    json_includeIngressServicePool = cJSON_GetObjectItem(params, "include-ingress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressServicePool, "include-ingress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressServicePool, "include-ingress-service-pool");
    /* Copy the value */
    command.includeIngressServicePool = json_includeIngressServicePool->valueint;
    /* Ensure  that the number 'include-ingress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-port-service-pool' from JSON buffer */
    json_includeEgressPortServicePool = cJSON_GetObjectItem(params, "include-egress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressPortServicePool, "include-egress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressPortServicePool, "include-egress-port-service-pool");
    /* Copy the value */
    command.includeEgressPortServicePool = json_includeEgressPortServicePool->valueint;
    /* Ensure  that the number 'include-egress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressPortServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-service-pool' from JSON buffer */
    json_includeEgressServicePool = cJSON_GetObjectItem(params, "include-egress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressServicePool, "include-egress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressServicePool, "include-egress-service-pool");
    /* Copy the value */
    command.includeEgressServicePool = json_includeEgressServicePool->valueint;
    /* Ensure  that the number 'include-egress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-uc-queue' from JSON buffer */
    json_includeEgressUcQueue = cJSON_GetObjectItem(params, "include-egress-uc-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressUcQueue, "include-egress-uc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressUcQueue, "include-egress-uc-queue");
    /* Copy the value */
    command.includeEgressUcQueue = json_includeEgressUcQueue->valueint;
    /* Ensure  that the number 'include-egress-uc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressUcQueue, 0, 1);


    /* Parsing and Validating 'include-egress-uc-queue-group' from JSON buffer */
    json_includeEgressUcQueueGroup = cJSON_GetObjectItem(params, "include-egress-uc-queue-group");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressUcQueueGroup, "include-egress-uc-queue-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressUcQueueGroup, "include-egress-uc-queue-group");
    /* Copy the value */
    command.includeEgressUcQueueGroup = json_includeEgressUcQueueGroup->valueint;
    /* Ensure  that the number 'include-egress-uc-queue-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressUcQueueGroup, 0, 1);


    /* Parsing and Validating 'include-egress-mc-queue' from JSON buffer */
    json_includeEgressMcQueue = cJSON_GetObjectItem(params, "include-egress-mc-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressMcQueue, "include-egress-mc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressMcQueue, "include-egress-mc-queue");
    /* Copy the value */
    command.includeEgressMcQueue = json_includeEgressMcQueue->valueint;
    /* Ensure  that the number 'include-egress-mc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressMcQueue, 0, 1);


    /* Parsing and Validating 'include-egress-cpu-queue' from JSON buffer */
    json_includeEgressCpuQueue = cJSON_GetObjectItem(params, "include-egress-cpu-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressCpuQueue, "include-egress-cpu-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressCpuQueue, "include-egress-cpu-queue");
    /* Copy the value */
    command.includeEgressCpuQueue = json_includeEgressCpuQueue->valueint;
    /* Ensure  that the number 'include-egress-cpu-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressCpuQueue, 0, 1);


    /* Parsing and Validating 'include-egress-rqe-queue' from JSON buffer */
    json_includeEgressRqeQueue = cJSON_GetObjectItem(params, "include-egress-rqe-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressRqeQueue, "include-egress-rqe-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressRqeQueue, "include-egress-rqe-queue");
    /* Copy the value */
    command.includeEgressRqeQueue = json_includeEgressRqeQueue->valueint;
    /* Ensure  that the number 'include-egress-rqe-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressRqeQueue, 0, 1);


    /* Parsing and Validating 'include-device' from JSON buffer */
    json_includeDevice = cJSON_GetObjectItem(params, "include-device");
    JSON_VALIDATE_JSON_POINTER(json_includeDevice, "include-device", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeDevice, "include-device");
    /* Copy the value */
    command.includeDevice = json_includeDevice->valueint;
    /* Ensure  that the number 'include-device' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeDevice, 0, 1);


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id,&command,INSIGHT_BST_CMD_API_GET_REPORT);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}
extern INSIGHT_STATUS
insight_bst_json_get_bst_feature (void *cookie, char *jsonBuffer, int bufLength)
{
  /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-feature" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-bst-feature");


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


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, NULL,INSIGHT_BST_CMD_API_GET_FEATURE);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
   return status;
}

extern INSIGHT_STATUS
insight_bst_json_get_bst_tracking (void *cookie, char *jsonBuffer, int bufLength)
{

 /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-tracking" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-bst-tracking");


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


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, NULL,INSIGHT_BST_CMD_API_GET_TRACK);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}
extern INSIGHT_STATUS
insight_bst_json_get_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength)
{
 /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id, *json_includeIngressPortPriorityGroup, *json_includeIngressPortServicePool;
    cJSON *json_includeIngressServicePool, *json_includeEgressPortServicePool, *json_includeEgressServicePool;
    cJSON *json_includeEgressUcQueue, *json_includeEgressUcQueueGroup, *json_includeEgressMcQueue;
    cJSON *json_includeEgressCpuQueue, *json_includeEgressRqeQueue, *json_includeDevice;
    cJSON  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_GET_BST_THRESHOLDS_t command;

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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-thresholds" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-bst-thresholds");


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


    /* Parsing and Validating 'include-ingress-port-priority-group' from JSON buffer */
    json_includeIngressPortPriorityGroup = cJSON_GetObjectItem(params, "include-ingress-port-priority-group");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressPortPriorityGroup, "include-ingress-port-priority-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressPortPriorityGroup, "include-ingress-port-priority-group");
    /* Copy the value */
    command.includeIngressPortPriorityGroup = json_includeIngressPortPriorityGroup->valueint;
    /* Ensure  that the number 'include-ingress-port-priority-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressPortPriorityGroup, 0, 1);


    /* Parsing and Validating 'include-ingress-port-service-pool' from JSON buffer */
    json_includeIngressPortServicePool = cJSON_GetObjectItem(params, "include-ingress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressPortServicePool, "include-ingress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressPortServicePool, "include-ingress-port-service-pool");
    /* Copy the value */
    command.includeIngressPortServicePool = json_includeIngressPortServicePool->valueint;
    /* Ensure  that the number 'include-ingress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressPortServicePool, 0, 1);


    /* Parsing and Validating 'include-ingress-service-pool' from JSON buffer */
    json_includeIngressServicePool = cJSON_GetObjectItem(params, "include-ingress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeIngressServicePool, "include-ingress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeIngressServicePool, "include-ingress-service-pool");
    /* Copy the value */
    command.includeIngressServicePool = json_includeIngressServicePool->valueint;
    /* Ensure  that the number 'include-ingress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeIngressServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-port-service-pool' from JSON buffer */
    json_includeEgressPortServicePool = cJSON_GetObjectItem(params, "include-egress-port-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressPortServicePool, "include-egress-port-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressPortServicePool, "include-egress-port-service-pool");
    /* Copy the value */
    command.includeEgressPortServicePool = json_includeEgressPortServicePool->valueint;
    /* Ensure  that the number 'include-egress-port-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressPortServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-service-pool' from JSON buffer */
    json_includeEgressServicePool = cJSON_GetObjectItem(params, "include-egress-service-pool");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressServicePool, "include-egress-service-pool", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressServicePool, "include-egress-service-pool");
    /* Copy the value */
    command.includeEgressServicePool = json_includeEgressServicePool->valueint;
    /* Ensure  that the number 'include-egress-service-pool' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressServicePool, 0, 1);


    /* Parsing and Validating 'include-egress-uc-queue' from JSON buffer */
    json_includeEgressUcQueue = cJSON_GetObjectItem(params, "include-egress-uc-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressUcQueue, "include-egress-uc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressUcQueue, "include-egress-uc-queue");
    /* Copy the value */
    command.includeEgressUcQueue = json_includeEgressUcQueue->valueint;
    /* Ensure  that the number 'include-egress-uc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressUcQueue, 0, 1);


    /* Parsing and Validating 'include-egress-uc-queue-group' from JSON buffer */
    json_includeEgressUcQueueGroup = cJSON_GetObjectItem(params, "include-egress-uc-queue-group");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressUcQueueGroup, "include-egress-uc-queue-group", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressUcQueueGroup, "include-egress-uc-queue-group");
    /* Copy the value */
    command.includeEgressUcQueueGroup = json_includeEgressUcQueueGroup->valueint;
    /* Ensure  that the number 'include-egress-uc-queue-group' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressUcQueueGroup, 0, 1);


    /* Parsing and Validating 'include-egress-mc-queue' from JSON buffer */
    json_includeEgressMcQueue = cJSON_GetObjectItem(params, "include-egress-mc-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressMcQueue, "include-egress-mc-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressMcQueue, "include-egress-mc-queue");
    /* Copy the value */
    command.includeEgressMcQueue = json_includeEgressMcQueue->valueint;
    /* Ensure  that the number 'include-egress-mc-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressMcQueue, 0, 1);


    /* Parsing and Validating 'include-egress-cpu-queue' from JSON buffer */
    json_includeEgressCpuQueue = cJSON_GetObjectItem(params, "include-egress-cpu-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressCpuQueue, "include-egress-cpu-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressCpuQueue, "include-egress-cpu-queue");
    /* Copy the value */
    command.includeEgressCpuQueue = json_includeEgressCpuQueue->valueint;
    /* Ensure  that the number 'include-egress-cpu-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressCpuQueue, 0, 1);


    /* Parsing and Validating 'include-egress-rqe-queue' from JSON buffer */
    json_includeEgressRqeQueue = cJSON_GetObjectItem(params, "include-egress-rqe-queue");
    JSON_VALIDATE_JSON_POINTER(json_includeEgressRqeQueue, "include-egress-rqe-queue", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeEgressRqeQueue, "include-egress-rqe-queue");
    /* Copy the value */
    command.includeEgressRqeQueue = json_includeEgressRqeQueue->valueint;
    /* Ensure  that the number 'include-egress-rqe-queue' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeEgressRqeQueue, 0, 1);


    /* Parsing and Validating 'include-device' from JSON buffer */
    json_includeDevice = cJSON_GetObjectItem(params, "include-device");
    JSON_VALIDATE_JSON_POINTER(json_includeDevice, "include-device", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_NUMBER(json_includeDevice, "include-device");
    /* Copy the value */
    command.includeDevice = json_includeDevice->valueint;
    /* Ensure  that the number 'include-device' is within range of [0,1] */
    JSON_CHECK_VALUE_AND_CLEANUP (command.includeDevice, 0, 1);


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, &command,INSIGHT_BST_CMD_API_GET_THRESHOLD);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

extern INSIGHT_STATUS
insight_bst_json_clear_bst_thresholds (void *cookie, char *jsonBuffer, int bufLength)
{
 /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "clear-bst-thresholds" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "clear-bst-thresholds");


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


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
   status = bstjson_send_msg_to_app (cookie, asicId, id, NULL,INSIGHT_BST_CMD_API_CLEAR_THRESHOLD);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}
extern INSIGHT_STATUS
insight_bst_json_clear_bst_statistics (void *cookie, char *jsonBuffer, int bufLength)
{
     /* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "clear-bst-statistics" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "clear-bst-statistics");


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


    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, NULL,INSIGHT_BST_CMD_API_CLEAR_STATS);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}



INSIGHT_STATUS bst_json_cgsn_get_req_type(char *input, INSIGHT_BST_CGSN_REQ_TYPE_t *val)
{
  if ((NULL == input)||
      (NULL == val))
    return INSIGHT_STATUS_INVALID_PARAMETER;

  unsigned int i = 0;

  const bst_json_string_value_t cgsn_drp_req_map[] = {
    {"top-drops", INSIGHT_BST_CGSN_TOP_DROPS},
    {"top-port-queue-drops", INSIGHT_BST_CGSN_TOP_PRT_Q_DROPS},
    {"port-drops", INSIGHT_BST_CGSN_PRT_DROPS},
    {"port-queue-drops", INSIGHT_BST_CGSN_PRT_Q_DROPS}
  };
  for (i = 0; i < (sizeof(cgsn_drp_req_map)/sizeof(bst_json_string_value_t)); i++)
  {
    if (0 == strcmp (input, cgsn_drp_req_map[i].req_str))
    {
      *val = cgsn_drp_req_map[i].value;
      return INSIGHT_STATUS_SUCCESS;
    }
  }

  return INSIGHT_STATUS_FAILURE;

}


INSIGHT_STATUS _bst_json_cgsn_get_q_type(char *input, INSIGHT_BST_CGSN_CTR_TYPE_t *val)
{
  if ((NULL == input)||
      (NULL == val))
    return INSIGHT_STATUS_INVALID_PARAMETER;

  unsigned int i = 0;

  const bst_json_string_value_t cgsn_drp_q_type_map[] = {
    {"ucast", INSIGHT_BST_CGSN_UCAST},
    {"mcast", INSIGHT_BST_CGSN_MCAST},
    {"all", INSIGHT_BST_CGSN_ALL}
  };
  for (i = 0; i < (sizeof(cgsn_drp_q_type_map)/sizeof(bst_json_string_value_t)); i++)
  {
    if (0 == strcmp (input, cgsn_drp_q_type_map[i].req_str))
    {
      *val = cgsn_drp_q_type_map[i].value;
      return INSIGHT_STATUS_SUCCESS;
    }
  }
  return INSIGHT_STATUS_FAILURE;
}


extern INSIGHT_STATUS
insight_bst_json_get_bst_congestion_drop_counters (void *cookie, char *jsonBuffer, int bufLength)
{
/* Local Variables for JSON Parsing */
    cJSON *json_jsonrpc, *json_method, *json_asicId;
    cJSON *json_id,  *root, *params;
    cJSON *json_req_type, *json_req_params, *json_count, *json_ports, *json_queue;
    cJSON *json_interval, *json_port_list, *json_q_type, *json_queue_array;

    /* Local non-command-parameter JSON variable declarations */
    char jsonrpc[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char method[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char req_type_str[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    char q_type_str[INSIGHT_JSON_MAX_NODE_LENGTH] = {0};
    int asicId = 0, id = 0, iter = 0;
    unsigned int prt= 0, queue = 0;
    unsigned int mask = 0;
    INSIGHT_PORT_MASK_t port_list;
    INSIGHT_QUEUE_MASK_t queue_list;
    bool valid = false;

    /* Local variable declarations */
    INSIGHT_STATUS status = INSIGHT_STATUS_SUCCESS;
    BSTJSON_GET_BST_CGSN_DROP_CTRS_t command;
    INSIGHT_BST_CGSN_REQ_TYPE_t req_type;
    INSIGHT_BST_CGSN_CTR_TYPE_t queue_type;

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
    strncpy (&jsonrpc[0], json_jsonrpc->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'jsonrpc' in the JSON equals "2.0" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("jsonrpc", &jsonrpc[0], "2.0");


    /* Parsing and Validating 'method' from JSON buffer */
    json_method = cJSON_GetObjectItem(root, "method");
    JSON_VALIDATE_JSON_POINTER(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_method, "method", INSIGHT_STATUS_INVALID_JSON);
    /* Copy the string, with a limit on max characters */
    strncpy (&method[0], json_method->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    /* Ensure that 'method' in the JSON equals "get-bst-congestion-drop-counters" */
    JSON_COMPARE_STRINGS_AND_CLEANUP ("method", &method[0], "get-bst-congestion-drop-counters");


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

    /* Parsing and Validating 'collection-interval' from JSON buffer */
    json_interval = cJSON_GetObjectItem(params, "collection-interval");
    if (NULL != json_interval)
    {
      JSON_VALIDATE_JSON_POINTER(json_interval, "collection-interval", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_interval, "collection-interval");
      command.intrvl = json_interval->valueint;
      JSON_CHECK_VALUE_AND_CLEANUP (command.intrvl, 0, 3600);
    }
    else
    {
      command.intrvl = 0;
    }
    /* Parsing and Validating 'request-type' from JSON buffer */
    json_req_type = cJSON_GetObjectItem(params, "request-type");
    JSON_VALIDATE_JSON_POINTER(json_req_type, "request-type", INSIGHT_STATUS_INVALID_JSON);
    JSON_VALIDATE_JSON_AS_STRING(json_req_type, "request-type", INSIGHT_STATUS_INVALID_JSON);
    strncpy(&req_type_str[0], json_req_type->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
    status = bst_json_cgsn_get_req_type(&req_type_str[0], &req_type);
    if (INSIGHT_STATUS_SUCCESS != status)
    {
      /* Free up any allocated resources and return status code */
      if (root != NULL)
      {
        cJSON_Delete(root);
      }
      return INSIGHT_STATUS_INVALID_JSON;
    }
    command.req_type = req_type;
    mask = mask | INSIGHT_BST_CGSN_REQ_TYPE;

    /* Parsing and Validating 'request-params' from JSON buffer */
    json_req_params = cJSON_GetObjectItem(params, "request-params");
    JSON_VALIDATE_JSON_POINTER(json_req_params, "request-params", INSIGHT_STATUS_INVALID_JSON);

    /* Parsing and Validating 'request-count' from JSON buffer */
    json_count = cJSON_GetObjectItem(json_req_params, "count");

    if (NULL != json_count)
    {
      JSON_VALIDATE_JSON_POINTER(json_count, "count", INSIGHT_STATUS_INVALID_JSON);
      JSON_VALIDATE_JSON_AS_NUMBER(json_count, "count");
      /* Copy the value */
      command.count = json_count->valueint;

      mask = mask | INSIGHT_BST_CGSN_COUNT;
    }

    /* Parsing and Validating 'port-list' from JSON buffer */
    sal_memset(&port_list, 0, sizeof(INSIGHT_PORT_MASK_t));
    json_port_list = cJSON_GetObjectItem(json_req_params, "port-list");

    if (NULL != json_port_list)
    {
        sal_memset (&command.port_list, 0, sizeof(INSIGHT_PORT_MASK_t));
      JSON_VALIDATE_JSON_POINTER(json_port_list, "port-list", INSIGHT_STATUS_INVALID_JSON);
      for (iter = 0; iter < cJSON_GetArraySize(json_port_list); iter++)
      {
        json_ports = cJSON_GetArrayItem(json_port_list, iter);
        if (0 == strncmp ("all", json_ports->valuestring, sal_strlen(json_ports->valuestring)))
        {
          if (1 < cJSON_GetArraySize(json_port_list))
          {
            /* expect only "all". Invalid JSON */
            if (root != NULL)
            {
              cJSON_Delete(root);
            }
            return INSIGHT_STATUS_INVALID_JSON;
          }
          command.all_prts = true;
        }
        else
        {
          /* convert the port from string to the valid format */
          JSON_PORT_MAP_FROM_NOTATION(prt, json_ports->valuestring);
          if (0 != prt)
          {
            /* set the bit in the mask */
            INSIGHT_SETMASKBIT(command.port_list, prt);
          }
        }
      }
      mask = mask | INSIGHT_BST_CGSN_PORT_LIST;
    }


      /* Parsing and Validating 'request-type' from JSON buffer */
      json_q_type = cJSON_GetObjectItem(json_req_params, "queue-type");

      if (NULL != json_q_type)
      {
        JSON_VALIDATE_JSON_POINTER(json_q_type, "queue-type", INSIGHT_STATUS_INVALID_JSON);
        JSON_VALIDATE_JSON_AS_STRING(json_q_type, "queue-type", INSIGHT_STATUS_INVALID_JSON);
        strncpy(&q_type_str[0], json_q_type->valuestring, INSIGHT_JSON_MAX_NODE_LENGTH - 1);
        status = _bst_json_cgsn_get_q_type(&q_type_str[0], &queue_type);
        if (INSIGHT_STATUS_SUCCESS != status)
        {
          /* Free up any allocated resources and return status code */
          if (root != NULL)
          {
            cJSON_Delete(root);
          }
          return INSIGHT_STATUS_INVALID_JSON;
        }
        command.queue_type = queue_type;
        mask = mask | INSIGHT_BST_CGSN_QUEUE_TYPE;
      }


    /* Parsing and Validating 'queue-list' from JSON buffer */
    sal_memset(&queue_list, 0, sizeof(INSIGHT_QUEUE_MASK_t));
    json_queue_array = cJSON_GetObjectItem(json_req_params, "queue-list");

    if (NULL != json_queue_array)
    {
      sal_memset (&command.queue_list, 0, sizeof(INSIGHT_QUEUE_MASK_t));
      JSON_VALIDATE_JSON_POINTER(json_queue_array, "queue-list", INSIGHT_STATUS_INVALID_JSON);
      for (iter = 0; iter < cJSON_GetArraySize(json_queue_array); iter++)
      {
        json_queue = cJSON_GetArrayItem(json_queue_array, iter);
        queue = json_queue->valueint;
        queue = queue+1;

        /* add +1 to the queue and remove the same while parsing at the other end */
        if (0 != queue)
        {
          /* set the bit in the mask */
          INSIGHT_SETMASKBIT(command.queue_list, queue);
          mask = mask | INSIGHT_BST_CGSN_QUEUE_LIST;
        }
      }
    }

    /* validations for permitted combinations */
     if (((INSIGHT_BST_CGSN_TOP_DROPS == command.req_type) &&
         (mask & INSIGHT_BST_TOP_PRT_DRP_VALID) &&
         (~mask & ~INSIGHT_BST_TOP_PRT_DRP_VALID)) ||
         ((INSIGHT_BST_CGSN_TOP_PRT_Q_DROPS == command.req_type) &&
         (mask & INSIGHT_BST_TOP_PRT_Q_DRP_VALID) &&
         (~mask & ~INSIGHT_BST_TOP_PRT_Q_DRP_VALID)) ||
         ((INSIGHT_BST_CGSN_PRT_DROPS == command.req_type) &&
         (mask & INSIGHT_BST_PRT_DRP_VALID) &&
         (~mask & ~INSIGHT_BST_PRT_DRP_VALID)) ||
         ((INSIGHT_BST_CGSN_PRT_Q_DROPS == command.req_type) &&
         (mask & INSIGHT_BST_PRT_Q_DRP_VALID) &&
         (~mask & ~INSIGHT_BST_PRT_Q_DRP_VALID)))
      {
        valid = true;
      }

      if (true != valid)
      {
        if (root != NULL)
        {
          cJSON_Delete(root);
        }
      }

    /* Send the 'command' along with 'asicId' and 'cookie' to the Application thread. */
    status = bstjson_send_msg_to_app (cookie, asicId, id, &command,INSIGHT_BST_CMD_API_GET_CGSN_DRP_CTRS);

    /* Free up any allocated resources and return status code */
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return status;
}

#define  INSIGHT_APP_TO_ENCODE_JSON



 /* Encode Json */
extern INSIGHT_STATUS
bstjson_encode_get_bst_feature( int asicId,
                                            int method,
                                            const BSTJSON_CONFIGURE_BST_FEATURE_t *pData,
                                            uint8 **pJsonBuffer
                                            )
{
    char *getBstFeatureTemplate = " {\
          \"jsonrpc\": \"2.0\",\
          \"method\": \"get-bst-feature\",\
          \"asic-id\": \"%s\",\
          \"version\": \"%d\",\
          \"result\": {\
          \"bst-enable\": %d,\
          \"send-async-reports\": %d,\
          \"collection-interval\": %d,\
          \"stat-units-in-cells\": %d,\
          \"trigger-rate-limit\": %d,\
          \"send-snapshot-on-trigger\": %d,\
          \"trigger-rate-limit-interval\": %d,\
          \"async-full-reports\": %d,\
          \"stats-in-percentage\": %d\
          },\
          \"id\": %d\
          }";

char *jsonBuf;
char asicIdStr[JSON_MAX_NODE_LENGTH] = {   0 };


 LOG_POST (INSIGHT_LOG_DEBUG,  "BST-JSON-Encoder : Request for Get-Bst-Feature \n");


/* allocate memory for JSON */
  jsonBuf = mem_malloc(MEM_APP_MODULE,INSIGHT_JSON_MEMSIZE_RESPONSE);
  if(!jsonBuf)
  {
    return INSIGHT_STATUS_INVALID_MEMORY;
  }

  /* clear the buffer */
  sal_memset(jsonBuf, 0, INSIGHT_JSON_MEMSIZE_RESPONSE);
/* convert asicId to external  notation */
JSON_ASIC_ID_MAP_TO_NOTATION(asicId, &asicIdStr[0]);

/* encode the JSON */
snprintf(jsonBuf, INSIGHT_JSON_MEMSIZE_RESPONSE, getBstFeatureTemplate,
         &asicIdStr[0], INSIGHT_JSON_VERSION, pData->bstEnable,
         pData->sendAsyncReports, pData->collectionInterval,
         pData->statUnitsInCells,
         pData->bstMaxTriggers, pData->sendSnapshotOnTrigger,
         pData->triggerTransmitInterval, (pData->sendIncrementalReport == 0)?1:0,
         pData->statsInPercentage, method);

/* setup the return value */
*pJsonBuffer = (uint8 *) jsonBuf;

LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : Encoding complete [%d bytes],buffer : %s \n", (int)sal_strlen(jsonBuf),jsonBuf);


return INSIGHT_STATUS_SUCCESS;
}

extern INSIGHT_STATUS
bstjson_encode_get_bst_tracking( int asicId,
                                             int method,
                                             const BSTJSON_CONFIGURE_BST_TRACKING_t *pData,
                                             uint8 **pJsonBuffer
                                             )
{
  char *getBstTrackingTemplate = " {\
\"jsonrpc\": \"2.0\",\
\"method\": \"get-bst-tracking\",\
\"asic-id\": \"%s\",\
\"version\": \"%d\",\
\"result\": {\
\"track-peak-stats\" : %d, \
\"track-ingress-port-priority-group\" : %d, \
\"track-ingress-port-service-pool\" : %d, \
\"track-ingress-service-pool\" : %d, \
\"track-egress-port-service-pool\" : %d, \
\"track-egress-service-pool\" : %d, \
\"track-egress-uc-queue\" : %d, \
\"track-egress-uc-queue-group\" : %d, \
\"track-egress-mc-queue\" : %d, \
\"track-egress-cpu-queue\" : %d, \
\"track-egress-rqe-queue\" : %d, \
\"track-device\" : %d \
},\
\"id\": %d\
}";

    char *jsonBuf;
    char asicIdStr[JSON_MAX_NODE_LENGTH] = { 0 };

   LOG_POST (INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : Request for Get-Bst-Tracking \n");

/* allocate memory for JSON */
  jsonBuf = mem_malloc(MEM_APP_MODULE,INSIGHT_JSON_MEMSIZE_RESPONSE);
  if(!jsonBuf)
  {
    return INSIGHT_STATUS_INVALID_MEMORY;
  }

  /* clear the buffer */
  sal_memset(jsonBuf, 0, INSIGHT_JSON_MEMSIZE_RESPONSE);

    /* convert asicId to external  notation */
    JSON_ASIC_ID_MAP_TO_NOTATION(asicId, &asicIdStr[0]);

    /* encode the JSON */
    snprintf(jsonBuf, INSIGHT_JSON_MEMSIZE_RESPONSE, getBstTrackingTemplate,
             &asicIdStr[0], INSIGHT_JSON_VERSION, pData->trackPeakStats,
             pData->trackIngressPortPriorityGroup,
             pData->trackIngressPortServicePool,
             pData->trackIngressServicePool,
             pData->trackEgressPortServicePool,
             pData->trackEgressServicePool,
             pData->trackEgressUcQueue,
             pData->trackEgressUcQueueGroup,
             pData->trackEgressMcQueue,
             pData->trackEgressCpuQueue,
             pData->trackEgressRqeQueue,
             pData->trackDevice, method);

    /* setup the return value */
    *pJsonBuffer = (uint8 *) jsonBuf;
    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : Encoding complete [%d bytes],buffer : %s \n", (int)sal_strlen(jsonBuf),jsonBuf);



    return INSIGHT_STATUS_SUCCESS;
}




/*******************************************************************
   Utility function to convert the data based on config
********************************************************************/
INSIGHT_STATUS bst_json_convert_data(const BSTJSON_REPORT_OPTIONS_t *options,
                                          const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                          uint64_t *value, uint64_t maxBufVal)
{
  double percentage =0;
  uint64_t data = 0;

  if ((NULL == options) ||
      (NULL == asic) ||
      (NULL == value))
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }

  data = *value;


  /* Report is threshold. */
  if (true == options->reportThreshold)
  {
    if (true == options->statsInPercentage)
    {
      if (0 == maxBufVal)
      {
        data = 0;
      }
      else
      {
        data = data/(asic->cellToByteConv);
        /* we just need the percentage of the configured value */
        percentage = ((double)(data * 100))/((double)(maxBufVal));
        data = round_int(percentage);
        if (100 < data)
        {
          data = 100;
        }
      }
    }
    else
    {

      if (true == options->statUnitsInCells)
      {
        /* threshold comes in bytes from asic */
        data = data / (asic->cellToByteConv);
      }
    }
  }
  else
  {
    /* report is stats */
    if (true == options->statsInPercentage)
    {
      if (0 == maxBufVal)
      {
        data = 0;
      }
      else
      {
        /* we just need the percentage of the configured value */
        percentage = ((double)(data * 100))/((double)(maxBufVal));
        data = round_int(percentage);
      }
    }
    else
    {
      /* conversion to bytes or cells based on config */
      if (false == options->statUnitsInCells)
      {
        /* check if we need to convert the data to cells
           the report always comes in cells from asic */
    //zhaomc    data = data * (asic->cellToByteConv);
     data = data;
      }
    }
  }

  *value = data;

  return INSIGHT_STATUS_SUCCESS;
}


/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - device part.
 *
 *********************************************************************/

INSIGHT_STATUS _jsonencode_report_device ( char *jsonBuf,
                                               const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                               const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                               const BSTJSON_REPORT_OPTIONS_t *options,
                                               const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                               int bufLen,
                                               int *length)
{

  char *getBstDeviceReportTemplate = "{ \"realm\" : \"device\", \"data\" : % " PRIu64 "}";
  /* Since this is an internal function, with all parameters validated already,
   * we jump to the logic straight-away
   */
  uint64_t data;
  *length = 0;
  uint64_t maxBufVal = 0;

  LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding device data \n");

  /* if collector is not interested in device stats, ignore it*/
  if (options->includeDevice == false)
  {
    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Device data not needed \n");
    return INSIGHT_STATUS_SUCCESS;
  }

  /* if there is no change in stats since we reported last time, ignore it*/
  if  ((previous != NULL) && (current->device.bufferCount == previous->device.bufferCount))
  {
    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Device data %" PRIu64 " has not changed since last reading \n",
        current->device.bufferCount);
    return INSIGHT_STATUS_SUCCESS;
  }
  /* data to be sent to collector */
  data = current->device.bufferCount;
  maxBufVal = options->bst_max_buffers_ptr->device.data.maxBuf;
  bst_json_convert_data(options, asic, &data, maxBufVal);

  /* encode the JSON */
  *length = snprintf(jsonBuf, bufLen, getBstDeviceReportTemplate, data);
  LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding device data [%d] complete \n", *length);

  return INSIGHT_STATUS_SUCCESS;
}




/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - ingress-port-port-group.
 *
 *********************************************************************/
static INSIGHT_STATUS _jsonencode_report_ingress_ippg ( char *buffer, int asicId,
                                                     const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                                     const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                                     const BSTJSON_REPORT_OPTIONS_t *options,
                                                     const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                                     int bufLen,
                                                     int *length)
{
    int remLength = bufLen;
    int actualLength  = 0;
    bool includePort = false;
    uint64_t val1 = 0;
    uint64_t val2 = 0;
    uint64_t maxBufVal = 0;
    int sendIncrReport = options->sendIncrementalReport;


    int includePriorityGroups[BVIEW_ASIC_MAX_PRIORITY_GROUPS] = { 0 };
    int port = 0, priGroup = 0;
    char portStr[JSON_MAX_NODE_LENGTH] = { 0 };

    char *ippgTemplate = " { \"realm\": \"ingress-port-priority-group\", \"%s\": [ ";
    char *ippgPortTemplate = " { \"port\": \"%s\", \"data\": [ ";
    char *ippgPortGroupTemplate = " [  %d , %" PRIu64 " , %" PRIu64 " ] ,";

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - IPPG data \n");

    /* copying the header . Pointer and Length adjustments are handled by the macro */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, ippgTemplate, "data");

    /* For each port, and for each priority group in that port,
     *  1. attempt to see if this port needs to be reported.
     *  2. create the report.
     */
    for (port = 1; port <= asic->numPorts; port++)
    {
      /* check if the trigger report request should contain snap shot */
        if ((port != options->triggerInfo.port) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
        {
          continue;
        }
        includePort = false;
        sal_memset (&includePriorityGroups[0], 0, sizeof (includePriorityGroups));

        /* lets see if this port needs to be included in the report at all */
        for (priGroup = 1; priGroup <= asic->numPriorityGroups; priGroup++)
        {
      /* check if the trigger report request should contain snap shot */
           if ((priGroup-1 != options->triggerInfo.queue) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
           {
             continue;
           }
            /* By default, we plan to include the pri-group */
            includePriorityGroups[priGroup - 1] = 1;

            if (true == sendIncrReport)
            {
            /* If there is no traffic reported for this priority group, ignore it */
            if ((previous == NULL) &&
                (current->iPortPg.data[port - 1][priGroup - 1].umShareBufferCount == 0) &&
                (current->iPortPg.data[port - 1][priGroup - 1].umHeadroomBufferCount == 0) )
            {
                includePriorityGroups[priGroup - 1] = 0;
                continue;
            }
           }

            /* If this is snapshot report, include the port in the data  */
            if (previous == NULL)
            {
                includePort = true;
                continue;
            }

            /* if there is traffic reported since the last snapshot, we can't ignore this priority group */
            if (previous->iPortPg.data[port - 1][priGroup - 1].umShareBufferCount
                != current->iPortPg.data[port - 1][priGroup - 1].umShareBufferCount)
            {
                includePort = true;
                continue;
            }

            if (previous->iPortPg.data[port - 1][priGroup - 1].umHeadroomBufferCount
                != current->iPortPg.data[port - 1][priGroup - 1].umHeadroomBufferCount)
            {
                includePort = true;
                continue;
            }

            /* since there is no reason to include the group, we can ignore it*/
            includePriorityGroups[priGroup - 1] = 0;
        }

        /* if this port needs not be reported, then we move to next port */
        if (includePort == false)
        {
            continue;
        }

          /* convert the port to an external representation */
          sal_memset(&portStr[0], 0, JSON_MAX_NODE_LENGTH);
          JSON_PORT_MAP_TO_NOTATION(port, asicId, &portStr[0]);

          /* Now that this port needs to be included in the report, copy the header */
          _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, ippgPortTemplate, &portStr[0]);

        /* for each priority-group, prepare the data */
        for (priGroup = 1; priGroup <= asic->numPriorityGroups; priGroup++)
        {
          /* we ignore if there is no data to be reported */
          if (includePriorityGroups[priGroup - 1] == 0)
            continue;

          val1 = current->iPortPg.data[port - 1][priGroup - 1].umShareBufferCount;
          maxBufVal = options->bst_max_buffers_ptr->iPortPg.data[port - 1][priGroup - 1].umShareMaxBuf;

          bst_json_convert_data(options, asic, &val1, maxBufVal);

          maxBufVal = options->bst_max_buffers_ptr->iPortPg.data[port - 1][priGroup - 1].umHeadroomMaxBuf;
          val2 = current->iPortPg.data[port - 1][priGroup - 1].umHeadroomBufferCount;
          bst_json_convert_data(options, asic, &val2, maxBufVal);


          /* add the data to the report */
          _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
              ippgPortGroupTemplate, priGroup-1, val1, val2);
        }

          /* adjust the buffer to remove the last ',' */
          buffer = buffer - 1;
          remLength += 1;
          *length -= 1;

        /* add the "] } ," for the next port */
        _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                      "] } ," );
    }

    /* adjust the buffer to remove the last ',' */
    buffer = buffer - 1;
    remLength += 1;
    *length -= 1;

    /* add the "] } ," for the next 'realm' */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                  "] } ," );

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - IPPG data Complete \n");

    return INSIGHT_STATUS_SUCCESS;

}


/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - ingress-port-service-pool.
 *
 *********************************************************************/
static INSIGHT_STATUS _jsonencode_report_ingress_ipsp ( char *buffer, int asicId,
                                                     const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                                     const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                                     const BSTJSON_REPORT_OPTIONS_t *options,
                                                     const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                                     int bufLen,
                                                     int *length)
{
    int remLength = bufLen;
    int actualLength  = 0;
    bool includePort = false;
    uint64_t val = 0;
    uint64_t maxBufVal = 0;
    int sendIncrReport = options->sendIncrementalReport;

    int includeServicePool[BVIEW_ASIC_MAX_SERVICE_POOLS] = { 0 };
    int port = 0, pool = 0;
    char portStr[JSON_MAX_NODE_LENGTH] = { 0 };


    char *ipspTemplate = " { \"realm\": \"ingress-port-service-pool\", \"%s\": [ ";
    char *ipspPortTemplate = " { \"port\": \"%s\", \"data\": [ ";
    //char *ippgPortGroupTemplate = " { \"pg\" : %d , \"um-share\": %" PRIu64 " , \"um-headroom\": %" PRIu64 " } ,";
    //char *ippgPortGroupTemplate = " { \"pg\" : %d , \"data\": [ %" PRIu64 " , %" PRIu64 " ] } ,";
    char *ipspServicePoolTemplate = " [  %d , %" PRIu64 " ] ,";

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - IPSP data \n");

    /* copying the header . Pointer and Length adjustments are handled by the macro */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, ipspTemplate, "data");

    /* For each port, and for each priority group in that port,
     *  1. attempt to see if this port needs to be reported.
     *  2. create the report.
     */
    for (port = 1; port <= asic->numPorts; port++)
    {
      /* check if the trigger report request should contain snap shot */
        if ((port != options->triggerInfo.port) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
        {
          continue;
        }
        includePort = false;
        sal_memset (&includeServicePool[0], 0, sizeof (includeServicePool));

        /* lets see if this port needs to be included in the report at all */
        for (pool = 1; pool <= asic->numServicePools; pool++)
        {
      /* check if the trigger report request should contain snap shot */
           if ((pool-1 != options->triggerInfo.queue) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
           {
             continue;
           }
            /* By default, we plan to include the pool */
            includeServicePool[pool - 1] = 1;

            if (true == sendIncrReport)
            {
              /* If there is no traffic reported for this priority group, ignore it */
              if ((previous == NULL) &&
                  (current->iPortSp.data[port - 1][pool - 1].umShareBufferCount == 0))
              {
                includeServicePool[pool - 1] = 0;
                continue;
              }
            }

            /* If this is snapshot report, include the port in the data  */
            if (previous == NULL)
            {
                includePort = true;
                continue;
            }

            /* if there is traffic reported since the last snapshot, we can't ignore this pool */
            if (previous->iPortSp.data[port - 1][pool - 1].umShareBufferCount
                != current->iPortSp.data[port - 1][pool - 1].umShareBufferCount)
            {
                includePort = true;
                continue;
            }

            /* since there is no reason to include the pool, we can ignore it*/
            includeServicePool[pool - 1] = 0;
        }

        /* if this port needs not be reported, then we move to next port */
        if (includePort == false)
        {
            continue;
        }

          /* convert the port to an external representation */
          sal_memset(&portStr[0], 0, JSON_MAX_NODE_LENGTH);
          JSON_PORT_MAP_TO_NOTATION(port, asicId, &portStr[0]);

          /* Now that this port needs to be included in the report, copy the header */
          _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, ipspPortTemplate,  &portStr[0]);

        /* for each priority-group, prepare the data */
        for (pool = 1; pool <= asic->numServicePools; pool++)
        {
            /* we ignore if there is no data to be reported */
            if (includeServicePool[pool - 1] == 0)
                continue;

            val = current->iPortSp.data[port - 1][pool - 1].umShareBufferCount;
            maxBufVal = options->bst_max_buffers_ptr->iPortSp.data[port - 1][pool - 1].umShareMaxBuf;
            bst_json_convert_data(options, asic, &val, maxBufVal);

            /* add the data to the report */
            _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                          ipspServicePoolTemplate, pool-1,val);
        }

        /* adjust the buffer to remove the last ',' */
        buffer = buffer - 1;
        remLength += 1;
        *length -= 1;

        /* add the "] } ," for the next port */
        _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                      "] } ," );
    }

    /* adjust the buffer to remove the last ',' */
    buffer = buffer - 1;
    remLength += 1;
    *length -= 1;

    /* add the "] } ," for the next 'realm' */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                  "] } ," );

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - IPSP data Complete \n");

    return INSIGHT_STATUS_SUCCESS;

}


/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - ingress-service-pool.
 *
 *********************************************************************/
static INSIGHT_STATUS _jsonencode_report_ingress_sp ( char *buffer, int asicId,
                                                   const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                                   const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                                   const BSTJSON_REPORT_OPTIONS_t *options,
                                                   const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                                   int bufLen,
                                                   int *length)
{
    int remLength = bufLen;
    int actualLength  = 0;
    int pool = 0;
    uint64_t val = 0;
    uint64_t maxBufVal = 0;
    int sendIncrReport = options->sendIncrementalReport;

    char *ispTemplate = " { \"realm\": \"ingress-service-pool\", \"%s\": [ ";
    char *ispServicePoolTemplate = " [  %d , %" PRIu64 " ] ,";

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - ISP data \n");

    /* copying the header . Pointer and Length adjustments are handled by the macro */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, ispTemplate, "data");

    /* For each service pool, check if there is a difference, and create the report. */
    for (pool = 1; pool <= asic->numServicePools; pool++)
    {
      /* check if the trigger report request should contain snap shot */
        if ((pool-1 != options->triggerInfo.queue) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
        {
          continue;
        }

        if (true == sendIncrReport)
        {
        /* lets see if this pool needs to be included in the report at all */
        /* if this pool needs not be reported, then we move to next pool */
         if ((previous == NULL) &&
            (current->iSp.data[pool-1].umShareBufferCount == 0))
            continue;
        }

        if ((previous != NULL) &&
            (previous->iSp.data[pool-1].umShareBufferCount == current->iSp.data[pool-1].umShareBufferCount))
            continue;

             val = current->iSp.data[pool-1].umShareBufferCount;
             maxBufVal = options->bst_max_buffers_ptr->iSp.data[pool-1].umShareMaxBuf;
             bst_json_convert_data(options, asic, &val, maxBufVal);
        /* Now that this pool needs to be included in the report, add the data to report */
        _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                      ispServicePoolTemplate, pool-1, val);

    }

    /* adjust the buffer to remove the last ',' */
    buffer = buffer - 1;
    remLength += 1;
    *length -= 1;

    /* add the "] } ," for the next 'realm' */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                  "] } ," );

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS - ISP data Complete \n");

    return INSIGHT_STATUS_SUCCESS;

}



/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - ingress part.
 *
 *********************************************************************/
INSIGHT_STATUS _jsonencode_report_ingress ( char *buffer,
                                         int asicId,
                                         const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                         const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                         const BSTJSON_REPORT_OPTIONS_t *options,
                                         const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                         int bufLen,
                                         int *length)
{
    INSIGHT_STATUS status;
    int tempLength = 0;

    *length = 0;

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS data \n");

    /* If Port-PriorityGroup realm is asked for, lets encode the corresponding data */
    if (options->includeIngressPortPriorityGroup)
    {
        tempLength = 0;
        status = _jsonencode_report_ingress_ippg(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

    /* If Port-ServicePool realm is asked for, lets encode the corresponding data */
    if (options->includeIngressPortServicePool)
    {
        tempLength = 0;
        status = _jsonencode_report_ingress_ipsp(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);
        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

    /* If ServicePool realm is asked for, lets encode the corresponding data */
    if (options->includeIngressServicePool)
    {
        tempLength = 0;
        status = _jsonencode_report_ingress_sp(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);
        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

    if (tempLength != 0)
    {
        if (! (options->includeEgressCpuQueue ||
               options->includeEgressMcQueue ||
               options->includeEgressPortServicePool ||
               options->includeEgressRqeQueue ||
               options->includeEgressServicePool ||
               options->includeEgressUcQueue ||
               options->includeEgressUcQueueGroup ))
        {
            *(length) -= 1;
        }
    }

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding INGRESS data complete \n");


    return INSIGHT_STATUS_SUCCESS;
}



/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - egress UC Queue.
 *
 *********************************************************************/
static INSIGHT_STATUS _jsonencode_report_egress_ucq ( char *buffer, int asicId,
                                                   const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                                   const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                                   const BSTJSON_REPORT_OPTIONS_t *options,
                                                   const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                                   int bufLen,
                                                   int *length)
{
    int remLength = bufLen;
    int actualLength  = 0;
    int queue = 0;
    uint64_t val = 0;
    uint64_t maxBufVal = 0;
    int sendIncrReport = options->sendIncrementalReport;

    char *realmTemplate = " { \"realm\": \"egress-uc-queue\", \"%s\": [ ";
    char *dataTemplate = " [  %d , \"%s\" , %" PRIu64 " ] ,";
    char portStr[JSON_MAX_NODE_LENGTH] = { 0 };


    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding EGRESS - UC Queue data \n");

    /* copying the header . Pointer and Length adjustments are handled by the macro */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length, realmTemplate, "data");

    /* For each unicast queues, check if there is a difference, and create the report. */
    for (queue = 1; queue <= asic->numUnicastQueues; queue++)
    {
      /* check if the trigger report request should contain snap shot */
        if ((queue-1 != options->triggerInfo.queue) &&
            (false == options->sendSnapShotOnTrigger) &&
            (true == options->reportTrigger))
        {
          continue;
        }


      if (true == sendIncrReport)
      {
        /* lets see if this queue needs to be included in the report at all */
        /* if this queue needs not be reported, then we move to next queue */
        if ((NULL == previous) &&
            (current->eUcQ.data[queue - 1].ucBufferCount == 0))
            continue;
      }
        if ((previous != NULL) &&
            (previous->eUcQ.data[queue - 1].ucBufferCount == current->eUcQ.data[queue - 1].ucBufferCount))
            continue;

        /* convert the port to an external representation */
        sal_memset(&portStr[0], 0, JSON_MAX_NODE_LENGTH);
        JSON_PORT_MAP_TO_NOTATION(current->eUcQ.data[queue - 1].port, asicId, &portStr[0]);

         val = current->eUcQ.data[queue - 1].ucBufferCount;
        maxBufVal = options->bst_max_buffers_ptr->eUcQ.data[queue - 1].ucMaxBuf;
         bst_json_convert_data(options, asic, &val, maxBufVal);
        /* Now that this ucq needs to be included in the report, add the data to report */
        _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                      dataTemplate, queue-1,
                                                      &portStr[0],
                                                      val
                                                      );
    }

    /* adjust the buffer to remove the last ',' */
    buffer = buffer - 1;
    remLength += 1;
    *length -= 1;

    /* add the "] } ," for the next 'realm' */
    _JSONENCODE_COPY_FORMATTED_STRING_AND_ADVANCE(actualLength, buffer, remLength, length,
                                                  "] } ," );

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding EGRESS - UC Queue data Complete \n");

    return INSIGHT_STATUS_SUCCESS;
}




/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API - egress part.
 *
 *********************************************************************/
INSIGHT_STATUS _jsonencode_report_egress ( char *buffer, int asicId,
                                        const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                        const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                        const BSTJSON_REPORT_OPTIONS_t *options,
                                        const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                        int bufLen,
                                        int *length)
{
    INSIGHT_STATUS status;
    int tempLength = 0;

    *length = 0;

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding EGRESS data \n");

#if 0
    /* If CPU realm is asked for, lets encode the corresponding data */

    if (options->includeEgressCpuQueue)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_cpuq(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }


    /* If Egress Multicast queue realm is asked for, lets encode the corresponding data */
    if (options->includeEgressMcQueue)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_mcq(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }



    /* If Egress Port - Service Pool realm is asked for, lets encode the corresponding data */

    if (options->includeEgressPortServicePool)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_epsp(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

    /* If Egress RQE queue realm is asked for, lets encode the corresponding data */
    if (options->includeEgressRqeQueue)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_rqeq(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

    /* If Egress Service Pool realm is asked for, lets encode the corresponding data */
    if (options->includeEgressServicePool)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_sp(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }
  #endif
    /* If Egress Unicast queue realm is asked for, lets encode the corresponding data */
    if (options->includeEgressUcQueue)
    {

        tempLength = 0;
        status = _jsonencode_report_egress_ucq(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }
    }

#if 0

    /* If Egress Unicast queue group realm is asked for, lets encode the corresponding data */
    if (options->includeEgressUcQueueGroup)
    {
        tempLength = 0;
        status = _jsonencode_report_egress_ucqg(buffer, asicId, previous, current, options, asic, bufLen, &tempLength);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        if (tempLength != 0)
        {
            bufLen -= (tempLength);
            buffer += (tempLength);
            *(length) += (tempLength);
        }

    }

  #endif

    if (tempLength != 0)
    {
        *(length) -= 1;
    }

    LOG_POST(INSIGHT_LOG_DEBUG, "BST-JSON-Encoder : (Report) Encoding EGRESS data complete \n");

    return INSIGHT_STATUS_SUCCESS;
}




static INSIGHT_STATUS bstjson_realm_to_indices_get(char *realm, char *index1, char *index2)
{
  static BSTJSON_REALM_INDEX_t bst_realm_indices_map [] = {
    {"device" ,NULL, NULL},
    {"ingress-service-pool", "service-pool", NULL},
    {"ingress-port-service-pool", "port", "service-pool" },
    {"ingress-port-priority-group", "port", "priority-group"},
    {"egress-port-service-pool", "port", "service-pool"},
    {"egress-service-pool", "service-pool", NULL},
    {"egress-uc-queue", "queue", NULL},
    {"egress-uc-queue-group", "queue-group", NULL},
    {"egress-mc-queue", "queue", NULL},
    {"egress-cpu-queue", "queue", NULL},
    {"egress-rqe-queue", "queue", NULL}
  };

  unsigned int i;

  if ((NULL == realm)||
      (NULL == index1) ||
      (NULL == index2))
  {
    return INSIGHT_STATUS_INVALID_PARAMETER;
  }

  for (i = 0; i < 11; i++)
  {
    if (0 == sal_strcmp(realm, bst_realm_indices_map[i].realm))
    {
      if (NULL != bst_realm_indices_map[i].index1)
      {
        sal_strncpy(index1, bst_realm_indices_map[i].index1, sal_strlen(bst_realm_indices_map[i].index1));
      }

      if (NULL != bst_realm_indices_map[i].index2)
      {
        sal_strncpy(index2, bst_realm_indices_map[i].index2, sal_strlen(bst_realm_indices_map[i].index2));
      }

      return INSIGHT_STATUS_SUCCESS;
    }
  }

  return INSIGHT_STATUS_FAILURE;

}

INSIGHT_STATUS bstjson_encode_trigger_realm_index_info(char *buffer, int asicId,
                                                     int bufLen,int *length,
                                                     char *index, int port, int queue)
{
  char portStr[JSON_MAX_NODE_LENGTH] = { 0 };

  char * portTemplate = "\"port\" : \"%s\",";
  char * indexTemplate = "\"%s\" : %d,";

  if (index != NULL)
  {
    if (0 == sal_strcmp("port", index))
    {
      /* convert the port to an external representation */
      sal_memset(&portStr[0], 0, JSON_MAX_NODE_LENGTH);
      JSON_PORT_MAP_TO_NOTATION(port, asicId, &portStr[0]);
       *length = snprintf(buffer, bufLen, portTemplate, &portStr[0]);
    }
    else
    {
       *length = snprintf(buffer, bufLen, indexTemplate, index, queue);
    }
    return INSIGHT_STATUS_SUCCESS;
  }
    return INSIGHT_STATUS_FAILURE;

}


/******************************************************************
 * @brief  Creates a JSON buffer using the supplied data for the
 *         "get-bst-report" REST API.
 *
 * @param[in]   asicId      ASIC for which this data is being encoded.
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
 *           bstjson_memory_free(). Failing to do so leads to memory leaks
 *********************************************************************/



INSIGHT_STATUS bstjson_encode_get_bst_report ( int asicId,
                                            int method,
                                            const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *previous,
                                            const INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *current,
                                            const BSTJSON_REPORT_OPTIONS_t *options,
                                            const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                            const INSIGHT_TIME_t *time,
                                            uint8 **pJsonBuffer
                                            )
{
    char *jsonBuf, *start;
    INSIGHT_STATUS status;
    int bufferLength = BSTJSON_MEMSIZE_REPORT;
    int tempLength = 0;

    INSIGHT_TIME_t report_time;
    struct tm *timeinfo;
    char timeString[64];
    char asicIdStr[JSON_MAX_NODE_LENGTH] = { 0 };
    char index1[256];
    char index2[256];



    char *getBstReportStart = " { \
\"jsonrpc\": \"2.0\",\
\"method\": \"%s\",\
\"asic-id\": \"%s\",\
\"version\": \"%d\",\
\"time-stamp\": \"%s\",\
\"report\": [ \
";

    char *getBstTriggerReportStart = " { \
\"jsonrpc\": \"2.0\",\
\"method\": \"%s\",\
\"asic-id\": \"%s\",\
\"version\": \"%d\",\
\"time-stamp\": \"%s\",\
\"realm\": \"%s\",\
\"counter\": \"%s\",\
";


    LOG_POST(INSIGHT_LOG_INFO, "BST-JSON-Encoder : Request for Get-Bst-Report \n");

    /* Validate Input Parameters */
    _JSONENCODE_ASSERT (options != NULL);
    _JSONENCODE_ASSERT (current != NULL);
    _JSONENCODE_ASSERT (time != NULL);
    _JSONENCODE_ASSERT (asic != NULL);

    /* obtain the time */
    sal_memset(&timeString, 0, sizeof (timeString));
    report_time = *(INSIGHT_TIME_t *) time;
    timeinfo = localtime(&report_time);
    strftime(timeString, 64, "%Y-%m-%d - %H:%M:%S ", timeinfo);

    /* allocate memory for JSON */
    jsonBuf = mem_malloc(MEM_APP_MODULE, BSTJSON_MEMSIZE_REPORT);

    if (!jsonBuf)
    {
        return INSIGHT_STATUS_INVALID_MEMORY;
    }

    start = jsonBuf;

    /* clear the buffer */
    sal_memset(jsonBuf, 0, BSTJSON_MEMSIZE_REPORT);

    /* convert asicId to external  notation */
    JSON_ASIC_ID_MAP_TO_NOTATION(asicId, &asicIdStr[0]);

    /* fill the header */
    /* encode the JSON */

    if (options->reportTrigger == false)
    {
      tempLength = snprintf(jsonBuf, bufferLength, getBstReportStart,
          (options->reportThreshold == true) ? "get-bst-thresholds" :"get-bst-report",
          &asicIdStr[0], INSIGHT_JSON_VERSION, timeString);
      bufferLength -= tempLength;
      jsonBuf += tempLength;
    }
    else
    {
      sal_memset(index1, 0, sizeof(index1));
      sal_memset(index2, 0, sizeof(index2));
      if (INSIGHT_STATUS_SUCCESS != bstjson_realm_to_indices_get((char *)options->triggerInfo.realm, &index1[0], &index2[0]))
      {
        return INSIGHT_STATUS_INVALID_PARAMETER;
      }
      tempLength = snprintf(jsonBuf, bufferLength, getBstTriggerReportStart,
          "trigger-report",
          &asicIdStr[0], INSIGHT_JSON_VERSION, timeString, options->triggerInfo.realm, options->triggerInfo.counter);

      jsonBuf += tempLength;
      bufferLength -= tempLength;
      if (0 != index1[0])
      {
        status = bstjson_encode_trigger_realm_index_info(jsonBuf, asicId, bufferLength, &tempLength, &index1[0],
            options->triggerInfo.port, options->triggerInfo.queue);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);
        bufferLength -= tempLength;
        jsonBuf += tempLength;
      }


      if (0 != index2[0])
      {
        status = bstjson_encode_trigger_realm_index_info(jsonBuf, asicId, bufferLength, &tempLength, &index2[0],
            options->triggerInfo.port, options->triggerInfo.queue);
        _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);
        bufferLength -= tempLength;
        jsonBuf += tempLength;
      }

      tempLength = snprintf(jsonBuf, bufferLength, "\"report\" : [" );
      bufferLength -= tempLength;
      jsonBuf += tempLength;
    }

    /* get the device report */
    status = _jsonencode_report_device(jsonBuf, previous, current, options, asic, bufferLength, &tempLength);
    if (status != INSIGHT_STATUS_SUCCESS)
    {
      *pJsonBuffer = (uint8 *) start;
    }
    _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

    if (tempLength)
    {
        bufferLength -= tempLength;
        jsonBuf += tempLength;

        tempLength = snprintf(jsonBuf, bufferLength, " ,");

        bufferLength -= tempLength;
        jsonBuf += tempLength;
    }

    /* if any of the ingress encodings are required, add them to report */
    if (options->includeIngressPortPriorityGroup ||
        options->includeIngressPortServicePool ||
        options->includeIngressServicePool)
    {
      status = _jsonencode_report_ingress(jsonBuf, asicId, previous, current, options, asic, bufferLength, &tempLength);
      if (status != INSIGHT_STATUS_SUCCESS)
      {
        *pJsonBuffer = (uint8 *) start;
      }
      _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        /* adjust the buffer */
        bufferLength -= (tempLength);
        jsonBuf += (tempLength);

    }

    /* if any of the egress encodings are required, add them to report */
    if (options->includeEgressCpuQueue ||
        options->includeEgressMcQueue ||
        options->includeEgressPortServicePool ||
        options->includeEgressRqeQueue ||
        options->includeEgressServicePool ||
        options->includeEgressUcQueue ||
        options->includeEgressUcQueueGroup )
    {
      status = _jsonencode_report_egress(jsonBuf, asicId, previous, current, options, asic, bufferLength, &tempLength);
      if (status != INSIGHT_STATUS_SUCCESS)
      {
        *pJsonBuffer = (uint8 *) start;
      }
      _JSONENCODE_ASSERT_ERROR((status == INSIGHT_STATUS_SUCCESS), status);

        /* adjust the buffer */
        bufferLength -= (tempLength);
        jsonBuf += (tempLength);

    }

    /* finalizing the report */

    bufferLength -= 1;
    jsonBuf -= 1;

    if (jsonBuf[0] == 0)
    {
        bufferLength -= 1;
        jsonBuf--;
    }

    tempLength = snprintf(jsonBuf, bufferLength, " ] } ");

    *pJsonBuffer = (uint8 *) start;

    LOG_POST(INSIGHT_LOG_INFO, "BST-JSON-Encoder : Request for Get-Bst-Report Complete [%d] bytes \n", (int)sal_strlen(start));

    LOG_POST(INSIGHT_LOG_INFO, "BST-JSON-Encoder : %s \n", start);


    return INSIGHT_STATUS_SUCCESS;
}
extern INSIGHT_STATUS
bstjson_encode_get_cgsn_drop_ctrs ( int asicId,
                                            const void *in_ptr,
                                            const INSIGHT_ASIC_CAPABILITIES_t *asic,
                                            uint8 **pJsonBuffer
                                            )
{
    return 0;
}

