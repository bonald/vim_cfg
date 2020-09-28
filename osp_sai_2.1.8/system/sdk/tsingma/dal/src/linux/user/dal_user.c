/**
 @file dal_user.c

 @date 2012-10-23

 @version v2.0

*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include "kernel/knet/knet_kernel.h"

/*****************************************************************************
 * defines
 *****************************************************************************/
#define CMD_READ_CHIP_EMU 25
#define CMD_WRITE_CHIP_EMU 24
/*****************************************************************************
 * typedef
 *****************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/

static int32 dal_dev_fd = -1;
static int32 knet_dev_fd = -1;
uint8 g_dal_debug_on = 0;
dal_dma_info_t g_dma_info[DAL_MAX_CHIP_NUM];

#ifndef HUMBER
dal_op_t g_dal_op =
{
    pci_read: dal_pci_read,
    pci_write: dal_pci_write,
    pci_conf_read: dal_pci_conf_read,
    pci_conf_write: dal_pci_conf_write,
    i2c_read: dal_i2c_read,
    i2c_write: dal_i2c_write,
    interrupt_register: dal_interrupt_register,
    interrupt_unregister: dal_interrupt_unregister,
    interrupt_set_en: dal_interrupt_set_en,
    interrupt_get_msi_info: dal_interrupt_get_msi_info,
    interrupt_set_msi_cap: dal_interrupt_set_msi_cap,
    logic_to_phy: dal_logic_to_phy,
    phy_to_logic: dal_phy_to_logic,
    dma_alloc: dal_dma_alloc,
    dma_free: dal_dma_free,
    dma_cache_inval: dal_dma_cache_inval,
    dma_cache_flush: dal_dma_cache_flush,
    interrupt_get_irq: dal_interrupt_get_irq,
    handle_netif: dal_handle_netif,
};
#else
dal_op_t g_dal_op =
{
    pci_read: dal_pci_read,
    pci_write: dal_pci_write,
    pci_conf_read: NULL,
    pci_conf_write: NULL,
    i2c_read: NULL,
    i2c_write: NULL,
    interrupt_register: NULL,
    interrupt_unregister: NULL,
    interrupt_set_en: NULL,
    interrupt_set_msi_cap:NULL,
    logic_to_phy: NULL,
    phy_to_logic: NULL,
    dma_alloc: NULL,
    dma_free: NULL,
    dma_cache_inval: NULL,
    dma_cache_flush: NULL,
    interrupt_get_irq : NULL,
};
#endif

static uint8 knet_enable = 0;
static dal_user_dev_t dal_dev[DAL_MAX_CHIP_NUM];
static void* dal_virt_base[DAL_MAX_CHIP_NUM];        /* Virtual base address */
static int32 dal_mem_fd = -1;
static int32 dal_version = VERSION_1DOT2;
static intr_handler_t intr_handlers[DAL_MAX_INTR_NUM];
static int32 dal_i2c_fd = -1;
static dal_mpool_mem_t* dma_pool[DAL_MAX_CHIP_NUM];
static dal_access_type_t g_dal_access = DAL_PCIE_MM;
static dal_dma_finish_handler_t knet_dma_cb;
STATIC void* dal_mmap(uintptr p, int size);

/*****************************************************************************
 * macros
 *****************************************************************************/
#define KNET_TX_MEM_SIZE    320 * 1024
/*****************************************************************************
 * functions
 *****************************************************************************/
int32
dal_dma_finish_cb_register(dal_dma_finish_callback_t func)
{
    knet_dma_cb.dma_finish_cb = func;

    return 0;
}

STATIC void
_knet_dma_finish_thread(void* param)
{
#define SYS_KNET_MAX_DMA_CHAN    5
    struct sal_pollfd poll_list;
    uint8 lchip = *(uint8 *)param;
    int ret = 0;

    poll_list.fd = knet_dev_fd;
    poll_list.events = POLLIN;

    while (1)
    {
        ret = sal_poll(&poll_list, 1, DAL_INTR_POLL_TIMEOUT);
        if (ret)
        {
            /* 2. if poll ok & have POLLIN, call dispatch */
            if (POLLIN != (poll_list.revents & POLLIN))
            {
                continue;
            }

            knet_dma_cb.dma_finish_cb(lchip);
        }

        if (!knet_dma_cb.active)
        {
            break;
        }
    }

    return;
}

