/****************************************************************************
 * chip_agent_dma.c  cmodel DMA simulation
 * Copyright:    (c)2011 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:
 * Author:
 * Date:         2012-12-08
 * Reason:
 *
 ****************************************************************************/
#include "sal.h"
#include "drv_api.h"
#include "chip_agent_dma.h"
#include "cm_com_remote.h"
#define CTC_MAX_AGENT_NUM          30
enum cm_interrupt_type_gg_e
{
    CM_INTR_CHIP_FATAL = 0,
    CM_INTR_CHIP_NORMAL = 1,

    CM_INTR_FUNC_PTP_TS_CAPTURE_FIFO = 2,
    CM_INTR_FUNC_PTP_TOD_PLUSE_IN = 3,
    CM_INTR_FUNC_PTP_TOD_CODE_IN_RDY = 4,
    CM_INTR_FUNC_PTP_SYNC_PLUSE_IN = 5,
    CM_INTR_FUNC_PTP_SYNC_CODE_IN_RDY = 6,
    CM_INTR_FUNC_PTP_SYNC_CODE_IN_ACC = 7,

    CM_INTR_FUNC_STATS_STATUS_ADDR = 8,
    CM_INTR_FUNC_MET_LINK_SCAN_DONE = 9,

    CM_INTR_FUNC_MDIO_CHANGE_0 = 10,
    CM_INTR_FUNC_MDIO_CHANGE_1 = 11,
    CM_INTR_FUNC_CHAN_LINKDOWN_SCAN = 12,
    CM_INTR_FUNC_IPFIX_USEAGE_OVERFLOW = 13,
    CM_INTR_FUNC_FIB_ACC_LEARN_OUT_FIFO = 14,

    CM_INTR_FUNC_OAM_DEFECT_CACHE = 15,

    CM_INTR_FUNC_BSR_C2C_PKT_CONGESTION = 16,
    CM_INTR_FUNC_BSR_TOTAL_CONGESTION = 17,
    CM_INTR_FUNC_BSR_OAM_CONGESTION = 18,
    CM_INTR_FUNC_BSR_DMA_CONGESTION = 19,
    CM_INTR_FUNC_BSR_CRITICAL_PKT_CONGESTION = 20,

    CM_INTR_FUNC_AUTO_GEN = 21,

    CM_INTR_PCS_LINK_31_0 = 22,
    CM_INTR_PCS_LINK_47_32 = 23,

    CM_INTR_FUNC_MDIO_XG_CHANGE_0 = 24,
    CM_INTR_FUNC_MDIO_XG_CHANGE_1 = 25,

    CM_INTR_PCIE_BURST_DONE = 26,
    CM_INTR_DMA = 27,
    CM_INTR_MCU_SUP = 28,
    CM_INTR_MCU_NET = 29,
};
typedef enum cm_interrupt_type_gg_e cm_interrupt_type_gg_t;

#define CM_INTR_INDEX_VAL_SET           0
#define CM_INTR_INDEX_VAL_RESET         1
#define CM_INTR_INDEX_MASK_SET          2
#define CM_INTR_INDEX_MASK_RESET        3
#define CM_INTR_INDEX_MAX               4

#define CM_INTR_STAT_SIZE               3
#define CM_INTR_ALL                     ((uint32)-1)    /**< all interrupt bits of sup-level/sub-level */

#define _CM_BMP_OP(_bmp, _offset, _op)     \
    (_bmp[(_offset) / 32] _op (1U << ((_offset) % 32)))

#define CM_BMP_INIT(bmp)                   \
    sal_memset((bmp), 0, sizeof((bmp)));
#define CM_BMP_SET(bmp, offset)            \
    _CM_BMP_OP((bmp), (offset), |=)


/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/
enum chip_agent_thread_type_e
{
    CHIP_AGENT_DMA_PKT_T,         /*[sdk modify] for dma pkt */
    CHIP_AGENT_DMA_LEARN_T,      /*[sdk modify] for dma learning/aging */
    CHIP_AGENT_DMA_HASH_DUMP_T,      /*[sdk modify] for dma hash dump */
    CHIP_AGENT_DMA_IPFIX_T,      /*[sdk modify] for dma hash dump */
    CHIP_AGENT_DMA_REG_T,      /*[sdk modify] for dma read */
    CHIP_AGENT_MAX_T,
};
typedef enum chip_agent_thread_type_e chip_agent_thread_type_t;

struct dma_agent_master_s
{
    sal_task_t* agent_thread[CHIP_AGENT_MAX_T];
    chip_agent_fifo_group_t agent_fifo[CHIP_AGENT_FIFO_TYPE_MAX];
    uint32 agent_info_count[CHIP_AGENT_FIFO_TYPE_MAX];
    uint8 agent_is_timeout[CHIP_AGENT_FIFO_TYPE_MAX];
};
typedef struct dma_agent_master_s   dma_agent_master_t;

dma_agent_master_t* p_dma_agent_master[CTC_MAX_AGENT_NUM] = {NULL};

/*temply*/
struct chip_agent_pkt_s
{
    uint8 chan_id;
    uint8 chip_id;
    void* pkt;
    uint32 packet_length;

};
typedef struct chip_agent_pkt_s chip_agent_pkt_t;

#define DMA_TX_CHANID                   79
#define AGENT_MTU                   (9600 + 64) /* 64 bytes for forwarding headers */
#define MAX_LOCAL_CHIP_NUM 1

/****************************************************************************
 *
 * Global and Declarations
 *
****************************************************************************/

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

uint32 agent_dma_debug_flag = 0;

