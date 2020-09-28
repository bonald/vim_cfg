/**
 @file sys_usw_stats.h

 @date 2009-12-15

 @version v2.0

*/
#ifndef _SYS_USW_STATS_H
#define _SYS_USW_STATS_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_stats.h"
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "sys_usw_chip.h"
#include "sys_usw_stats_api.h"

/***************************************************************
 *
 *  common
 *
 ***************************************************************/

#define SYS_STATS_DBG_OUT(level, FMT, ...)                              \
    do {                                                                       \
        CTC_DEBUG_OUT(stats, stats, STATS_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

#define SYS_STATS_DBG_DUMP(FMT, ...)                              \
    do {                                                                       \
        SYS_STATS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_STATS_DBG_INFO(FMT, ...)                              \
    do {                                                                       \
        SYS_STATS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_STATS_DBG_ERROR(FMT, ...)                              \
    do {                                                                       \
        SYS_STATS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_STATS_DBG_FUNC() \
    do {                                                                       \
        SYS_STATS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);  \
    } while (0)

/***************************************************************
 *
 *  flow stats
 *
 ***************************************************************/

#define SYS_STATS_CHACHE_UPDATE_BYTE_THRESHOLD      32768
#define SYS_STATS_CHACHE_UPDATE_PKT_THRESHOLD       10
#define SYS_STATS_DEFAULT_FIFO_DEPTH_THRESHOLD      15
#define SYS_STATS_DEFAULT_BYTE_THRESHOLD_LO         0x1ff
#define SYS_STATS_DEFAULT_PACKET_THRESHOLD_LO       0x1ff
#define SYS_STATS_DEFAULT_BYTE_THRESHOLD_HI         0x2ff
#define SYS_STATS_DEFAULT_PACKET_THRESHOLD_HI       0x2ff
#define SYS_STATS_DEFAULT_FIFO_DEPTH                0xF
#define SYS_STATS_FLOW_ENTRY_HASH_BLOCK_SIZE        16384


#define SYS_STATS_MAX_MEM_BLOCK                     8

#define SYS_STATS_PTR_FLAG_HAVE_STATSID             0x00000001
#define SYS_STATS_PTR_FLAG_CREATE_ON_INIT           0x00000002

/* Define opf index type */
enum sys_stats_ram_type_e
{
    SYS_STATS_RAM_EPE_GLOBAL0,
    SYS_STATS_RAM_EPE_GLOBAL1,
    SYS_STATS_RAM_EPE_GLOBAL2,
    SYS_STATS_RAM_EPE_GLOBAL3,
    SYS_STATS_RAM_EPE_PRIVATE0,

    SYS_STATS_RAM_IPE_GLOBAL0,
    SYS_STATS_RAM_IPE_GLOBAL1,
    SYS_STATS_RAM_IPE_GLOBAL2,
    SYS_STATS_RAM_IPE_GLOBAL3,
    SYS_STATS_RAM_IPE_PRIVATE0,
    SYS_STATS_RAM_IPE_PRIVATE1,
    SYS_STATS_RAM_IPE_PRIVATE2,
    SYS_STATS_RAM_IPE_PRIVATE3,

    SYS_STATS_RAM_QUEUE,

    SYS_STATS_RAM_SDC,
    SYS_STATS_RAM_MAX
};
typedef enum sys_stats_ram_type_e sys_stats_ram_type_t;

struct sys_stats_flow_stats_s
{
    ctc_list_pointer_node_t list_node;
    uint32 stats_ptr:30;
    uint32 flag:2;
    uint64 packet_count;
    uint64 byte_count;
};
typedef struct sys_stats_flow_stats_s sys_stats_flow_stats_t;

struct sys_stats_prop_s
{
    uint16 ram_bmp;
    uint16 used_cnt_ram[SYS_STATS_RAM_MAX];
    uint16 used_cnt;
};
typedef struct sys_stats_prop_s sys_stats_prop_t;

struct sys_stats_ram_s
{
    uint32 stats_bmp;
    uint32 cache_id;
    uint32 base_field_id;
    uint16 total_cnt;
    uint16 used_cnt;
    uint32 base_ptr;
    uint32 base_idx;
    uint8 acl_priority;
};
typedef struct sys_stats_ram_s sys_stats_ram_t;

struct sys_stats_master_s
{
    sal_mutex_t*        p_stats_mutex;

    uint32              stats_bitmap;
    uint8               stats_mode;
    uint8               clear_read_en;

    uint8               opf_type_flow_stats;
    uint8               opf_type_stats_id;

    ctc_list_pointer_t  flow_stats_list[SYS_STATS_MAX_MEM_BLOCK];
    ctc_hash_t*         flow_stats_hash;
    ctc_hash_t*         flow_stats_id_hash;

    uint16              alloc_count;
    sys_stats_prop_t    stats_type[CTC_BOTH_DIRECTION][SYS_STATS_TYPE_MAX];
    sys_stats_ram_t     stats_ram[SYS_STATS_RAM_MAX];

    uint32              ecmp_base_ptr;
    uint32              port_base_ptr[CTC_BOTH_DIRECTION];
};
typedef struct sys_stats_master_s sys_stats_master_t;


/***************************************************************
 *
 *  mac stats
 *
 ***************************************************************/

#define SYS_STATS_MTU_PKT_MIN_LENGTH 1024
#define SYS_STATS_MTU_PKT_MAX_LENGTH 16383
#define SYS_STATS_MAC_BASED_STATS_XQMAC_RAM_DEPTH     160u
#define SYS_STATS_XQMAC_PORT_MAX               4u
#define SYS_STATS_XQMAC_RAM_MAX               18u
#define SYS_STATS_MTU1_PKT_DFT_LENGTH       1518u
#define SYS_STATS_MTU2_PKT_DFT_LENGTH       1536u
#define SYS_STATS_DMA_IO_TIMER              60 /*unit in seconds*/

enum sys_mac_stats_property_e
{
    SYS_MAC_STATS_PROPERTY_CLEAR,
    SYS_MAC_STATS_PROPERTY_HOLD,
    SYS_MAC_STATS_PROPERTY_SATURATE,
    SYS_MAC_STATS_PROPERTY_MTU1,
    SYS_MAC_STATS_PROPERTY_MTU2,
    SYS_MAC_STATS_PROPERTY_PFC,
    SYS_MAC_STATS_PROPERTY_NUM
};
typedef enum sys_mac_stats_property_e sys_mac_stats_property_t;

/**
 @brief  Define mac receive statistics storage structor
*/
struct sys_stats_mac_rec_s
{
    uint64 good_ucast_pkts;
    uint64 good_ucast_bytes;
    uint64 good_mcast_pkts;
    uint64 good_mcast_bytes;
    uint64 good_bcast_pkts;
    uint64 good_bcast_bytes;
    uint64 good_normal_pause_pkts;
    uint64 good_normal_pause_bytes;
    uint64 good_pfc_pause_pkts;
    uint64 good_pfc_pause_bytes;
    uint64 good_control_pkts;
    uint64 good_control_bytes;
    uint64 fcs_error_pkts;
    uint64 fcs_error_bytes;
    uint64 mac_overrun_pkts;
    uint64 mac_overrun_bytes;
    uint64 good_63_pkts;
    uint64 good_63_bytes;
    uint64 bad_63_pkts;
    uint64 bad_63_bytes;
    uint64 good_mtu1_to_mtu2_pkts;
    uint64 good_mtu1_to_mtu2_bytes;
    uint64 bad_mtu1_to_mtu2_pkts;
    uint64 bad_mtu1_to_mtu2_bytes;
    uint64 good_jumbo_pkts;
    uint64 good_jumbo_bytes;
    uint64 bad_jumbo_pkts;
    uint64 bad_jumbo_bytes;
    uint64 pkts_64;
    uint64 bytes_64;
    uint64 pkts_65_to_127;
    uint64 bytes_65_to_127;
    uint64 pkts_128_to_255;
    uint64 bytes_128_to_255;
    uint64 pkts_256_to_511;
    uint64 bytes_256_to_511;
    uint64 pkts_512_to_1023;
    uint64 bytes_512_to_1023;
    uint64 pkts_1024_to_1518;
    uint64 bytes_1024_to_1518;
    uint64 pkts_1518_to_2047;
    uint64 bytes_1518_to_2047;
    uint64 pkts_2047_to_mtu1;
    uint64 bytes_2047_to_mtu1;

};
typedef struct sys_stats_mac_rec_s sys_stats_mac_rec_t;

/**
 @brief  Define mac transmit statistics storage structor
*/
struct sys_stats_mac_snd_s
{
    uint64 good_ucast_pkts;
    uint64 good_ucast_bytes;
    uint64 good_mcast_pkts;
    uint64 good_mcast_bytes;
    uint64 good_bcast_pkts;
    uint64 good_bcast_bytes;
    uint64 good_pause_pkts;
    uint64 good_pause_bytes;
    uint64 good_control_pkts;
    uint64 good_control_bytes;
    uint64 fcs_error_pkts;
    uint64 fcs_error_bytes;
    uint64 mac_underrun_pkts;
    uint64 mac_underrun_bytes;
    uint64 pkts_63;
    uint64 bytes_63;
    uint64 pkts_64;
    uint64 bytes_64;
    uint64 pkts_65_to_127;
    uint64 bytes_65_to_127;
    uint64 pkts_128_to_255;
    uint64 bytes_128_to_255;
    uint64 pkts_256_to_511;
    uint64 bytes_256_to_511;
    uint64 pkts_512_to_1023;
    uint64 bytes_512_to_1023;
    uint64 pkts_1024_to_1518;
    uint64 bytes_1024_to_1518;
    uint64 pkts_1519_to_2047;
    uint64 bytes_1519_to_2047;
    uint64 pkts_2047_to_mtu1;
    uint64 bytes_2047_to_mtu1;
    uint64 pkts_mtu1_to_mtu2;
    uint64 bytes_mtu1_to_mtu2;
    uint64 jumbo_pkts;
    uint64 jumbo_bytes;

};
typedef struct sys_stats_mac_snd_s sys_stats_mac_snd_t;

/*mac statistics type*/
enum sys_stats_mac_rec_stats_type_e
{
    SYS_STATS_MAC_RCV_GOOD_UCAST = 0,
    SYS_STATS_MAC_RCV_GOOD_MCAST = 1,
    SYS_STATS_MAC_RCV_GOOD_BCAST = 2,
    SYS_STATS_MAC_RCV_GOOD_NORMAL_PAUSE = 3,
    SYS_STATS_MAC_RCV_GOOD_PFC_PAUSE = 4,
    SYS_STATS_MAC_RCV_GOOD_CONTROL = 5,
    SYS_STATS_MAC_RCV_FCS_ERROR = 6,
    SYS_STATS_MAC_RCV_MAC_OVERRUN = 7,
    SYS_STATS_MAC_RCV_GOOD_63B = 8,
    SYS_STATS_MAC_RCV_BAD_63B = 9,
    SYS_STATS_MAC_RCV_GOOD_MTU1_TO_MTU2 = 10,
    SYS_STATS_MAC_RCV_BAD_MTU1_TO_MTU2 = 11,
    SYS_STATS_MAC_RCV_GOOD_JUMBO = 12,
    SYS_STATS_MAC_RCV_BAD_JUMBO =13,
    SYS_STATS_MAC_RCV_64B = 14,
    SYS_STATS_MAC_RCV_127B = 15,
    SYS_STATS_MAC_RCV_255B = 16,
    SYS_STATS_MAC_RCV_511B = 17,
    SYS_STATS_MAC_RCV_1023B = 18,
    SYS_STATS_MAC_RCV_1518B = 19,
    SYS_STATS_MAC_RCV_2047B = 20,
    SYS_STATS_MAC_RCV_MTU1 = 21,

    SYS_STATS_MAC_RCV_NUM
};
typedef enum sys_stats_mac_rec_stats_type_e sys_stats_mac_rec_stats_type_t;

enum sys_stats_mac_snd_stats_type_e
{
    SYS_STATS_MAC_SEND_UCAST = 22,
    SYS_STATS_MAC_SEND_MCAST = 23,
    SYS_STATS_MAC_SEND_BCAST = 24,
    SYS_STATS_MAC_SEND_PAUSE = 25,
    SYS_STATS_MAC_SEND_CONTROL = 26,
    SYS_STATS_MAC_SEND_FCS_ERROR = 27,
    SYS_STATS_MAC_SEND_MAC_UNDERRUN = 28,
    SYS_STATS_MAC_SEND_63B = 29,
    SYS_STATS_MAC_SEND_64B = 30,
    SYS_STATS_MAC_SEND_127B = 31,
    SYS_STATS_MAC_SEND_255B =32,
    SYS_STATS_MAC_SEND_511B = 33,
    SYS_STATS_MAC_SEND_1023B = 34,
    SYS_STATS_MAC_SEND_1518B =35,
    SYS_STATS_MAC_SEND_2047B = 36,
    SYS_STATS_MAC_SEND_MTU1 = 37,
    SYS_STATS_MAC_SEND_MTU2 = 38,
    SYS_STATS_MAC_SEND_JUMBO = 39,

    SYS_STATS_MAC_STATS_TYPE_NUM = 40 ,
    SYS_STATS_MAC_SEND_NUM = SYS_STATS_MAC_STATS_TYPE_NUM-SYS_STATS_MAC_SEND_UCAST
};
typedef enum sys_stats_mac_snd_stats_type_e sys_stats_mac_snd_stats_type_t;

/* the value defined should be consistent with ctc_stats.h */
enum sys_stats_map_mac_rx_type_e
{
    SYS_STATS_MAP_MAC_RX_GOOD_UC_PKT = 0,
    SYS_STATS_MAP_MAC_RX_GOOD_UC_BYTE = 1,
    SYS_STATS_MAP_MAC_RX_GOOD_MC_PKT = 2,
    SYS_STATS_MAP_MAC_RX_GOOD_MC_BYTE = 3,
    SYS_STATS_MAP_MAC_RX_GOOD_BC_PKT = 4,
    SYS_STATS_MAP_MAC_RX_GOOD_BC_BYTE = 5,
    SYS_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_PKT = 8,
    SYS_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_BYTE = 9,
    SYS_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_PKT = 10,
    SYS_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_BYTE = 11,
    SYS_STATS_MAP_MAC_RX_GOOD_CONTROL_PKT = 12,
    SYS_STATS_MAP_MAC_RX_GOOD_CONTROL_BYTE = 13,
    SYS_STATS_MAP_MAC_RX_FCS_ERROR_PKT = 18,
    SYS_STATS_MAP_MAC_RX_FCS_ERROR_BYTE = 19,
    SYS_STATS_MAP_MAC_RX_MAC_OVERRUN_PKT = 22,
    SYS_STATS_MAP_MAC_RX_MAC_OVERRUN_BYTE = 23,
    SYS_STATS_MAP_MAC_RX_GOOD_63B_PKT = 30,
    SYS_STATS_MAP_MAC_RX_GOOD_63B_BYTE = 31,
    SYS_STATS_MAP_MAC_RX_BAD_63B_PKT = 32,
    SYS_STATS_MAP_MAC_RX_BAD_63B_BYTE = 33,
    SYS_STATS_MAP_MAC_RX_GOOD_1519B_PKT = 34,
    SYS_STATS_MAP_MAC_RX_GOOD_1519B_BYTE = 35,
    SYS_STATS_MAP_MAC_RX_BAD_1519B_PKT = 36,
    SYS_STATS_MAP_MAC_RX_BAD_1519B_BYTE = 37,
    SYS_STATS_MAP_MAC_RX_GOOD_JUMBO_PKT = 38,
    SYS_STATS_MAP_MAC_RX_GOOD_JUMBO_BYTE = 39,
    SYS_STATS_MAP_MAC_RX_BAD_JUMBO_PKT = 40,
    SYS_STATS_MAP_MAC_RX_BAD_JUMBO_BYTE = 41,
    SYS_STATS_MAP_MAC_RX_64B_PKT = 42,
    SYS_STATS_MAP_MAC_RX_64B_BYTE = 43,
    SYS_STATS_MAP_MAC_RX_127B_PKT = 44,
    SYS_STATS_MAP_MAC_RX_127B_BYTE = 45,
    SYS_STATS_MAP_MAC_RX_255B_PKT = 46,
    SYS_STATS_MAP_MAC_RX_255B_BYTE = 47,
    SYS_STATS_MAP_MAC_RX_511B_PKT = 48,
    SYS_STATS_MAP_MAC_RX_511B_BYTE = 49,
    SYS_STATS_MAP_MAC_RX_1023B_PKT = 50,
    SYS_STATS_MAP_MAC_RX_1023B_BYTE = 51,
    SYS_STATS_MAP_MAC_RX_1518B_PKT = 52,
    SYS_STATS_MAP_MAC_RX_1518B_BYTE = 53,
    SYS_STATS_MAP_MAC_RX_2047B_PKT = 54,
    SYS_STATS_MAP_MAC_RX_2047B_BYTE = 55,
    SYS_STATS_MAP_MAC_RX_MTU1_PKT = 56,
    SYS_STATS_MAP_MAC_RX_MTU1_BYTE = 57,

    SYS_STATS_MAP_MAC_RX_TYPE_NUM
};
typedef enum sys_stats_map_mac_rx_type_e sys_stats_map_mac_rx_type_t;

/* the value defined should be consistent with ctc_stats.h */
enum sys_stats_map_mac_tx_type_e
{
    SYS_STATS_MAP_MAC_TX_GOOD_UC_PKT = 0,
    SYS_STATS_MAP_MAC_TX_GOOD_UC_BYTE = 1,
    SYS_STATS_MAP_MAC_TX_GOOD_MC_PKT = 2,
    SYS_STATS_MAP_MAC_TX_GOOD_MC_BYTE = 3,
    SYS_STATS_MAP_MAC_TX_GOOD_BC_PKT = 4,
    SYS_STATS_MAP_MAC_TX_GOOD_BC_BYTE = 5,
    SYS_STATS_MAP_MAC_TX_GOOD_PAUSE_PKT = 6,
    SYS_STATS_MAP_MAC_TX_GOOD_PAUSE_BYTE = 7,
    SYS_STATS_MAP_MAC_TX_GOOD_CONTROL_PKT = 8,
    SYS_STATS_MAP_MAC_TX_GOOD_CONTROL_BYTE = 9,
    SYS_STATS_MAP_MAC_TX_FCS_ERROR_PKT = 32,
    SYS_STATS_MAP_MAC_TX_FCS_ERROR_BYTE = 33,
    SYS_STATS_MAP_MAC_TX_MAC_UNDERRUN_PKT = 30,
    SYS_STATS_MAP_MAC_TX_MAC_UNDERRUN_BYTE = 31,
    SYS_STATS_MAP_MAC_TX_63B_PKT = 12,
    SYS_STATS_MAP_MAC_TX_63B_BYTE = 13,
    SYS_STATS_MAP_MAC_TX_64B_PKT = 14,
    SYS_STATS_MAP_MAC_TX_64B_BYTE = 15,
    SYS_STATS_MAP_MAC_TX_127B_PKT = 16,
    SYS_STATS_MAP_MAC_TX_127B_BYTE = 17,
    SYS_STATS_MAP_MAC_TX_225B_PKT = 18,
    SYS_STATS_MAP_MAC_TX_225B_BYTE = 19,
    SYS_STATS_MAP_MAC_TX_511B_PKT = 20,
    SYS_STATS_MAP_MAC_TX_511B_BYTE = 21,
    SYS_STATS_MAP_MAC_TX_1023B_PKT = 22,
    SYS_STATS_MAP_MAC_TX_1023B_BYTE = 23,
    SYS_STATS_MAP_MAC_TX_1518B_PKT = 24,
    SYS_STATS_MAP_MAC_TX_1518B_BYTE = 25,

    SYS_STATS_MAP_MAC_TX_2047B_PKT = 34 ,
    SYS_STATS_MAP_MAC_TX_2047B_BYTE = 35,

    SYS_STATS_MAP_MAC_TX_MTU1_PKT = 36,
    SYS_STATS_MAP_MAC_TX_MTU1_BYTE = 37,
    SYS_STATS_MAP_MAC_TX_MTU2_PKT = 38,
    SYS_STATS_MAP_MAC_TX_MTU2_BYTE = 39,

    SYS_STATS_MAP_MAC_TX_JUMBO_PKT = 28,
    SYS_STATS_MAP_MAC_TX_JUMBO_BYTE = 29,

    SYS_STATS_MAP_MAC_TX_TYPE_NUM
};
typedef enum sys_stats_map_mac_tx_type_e sys_stats_map_mac_tx_type_t;

enum sys_stats_mac_cnt_type_e
{
    SYS_STATS_MAC_CNT_TYPE_PKT,
    SYS_STATS_MAC_CNT_TYPE_BYTE,
    SYS_STATS_MAC_CNT_TYPE_NUM
};
typedef enum sys_stats_mac_cnt_type_e sys_stats_mac_cnt_type_t;

enum sys_mac_stats_store_mode_e
{
     SYS_MAC_STATS_STORE_HW,
     SYS_MAC_STATS_STORE_SW
};
typedef enum sys_mac_stats_store_mode_e sys_mac_stats_store_mode_t;

struct sys_mac_stats_rx_s
{
    uint64 mac_stats_rx_bytes[SYS_STATS_MAC_RCV_NUM];
    uint64 mac_stats_rx_pkts[SYS_STATS_MAC_RCV_NUM];
};
typedef  struct sys_mac_stats_rx_s sys_mac_stats_rx_t;

struct sys_mac_stats_tx_s
{
    uint64 mac_stats_tx_bytes[SYS_STATS_MAC_SEND_NUM];
    uint64 mac_stats_tx_pkts[SYS_STATS_MAC_SEND_NUM];
};
typedef struct sys_mac_stats_tx_s sys_mac_stats_tx_t;

struct sys_qmac_stats_s
{
    sys_mac_stats_rx_t mac_stats_rx[SYS_STATS_XQMAC_PORT_MAX];
    sys_mac_stats_tx_t mac_stats_tx[SYS_STATS_XQMAC_PORT_MAX];
};
typedef struct sys_qmac_stats_s sys_qmac_stats_t;

struct sys_stats_mac_throughput_s
{
    uint64 bytes[SYS_USW_MAX_PORT_NUM_PER_CHIP][CTC_STATS_MAC_STATS_MAX];
    sal_systime_t timestamp[SYS_USW_MAX_PORT_NUM_PER_CHIP];
};
typedef struct sys_stats_mac_throughput_s sys_stats_mac_throughput_t;

struct sys_mac_stats_master_s
{
    sal_mutex_t* p_stats_mutex;

    uint8    stats_mode;
    uint8    query_mode;
    uint8    store_mode;
    uint8    rsv;
    uint32   mac_timer;
    sys_qmac_stats_t    qmac_stats_table[SYS_STATS_XQMAC_RAM_MAX];
};
typedef struct sys_mac_stats_master_s sys_mac_stats_master_t;

#ifdef __cplusplus
}
#endif

#endif

