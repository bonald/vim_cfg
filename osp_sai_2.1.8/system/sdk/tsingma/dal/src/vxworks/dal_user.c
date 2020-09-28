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
#include <sys/mman.h>
#include <pci.h>
#include <intLib.h>
#include <cacheLib.h>
 /*-#include <ivPpc.h>*/
#include "sal.h"
#include "dal.h"
#include "dal_user.h"
#include "dal_mpool.h"

struct dma_info_s
{
    unsigned int phy_base;
    unsigned int size;
    unsigned int* virt_base;
};
typedef struct dma_info_s dma_info_t;

struct dev_identify_s
{
    uint32 vendor_id;
    uint32 device_id;
};
typedef struct dev_identify_s dev_identify_t;

/*****************************************************************************
 * defines
 *****************************************************************************/
#define CTC_VENDOR_VID 0xc001
#define CTC_GREATBELT_DEVICE_ID 0x03e8

#define PCI_BUS_NO_START        0
#define PCI_MAX_BUS         0x100
#define PCI_MAX_DEVICES     0x20
#define PCI_CONF_VENDOR_ID              0x00     /* Word @ Offset 0x00 */
#define PCI_CONF_DEVICE_ID              0x02     /* Word @ Offset 0x02 */
#define PCI_CONF_REVISION_ID            0x08     /* Byte @ Offset 0x08 */
#define PCI_CONF_COMMAND                0x04     /* Word @ Offset 0x04 */
#define PCI_CONF_BAR0                   0x10     /* DWORD @ Offset 0x10 */
#define PCI_CONF_BAR1                   0x14     /* DWORD @ Offset 0x14 */
#define PCI_CONF_BAR2                   0x18     /* DWORD @ Offset 0x18 */
#define PCI_CONF_BAR3                   0x1C     /* DWORD @ Offset 0x1C */
#define PCI_CONF_BAR4                   0x20     /* DWORD @ Offset 0x20 */
#define PCI_CONF_BAR5                   0x24     /* DWORD @ Offset 0x24 */

#define PCI_CONF_CAP_PTR                0x34     /* DWORD @ Offset 0x34 */
#define PCI_CONF_TRDY_TO                0x40     /* Byte @ Offset 0x40 */
#define PCI_CONF_EXT_CAPABILITY_PTR     0x100    /* DWORD @ Offset 0x100 */

#define PCI_CONF_BAR_TYPE_MASK          (3 << 1)
#define PCI_CONF_BAR_TYPE_64B           (2 << 1)

#define DAL_INTERRUPT_NUM 4
#define DAL_DMA_BUF_ALIGNMENT   256
#define DAL_ALIGN(p, a)     ((((uintptr)(p)) + (a) - 1) & ~((a) - 1))
/*****************************************************************************
 * typedef
 *****************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/
uint8 g_dal_debug_on = 0;
static dal_access_type_t g_dal_access = DAL_PCIE_MM;
dal_pci_dev_t  g_dal_pci_dev[DAL_MAX_CHIP_NUM];    /* PCI device type */
uint8 g_dal_init = 0;
uint32 g_mem_base[DAL_MAX_CHIP_NUM] = {0x80000000};
dal_intr_para_t g_intr_para[DAL_INTERRUPT_NUM];

static dev_identify_t dal_id_table[] =
{
    {0xc001, 0x03e8},  /*Greatbelt*/
    {0xcb10, 0xc010},  /*Goldengate*/
    {0xcb11, 0xc011},  /*Goldengate*/
    {0xcb10, 0x7148},  /*Duet2*/
    {0xcb10, 0x5236},  /*Tsingma*/
    {0, },
};