#define AGENT_DMA_DBG(fmt, args...) \
do \
{ \
    if (agent_dma_debug_flag) \
        sal_printf(fmt, ##args); \
} while (0)


#define ___________COMMOM___________
/* get WORD in chip */
static inline int32
_chip_agent_dma_get_words_in_chip(uint32 entry_words, uint32* words_in_chip)
{
    if (1 == entry_words)
    {
        *words_in_chip = 1;
    }
    else if (2 == entry_words)
    {
        *words_in_chip = 2;
    }
    else if ((3 <= entry_words) && (entry_words <= 4))
    {
        *words_in_chip = 4;
    }
    else if ((5 <= entry_words) && (entry_words <= 8))
    {
        *words_in_chip = 8;
    }
    else if ((9 <= entry_words) && (entry_words <= 16))
    {
        *words_in_chip = 16;
    }
    else if ((17 <= entry_words) && (entry_words <= 32))
    {
        *words_in_chip = 32;
    }
    else if ((33 <= entry_words) && (entry_words <= 64))
    {
        *words_in_chip = 64;
    }
    else
    {
        return -1;
    }

    return 0;
}


int32
chip_agent_fifo_init(uint8 lchip, uint8 depth)
{
    uint8 index = 0;
    chip_agent_fifo_t* p_fifo_sample = NULL;

    for (index = 0; index < CHIP_AGENT_FIFO_TYPE_MAX; index++)
    {
        p_dma_agent_master[lchip]->agent_fifo[index].p_fifo = sal_malloc(p_dma_agent_master[lchip]->agent_fifo[index].size);
        if (NULL == p_dma_agent_master[lchip]->agent_fifo[index].p_fifo)
        {
            return -1;
        }

        p_fifo_sample = p_dma_agent_master[lchip]->agent_fifo[index].p_fifo;
        sal_memset(p_fifo_sample, 0, p_dma_agent_master[lchip]->agent_fifo[index].size);

        /* plug 1 means using 1 member to adjust full */
        p_fifo_sample->depth = depth+1;

        p_fifo_sample->p_data = sal_malloc((depth+1)*p_dma_agent_master[lchip]->agent_fifo[index].size);
        sal_memset(p_fifo_sample->p_data, 0, p_dma_agent_master[lchip]->agent_fifo[index].size*(depth+1));

        p_fifo_sample->front = 0;
        p_fifo_sample->rear = 0;

        sal_sem_create(&p_dma_agent_master[lchip]->agent_fifo[index].p_fifo_sem, 0);
    }

    return 0;
}


uint8
chip_agent_fifo_get_length(uint8 lchip, uint8 fifo_type)
{
    chip_agent_fifo_t* p_fifo = NULL;


    if (fifo_type >= CHIP_AGENT_FIFO_TYPE_MAX)
    {
        return -1;
    }
    /* special used for check fifo full */

    p_fifo = p_dma_agent_master[lchip]->agent_fifo[fifo_type].p_fifo;

    if (NULL == p_fifo)
    {
        return -1;
    }

    return ((p_fifo->rear - p_fifo->front + p_fifo->depth)%p_fifo->depth);
}

int32
_chip_agent_fifo_push(uint8 lchip, uint8 fifo_type, void* p_info)
{
    chip_agent_fifo_t* p_fifo = NULL;
    uint8* p_user_data_learn = NULL;
    uint8* p_fifo_data_learn = NULL;
    uint32 size = 0;

    if (fifo_type >= CHIP_AGENT_FIFO_TYPE_MAX)
    {
        return -1;
    }

    p_fifo = p_dma_agent_master[lchip]->agent_fifo[fifo_type].p_fifo;
    p_user_data_learn = (uint8*)p_info;
    p_fifo_data_learn = (uint8*)p_fifo->p_data;
    size =  p_dma_agent_master[lchip]->agent_fifo[fifo_type].size;

    if (NULL == p_fifo)
    {
        return -1;
    }

    if (NULL == p_info)
    {
        return -1;
    }

    AGENT_DMA_DBG("Push data to Dma Fifo, front:%d rear:%d \n", p_fifo->front, p_fifo->rear);

    /* check fifo is full */
    if (chip_agent_fifo_get_length(lchip, fifo_type) == 32)
    {
        sal_printf("[CM] Dma Fifo is full!\n");
        return -1;
    }

    sal_memcpy((uint8*)&p_fifo_data_learn[(p_fifo->rear)*size], (uint8*)p_user_data_learn, size);

    p_fifo->rear = (p_fifo->rear + 1)%p_fifo->depth;

    //-sal_sem_give(chip_agent_fifo[fifo_type].p_fifo_sem);

    return 0;
}

int32
chip_agent_fifo_pop(uint8 lchip, uint8 fifo_type, uint8 pop_num, void* p_info)
{
    chip_agent_fifo_t* p_fifo = NULL;
    uint8 fifo_len = 0;
    uint8 act_num = pop_num;
    uint8 index = 0;
    uint8* p_user_data_learn = NULL;
    uint8* p_fifo_data_learn = NULL;
    uint32 size = 0;

    if (fifo_type >= CHIP_AGENT_FIFO_TYPE_MAX)
    {
        return -1;
    }

    p_fifo = p_dma_agent_master[lchip]->agent_fifo[fifo_type].p_fifo;
    p_user_data_learn = (uint8*)p_info;
    p_fifo_data_learn = (uint8*)p_fifo->p_data;
    size =  p_dma_agent_master[lchip]->agent_fifo[fifo_type].size;

    if (NULL == p_fifo)
    {
        return -1;
    }

    AGENT_DMA_DBG("Pop data from Dma Fifo, front:%d rear:%d num:%d\n", p_fifo->front, p_fifo->rear, pop_num);

    fifo_len = chip_agent_fifo_get_length(lchip, fifo_type);
    if (pop_num > fifo_len)
    {
        act_num = fifo_len;
    }

    for (index = 0; index < act_num; index++)
    {
        sal_memcpy((uint8*)&p_user_data_learn[index*size], (uint8*)&p_fifo_data_learn[(p_fifo->front)*size], size);
        p_fifo->front = (p_fifo->front + 1)%p_fifo->depth;
    }

    return 0;
}

int32
_cm_com_interrupt_set_value(uint8 lchip, uint32 tbl_id, uint32 fld_id, uint32 bit_offset, uint32 enable)
{
    uint32 cmd = 0;
    uint32 value[CM_INTR_STAT_SIZE];
    uint32 value_set[CM_INTR_STAT_SIZE];
    uint32 value_reset[CM_INTR_STAT_SIZE];

    CM_BMP_INIT(value);
    CM_BMP_INIT(value_set);
    CM_BMP_INIT(value_reset);

    if (enable)
    {
        /* trigger/set */
        /* 1. read value */
        cmd = DRV_IOR(tbl_id, fld_id);
        (DRV_IOCTL(lchip, CM_INTR_INDEX_VAL_SET, cmd, value_set));
        (DRV_IOCTL(lchip, CM_INTR_INDEX_VAL_RESET, cmd, value_reset));

        /* 2. do reset action */
        if (CM_INTR_ALL != bit_offset)
        {
            CM_BMP_SET(value, bit_offset);
            value_set[0] |= value[0];
            value_set[1] |= value[1];
            value_set[2] |= value[2];
        }

        /* 3. write value */
        cmd = DRV_IOW(tbl_id, fld_id);
        (DRV_IOCTL(lchip, CM_INTR_INDEX_VAL_SET, cmd, value_set));
        (DRV_IOCTL(lchip, CM_INTR_INDEX_VAL_RESET, cmd, value_set));
    }
    else
    {
        /* clear */
    }

    return DRV_E_NONE;
}

int32
cm_com_interrupt_set(uint8 lchip, uint32 enable, uint32 intr, uint32 sub_intr)
{
    uint32 tbl_id = 0;
    uint32 fld_id = DRV_ENTRY_FLAG;
    uint32 bit_offset = 0;
    uint32 cmd = 0;
    DmaIntrVecRec_m intr_rec_m;
    uint32 intr_rec = 0;
    uint32 intr_vec[1];

    if ((intr == CM_INTR_DMA))
    {
        cmd = DRV_IOR(DmaIntrVecRec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &intr_rec_m);
        DRV_IOR_FIELD(DmaIntrVecRec_t, DmaIntrVecRec_dmaIntrVecRec_f, &intr_rec, &intr_rec_m);
        intr_rec |= (1 << sub_intr);
        DRV_IOW_FIELD(DmaIntrVecRec_t, DmaIntrVecRec_dmaIntrVecRec_f, &intr_rec, &intr_rec_m);
        cmd = DRV_IOW(DmaIntrVecRec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip,0, cmd, &intr_rec_m);

        tbl_id = DmaCtlIntrFunc_t;
        bit_offset = sub_intr;
        _cm_com_interrupt_set_value(lchip, tbl_id, fld_id, bit_offset, enable);
    }


    /*set supintrfunc*/
    if((intr != CM_INTR_CHIP_FATAL) && (intr != CM_INTR_CHIP_NORMAL))
    {
        _cm_com_interrupt_set_value(lchip, SupIntrFunc_t, fld_id, intr-2, enable);
    }

    /*set SupCtlIntrVec_t*/
    CM_BMP_INIT(intr_vec);
    cmd = DRV_IOR(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &intr_vec);
    CM_BMP_SET(intr_vec, intr);
    cmd = DRV_IOW(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &intr_vec);
    return DRV_E_NONE;
}

#define __________DMA_2_CPU______________

/* Direct: Dma->Cpu */
STATIC int32
chip_agent_dma_reg_process(uint8 lchip, uint8 chan_id)
{
    chip_agent_dma_desc_t* p_desc_base = NULL;
    uint32 cmd = 0;
    uint32 value = 0;
    DmaCtlTab_m ctl_tab;
    DmaStaticInfo_m static_info;
    DmaDynInfo_m dyn_info;
    DsDesc_m* p_desc = NULL;
    uint32 valid_cnt = 0;
    uint32 chan_en = 0;
    uint32 ring_depth = 0;
    uintptr ring_base = 0;
    uint32 cur_idx = 0;
    uintptr phy_addr = 0;
    uint32 cfg_addr = 0;
    uint32 length = 0;
    uint32 tb_id = 0;
    uint32 tb_index = 0;
    uint32 word_num = 0;
    uint32 chip_word_num = 0;
    uint32 entry_num = 0;
    uint32 index = 0;
    uint32 high_addr = 0;
    uint32 tmp_base = 0;
    uint8* buf_tmp = NULL;

    sal_memset(&ctl_tab, 0, sizeof(DmaCtlTab_m));
    sal_memset(&static_info, 0, sizeof(static_info));

    /* check chan enable */
    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, chan_id, cmd, &static_info));
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_chanEn_f, &chan_en, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f, &ring_depth, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringBase_f, (uint32*)&tmp_base, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

    CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (tmp_base << 4), ring_base);

    if (chan_en == 0)
    {
        //AGENT_DMA_DBG("chan %d is not enable \n", chan_id);
        return 0;
    }

    /* check desc valid cnt */
    cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, chan_id, cmd, &ctl_tab));
    DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);

    if (valid_cnt == 0)
    {
        //AGENT_DMA_DBG("chan %d is not enable \n", chan_id);

        /* trigger interrupt */
        return 0;
    }

    /* Get desc base, depth, valid num */
    p_desc_base = (chip_agent_dma_desc_t*)ring_base;

    AGENT_DMA_DBG("[DMA REG Engine TX] desc_base %p depth %d valid %d \n", ring_base,
        ring_depth, valid_cnt);

    /*Get Current desc information*/
    cmd = DRV_IOR(DmaDynInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, chan_id, cmd, &dyn_info));
    DRV_IOR_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);

    /* Copy data to desc dataAddress */
    p_desc = &(p_desc_base[cur_idx].desc_info);
    DRV_IOR_FIELD(DsDesc_t, DsDesc_memAddr_f, (uint32*)&phy_addr, p_desc);
    phy_addr = (phy_addr << 4);
    DRV_IOR_FIELD(DsDesc_t, DsDesc_chipAddr_f, (uint32*)&cfg_addr, p_desc);
    DRV_IOR_FIELD(DsDesc_t, DsDesc_cfgSize_f, (uint32*)&length, p_desc);
    DRV_IOR_FIELD(DsDesc_t, DsDesc_dataStruct_f, (uint32*)&word_num, p_desc);

    _chip_agent_dma_get_words_in_chip(word_num, &chip_word_num);
    if(chip_word_num == 0)
    {
        return 0;
    }

    entry_num = length/(chip_word_num*4);

    tb_id = (cfg_addr >> 16)&0x3fff;
    tb_index = cfg_addr&0x3fff;
    // gonghd dma >>>>
    // ¡Ÿ ±bufferƒ⁄¥Ê∑÷≈‰
    buf_tmp = (uint8*)sal_malloc(word_num*4);

    for (index = 0; index < entry_num; index++)
    {
    /*TBD, Get from cmodel through socket*/
    sal_memset(buf_tmp, 0x00, word_num*4);

    cm_sim_reg_memory_dump(lchip, tb_id, index, word_num*4, buf_tmp);
    sal_memcpy((void*)((uint8*)phy_addr+chip_word_num*4*index), buf_tmp, word_num*4);
    #if 0
        sal_memcpy((void*)((uint8*)phy_addr+chip_word_num*4*index), ((uint8*)tbl_addr+word_num*4*index), word_num*4);
    #endif
    }
    sal_free(buf_tmp);
    // gonghd dma <<<<
    valid_cnt--;
    cur_idx++;

    if (cur_idx >= ring_depth)
    {
        cur_idx = 0;
    }

    /* valid count descrease */
    DRV_IOW_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
    cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, chan_id, cmd, &ctl_tab));

    /* Cur index increase */
    DRV_IOW_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);
    cmd = DRV_IOW(DmaDynInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, chan_id, cmd, &dyn_info));
    /* set desc done */
    value = 1;
    DRV_IOW_FIELD(DsDesc_t, DsDesc_done_f, &value, p_desc);

    return 0;
}

