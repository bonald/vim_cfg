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

#ifndef INCLUDE_INSIGHT_H
#define INCLUDE_INSIGHT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sal.h"
#include "ctcs_api.h"

/** Maximum length of network os string*/
#define INSIGHT_NETWORK_OS_LEN_MAX       32

/** Maximum length of feature name string*/
#define INSIGHT_MAX_FEATURE_NAME_LEN       32

/** Maximum supported asics on a platform */
#define INSIGHT_MAX_ASICS   1

/** Maximum supported features */
#define INSIGHT_MAX_FEATURES              16

/** Max api cmds supported per feature */
#define INSIGHT_MAX_API_CMDS_PER_FEATURE  16

/** json version */
#define INSIGHT_JSON_VERSION   3

#define INSIGHT_ASIC_MAX_PORTS  64

#define INSIGHT_ASIC_MAX_QUEUES  512

#define INSIGHT_JSON_MAX_NODE_LENGTH 128
#define INSIGHT_JSON_MEMSIZE_RESPONSE  1024


#define INSIGHT_MSG_QUEUE_ID_TO_BST  0x100
#define INSIGHT_MSG_QUEUE_ID_TO_BST_TRIGGER  0x108

#define INSIGHT_MSG_QUEUE_ID_TO_SYSTEM  0x300

/** agent software name */
#define INSIGHT_AGENT_SW_NAME   "broadview"
    /** Indicates various status codes.
    *  This status codes are typically used as return values for function
    *  calls. However, they can be used elsewhere as well to indicate
    *  execution status.
    */

    typedef enum _insight_status
    {
        INSIGHT_STATUS_SUCCESS = 0, /**< Successful completion of request */
        INSIGHT_STATUS_FAILURE, /**< Unsuccessful completion of request */
        INSIGHT_STATUS_INVALID_PARAMETER,
        INSIGHT_STATUS_UNSUPPORTED,
        INSIGHT_STATUS_OUTOFMEMORY,
        INSIGHT_STATUS_TIMEOUT,
        INSIGHT_STATUS_NOTREADY,
        INSIGHT_STATUS_TABLE_FULL,
        INSIGHT_STATUS_INVALID_COMMAND,
        INSIGHT_STATUS_INVALID_JSON,
        INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE,
        INSIGHT_STATUS_INVALID_MEMORY,
        INSIGHT_STATUS_OUTOFRANGE,
        INSIGHT_STATUS_INIT_FAILED,
        INSIGHT_STATUS_INVALID_ID,
        INSIGHT_STATUS_FEATURE_NOT_ENABLED,
        INSIGHT_STATUS_NOT_CONFIGURED,
        INSIGHT_STATUS_ALREADY_CONFIGURED,
        INSIGHT_STATUS_DUPLICATE  /** If the entry is alreadey present */
    } INSIGHT_STATUS;



    typedef enum _insight_feature_id_
    {
        /** enum to represent system feature */
        INSIGHT_FEATURE_SYSTEM = (0x1 << 0),
        /** enum to represent BST feature */
        INSIGHT_FEATURE_BST = (0x1 << 1),
        /** enum to represent Packet Trace feature */
        INSIGHT_FEATURE_PACKET_TRACE = (0x1 << 2),
        /** enum to represent Packet Trace feature */
        INSIGHT_FEATURE_BHD = (0x1 << 3),
        /** enum to represent Live Packet Trace feature */
        INSIGHT_FEATURE_LIVE_PT = (0x1 << 4)

    } INSIGHT_FEATURE_ID;

typedef struct _switch_asic_info_
{
  char          asic_notation[32];
  uint8          asicType;
  uint8          numPorts;
}  insight_switch_asic_info_t;



    /** the web server invokes the handler associated with the incoming REST API  *
      * the incoming API is containing in the jsonBuffer.                         *
      * The cookie is used by the web server to hold any context associated with   *
      * the request. It is passed back the handler when some data needs to be sent back */
    typedef INSIGHT_STATUS(*INSIGHT_REST_API_HANDLER_t) (void *cookie, char *jsonBuffer,  int bufLength);

    /** Definition of an REST API */
    typedef struct _insight_rest_api_
    {
        /** Api string in the Json buffer */
        char *apiString;
        /** Handler associated with the corresponding api string */
        INSIGHT_REST_API_HANDLER_t    handler;
    }INSIGHT_REST_API_t;

typedef INSIGHT_STATUS(*INSIGHT_APP_API_HANDLER_t) (void * msg_data);

/** Definition of an BST API */
typedef struct _insight_app_api_
{
  /** bst command */
  int  command;
  /** Handler associated with the corresponding bst command */
  INSIGHT_APP_API_HANDLER_t    handler;
}insight_app_api_t;


/* Interface storage */
typedef struct
{
  uint32   value[INSIGHT_ASIC_MAX_PORTS/32];
} INSIGHT_PORT_MASK_t;


/* Interface storage */
typedef struct
{
  uint32   value[INSIGHT_ASIC_MAX_QUEUES/32];
} INSIGHT_QUEUE_MASK_t;


  /* SETMASKBIT turns on bit index # k in mask j. */
#define INSIGHT_SETMASKBIT(j, k)                                     \
                ((j).value[((k-1)/(8*sizeof(32)))]                \
                                                         |= 1 << ((k-1) % (8*sizeof(32))))


    /* CLRMASKBIT turns off bit index # k in mask j. */
#define INSIGHT_CLRMASKBIT(j, k)                                     \
               ((j).value[((k-1)/(8*sizeof(32)))]                 \
                                                      &= ~(1 << ((k-1) % (8*sizeof(32)))))


#define SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT   "10.10.25.117"
#define SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT   9070
#define SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT    6000

#define INSIGHT_TIME_BUFFER_SIZE       64

INSIGHT_STATUS system_timer_add(void * handler,timer_t * timerId, int timeInMilliSec,uint8 mode,void * param);
INSIGHT_STATUS system_timer_delete(timer_t timerId);
INSIGHT_STATUS system_timer_set(timer_t timerId,int timeInMilliSec,uint8 mode);



#endif /* INCLUDE_INSIGHT_H */
