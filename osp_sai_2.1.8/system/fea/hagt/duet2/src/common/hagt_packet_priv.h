#ifndef __HAGT_PACKET_PRIV_H__
#define __HAGT_PACKET_PRIV_H__
/****************************************************************************
* hagt_packet_priv.h       :  packet process for DMA
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2013-02-18
* Reason        : First Create.
****************************************************************************/
#include "ctc_packet.h"
#include <asm/types.h>
#include <linux/netlink.h>
#include "sal.h"

#if 0
/*sys_greatbelt_dma.h*/
#define MIN_RX_CHANNEL 3
#define RX_CHANNEL_NUM 5 /*BFD channel also may use packet rx, use channel 7*/
#define MAX_RX_CHANNEL (MIN_RX_CHANNEL+RX_CHANNEL_NUM-1) /*BFD channel also may use packet rx, for bug 24580*/
#define TX_CHANNEL_NUM 1
#define MIN_TX_CHANNEL 3 

struct hagt_pkt_nlsk_s
{
    int32 fd;
    struct sockaddr_nl local;
    uint32 seq;
    uint32 type; /*0 sync, 1 data*/
    uint8* buf; /*size 0x2580(9600)*/
};
typedef struct hagt_pkt_nlsk_s hagt_pkt_nlsk_t;

struct hagt_packet_master_s
{
    ctc_pkt_skb_t* p_rx_skb[RX_CHANNEL_NUM];
    ctc_pkt_tx_t* p_tx_skb[TX_CHANNEL_NUM];
#ifndef _GLB_DISTRIBUTE_SYSTEM_    
    sal_task_t*    p_tx_task; /*Independent task used to receive packet from 
                                kernel peth; then send packet to cpu port*/
    hagt_pkt_nlsk_t  umpeth_nlsk; /*Netlinke socket used to receive/transmit from/to kernel peth*/
#endif /*_GLB_DISTRIBUTE_SYSTEM_*/
};
typedef struct hagt_packet_master_s hagt_packet_master_t;
#endif

#endif /*__HAGT_PACKET_PRIV_H__*/