extern uint64 dal_logic_to_phy(uint8 lchip, void* laddr);
extern void*
dal_phy_to_logic(uint8 lchip, uint64 paddr);
extern uint32*
dal_dma_alloc(uint8 lchip, int32 size, int32 type);
extern void
dal_dma_free(uint8 lchip, void* ptr);
extern int32
dal_dma_cache_inval(uint8 lchip, uint64 ptr, int32 length);
extern int32
dal_dma_cache_flush(uint8 lchip, uint64 ptr, int32 length);


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
STATIC int32
dal_device_probe(uint32 vendor_id, uint32 device_id)
{
    uint8 index = 0;
    uint8 is_match = 0;

    for (index = 0; ;index++)
    {
        if (dal_id_table[index].vendor_id == 0)
        {
            break;
        }

        if ((dal_id_table[index].vendor_id == vendor_id) && (dal_id_table[index].vendor_id == vendor_id))
        {
            is_match = 1;
            break;
        }
    }

    return ((is_match)?1:0);
}

STATIC int32
dal_init_config_pci(uint8 lchip, uint32 bus_no, uint32 dev_no)
{
    uint32 cap_base = 0;
    uint32 value = 0;
    int32 ret = 0;
    uint16 len = 0;

    /* Configure PCI */
    ret += pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_CAP_PTR, &value);
    cap_base = value & 0xff;
    if (!cap_base)
    {
        /*
         * Set # retries to infinite.  Otherwise other devices using the bus
         * may monopolize it long enough for us to time out.
         */
        ret += pciConfigOutLong(bus_no, dev_no, 0, PCI_CONF_TRDY_TO, 0x0080);

    }
    else
    {
        while (cap_base)
        {
            ret += pciConfigInLong(bus_no, dev_no, 0, cap_base, &value);
            /*
             * PCIE spec 1.1 section 7.8.1
             * PCI-Express capability ID = 0x10
             * Locate PCI-E capability structure
             */
            if ((value & 0xff) != 0x10)
            {
                cap_base = (value >> 8) & 0xff;
                continue;
            }
            /*
             * PCIE spec 1.1 section 7.8.1
             * offset 0x04 : Device capabilities register
             * bit 2-0 : Max_payload_size supported
             * 000 - 128 Bytes max payload size
             * 001 - 256 Bytes max payload size
             * 010 - 512 Bytes max payload size
             * 011 - 1024 Bytes max payload size
             * 100 - 2048 Bytes max payload size
             * 101 - 4096 Bytes max payload size
             */
            ret += pciConfigInLong(bus_no, dev_no, 0, cap_base + 0x04, &value);
            len = value&0x07; /* PCI_EXP_DEVCAP */

            /*
             * Restrict Max_payload_size to 128
             */
            len = 0;

            /*
             * PCIE spec 1.1 section 7.8.4
             * offset 0x08 : Device control register
             * bit 4-4 : Enable relaxed ordering (Disable)
             * bit 5-7 : Max_payload_size        (256)
             * bit 12-14 : Max_Read_Request_Size (256)
             */
            ret += pciConfigInLong(bus_no, dev_no, 0, cap_base + 0x08, &value);

            /* Max_Payload_Size = 0 (128 bytes) */
            value &= ~0x00e0;
            value |= len << 5;

            /* Max_Read_Request_Size = 0 (128 bytes) */
            value &= ~0x7000;
            value |= len << 12;

            value &= ~0x0010; /* Enable Relex Ordering = 0 (disabled) */
            pciConfigOutLong(bus_no, dev_no, 0, cap_base + 0x08, value);
            break;
        }
    }

    return ret;
}

