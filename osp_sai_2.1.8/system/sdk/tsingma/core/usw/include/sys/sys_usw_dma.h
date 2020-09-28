/**
 @file sys_usw_dma.h

 @date 2012-3-10

 @version v2.0

*/

#ifndef _SYS_USW_DMA_H
#define _SYS_USW_DMA_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
/* commone */
#include "sal.h"
#include "ctc_const.h"
#include "ctc_linklist.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_learning_aging.h"
#include "ctc_dma.h"
#include "ctc_packet.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
/****************************************************************
*
* Functions
*
****************************************************************/
#define SYS_DMA_DESC_LEN 32

#define SYS_DMA_MTU  (CTC_PKT_MTU + SYS_USW_PKT_HEADER_LEN)

#define SYS_DMA_TX_PKT_MEM_NUM 512
#define SYS_DMA_TX_PKT_MEM_SIZE (1576)
#define SYS_DMA_TX_MEM_BLOCK_SIZE    16

#define SYS_DMA_HIGH_WEIRGH  8
#define SYS_DMA_MID_WEIGHT 6
#define SYS_DMA_LOW_WEIGHT 4

#define SYS_DMA_PACKET_RX0_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_RX0_CHAN_ID)
#define SYS_DMA_PACKET_RX1_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_RX1_CHAN_ID)
#define SYS_DMA_PACKET_RX2_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_RX2_CHAN_ID)
#define SYS_DMA_PACKET_RX3_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_RX3_CHAN_ID)
#define SYS_DMA_PACKET_TX0_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_TX0_CHAN_ID)
#define SYS_DMA_PACKET_TX1_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_TX1_CHAN_ID)
#define SYS_DMA_PACKET_TX2_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_TX2_CHAN_ID)
#define SYS_DMA_PACKET_TX3_CHAN_ID  DRV_ENUM(DRV_DMA_PACKET_TX3_CHAN_ID)
#define SYS_DMA_TBL_WR_CHAN_ID      DRV_ENUM(DRV_DMA_TBL_WR_CHAN_ID)
#define SYS_DMA_TBL_RD_CHAN_ID      DRV_ENUM(DRV_DMA_TBL_RD_CHAN_ID)
#define SYS_DMA_PORT_STATS_CHAN_ID  DRV_ENUM(DRV_DMA_PORT_STATS_CHAN_ID)
#define SYS_DMA_FLOW_STATS_CHAN_ID  DRV_ENUM(DRV_DMA_FLOW_STATS_CHAN_ID)
#define SYS_DMA_REG_MAX_CHAN_ID     DRV_ENUM(DRV_DMA_REG_MAX_CHAN_ID)
#define SYS_DMA_TBL_RD1_CHAN_ID     DRV_ENUM(DRV_DMA_TBL_RD1_CHAN_ID)
#define SYS_DMA_TBL_RD2_CHAN_ID     DRV_ENUM(DRV_DMA_TBL_RD2_CHAN_ID)
#define SYS_DMA_LEARNING_CHAN_ID    DRV_ENUM(DRV_DMA_LEARNING_CHAN_ID)
#define SYS_DMA_HASHKEY_CHAN_ID     DRV_ENUM(DRV_DMA_HASHKEY_CHAN_ID)
#define SYS_DMA_IPFIX_CHAN_ID       DRV_ENUM(DRV_DMA_IPFIX_CHAN_ID)
#define SYS_DMA_SDC_CHAN_ID         DRV_ENUM(DRV_DMA_SDC_CHAN_ID)
#define SYS_DMA_MONITOR_CHAN_ID     DRV_ENUM(DRV_DMA_MONITOR_CHAN_ID)

#define SYS_DMA_CTL_USED_NUM   1
#define SYS_DMA_MAX_INFO_NUM   5
#define SYS_DMA_RX_CHAN_NUM         4
#define SYS_DMA_MAX_CHAN_NUM      20
#define SYS_DMA_INFO_CHAN_NUM         4

#define SYS_DMA_STATS_INTERVAL   (5)     /* temp using 5min, need adjust according stats module */
#define SYS_SUP_CLOCK   500    /* 250M */