STATIC int32
_chip_agent_dma_engine_chip_set_tx_pkt_desc(uint8 chip_id, uint32 chan, chip_agent_dma_desc_t* p_desc_base, uint32 depth,
    uint8* p_pkt, uint32 pkt_len)
{
    DsDesc_m* p_desc = NULL;
    uintptr phy_addr = 0;
    uint8* p_mem = NULL;
    uint8* p = NULL;
    uint32 left = 0;
    uint32 copy_len = 0;
    uint32 is_sop = TRUE;
    uint32 is_eop = 0;
    uint32 cmd = 0;
    DmaDynInfo_m dyn_info;
    DmaPktIntrEnCfg_m pkt_intr;
    DmaCtlTab_m ctl_tab;
    uint32 cur_idx = 0;
    uint32 valid_cnt = 0;
    uint32 intr_en = 0;
    uint32 intr_eop = 0;
    uint32 temp = 0;
    uint32 desc_len = 0;
    uint32 low_addr = 0;
    uint32 high_addr = 0;
    DmaStaticInfo_m static_info;

    p = p_pkt;
    left = pkt_len;

    sal_memset(&dyn_info, 0, sizeof(DmaDynInfo_m));
    sal_memset(&ctl_tab, 0, sizeof(DmaCtlTab_m));

    cmd = DRV_IOR(DmaDynInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));
    DRV_IOR_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);

    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &static_info));
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

    for (;;)
    {
        cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
        (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));
        DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);

        if (valid_cnt == 0)
        {
            break;
        }

        p_desc = &(p_desc_base[cur_idx].desc_info);
        if (is_sop)
        {
            temp =1 ;
            DRV_IOW_FIELD(DsDesc_t, DsDesc_u1_pkt_sop_f, &temp, p_desc);
            is_sop = FALSE;
        }

        valid_cnt--;
        cur_idx++;

        if (cur_idx >= depth)
        {
            cur_idx = 0;
        }

        DRV_IOR_FIELD(DsDesc_t, DsDesc_memAddr_f, (uint32*)&low_addr, p_desc);

        CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (low_addr << 4), phy_addr);

        DRV_IOR_FIELD(DsDesc_t, DsDesc_cfgSize_f, &desc_len, p_desc);

        p_mem = (uint8*)phy_addr;
        if (left <= desc_len)
        {
            /* using only one desc, update size and eop exist */
            copy_len = left;
            sal_memcpy(p_mem, p, copy_len);

            temp = 1;
            DRV_IOW_FIELD(DsDesc_t, DsDesc_realSize_f, &copy_len, p_desc);
            DRV_IOW_FIELD(DsDesc_t, DsDesc_u1_pkt_eop_f, &temp, p_desc);
            DRV_IOW_FIELD(DsDesc_t, DsDesc_done_f, &temp, p_desc);

            /* update cur ptr */
            DRV_IOW_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);
            cmd = DRV_IOW(DmaDynInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));

            /* update valid cnt */
            DRV_IOW_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
            cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));

            break;
        }
        else
        {
            /* using multi desc */
            DRV_IOR_FIELD(DsDesc_t, DsDesc_cfgSize_f, &copy_len, p_desc);
            sal_memcpy(p_mem, p, copy_len);

            temp = 1;
            DRV_IOW_FIELD(DsDesc_t, DsDesc_done_f, &temp, p_desc);
            DRV_IOW_FIELD(DsDesc_t, DsDesc_realSize_f, &copy_len, p_desc);

            /* update cur ptr */
            DRV_IOW_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);
            cmd = DRV_IOW(DmaDynInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));

            /* update valid cnt */
            DRV_IOW_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
            cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));

            p += copy_len;
            left -= copy_len;
        }

    }

    DRV_IOR_FIELD(DsDesc_t, DsDesc_u1_pkt_eop_f, &is_eop, p_desc);

    if (is_eop)
    {
        cmd = DRV_IOR(DmaPktIntrEnCfg_t, DRV_ENTRY_FLAG);
        (DRV_IOCTL(chip_id, 0, cmd, &pkt_intr));

        if (chan == 0)
        {
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx0DmaIntrEn_f, &intr_en, &pkt_intr);
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx0EopIntrEn_f, &intr_eop, &pkt_intr);
        }
        else if (chan == 1)
        {
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx1DmaIntrEn_f, &intr_en, &pkt_intr);
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx1EopIntrEn_f, &intr_eop, &pkt_intr);
        }
        else if (chan == 2)
        {
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx2DmaIntrEn_f, &intr_en, &pkt_intr);
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx2EopIntrEn_f, &intr_eop, &pkt_intr);
        }
        else if (chan == 3)
        {
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx3DmaIntrEn_f, &intr_en, &pkt_intr);
            DRV_IOR_FIELD(DmaPktIntrEnCfg_t, DmaPktIntrEnCfg_cfgPktRx3EopIntrEn_f, &intr_eop, &pkt_intr);
        }

        /* check EOP to trigger interrupt */
        if (intr_en)/*SDK Modify*/
        {
            /*TBD, should send to cmodel from socket */
            cm_com_interrupt_set(chip_id, 1, CM_INTR_DMA, chan);
        }
    }
    else
    {
        sal_printf("[DMA Engine TX] chip %d chan %d packet RX fail!!! \n", chip_id, chan);
    }

    return 0;
}