static int32
dal_init_pcie_resource(uintptr* pcie_base)
{
    uint32 venID = 0;
    uint32 devID = 0;
    uint32 revID = 0;
    uint16 bus_no = 0;
    uint8 dev_no = 0;
    uint32 baroff = PCI_CONF_BAR0;
    uint32 rval = 0;
    int32 ret = 0;
    uint8 lchip_num = 0;

    for (bus_no = PCI_BUS_NO_START; bus_no < PCI_MAX_BUS; bus_no++)
    {
        for (dev_no = 0; dev_no < PCI_MAX_DEVICES; dev_no++)
        {
            ret = pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_VENDOR_ID, &venID);
            if (venID == 0xffff)
            {
                /*continue;*/
                break;
            }

            ret += pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_REVISION_ID, &revID);

            devID = venID >> 16;
            venID = (venID & 0xffff);

            if (dal_device_probe(venID, devID))
            {
                if (devID == 0x5236)
                {
                    DAL_DEBUG_DUMP("use bar2 to config memory space\n");
                    baroff = PCI_CONF_BAR2;
                }

                ret += pciConfigOutLong(bus_no, dev_no, 0, baroff, 0xffffffff);
                ret += pciConfigInLong(bus_no, dev_no, 0, baroff, &rval);
                ret += pciConfigOutLong(bus_no, dev_no, 0, baroff, pcie_base[lchip_num]);
                if ((rval & PCI_CONF_BAR_TYPE_MASK) == PCI_CONF_BAR_TYPE_64B)
                {
                    ret += pciConfigOutLong(bus_no, dev_no, 0, PCI_CONF_BAR1, 0);
                }

                /*enable cmd register */
                ret += pciConfigOutLong(bus_no, dev_no, 0, PCI_CONF_COMMAND, 0x06);
                ret += dal_init_config_pci(0, bus_no, dev_no);
                lchip_num++;
                if (lchip_num > DAL_MAX_CHIP_NUM)
                {
                    return -1;
                }
            }
        }


        /* for vxworks */
        sal_task_sleep(1);

    }

    /* can not find centec device */
    if (lchip_num == 0)
    {
        return DAL_E_DEV_NOT_FOUND;
    }
    return ret;
}

STATIC int32
dal_init_one_pcie_resource(uintptr pcie_base, dal_pci_dev_t* p_dal_dev)
{
    uint32 venID = 0;
    uint32 devID = 0;
    uint32 revID = 0;
    uint16 bus_no = 0;
    uint8 dev_no = 0;
    uint32 baroff = PCI_CONF_BAR0;
    uint32 rval = 0;
    int32 ret = 0;
    uint8 lchip_num = 0;


    bus_no = p_dal_dev->busNo;
    dev_no = p_dal_dev->devNo;
    ret = pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_VENDOR_ID, &venID);
    if (venID == 0xffff)
    {
        return DAL_E_DEV_NOT_FOUND;
    }

    ret += pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_REVISION_ID, &revID);

    devID = venID >> 16;
    venID = (venID & 0xffff);

    if (dal_device_probe(venID, devID))
    {
        if (devID == 0x5236)
        {
            DAL_DEBUG_DUMP("use bar2 to config memory space\n");
            baroff = PCI_CONF_BAR2;
        }

        ret += pciConfigOutLong(bus_no, dev_no, 0, baroff, 0xffffffff);
        ret += pciConfigInLong(bus_no, dev_no, 0, baroff, &rval);
        ret += pciConfigOutLong(bus_no, dev_no, 0, baroff, pcie_base);
        if ((rval & PCI_CONF_BAR_TYPE_MASK) == PCI_CONF_BAR_TYPE_64B)
        {
            ret += pciConfigOutLong(bus_no, dev_no, 0, PCI_CONF_BAR1, 0);
        }

        /*enable cmd register */
        ret += pciConfigOutLong(bus_no, dev_no, 0, PCI_CONF_COMMAND, 0x06);
        ret += dal_init_config_pci(0, bus_no, dev_no);
        lchip_num++;
        if (lchip_num > DAL_MAX_CHIP_NUM)
        {
            return -1;
        }
    }

    return ret;
}

