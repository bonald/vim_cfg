/**
 @file dal_kernal.c

 @date 2012-10-18

 @version v2.0


*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/bug.h>
#ifdef SOC_ACTIVE
#include <linux/platform_device.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#include <linux/irqdomain.h>
#endif
#include "dal_kernel.h"
#include "dal_mpool.h"
#include "dal.h"
#include "sal.h"

MODULE_AUTHOR("Centec Networks Inc.");
MODULE_DESCRIPTION("DAL kernel module");
MODULE_LICENSE("GPL");

/* DMA memory pool size */
static char* dma_pool_size;
static dal_access_type_t g_dal_access = DAL_PCIE_MM;

module_param(dma_pool_size, charp, 0);
MODULE_PARM_DESC(dma_pool_size,
                 "Specify DMA memory pool size (default 4MB)");

dal_op_t g_dal_op =
{
    pci_read: dal_pci_read,
    pci_write: dal_pci_write,
    pci_conf_read: dal_pci_conf_read,
    pci_conf_write: dal_pci_conf_write,
    i2c_read: NULL,
    i2c_write: NULL,
    interrupt_register: dal_interrupt_register,
    interrupt_unregister: dal_interrupt_unregister,
    interrupt_set_en: dal_interrupt_set_en,
    interrupt_get_msi_info: dal_interrupt_get_msi_info,
    interrupt_set_msi_cap: dal_kernel_set_msi_enable,
    logic_to_phy: dal_logic_to_phy,
    phy_to_logic: dal_phy_to_logic,
    dma_alloc: dal_dma_alloc,
    dma_free: dal_dma_free,
    dma_cache_inval: dal_dma_cache_inval,
    dma_cache_flush: dal_dma_cache_flush,
    interrupt_get_irq: dal_interrupt_get_irq,
};

/*****************************************************************************
 * defines
 *****************************************************************************/
#define MB_SIZE 0x100000
#define CTC_MAX_INTR_NUM 8

#define MEM_MAP_RESERVE SetPageReserved
#define MEM_MAP_UNRESERVE ClearPageReserved

#define CTC_VENDOR_VID 0xc001
#define CTC_GREATBELT_DEVICE_ID 0x03e8
#define CTC_TSINGMA_DEVICE_ID  0x5236

#define MEM_MAP_RESERVE SetPageReserved
#define MEM_MAP_UNRESERVE ClearPageReserved

#define CTC_POLL_INTERRUPT_STR "poll_intr"
#define VIRT_TO_PAGE(p)     virt_to_page((p))
#define DAL_UNTAG_BLOCK         0
#define DAL_DISCARD_BLOCK      1
#define DAL_MATCHED_BLOCK     2
#define DAL_CUR_MATCH_BLOCk 3

#define M_INTR_HANDLER_IMPLEMENT(idx) STATIC irqreturn_t \
intr##idx##_handler(int irq, void* dev_id) \
{ \
    if(intr_trigger[idx]) \
    { \
        return IRQ_HANDLED; \
    } \
    disable_irq_nosync(irq); \
    intr_trigger[idx] = 1; \
    sal_sem_give(dal_isr[idx].p_inr_sem);  \
    return IRQ_HANDLED; \
}


/*****************************************************************************
 * typedef
 *****************************************************************************/
/* Control Data */
typedef struct dal_adapter {
    struct net_device *dev;
} dal_adapter_t;

typedef struct dal_isr_s
{
    int irq;
    void (* isr)(void*);
    void* isr_data;
    int trigger;
    wait_queue_head_t wqh;
    sal_sem_t*  p_inr_sem;
    sal_task_t*  p_intr_thread;
    int ref_cnt;
    int active;
} dal_isr_t;

struct dal_intr_para_s
{
    uint8 irq;
    int8 intr_idx;
    int16 prio;
};
typedef struct dal_intr_para_s dal_intr_para_t;

#ifdef SOC_ACTIVE
typedef struct dal_kernel_dev_s
{
    struct list_head list;
    struct platform_device* pci_dev;

    /* PCI I/O mapped base address */
    void __iomem * logic_address;

    /* Physical address */
    uintptr phys_address;
} dal_kern_dev_t;
#else
typedef struct dal_kernel_dev_s
{
    struct list_head list;
    struct pci_dev* pci_dev;

    /* PCI I/O mapped base address */
    uintptr logic_address;

    /* Physical address */
    uint64 phys_address;
} dal_kern_dev_t;
#endif