int32
chip_agent_dma_tx_pkt(chip_agent_pkt_t* p_pkt)
{
    chip_agent_dma_desc_t* p_desc_base = NULL;
    uint32 chan = 0;
    uint32 cmd = 0;
    uint8* p_pkt_data = NULL;
    uint32 pkt_len = 0;
    uint8 chip_id = 0;
    DmaCtlTab_m ctl_tab;
    DmaStaticInfo_m static_info;
    uint32 valid_cnt = 0;
    uint32 chan_en = 0;
    uint32 ring_depth = 0;
    uintptr ring_base = 0;
    uint32 high_addr = 0;
    uint32 tmp_base = 0;

    /*TBD*/
    /*Packet should get from cmodel from socket, channel id also need*/


    /* get DMA channel Dma channel 56, internal channel is 0*/
    chan = p_pkt->chan_id - DMA_TX_CHANID;
    chip_id = p_pkt->chip_id;

    if (chan >= 4)
    {
        return -1;
    }

    /* strip 20 Bytes CPUMAC header */
    p_pkt_data = p_pkt->pkt;
    pkt_len = p_pkt->packet_length;
    AGENT_DMA_DBG("[DMA Engine TX] chan %d length %d \n", chan, pkt_len);

    /* strip 4 bytes crc*/
    pkt_len -= 4;

    sal_memset(&ctl_tab, 0, sizeof(DmaCtlTab_m));

    /* check chan enable */
    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &static_info));
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_chanEn_f, &chan_en, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f, &ring_depth, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringBase_f, (uint32*)&tmp_base, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

    CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (tmp_base << 4), ring_base);

    if (chan_en == 0)
    {
        AGENT_DMA_DBG("chan %d is not enable \n", chan);
        return 0;
    }

    /* check desc valid cnt */
    cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));
    DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);

    if (valid_cnt == 0)
    {
        AGENT_DMA_DBG("chan %d is not enable \n", chan);

        /* trigger interrupt */
        return 0;
    }

    /* Get desc base, depth, valid num */
    p_desc_base = (chip_agent_dma_desc_t*)ring_base;

    AGENT_DMA_DBG("[DMA Engine TX] desc_base %p depth %d valid %d \n", ring_base,
        ring_depth, valid_cnt);

    _chip_agent_dma_engine_chip_set_tx_pkt_desc(p_pkt->chip_id, chan,
        p_desc_base, ring_depth, p_pkt_data, pkt_len);

    return 0;
}