int32
dal_op_init(dal_op_t* p_dal_op)
{
    uint8 lchip = 0;
    dal_pci_dev_t pci_dev;
    uint32 temp = 0;

    if (1 == g_dal_init)
    {
        return 0;
    }

    sal_memset(&pci_dev, 0, sizeof(dal_pci_dev_t));

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
        }
        sal_memcpy(&pci_dev, &(p_dal_op->pci_dev), sizeof(dal_pci_dev_t));
    }

    if ((0 != pci_dev.busNo) || (0 != pci_dev.devNo) || (0 != pci_dev.funNo))
    {
        if ((NULL != p_dal_op) && p_dal_op->pcie_base_addr[lchip])
        {
            dal_init_one_pcie_resource(p_dal_op->pcie_base_addr[lchip], &(p_dal_op->pci_dev));
        }

        /* get pcie base address */
        dal_get_one_pci_membase(lchip, &pci_dev);

        /* software init */
        sal_memset(&g_intr_para[lchip], 0, sizeof(dal_intr_para_t));
    }
    else
    {
        /*if pcie_base_addr is not NULL means need sdk init pcie resource, otherwise user init */
        if ((NULL != p_dal_op) && p_dal_op->pcie_base_addr[0])
        {
            dal_init_pcie_resource(p_dal_op->pcie_base_addr);
        }

        /* get pcie base address */
        dal_get_pci_membase();

        /* software init */
        sal_memset(&g_intr_para, 0, sizeof(dal_intr_para_t)*DAL_INTERRUPT_NUM);

        g_dal_init = 1;
    }

    dal_pci_read(lchip, 0x48, &temp);
    if (((temp >> 8) & 0xffff) == 0x3412)
    {
        DAL_DEBUG_DUMP("@@@@@Little endian Cpu detected!!! \n");
        dal_pci_write(lchip, 0x48, 0xFFFFFFFF);
    }

    return DAL_E_NONE;
}

int32
dal_op_deinit(uint8 lchip)
{

    sal_memset(&g_intr_para[lchip], 0, sizeof(dal_intr_para_t));
    return DAL_E_NONE;
}

int32
dal_get_device_info(uint8 lchip, dal_pci_dev_t* pci_info)
{
    if (g_dal_init == 0)
    {
        return DAL_E_NOT_INIT;
    }

    if (pci_info == NULL)
    {
        return DAL_E_INVALID_PTR;
    }

    sal_memcpy(pci_info, &g_dal_pci_dev[lchip], sizeof(dal_pci_dev_t));

    return DAL_E_NONE;
}

int32
dal_get_pci_membase(void)
{
    uint32 venID = 0;
    uint32 devID = 0;
    uint32 revID = 0;
    dal_pci_dev_t dev;
    uint32 baroff = PCI_CONF_BAR0;
    uint8 lchip_num = 0;

    sal_memset(&dev, 0, sizeof(dal_pci_dev_t));

    for (dev.busNo = PCI_BUS_NO_START; dev.busNo < PCI_MAX_BUS; dev.busNo++)
    {

        for (dev.devNo = 0; dev.devNo < PCI_MAX_DEVICES; dev.devNo++)
        {
            pciConfigInLong(dev.busNo, dev.devNo, 0, PCI_CONF_VENDOR_ID, &venID);

            if (venID == 0xffff)
            {
                /*continue;*/
                break;
            }

             pciConfigInLong(dev.busNo, dev.devNo, 0, PCI_CONF_REVISION_ID, &revID);

            devID = venID >> 16;
            venID = (venID & 0xffff);

            if (dal_device_probe(venID, devID))
            {
                if (devID == 0x5236)
                {
                    DAL_DEBUG_DUMP("use bar2 to config memory space\n");
                    baroff = PCI_CONF_BAR2;
                }
                pciConfigInLong(dev.busNo, dev.devNo, 0, baroff, &g_mem_base[lchip_num]);
                DAL_DEBUG_DUMP("@@@@@Find centec device : devno:%d, devno:%d, membase:0x%x \n", dev.busNo, dev.devNo, g_mem_base[lchip_num]);
                sal_memcpy(&g_dal_pci_dev[lchip_num], &dev, sizeof(dal_pci_dev_t));
                lchip_num++;
                if (lchip_num > DAL_MAX_CHIP_NUM)
                {
                    return -1;
                }
            }
        }

        /* for vxworks */
        sal_task_sleep(1);
    }

    if (lchip_num == 0)
    {
        /* Not find centec device */
        DAL_DEBUG_DUMP("@@@@@Can not Find centec device \n");

        return DAL_E_DEV_NOT_FOUND;
    }

    return DAL_E_NONE;
}

