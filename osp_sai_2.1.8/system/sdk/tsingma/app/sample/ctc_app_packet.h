/**
 @file ctc_app_packet.h

 @date 2012-11-30

 @version v2.0

 This file define the cpu APIs
*/

#ifndef _CTC_PACKET_SAMPLE_H
#define _CTC_PACKET_SAMPLE_H
#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 *
* Function
*
*****************************************************************************/
#define MAX_APP_NETIF_NUM   64

struct app_pkt_stats_s
{
    uint32 tx;
    uint32 rx;
};
typedef struct app_pkt_stats_s app_pkt_stats_t;

struct app_pkt_eth_s
{
    int32 sock;
    char  ifname[10];
    sal_task_t* t_rx;
    app_pkt_stats_t stats;
};
typedef struct app_pkt_eth_s app_pkt_eth_t;

#ifdef _SAL_LINUX_UM
struct app_pkt_netif_info_s
{
    struct sockaddr_ll sl;
    struct ifreq ifr;
    int32 sock_fd;
};
typedef struct app_pkt_netif_info_s app_pkt_netif_info_t;
#endif

struct app_pkt_sample_master_s
{
    uint32 init_done;
    uint32 rx_print_en;
    CTC_PKT_RX_CALLBACK internal_rx_cb;  /* for SDK's internal process */
    CTC_PKT_RX_CALLBACK rx_cb;
    app_pkt_eth_t eth;
#ifdef _SAL_LINUX_UM
    uint16 netif_count;
    sal_task_t* netif_rx_task;
    app_pkt_netif_info_t  *pkt_netif[MAX_APP_NETIF_NUM];
#endif
};
typedef struct app_pkt_sample_master_s app_pkt_sample_master_t;

extern int32
ctc_app_packet_sample_show();

extern int32
ctc_app_packet_sample_set_rx_print_en(uint32 enable);

extern int32
ctc_app_packet_sample_tx(ctc_pkt_tx_t* p_pkt_tx);

extern int32
ctc_app_packet_sample_rx(ctc_pkt_rx_t* p_pkt_rx);

extern int32
ctc_app_packet_eth_init(void);

extern int32
ctc_packet_sample_register_internal_rx_cb(CTC_PKT_RX_CALLBACK internal_rx_cb);

extern int32
ctc_app_packet_sample_eth_open_raw_socket(uint16 cpu_ifid);

extern int32
ctc_app_packet_sample_eth_close_raw_socket(void);
#ifdef __cplusplus
}
#endif

#endif /* !_CTC_PACKET_SAMPLE_H */