/*
   Notice: for user space 32bits mode and kernel space 64bits mode, if do mmap fail, should using
              DAL_USE_MMAP2 by #define DAL_USE_MMAP2
*/
int32
dal_op_init(dal_op_t* p_dal_op)
{
    int32 ret = 0;
    uint8 index = 0;
    unsigned char dal_name[32] = {0};
    uint64 pbase = 0;
    dal_dma_info_t dma_info;
    int32 dal_ver;
    int32 knet_ver;
    dal_user_dev_t dal_tmp;
    dal_pci_dev_t pci_dev;
    uint8 lchip = 0;

    sal_memset(&dal_tmp, 0, sizeof(dal_user_dev_t));
    sal_memset(&pci_dev, 0, sizeof(dal_pci_dev_t));
    sal_memset(&dma_info, 0, sizeof(dal_dma_info_t));

    /* if dal op is provided by user , using user defined */
    if (p_dal_op)
    {
        lchip = p_dal_op->lchip;
        if (0 == lchip)
        {
            g_dal_op.pci_read               = p_dal_op->pci_read?p_dal_op->pci_read:g_dal_op.pci_read;
            g_dal_op.pci_write              = p_dal_op->pci_write?p_dal_op->pci_write:g_dal_op.pci_write;
            g_dal_op.pci_conf_read          = p_dal_op->pci_conf_read?p_dal_op->pci_conf_read:g_dal_op.pci_conf_read;
            g_dal_op.pci_conf_write         = p_dal_op->pci_conf_write?p_dal_op->pci_conf_write:g_dal_op.pci_conf_write;
            g_dal_op.i2c_read               = p_dal_op->i2c_read?p_dal_op->i2c_read:g_dal_op.i2c_read;
            g_dal_op.i2c_write              = p_dal_op->i2c_write?p_dal_op->i2c_write:g_dal_op.i2c_write;
            g_dal_op.interrupt_register     = p_dal_op->interrupt_register?p_dal_op->interrupt_register:g_dal_op.interrupt_register;
            g_dal_op.interrupt_unregister   = p_dal_op->interrupt_unregister?p_dal_op->interrupt_unregister:g_dal_op.interrupt_unregister;
            g_dal_op.interrupt_set_en       = p_dal_op->interrupt_set_en?p_dal_op->interrupt_set_en:g_dal_op.interrupt_set_en;
            g_dal_op.interrupt_get_msi_info = p_dal_op->interrupt_get_msi_info?p_dal_op->interrupt_get_msi_info:g_dal_op.interrupt_get_msi_info;
            g_dal_op.interrupt_set_msi_cap  = p_dal_op->interrupt_set_msi_cap?p_dal_op->interrupt_set_msi_cap:g_dal_op.interrupt_set_msi_cap;
            g_dal_op.logic_to_phy           = p_dal_op->logic_to_phy?p_dal_op->logic_to_phy:g_dal_op.logic_to_phy;
            g_dal_op.phy_to_logic           = p_dal_op->phy_to_logic?p_dal_op->phy_to_logic:g_dal_op.phy_to_logic;
            g_dal_op.dma_alloc              = p_dal_op->dma_alloc?p_dal_op->dma_alloc:g_dal_op.dma_alloc;
            g_dal_op.dma_free               = p_dal_op->dma_free?p_dal_op->dma_free:g_dal_op.dma_free;
            g_dal_op.dma_cache_inval        = p_dal_op->dma_cache_inval?p_dal_op->dma_cache_inval:g_dal_op.dma_cache_inval;
            g_dal_op.dma_cache_flush        = p_dal_op->dma_cache_flush?p_dal_op->dma_cache_flush:g_dal_op.dma_cache_flush;
            g_dal_op.interrupt_get_irq        = p_dal_op->interrupt_get_irq?p_dal_op->interrupt_get_irq:g_dal_op.interrupt_get_irq;
            g_dal_op.handle_netif               = p_dal_op->handle_netif?p_dal_op->handle_netif:g_dal_op.handle_netif;
        }
        sal_memcpy(&pci_dev, &(p_dal_op->pci_dev), sizeof(dal_pci_dev_t));
    }

    if (g_dal_access == DAL_PCIE_MM)
    {
        /* 1. get the fd of /dev/linux_dal  */
        if (dal_dev_fd < 0)
        {
            snprintf((char*)dal_name, 16, "%s", DAL_DEV_NAME);
            if ((dal_dev_fd = sal_open((char *)dal_name, O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0)
            {
                DAL_DEBUG_DUMP("open %s error!!", dal_name);
                perror("open " DAL_DEV_NAME ": ");
                return -1;
            }
        }

        if (knet_dev_fd < 0)
        {
            snprintf((char*)dal_name, 16, "%s", KNET_DEV_NAME);
            if ((knet_dev_fd = sal_open((char *)dal_name, O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0)
            {
                if (errno != ENOENT)
                {
                    DAL_DEBUG_DUMP("open %s error!!", dal_name);
                    perror("open " KNET_DEV_NAME ": ");
                    return -1;
                }
            }
        }

        /* 2. get dal_kernel version, and check with dal_user version */
        ret = dal_usrctrl_do_cmd(CMD_GET_DAL_VERSION, (uintptr)& dal_ver);
        if (ret)
        {
            DAL_DEBUG_DUMP("get dal version fail !!\n");
            return ret;
        }
        if (dal_ver != dal_version)
        {
            DAL_DEBUG_DUMP("dal_user: %d dal_kernel: %d version are not match !!\n", dal_version, dal_ver);
            return ret;
        }

        if (knet_dev_fd > 0)
        {
            ret = dal_usrctrl_do_cmd(CMD_GET_KNET_VERSION, (uintptr)& knet_ver);
            if (ret)
            {
                DAL_DEBUG_DUMP("get knet version fail !!\n");
                return ret;
            }
            if (knet_ver)
            {
                knet_enable = 1;
            }
        }

        if (!knet_enable)
        {
            g_dal_op.handle_netif = NULL;
        }

        /* 3. get fd for mmap */
        if (-1 == dal_mem_fd)
        {
            if ((dal_mem_fd = sal_open("/dev/mem", O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0)
            {
                perror("open /dev/mem: ");
                DAL_DEBUG_DUMP("open /dev/mem error!!\n");
                return -1;
            }
        }

        if ((0 != pci_dev.busNo) || (0 != pci_dev.devNo) || (0 != pci_dev.funNo))
        {
            /*mapping lchip to centec device scaned in kernel*/
            for (index = 0; index < DAL_MAX_CHIP_NUM; index++)
            {
                dal_tmp.lchip = index;
                ret = dal_usrctrl_do_cmd(CMD_GET_DEVICES, (uintptr)&dal_tmp);
                if (ret)
                {
                    DAL_DEBUG_DUMP("get chip num fail !!\n");
                    SAL_LOG_DEBUG("get chip num fail !! ret = 0x%x\n", ret);
                    return ret;
                }

                if ((dal_tmp.bus_no == pci_dev.busNo) && (dal_tmp.dev_no == pci_dev.devNo)
                    && (dal_tmp.fun_no == pci_dev.funNo))
                {
                    dal_dev[lchip].lchip = dal_tmp.lchip;
                    break;
                }
            }

            if (index >= DAL_MAX_CHIP_NUM)
            {
                DAL_DEBUG_DUMP("not find the lchip %d device bus:0x%x dev:0x%x fun:0x%x\n", lchip, pci_dev.busNo, pci_dev.devNo, pci_dev.funNo);
                return -1;
            }
        }
        else
        {
            dal_dev[lchip].lchip = lchip;
        }

        /* 5. get chip address info from kernel for each chip */
        ret = dal_usrctrl_do_cmd(CMD_GET_DEVICES, (uintptr)&dal_dev[lchip]);
        if (ret)
        {
            DAL_DEBUG_DUMP("get device physical address error!!\n");
            SAL_LOG_DEBUG("get device physical address error!! ret = 0x%x\n", ret);
            return ret;
        }

        g_dal_op.soc_active = dal_dev[lchip].soc_active;
        /* Get physical devices address */
        pbase = dal_dev[lchip].phy_base1;
        pbase <<= 32;
        pbase |= dal_dev[lchip].phy_base0;

        dal_virt_base[lchip] = dal_mmap(pbase, 0x1000);
        DAL_DEBUG_DUMP("dal_virt_base[%d]: 0x%x\n", lchip, dal_virt_base[lchip]);
        if (!dal_virt_base[lchip])
        {
            DAL_DEBUG_DUMP("mmap failed \n");
            return -1;
        }


        dal_i2c_open();

        /* 6. DMA init */
        dal_mpool_init(lchip);

        /* get dma info from kernel */
        dma_info.lchip = dal_dev[lchip].lchip;
        dal_usrctrl_do_cmd(CMD_GET_DMA_INFO, (uintptr) &dma_info);

        sal_task_sleep(2);
        pbase = dma_info.phy_base_hi;
        pbase <<= 32;
        pbase |= dma_info.phy_base;

        DAL_DEBUG_DUMP("Dma info, size: 0x%x, phy_base_hi: 0x%lx, phy_base: 0x%lx, pbase: 0x%"PRIx64"\n",
                                                        dma_info.size, dma_info.phy_base_hi, dma_info.phy_base, pbase);

        if (knet_enable)
        {
            dma_info.knet_tx_size = KNET_TX_MEM_SIZE;
            dma_info.knet_tx_offset = knet_enable ? (dma_info.size - dma_info.knet_tx_size) : dma_info.size;

            ret = dal_usrctrl_do_cmd(CMD_SET_DMA_INFO, (uintptr) &dma_info);
            if (ret)
            {
                DAL_DEBUG_DUMP("set knet dma info fail !!\n");
                return ret;
            }
        }
        else
        {
            dma_info.knet_tx_size = 0;
            dma_info.knet_tx_offset = dma_info.size;
        }

        dma_info.virt_base = dal_mmap(pbase, dma_info.knet_tx_offset);
        if (!dma_info.virt_base)
        {
            DAL_DEBUG_DUMP("mmap dma physical address failed \n");
            return -1;
        }
        DAL_DEBUG_DUMP("Dma info, virt_base: 0x%x\n", dma_info.virt_base);

        g_dma_info[lchip].virt_base = dma_info.virt_base;
        g_dma_info[lchip].phy_base = dma_info.phy_base;
        g_dma_info[lchip].phy_base_hi = dma_info.phy_base_hi;
        g_dma_info[lchip].knet_tx_offset = dma_info.knet_tx_offset;
        g_dma_info[lchip].size = dma_info.size;

        dma_pool[lchip] = dal_mpool_create(lchip, dma_info.virt_base, dma_info.knet_tx_offset);

        if (knet_enable)
        {
            ret = sal_task_create(&knet_dma_cb.p_dma_cb_task, "knet_dma_cb_task",
                                          SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_LOW, _knet_dma_finish_thread, (void*)&lchip);
            if (ret < 0)
            {
    	        return -1;
            }
            knet_dma_cb.active = 1;
        }
    }

    return 0;
}

int32
dal_op_deinit(uint8 lchip)
{
    uint8 loop = 0;
    dal_user_dev_t dev_zero;
    /*close fd:centec+mem*/

    if (knet_enable && knet_dma_cb.p_dma_cb_task)
    {
        knet_dma_cb.active = 0;
        sal_task_destroy(knet_dma_cb.p_dma_cb_task);
    }

    /*mpool destroy*/
    dal_mpool_destroy(lchip, dma_pool[lchip]);

    /*unmap*/
    munmap(dal_virt_base[lchip], 0x1000);
    munmap(g_dma_info[lchip].virt_base, g_dma_info[lchip].knet_tx_offset);

    /*clear soft table*/
    sal_memset(&dal_dev[lchip], 0, sizeof(dal_user_dev_t));
    sal_memset(&g_dma_info[lchip], 0, sizeof(dal_dma_info_t));
    dal_mpool_deinit(lchip);

    /*traverse all chip, if all chip have deinited, close dev fd*/
    sal_memset(&dev_zero, 0, sizeof(dev_zero));
    for(loop=0; loop < DAL_MAX_CHIP_NUM; loop++)
    {
        if(sal_memcmp(&dev_zero, &dal_dev[loop], sizeof(dev_zero)))
        {
            break;
        }
    }
    if(loop == DAL_MAX_CHIP_NUM)
    {
        if(dal_dev_fd > 0)
        {
            sal_close(dal_dev_fd);
            dal_dev_fd = -1;
        }
        if(knet_dev_fd > 0)
        {
            sal_close(knet_dev_fd);
            dal_dev_fd = -1;
        }
    }
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
    int32 ret = 0;
    dal_user_dev_t dev;

    sal_memset(&dev, 0, sizeof(dal_user_dev_t));

    ret = dal_usrctrl_do_cmd(CMD_GET_DEVICES, (uintptr)&dev);
    if (ret)
    {
        DAL_DEBUG_DUMP("get chip num fail !!\n");
        SAL_LOG_DEBUG("get chip num fail !! ret = 0x%x\n", ret);
        return ret;
    }

    *p_num = dev.chip_num;

    return ret;
}

/* get local chip device id */
int32
dal_get_chip_dev_id(uint8 lchip, uint32* dev_id)
{
    int32 ret = 0;
    dal_user_dev_t dev;

    sal_memset(&dev, 0, sizeof(dal_user_dev_t));

    dev.lchip = lchip;
    ret = dal_usrctrl_do_cmd(CMD_GET_DEVICES, (uintptr)&dev);
    if (ret)
    {
        DAL_DEBUG_DUMP("get chip dev id fail !!\n");
        SAL_LOG_DEBUG("get chip dev id fail !! ret = 0x%x\n", ret);
        return ret;
    }

    *dev_id = dev.dev_no;

    return ret;
}

int32
dal_usrctrl_do_cmd(uint32 cmd, uintptr p_para)
{
    int32 dev_fd = 0;

    if ((cmd == CMD_GET_KNET_VERSION) ||
        (cmd == CMD_CONNECT_INTERRUPTS) ||
        (cmd == CMD_DISCONNECT_INTERRUPTS) ||
        (cmd == CMD_SET_DMA_INFO) ||
        (cmd == CMD_REG_DMA_CHAN) ||
        (cmd == CMD_HANDLE_NETIF))
    {
        dev_fd = knet_dev_fd;
    }
    else
    {
        dev_fd = dal_dev_fd;
    }

    CHECK_FD(dev_fd);
    CHECK_PTR(p_para);

    return sal_ioctl(dev_fd, cmd, p_para);
}

/* this interface is compatible for humber access */




/* this interface is compatible for humber access */



/* pci read function */
int32
dal_pci_read(uint8 lchip, uint32 offset, uint32* value)
{
    int32 ret = 0;
    dal_chip_parm_t cmdpara_chip;

    sal_memset(&cmdpara_chip, 0, sizeof(dal_chip_parm_t));
    DAL_DEBUG_OUT("reg_read 0x%x lchip:%d base:%p\n", offset, lchip, dal_virt_base[lchip]);

    /* dal for mulit-chip sim, dal_version is VERSION_TEST */
     /*if((VERSION_TEST == dal_version) && sdk_init_ok)*/
    if(0)
    {
        CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, 0, cmdpara_chip);
        ret = dal_usrctrl_do_cmd(CMD_READ_CHIP, (uintptr) & cmdpara_chip);
        if (ret < 0)
        {
            SAL_LOG_DEBUG("read addr :%08X fail, ret:%d\n", offset, ret);
        }
        *value = cmdpara_chip.value;
    }
    else
    {
        switch(g_dal_access)
        {
        case DAL_PCIE_MM:
            /* this mode is usual mode, for support mmap device access */
            if (dal_virt_base[lchip] == NULL)
            {
                return DAL_E_INVALID_PTR;
            }

            *value = *(volatile uint32*)((uint8*)dal_virt_base[lchip] + offset);

            return 0;

        case DAL_PCI_IO:
            /* this mode is used for ioctrl, Humber will use */
            CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, 0, cmdpara_chip);
            ret = dal_usrctrl_do_cmd(CMD_READ_CHIP, (uintptr) & cmdpara_chip);
            if (ret < 0)
            {
                SAL_LOG_DEBUG("read addr :%08X fail, ret:%d\n", offset, ret);
            }

            *value = cmdpara_chip.value;
            break;

        case DAL_SPECIAL_EMU:

            /* this mode is used for ioctrl,  Emulation will use */
            CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, 0, cmdpara_chip);
            ret = dal_usrctrl_do_cmd(CMD_READ_CHIP_EMU, (uintptr) & cmdpara_chip);
            if (ret < 0)
            {
                SAL_LOG_DEBUG("read addr :%08X fail, ret:%d\n", offset, ret);
            }

            *value = cmdpara_chip.value;
            break;

        case DAL_SUPER_IF:

            /* this mode is special for some customer */
            return 0;

        default:
            return DAL_E_INVALID_ACCESS;
        }
    }

    return 0;
}

/*pci write function */
int32
dal_pci_write(uint8 lchip, uint32 offset, uint32 value)
{
    int32 ret = 0;
    dal_chip_parm_t cmdpara_chip;

    sal_memset(&cmdpara_chip, 0, sizeof(dal_chip_parm_t));

    DAL_DEBUG_OUT("reg_write 0x%x lchip:%d base:%p\n", offset, lchip, dal_virt_base[lchip]);

    /* if dal_version is VERSION_TEST, only used for mulit-chip test */
     /*if(VERSION_TEST == dal_version)*/
    if(0)
    {
        CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, value, cmdpara_chip);
        ret = dal_usrctrl_do_cmd(CMD_WRITE_CHIP, (uintptr) & cmdpara_chip);
        if (ret < 0)
        {
            SAL_LOG_DEBUG("Write addr :%08X fail, ret:%d\n", offset, ret);
        }
    }
    else
    {
        switch (g_dal_access)
        {
        case DAL_PCIE_MM:
            /* this mode is usual mode, for support mmap device access */
            if (dal_virt_base[lchip] == NULL)
            {
                return DAL_E_INVALID_PTR;
            }
            *(volatile uint32*)((uint8*)dal_virt_base[lchip] + offset) = value;

            return 0;

        case DAL_PCI_IO:
            /* this mode is used for ioctrl, Humber and Emulation will use */
            CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, value, cmdpara_chip);
            ret = dal_usrctrl_do_cmd(CMD_WRITE_CHIP, (uintptr) & cmdpara_chip);
            if (ret < 0)
            {
                SAL_LOG_DEBUG("Write addr :%08X fail, ret:%d\n", offset, ret);
            }

            return 0;

        case DAL_SPECIAL_EMU:
            /* this mode is used for ioctrl, Humber and Emulation will use */
            CMDPARA_ENCODE_CHIP(dal_dev[lchip].lchip, offset, value, cmdpara_chip);
            ret = dal_usrctrl_do_cmd(CMD_WRITE_CHIP_EMU, (uintptr) & cmdpara_chip);
            if (ret < 0)
            {
                SAL_LOG_DEBUG("Write addr :%08X fail, ret:%d\n", offset, ret);
            }

            return 0;

        case DAL_SUPER_IF:
            /* this mode is special for some customer */
            return 0;

        default:
            return DAL_E_INVALID_ACCESS;
        }
    }

    return 0;
}

/* interface for read pci configuration space, usual using for debug */
int32
dal_pci_conf_read(uint8 lchip, uint32 offset, uint32* value)
{
    dal_pci_cfg_ioctl_t pci_cfg;

    pci_cfg.lchip = dal_dev[lchip].lchip;
    pci_cfg.offset = offset;

    dal_usrctrl_do_cmd(CMD_PCI_CONFIG_READ, (uintptr) & pci_cfg);
    *value = pci_cfg.value;

    return 0;
}

/* interface for write pci configuration space, usual using for debug */
int32
dal_pci_conf_write(uint8 lchip, uint32 offset, uint32 value)
{
    dal_pci_cfg_ioctl_t pci_cfg;

    DAL_DEBUG_OUT("pci_config_write 0x%x 0x%x\n", offset, value);
    pci_cfg.lchip = dal_dev[lchip].lchip;
    pci_cfg.offset = offset;
    pci_cfg.value = value;

    dal_usrctrl_do_cmd(CMD_PCI_CONFIG_WRITE, (uintptr) & pci_cfg);

    return 0;
}

/* open i2c device */
void
dal_i2c_open()
{
    if (dal_i2c_fd < 0)
    {
#ifndef TSINGMA
        dal_i2c_fd = sal_open("/dev/i2c-0",  O_RDWR);
#else
        dal_i2c_fd = sal_open("/dev/i2c-1",  O_RDWR);
#endif
    }

    return;
}

/* just set i2c_msg struct */
void
dal_i2c_switch_msg(struct i2c_msg* msg, uint16 addr,
                   uint8* p_buf, uint16 flag, int32 len)
{
    msg->addr = addr;
    msg->buf = p_buf;
    msg->flags = flag;
    msg->len = len;

    return;
}

/* interface for i2c read */
int32
dal_i2c_read(uint8 lchip, uint16 offset, uint8 len, uint8* buf)
{
    dal_i2c_ioctl_t msgset;
    struct i2c_msg msgs[3];
    uint8 tmp;
    int32 ret = 0;

    DAL_DEBUG_OUT("reg_read offset:0x%x len:0x%x \n", offset, len);

    CHECK_FD(dal_i2c_fd);

    tmp = (uint8)(offset & 0xff);
    dal_i2c_switch_msg(msgs, DAL_ASIC_I2C_ADDR, &tmp, DAL_I2C_WRITE_OP, DAL_ASIC_I2C_ALEN);
    dal_i2c_switch_msg(msgs + 1, DAL_ASIC_I2C_ADDR, buf, DAL_I2C_READ_OP, len);
    msgset.msgs = msgs;
    msgset.nmsgs = 2;

    ret = sal_ioctl(dal_i2c_fd, DAL_I2C_RDWR, &msgset);
    if (ret < 0)
    {
        DAL_DEBUG_DUMP("read i2c error, ret = %d \n", ret);
        return ret;
    }

    return 0;
}

/* interface fot i2c write */
int32
dal_i2c_write(uint8 lchip, uint16 offset, uint8 len, uint8* buf)
{
    dal_i2c_ioctl_t msgset;
    struct i2c_msg msgs[3];
    uint8* tmp;
    int32 ret = 0;
    uint8 index = 0;

    DAL_DEBUG_OUT("reg_write offset:0x%x len:0x%x \n", offset, len);
    DAL_DEBUG_OUT("data_buf: \n");
    for (index = 0; index < len; index++)
    {
        DAL_DEBUG_OUT("0x%8x  ", buf[index]);
    }
    DAL_DEBUG_OUT("\n");

    CHECK_FD(dal_i2c_fd);

    tmp = sal_malloc(len + DAL_ASIC_I2C_ALEN);
    if (!tmp)
    {
        return -1;
    }

    *tmp = (uint8)(offset & 0xff);
    sal_memcpy(tmp + 1, buf, len);
    dal_i2c_switch_msg(msgs, DAL_ASIC_I2C_ADDR, tmp, DAL_I2C_WRITE_OP, len + DAL_ASIC_I2C_ALEN);
    msgset.msgs = msgs;
    msgset.nmsgs = 1;

    ret = sal_ioctl(dal_i2c_fd, DAL_I2C_RDWR, &msgset);
    if (ret < 0)
    {
        DAL_DEBUG_DUMP("write i2c error, ret = %d \n", ret);
        return ret;
    }

    sal_free(tmp);
    tmp = NULL;

    return 0;
}

/* interrupt thread */
STATIC void
dal_intr_thread(void* param)
{
    uint32 para = (uintptr)param;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int32 ret = 0;
    int32 intr_idx = 0;
    int32 irq = 0;
    /*int32 prio = 0;*/
    intr_handler_t* intr_handler = NULL;
    struct sal_pollfd poll_list;
    char dev_name[32];
    char nod_name[32];

    intr_idx = p_para->intr_idx;
    irq = p_para->irq;
    /*prio = p_para->prio;*/
    intr_handler = (intr_handler_t*)&(intr_handlers[intr_idx]);

    sal_memset(dev_name, 0, 32);
    sal_snprintf(dev_name, 32, "%s%d", DAL_INTR_DEV_NAME, intr_idx);
    sal_snprintf(nod_name, 32, "mknod %s%d c %d 0", DAL_INTR_DEV_NAME, intr_idx,DAL_DEV_INTR_MAJOR_BASE+intr_idx);

    system(nod_name);

    poll_list.fd = sal_open(dev_name, O_RDWR, 0644);
    if (poll_list.fd < 0)
    {
        return;
    }

    poll_list.events = POLLIN;

    /*for many chips in one board, there interrupt line should merge together */
    while (1)
    {
        if(intr_handlers[intr_idx].active == 0)
        {
            break;
        }
        /* 1. poll */
        ret = sal_poll(&poll_list, 1, DAL_INTR_POLL_TIMEOUT);
        if (ret)
        {
            /* 2. if poll ok & have POLLIN, call dispatch */
            if (POLLIN == (poll_list.revents & POLLIN))
            {
                intr_handler->handler((void*)&(irq));
            }
        }
    }
}


int32
dal_interrupt_get_msi_info(uint8 lchip, uint8* irq_base)
{
    int32 ret = 0;
    dal_msi_info_t msi_info;

    sal_memset(&msi_info, 0, sizeof(dal_msi_info_t));
    msi_info.lchip = dal_dev[lchip].lchip;

    dal_usrctrl_do_cmd(CMD_GET_MSI_INFO, (uintptr) &msi_info);
    *irq_base = msi_info.irq_base;

    return ret;
}

int32
dal_create_irq_mapping(uint8 hw_irq, uint8* sw_irq)
{
    dal_irq_mapping_t irq_map;

    sal_memset(&irq_map, 0, sizeof(dal_irq_mapping_t));
    irq_map.hw_irq = hw_irq;

    dal_usrctrl_do_cmd(CMD_IRQ_MAPPING, (uintptr) &irq_map);
    if (!irq_map.sw_irq)
    {
        DAL_DEBUG_DUMP("IRQ mapping fail !!!\n");
        return -1;
    }

    *sw_irq = irq_map.sw_irq;

    return 0;
}

int32
dal_interrupt_set_msi_cap(uint8 lchip, uint32 enable, uint32 num)
{
    int32 ret = 0;
    dal_msi_info_t msi_info;

    sal_memset(&msi_info, 0, sizeof(dal_msi_info_t));
    msi_info.lchip = dal_dev[lchip].lchip;
    msi_info.irq_num = num;

    if (enable)
    {
        if (msi_info.irq_num == 0)
        {
            return DAL_E_ENVALID_MSI_PARA;
        }
    }
    else
    {
        msi_info.irq_num = 0;
    }

    ret = dal_usrctrl_do_cmd(CMD_SET_MSI_CAP, (uintptr) &msi_info);
    if (ret&&enable)
    {
        DAL_DEBUG_DUMP("msi enable fail !!!\n");
    }

    return ret;
}

/* NOTE: register interrupt, pin irq and msi irq both are actual irq */
int32
dal_interrupt_register(uint32 irq, int32 prio, void (* isr)(void*), void* data)
{
    uint32 arg = irq;
    char intr_name[32];
    uint32 para = 0;
    dal_intr_para_t* p_para = (dal_intr_para_t*)&para;
    int32 intr_idx = 0;
    int32 ret = 0;
    dal_intr_info_t dal_info;
    char  is_find = 0;

    for(intr_idx=0; intr_idx < DAL_MAX_INTR_NUM; intr_idx++)
    {
        if(irq == intr_handlers[intr_idx].irq)
        {
            is_find = 1;
            break;
        }
    }
    if(is_find)
    {
        intr_handlers[intr_idx].ref_cnt++;
        return 0;
    }

    ret = dal_usrctrl_do_cmd(CMD_REG_INTERRUPTS, (uintptr) &arg);

    sal_memset(intr_name, 0, 32);

    sal_snprintf(intr_name, 32, "ctcIntr%d", irq);

    intr_idx = (uintptr)data;
    if (DAL_MAX_INTR_NUM <= intr_idx)
    {
        return -1;
    }

    sal_memset(&dal_info, 0, sizeof(dal_info));
    dal_info.irq = irq;
    ret = dal_usrctrl_do_cmd(CMD_GET_INTR_INFO, (uintptr)&dal_info);
    if (ret || (dal_info.irq_idx >= DAL_MAX_INTR_NUM))
    {
        DAL_DEBUG_DUMP("IRQ %d not register in kernel!!!\n", irq);
        return -1;
    }

    intr_idx = dal_info.irq_idx;

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
                    (void*)((uintptr)para));
    intr_handlers[intr_idx].ref_cnt++;

    if (knet_enable)
    {
        ret = dal_usrctrl_do_cmd(CMD_CONNECT_INTERRUPTS, (uintptr)&irq);
        if (ret < 0)
        {
            DAL_DEBUG_DUMP("knet connect interrupt error, ret:%d\n", ret);
        }
    }

    return ret;
}

