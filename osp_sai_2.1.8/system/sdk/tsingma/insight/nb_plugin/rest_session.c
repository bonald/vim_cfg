

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "insight.h"
#include "rest.h"

/******************************************************************
 * @brief  initialize sessions 
 *
 * @param[in]   rest      REST context for operation
 *
 * @retval   INSIGHT_STATUS_SUCCESS on successful initialization
 * @retval   INSIGHT_STATUS_FAILURE otherwise
 * 
 * @note     
 *********************************************************************/
INSIGHT_STATUS rest_sessions_init(REST_CONTEXT_t *context)
{
    int i = 0;
    REST_SESSION_t *session;

    /* Just basic initialization */

    for (i = 0; i < REST_MAX_SESSIONS; i++)
    {
        session = &context->sessions[i];
        memset(session, 0, sizeof (REST_SESSION_t));
        session->inUse = false;
    }

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  allocates an available session (returns the index) 
 *
 * @param[in]   rest      REST context for operation
 *
 * @retval   INSIGHT_STATUS_SUCCESS on successful allocation
 * @retval   INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE otherwise
 * 
 * @note     
 *********************************************************************/
INSIGHT_STATUS rest_allocate_session(REST_CONTEXT_t *context, int *sessionId)
{
    int i = 0;
    for (i = 0; i < REST_MAX_SESSIONS; i++)
    {
        if (context->sessions[i].inUse == false)
        {
            *sessionId = i;
            memset(&context->sessions[i], 0, sizeof (REST_SESSION_t));
            return INSIGHT_STATUS_SUCCESS;
        }
    }

    return INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE;
}

/******************************************************************
 * @brief  Checks if the supplied session is valid 
 *
 * @param[in]   session      session to be validated
 *
 *********************************************************************/
INSIGHT_STATUS rest_session_validate(REST_CONTEXT_t *context, REST_SESSION_t *session)
{
    int i = 0;
    
    for (i = 0; i < REST_MAX_SESSIONS; i++)
    {
        if (&context->sessions[i] == session)
            return INSIGHT_STATUS_SUCCESS;
    }
    
    return INSIGHT_STATUS_RESOURCE_NOT_AVAILABLE;
}

/******************************************************************
 * @brief  dump a session on console/log
 *
 * @param[in]   session      data to be dumped
 *
 *********************************************************************/
void rest_session_dump(REST_SESSION_t *session)
{
    _REST_LOG(_REST_DEBUG_TRACE, "Session : HTTPMethod : %s - REST Method : %s - URL : %s - Length %d \n",
              session->httpMethod, session->restMethod, session->url, session->length);
}


