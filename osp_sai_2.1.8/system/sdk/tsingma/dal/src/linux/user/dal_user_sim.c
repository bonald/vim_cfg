/**
 @file dal_user.c

 @date 2012-10-23

 @version v2.0

*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/klog.h>
#include <sys/mman.h>

#include <poll.h>

#include "sal.h"
#include "dal.h"
#include "dal_user.h"
#include "dal_mpool.h"
#include "kernel/dal_kernel.h"

#if defined(GOLDENGATE) || defined(GREATBELT)
#if defined(GOLDENGATE)
#include "goldengate/include/drv_chip_agent.h"
extern int32 sys_goldengate_chip_check_active(uint8 lchip);
#else
#include "greatbelt/include/drv_chip_agent.h"
extern int32 sys_greatbelt_chip_check_active(uint8 lchip);
#endif
#else
#include "drv_api.h"
extern int32 sys_usw_chip_check_active(uint8 lchip);
#endif

#if (SDK_WORK_PLATFORM == 1 && SDK_WORK_ENV == 1)  /*simulation */
uint32 cm_sim_intr_engine(void);
#endif
/*****************************************************************************
 * defines
 *****************************************************************************/

/*****************************************************************************
 * typedef
 *****************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/
static intr_handler_t intr_handlers[DAL_MAX_INTR_NUM];
uint8 g_dal_debug_on = 0;
static dal_access_type_t g_dal_access = DAL_PCIE_MM;
static dal_mpool_mem_t* dma_pool[DAL_MAX_CHIP_NUM];
dal_dma_info_t dma_info[DAL_MAX_CHIP_NUM];
uint8 g_dal_init = 0;
dal_op_t g_dal_op =
{
    pci_read: NULL,
    pci_write: NULL,
    pci_conf_read: NULL,
    pci_conf_write: NULL,
    i2c_read: NULL,
    i2c_write: NULL,
    interrupt_register: dal_interrupt_register,
    interrupt_unregister:dal_interrupt_unregister,
    interrupt_set_en: NULL,
    interrupt_get_msi_info: NULL,
    interrupt_set_msi_cap:NULL,
    logic_to_phy: dal_logic_to_phy,
    phy_to_logic: dal_phy_to_logic,
    dma_alloc: dal_dma_alloc,
    dma_free: dal_dma_free,
    dma_cache_inval: dal_dma_cache_inval,
    dma_cache_flush: dal_dma_cache_flush,
};

/*****************************************************************************
 * macros
 *****************************************************************************/
/*****************************************************************************
 * functions
 *****************************************************************************/

int32
dal_op_init(dal_op_t* p_dal_op)
{
    uint8 lchip = 0;
    uint64 pbase = 0;

    if (1 == g_dal_init)
    {
        return 0;
    }

    if (p_dal_op)
    {
        lchip = p_dal_op->lchip;
    }

    lchip = ((lchip>=DAL_MAX_CHIP_NUM) ? (DAL_MAX_CHIP_NUM-1): lchip);

    dal_mpool_init(lchip);

    sal_memset(&dma_info[lchip], 0, sizeof(dma_info[lchip]));
    dma_info[lchip].size = 1024 * 1024*12;
    dma_info[lchip].virt_base = sal_malloc(dma_info[lchip].size);
    if (!dma_info[lchip].virt_base)
    {
        return -1;
    }


    pbase = (uintptr)dma_info[lchip].virt_base;
    dma_info[lchip].phy_base = (uint32)pbase;
    pbase >>= 32;
    dma_info[lchip].phy_base_hi = pbase;

    dma_pool[lchip] = dal_mpool_create(lchip, dma_info[lchip].virt_base, dma_info[lchip].size);

    g_dal_init = 1;

    return 0;
}

int32
dal_op_deinit(uint8 lchip)
{
    g_dal_init = 0;
    dal_mpool_destroy(lchip, dma_pool[lchip]);
    sal_free(dma_info[lchip].virt_base);
    dal_mpool_deinit(lchip);
    return 0;
}

/* set device access type, must be configured before dal_op_init */
int32
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
int32
dal_get_device_access_type(dal_access_type_t* p_device_type)
{
    *p_device_type = g_dal_access;
    return 0;
}


/* get local chip num */
int32
dal_get_chip_number(uint8* p_num)
{
    *p_num = DAL_MAX_CHIP_NUM;

    return 0;
}

/* get local chip device id */
int32
dal_get_chip_dev_id(uint8 lchip, uint32* dev_id)
{
    int32 ret = 0;

    *dev_id = 0xFFFF;
#ifdef GREATBELT
            *dev_id = DAL_GREATBELT_DEVICE_ID;

#elif defined GOLDENGATE
            *dev_id = DAL_GOLDENGATE_DEVICE_ID;

#elif defined TSINGMA
            *dev_id = DAL_TSINGMA_DEVICE_ID;

#elif defined DUET2
            *dev_id = DAL_DUET2_DEVICE_ID;

#else
            *dev_id = DAL_HUMBER_DEVICE_ID;
#endif

    return ret;
}