int32
dal_interrupt_unregister(uint32 irq)
{
    uint32 arg = irq;
    int32 ret = 0;
    char nod_name[32];
    dal_intr_info_t dal_info;
    uint32 irq_in_kernel = 0;

    for(irq_in_kernel=0; irq_in_kernel < DAL_MAX_INTR_NUM; irq_in_kernel++)
    {
        if(irq == intr_handlers[irq_in_kernel].irq)
        {
            break;
        }
    }
    if(irq_in_kernel >= DAL_MAX_INTR_NUM)
    {
        DAL_DEBUG_DUMP("IRQ %d not register in user mode!!!\n", irq);
        return -1;
    }

    if(intr_handlers[irq_in_kernel].ref_cnt > 1)
    {
        intr_handlers[irq_in_kernel].ref_cnt--;
        return 0;
    }

    if (knet_enable)
    {
        ret = dal_usrctrl_do_cmd(CMD_DISCONNECT_INTERRUPTS, (uintptr)&irq);
        if (ret < 0)
        {
            DAL_DEBUG_DUMP("knet disconnect interrupt error, ret:%d\n", ret);
        }
    }

    sal_memset(&dal_info, 0, sizeof(dal_info));
    dal_info.irq = irq;
    ret = dal_usrctrl_do_cmd(CMD_GET_INTR_INFO, (uintptr)&dal_info);
    if (ret || (dal_info.irq_idx >= DAL_MAX_INTR_NUM))
    {
        DAL_DEBUG_DUMP("IRQ %d not register in kernel!!!\n", irq);
        return -1;
    }
    ret = dal_usrctrl_do_cmd(CMD_UNREG_INTERRUPTS, (uintptr) &arg);

    /* delete device node */
    sal_snprintf(nod_name, 32, "rm %s%d", DAL_INTR_DEV_NAME, irq_in_kernel);

    system(nod_name);

    /* destroy task */
    intr_handlers[irq_in_kernel].active = 0;
    sal_task_destroy(intr_handlers[irq_in_kernel].p_intr_thread);
    sal_memset(&(intr_handlers[irq_in_kernel]), 0, sizeof(intr_handler_t));

    return 0;
}

