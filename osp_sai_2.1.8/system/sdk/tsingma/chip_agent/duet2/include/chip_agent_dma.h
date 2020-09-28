/**
  @file chip_agent_dma.h

  @date 2013-07-01

  @version v5.1

  The file implement driver for dma software simulation defines and macros
*/
/****************************************************************************
 * cm_com_dma.h  DMA for ASIC C-Model.
 * Copyright:      (c)2012 Centec Networks Inc. All rights reserved.
 *
 * Revision:        V1.0.
 * Author:         kcao.
 * Date:           2012-12-08.
 * Reason:         First Create.
 ****************************************************************************/
#ifndef _CHIP_AGENT_DMA_H_
#define _CHIP_AGENT_DMA_H_

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

/****************************************************************************
 *
* Defines and Macros
*
****************************************************************************/
#define AGENT_MAX_DMA_CHAN_NUM 4
#define AGENT_DMA_PACKET_TX_CHAN  4
#define AGENT_DMA_REG_READ_CHAN  7
#define AGENT_DMA_PACKET_TX_MAX_CHAN  5
#define AGENT_DMA_INFO_THREASHOLD  8
#define AGENT_DMA_INFO_TIMER_CNT  10    /* for 500ms */
#define AGENT_DMA_LEARNING_CHAN  11
#define AGENT_DMA_HASH_DUMP_CHAN  12
#define AGENT_DMA_IPFIX_CHAN  13

#define AGENT_DMA_FIB_LEARN_BYTES                                         20

enum cip_agent_dma_mode_e
{
   CHIP_AGENT_DMA_MODE_LEARNING,
   CHIP_AGENT_DMA_MODE_AGING,
   CHIP_AGENT_DMA_MODE_IPFIX,
   CHIP_AGENT_DMA_MODE_EFD,
   CHIP_AGENT_DMA_MODE_DUMP,
   CHIP_AGENT_DMA_MODE_MONITOR,

   CHIP_AGENT_DMA_MODE_MAX
};
typedef enum cip_agent_dma_mode_e cip_agent_dma_mode_t;

struct chip_agent_dma_info_s
{
    uint8 valid;
    uint8 rsv0[2];
    uint8 dma_mode;

    union
    {
        struct
        {
            uint8 fast_learning_en;
            uint8 is_gport;
            uint8 is_eth_oam;
            uint8 hash_type;   /*SDK-Modify*/

            uint8 is_add;
            uint8 eth_oam_level;
            mac_addr_t  mac;

            uint16 learning_src_port;
            uint16 vsi;
            uint32 old_svlan_id;
            uint32 old_cvlan_id;
            uint32 new_svlan_id;
            uint32 new_cvlan_id; //may be new cvlan id or ad index

            uint32 key_index;
            uint32 aging_ptr;   /*SDK-Modify*/

        }
        learning;
        struct
        {
            uint32 aging_ptr;
        }
        aging;
        struct
        {
            uint16 flow_id;
            uint8 op_code;
            uint8 rsv0;
        }
        efd;
        struct
        {
            mac_addr_t  mac;

            uint8 pending;
            uint8 valid;
            uint16 vsiid;

            uint32 ad_index;

            uint8 hash_type;
            uint8 rsv0[3];

            uint32 key_index;
        }
        mac;
        struct
        {
            //-struct IpfixSessionRecord_Fifo ipfix_session_record;

            uint32 export_reason;
        }
        ipfix;
        struct
        {
            uint16 statsPtr        :14;
            uint32 packetCount     :32;
            uint64 byteCount       :40;
        }
        stats;
        struct
        {
            uint32 info_type                            :1;
            uint32 channel_id                           :7;
            uint32 latency_event_data_timestamp_23_0    :24;

            uint32 latency_event_data_timestamp_55_24   :32;

            uint32 latency_event_data_timestamp_63_56   :8;
            uint32 latency_event_data_valid             :1;
            uint32 rsv_0                                :24;
        }
        latency;
    }u;
};
typedef struct chip_agent_dma_info_s chip_agent_dma_info_t;

typedef struct
{
    DsDesc_m desc_info;
    uint32   desc_padding0;
    uint32   desc_padding1;
} chip_agent_dma_desc_t;

typedef struct
{
    DmaFibLearnFifo_m learn_info;
    uint32 padding[3];
} chip_agent_fib_learn_t;

typedef uint32 chip_agent_fib_ipfix_t[16];
typedef struct
{
    void* p_data;                       /*fifo data */
    uint8 front;                           /* fifo header*/
    uint8 rear;                            /* fifo tail*/
    uint8 depth;                          /* fifo depth */
}chip_agent_fifo_t;

