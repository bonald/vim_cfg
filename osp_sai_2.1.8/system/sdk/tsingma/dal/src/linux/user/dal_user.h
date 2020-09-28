#ifndef _DAL_USER_H_
#define _DAL_USER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"

#define sal_poll        poll
#define sal_pollfd      pollfd

#define CMDPARA_ENCODE_CHIP(lchip_1, reg_addr_1, value_1, para_chip)  \
    para_chip.lchip   = lchip_1;                                      \
    para_chip.fpga_id = 0; \
    para_chip.reg_addr = reg_addr_1;                                  \
    para_chip.value = value_1

#define   CHECK_FD(fd)                    if (fd < 0) return DAL_E_INVALID_FD
#define CHECK_PTR(ptr)                  if (!ptr) return DAL_E_INVALID_PTR

#define DAL_I2C_WRITE_OP    0
#define DAL_I2C_READ_OP     1
#define DAL_ASIC_I2C_ALEN 1
#define DAL_ASIC_I2C_ADDR 0x3C
#define DAL_INTR_POLL_TIMEOUT  2000        /* [GB]ms */
#define DAL_I2C_RDWR        0x0707              /* [GB]Combined R/W transfer (one stop only)*/

#define DAL_INTR_DEV_NAME "/dev/dal_intr"  /* [GB] */

struct i2c_msg
{
    uint16 addr;
    uint16 flags;
    uint16 len;
    uint8* buf;
};

struct dal_i2c_ioctl_s
{
    struct i2c_msg* msgs;     /* pointers to i2c_msgs */
    int32 nmsgs;              /* number of i2c_msgs */
};
typedef struct dal_i2c_ioctl_s dal_i2c_ioctl_t;

struct intr_handler_s
{
    sal_task_t*  p_intr_thread;
    int32 irq;
    void* data;
    void (* handler)(void*);
    uint32 ref_cnt;
    uint8  active;
};
typedef struct intr_handler_s intr_handler_t;

struct dal_dma_finish_handler_s
{
    sal_task_t* p_dma_cb_task;
    dal_dma_finish_callback_t dma_finish_cb;
    uint8  active;
};
typedef struct dal_dma_finish_handler_s dal_dma_finish_handler_t;

struct dal_intr_para_s
{
    uint8 irq;
    int8 intr_idx;
    int16 prio;
};
typedef struct dal_intr_para_s dal_intr_para_t;

extern int32 dal_pci_read(uint8 lchip, uint32 offset, uint32* value);
extern int32 dal_pci_write(uint8 lchip, uint32 offset, uint32 value);
extern  int32 dal_pci_conf_read(uint8 lchip, uint32 offset, uint32* value);
extern int32 dal_pci_conf_write(uint8 lchip, uint32 offset, uint32 value);
extern int32 dal_i2c_read(uint8 lchip, uint16 offset, uint8 len, uint8* buf);
extern int32 dal_i2c_write(uint8 lchip, uint16 offset, uint8 len, uint8* buf);
extern int32 dal_interrupt_register(uint32 irq, int32 prio, void (* isr)(void*), void* data);
extern int32 dal_interrupt_unregister(uint32 irq);
extern int32 dal_interrupt_set_en(uint32 irq, uint32 enable);
extern uint64 dal_logic_to_phy(uint8 lchip, void* laddr);
extern void* dal_phy_to_logic(uint8 lchip, uint64 paddr);
extern uint32* dal_dma_alloc(uint8 lchip, int32 size, int32 type);
extern void  dal_dma_free(uint8 lchip, void* ptr);
extern int32 dal_dma_cache_inval(uint8 lchip, uint64 ptr, int32 length);
extern int32 dal_dma_cache_flush(uint8 lchip, uint64 ptr, int32 length);
extern int32 dal_usrctrl_do_cmd(uint32 cmd, uintptr p_para);
extern void dal_i2c_open();
extern int32 dal_interrupt_set_msi_cap(uint8 lchip, uint32 enable, uint32 num);
extern int32 dal_interrupt_get_msi_info(uint8 lchip, uint8* irq_base);
extern int32  dal_interrupt_get_irq(uint8 lchip, uint8 type , uint16* irq_array, uint8* num);
extern int32 dal_handle_netif(uint8 lchip, void* data);
#ifdef __cplusplus
}
#endif

#endif