/*set interrupt enable */
int32
dal_interrupt_set_en(uint32 irq, uint32 enable)
{
    dal_intr_parm_t intr_para;
    int32 ret = 0;

    sal_memset(&intr_para, 0, sizeof(dal_intr_parm_t));
    intr_para.irq = irq;
    intr_para.enable = enable;
    ret = dal_usrctrl_do_cmd(CMD_EN_INTERRUPTS, (uintptr) & intr_para);

    return ret;
}

/*
type  1: msi
type  2: legacy
*/
int32  dal_interrupt_get_irq(uint8 lchip, uint8 type , uint16* irq_array, uint8* num)
{
    if(1 == type || g_dal_op.soc_active)
    {
        int8  loop;
        dal_msi_info_t msi_info;

        sal_memset(&msi_info, 0, sizeof(dal_msi_info_t));
        msi_info.lchip = dal_dev[lchip].lchip;

        dal_usrctrl_do_cmd(CMD_GET_MSI_INFO, (uintptr) &msi_info);

        for(loop=0;loop < msi_info.irq_num; loop++)
        {
            *(irq_array+loop) = msi_info.irq_base+loop;
        }
        *num = msi_info.irq_num;
    }
    else if(2 == type)
    {
        uint32 conf_value = 0;
        dal_pci_conf_read(lchip, 0x3c, &conf_value);
        *irq_array = conf_value&0xFF;
        *num = 1;
    }
    else
    {
        return -1;
    }

    return 0;
}
/* get dma information */
int32
dal_get_dma_info(unsigned int lchip, void* p_info)
{
    dal_dma_info_t *p_dma = (dal_dma_info_t*)p_info;

    sal_memcpy(p_dma, &g_dma_info[lchip], sizeof(dal_dma_info_t));

    return 0;
}