#define __________CPU_2_DMA______________

STATIC int32
_chip_agent_dma_engine_chip_set_rx_pkt_desc(uint8 chip_id, uint32 chan, chip_agent_dma_desc_t* p_desc_base, uint32 depth)
{
    DsDesc_m* p_desc = NULL;
    uintptr phy_addr = 0;
    uint32 pkt_len = 0;
    uint8* packet = NULL;
    uint32 cmd = 0;
    DmaCtlTab_m ctl_tab;
    DmaDynInfo_m dyn_info;
    uint32 valid_cnt = 0;
    uint32 desc_len = 0;
    uint32 temp = 0;
    uint32 cur_idx = 0;
    uint32 low_addr = 0;
    uint32 high_addr = 0;
    DmaStaticInfo_m static_info;

    cmd = DRV_IOR(DmaDynInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));
    DRV_IOR_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);

    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &static_info));
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

    p_desc = &(p_desc_base[cur_idx].desc_info);
    DRV_IOR_FIELD(DsDesc_t, DsDesc_memAddr_f, (uint32*)&low_addr, p_desc);

    CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (low_addr << 4), phy_addr);

    valid_cnt--;
    cur_idx++;

    if (cur_idx >= depth)
    {
        cur_idx = 0;
    }

    DRV_IOR_FIELD(DsDesc_t, DsDesc_cfgSize_f, &desc_len, p_desc);

    if (desc_len)
    {
        /* can only handle one desc */
        packet = (uint8*)sal_malloc(AGENT_MTU);
        sal_memcpy(packet, (uint8*)phy_addr, desc_len);
        pkt_len = desc_len;

        /*TBD, Send packet to cmodel through socket*/
        //gonghd dma >>>>
        cm_sim_send_pkt2dma(chip_id, packet, pkt_len);
        //gonghd dma <<<<
        #if 0
        _swemu_received_packet_process(packet, pkt_len, chip_id, DMA_MACNUM);
        #endif

        /* update desc info */
        temp = 1;
        DRV_IOW_FIELD(DsDesc_t, DsDesc_done_f, &temp, p_desc);
        DRV_IOW_FIELD(DsDesc_t, DsDesc_realSize_f, &desc_len, p_desc);
    }

    /* update cur ptr */
    DRV_IOW_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);
    cmd = DRV_IOW(DmaDynInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));

    /* update valid cnt */
    cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));
    DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
    valid_cnt--;
    DRV_IOW_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
    cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));

    /* no need to trigger interrupt */

    return 0;
}

STATIC int32
_chip_agent_dma_engine_chip_process(uint8 chip_id)
{
    chip_agent_dma_desc_t* p_desc_base = NULL;
    uint32 chan = 0;
    uint32 cmd = 0;
    uintptr phy_addr = 0;
    DmaCtlTab_m ctl_tab;
    DmaStaticInfo_m static_info;
    uint32 valid_cnt = 0;
    uint32 chan_en = 0;
    uint32 ring_depth = 0;
    uint32 ring_base = 0;
    uint32 high_addr = 0;

    sal_memset(&ctl_tab, 0, sizeof(ctl_tab));
    sal_memset(&static_info, 0, sizeof(static_info));

    /* now only do packet tx simulation */
    for (chan = AGENT_DMA_PACKET_TX_CHAN; chan <= AGENT_DMA_PACKET_TX_MAX_CHAN; chan++)
    {
        /* get channel enable */
        cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
        (DRV_IOCTL(chip_id, chan, cmd, &static_info));
        DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_chanEn_f, &chan_en, &static_info);
        DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f, &ring_depth, &static_info);
        DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringBase_f, &ring_base, &static_info);
        DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

        CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (ring_base << 4), phy_addr);

        if (chan_en == 0)
        {
            continue;
        }

        /* get desc membase, desc depth, valid cnt */
        p_desc_base = (chip_agent_dma_desc_t*)phy_addr;

        cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
        (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));
        DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);

        /* process */
        if (valid_cnt > 0)
        {
            _chip_agent_dma_engine_chip_set_rx_pkt_desc(chip_id, chan, p_desc_base, ring_depth);
        }

    }

    return 0;
}