/* convert logic address to physical, only using for DMA */
uint64
dal_logic_to_phy(uint8 lchip, void* laddr)
{
    uint64 pbase = 0;

    pbase = dma_info[lchip].phy_base_hi;
    pbase <<= 32;
    pbase |= dma_info[lchip].phy_base;

    if (dma_info[lchip].size)
    {
        /* dma memory is a contiguous block */
        if (laddr)
        {
            return (pbase + ((uintptr)(laddr) - (uintptr)(dma_info[lchip].virt_base)));
        }
    }

    return 0;
}

/* convert physical address to logical, only using for DMA */
void*
dal_phy_to_logic(uint8 lchip, uint64 paddr)
{
    uint64 pbase = 0;

    pbase = dma_info[lchip].phy_base_hi;
    pbase <<= 32;
    pbase |= dma_info[lchip].phy_base;

    if (dma_info[lchip].size)
    {
        /* dma memory is a contiguous block */
        return (void*)(paddr ? (char*)dma_info[lchip].virt_base + (paddr - pbase) : NULL);
    }

    return 0;
}

/* alloc dma memory from memory pool*/
uint32*
dal_dma_alloc(uint8 lchip, int32 size, int32 type)
{
    return dal_mpool_alloc(lchip, dma_pool[lchip], size, type);
}

/*free memory to memory pool*/
void
dal_dma_free(uint8 lchip, void* ptr)
{
    return dal_mpool_free(lchip, dma_pool[lchip], ptr);
}

/*invalid dma cache */
int32
dal_dma_cache_inval(uint8 lchip, uint64 ptr, int32 length)
{
    return 0;
}

/*flush dma cache*/
int32
dal_dma_cache_flush(uint8 lchip, uint64 ptr, int32 length)
{
    return 0;
}

/* this interface is compatible for humber access */
int32
dal_usrctrl_write_chip(uint8 lchip, uint32 offset, uint32 value)
{
    int32 ret = 0;

    if (g_dal_op.pci_write)
    {
        ret = g_dal_op.pci_write(lchip, offset, value);
        if (ret < 0)
        {
            DAL_DEBUG_DUMP("dal_usrctrl_write_chip   write failed ,%d\n", ret);

            return ret;
        }
    }

    return 0;
}

/* just for compile*/
int32
dal_usrctrl_read_chip(uint8 lchip, uint32 offset, uint32 p_value)
{

    return 0;
}

#ifndef GREATBELT
STATIC int32
_dal_get_table_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, void* ds, uint32* value)
{
#if defined(GOLDENGATE)
    return drv_goldengate_get_field(tbl_id, field_id, ds, value);
#else
    return drv_get_field(lchip, tbl_id, field_id, ds, value);
#endif
}

STATIC uint32
_dal_intr_engine_sim(void)
{
    uint32 map[8] = {0};
    uint8 lchip = 0;
    int32 ret = 0;
    uint32 map_ctl[MAX_ENTRY_WORD];   /* SupIntrMapCtl_t */
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 group_id = 0;
    uint8  chip_num = DAL_MAX_CHIP_NUM;
    uint8  occur_flag = 0;
    uint32 intr_vec = 0;

    sal_memset(&map_ctl, 0, sizeof(map_ctl));
    sal_memset(&map, 0, sizeof(map));

    for (lchip = 0; lchip < chip_num; lchip++)
    {
#if defined(GOLDENGATE) || defined(GREATBELT)
#if defined(GOLDENGATE)
        if (sys_goldengate_chip_check_active(lchip) < 0)
#else
        if (sys_greatbelt_chip_check_active(lchip) < 0)
#endif
#else
        if (sys_usw_chip_check_active(lchip) < 0)
#endif
        {
            continue;
        }

        cmd = DRV_IOR(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &intr_vec);

        if (intr_vec)
        {
            tbl_id = SupIntrMapCtl_t;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, map_ctl);
            if (ret < 0)
            {
                continue;
            }

            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap0_f, map_ctl, &map[0]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap1_f, map_ctl, &map[1]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap2_f, map_ctl, &map[2]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap3_f, map_ctl, &map[3]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap4_f, map_ctl, &map[4]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap5_f, map_ctl, &map[5]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap6_f, map_ctl, &map[6]);
            _dal_get_table_field(lchip, tbl_id, SupIntrMapCtl_supIntrMap7_f, map_ctl, &map[7]);

            if (map[0] & intr_vec)
            {
                group_id = 0;
                occur_flag = 1;
            }
            else if (map[1] & intr_vec)
            {
                group_id = 1;
                occur_flag = 1;
            }
            else if (map[2] & intr_vec)
            {
                group_id = 2;
                occur_flag = 1;
            }
            else if (map[3] & intr_vec)
            {
                group_id = 3;
                occur_flag = 1;
            }
            else if (map[4] & intr_vec)
            {
                group_id = 4;
                occur_flag = 1;
            }
            else if (map[5] & intr_vec)
            {
                group_id = 5;
                occur_flag = 1;
            }
            else if (map[6] & intr_vec)
            {
                group_id = 6;
                occur_flag = 1;
            }
            else if (map[7] & intr_vec)
            {
                group_id = 7;
                occur_flag = 1;
            }
            else
            {
                occur_flag = 0;
                continue;
            }
        }
    }
    return (occur_flag)?group_id:0xff;

}
#endif