typedef struct _dma_segment
{
    struct list_head list;
    unsigned long req_size;     /* Requested DMA segment size */
    unsigned long blk_size;     /* DMA block size */
    unsigned long blk_order;    /* DMA block size in alternate format */
    unsigned long seg_size;     /* Current DMA segment size */
    unsigned long seg_begin;    /* Logical address of segment */
    unsigned long seg_end;      /* Logical end address of segment */
    unsigned long* blk_ptr;     /* Array of logical DMA block addresses */
    int blk_cnt_max;            /* Maximum number of block to allocate */
    int blk_cnt;                /* Current number of blocks allocated */
} dma_segment_t;

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/slab.h>
#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt
#endif
/***************************************************************************
 *declared
 ***************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/
static dal_kern_dev_t dal_dev[DAL_MAX_CHIP_NUM];
static dal_isr_t dal_isr[CTC_MAX_INTR_NUM];
#ifdef SOC_ACTIVE
static dal_isr_t dal_int[CTC_MAX_INTR_NUM] = {0};
#endif
static int dal_chip_num = 0;
static int dal_intr_num = 0;
static int use_high_memory = 0;
static dal_mpool_mem_t* dma_pool[DAL_MAX_CHIP_NUM];
static unsigned int* dma_virt_base[DAL_MAX_CHIP_NUM];
static uint64 dma_phy_base[DAL_MAX_CHIP_NUM];
static unsigned int dma_mem_size = 0xc00000;
static unsigned int msi_irq_base[DAL_MAX_CHIP_NUM] = {0};
static unsigned int msi_irq_num[DAL_MAX_CHIP_NUM] = {0};
static unsigned int msi_used = 0;
static uint8 dal_lchip[DAL_MAX_CHIP_NUM] = {0};

STATIC LIST_HEAD(_dma_seg);
static int dal_debug = 0;
module_param(dal_debug, int, 0);
MODULE_PARM_DESC(dal_debug, "Set debug level (default 0)");

#ifndef SOC_ACTIVE
static struct pci_device_id dal_id_table[] =
{
    {PCI_DEVICE(CTC_VENDOR_VID, CTC_GREATBELT_DEVICE_ID)},
	{PCI_DEVICE(0xcb10, 0xc010)},
    {PCI_DEVICE(0xcb11, 0xc011)},
    {PCI_DEVICE(0xcb10, 0x7148)},
    {PCI_DEVICE(0xcb10, 0x5236)},
    {0, },
};
#else
static const struct of_device_id linux_dal_of_match[] = {
    { .compatible = "centec,dal-localbus",},
    {},
};
MODULE_DEVICE_TABLE(of, linux_dal_of_match);
#endif

static int intr_trigger[CTC_MAX_INTR_NUM] = {0};
p_func intr_handler_fun[CTC_MAX_INTR_NUM];

M_INTR_HANDLER_IMPLEMENT(0);
M_INTR_HANDLER_IMPLEMENT(1);
M_INTR_HANDLER_IMPLEMENT(2);
M_INTR_HANDLER_IMPLEMENT(3);
M_INTR_HANDLER_IMPLEMENT(4);
M_INTR_HANDLER_IMPLEMENT(5);
M_INTR_HANDLER_IMPLEMENT(6);
M_INTR_HANDLER_IMPLEMENT(7);

/*****************************************************************************
 * macros
 *****************************************************************************/
#define VERIFY_CHIP_INDEX(n)  (n < dal_chip_num)

#define _KERNEL_INTERUPT_PROCESS

int32_t
dal_op_init(dal_op_t* p_dal_op)
{
    /*kernel mode */
    unsigned char lchip = 0;
    unsigned char index = 0;
    dal_pci_dev_t pci_dev;
    unsigned char dal_chip = 0;

    sal_memset(&pci_dev, 0, sizeof(dal_pci_dev_t));
    /* if dal op is provided by user , using user defined */
    if (p_dal_op)
    {
        lchip = p_dal_op->lchip;
        if (0 == lchip)
        {
            *((u_int32_t*)&g_dal_op) = *((u_int32_t*)p_dal_op);
        }
        sal_memcpy(&pci_dev, &(p_dal_op->pci_dev), sizeof(dal_pci_dev_t));
    }
#ifndef SOC_ACTIVE
    if ((0 != pci_dev.busNo) || (0 != pci_dev.devNo) || (0 != pci_dev.funNo))
    {
        for(index=0; index < DAL_MAX_CHIP_NUM; index++)
        {
            if (NULL == dal_dev[index].pci_dev)
            {
                continue;
            }
            if ((pci_dev.busNo == dal_dev[index].pci_dev->bus->number) && (pci_dev.devNo == dal_dev[index].pci_dev->device) && (pci_dev.funNo == dal_dev[index].pci_dev->devfn))
            {
                dal_lchip[lchip] = index;
                break;
            }
        }
        if (index >= DAL_MAX_CHIP_NUM)
        {
            printk("not find the lchip %d device bus:0x%x dev:0x%x fun:0x%x\n", lchip, pci_dev.busNo, pci_dev.devNo, pci_dev.funNo);
            return -1;
        }
    }
    else
#endif
    {
        dal_lchip[lchip] = lchip;
    }
#ifndef SOC_ACTIVE
    g_dal_op.soc_active = 0;
#else
    g_dal_op.soc_active = 1;
#endif

    dal_mpool_init(lchip);

    dal_chip = dal_lchip[lchip];
    dma_pool[lchip ] = dal_mpool_create(lchip, dma_virt_base[dal_chip], dma_mem_size);
    if (!dma_pool[lchip ])
    {
        printk("Create mpool fail, dma_pool:%p \n", dma_pool[lchip]);
        return -1;
    }
    return DAL_E_NONE;
}

int32
dal_op_deinit(uint8 lchip)
{
    /*mpool destroy*/
    dal_mpool_destroy(lchip, dma_pool[lchip]);

    return 0;
}