int32
dal_dma_chan_register(uint8 lchip, void* data)
{
    int32 ret = 0;

    ret = dal_usrctrl_do_cmd(CMD_REG_DMA_CHAN, (uintptr)data);
    if (ret && (ret != DAL_E_INVALID_FD))
    {
        DAL_DEBUG_DUMP("register dma chan fail, ret:%d\n", ret);
    }

    return ret;
}

int32
dal_handle_netif(uint8 lchip, void* data)
{
    int32 ret = 0;

    ret = dal_usrctrl_do_cmd(CMD_HANDLE_NETIF, (uintptr)data);
    if (ret < 0)
    {
        DAL_DEBUG_DUMP("operate netif fail, ret:%d\n", ret);
    }

    return ret;
}

/* convert logic address to physical, only using for DMA */
uint64
dal_logic_to_phy(uint8 lchip, void* laddr)
{
    uint64 pbase = 0;

    pbase = g_dma_info[lchip].phy_base_hi;
    pbase <<= 32;
    pbase |= g_dma_info[lchip].phy_base;

    if (g_dma_info[lchip].size)
    {
        /* dma memory is a contiguous block */
        if (laddr)
        {
            if(g_dal_op.soc_active)
            {
                return (pbase + ((uintptr)(laddr) - (uintptr)(g_dma_info[lchip].virt_base)) - 0x80000000);
            }
            else
            {
                return (pbase + ((uintptr)(laddr) - (uintptr)(g_dma_info[lchip].virt_base)));
            }
        }
    }

    return 0;
}