STATIC int32
_chip_agent_dma_info_engine_process(uint8 chip_id, uint8 chan)
{
    uint8 fifo_len = 0;
    uint8 proc_num = 0;
    uint8* p_info = NULL;
    uint32 cmd = 0;
    uint32 chan_en = 0;
    uintptr ring_base = 0;
    uintptr phy_addr = 0;
    uint32 ring_depth = 0;
    uint32 cur_idx = 0;
    uint32 real_size = 0;
    uint32 cfg_size = 0;
    uintptr data_addr = 0;
    uint8* p_mem = NULL;
    uint32 valid_cnt = 0;
    uint32 temp = 0;
    uint32 intr_en = 0;
    uint32 intr_mode = 0;
    uint32 high_addr = 0;
    DmaStaticInfo_m static_info;
    DmaCtlDrainEnable_m dma_drain;
    DmaDynInfo_m dyn_info;
    chip_agent_dma_desc_t* p_desc_base = NULL;
    DsDesc_m* p_desc = NULL;
    DmaCtlTab_m ctl_tab;
    DmaInfoIntrEnCfg_m info_intr;
    uint8 fifo_type = 0;
    uint32 total_cnt = 0;
    uint32 drain_en = 0;
    uint32 low_addr = 0;

    sal_memset(&ctl_tab, 0, sizeof(ctl_tab));
    sal_memset(&static_info, 0, sizeof(static_info));

    /* check chan enable */
    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, chan, cmd, &static_info));
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringBase_f, (uint32*)&ring_base, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_chanEn_f, &chan_en, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f, &ring_depth, &static_info);
    DRV_IOR_FIELD(DmaStaticInfo_t, DmaStaticInfo_highBase_f, &high_addr, &static_info);

    if (0 == chan_en)
    {
        return 0;
    }

    /* check drain enable */
    cmd = DRV_IOR(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(chip_id, 0, cmd, &dma_drain));
    if (chan <= 3)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaPktRxDrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if (chan == 4)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaPktTxDrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if(chan == 11)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaInfo0DrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if(chan == 12)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaInfo1DrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if(chan == 13)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaInfo2DrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if(chan == 14)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaInfo3DrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else if(chan == 15)
    {
        DRV_IOR_FIELD(DmaCtlDrainEnable_t, DmaCtlDrainEnable_dmaInfo4DrainEn_f, (uint32*)&drain_en, &dma_drain);
    }
    else
    {
         drain_en = 1;
    }

    if (!drain_en)
    {
        return 0;
    }

    CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (ring_base << 4), phy_addr);

    fifo_type = (chan == AGENT_DMA_LEARNING_CHAN)?CHIP_AGENT_FIFO_TYPE_LEARNING\
        :((chan == AGENT_DMA_HASH_DUMP_CHAN)?CHIP_AGENT_FIFO_TYPE_HASH_DUMP:CHIP_AGENT_FIFO_TYPE_IPFIX);

    do
    {
        /* get learning fifo length */
        fifo_len = chip_agent_fifo_get_length(chip_id, fifo_type);
        if (fifo_len == 0)
        {
            return 0;
        }

        AGENT_DMA_DBG("fifo is not empty:%d g_info_count:%d\n", fifo_len, p_dma_agent_master[chip_id]->agent_info_count[fifo_type]);

        /* check resource */
        cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
        (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));
        DRV_IOR_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
        if (0 == valid_cnt)
        {
            return 0;
        }

        if ((AGENT_DMA_INFO_THREASHOLD <= fifo_len) || p_dma_agent_master[chip_id]->agent_is_timeout[fifo_type])
        {
            AGENT_DMA_DBG("process fifo_len:%d\n", fifo_len);
            proc_num = (fifo_len >= AGENT_DMA_INFO_THREASHOLD)?AGENT_DMA_INFO_THREASHOLD:fifo_len;
            p_info = (uint8*)sal_malloc((p_dma_agent_master[chip_id]->agent_fifo[fifo_type].size)*proc_num);
            if (NULL == p_info)
            {
                p_dma_agent_master[chip_id]->agent_is_timeout[fifo_type] = 0;
                p_dma_agent_master[chip_id]->agent_info_count[fifo_type] = 0;
                return -1;
            }

            /* pop info from fifo */
            chip_agent_fifo_pop(chip_id, fifo_type, proc_num, (void*)p_info);

            /* get cur index */
            cmd = DRV_IOR(DmaDynInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));
            DRV_IOR_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);

            p_desc_base = (chip_agent_dma_desc_t*)phy_addr;
            p_desc = &(p_desc_base[cur_idx].desc_info);

            real_size = proc_num;

            DRV_IOR_FIELD(DsDesc_t, DsDesc_cfgSize_f, &cfg_size, p_desc);
            if (real_size > cfg_size)
            {
                /* just simulation one prrocess using one desc, sdk coding is also design so */
                sal_printf("[DMA Info] learning process warning!! data allocate is too small !! cfg:%d, real:%d\n", cfg_size, real_size);
                p_dma_agent_master[chip_id]->agent_is_timeout[fifo_type] = 0;
                p_dma_agent_master[chip_id]->agent_info_count[fifo_type] = 0;
                return 0;
            }

            AGENT_DMA_DBG("real_size:%d,cfg_size:%d, high_addr:0x%x\n", real_size, cfg_size, high_addr);

            DRV_IOW_FIELD(DsDesc_t, DsDesc_realSize_f, &real_size, p_desc);
            DRV_IOR_FIELD(DsDesc_t, DsDesc_memAddr_f, (uint32*)&low_addr, p_desc);

            CHIP_AGENT_COMBINE_64BITS_DATA(high_addr, (low_addr << 4), data_addr);

            p_mem = (uint8*)data_addr;
            if (chan == AGENT_DMA_LEARNING_CHAN)
            {
                sal_memcpy(p_mem, (uint8*)p_info, real_size*sizeof(chip_agent_fib_learn_t));
            }
            else
            {
                if (chan == AGENT_DMA_HASH_DUMP_CHAN)
                {
                    sal_memcpy(p_mem, (uint8*)p_info, real_size*sizeof(DmaFibDumpFifo_m));
                }
                else
                {
                    /* ipfix push dma fifo 16dwords at one time, pay attention here, may have problem*/
                    sal_memcpy(p_mem, (uint8*)p_info, real_size*sizeof(chip_agent_fib_ipfix_t));
                }
            }

            /* record total count entry in one desc */
            total_cnt += proc_num;
