
#ifndef INCLUDE_PACKET_MULTIPLEXER_H
#define INCLUDE_PACKET_MULTIPLEXER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "insight.h"


#define     INSIGHT_MAX_PACKET_SIZE                1558 
#define     INSIGHT_PKT_MUX_MAX_FUNC_NAME          256
#define     INSIGHT_PT_MAX_PROFILE_REQUESTS          5

typedef struct _bview_packet_
{
  unsigned char     data[INSIGHT_MAX_PACKET_SIZE];
  unsigned int      asic;
  unsigned int      pkt_len;
  unsigned int      source_port;
//  INSIGHT_PORT_MASK_t dst_port_mask;
} INSIGHT_PACKET_t;


typedef struct _bview_packet_msg_
{
  unsigned int      appMask;
  INSIGHT_PACKET_t    packet;
  int               ltcRequestId;
} INSIGHT_PACKET_MSG_t;


/* The callback for invoking when a configured trigger goes off */
typedef INSIGHT_STATUS(*INSIGHT_PKT_MUX_CALLBACK_t) ( INSIGHT_PACKET_MSG_t *packet);

typedef struct pktMuxRegister_s
{
  unsigned int appID;                             /* application ID*/
  char         appFuncName[INSIGHT_PKT_MUX_MAX_FUNC_NAME]; /* application function name */  
  INSIGHT_PKT_MUX_CALLBACK_t callbackFunc;           /* Function to be called at specified hook */
} pktMuxRegister_t;



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
INSIGHT_STATUS pkt_mux_init();

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
INSIGHT_STATUS pkt_mux_register(pktMuxRegister_t *pkt_mux_hook);

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
INSIGHT_STATUS pkt_mux_deregister (pktMuxRegister_t *pkt_mux_hook);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_PACKET_MULTIPLEXER_H */

