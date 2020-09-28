
/**
 @file ctc_usw_packet.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0
 \p
 \p The packet module is used for communication protocol packets between CPU and The Module.
 \p There are two transport mode for packet: DMA or Ethernet.
 \p The Module will encapsulate CPUMAC (20B) + Packet (40B) for Ethernet mode.
 \p The Module will encapsulate Packet (40B) for DMA mode.

 \S ctc_packet.h:ctc_pkt_mode_t
 \S ctc_packet.h:ctc_pkt_skb_t
 \S ctc_packet.h:ctc_pkt_info_t
 \S ctc_packet.h:ctc_pkt_oam_info_t
 \S ctc_packet.h:ctc_pkt_ptp_info_t
 \S ctc_packet.h:ctc_pkt_cpu_reason_t
 \S ctc_packet.h:ctc_pkt_tx_t
 \S ctc_packet.h:ctc_pkt_global_cfg_t

*/

#ifndef _CTC_USW_PACKET_H
#define _CTC_USW_PACKET_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Header Files
*
***************************************************************/
#include "ctc_packet.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @addtogroup packet PACKET
 @{
*/

/****************************************************************************
*
* Function
*
*****************************************************************************/

/**
 @brief alloc DMA mem for tx pkt.

 @param[in] lchip    local chip id

 @param[in] size    mem size

 @param[in|out] p_addr     pointer to packet TX DMA mem.

 @remark[D2.TM]  This API is used for zero-copy mode.

 @return memary address or NULL
*/
extern int32
ctc_usw_packet_tx_alloc(uint8 lchip, uint32 size, void** p_addr);


/**
 @brief free DMA mem for tx pkt.

 @param[in] lchip    local chip id

 @param[in] addr    DMA mem address will be free

 @remark[D2.TM]  This API is used for zero-copy mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_tx_free(uint8 lchip, void* addr);
/**
 @brief Send packet.

 @param[in] lchip    local chip id

 @param[in|out] p_pkt_tx    pointer to packet TX

 @remark[D2.TM]  SDK will automatically encapsulate packet header if SDK is in DMA mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_tx(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);
/**
 @brief Send packet .

 @param[in] lchip  local chip id

 @param[in|out] p_pkt_array    pointer to TX packet array

 @param[in] count packet number

 @param[in] all_done_cb    callback function pointer ,will be call when all the packet finish tx.

 @param[in] cookie    parameter for callback function.

 @remark[D2.TM] Not support async mode, ctc_pkt_tx_t->callback must be NULL;

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_tx_array(uint8 lchip, ctc_pkt_tx_t** p_pkt_array, uint32 count, ctc_pkt_callback all_done_cb, void* cookie);

/**
 @brief Encapsulate packet header for packet from CPU to Chip

 @param[in] lchip    local chip id

 @param[in|out] p_pkt_tx    pointer to packet TX

 @remark[D2.TM]  User need to call this API to encapsulate packet header if SDK is in ETH mode.
          SDK will automatically encapsulate packet header if SDK is in DMA mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_encap(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);

/**
 @brief Decapsulate packet header for packet from Chip to CPU

 @param[in] lchip    local chip id

 @param[in|out] p_pkt_rx    pointer to packet RX

 @remark[D2.TM]  User need to call this API to decapsulate packet header if SDK is in ETH mode.
          SDK will automatically decapsulate packet header if SDK is in DMA mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_decap(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx);

/**
 @brief Initialize packet module

 @param[in] lchip    local chip id

 @param[in] p_global_cfg    pointer to packet initialize parameter, refer to ctc_pkt_global_cfg_t

 @remark[D2.TM]  User need to decide SDK's packet transport mode

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize packet module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the packet configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_deinit(uint8 lchip);

/**
 @brief Configure packet tx timer

 @param[in] lchip_id    local chip id

 @param[in] p_pkt_timer    packet tx timer configuration

 @remark[D2.TM]  Configure packet tx timer

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_set_tx_timer(uint8 lchip_id, ctc_pkt_tx_timer_t* p_pkt_timer);

/**
 @brief Create network interface

 @param[in] lchip_id    local chip id

 @param[in] p_netif    network interface configuration

 @remark[D2.TM]  Create network interface

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_create_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif);

/**
 @brief Destory network interface

 @param[in] lchip_id    local chip id

 @param[in] p_netif    network interface configuration

 @remark[D2.TM]  Destory network interface

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_destory_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif);

/**
 @brief Get network interface

 @param[in] lchip_id    local chip id

 @param[in] p_netif    network interface configuration

 @remark[D2.TM]  Get network interface

 @return CTC_E_XXX
*/
extern int32
ctc_usw_packet_get_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif);

/**@} end of @addtogroup packet PACKET */

#ifdef __cplusplus
}
#endif

#endif

