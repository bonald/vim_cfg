
/**
 @file sys_usw_dma_priv.h

 @date 2012-11-30

 @version v2.0

*/
#ifndef _SYS_USW_DMA_PRIV_H
#define _SYS_USW_DMA_PRIV_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_hash.h"
#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_vector.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
#define SYS_DMA_DUMP(FMT, ...)                  \
    do                                          \
    {                                           \
        CTC_DEBUG_OUT(dma, dma, DMA_SYS, CTC_DEBUG_LEVEL_DUMP, FMT, ## __VA_ARGS__); \
    } while (0)

#define SYS_DMA_CHAN_MAPPING(chan_id)  ((chan_id) < 4) ? 0: (chan_id)

#define SYS_DMA_DIR_TX  0
#define SYS_DMA_DIR_RX 1

/* just support quadmac stats and sgmac stats */
#define SYS_DMA_STATS_COUNT 2

/* dma learning cache for gen interrupt */
#define SYS_DMA_LEARNING_THRESHOLD  17
#define SYS_DMA_LEARNING_TIMER 1000

#define SYS_DMA_TX_COUNT            1000
#if(SDK_WORK_PLATFORM == 1)
    #define SYS_DMA_TBL_COUNT           40000*20
#else
    #define SYS_DMA_TBL_COUNT           40000
#endif
#define SYS_DMA_INIT_COUNT          4000

#define SYS_DMA_PKT_LEN_60          60
#define SYS_DMA_WORD_LEN            4

#define CRCPOLY_LE                  0xedb88320
#define CRCPOLY_BE                  0x04c11db7

#define SYS_DMA_MAX_STATS_CNT 5
#define SYS_DMA_QUADMAC_STATS_LEN (12 * 144 * (3 + 1)*4)
#define SYS_DMA_SGMAC_STATS_LEN (12 * 40 * 4*4)
#define SYS_DMA_INBONDFLOW_STATS_LEN  0
#define SYS_DMA_EEE_STATS_LEN  0
#define SYS_DMA_USER_STATS_LEN 0

#define SYS_DMA_MAX_LEARNING_BLOCK 4
#define DMA_STATS_PRI_HIGH 1
#define DMA_TABLE_PRI_HIGH 0

#define SYS_DMA_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(dma, dma, DMA_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#ifndef PACKET_TX_USE_SPINLOCK
#define DMA_LOCK(mutex) \
    if (mutex) sal_mutex_lock(mutex)
#define DMA_UNLOCK(mutex) \
    if (mutex) sal_mutex_unlock(mutex)
#else
#define DMA_LOCK(mutex) \
    if (mutex) sal_spinlock_lock((sal_spinlock_t*)mutex)
#define DMA_UNLOCK(mutex) \
    if (mutex) sal_spinlock_unlock((sal_spinlock_t*)mutex)
#endif

#define SYS_DMA_PKT_LEN_CHECK(len) \
    do { \
        if ((len) < 12 || (len) > (9600)) \
        { \
            return CTC_E_INVALID_PARAM; \
        } \
    } while (0)

enum swap_direction
{
    HOST_TO_NETWORK,
    NETWORK_TO_HOST
};

struct sys_dma_stats_s
{
    union
    {
        uint64 total_pkt_cnt;    /*only D2 used*/
        uint64 good_pkt_cnt;
    }u1;
    union
    {
        uint64 total_byte_cnt;   /*only D2 used*/
        uint64 good_byte_cnt;
    }u2;
    union
    {
        uint64 drop_cnt;         /*only D2 used*/
        uint64 bad_pkt_cnt;
    }u3;
    union
    {
        uint64 error_cnt;        /*only D2 used*/
        uint64 bad_byte_cnt;
    }u4;
};
typedef struct sys_dma_stats_s sys_dma_stats_t;

struct sys_dma_desc_used_info_s
{
    void* p_mem;
};
typedef struct sys_dma_desc_used_info_s sys_dma_desc_used_info_t;

struct sys_dma_desc_used_s
{
    uint32  count;
    uint32  entry_num;
    sys_dma_desc_used_info_t*   p_array;
};
typedef struct sys_dma_desc_used_s sys_dma_desc_used_t;

struct sys_dma_chan_s
{
    uint8 chan_en;
    uint8 sync_chan;
    uint8 sync_en;
    uint8 pkt_knet_en;

    uint8 channel_id;
    uint8 func_type;
    uint8 weight;
    uint8 dmasel;                                              /**< 0:DmaCtl0, 1:DmaCtl1 */

    uint32 current_index;
    uint32 desc_num;                                       /**< init desc number for channel */
    uint32 desc_depth;
    uintptr mem_base;

    uint16 data_size;                                        /**< data size per desc, used for alloc memory */
    uint16 cfg_size;                                          /**< desc cfg size used for config desc, for packet/reg uint is Byte , for InfoDma uint is Entry */

    sys_dma_desc_t* p_desc;                               /**< descriptors used by packet(TX) DMA */
    uint8* p_desc_used;                                     /**< indicate desc is used ot not , used for packet tx>*/

    sal_mutex_t* p_mutex;    /**< channel mutex  */
    sys_dma_desc_info_t* p_desc_info;
};
typedef struct sys_dma_chan_s sys_dma_chan_t;

struct sys_dma_tx_mem_s
{
    ctc_slistnode_t node_head;
    uintptr addr;
    uint8 mem_type;         /*0:from tx_mem_pool, 1:new alloc*/
    uint8 internal_alloc;
    uint32 opf_idx;
    uint32 pkt_len;
    ctc_pkt_info_t  tx_info;
    ctc_pkt_tx_t* tx_pkt;
    ctc_pkt_callback callback;  /**< [GB.GG] Packet Tx async callback function */
    void*           user_data;  /**< [GB.GG] User data for callback */
};
typedef struct sys_dma_tx_mem_s sys_dma_tx_mem_t;

struct sys_dma_timer_session_s
{
    uint8   state;        /*0: diable tx, 1: enable tx */
    uint8   rsv;
    uint16  desc_idx;     /*session used desc index*/
    uintptr phy_addr;     /*session data address */
};
typedef struct sys_dma_timer_session_s sys_dma_timer_session_t;

struct sys_dma_master_s
{
    sys_dma_chan_t dma_chan_info[SYS_DMA_MAX_CHAN_NUM];
    sys_dma_stats_t dma_stats[8];
    uint16 dma_thread_pri[SYS_DMA_MAX_CHAN_NUM];
    uint16 packet_rx_chan_num;
    ctc_vector_t* p_thread_vector; /* store sync thread info */

    DMA_CB_FUN_P dma_cb[SYS_DMA_CB_MAX_TYPE];
    DMA_DUMP_FUN_P dma_dump_cb;

    uint16      pkt_rx_size_per_desc;   /**< size of received pkt for every descriptor */
    uint32      dma_en_flag;                /**< flag indate which dma channel is enable, ont bit present one channel */
    uint32 dma_high_addr;

    uint8  hw_learning_sync;
    uint8  polling_mode;  /*1:donot using interrupt, using thread to monitor dma done*/

    ctc_slist_t* tx_pending_list_H; /*high priority*/
    ctc_slist_t* tx_pending_list_L; /*low priority*/
    uintptr tx_mem_pool[1024];
    ctc_hash_t* dma_tx_mem_entry;
    uint8 opf_type_dma_tx_mem;
    sal_task_t* p_async_tx_task;
    sal_sem_t* tx_sem;
    sal_spinlock_t* p_tx_mutex;
    sal_spinlock_t* p_list_lock;

    uint8  pkt_tx_timer_en;
    uint16  tx_timer;
    sys_dma_timer_session_t  tx_session[SYS_PKT_MAX_TX_SESSION];
    uint8 wb_reloading;
};
typedef struct sys_dma_master_s sys_dma_master_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#ifdef __cplusplus
}
#endif

#endif

