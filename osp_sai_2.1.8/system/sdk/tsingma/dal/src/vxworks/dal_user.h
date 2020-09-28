/**
 @file dal_kernel_io.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-4-9

 @version v2.0

*/
#ifndef _DAL_IO_H_
#define _DAL_IO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "dal.h"

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

typedef void (* DAL_ISR_CALLBACK_FUN_P)  (void*);

struct dal_intr_para_s
{
    sal_sem_t*  p_isr_sem;                  /* interrupt sem */
    sal_task_t*  p_isr_thread;             /* interrupt thread */
    int16 irq;                                 /* interrupt group id, interrupt module define */
    int16 intr_line;                             /* interrupt line, special using for FPGA ctl */
    DAL_ISR_CALLBACK_FUN_P p_dal_isr_cb;     /* interrupt process function */
};
typedef struct dal_intr_para_s dal_intr_para_t;

struct dal_mblock_s
{
    uint32* user_data_start;
    uint32 size;
    uint32 type;
    uintptr user_data[1];
};
typedef struct dal_mblock_s dal_mblock_t;

extern int32
dal_cfg_pci_membase(void);
extern int32
dal_pci_read(uint8 lchip, uint32 offset, uint32* value);
extern int32
dal_pci_write(uint8 lchip, uint32 offset, uint32 value);
extern int32
dal_pci_conf_read(uint8 lchip, uint32 offset, uint32* value);
extern int32
dal_pci_conf_write(uint8 lchip, uint32 offset, uint32 value);
extern int32
dal_interrupt_register(uint32 irq, int32 prio, void (* isr)(void*), void* data);
extern int32
dal_interrupt_unregister(uint32 irq);
extern int32
dal_interrupt_set_en(uint32 irq, uint32 enable);

extern int32
dal_get_pci_membase(void);
extern int32
dal_get_one_pci_membase(uint8 lchip, dal_pci_dev_t* p_dal_dev);
extern int32
dal_i2c_read(uint8 lchip, uint16 offset, uint8 len, uint8* buf);
extern int32
dal_i2c_write(uint8 lchip, uint16 offset, uint8 len, uint8* buf);

extern int32
pciConfigInLong(int bus_no, int dev_no, int func_no, int offset, uint32*value);
extern int32
pciConfigOutLong(int bus_no, int dev_no, int func_no, int offset, uint32 value);
extern int32
intConnect(VOIDFUNCPTR *irq, VOIDFUNCPTR routine, int32 data);

#ifdef __cplusplus
}
#endif

#endif