#define SYS_DMA_MAX_DESC_NUM  0xffff
#define SYS_DMA_MAX_PACKET_RX_DESC_NUM (5*1024)
#define SYS_DMA_MAX_PACKET_TX_DESC_NUM (1*1024)
#define SYS_DMA_MAX_LEARNING_DESC_NUM (1*1024)
#define SYS_DMA_MAX_IPFIX_DESC_NUM (1*256)
#define SYS_DMA_MAX_SDC_DESC_NUM (1*256)
#define SYS_DMA_MAX_MONITOR_DESC_NUM (1*256)
#define SYS_DMA_MAX_HASHDUMP_DESC_NUM (1*256)
#define SYS_DMA_MAX_STATS_DESC_NUM (1*256)
#define SYS_DMA_MAX_TAB_RD_DESC_NUM (1*256)
#define SYS_DMA_MAX_TAB_WR_DESC_NUM (1*256)

#define SYS_DMA_LEARN_AGING_SYNC_CNT 32
#define SYS_DMA_MONITOR_SYNC_CNT 64
#define SYS_DMA_IPFIX_SYNC_CNT 32
#define SYS_DMA_SDC_SYNC_CNT 32
#ifdef DUET2
#define SYS_DMA_PKT_TX_TIMER_CHAN_ID (SYS_DMA_PACKET_TX1_CHAN_ID)
#else
#define SYS_DMA_PKT_TX_TIMER_CHAN_ID (SYS_DMA_PACKET_TX3_CHAN_ID)
#endif

#define SYS_DMA_DMACTL_CHECK(id) \
    do { \
        if (id >= SYS_DMA_CTL_USED_NUM){ \
            return CTC_E_DMA; } \
    } while (0)

#define SYS_DMA_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip);\
        if (NULL == p_usw_dma_master[lchip]){ \
            return CTC_E_NOT_INIT; } \
    } while (0)

typedef int32 (* SYS_DMA_THREAD_FUNC)(void* p_data);

#define GET_HIGH_32BITS(addr, high) \
    { \
        (high) = ((addr) >> 32); \
    } \

#define COMBINE_64BITS_DATA(high, low, data) \
    {  \
        (data) = high; \
        (data) = (data <<32) | low; \
    }

/**
 @brief dma desc define, for 4-bytes align
*/
struct sys_dma_desc_s
{
    DsDesc_m desc_info;
    uint32   desc_padding0;  /* Notice: padding fields cannot used for any purpose*/
    uint32   desc_padding1;  /* Notice: padding fields cannot used for any purpose*/
};
typedef struct sys_dma_desc_s sys_dma_desc_t;
struct sys_dma_desc_info_s
{
    uint16   value0;  /* for stats dma means mac id ,
                         for info dma means current entry index in desc,
                         for tcam scan dma means mem_id and sub id,
                         other dma function have no meaning */
    uint16   value1;  /*used to record desc real size, for tcam scan means entry size*/
};
typedef struct sys_dma_desc_info_s sys_dma_desc_info_t;
struct sys_dma_desc_ts_s
{
    uint32 ts_ns[2];
};
typedef struct sys_dma_desc_ts_s sys_dma_desc_ts_t;

/**
 @brief dma learning info struct define, for 4-bytes align
*/
struct sys_dma_learning_info_s
{
    DmaFibLearnFifo_m learn_info;
    uint32 learn_pad[3];
};
typedef struct sys_dma_learning_info_s sys_dma_learning_info_t;

enum sys_dma_info_type_e
{
    SYS_DMA_INFO_TYPE_LEARN,
    SYS_DMA_INFO_TYPE_HASHDUMP,
    SYS_DMA_INFO_TYPE_IPFIX,
    SYS_DMA_INFO_TYPE_SDC,
    SYS_DMA_INFO_TYPE_MONITOR,

    SYS_DMA_INFO_TYPE_MAX
};
typedef  enum sys_dma_info_type_e sys_dma_info_type_t;

enum sys_dma_cb_type_e
{
    SYS_DMA_CB_TYPE_LERNING,
    SYS_DMA_CB_TYPE_IPFIX,
    SYS_DMA_CB_TYPE_MONITOR,
    SYS_DMA_CB_TYPE_PORT_STATS,
    SYS_DMA_CB_TYPE_FLOW_STATS,
    SYS_DMA_CB_TYPE_SDC_STATS,
    SYS_DMA_CB_TYPE_DOT1AE_STATS,
    SYS_DMA_CB_MAX_TYPE
};
typedef  enum sys_dma_cb_type_e sys_dma_cb_type_t;

struct sys_dma_info_s
{
    uint32 entry_num; /*uint32 for chip agent*/
    void* p_data;
};
typedef struct sys_dma_info_s sys_dma_info_t;

struct sys_dma_reg_s
{
    void* p_data;
    void* p_ext;  /*for port stats means mac_id,for flow stats means block id*/
};
typedef struct sys_dma_reg_s sys_dma_reg_t;