typedef enum
{
    CHIP_AGENT_FIFO_TYPE_LEARNING,
    CHIP_AGENT_FIFO_TYPE_HASH_DUMP,
    CHIP_AGENT_FIFO_TYPE_IPFIX,

    CHIP_AGENT_FIFO_TYPE_MAX
}chip_gent_fifo_type_t;

typedef struct
{
    chip_agent_fifo_t* p_fifo;
    sal_sem_t* p_fifo_sem;
    uint32 size;
}chip_agent_fifo_group_t;

/****************************************************************************
 *
* Global and Declarations
*
****************************************************************************/
//int32
//cm_sim_dma_tx_pkt(out_pkt_t* p_pkt);

#define SetDmaIpfixAccL2KeyFifo(X, ...)        DRV_SET_FLD(X, DmaIpfixAccL2KeyFifo, ##__VA_ARGS__)
#define GetDmaIpfixAccL2KeyFifo(X, ...)        DRV_GET_FLD(X, DmaIpfixAccL2KeyFifo, ##__VA_ARGS__)
#define SetDmaIpfixAccL2L3KeyFifo(X, ...)        DRV_SET_FLD(X, DmaIpfixAccL2L3KeyFifo, ##__VA_ARGS__)
#define GetDmaIpfixAccL2L3KeyFifo(X, ...)        DRV_GET_FLD(X, DmaIpfixAccL2L3KeyFifo, ##__VA_ARGS__)
#define SetDmaIpfixAccL3Ipv4KeyFifo(X, ...)        DRV_SET_FLD(X, DmaIpfixAccL3Ipv4KeyFifo, ##__VA_ARGS__)
#define GetDmaIpfixAccL3Ipv4KeyFifo(X, ...)        DRV_GET_FLD(X, DmaIpfixAccL3Ipv4KeyFifo, ##__VA_ARGS__)
#define SetDmaIpfixAccL3Ipv6KeyFifo(X, ...)        DRV_SET_FLD(X, DmaIpfixAccL3Ipv6KeyFifo, ##__VA_ARGS__)
#define GetDmaIpfixAccL3Ipv6KeyFifo(X, ...)        DRV_GET_FLD(X, DmaIpfixAccL3Ipv6KeyFifo, ##__VA_ARGS__)
#define SetDmaIpfixAccL3MplsKeyFifo(X, ...)        DRV_SET_FLD(X, DmaIpfixAccL3MplsKeyFifo, ##__VA_ARGS__)
#define GetDmaIpfixAccL3MplsKeyFifo(X, ...)        DRV_GET_FLD(X, DmaIpfixAccL3MplsKeyFifo, ##__VA_ARGS__)
#define GetDsIpfixL2HashKey(X, ...)        DRV_GET_FLD(X, DsIpfixL2HashKey, ##__VA_ARGS__)
#define GetDsIpfixSessionRecord(X, ...)        DRV_GET_FLD(X, DsIpfixSessionRecord, ##__VA_ARGS__)
#define GetDsIpfixL2L3HashKey(X, ...)        DRV_GET_FLD(X, DsIpfixL2L3HashKey, ##__VA_ARGS__)
#define GetDsIpfixL3Ipv4HashKey(X, ...)        DRV_GET_FLD(X, DsIpfixL3Ipv4HashKey, ##__VA_ARGS__)
#define GetDsIpfixL3Ipv6HashKey(X, ...)        DRV_GET_FLD(X, DsIpfixL3Ipv6HashKey, ##__VA_ARGS__)
#define GetDsIpfixL3MplsHashKey(X, ...)        DRV_GET_FLD(X, DsIpfixL3MplsHashKey, ##__VA_ARGS__)

#if (__WORDSIZE == 64)
#define CHIP_AGENT_COMBINE_64BITS_DATA(high, low, data) \
    {  \
        (data) = high; \
        (data) = (data <<32) | low; \
    }
#else
#define CHIP_AGENT_COMBINE_64BITS_DATA(high, low, data) \
    {  \
        (data) = low; \
    }
#endif

/*Cmodel need */
#if 0
/**
 @brief add the dma fifo
*/
extern int32
drv_model_dma_add_fifo(uint8 chip_id_offset, drv_model_dma_info_t * dma_info);

/**
 @brief get the dma fifo storeg status
*/
extern int32
drv_model_dma_is_not_full(uint8 chip_id_offset);

/**
 @brief set the dma fifo full, for ipfix module test
*/
extern int32
drv_model_dma_set_fifo_full(uint8 chip_id_offset);
#endif

#endif

