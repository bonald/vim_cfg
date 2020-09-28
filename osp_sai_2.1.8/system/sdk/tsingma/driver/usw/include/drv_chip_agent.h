#ifndef _DRV_CHIP_AGENT_H_
#define _DRV_CHIP_AGENT_H_

#include "sal.h"
#include "usw/include/drv_io.h"

#define GG_HEADER_LEN       40
#define GG_CPUMAC_HDR_LEN   32 /* MacDa + MacSa + VlanTag(4B) + EtherType(2B) + Rsv(2B) ??? */

#define MAX_ACC_BUF_IN  34   /* buf in  max table size is 32*/
#define MAX_ACC_BUF_OUT 34   /* buf out for ipfix max data size use 34*/
#define MAX_DMA_NUM 16*32    /* DMA_IPFIX_ACC_FIFO_BYTES(64=16words)*DMA_NUM */
#define MAX_DMA_EXT_NUM 2*32 /* sys_dma_desc_ts_t(8=2 words)*DMA_NUM */

#define DRV_CHIP_AGT_LOG            sal_printf

#define DRV_EADP_DBG_INFO(enable,FMT, ...)\
        if(enable)\
        {\
            DRV_DBG_INFO(FMT, ##__VA_ARGS__);\
        }\

typedef enum
{
    DRV_CHIP_AGT_E_NONE = 0,                /**< NO error */

    DRV_CHIP_AGT_E_GEN = -20000,
    DRV_CHIP_AGT_E_TASK,
    DRV_CHIP_AGT_E_MUTEX_CREAT,
    DRV_CHIP_AGT_E_MODE_SERVER,
    DRV_CHIP_AGT_E_MODE_CLIENT,
    DRV_CHIP_AGT_E_SOCK_SYNC_TO,
    DRV_CHIP_AGT_E_SOCK_ERROR,
    DRV_CHIP_AGT_E_SOCK_CREAT,
    DRV_CHIP_AGT_E_SOCK_EXIST,
    DRV_CHIP_AGT_E_SOCK_NONEXIST,
    DRV_CHIP_AGT_E_SOCK_SET_OPT,
    DRV_CHIP_AGT_E_SOCK_ADDR,
    DRV_CHIP_AGT_E_SOCK_CONNECT,
    DRV_CHIP_AGT_E_SOCK_NOT_CONNECT,
    DRV_CHIP_AGT_E_SOCK_BIND,
    DRV_CHIP_AGT_E_SOCK_LISTEN,
    DRV_CHIP_AGT_E_MSG_LEN,
    DRV_CHIP_AGT_E_MSG_LEN_EXCEED,
    DRV_CHIP_AGT_E_MSG_IO_TYPE,
    DRV_CHIP_AGT_E_IO_UNSUP,
    DRV_CHIP_AGT_E_VER_MISMATCH,
    DRV_CHIP_AGT_E_DATE_MISMATCH,
    DRV_CHIP_AGT_E_PROFILE_MISMATCH,
    DRV_CHIP_AGT_E_ENGINE_MISMATCH,
    DRV_CHIP_AGT_E_DECODE_NULL_PTR,
    DRV_CHIP_AGT_E_DECODE_ZERO_LEN,
    DRV_CHIP_AGT_E_SIM_DGB_ACT_SAME,
    DRV_CHIP_AGT_E_SIM_DGB_ACT_LOCAL_PRINTF,
    DRV_CHIP_AGT_E_SIM_DGB_ACT_REMOTE_PRINTF,
    DRV_CHIP_AGT_E_SIM_DGB_ACT_REMOTE_FILE,
    DRV_CHIP_AGT_E_MAX
}drv_chip_agt_err_t;

enum chip_agent_dma_op_type_e
{
    CHIP_AGENT_DMA_INFO_TYPE_LEARN,
    CHIP_AGENT_DMA_INFO_TYPE_HASHDUMP,
    CHIP_AGENT_DMA_INFO_TYPE_IPFIX,
    CHIP_AGENT_DMA_INFO_TYPE_SDC,
    CHIP_AGENT_DMA_INFO_TYPE_MONITOR,
    CHIP_AGENT_DMA_INFO_TYPE_AGING,
    CHIP_AGENT_DMA_INFO_TYPE_MAX
};
typedef  enum chip_agent_dma_op_type_e chip_agent_dma_op_type_t;

#define CHIP_AGT_BUF_SIZE       5120
#define CHIP_AGT_DBG_SOCK(FMT, ...) \
    CTC_DEBUG_OUT(chipagent, chipagent, CHIP_AGT_SOCKET, CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)

#define CHIP_AGT_DBG_CODE(FMT, ...) \
    CTC_DEBUG_OUT(chipagent, chipagent, CHIP_AGT_CODE, CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)

#define CHIP_AGT_LOCK(mutex_type) \
    if (mutex_type) sal_mutex_lock(mutex_type)

#define CHIP_AGT_UNLOCK(mutex_type) \
    if (mutex_type) sal_mutex_unlock(mutex_type)


#define DRV_IO_DATA_SIZE                16
#define DRV_IO_DEBUG_CACHE_SIZE         256


typedef enum chip_io_op_s
{
    /*------USW---------------*/
    CHIP_IO_OP_IOCTL_SRAM_READ,   /*[D2]*/
    CHIP_IO_OP_IOCTL_SRAM_WRITE,  /*[D2]*/
    CHIP_IO_OP_IOCTL_TCAM_READ,   /*[D2]*/
    CHIP_IO_OP_IOCTL_TCAM_WRITE,  /*[D2]*/
    CHIP_IO_OP_IOCTL_TCAM_REMVOE, /*[D2]*/
    CHIP_IO_OP_DMA_DUMP,          /*[GG].[D2]*/

    CHIP_IO_OP_IOCTL,          /*[GB,GG]*/
    CHIP_IO_OP_TCAM_REMOVE,    /*[GB,GG]*/
    CHIP_IO_OP_HASH_KEY_IOCTL, /*[GB]*/
    CHIP_IO_OP_HASH_LOOKUP,    /*[GB]*/
    CHIP_IO_OP_FIB_ACC,        /*[GG]*/
    CHIP_IO_OP_CPU_ACC,        /*[GG]*/
    CHIP_IO_OP_IPFIX_ACC,      /*[GG]*/


    CHIP_IO_OP_MAX,
} chip_io_op_t;

typedef struct chip_io_para_s
{
    chip_io_op_t op;

    uint8 lchip;

    union
    {
        struct
        {
            uint32 tbl_id;
            uint32 index;
            void* val;
        } ioctl;

        struct
        {
            uint16 threshold;
            uint16 entry_num;
            void* val;
        } dma_dump;
    } u;
} chip_io_para_t;


/* Driver IO entry */
typedef struct
{
    sal_systime_t   tv;             /* timestamp */
    uint8 data[DRV_IO_DATA_SIZE];   /* used to store content of pointer in IO parameter */
} drv_io_debug_cache_entry_t;

/* Driver IO cache */
typedef struct
{
    uint32          count;
    uint32          index;
    drv_io_debug_cache_entry_t entry[DRV_IO_DEBUG_CACHE_SIZE];
} drv_io_debug_cache_info_t;

typedef int32 (* DRV_IO_AGENT_CALLBACK)(uint8, void*);

typedef struct
{
    DRV_IO_AGENT_CALLBACK       chip_agent_cb;  /* chip_agent callback */
    DRV_IO_AGENT_CALLBACK       debug_cb;       /* debug callback */
    uint32                      debug_stats[CHIP_IO_OP_MAX];
    uint32                      debug_action;
    drv_io_debug_cache_info_t   cache;
} drv_io_t;


/* parameter for CHIP_AGT_MSG_MODEL */
typedef struct
{
    uint32 type;
    uint32 param_count;
    uint32 param[5];
    int32 ret;
} chip_agent_msg_model_para_t;

/* parameter for CHIP_AGT_MSG_INIT */
typedef struct chip_agent_msg_init_para_s
{
    uint8 profile_type;
    uint8 endian;
    uint8 ver_len;
    uint8 date_len;
    uint32 is_asic;
    uint32 sim_debug_action;
    uint32 sim_packet_debug;
    int32 ret;
    char* ver;
    char* date;
} chip_agent_msg_init_para_t;

/* parameter for CHIP_AGT_MSG_CTRL */
typedef struct
{
    uint32 action;
    uint32 type;
    uint32 flag;
    uint32 enable;
    int32 ret;
} chip_agent_msg_ctrl_para_t;


/* parameter for CHIP_AGT_MSG_IO common header */
typedef struct
{
    uint8 op;
    uint8 lchip;
    uint16 len;
    int32 ret;
} chip_agent_msg_io_hdr_t;


/*------------------------------------------------------
             Messager IO structure
---------------------------------------------------------*/
typedef struct
{
    chip_agent_msg_io_hdr_t hdr;
    chip_io_para_t para;
} chip_agent_msg_io_para_t;



/*------------------------------------------------------
             Messager DMA dump  structure
---------------------------------------------------------*/
typedef struct
{
    chip_agent_msg_io_hdr_t hdr;
    uint32 ret;
    uint16 threshold;
    uint16 entry_num;
    uint32 acc_buf_out[MAX_DMA_NUM]; /*buf out*/
} chip_agent_msg_dma_dump_t;

/* parameter for CHIP_AGT_MSG_PACKET */
typedef struct
{
    uint32 pkt_len;
    uint32 pkt_mode;
    uint8* pkt;
} chip_agent_msg_pkt_para_t;

/* parameter for CHIP_AGT_MSG_PACKET_RX_INFO */
typedef struct
{
    uint32 pkt_len;
    uint32 pkt_mode;
    uint32 dma_chan;    /**< [GB] DMA Channel ID of packet RX */
    uint32 buf_count;   /**< [GB] Count of packet buffer array */
    uint32 pkt_buf_len;
    uint8* pkt;
} chip_agent_msg_pkt_rx_para_t;

/* parameter for CHIP_AGT_MSG_INTERRUPT */
typedef struct
{
    uint32 group;
} chip_agent_msg_interrupt_para_t;

/* parameter for CHIP_AGT_MSG_INTERRUPT */
typedef struct
{
    uint32 lchip;
    uint32 buf[MAX_DMA_NUM];   /*buf in  max table size is 32*/
} chip_agent_msg_interrupt_oam_t;

/* parameter for CHIP_AGT_MSG_DMA common header */
typedef struct
{
    uint8 op;
    uint8 lchip;
    uint16 num;
    uint32 data_buf[MAX_DMA_NUM];
    uint32 data_ext[MAX_DMA_EXT_NUM];
} chip_agent_msg_dma_para_t;

/* parameter for CHIP_AGT_MSG_LOG */
typedef struct
{
    uint32 len;
    uint8* data;
} chip_agent_msg_log_para_t;



/*------------------------------------------------------
*
*  aging info
*
---------------------------------------------------------*/
struct drv_learn_aging_info_s
{
    uint32 key_index;

    mac_addr_t   mac;
    uint16 vsi_id;

    uint16 damac_index;
    uint8 is_mac_hash;
    uint8 valid;
    uint8 is_aging;
};
typedef struct drv_learn_aging_info_s  drv_learn_aging_info_t;


#define ENCODE_PUTC(V)                      \
    do {                                    \
        *(*pnt) = (V) & 0xFF;               \
        (*pnt)++;                           \
        (*size)--;                          \
    } while (0)

#define ENCODE_PUTW(V)                      \
    do {                                    \
        *(*pnt) = ((V) >> 8) & 0xFF;        \
        *(*pnt + 1) = (V) & 0xFF;           \
        *pnt += 2;                          \
        *size -= 2;                         \
    } while (0)

#define ENCODE_PUTL(V)                      \
    do {                                    \
        *(*pnt) = ((V) >> 24) & 0xFF;       \
        *(*pnt + 1) = ((V) >> 16) & 0xFF;   \
        *(*pnt + 2) = ((V) >> 8) & 0xFF;    \
        *(*pnt + 3) = (V) & 0xFF;           \
        *pnt += 4;                          \
        *size -= 4;                         \
    } while (0)

#define ENCODE_PUT_EMPTY(L)                 \
    do {                                    \
        sal_memset((void*)(*pnt), 0, (L));  \
        *pnt += (L);                        \
        *size -= (L);                       \
    } while (0)

#define ENCODE_PUT(P, L)                                    \
    do {                                                    \
        sal_memcpy((void*)(*pnt), (void*)(P), (L));         \
        *pnt += (L);                                        \
        *size -= (L);                                       \
    } while (0)

#define DECODE_GETC(V)                                      \
    do {                                                    \
        (V) = **pnt;                                        \
        (*pnt)++;                                           \
        (*size)--;                                          \
    } while (0)

#define DECODE_GETW(V)                                      \
    do {                                                    \
        (V) = ((*(*pnt)) << 8)                              \
            + (*(*pnt + 1));                              \
        *pnt += 2;                                          \
        *size -= 2;                                         \
    } while (0)

#define DECODE_GETL(V)                                      \
    do {                                                    \
        (V) = ((uint32_t)(*(*pnt)) << 24)                   \
            + ((uint32_t)(*(*pnt + 1)) << 16)             \
            + ((uint32_t)(*(*pnt + 2)) << 8)              \
            + (uint32_t)(*(*pnt + 3));                    \
        *pnt += 4;                                          \
        *size -= 4;                                         \
    } while (0)

#define DECODE_GET(V, L)                                    \
    do {                                                    \
        sal_memcpy((void*)(V), (const void*)(*pnt), (L));   \
        *pnt += (L);                                        \
        *size -= (L);                                       \
    } while (0)

#define DECODE_GET_EMPTY(L)                                 \
    do {                                                    \
        *pnt += (L);                                        \
        *size -= (L);                                       \
    } while (0)

#define CHIP_AGT_DECODE_BUF(P, L)                           \
    do {                                                    \
        if (L)                                              \
        {                                                   \
            if ((P))                                        \
            {                                               \
                DECODE_GET((P), (L));                       \
            }                                               \
            else                                            \
            {                                               \
                return DRV_CHIP_AGT_E_DECODE_NULL_PTR;          \
            }                                               \
        }                                                   \
    } while (0)




enum sys_dma_cb_type_e
{
    SYS_DMA_CB_TYPE_LERNING,
    SYS_DMA_CB_TYPE_IPFIX,
    SYS_DMA_CB_TYPE_MONITOR,
    SYS_DMA_CB_TYPE_PORT_STATS,
    SYS_DMA_CB_TYPE_FLOW_STATS,

    SYS_DMA_CB_MAX_TYPE
};
typedef  enum sys_dma_cb_type_e sys_dma_cb_type_t;


typedef int32 (*DRV_EADP_PKT_RX_FUNC)(uint8* pkt, uint32 mode, uint32 pkt_len);
typedef int32 (*DRV_AGENT_OAM_DEFECT_PROCESS_FUNC)(uint8 lchip, void* p_data);
typedef int32 (*DRV_AGENT_DMA_FUNC)(uint8 lchip, void* p_data);
typedef int32 (*DRV_AGENT_DMA_DUMP_FUNC)(uint8 lchip, uint16* p_entry_num, void* p_data, uint16 threshold);
typedef int32 (*DRV_AGENT_IPFIX_EXPORT_FUNC)(uint8 lchip, uint8 exp_reason, uint64 pkt_cnt, uint64 byte_cnt);
typedef int32 (*DRV_AGENT_PKT_FUNC)(uint8 lchip, void* p_pkt_rx);
typedef int32 (*DRV_AGENT_PKT_CB)(void* p_pkt_rx);


typedef int32 (*DRV_AGENT_SET_DMA_CB)(uint8 lchip, uint8 type,  void* cb);
typedef int32 (*DRV_AGENT_SET_CB)(uint8 lchip, void* cb);
typedef int32 (*DRV_AGENT_GET_CB)(uint8 lchip, void **cb, void** user_data);




#define FUNC_DRV_AGENT_CB_SET_OAM_DEFECT g_drv_agent.set_oam_defect_cb
#define FUNC_DRV_AGENT_CB_SET_PKT_RX     g_drv_agent.set_packet_rx_cb
#define FUNC_DRV_AGENT_CB_SET_PKT_TX     g_drv_agent.set_packet_tx_cb
#define FUNC_DRV_AGENT_CB_SET_DMA_RX     g_drv_agent.set_dma_dump_cb
#define FUNC_DRV_AGENT_CB_SET_DMA_DUMP   g_drv_agent.set_dma_cb
#define FUNC_DRV_AGENT_CB_GET_LEARN      g_drv_agent.get_learn_cb
#define FUNC_DRV_AGENT_CB_GET_AGING      g_drv_agent.get_aging_cb
#define FUNC_DRV_AGENT_CB_GET_IPFIX      g_drv_agent.get_ipfix_cb
#define FUNC_DRV_AGENT_CB_GET_MONITOR    g_drv_agent.get_monitor_cb
#define FUNC_DRV_AGENT_CB_GET_OAM_DEFECT g_drv_agent.get_oam_defect_cb
#define FUNC_DRV_AGENT_CB_GET_DMA_DUMP   g_drv_agent.get_dma_dump_cb
#define FUNC_DRV_AGENT_CB_IPFIX_EXPORT   g_drv_agent.ipfix_export_stats
#define FUNC_DRV_AGENT_CB_PKT_TX         g_drv_agent.dma_pkt_tx
#define FUNC_DRV_AGENT_CB_PKT_RX         g_drv_agent.packet_rx

#define DRV_AGENT_FUNC(cb)  (*FUNC_##cb)


typedef struct
{
    DRV_AGENT_SET_CB  set_oam_defect_cb;
    DRV_AGENT_SET_CB  set_packet_rx_cb;
    DRV_AGENT_SET_CB  set_packet_tx_cb;
    DRV_AGENT_SET_CB  set_dma_dump_cb;
    DRV_AGENT_SET_DMA_CB  set_dma_cb;
    DRV_AGENT_GET_CB get_learn_cb;
    DRV_AGENT_GET_CB get_aging_cb;
    DRV_AGENT_GET_CB get_oam_defect_cb;
    DRV_AGENT_GET_CB get_ipfix_cb;
    DRV_AGENT_GET_CB get_monitor_cb;
    DRV_AGENT_GET_CB get_dma_dump_cb;
    DRV_AGENT_IPFIX_EXPORT_FUNC ipfix_export_stats;
    DRV_AGENT_PKT_FUNC packet_rx;
    DRV_AGENT_PKT_CB  dma_pkt_tx;
    DRV_IO_AGENT_CALLBACK  chip_agent_cb;

} drv_chip_agent_t;


extern drv_chip_agent_t g_drv_agent;

int32 drv_usw_agent_io_callback(chip_agent_msg_io_para_t *p_msg_io);
int32 drv_usw_register_chip_agent_cb(DRV_IO_AGENT_CALLBACK cb);

int32 drv_usw_agent_init(uint8 lchip);

int32 drv_usw_chip_agent_mode(void);

int32 drv_usw_agent_encode_init(uint8* buf, chip_agent_msg_init_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_init(uint8* buf, chip_agent_msg_init_para_t* para, uint32 len);

int32 drv_usw_agent_encode_model(uint8* buf, chip_agent_msg_model_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_model(uint8* buf, chip_agent_msg_model_para_t* para, uint32 len);

int32 drv_usw_agent_encode_ctrl(uint8* buf, chip_agent_msg_ctrl_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_ctrl(uint8* buf, chip_agent_msg_ctrl_para_t* para, uint32 len);

int32 drv_usw_agent_encode_pkt(uint8* buf, chip_agent_msg_pkt_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_pkt(uint8* buf, chip_agent_msg_pkt_para_t* para, uint32 len);

int32 drv_usw_agent_encode_log(uint8* buf, chip_agent_msg_log_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_log(uint8* buf, chip_agent_msg_log_para_t* para, uint32 len);


int32 drv_usw_agent_encode_io_ioctl(uint8* buf, chip_agent_msg_io_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_io_header(uint8* buf, chip_agent_msg_io_hdr_t* hdr, uint32 len);
int32 drv_usw_agent_decode_io_ioctl(uint8* buf, chip_agent_msg_io_para_t* para, uint32 len);

int32 drv_usw_agent_decode_rx_pkt(uint8* buf, chip_agent_msg_pkt_rx_para_t* para, uint32 len);
int32 drv_usw_agent_encode_dma(uint8* buf, chip_agent_msg_dma_para_t* para, uint32* req_len);
int32 drv_usw_agent_decode_dma(uint8* buf, chip_agent_msg_dma_para_t* para, uint32 len);


#endif
