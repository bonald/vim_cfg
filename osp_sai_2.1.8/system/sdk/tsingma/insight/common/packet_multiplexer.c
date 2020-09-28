
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "insight.h"
#include "log_api.h"
#include "packet_multiplexer.h"





#define        INSIGHT_PKT_MUX_MAX_HOOKS            5

typedef struct _pktMuxHooks_
{
  unsigned int appID;         
  char         appFuncName[INSIGHT_PKT_MUX_MAX_FUNC_NAME]; /* application function name */  
  INSIGHT_PKT_MUX_CALLBACK_t callbackFunc;        /* Function to be called at specified hook */
  unsigned int packetCount;
} pktMuxHooks_t;


typedef struct _pktMuxInfo_
{
  key_t key;
  /* message queue id for packet multiplexer */
  int recvMsgQid;
  /* pthread ID*/
  pthread_t pt_thread;
  /* Mutex for Packet Multiplexer*/
  pthread_mutex_t  pkt_mux_mutex;
   
  /* Database to Hold Application Registration Info*/
  pktMuxHooks_t   pktMuxHooks[INSIGHT_PKT_MUX_MAX_HOOKS];
} pktMuxInfo_t;


pktMuxInfo_t            pkt_mux_info;
char                    pkt_mux_Initialized = false;

  /* Macro to acquire lock */
#define PKT_MUX_LOCK_TAKE()                                                  \
  {                                                                          \
    if (0 != pthread_mutex_lock (&pkt_mux_info.pkt_mux_mutex))               \
    {                                                                        \
      return INSIGHT_STATUS_FAILURE;                                           \
    }                                                                        \
  }
  /* Macro to release lock*/
#define PKT_MUX_LOCK_GIVE()                                                  \
  {                                                                          \
    if (0 != pthread_mutex_unlock(&pkt_mux_info.pkt_mux_mutex))              \
    {                                                                        \
      return INSIGHT_STATUS_FAILURE;                                           \
    }                                                                        \
  }

/*********************************************************************
* @brief :   Packet Multiplexer thread.
*
* @param[in] : none
*
* @retval  : 
*            
*            
* @retval  : 
*
* @note  : This api is the processing thread of the packet multiplexer 
*          application. All the incoming requests are processed and 
*          notify the applciations registered. 
*
*********************************************************************/
INSIGHT_STATUS pkt_mux_main (void)
{
  INSIGHT_PACKET_MSG_t msg_data;
  int appIndex = 0;

  while (1)
  {
    if (-1 != (msgrcv (pkt_mux_info.recvMsgQid, &msg_data, sizeof (msg_data), 0, 0)))
    {
      for (appIndex = 0; appIndex < INSIGHT_PKT_MUX_MAX_HOOKS; appIndex++)
      {
        if (msg_data.appMask & pkt_mux_info.pktMuxHooks[appIndex].appID)
        {
          pkt_mux_info.pktMuxHooks[appIndex].callbackFunc(&msg_data);
        }
      }
    }
    else 
    {
      /* LOG a message*/  
    }
  } /* End of While */                                                
  return INSIGHT_STATUS_SUCCESS;
}   

/*********************************************************************
* @brief  De-Register with packet multiplexer
*
* @param    pkt_mux_hook @b{(input)} Pointer to application registration info
*
* @returns  INSIGHT_STATUS_SUCCESS  if successful de-registation
* @returns  INSIGHT_STATUS_FAILURE  if 1) hooks uninitialized,
*                                    2) or no registation present
*
* @notes    none
*
* @end
*********************************************************************/
INSIGHT_STATUS pkt_mux_packet_handler (void *cookie,
                                     INSIGHT_PACKET_MSG_t *msg)
{
  /* Process the packet and return */
  INSIGHT_PACKET_MSG_t msg_data;

  memcpy (&msg_data, msg, sizeof (msg_data));
  if (-1 == msgsnd (pkt_mux_info.recvMsgQid, &msg_data,
            sizeof (INSIGHT_PACKET_MSG_t), 0))
  {
    return INSIGHT_STATUS_FAILURE;
  }
  return INSIGHT_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Initialize the Packet Multiplexer hook structure
*
* @param    none
*
* @returns  INSIGHT_STATUS_SUCCESS
*
* @notes    none
*
* @end
*********************************************************************/
INSIGHT_STATUS pkt_mux_init()
{
  int recvMsgQid;
//  int cookie = 0;
  //INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

  /* locks */
  pthread_mutex_init (&pkt_mux_info.pkt_mux_mutex, NULL);

  pkt_mux_info.key = 0x103;
   /* Check if message queue is already exitsing */
  if (0 > (recvMsgQid = msgget(pkt_mux_info.key, IPC_EXCL | IPC_CREAT | 0777)))
  {
    if (EEXIST == errno)
    {
      /* get the already existing  message queue id for pt */
      if ((recvMsgQid = msgget (pkt_mux_info.key, IPC_CREAT | 0777)) < 0)
      {
        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "(Packet Multiplexer) Failed to create msgQ: error:%d\n", errno);
        return  INSIGHT_STATUS_FAILURE;
      }

      /* message queue exits . Delete the message queue */
      if  (msgctl(recvMsgQid, IPC_RMID, (struct msqid_ds *)NULL) <0)
      {
        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "(Packet Multiplexer) Failed to destroy msgQ id, error:%d\n", errno);
        return  INSIGHT_STATUS_FAILURE;
      }

      /* create the message queue for pt */
      if ((recvMsgQid = msgget (pkt_mux_info.key, IPC_CREAT | 0777)) < 0)
      {
        LOG_POST (INSIGHT_LOG_EMERGENCY,
            "(Packet Multiplexer) Failed to create  msgQ. error:%d\n", errno);
        return  INSIGHT_STATUS_FAILURE;
      }
    }
  }
  pkt_mux_info.recvMsgQid = recvMsgQid;

   /* create pthread for pt application */
  if (0 != pthread_create (&pkt_mux_info.pt_thread, NULL,
                           (void *) &pkt_mux_main, NULL))
  {
    LOG_POST (INSIGHT_LOG_EMERGENCY,
              "(Packet Multiplexer) Thread creation failed error:%d\r\n", errno);
    return  INSIGHT_STATUS_FAILURE;
  }