/* interrupt thread */
STATIC void
dal_intr_thread(void* param)
{
    unsigned int para = (uintptr)param;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int32 ret = 0;
    int32 intr_idx = 0;
    int32 irq = 0;
    dal_isr_t* intr_handler = NULL;
    intr_idx = p_para->intr_idx;
    intr_handler = (dal_isr_t*)&(dal_isr[intr_idx]);
    irq = p_para->irq;

    /*for many chips in one board, there interrupt line should merge together */
    while (1)
    {
        ret = sal_sem_take(dal_isr[intr_idx].p_inr_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        if(dal_isr[intr_idx].active == 0)
        {
            return;
        }
        if (intr_trigger[intr_idx])
        {
            intr_trigger[intr_idx] = 0;
        }
        intr_handler->isr((void*)(&irq));
    }
}

int
dal_interrupt_register(unsigned int irq, int prio, void (* isr)(void*), void* data)
{
    int ret;
    unsigned char* int_name = NULL;
    uintptr para = 0;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int intr_idx = CTC_MAX_INTR_NUM;
    int intr_idx_tmp = 0;
    unsigned long irq_flags = 0;

    if (dal_intr_num >= CTC_MAX_INTR_NUM)
    {
        printk("Interrupt numbers exceeds max.\n");
        return -1;
    }

    for (intr_idx_tmp=0;intr_idx_tmp < CTC_MAX_INTR_NUM; intr_idx_tmp++)
    {
        if (irq == dal_isr[intr_idx_tmp].irq)
        {
           dal_isr[intr_idx_tmp].ref_cnt++;
           return 0;
        }
        if ((0 == dal_isr[intr_idx_tmp].irq) && (CTC_MAX_INTR_NUM == intr_idx))
        {
            intr_idx = intr_idx_tmp;
            dal_isr[intr_idx].ref_cnt = 0;
        }
    }

    if (msi_used)
    {
        /* for msi interrupt, irq means msi irq index */
        int_name = "dal_msi";
    }
    else
    {
        int_name = "dal_intr";
    }

    p_para->intr_idx = (int8)intr_idx;
    p_para->irq = (uint8)irq;
    p_para->prio = (int16)prio;

    dal_isr[intr_idx].irq = irq;
    dal_isr[intr_idx].isr = isr;
    dal_isr[intr_idx].isr_data = data;
    dal_isr[intr_idx].active = 1;
    ret = sal_sem_create(&dal_isr[intr_idx].p_inr_sem, 0);
    if (ret < 0)
    {
        return ret;
    }

    sal_task_create(&dal_isr[intr_idx].p_intr_thread,
                    int_name,
                    SAL_DEF_TASK_STACK_SIZE,
                    prio,
                    dal_intr_thread,
                    (void*)para);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    irq_flags = 0;
#else
    irq_flags = IRQF_DISABLED;
#endif
    if ((ret = request_irq(irq,
                           intr_handler_fun[intr_idx],
                           irq_flags,
                           int_name,
                           &dal_isr[intr_idx])) < 0)
    {
        printk("Cannot request irq %d, ret %d.\n", irq, ret);
    }

    if (0 == ret)
    {
        dal_intr_num++;
    }

    return ret;
}

int
dal_interrupt_unregister(unsigned int irq)
{
    int intr_idx = 0;
    int find_flag = 0;

    /* get intr device index */
    for (intr_idx = 0; intr_idx < CTC_MAX_INTR_NUM; intr_idx++)
    {
        if (dal_isr[intr_idx].irq == irq)
        {
            find_flag = 1;
            break;
        }
    }

    if (find_flag == 0)
    {
        printk ("<0>irq%d is not registered! unregister failed \n", irq);
        return -1;
    }

    if(dal_isr[intr_idx].ref_cnt > 1)
    {
        dal_isr[intr_idx].ref_cnt --;
        return 0;
    }

    free_irq(irq, &dal_isr[intr_idx]);
    dal_isr[intr_idx].active = 0;
    sal_sem_give(dal_isr[intr_idx].p_inr_sem);
    sal_task_destroy(dal_isr[intr_idx].p_intr_thread);
    sal_memset(&(dal_isr[intr_idx]), 0, sizeof(dal_isr_t));
    dal_intr_num--;

    return 0;
}

int
dal_interrupt_set_en(unsigned int irq, unsigned int enable)
{
    enable ? enable_irq(irq) : disable_irq_nosync(irq);

    return 0;
}

STATIC int
_dal_set_msi_enabe(int lchip, unsigned int msi_num)
{
    int ret = 0;

#ifndef SOC_ACTIVE
    if (msi_num == 1)
    {
        ret = pci_enable_msi(dal_dev[lchip].pci_dev);
        if (ret)
        {
            printk ("msi enable failed!!! lchip = %d, irq_num = %d\n", lchip, msi_num);
            pci_disable_msi(dal_dev[lchip].pci_dev);
            msi_used = 0;
        }

        msi_irq_base[lchip] = dal_dev[lchip].pci_dev->irq;
        msi_irq_num[lchip] = 1;
    }
    else
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 79))
        ret = pci_enable_msi_exact(dal_dev[lchip].pci_dev, msi_num);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 26, 32))
        ret = pci_enable_msi_block(dal_dev[lchip].pci_dev, msi_num);
#else
        ret = -1;
#endif
        if (ret)
        {
            printk ("msi enable failed!!! lchip = %d, irq_num = %d\n", lchip, msi_num);
            pci_disable_msi(dal_dev[lchip].pci_dev);
            msi_used = 0;
        }

        msi_irq_base[lchip] = dal_dev[lchip].pci_dev->irq;
        msi_irq_num[lchip] = msi_num;
    }
#endif

    return ret;
}

STATIC int
_dal_set_msi_disable(unsigned int lchip)
{
#ifndef SOC_ACTIVE
    pci_disable_msi(dal_dev[lchip].pci_dev);

    msi_irq_base[lchip] = 0;
    msi_irq_num[lchip] = 0;
#endif
    return 0;
}

/*enable parameter is used in kernel mode, for user mode useless */
int
dal_kernel_set_msi_enable(unsigned char lchip, unsigned int enable, unsigned int msi_num)
{
    int ret = 0;
#ifndef SOC_ACTIVE
    unsigned int used_num = 0;

    used_num = (enable)?msi_num:0;

    if (used_num > 0)
    {
        msi_used = 1;
        ret = _dal_set_msi_enabe(lchip, used_num);
    }
    else
    {
        used_num = 0;
        ret = _dal_set_msi_disable(lchip);
    }
#endif

    return ret;
}

int
dal_set_msi_cap(unsigned int lchip, unsigned long msi_num)
{
    int ret = 0;

    ret =  dal_kernel_set_msi_enable(lchip, 1, msi_num);

    return ret;
}
int dal_interrupt_get_msi_info(unsigned char lchip, unsigned char* irq_base)
{
    *irq_base = msi_irq_base[lchip];
    return 0;
}

int dal_interrupt_get_irq(unsigned char lchip, unsigned char type , unsigned short* irq_array, unsigned char* num)
{
#ifndef SOC_ACTIVE
    if(1 == type)
    {
        unsigned char loop;

        for(loop=0;loop < msi_irq_num[lchip]; loop++)
        {
            *(irq_array+loop) = msi_irq_base[lchip]+loop;
        }
        *num = msi_irq_num[lchip];
    }
    else if(2 == type)
    {
        unsigned int conf_value = 0;
        dal_pci_conf_read(lchip, 0x3c, &conf_value);
        *irq_array = conf_value&0xFF;
        *num = 1;
    }
    else
    {
        return -1;
    }
#else
    unsigned char loop;
    for(loop=0;loop < CTC_MAX_INTR_NUM; loop++)
    {
        *(irq_array+loop) = dal_int[loop].irq;
    }
    *num = CTC_MAX_INTR_NUM;
#endif

    return 0;
}
#define _KERNEL_DMA_PROCESS