int32
dal_get_one_pci_membase(uint8 lchip, dal_pci_dev_t* p_dal_dev)
{
    uint32 venID = 0;
    uint32 devID = 0;
    uint32 revID = 0;
    uint32 baroff = PCI_CONF_BAR0;

    pciConfigInLong(p_dal_dev->busNo, p_dal_dev->devNo, 0, PCI_CONF_VENDOR_ID, &venID);

    if (venID == 0xffff)
    {
        return DAL_E_DEV_NOT_FOUND;
    }

    pciConfigInLong(p_dal_dev->busNo, p_dal_dev->devNo, 0, PCI_CONF_REVISION_ID, &revID);

    devID = venID >> 16;
    venID = (venID & 0xffff);

    if (dal_device_probe(venID, devID))
    {
        if (devID == 0x5236)
        {
            DAL_DEBUG_DUMP("use bar2 to config memory space\n");
            baroff = PCI_CONF_BAR2;
        }
        pciConfigInLong(p_dal_dev->busNo, p_dal_dev->devNo, 0, baroff, &g_mem_base[lchip]);
        DAL_DEBUG_DUMP("@@@@@Find centec device : devno:%d, devno:%d, membase:0x%x \n", p_dal_dev->busNo, p_dal_dev->devNo, g_mem_base[lchip]);
        sal_memcpy(&g_dal_pci_dev[lchip], p_dal_dev, sizeof(dal_pci_dev_t));
    }

    return DAL_E_NONE;
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
    uint32 venID = 0;
    uint32 devID = 0;
    uint32 revID = 0;
    uint16 bus_no = 0;
    uint8 dev_no = 0;
    int32 ret = 0;
    uint8 lchip_num = 0;

    for (bus_no = PCI_BUS_NO_START; bus_no < PCI_MAX_BUS; bus_no++)
    {
        for (dev_no = 0; dev_no < PCI_MAX_DEVICES; dev_no++)
        {
            ret = pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_VENDOR_ID, &venID);
            if (venID == 0xffff)
            {
                /*continue;*/
                break;
            }

            ret += pciConfigInLong(bus_no, dev_no, 0, PCI_CONF_REVISION_ID, &revID);

            devID = venID >> 16;
            venID = (venID & 0xffff);

            if (dal_device_probe(venID, devID))
            {
                lchip_num++;
                if (lchip_num > DAL_MAX_CHIP_NUM)
                {
                    return -1;
                }
            }
        }

        /* for vxworks */
        sal_task_sleep(1);

    }

    *p_num = lchip_num;

    return ret;
}

/* get local chip device id */
int32
dal_get_chip_dev_id(uint8 lchip, uint32* dev_id)
{
    int32 ret = 0;

    *dev_id = g_dal_pci_dev[lchip].devNo;

    return ret;
}

/* convert logic address to physical, only using for DMA */
uint64
dal_logic_to_phy(uint8 lchip, void* laddr)
{
    return (uintptr)laddr;
}

/* convert physical address to logical, only using for DMA */
void*
dal_phy_to_logic(uint8 lchip, uint64 paddr)
{
    return (void*)(uintptr)paddr;
}

/* alloc dma memory from memory pool*/
uint32*
dal_dma_alloc(uint8 lchip, int32 size, int32 type)
{
    dal_mblock_t* block = NULL;
    uint32* user_data = NULL;

    lchip = lchip;
    type = type;

    block = cacheDmaMalloc(sizeof(dal_mblock_t)+ size + DAL_DMA_BUF_ALIGNMENT);
    if (NULL == block)
    {
        return NULL;
    }
    block->size = size;
    block->type = type;
    user_data = (uint32*)DAL_ALIGN(&block->user_data[0], DAL_DMA_BUF_ALIGNMENT);
    block->user_data_start = user_data;
    *(((uintptr*)user_data)-1) = (uintptr)block;

    return user_data;
}

