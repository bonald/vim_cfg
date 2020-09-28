/**
 @file dal_kernel_io.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-4-9

 @version v2.0

*/
#ifndef _DAL_KERNEL_H_
#define _DAL_KERNEL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal_types.h"

#if defined(CONFIG_RESOURCES_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)
#define PHYS_ADDR_IS_64BIT
#endif

#define DAL_PCI_READ_ADDR  0x0
#define DAL_PCI_READ_DATA  0xc
#define DAL_PCI_WRITE_ADDR 0x8
#define DAL_PCI_WRITE_DATA 0x4
#define DAL_PCI_STATUS     0x10

#define DAL_PCI_STATUS_IN_PROCESS      31
#define DAL_PCI_STATUS_BAD_PARITY      5
#define DAL_PCI_STATUS_CPU_ACCESS_ERR  4
#define DAL_PCI_STATUS_READ_CMD        3
#define DAL_PCI_STATUS_REGISTER_ERR    1
#define DAL_PCI_STATUS_REGISTER_ACK    0

#define DAL_PCI_ACCESS_TIMEOUT 0x64

#define DAL_NAME          "linux_dal"  /* "linux_dal" */

#define DAL_DEV_MAJOR     199
#define DAL_DEV_INTR_MAJOR_BASE     200

#define DAL_DEV_NAME      "/dev/" DAL_NAME
#define DAL_ONE_KB 1024
#define DAL_ONE_MB (1024*1024)
struct dal_chip_parm_s
{
    unsigned int lchip;     /*tmp should be uint8*/
    unsigned int fpga_id;     /*tmp add*/
    unsigned int reg_addr;
    unsigned int value;
};
typedef struct dal_chip_parm_s dal_chip_parm_t;

struct dal_intr_parm_s
{
    unsigned int irq;
    unsigned int enable;
};
typedef struct dal_intr_parm_s dal_intr_parm_t;

struct dal_user_dev_s
{
    unsigned int chip_num;   /*output: local chip number*/
    unsigned int lchip;       /*input: used to identify chip in kernel */
    unsigned int phy_base0; /* low 32bits physical base address */
    unsigned int phy_base1; /* high 32bits physical base address */
    void* virt_base[2];        /* Virtual base address */
    unsigned int bus_no;
    unsigned int dev_no;
    unsigned int fun_no;
};
typedef  struct dal_user_dev_s dal_user_dev_t;

struct dma_info_s
{
    unsigned int lchip;
    unsigned int phy_base;
    unsigned int phy_base_hi;
    unsigned int size;
    unsigned int* virt_base;
};
typedef struct dma_info_s dma_info_t;

struct dal_pci_cfg_ioctl_s
{
    unsigned int lchip;                      /* Device ID */
    unsigned int offset;
    unsigned int value;
};
typedef struct dal_pci_cfg_ioctl_s  dal_pci_cfg_ioctl_t;

struct dal_msi_info_s
{
    unsigned int irq_base;
    unsigned int irq_num;
};
typedef struct dal_msi_info_s dal_msi_info_t;

#define CMD_MAGIC 'C'
#define CMD_WRITE_CHIP              _IO(CMD_MAGIC, 0) /* for humber ioctrol*/
#define CMD_READ_CHIP               _IO(CMD_MAGIC, 1) /* for humber ioctrol*/
#define CMD_GET_DEVICES             _IO(CMD_MAGIC, 2)
#define CMD_PCI_CONFIG_WRITE        _IO(CMD_MAGIC, 3)
#define CMD_PCI_CONFIG_READ         _IO(CMD_MAGIC, 4)
#define CMD_GET_DMA_INFO            _IO(CMD_MAGIC, 5)
#define CMD_REG_INTERRUPTS          _IO(CMD_MAGIC, 6)
#define CMD_UNREG_INTERRUPTS        _IO(CMD_MAGIC, 7)
#define CMD_EN_INTERRUPTS           _IO(CMD_MAGIC, 8)
#define CMD_I2C_READ                _IO(CMD_MAGIC, 9)
#define CMD_I2C_WRITE               _IO(CMD_MAGIC, 10)
#define CMD_GET_MSI_INFO            _IO(CMD_MAGIC, 11)
#define CMD_SET_MSI_CAP             _IO(CMD_MAGIC, 12)

/* We try to assemble a contiguous segment from chunks of this size */
#define DMA_BLOCK_SIZE (512 * DAL_ONE_KB)

int dal_pci_read(unsigned char lchip, unsigned int offset, unsigned int* value);
int dal_pci_write(unsigned char lchip, unsigned int offset, unsigned int value);
int dal_pci_conf_read(unsigned char lchip, unsigned int offset, unsigned int* value);
int dal_pci_conf_write(unsigned char lchip, unsigned int offset, unsigned int value);
int dal_interrupt_register(unsigned int irq, int prio, void (* isr)(void*), void* data);
int dal_interrupt_unregister(unsigned int irq);
int dal_interrupt_set_en(unsigned int irq, unsigned int enable);
int dal_kernel_set_msi_enable(unsigned char lchip, unsigned int enable, unsigned int msi_num);
uint64 dal_logic_to_phy(unsigned char lchip, void* laddr);
void* dal_phy_to_logic(unsigned char lchip, uint64 paddr);
unsigned int* dal_dma_alloc(unsigned char lchip, int size, int type);
void dal_dma_free(unsigned char lchip, void* ptr);
int dal_dma_cache_inval(uint8 lchip, uint64 ptr, int length);
int dal_dma_cache_flush(uint8 lchip, uint64 ptr, int length);
int dal_interrupt_get_msi_info(unsigned char lchip, unsigned char* irq_base);
int dal_interrupt_get_irq(unsigned char lchip, unsigned char type , unsigned short* irq_array, unsigned char* num);
#ifdef __cplusplus
}
#endif

#endif