uint64
dal_logic_to_phy(unsigned char lchip, void* laddr)
{
    lchip = dal_lchip[lchip];

    if (dma_mem_size)
    {
        /* dma memory is a contiguous block */
        if (laddr)
        {
            return dma_phy_base[lchip] + ((uintptr)(laddr) - (uintptr)(dma_virt_base[lchip]));
        }

        return 0;
    }

    return virt_to_bus(laddr);
}

void*
dal_phy_to_logic(unsigned char lchip, uint64 paddr)
{
    lchip = dal_lchip[lchip];
    if (dma_mem_size)
    {
        /* dma memory is a contiguous block */
        return (void*)(paddr ? (char*)dma_virt_base[lchip] + (paddr - dma_phy_base[lchip]) : NULL);
    }

    return bus_to_virt(paddr);
}

unsigned int*
dal_dma_alloc(unsigned char lchip, int size, int type)
{
    return dal_mpool_alloc(lchip, dma_pool[lchip], size, type);
}

void
dal_dma_free(unsigned char lchip, void* ptr)
{
    return dal_mpool_free(lchip, dma_pool[lchip], ptr);
}

#ifndef DMA_MEM_MODE_PLATFORM
/*
 * Function: _dal_dma_segment_free
 */

/*
 * Function: _find_largest_segment
 *
 * Purpose:
 *    Find largest contiguous segment from a pool of DMA blocks.
 * Parameters:
 *    dseg - DMA segment descriptor
 * Returns:
 *    0 on success, < 0 on error.
 * Notes:
 *    Assembly stops if a segment of the requested segment size
 *    has been obtained.
 *
 *    Lower address bits of the DMA blocks are used as follows:
 *       0: Untagged
 *       1: Discarded block
 *       2: Part of largest contiguous segment
 *       3: Part of current contiguous segment
 */
STATIC int
_dal_find_largest_segment(dma_segment_t* dseg)
{
    int i, j, blks, found;
    unsigned long seg_begin;
    unsigned long seg_end;
    unsigned long seg_tmp;

    blks = dseg->blk_cnt;

    /* Clear all block tags */
    for (i = 0; i < blks; i++)
    {
        dseg->blk_ptr[i] &= ~3;
    }

    for (i = 0; i < blks && dseg->seg_size < dseg->req_size; i++)
    {
        /* First block must be an untagged block */
        if ((dseg->blk_ptr[i] & 3) == DAL_UNTAG_BLOCK)
        {
            /* Initial segment size is the block size */
            seg_begin = dseg->blk_ptr[i];
            seg_end = seg_begin + dseg->blk_size;
            dseg->blk_ptr[i] |= DAL_CUR_MATCH_BLOCk;

            /* Loop looking for adjacent blocks */
            do
            {
                found = 0;

                for (j = i + 1; j < blks && (seg_end - seg_begin) < dseg->req_size; j++)
                {
                    seg_tmp = dseg->blk_ptr[j];
                    /* Check untagged blocks only */
                    if ((seg_tmp & 3) == DAL_UNTAG_BLOCK)
                    {
                        if (seg_tmp == (seg_begin - dseg->blk_size))
                        {
                            /* Found adjacent block below current segment */
                            dseg->blk_ptr[j] |= DAL_CUR_MATCH_BLOCk;
                            seg_begin = seg_tmp;
                            found = 1;
                        }
                        else if (seg_tmp == seg_end)
                        {
                            /* Found adjacent block above current segment */
                            dseg->blk_ptr[j] |= DAL_CUR_MATCH_BLOCk;
                            seg_end += dseg->blk_size;
                            found = 1;
                        }
                    }
                }
            }
            while (found);

            if ((seg_end - seg_begin) > dseg->seg_size)
            {
                /* The current block is largest so far */
                dseg->seg_begin = seg_begin;
                dseg->seg_end = seg_end;
                dseg->seg_size = seg_end - seg_begin;

                /* Re-tag current and previous largest segment */
                for (j = 0; j < blks; j++)
                {
                    if ((dseg->blk_ptr[j] & 3) == DAL_CUR_MATCH_BLOCk)
                    {
                        /* Tag current segment as the largest */
                        dseg->blk_ptr[j] &= ~1;
                    }
                    else if ((dseg->blk_ptr[j] & 3) == DAL_MATCHED_BLOCK)
                    {
                        /* Discard previous largest segment */
                        dseg->blk_ptr[j] ^= 3;
                    }
                }
            }
            else
            {
                /* Discard all blocks in current segment */
                for (j = 0; j < blks; j++)
                {
                    if ((dseg->blk_ptr[j] & 3) == DAL_CUR_MATCH_BLOCk)
                    {
                        dseg->blk_ptr[j] &= ~2;
                    }
                }
            }
        }
    }

    return 0;
}

/*
 * Function: _alloc_dma_blocks
 */
STATIC int
_dal_alloc_dma_blocks(dma_segment_t* dseg, int blks)
{
    int i, start;
    unsigned long addr;

    if (dseg->blk_cnt + blks > dseg->blk_cnt_max)
    {
        printk("No more DMA blocks\n");
        return -1;
    }

    start = dseg->blk_cnt;
    dseg->blk_cnt += blks;

    for (i = start; i < dseg->blk_cnt; i++)
    {
        addr = __get_free_pages(GFP_ATOMIC, dseg->blk_order);
        if (addr)
        {
            dseg->blk_ptr[i] = addr;
        }
        else
        {
            printk("DMA allocation failed\n");
            return -1;
        }
    }

    return 0;
}

