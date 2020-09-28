/**
 @file dal.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-4-9

 @version v2.0

*/
#ifndef _DAL_H_
#define _DAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "dal_common.h"

#define DAL_DEBUG_OUT(fmt, args...)            \
{ \
    if (g_dal_debug_on)  \
    {   \
        sal_printf(fmt, ##args);                        \
    }   \
}

#define DAL_DEBUG_DUMP(fmt, args...)            \
{ \
        sal_printf(fmt, ##args);                        \
}

#define DAL_HUMBER_DEVICE_ID     0x6048
#define DAL_GREATBELT_DEVICE_ID  0x03E8
#define DAL_GOLDENGATE_DEVICE_ID 0xC010
#define DAL_GOLDENGATE_DEVICE_ID1 0xC011
#define DAL_DUET2_DEVICE_ID      0x7148
#define DAL_TSINGMA_DEVICE_ID    0x5236
#define DAL_USW_DEVICE_ID        0x5a5a        /*dal virtual device id*/

typedef int32 (*dal_dma_finish_callback_t)(uint8 lchip);

struct dal_op_s
{
    int32   (* pci_read)(uint8 lchip, uint32 offset, uint32* value);
    int32   (* pci_write)(uint8 lchip, uint32 offset, uint32 value);
    int32   (* pci_conf_read)(uint8 lchip, uint32 offset, uint32* value);
    int32   (* pci_conf_write)(uint8 lchip, uint32 offset, uint32 value);
    int32   (* i2c_read)(uint8 lchip, uint16 offset, uint8 len, uint8* buf);
    int32   (* i2c_write)(uint8 lchip, uint16 offset, uint8 len, uint8* buf);
    int32   (* interrupt_register)(uint32 irq, int32 prio, void (*)(void*), void* data);
    int32   (* interrupt_unregister)(uint32 irq);
    int32   (* interrupt_set_en)(uint32 irq, uint32 enable);
    int32   (* interrupt_get_msi_info)(uint8 lchip, uint8* irq_base);
    int32   (* interrupt_set_msi_cap)(uint8 lchip, uint32 enable, uint32 num);
    uint64  (* logic_to_phy)(uint8 lchip, void* laddr);
    void* (* phy_to_logic)(uint8 lchip, uint64 paddr);
    uint32* (* dma_alloc)(uint8 lchip, int32 size, int32 dma_type);
    void    (* dma_free)(uint8 lchip, void* ptr);
    int32   (* dma_cache_inval)(uint8 lchip, uint64 ptr, int length);
    int32   (* dma_cache_flush)(uint8 lchip, uint64 ptr, int length);
    uintptr pcie_base_addr[DAL_MAX_CHIP_NUM]; /* Configure PCIe base address  in vxworks environment*/
    dal_pci_dev_t pci_dev;
    uint8  lchip;
    uint8  soc_active;
    int32   (* interrupt_get_irq)(uint8 lchip, uint8 type , uint16* irq_array, uint8* num);
    int32   (* handle_netif)(uint8 lchip, void* data);
};
typedef struct dal_op_s dal_op_t;

int32 dal_op_init(dal_op_t* dal_op);
int32 dal_op_deinit(uint8 lchip);
int32 dal_set_device_access_type(dal_access_type_t device_type);
int32 dal_get_device_access_type(dal_access_type_t* p_device_type);
int32 dal_dma_debug_info(uint8 lchip);
int32 dal_get_dma_info(unsigned int lchip, void* p_info);
int32 dal_create_irq_mapping(uint8 hw_irq, uint8* sw_irq);
int32 dal_get_chip_dev_id(uint8 lchip, uint32 * dev_id);
int32 dal_dma_chan_register(uint8 lchip, void* data);
int32 dal_dma_finish_cb_register(dal_dma_finish_callback_t func);

#ifdef __cplusplus
}
#endif

#endif