#if 0
            /* total count reach cfg size or timeout, end current desc, notify sdk */
            if (total_cnt >= cfg_size)
            {
            }
#endif
            temp = 1;
            DRV_IOW_FIELD(DsDesc_t, DsDesc_done_f, &temp, p_desc);

            /* update cur ptr */
            cur_idx++;
            if (cur_idx >= ring_depth)
            {
                cur_idx = 0;
            }

            DRV_IOW_FIELD(DmaDynInfo_t, DmaDynInfo_ringRdPtr_f, &cur_idx, &dyn_info);
            cmd = DRV_IOW(DmaDynInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &dyn_info));

            /* update valid cnt */
            valid_cnt--;
            DRV_IOW_FIELD(DmaCtlTab_t, DmaCtlTab_vldNum_f, &valid_cnt, &ctl_tab);
            cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, chan, cmd, &ctl_tab));

            /* trigger interrupt */
            cmd = DRV_IOR(DmaInfoIntrEnCfg_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(chip_id, 0, cmd, &info_intr));
            DRV_IOR_FIELD(DmaInfoIntrEnCfg_t, DmaInfoIntrEnCfg_cfgInfo0DmaIntrEn_f, &intr_en, &info_intr);
            DRV_IOR_FIELD(DmaInfoIntrEnCfg_t, DmaInfoIntrEnCfg_cfgInfo0DescIntrEn_f, &intr_mode, &info_intr);

           if (intr_en && intr_mode)
            {
                /* hashdump don't gen interrupt */
                if (fifo_type != 1)
                {
                    /*TBD, send to cmodel through socket*/
                    cm_com_interrupt_set(chip_id, 1, CM_INTR_DMA, chan);
                }
            }

           p_dma_agent_master[chip_id]->agent_is_timeout[fifo_type] = 0;

           sal_free(p_info);
           p_info = NULL;
        }
        else
        {
            if (p_dma_agent_master[chip_id]->agent_info_count[fifo_type] >= AGENT_DMA_INFO_TIMER_CNT)
            {
                p_dma_agent_master[chip_id]->agent_is_timeout[fifo_type] = 1;
                p_dma_agent_master[chip_id]->agent_info_count[fifo_type] = 0;
            }
            else
            {
                p_dma_agent_master[chip_id]->agent_info_count[fifo_type]++;
            }
        }
    }
    while(chip_agent_fifo_get_length(chip_id, fifo_type));

    return 0;
}

#define __________INTERFACE_________________
/**
 @brief sram table model set entry write bit
*/
int32
chip_agent_dma_add_fifo(uint8 chip_id, chip_agent_dma_info_t * dma_info)
{
    DmaFibLearnFifo_m fib_info;
    hw_mac_addr_t             mac_sa   = { 0 };

    /*TBD, dma_info should get from cmodel from socket*/

    sal_memset(&fib_info, 0, sizeof(DmaFibLearnFifo_m));

    if ((dma_info->dma_mode == CHIP_AGENT_DMA_MODE_LEARNING) || (dma_info->dma_mode == CHIP_AGENT_DMA_MODE_AGING))
    {
        mac_sa[1]= ((dma_info->u.learning.mac[5] << 8) | (dma_info->u.learning.mac[4]));
        mac_sa[0]= ((dma_info->u.learning.mac[3] << 24) | ((dma_info->u.learning.mac)[2] << 16) |
                    (dma_info->u.learning.mac[1] << 8) | (dma_info->u.learning.mac[0]));

        SetDmaFibLearnFifo(V,fastLearningEn_f, &fib_info, dma_info->u.learning.fast_learning_en);
        SetDmaFibLearnFifo(V,learningSourcePort_f, &fib_info, dma_info->u.learning.learning_src_port);
        SetDmaFibLearnFifo(V,isGlobalSrcPort_f, &fib_info, dma_info->u.learning.is_gport);
        SetDmaFibLearnFifo(V, vsiId_f, &fib_info, dma_info->u.learning.vsi);
        SetDmaFibLearnFifo(A, macSa_f, &fib_info, mac_sa);
        SetDmaFibLearnFifo(V,isLearning_f, &fib_info, dma_info->u.learning.is_add);

        if (dma_info->dma_mode == CHIP_AGENT_DMA_MODE_LEARNING)
        {
            SetDmaFibLearnFifo(V,uShare_gLearning_isEtherOam_f, &fib_info, dma_info->u.learning.is_eth_oam);
            SetDmaFibLearnFifo(V,uShare_gLearning_etherOamLevel_f, &fib_info, dma_info->u.learning.eth_oam_level);
            SetDmaFibLearnFifo(V,uShare_gLearning_newCvlanId_f, &fib_info, dma_info->u.learning.new_cvlan_id);
            SetDmaFibLearnFifo(V,uShare_gLearning_newSvlanId_f, &fib_info, dma_info->u.learning.new_svlan_id);
            SetDmaFibLearnFifo(V,uShare_gLearning_oldCvlanId_f, &fib_info, dma_info->u.learning.old_cvlan_id);
            SetDmaFibLearnFifo(V,uShare_gLearning_oldSvlanId_f, &fib_info, dma_info->u.learning.old_svlan_id);
        }
        else
        {
            SetDmaFibLearnFifo(V,uShare_gAging_hashType_f, &fib_info, dma_info->u.learning.hash_type);
            SetDmaFibLearnFifo(V,uShare_gAging_agingPtr_f, &fib_info, dma_info->u.learning.old_svlan_id);
            SetDmaFibLearnFifo(V,uShare_gAging_dsAdIndex_f, &fib_info, dma_info->u.learning.new_cvlan_id);
        }

        SetDmaFibLearnFifo(V,hashResultIndex_f, &fib_info, (dma_info->u.learning.key_index+32));

        _chip_agent_fifo_push(chip_id, CHIP_AGENT_FIFO_TYPE_LEARNING, &fib_info);
    }
    else if (dma_info->dma_mode == CHIP_AGENT_DMA_MODE_AGING)
    {
    }

    return DRV_E_EXCEED_MAX_SIZE;
}