/*
 * Function: _dal_dma_segment_alloc
 */
STATIC dma_segment_t*
_dal_dma_segment_alloc(unsigned int size, unsigned int blk_size)
{
    dma_segment_t* dseg;
    int i, blk_ptr_size;
    uintptr page_addr;
    struct sysinfo si;

    /* Sanity check */
    if (size == 0 || blk_size == 0)
    {
        return NULL;
    }

    /* Allocate an initialize DMA segment descriptor */
    if ((dseg = kmalloc(sizeof(dma_segment_t), GFP_ATOMIC)) == NULL)
    {
        return NULL;
    }

    memset(dseg, 0, sizeof(dma_segment_t));
    dseg->req_size = size;
    dseg->blk_size = PAGE_ALIGN(blk_size);

    while ((PAGE_SIZE << dseg->blk_order) < dseg->blk_size)
    {
        dseg->blk_order++;
    }

    si_meminfo(&si);
    dseg->blk_cnt_max = (si.totalram << PAGE_SHIFT) / dseg->blk_size;
    blk_ptr_size = dseg->blk_cnt_max * sizeof(unsigned long);
    /* Allocate an initialize DMA block pool */
    dseg->blk_ptr = kmalloc(blk_ptr_size, GFP_KERNEL);
    if (dseg->blk_ptr == NULL)
    {
        kfree(dseg);
        return NULL;
    }

    memset(dseg->blk_ptr, 0, blk_ptr_size);
    /* Allocate minimum number of blocks */
    _dal_alloc_dma_blocks(dseg, dseg->req_size / dseg->blk_size);

    /* Allocate more blocks until we have a complete segment */
    do
    {
        _dal_find_largest_segment(dseg);
        if (dseg->seg_size >= dseg->req_size)
        {
            break;
        }
    }
    while (_dal_alloc_dma_blocks(dseg, 8) == 0);

    /* Reserve all pages in the DMA segment and free unused blocks */
    for (i = 0; i < dseg->blk_cnt; i++)
    {
        if ((dseg->blk_ptr[i] & 3) == 2)
        {
            dseg->blk_ptr[i] &= ~3;

            for (page_addr = dseg->blk_ptr[i];
                 page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                 page_addr += PAGE_SIZE)
            {
                MEM_MAP_RESERVE(VIRT_TO_PAGE((void*)page_addr));
            }
        }
        else if (dseg->blk_ptr[i])
        {
            dseg->blk_ptr[i] &= ~3;
            free_pages(dseg->blk_ptr[i], dseg->blk_order);
            dseg->blk_ptr[i] = 0;
        }
    }

    return dseg;
}

/*
 * Function: _dal_dma_segment_free
 */
STATIC void
_dal_dma_segment_free(dma_segment_t* dseg)
{
    int i;
    uintptr page_addr;

    if (dseg->blk_ptr)
    {
        for (i = 0; i < dseg->blk_cnt; i++)
        {
            if (dseg->blk_ptr[i])
            {
                for (page_addr = dseg->blk_ptr[i];
                     page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                     page_addr += PAGE_SIZE)
                {
                    MEM_MAP_UNRESERVE(VIRT_TO_PAGE((void*)page_addr));
                }

                free_pages(dseg->blk_ptr[i], dseg->blk_order);
            }
        }

        kfree(dseg->blk_ptr);
        kfree(dseg);
    }
}

/*
 * Function: -dal_pgalloc
 */
STATIC void*
_dal_pgalloc(unsigned int size)
{
    dma_segment_t* dseg;
    unsigned int blk_size;

    blk_size = (size < DMA_BLOCK_SIZE) ? size : DMA_BLOCK_SIZE;
    if ((dseg = _dal_dma_segment_alloc(size, blk_size)) == NULL)
    {
        return NULL;
    }

    if (dseg->seg_size < size)
    {
        /* If we didn't get the full size then forget it */
        printk("Notice: Can not get enough memory for requset!!\n");
        printk("actual size:0x%lx, request size:0x%x\n", dseg->seg_size, size);
         /*-_dal_dma_segment_free(dseg);*/
         /*-return NULL;*/
    }

    list_add(&dseg->list, &_dma_seg);
    return (void*)dseg->seg_begin;
}

/*
 * Function: _dal_pgfree
 */
STATIC int
_dal_pgfree(void* ptr)
{
    struct list_head* pos;

    list_for_each(pos, &_dma_seg)
    {
        dma_segment_t* dseg = list_entry(pos, dma_segment_t, list);

        if (ptr == (void*)dseg->seg_begin)
        {
            list_del(&dseg->list);
            _dal_dma_segment_free(dseg);
            return 0;
        }
    }
    return -1;
}
#endif

/*lchip in this function, means dal chip id, which mapping to sdk lchip, but not eq */
STATIC void
dal_alloc_dma_pool(unsigned char dal_chip, int size)
{
    if (use_high_memory)
    {
        dma_phy_base[dal_chip] = virt_to_bus(high_memory);
        dma_virt_base[dal_chip] = ioremap_nocache(dma_phy_base[dal_chip], size);
    }
    else
    {
#if defined(DMA_MEM_MODE_PLATFORM) || defined(SOC_ACTIVE)
    dma_virt_base[dal_chip] = dma_alloc_coherent(&(dal_dev[dal_chip].pci_dev->dev), dma_mem_size,
                                       &dma_phy_base[dal_chip], GFP_KERNEL);
    printk(KERN_WARNING "########Using DMA_MEM_MODE_PLATFORM \n");
    printk(KERN_WARNING "########Dma alloc physical address:0x%"PRIx64" dma_mem_size:%d, logic_addr:%p\n", dma_phy_base[dal_chip], dma_mem_size, dma_virt_base[dal_chip]);
#else
    /* Get DMA memory from kernel */
    dma_virt_base[dal_chip] = _dal_pgalloc(size);
    printk("<0>_dal_pgalloc return %p\r\n", dma_virt_base[dal_chip]);
    dma_phy_base[dal_chip] = virt_to_bus(dma_virt_base[dal_chip]);
    printk("<0>Dma physical address 0x%"PRIx64"\n", dma_phy_base[dal_chip]);
    printk("<0>Using SDK malloc Dma memory pool!!\n");
    printk(KERN_WARNING "########Dma alloc physical address:0x%"PRIx64" dma_mem_size:%d, logic_addr:%p\n", dma_phy_base[dal_chip], dma_mem_size, dma_virt_base[dal_chip]);
#endif
         /*dma_virt_base = ioremap_nocache(dma_phy_base, size);*/
         /*printk("after ioremap_nocache return %p\r\n", dma_virt_base);*/
    }
#ifdef SOC_ACTIVE
        dma_phy_base[dal_chip] -= 0x80000000;
#endif
}