/* convert physical address to logical, only using for DMA */
void*
dal_phy_to_logic(uint8 lchip, uint64 paddr)
{
    uint64 pbase = 0;

    pbase = g_dma_info[lchip].phy_base_hi;
    pbase <<= 32;
    pbase |= g_dma_info[lchip].phy_base;

    if(g_dal_op.soc_active)
    {
        paddr += 0x80000000;
    }
    if (g_dma_info[lchip].size)
    {
        /* dma memory is a contiguous block */
        return (void*)(paddr ? (char*)g_dma_info[lchip].virt_base + (paddr - pbase) : NULL);
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
    dal_dma_cache_info_t intr_para;
    int32 ret = 0;

    sal_memset(&intr_para, 0, sizeof(dal_dma_cache_info_t));
    intr_para.ptr = (unsigned long)ptr;
    intr_para.length = length;
    ret = dal_usrctrl_do_cmd(CMD_CACHE_INVAL, (uintptr)&intr_para);

    return ret;
}

/*flush dma cache*/
int32
dal_dma_cache_flush(uint8 lchip, uint64 ptr, int32 length)
{
    dal_dma_cache_info_t intr_para;
    int32 ret = 0;

    sal_memset(&intr_para, 0, sizeof(dal_dma_cache_info_t));
    intr_para.ptr = (unsigned long)ptr;
    intr_para.length = length;
    ret = dal_usrctrl_do_cmd(CMD_CACHE_FLUSH, (uintptr)&intr_para);

    return ret;
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

/* address mmapping, this function using /dev/mem device for mapping  */
STATIC void *
dal_mmap(uintptr p, int size)
{
    void *map;
    unsigned int page_size, page_mask,offset;
    uintptr  paddr;
    uintptr  vmap;

    if (!dal_mem_fd)
    {
        return NULL;
    }

    page_size = getpagesize();
    page_mask = ~(page_size - 1);
    offset = 0;
    paddr = 0;
    vmap = 0;

/*User Space is 32 bits and kernel space is 64 bits, using this mode
   Now test on Mips ls2f platform need this mode */
#ifdef DAL_USE_MMAP2
    size += (p & ~page_mask);
    map = (void *)syscall(SYS_mmap2, 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dal_mem_fd, (off_t)((p & page_mask) >> 12));
    if (map == MAP_FAILED)
    {
        perror("mmap2 failed: ");
        map = NULL;
    }
    else
    {
        map = (void *)(((unsigned char *)map) + (p & ~page_mask));
    }
#else
    if (p & ~page_mask)
    {
        /*
        * If address (p) not aligned to page_size, we could not get the virtual
        * address. So we make the paddr aligned with the page size.
        * Get the _map by using the aligned paddr.
        * Add the offset back to return the virtual mapped region of p.
        */

        paddr = p & page_mask;
        offset = p - paddr;
        size += offset;
/*#ifdef PHYS_ADDR_IS_64BIT
        map = mmap64(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dal_mem_fd, paddr);
#else*/
        map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dal_mem_fd, paddr);
/*#endif*/
        if (map == MAP_FAILED)
        {
            perror("aligned mmap failed: ");
            map = NULL;
        }
        vmap = (uintptr)(map) + offset;

        return (void *)(vmap);
    }

/*#ifdef PHYS_ADDR_IS_64BIT
    map = mmap64(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dal_mem_fd, p);
#else*/
    map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dal_mem_fd, p);
/*#endif*/
    if (map == MAP_FAILED)
    {
        perror("mmap failed: ");
        map = NULL;
    }
#endif

    return map;
}