#if 0
  /* Registration with silicon for CPU packets*/
  rv = sbapi_system_packet_rx_register (0, pkt_mux_packet_handler, 
                                        "pkt_mux_packet_handler", &cookie);
  if (INSIGHT_STATUS_SUCCESS != rv)
  {
    LOG_POST (INSIGHT_LOG_EMERGENCY,
           "(Packet Multiplexer) Failed to Register with silicon for CPU packets");
    return INSIGHT_STATUS_FAILURE;
  }
  #endif
  
  pkt_mux_Initialized = true;
  return INSIGHT_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Register for a Packet
*
* @param    pkt_mux_hook @b{(input)} Pointer to application registration info
*
* @returns  INSIGHT_STATUS_SUCCESS    if registration is successful
* @returns  INSIGHT_STATUS_DUPLICATE  if application is already registered
* @returns  INSIGHT_STATUS_FAILURE    if packet multiplexer is not initialized
*                                  
*
* @notes    none
*
* @end
*********************************************************************/
INSIGHT_STATUS pkt_mux_register(pktMuxRegister_t *pkt_mux_hook)
{
  int index =0;

  if ((pkt_mux_Initialized == false) && 
      (pkt_mux_init() != INSIGHT_STATUS_SUCCESS))
  {
    return INSIGHT_STATUS_FAILURE;
  }

  PKT_MUX_LOCK_TAKE();

  for (index = 0; index < INSIGHT_PKT_MUX_MAX_HOOKS; index++)
  {
    if (pkt_mux_hook->appID == pkt_mux_info.pktMuxHooks[index].appID)
    {
      /*hook already registered */
      PKT_MUX_LOCK_GIVE();
      return INSIGHT_STATUS_DUPLICATE;
    }
    
    if (pkt_mux_info.pktMuxHooks[index].appID == 0)
    {
      break;
    }
  }
  if (index == INSIGHT_PKT_MUX_MAX_HOOKS)
  {
    PKT_MUX_LOCK_GIVE();
    return INSIGHT_STATUS_TABLE_FULL;
  } 

  strcpy(pkt_mux_info.pktMuxHooks[index].appFuncName,
         pkt_mux_hook->appFuncName);
  pkt_mux_info.pktMuxHooks[index].callbackFunc = 
                              pkt_mux_hook->callbackFunc;
  pkt_mux_info.pktMuxHooks[index].appID = 
                              pkt_mux_hook->appID;

  PKT_MUX_LOCK_GIVE();
  return INSIGHT_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  De-Register with packet multiplexer
*
* @param    pkt_mux_hook @b{(input)} Pointer to application registration info
*
* @returns  INSIGHT_STATUS_SUCCESS  if successful de-registation
* @returns  INSIGHT_STATUS_FAILURE  if 1) hooks uninitialized, 
*                                    2) or no registation present
*
* @notes    none
*
* @end
*********************************************************************/
INSIGHT_STATUS pkt_mux_deregister (pktMuxRegister_t *pkt_mux_hook)
{
  int index = 0;

  if (pkt_mux_Initialized == false)
  {
    return INSIGHT_STATUS_FAILURE;
  }

  PKT_MUX_LOCK_TAKE();
  for (index = 0; index < INSIGHT_PKT_MUX_MAX_HOOKS; index++)
  {
    if (pkt_mux_hook->appID == pkt_mux_info.pktMuxHooks[index].appID)
    {
      break;
    }
  }

  if (index == INSIGHT_PKT_MUX_MAX_HOOKS)
  {
    PKT_MUX_LOCK_GIVE();
    return INSIGHT_STATUS_FAILURE;
  }

  pkt_mux_info.pktMuxHooks[index].appID = 0;
  pkt_mux_info.pktMuxHooks[index].callbackFunc = NULL;
  memset (pkt_mux_info.pktMuxHooks[index].appFuncName, 0x00, 
          INSIGHT_PKT_MUX_MAX_FUNC_NAME);
  PKT_MUX_LOCK_GIVE();
  return INSIGHT_STATUS_SUCCESS;
}