STATIC void
dal_free_dma_pool(unsigned char dal_chip)
{
    int ret = 0;
    ret = ret;

#ifdef SOC_ACTIVE
     dma_phy_base[dal_chip] += 0x80000000;
#endif
    if (use_high_memory)
    {
        iounmap(dma_virt_base[dal_chip]);
    }
    else
    {
#if defined(DMA_MEM_MODE_PLATFORM) || defined(SOC_ACTIVE)
    dma_free_coherent(&(dal_dev[dal_chip].pci_dev->dev), dma_mem_size,
                                                  dma_virt_base[dal_chip], dma_phy_base[dal_chip]);
#else
    ret = _dal_pgfree(dma_virt_base[dal_chip]);
    if(ret<0)
    {
        printk("Dma free memory fail !!!!!! \n");
    }
#endif
    }
}
#define _KERNEL_DAL_IO
int
dal_pci_read(unsigned char lchip, unsigned int offset, unsigned int* value)
{
    if (!VERIFY_CHIP_INDEX(lchip))
    {
        WARN_ON(1);
        return -1;
    }

    lchip = dal_lchip[lchip];

    *value = *(volatile unsigned int*)(dal_dev[lchip].logic_address + offset);
    return 0;
}

int
dal_pci_write(unsigned char lchip, unsigned int offset, unsigned int value)
{
    if (!VERIFY_CHIP_INDEX(lchip))
    {
        WARN_ON(1);
        return -1;
    }

    lchip = dal_lchip[lchip];

    *(volatile unsigned int*)(dal_dev[lchip].logic_address + offset) = value;
    return 0;
}

int
dal_pci_conf_read(unsigned char lchip, unsigned int offset, unsigned int* value)
{
    if (!VERIFY_CHIP_INDEX(lchip))
    {
        return -1;
    }

#ifndef SOC_ACTIVE
    lchip = dal_lchip[lchip];

    pci_read_config_dword(dal_dev[lchip].pci_dev, offset, value);
#endif

    return 0;
}

int
dal_pci_conf_write(unsigned char lchip, unsigned int offset, unsigned int value)
{
    if (!VERIFY_CHIP_INDEX(lchip))
    {
        return -1;
    }

#ifndef SOC_ACTIVE
    lchip = dal_lchip[lchip];

    pci_write_config_dword(dal_dev[lchip].pci_dev, offset, value);
#endif

    return 0;
}

int
dal_user_read_pci_conf(unsigned long arg)
{
    dal_pci_cfg_ioctl_t dal_cfg;

    if (copy_from_user(&dal_cfg, (void*)arg, sizeof(dal_pci_cfg_ioctl_t)))
    {
        return -EFAULT;
    }

    if (dal_pci_conf_read(dal_cfg.lchip, dal_cfg.offset, &dal_cfg.value))
    {
        printk("dal_pci_conf_read failed.\n");
        return -EFAULT;
    }

    if (copy_to_user((dal_pci_cfg_ioctl_t*)arg, (void*)&dal_cfg, sizeof(dal_pci_cfg_ioctl_t)))
    {
        return -EFAULT;
    }

    return 0;
}

int
linux_dal_mmap0(struct file* flip, struct vm_area_struct* vma)
{
    size_t size = vma->vm_end - vma->vm_start;
    unsigned long pfn;

    printk("linux_dal0_mmap begin.\n");

    pfn = (dal_dev[0].phys_address) >> PAGE_SHIFT;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot))
    {
        return -EAGAIN;
    }

    printk("linux_dal_mmap0 finish.\n");

    return 0;
}

int
dal_user_write_pci_conf(unsigned long arg)
{
    dal_pci_cfg_ioctl_t dal_cfg;

    if (copy_from_user(&dal_cfg, (void*)arg, sizeof(dal_pci_cfg_ioctl_t)))
    {
        return -EFAULT;
    }

    return dal_pci_conf_write(dal_cfg.lchip, dal_cfg.offset, dal_cfg.value);
}

int32
dal_dma_cache_inval(uint8 lchip, uint64 ptr, int length)
{
#ifndef DMA_MEM_MODE_PLATFORM
#ifdef DMA_CACHE_COHERENCE_EN
    /*dma_cache_wback_inv((unsigned long)ptr, length);*/

    dma_sync_single_for_cpu(NULL, ptr, length, DMA_FROM_DEVICE);

    /*dma_cache_sync(NULL, (void*)bus_to_virt(ptr), length, DMA_FROM_DEVICE);*/
#endif
#endif
    return 0;
}

int32
dal_dma_cache_flush(uint8 lchip, uint64 ptr, int length)
{
#ifndef DMA_MEM_MODE_PLATFORM
#ifdef DMA_CACHE_COHERENCE_EN
    /*dma_cache_wback_inv(ptr, length);*/

    dma_sync_single_for_device(NULL, ptr, length, DMA_TO_DEVICE);

    /*dma_cache_sync(NULL, (void*)bus_to_virt(ptr), length, DMA_TO_DEVICE);*/
#endif
#endif
    return 0;
}

