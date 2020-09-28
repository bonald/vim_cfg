/*****************************************************************************
  *
  * Copyright � 2016 Broadcom.  The term "Broadcom" refers
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "rest.h"




/******************************************************************
 * @brief  Initializes configuration, reads it from file or assumes defaults.
 *
 * @param[in]   rest      REST context for operation
 *                           
 * @retval   INSIGHT_STATUS_SUCCESS  when configuration is initialized successfully
 *
 * @note     
 *********************************************************************/
INSIGHT_STATUS rest_config_init(REST_CONTEXT_t *rest)
{
    pthread_mutex_t *rest_mutex = NULL;

    /* aim to read */
    _REST_LOG(_REST_DEBUG_TRACE, "REST : Configuring ...");
 
    /* create the mutex for agent_config data */
    rest_mutex = &rest->config_mutex;
    pthread_mutex_init (rest_mutex, NULL);



 strncpy(&rest->config.clientIp[0], SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT, 20);

    /* setup default client port */
    rest->config.clientPort = SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT;

    /* setup default local port */
    rest->config.localPort = SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT;

#if 0	
    /* get the client port */
    system_agent_client_port_get(&rest->config.clientPort);

    /* get the client ip addres */
    system_agent_client_ipaddr_get(&rest->config.clientIp[0]);

    /* get the local port */
   system_agent_port_get(&rest->config.localPort);
#endif

    _REST_LOG(_REST_DEBUG_TRACE, "REST : Configuration Complete");

    return INSIGHT_STATUS_SUCCESS;
}