int32
chip_agent_dma_is_not_full(uint8 chip_id)
{

    /*TBD, chip_id_offset should get from cmodel from socket, and the result should send to cmodel too*/

    if (chip_id >= 30)
    {
        return 0;
    }

    /* check fifo is full */
    if (chip_agent_fifo_get_length(chip_id, CHIP_AGENT_FIFO_TYPE_LEARNING) == 32)
    {
        sal_printf("[CM] Dma Fifo is full!\n");
        return 0;
    }
    else
    {
        return 1;
    }
}
int32
chip_agent_dma_fifo_is_not_full(uint8 chip_id, uint8 fifo_type)
{

    /*TBD, chip_id_offset should get from cmodel from socket, and the result should send to cmodel too*/

    if (chip_id >= 30)
    {
        return 0;
    }

    /* check fifo is full */
    if (chip_agent_fifo_get_length(chip_id, fifo_type) == 32)
    {
        sal_printf("[CM] Dma Fifo is full!\n");
        return 0;
    }
    else
    {
        return 1;
    }
}

void
chip_agent_ipfix_push_dma_info(uint8 lchip, void* p_key)
{
    /*TBD, p_key should get from cmodel from socket, and the result should send to cmodel too*/

    _chip_agent_fifo_push(lchip, CHIP_AGENT_FIFO_TYPE_IPFIX, (void*)p_key);

    return;
}

int32
chip_agent_fifo_push(uint8 lchip, uint8 fifo_type, void* p_info)
{

    /*TBD, fifo_type and  p_info should get from cmodel from socket, and the result should send to cmodel too*/

    _chip_agent_fifo_push(lchip, fifo_type, (void*)p_info);

    return 0;
}

#define __________INIT__________

/* dma engine for packet tx */
void
chip_agent_dma_pkt_engine(void* user_param)
{
    uint8 chip_id = (uintptr)user_param;

    for (;;)
    {
        _chip_agent_dma_engine_chip_process(chip_id);

        sal_task_sleep(50);
    }

    return;
}

/* dma engine for learning/aging */
void
chip_agent_dma_learn_aging_engine(void* user_param)
{
    uint8 chip_id = (uintptr)user_param;

    for (;;)
    {
        _chip_agent_dma_info_engine_process(chip_id, AGENT_DMA_LEARNING_CHAN);

        sal_task_sleep(50);
    }

    return;
}

/* dma engine for hash dump */
void
chip_agent_dma_hash_dump_engine(void* user_param)
{
    uint8 chip_id = (uintptr)user_param;

    for (;;)
    {
        _chip_agent_dma_info_engine_process(chip_id, AGENT_DMA_HASH_DUMP_CHAN);

        sal_task_sleep(50);
    }

    return;
}

/* dma engine for TB read  */
void
chip_agent_dma_reg_engine(void* user_param)
{
    uint8 chip_id = (uintptr)user_param;

    for (;;)
    {
        chip_agent_dma_reg_process(chip_id, AGENT_DMA_REG_READ_CHAN);

        sal_task_sleep(50);
    }

    return;
}

/* dma engine for ipfix */
void
chip_agent_dma_ipfix_engine(void* user_param)
{
    uint8 chip_id = (uintptr)user_param;

    for (;;)
    {
        _chip_agent_dma_info_engine_process(chip_id, AGENT_DMA_IPFIX_CHAN);

        sal_task_sleep(50);
    }

    return;
}

int32
chip_agent_dma_init(uint8 lchip)
{
    /* malloc master */
    p_dma_agent_master[lchip] = sal_malloc(sizeof(dma_agent_master_t));
    if (NULL == p_dma_agent_master[lchip])
    {
        return -1;
    }

    p_dma_agent_master[lchip]->agent_fifo[CHIP_AGENT_FIFO_TYPE_LEARNING].size = sizeof(chip_agent_fib_learn_t);
    p_dma_agent_master[lchip]->agent_fifo[CHIP_AGENT_FIFO_TYPE_HASH_DUMP].size = sizeof(DmaFibDumpFifo_m);
    p_dma_agent_master[lchip]->agent_fifo[CHIP_AGENT_FIFO_TYPE_IPFIX].size = sizeof(chip_agent_fib_ipfix_t);

    if (0 == p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_PKT_T])
    {
        (sal_task_create(&p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_PKT_T], "agent_dma_pkt", SAL_DEF_TASK_STACK_SIZE,
                                        SAL_TASK_PRIO_DEF, chip_agent_dma_pkt_engine, (void*)(uintptr)lchip));
    }

    if (0 == p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_HASH_DUMP_T])
    {
        (sal_task_create(&p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_HASH_DUMP_T], "agent_dma_dump", SAL_DEF_TASK_STACK_SIZE,
                                        SAL_TASK_PRIO_DEF,chip_agent_dma_hash_dump_engine, (void*)(uintptr)lchip));
    }
#if gonghd
    if (0 == p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_IPFIX_T])
    {
        (sal_task_create(&p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_IPFIX_T], "agent_dma_ipfix", SAL_DEF_TASK_STACK_SIZE,
                                        SAL_TASK_PRIO_DEF, chip_agent_dma_ipfix_engine, (void*)(uintptr)lchip));
    }

    if (0 == p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_REG_T])
    {
        (sal_task_create(&p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_REG_T], "agent_dma_reg", SAL_DEF_TASK_STACK_SIZE,
                                        SAL_TASK_PRIO_DEF, chip_agent_dma_reg_engine, (void*)(uintptr)lchip));
    }
#endif
    if (0 == p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_LEARN_T])
    {
        (sal_task_create(&p_dma_agent_master[lchip]->agent_thread[CHIP_AGENT_DMA_LEARN_T], "agent_learning_aging", SAL_DEF_TASK_STACK_SIZE,
                                        SAL_TASK_PRIO_DEF, chip_agent_dma_learn_aging_engine, (void*)(uintptr)lchip));
    }

    chip_agent_fifo_init(lchip, 32);

    return 0;
}