#ifndef SOC_ACTIVE
int linux_dal_probe(struct pci_dev* pdev, const struct pci_device_id* id)
#else
static int linux_dal_probe(struct platform_device *pdev)
#endif
{
    dal_kern_dev_t* dev = NULL;
    unsigned int temp = 0;
    unsigned char lchip = 0;
#ifndef SOC_ACTIVE
    int bar = 0;
    int ret = 0;
#else
    int i = 0;
    int irq = 0;
    struct resource * res = NULL;
#endif

    printk(KERN_WARNING "********found dal device deviceid:%d*****\n", dal_chip_num);


    for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip ++)
    {
        if (NULL == dal_dev[lchip].pci_dev)
        {
            break;
        }
    }

    if (lchip >= DAL_MAX_CHIP_NUM)
    {
        printk("Exceed max local chip num\n");
        return -1;
    }

    dev = &dal_dev[lchip];
    if (NULL == dev)
    {
        printk("Cannot obtain PCI resources\n");
    }

    lchip = lchip;
    dal_chip_num++;

    dev->pci_dev = pdev;

#ifndef SOC_ACTIVE
    if (pci_enable_device(pdev) < 0)
    {
        printk("Cannot enable PCI device: vendor id = %x, device id = %x\n",
               pdev->vendor, pdev->device);
    }

    ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
    if (ret)
    {
        ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
        if (ret) {
        printk("Could not set PCI DMA Mask\n");
        return ret;
        }
    }

    if (pci_request_regions(pdev, DAL_NAME) < 0)
    {
        printk("Cannot obtain PCI resources\n");
    }

    if (pdev->device == 0x5236)
    {
        printk("use bar2 to config memory space\n");
        bar = 2;
    }

    dev->phys_address = pci_resource_start(pdev, bar);
    dev->logic_address = (uintptr)ioremap_nocache(dev->phys_address,
                                                pci_resource_len(dev->pci_dev, bar));
#else
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    dev->phys_address = res->start;
    dev->logic_address = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(dev->logic_address))
    {
        return PTR_ERR(dev->logic_address);
    }

    for (i = 0; i < CTC_MAX_INTR_NUM; i++)
    {
        irq = platform_get_irq(pdev, i);
        if (irq < 0)
        {
            printk( "can't get irq number\n");
            return irq;
        }
        dal_int[i].irq = irq;
        printk( "irq %d vector %d\n", i, irq);
    }
#endif
    dal_pci_read(lchip, 0x48, &temp);
    if (((temp >> 8) & 0xffff) == 0x3412)
    {
        printk("Little endian Cpu detected!!! \n");
        dal_pci_write(lchip, 0x48, 0xFFFFFFFF);
    }

    dal_pci_read(lchip, 0x48, &temp);

#ifndef SOC_ACTIVE
    pci_set_master(pdev);
#endif

    if (dma_mem_size)
    {
        dal_alloc_dma_pool(lchip , dma_mem_size);
        /*add check Dma memory pool cannot cross 4G space*/
        if ((0==(dma_phy_base[lchip]>>32)) && (0!=((dma_phy_base[lchip]+dma_mem_size)>>32)))
        {
            printk("Dma malloc memory cross 4G space!!!!!! \n");
            return -1;
        }
    }

    printk(KERN_WARNING "linux_dal_probe end \n");

    return 0;
}


STATIC int
linux_dal_get_device(unsigned long arg)
{
    dal_user_dev_t user_dev;
    int lchip = 0;

    if (copy_from_user(&user_dev, (void*)arg, sizeof(user_dev)))
    {
        return -EFAULT;
    }

    user_dev.chip_num = dal_chip_num;
    lchip = user_dev.lchip;

    if (lchip < dal_chip_num)
    {
        user_dev.phy_base0 = (unsigned int)dal_dev[lchip].phys_address;
        user_dev.phy_base1 = (unsigned int)(dal_dev[lchip].phys_address >> 32);
#ifndef SOC_ACTIVE
        user_dev.bus_no = dal_dev[lchip].pci_dev->bus->number;
        user_dev.dev_no = dal_dev[lchip].pci_dev->device;
        user_dev.fun_no = dal_dev[lchip].pci_dev->devfn;
#else
        user_dev.bus_no = 0;
        user_dev.dev_no = CTC_TSINGMA_DEVICE_ID;
        user_dev.fun_no = 0;
#endif
    }

    if (copy_to_user((dal_user_dev_t*)arg, (void*)&user_dev, sizeof(user_dev)))
    {
        return -EFAULT;
    }

    return 0;
}

/* get dma information */
int32
dal_get_dma_info(unsigned int lchip, void* p_info)
{
    dma_info_t* p_dma = NULL;

    p_dma = (dma_info_t*)p_info;

    p_dma->phy_base = (unsigned int)dma_phy_base[lchip];
#ifdef PHYS_ADDR_IS_64BIT
    p_dma->phy_base_hi = dma_phy_base[lchip] >> 32;
#else
    p_dma->phy_base_hi = 0;
#endif

    p_dma->size = dma_mem_size;

    return 0;
}

#ifdef CONFIG_COMPAT
STATIC long
linux_dal_ioctl(struct file* file,
                unsigned int cmd, unsigned long arg)
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
STATIC long
linux_dal_ioctl(struct file* file,
                unsigned int cmd, unsigned long arg)
#else
STATIC int
linux_dal_ioctl(struct inode* inode, struct file* file,
                unsigned int cmd, unsigned long arg)
#endif
#endif
{
    switch (cmd)
    {
#if 0
    case CMD_READ_CHIP:
        return linux_dal_read(arg);

    case CMD_WRITE_CHIP:
        return linux_dal_write(arg);

#endif

    case CMD_GET_DEVICES:
        return linux_dal_get_device(arg);

    case CMD_PCI_CONFIG_READ:
        return dal_user_read_pci_conf(arg);

    case CMD_PCI_CONFIG_WRITE:
        return dal_user_write_pci_conf(arg);

		break;

    default:
        break;
    }

    return 0;
}

