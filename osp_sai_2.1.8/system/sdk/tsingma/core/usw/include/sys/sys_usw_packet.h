/**
 @file sys_usw_packet.h

 @date 2012-10-23

 @version v2.0

*/
#ifndef _SYS_USW_PACKET_H
#define _SYS_USW_PACKET_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_packet.h"
#include "ctc_debug.h"
#include "ctc_const.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
typedef int32 (* SYS_PKT_RX_CALLBACK)(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx);
#define SYS_PKT_MAX_TX_SESSION 2048

enum sys_packet_tx_type_e
{
    SYS_PKT_TX_TYPE_TPID,
    SYS_PKT_TX_TYPE_C2C_SUB_QUEUE_ID,
    SYS_PKT_TX_TYPE_FWD_CPU_SUB_QUEUE_ID,
    SYS_PKT_TX_TYPE_PTP_ADJUST_OFFSET,
    SYS_PKT_TX_TYPE_VLAN_PTR,
    SYS_PKT_TX_TYPE_RSV_NH,
    SYS_PKT_TX_MAX_TYPE
};
typedef enum sys_packet_tx_type_e sys_packet_tx_type_t;

/**
 @brief packet RX in DMA mode, called by DMA RX function
*/
extern int32
sys_usw_packet_rx(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx);

/**
 @brief alloc mem for tx packet in DMA mode
*/
extern  void*
sys_usw_packet_tx_alloc(uint8 lchip, uint32 size);

/**
 @brief free mem for tx packet in DMA mode
*/
extern void
sys_usw_packet_tx_free(uint8 lchip, void* addr);

/**
 @brief packet TX in DMA mode
*/

extern int32
sys_usw_packet_tx_array(uint8 lchip, ctc_pkt_tx_t **p_pkt_array, uint32 count, ctc_pkt_callback all_done_cb, void *cookie);

/**
 @brief packet TX in DMA mode, call DMA TX function
*/
extern int32
sys_usw_packet_tx(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);

/**
 @brief register internal RX callback
*/
extern int32
sys_usw_packet_register_internal_rx_cb(uint8 lchip, SYS_PKT_RX_CALLBACK oam_rx_cb);

/**
 @brief Encapsulate packet header for packet from CPU to Chip
*/
extern int32
sys_usw_packet_encap(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);

/**
 @brief Decapsulate packet header for packet from Chip to CPU
*/
extern int32
sys_usw_packet_decap(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx);

/**
 @brief Initialize packet module
*/
extern int32
sys_usw_packet_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize packet module
*/
extern int32
sys_usw_packet_deinit(uint8 lchip);

extern int32
sys_usw_packet_set_tx_timer(uint8 lchip, ctc_pkt_tx_timer_t* p_pkt_timer);

int32
sys_usw_packet_create_netif(uint8 lchip, ctc_pkt_netif_t* p_netif);

int32
sys_usw_packet_destory_netif(uint8 lchip, ctc_pkt_netif_t* p_netif);

int32
sys_usw_packet_get_netif(uint8 lchip, ctc_pkt_netif_t* p_netif);

int32
sys_usw_packet_tx_set_property(uint8 lchip, uint16 type, uint32 value1, uint32 value2);
#ifdef __cplusplus
}
#endif

#endif

