#ifndef __MEM_IO_H__
#define __MEM_IO_H__

#include "glb_macro.h"

#define PCIIO_WR_8(OFFSET,VAL) \
do {  \
    pci_io_write8((OFFSET), (VAL)); \
} while (0)

#define PCIIO_RD_8(OFFSET,VAL) \
do {  \
    int8 pci_val_8;              \
    pci_io_read8((OFFSET), &pci_val_8); \
    VAL = pci_val_8;          \
} while (0)

#define PCIIO_WR_16(OFFSET,VAL) \
do {  \
    pci_io_write16((OFFSET), (VAL)); \
} while (0)

#define PCIIO_RD_16(OFFSET,VAL) \
do {  \
    int16 pci_val_16;                \
    pci_io_read16((OFFSET), &pci_val_16); \
    VAL = pci_val_16;       \
} while (0)

#define PCIIO_WR_32(OFFSET,VAL) \
do {  \
    pci_io_write32((OFFSET), (VAL)); \
} while (0)

#define PCIIO_RD_32(OFFSET,VAL) \
do {  \
    int32 pci_val_32;                \
    pci_io_read32((OFFSET), &pci_val_32); \
    VAL = pci_val_32;       \
} while (0)

extern void *get_pci_base();

static inline void 
pci_io_write8(int32 offset, int8 value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    *(volatile uint8 *)(pci_base + offset) = value;
}

static inline void 
pci_io_read8(int32 offset, int8 *value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    *value = *(volatile uint8*)(pci_base + offset);
}

static inline void 
pci_io_write16(int32 offset, int16 value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    value = GLB_CTC_SWAP16(value);
    *(volatile uint16 *)(pci_base + offset) = value;    
}

static inline void 
pci_io_read16(int32 offset, int16 *value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    *value = *(volatile uint16*)(pci_base + offset);
    *value = GLB_CTC_SWAP16(*value);
}

static inline void 
pci_io_write32(int32 offset, int32 value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    value = GLB_CTC_SWAP32(value);
    *(volatile uint32 *)(pci_base + offset) = value;    
}

static inline void 
pci_io_read32(int32 offset, int32 *value)
{
    uintptr_t pci_base;

    pci_base = (uintptr_t)get_pci_base();
    *value = *(volatile uint32*)(pci_base + offset);
    *value = GLB_CTC_SWAP32(*value);
}

int ctc_hw_fd_init();
int ctc_phy_fd_init();
int ctc_reset_fd_init();
/* Added by liuht for bug 27657, 2014-03-25 */
int ctc_power_fd_init();
/* Merge from openflow by liuht for bug 26911, 2014-03-27 */
/* Added by liuht for bug 27412, 2014-03-27 */
int ctc_sys_led_fd_init();
/* End of Merge */
int pci_memio_init(uint8 is_fpga);
void *get_pci_base();
int32 get_ctc_hw_fd();
int32 get_ctc_phy_fd();
int32 get_ctc_reset_fd();
/* Added by liuht for bug 27657, 2014-03-25 */
int32 get_ctc_power_fd();
/* Merge from openflow by liuht for bug 26911, 2014-03-27 */
/* Added by liuht for bug 27412, 2014-03-27 */
int32 get_ctc_sys_led_fd();
/* End of Merge */
int32 get_ctc_dpll_fd();
int32 ctc_gpio_fd_init();
int32 get_ctc_gpio_fd();
int32 get_ctc_phy_stat(int32* phy_intr_stat);
int32 ctc_en_phy_irq(int32* irq_num);
int32 ctc_dpll_fd_init();
int32 ctc_en_dpll_irq(int32* irq_num);
int32 ctc_disable_all_irqs();

#endif /* !__MEM_IO_H__ */