#ifndef SOC_ACTIVE
void
linux_dal_remove(struct pci_dev* pdev)
#else
static int
linux_dal_remove(struct platform_device *pdev)
#endif
{
    unsigned int lchip = 0;
    unsigned int flag = 0;

    for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip ++)
    {
        if (pdev == dal_dev[lchip].pci_dev)
        {
            flag = 1;
            break;
        }
    }
    if (1 == flag)
    {
        /*mapping to dal chip*/
        dal_free_dma_pool(lchip);
#ifndef SOC_ACTIVE
        pci_release_regions(pdev);
        pci_disable_device(pdev);
#endif

        dal_dev[lchip].pci_dev = NULL;
        dal_chip_num--;
    }

#ifdef SOC_ACTIVE
    return 0;
#endif
}

static struct file_operations fops_dal =
{
    .owner = THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = linux_dal_ioctl,
    .unlocked_ioctl = linux_dal_ioctl,
    .mmap = linux_dal_mmap0,
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
    .unlocked_ioctl = linux_dal_ioctl,
    .mmap = linux_dal_mmap0,
#else
    .ioctl = linux_dal_ioctl,
    .mmap = linux_dal_mmap0,
#endif
#endif
};

#ifndef SOC_ACTIVE
static struct pci_driver linux_dal_driver =
{
    .name = DAL_NAME,
    .id_table = dal_id_table,
    .probe = linux_dal_probe,
    .remove = linux_dal_remove,
};
#else
static struct platform_driver linux_dal_driver =
{
    .probe = linux_dal_probe,
    .remove = linux_dal_remove,
    .driver = {
        .name = DAL_NAME,
        .of_match_table = of_match_ptr(linux_dal_of_match),
    },
};
#endif

extern int32 userinit(uint8 ctc_shell_en, uint8 ctcs_api_en, void* p_init_config);

STATIC int __init
linux_kernel_init(void)
{
    int ret;

    /* Get DMA memory pool size */
    if (dma_pool_size)
    {
        if ((dma_pool_size[strlen(dma_pool_size) - 1] & ~0x20) == 'M')
        {
            dma_mem_size = simple_strtoul(dma_pool_size, NULL, 0);
            dma_mem_size *= MB_SIZE;
        }
        else
        {
            printk("DMA memory pool size must be specified as e.g. dma_pool_size=8M\n");
        }

        if (dma_mem_size & (dma_mem_size - 1))
        {
            printk("dma_mem_size must be a power of 2 (1M, 2M, 4M, 8M etc.)\n");
            dma_mem_size = 0;
        }
    }

    ret = register_chrdev(DAL_DEV_MAJOR, "linux_dal0", &fops_dal);
    if (ret < 0)
    {
        printk(KERN_WARNING "Register linux_dal device, ret %d\n", ret);
        return ret;
    }

#ifndef SOC_ACTIVE
    ret = pci_register_driver(&linux_dal_driver);
    if (ret < 0)
    {
        printk(KERN_WARNING "Register ASIC PCI driver failed, ret %d\n", ret);
        return ret;
    }
#else
    ret = platform_driver_register(&linux_dal_driver);
    if (ret < 0)
    {
        printk(KERN_WARNING "Register ASIC LOCALBUS driver failed, ret %d\n", ret);
    }
#endif

#if 0
    printk("linux_dal_init: dma_virt_base %p dma_phy_base %d 0x%p %d\r\n",
        dma_virt_base, dma_phy_base, dma_pool, dma_mem_size);
#endif
    /* init interrupt function */
    intr_handler_fun[0] = intr0_handler;
    intr_handler_fun[1] = intr1_handler;
    intr_handler_fun[2] = intr2_handler;
    intr_handler_fun[3] = intr3_handler;
    intr_handler_fun[4] = intr4_handler;
    intr_handler_fun[5] = intr5_handler;
    intr_handler_fun[6] = intr6_handler;
    intr_handler_fun[7] = intr7_handler;

    ret = userinit(1, 0, NULL);

    return ret;
}

extern void
ctc_vty_close(void);

STATIC void __exit
linux_kernel_exit(void)
{
    int intr_idx = 0;
    int intr_num = 0;
    uint8 lchip = 0;

    intr_num = dal_intr_num;

    for (intr_idx = 0; intr_idx < intr_num; intr_idx++)
    {
        dal_interrupt_unregister(dal_isr[intr_idx].irq);
    }
    if (msi_used)
    {
        for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip ++)
        {

            dal_kernel_set_msi_enable(lchip, FALSE, msi_irq_num[lchip]);
        }
    }
    unregister_chrdev(DAL_DEV_MAJOR, "linux_dal0");
#ifndef SOC_ACTIVE
    pci_unregister_driver(&linux_dal_driver);
#else
    platform_driver_unregister(&linux_dal_driver);
#endif

    ctc_vty_close();
}

/* set device access type, must be configured before dal_op_init */
int32_t
dal_set_device_access_type(dal_access_type_t device_type)
{
    if (device_type >= DAL_MAX_ACCESS_TYPE)
    {
        return DAL_E_INVALID_ACCESS;
    }

    g_dal_access = device_type;
    return 0;
}

/* get device access type */
int32_t
dal_get_device_access_type(dal_access_type_t* p_device_type)
{
    *p_device_type = g_dal_access;
    return 0;
}


/* get local chip num */
int32_t
dal_get_chip_number(uint8* p_num)
{
    *p_num = dal_chip_num;

    return 0;
}

/* get local chip device id */
int32_t
dal_get_chip_dev_id(uint8 lchip, uint32* dev_id)
{
    uint8 index = 0;

    index = dal_lchip[lchip];

#ifndef SOC_ACTIVE
    *dev_id = dal_dev[index].pci_dev->device;
#else
    *dev_id = CTC_TSINGMA_DEVICE_ID;
#endif
    return 0;
}

module_init(linux_kernel_init);
module_exit(linux_kernel_exit);

