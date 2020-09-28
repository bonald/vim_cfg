/**
 @file sys_usw_packet_priv.h

 @date 2012-10-31

 @version v2.0

*/
#ifndef _SYS_USW_PACKET_PRIV_H
#define _SYS_USW_PACKET_PRIV_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_packet.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define SYS_PKT_DUMP(FMT, ...)      \
    do                                  \
    {                                   \
        CTC_DEBUG_OUT(packet, packet, PACKET_SYS, CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_PKT_DBG_INFO(FMT, ...)  \
    do                                  \
    {                                   \
        CTC_DEBUG_OUT_INFO(packet, packet, PACKET_SYS, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_PKT_DBG_FUNC()          \
    do                                  \
    {                                   \
        CTC_DEBUG_OUT_FUNC(packet, packet, PACKET_SYS); \
    } while (0)

#define SYS_PACKET_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(packet, packet, PACKET_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_PKT_BUF_PKT_LEN 256 /*refer to DMA default store packet pkt*/
#define SYS_PKT_BUF_MAX 64
#define STS_PKT_REASON_BLOCK_NUM (CTC_PKT_CPU_REASON_MAX_COUNT+31)/32
#define SYS_PKT_MAX_TX_SESSION 2048
#define STS_PKT_SESSION_BLOCK_NUM (SYS_PKT_MAX_TX_SESSION+31)/32
#define SYS_PKT_MAX_NETIF_NUM   64
#define SYS_PKT_NETIF_HASH_SIZE 256

struct sys_pkt_stats_s
{
    uint64 encap;
    uint64 decap;
    uint64 uc_tx;
    uint64 mc_tx;
    uint64 rx[CTC_PKT_CPU_REASON_MAX_COUNT];
};
typedef struct sys_pkt_stats_s sys_pkt_stats_t;

struct sys_pkt_buf_s
{
   uint16 pkt_len;
   uint8  mode;
   uint8  rsv;
   uint8  pkt_data[SYS_PKT_BUF_PKT_LEN];
   sal_time_t tm;
   uint32 hash_seed;
   uint32  seconds;    /* for packet rx timestamp dump */
   uint32  nanoseconds;    /* for packet rx timestamp dump */
};
typedef struct sys_pkt_buf_s sys_pkt_buf_t;

struct sys_pkt_tx_buf_s
{
   uint16 pkt_len;
   uint8 mode;
   uint8  pkt_data[SYS_PKT_BUF_PKT_LEN];
   ctc_pkt_info_t  tx_info;
   sal_time_t tm;
   uint32 hash_seed;
};
typedef struct sys_pkt_tx_buf_s sys_pkt_tx_buf_t;

struct sys_pkt_tx_hdr_info_s
{
    uint16 tp_id[4];
    uint32 offset_ns;   /*ptp adjust offset ns*/
    uint32 offset_s;    /*ptp adjust offset s*/
    uint16 c2c_sub_queue_id;
    uint16 fwd_cpu_sub_queue_id;
    uint16 vlanptr[CTC_MAX_VLAN_ID];
    uint32 rsv_nh[4];
};
typedef struct sys_pkt_tx_hdr_info_s sys_pkt_tx_hdr_info_t;

struct sys_pkt_master_s
{
    ctc_pkt_global_cfg_t    cfg;
    sys_pkt_stats_t         stats;
    mac_addr_t              cpu_mac_sa;
    mac_addr_t              cpu_mac_da;
    sys_pkt_buf_t           rx_buf[SYS_PKT_BUF_MAX];
    sys_pkt_tx_buf_t        tx_buf[SYS_PKT_BUF_MAX];
    uint8                   cursor[CTC_BOTH_DIRECTION];
    uint8                   tx_dump_enable;
    uint8    rx_header_en[CTC_PKT_CPU_REASON_MAX_COUNT];      /*just using for debug packet*/
    uint32   rx_reason_bm[STS_PKT_REASON_BLOCK_NUM];
    uint8    is_skb_buf;
    uint8    pkt_buf_en[CTC_BOTH_DIRECTION];
    uint16   rx_buf_reason_id;         /*0-all packet; other -specical reason_id*/
    sal_mutex_t* mutex;
    sal_mutex_t* mutex_rx;
    uint32   session_bm[STS_PKT_SESSION_BLOCK_NUM];
    ctc_hash_t *netif_hash;

    SYS_PKT_RX_CALLBACK oam_rx_cb;
    sys_pkt_tx_hdr_info_t tx_hdr_info;

    CTC_PKT_RX_CALLBACK drop_rx_cb;
};
typedef struct sys_pkt_master_s sys_pkt_master_t;

struct sys_usw_cpumac_header_s
{
    mac_addr_t macda;       /* destination eth addr */
    mac_addr_t macsa;       /* source ether addr */
    uint16 vlan_tpid;
    uint16 vlan_vid;
    uint16 type;            /* packet type ID field */
};
typedef struct sys_usw_cpumac_header_s sys_usw_cpumac_header_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#ifdef __cplusplus
}
#endif

#endif