void
dal_intr_thread(void* user_param)
{
    uint32 para = (uintptr)user_param;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int32 intr_idx = 0;
    int32 prio = 0;
    int32 irq = 0;
    intr_handler_t* intr_handler = NULL;
    intr_idx = p_para->intr_idx;
    prio = p_para->prio;
    sal_task_set_priority(prio);
    intr_handler = (intr_handler_t*)&(intr_handlers[intr_idx]);

    irq = p_para->irq;
    while(1)
    {
#if ((SDK_WORK_PLATFORM == 1) ||((SDK_WORK_PLATFORM == 0) && (SDK_WORK_ENV == 1)))   /*simulation */
#if defined(GOLDENGATE) || defined(GREATBELT)
#if defined(GOLDENGATE)
        if( DRV_CHIP_AGT_MODE_CLIENT == drv_goldengate_chip_agent_mode())
#else
        if( DRV_CHIP_AGT_MODE_CLIENT == drv_greatbelt_chip_agent_mode())
#endif
#else
        if( DRV_CHIP_AGT_MODE_CLIENT == drv_get_chip_agent_mode())
#endif
        {
            return;
        }
        uint32 grp_id = 0;

        if(intr_handlers[intr_idx].active == 0)
        {
            break;
        }

#if defined(GREATBELT)
        grp_id = cm_sim_intr_engine();
#else
        grp_id = _dal_intr_engine_sim();
#endif
        if (grp_id != 0xff)
        {
            intr_handler->handler((void*)&(irq));
        }
#endif
         /*sal_task_sleep(50);*/
        usleep(50*1000);
    }

    return;
}

/* register interrupt, for msi irq just mean msi irq index, not actual irq */
int32
dal_interrupt_register(uint32 irq, int32 prio, void (* isr)(void*), void* data)
{
    char intr_name[32];
    uintptr para = 0;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int32 intr_idx = 0;
    int32 ret = 0;

    sal_memset(intr_name, 0, 32);

    sal_snprintf(intr_name, 32, "ctcIntr%d", irq);

    intr_idx = (intptr)data;
    if (DAL_MAX_INTR_NUM <= intr_idx)
    {
        return -1;
    }
    intr_handlers[intr_idx].handler = isr;
    intr_handlers[intr_idx].irq = irq;

    p_para->intr_idx = (int8)intr_idx;
    p_para->prio = (int16)prio;
    p_para->irq = (uint8)irq;
    intr_handlers[intr_idx].active = 1;
    sal_task_create(&intr_handlers[intr_idx].p_intr_thread,
            intr_name,
            SAL_DEF_TASK_STACK_SIZE,
            prio,
            dal_intr_thread,
            (void*)para);

    return ret;
}

int32
dal_interrupt_unregister(unsigned int irq)
{
    uint32 irq_index = 0;

    for(irq_index=0; irq_index < DAL_MAX_INTR_NUM; irq_index++)
    {
        if(irq == intr_handlers[irq_index].irq)
        {
            break;
        }
    }
    if(irq_index >= DAL_MAX_INTR_NUM)
    {
        DAL_DEBUG_DUMP("IRQ %d not register in user mode!!!\n", irq);
        return -1;
    }

    intr_handlers[irq_index].active = 0;
    sal_task_destroy(intr_handlers[irq_index].p_intr_thread);
    sal_memset(&(intr_handlers[irq_index]), 0, sizeof(intr_handler_t));

    return 0;
}

int32
dal_get_dma_info(unsigned int lchip, void* p_info)
{
    dal_dma_info_t *p_dma = (dal_dma_info_t*)p_info;

    p_dma->phy_base = dma_info[lchip].phy_base;
    p_dma->size = dma_info[lchip].size;
    p_dma->virt_base = dma_info[lchip].virt_base;
    p_dma->phy_base_hi = dma_info[lchip].phy_base_hi;

    return 0;
}

int32
dal_dma_chan_register(uint8 lchip, void* data)
{
    return -1;
}

int32
dal_dma_finish_cb_register(dal_dma_finish_callback_t func)
{
    return -1;
}

/* debug dma pool */
int32
dal_dma_debug_info(uint8 lchip)
{
    if (NULL == dma_pool[lchip])
    {
        return DAL_E_MPOOL_NOT_CREATE;
    }

    dal_mpool_debug(dma_pool[lchip]);
    return 0;
}