/*free memory to memory pool*/
void
dal_dma_free(uint8 lchip, void* ptr)
{
    dal_mblock_t* block = NULL;

    lchip = lchip;

    block = (dal_mblock_t*)*(((uintptr*)ptr) -1);

    cacheDmaFree(block);

    return;
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

/* this interface is compatible for humber access */
int32
dal_usrctrl_read_chip(uint8 lchip, uint32 offset, uint32 p_value)
{
    int32 ret = 0;

    if (g_dal_op.pci_read)
    {
        ret = g_dal_op.pci_read(lchip, offset, (uint32*)p_value);
        if (ret < 0)
        {
            DAL_DEBUG_DUMP("dal_usrctrl_write_chip   write failed ,%d\n", ret);
            return ret;
        }
    }

    return 0;
}

/* pci read function */
int32
dal_pci_read(uint8 lchip, uint32 offset, uint32* value)
{

    DAL_DEBUG_OUT("reg_read 0x%x \n", offset);

    switch(g_dal_access)
    {
    case DAL_PCIE_MM:
        /* this mode is usual mode, for support mmap device access */
        *value = *(volatile uint32*)(g_mem_base[lchip] + offset);

        return 0;

    case DAL_PCI_IO:

        /* this mode is used for ioctrl, Humber will use */
        break;

    case DAL_SPECIAL_EMU:

        /* this mode is used for ioctrl,  Emulation will use */
        break;

    case DAL_SUPER_IF:

        /* this mode is special for some customer */
        return 0;

    default:
        return DAL_E_INVALID_ACCESS;
    }

    return 0;
}


/*pci write function */
int32
dal_pci_write(uint8 lchip, uint32 offset, uint32 value)
{

    DAL_DEBUG_OUT("reg_write 0x%x 0x%x \n", offset, value);
    switch (g_dal_access)
    {
    case DAL_PCIE_MM:
        /* this mode is usual mode, for support mmap device access */
        *(volatile uint32*)(g_mem_base[lchip] + offset) = value;
        return 0;

    case DAL_PCI_IO:
        /* this mode is used for ioctrl, Humber and Emulation will use */

        return 0;

    case DAL_SPECIAL_EMU:
        /* this mode is used for ioctrl, Humber and Emulation will use */
        return 0;

    case DAL_SUPER_IF:

        /* this mode is special for some customer */
        return 0;

    default:
        return DAL_E_INVALID_ACCESS;
    }

    return 0;
}


/* interface for read pci configuration space, usual using for debug */
int32
dal_pci_conf_read(uint8 lchip, uint32 offset, uint32* value)
{
    int32 ret = 0;

     ret = pciConfigInLong(g_dal_pci_dev[lchip].busNo, g_dal_pci_dev[lchip].devNo, g_dal_pci_dev[lchip].funNo, offset, value);

    return ret;
}

/* interface for write pci configuration space, usual using for debug */
int32
dal_pci_conf_write(uint8 lchip, uint32 offset, uint32 value)
{
    int32 ret = 0;

    ret = pciConfigOutLong(g_dal_pci_dev[lchip].busNo, g_dal_pci_dev[lchip].devNo, g_dal_pci_dev[lchip].funNo, offset, value);

    return ret;
}


/* create isr thread for sync, para is register interrupt index */
STATIC void
_dal_isr_thread(void* param)
{
    int32 ret = 0;
    uint32 grp_index = (uint32)param;
    uint32 irq = 0;

    while (1)
    {
        ret = sal_sem_take(g_intr_para[grp_index].p_isr_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }

        /* interrupt dispatch */
        if (g_intr_para[grp_index].p_dal_isr_cb)
        {
           irq = g_intr_para[grp_index].irq;
           g_intr_para[grp_index].p_dal_isr_cb((void*)&irq);
        }
    }

    return;
}

/* control interrupt by fpga */
void
_dal_interrupt_disable(uint32 irq)
{
    intDisable(irq);

    return;
}

/* interrupt isr function, para is group id which passed by sdk code */
void
_dal_isr_function(void *data)
{
    uint32 para = (uint32)data;
    uint32 irq = 0;

    if (para >= DAL_INTERRUPT_NUM)
    {
        return;
    }

    irq = g_intr_para[para].irq;

    /* disable interrupt, intDisable have no effect why???? */
    _dal_interrupt_disable(irq);

    /* according interrupt line release sem */
    sal_sem_give(g_intr_para[para].p_isr_sem);

    return;
}

/* control interrupt by fpga */
void
_dal_interrupt_enable(uint32 irq)
{

    intEnable(irq);

    return;
}

/* register interrupt */
int32
dal_interrupt_register(uint32 irq, int32 prio, void (* isr)(void*), void* data)
{
    int32 ret = 0;
    uint32 grp_idx = (uint32)data;

    if (grp_idx >= DAL_INTERRUPT_NUM)
    {
        DAL_DEBUG_DUMP("register interrupt group id exceed!!%d \n", grp_idx);
        return DAL_E_EXCEED_MAX;
    }
   /*-  logMsg(" dal_interrupt_register!!irq:%d,  data:%d\n", irq, grp_idx, 0,0,0,0);*/

    /* create sync sem for interrupt routing */
    ret = sal_sem_create(&g_intr_para[grp_idx].p_isr_sem, 0);
    if (ret < 0)
    {
        return ret;
    }

    /* create thread to sync sem */
    ret = sal_task_create(&g_intr_para[grp_idx].p_isr_thread, "ctcisrthread",
                          SAL_DEF_TASK_STACK_SIZE, prio, _dal_isr_thread, (void*)data);
    if (ret < 0)
    {
        return ret;
    }

    /* need modify according user hardware connect */
    g_intr_para[grp_idx].intr_line = irq;
    g_intr_para[grp_idx].irq = irq;

    /* register callback function for interrupt dispatch */
    g_intr_para[grp_idx].p_dal_isr_cb = isr;

    /* connect interrupt */
    if (intConnect((VOIDFUNCPTR *)(irq), (VOIDFUNCPTR)_dal_isr_function, (uint32) (data)))
    {
        /*- logMsg("connect interrupt failed!! \n", 0, 0, 0,0,0,0);*/
        return -1;
    }

    /* enable interrupt */
    if (intEnable(irq ) != 0)
    {
        /*- logMsg(" set interrupt enable failed!! \n", 0, 0, 0,0,0,0);*/
        return -1;
    }

    return ret;
}

int32 dal_interrupt_unregister(uint32 irq)
{
    uint32 irq_index = 0;

    for(irq_index=0; irq_index < DAL_MAX_INTR_NUM; irq_index++)
    {
        if(irq == g_intr_para[irq_index].irq)
        {
            break;
        }
    }
    if(irq_index >= DAL_MAX_INTR_NUM)
    {
        DAL_DEBUG_DUMP("IRQ %d not register in user mode!!!\n", irq);
        return -1;
    }

    intDisable(irq);
    sal_task_destroy(g_intr_para[irq_index].p_isr_thread);
    sal_sem_destroy(g_intr_para[irq_index].p_isr_sem);
    sal_memset(&(g_intr_para[irq_index]), 0, sizeof(dal_intr_para_t));

    return 0;
}

int32
dal_interrupt_set_en(uint32 irq, uint32 enable)
{
    enable?_dal_interrupt_enable(irq):_dal_interrupt_disable(irq);

    return 0;
}

int32
dal_i2c_read(uint8 lchip, uint16 offset, uint8 len, uint8* buf)
{

    return DAL_E_NONE;
}

int32
dal_i2c_write(uint8 lchip, uint16 offset, uint8 len, uint8* buf)
{

    return DAL_E_NONE;
}

/* get dma information */
int32
dal_get_dma_info(unsigned int lchip, void* p_info)
{
    return 0;
}

int32
dal_dma_debug_info(uint8 lchip)
{
    return 0;
}

int32
dal_dma_cache_inval(uint8 lchip, uint64 ptr, int32 length)
{
    return 0;
}

int32
dal_dma_cache_flush(uint8 lchip, uint64 ptr, int32 length)
{
    return 0;
}