typedef int32 (* DMA_CB_FUN_P)  (uint8 lchip, void* p_data);
struct dma_dump_cb_parameter_s
{
    uint32 entry_count;
    uint16 threshold;
    uint8  fifo_full;
    uint8  rsv;
};
typedef struct dma_dump_cb_parameter_s dma_dump_cb_parameter_t;
typedef int32 (*DMA_DUMP_FUN_P)(uint8 lchip, dma_dump_cb_parameter_t* parameter, uint16* p_entry_num, void* p_data);

struct sys_dma_thread_s
{
    uint8 chan_num;
    uint8 chan_id[8];
    uint16 prio;
    uint8 lchip;
    uint8 rsv[3];
    char desc[64];
    sal_sem_t* p_sync_sem;
    sal_task_t* p_sync_task;
};
typedef struct sys_dma_thread_s sys_dma_thread_t;

struct sys_dma_tbl_rw_s
{
    uint32  tbl_addr;
    uint32* buffer;

    uint16  entry_num;
    uint16  entry_len;

    uint8   rflag;
    uint8   is_pause;
    uint8   user_dma_mode; /*use the user alloced dma memory*/
    uint8   rsv;
};
typedef struct sys_dma_tbl_rw_s sys_dma_tbl_rw_t;

#define GET_CHAN_TYPE(chan_id) sys_usw_dma_get_chan_type(lchip, chan_id)
extern int32
sys_usw_dma_get_chan_type(uint8 lchip, uint8 chan_id);

extern int32
sys_usw_dma_init(uint8 lchip, ctc_dma_global_cfg_t* dma_global_cfg);

extern void*
sys_usw_dma_tx_alloc(uint8 lchip, uint32 pkt_size);

extern int32
sys_usw_dma_tx_free(uint8 lchip, void* addr);

extern int32
sys_usw_dma_pkt_tx(ctc_pkt_tx_t* p_pkt_tx);

extern int32
sys_usw_dma_rw_table_user_dma_addr(uint8 lchip, sys_dma_tbl_rw_t* tbl_cfg);
extern int32
sys_usw_dma_rw_table(uint8 lchip, sys_dma_tbl_rw_t* tbl_cfg);

extern int32
sys_usw_dma_show_stats(uint8 lchip);

extern int32
sys_usw_dma_register_cb(uint8 lchip, uint8 type, void* cb);

extern int32
sys_usw_dma_get_dump_cb(uint8 lchip, void**cb, void** user_data);

extern int32
sys_usw_dma_sync_info_func(uint8 lchip, uint8 type, sys_dma_info_t* p_info);

extern int32
sys_usw_dma_clear_chan_data(uint8 lchip, uint8 chan_id);

extern int32
sys_usw_dma_set_chan_en(uint8 lchip, uint8 chan_id, uint8 chan_en);

extern int32
sys_usw_dma_get_chan_en(uint8 lchip, uint8 chan_id, uint8* chan_en);

extern int32
sys_usw_dma_wait_desc_done(uint8 lchip, uint8 chan_id);

extern int32
sys_usw_dma_get_packet_rx_chan(uint8 lchip, uint16* p_num);

extern int32
sys_usw_dma_get_hw_learning_sync(uint8 lchip, uint8* b_sync);

extern int32
sys_usw_dma_set_monitor_timer(uint8 lchip, uint32 timer);

extern int32
sys_usw_dma_set_mac_stats_timer(uint8 lchip, uint32 timer);

extern int32
sys_usw_dma_deinit(uint8 lchip);

extern int32
sys_usw_dma_stats_func(uint8 lchip, uint8 chan, uint8 dmasel);

extern int32
sys_usw_dma_set_pkt_timer(uint8 lchip, uint32 timer, uint8 enable);

extern int32
sys_usw_dma_set_packet_timer_cfg(uint8 lchip, uint16 max_session, uint16 interval, uint16 pkt_len, uint8 is_destroy);

extern int32
sys_usw_dma_set_session_pkt(uint8 lchip, uint16 session_id, ctc_pkt_tx_t* p_pkt);

extern int32
sys_usw_dma_set_tcam_scan_mode(uint8 lchip, uint8 mode, uint32 timer);
extern int32
sys_usw_dma_write_dynamic_table(uint8 lchip, uint32 tbl_addr, uint32* buffer, uint16 entry_num, uint16 entry_len, uint8 user_dma_mode);
#ifdef __cplusplus
}
#endif

#endif

