#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include "sal_common.h"
#include "ctc_hw.h"
#include "ctc_reset.h"
#include "ctc_power.h"
#include "ctc_phy.h"
#include "ctc_watchdog.h"
#include "ctc_gpio.h"
#include "ctc_dpll.h"
#if (defined _CTC_FSCL_P1014_)||(defined _CTC_FSCL_P1010_)||(defined _CTC_LOONGSON_)
#include "kernel_monitor.h"
#endif

void* pci_mem_base;
int32 hw_fd = -1;
int32 phy_fd = -1;
int32 dpll_fd = -1;
int32 reset_fd = -1;
/* Added by liuht for bug 27657, 2014-03-25 */
int32 power_fd = -1;
int32 sys_led_fd = -1;
int32 gpio_fd = -1;

int32 ctc_hw_fd_init()
{
    hw_fd = open(CTC_HW_DEV_NAME, O_RDWR);
    return hw_fd;
}

int32 get_ctc_hw_fd()
{
    return hw_fd;
}

int32 
pci_memio_init(uint8 is_fpga)
{
    if (ctc_hw_fd_init() < 0)
    {
        return -1;
    }

    if(is_fpga)   
    {
        pci_mem_base = mmap(NULL, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, hw_fd, 0);
        if(pci_mem_base < 0)
        {
            return -1;
        }
    }
    
    return 0;
}

void*
get_pci_base()
{
    return pci_mem_base;
}

int32 ctc_phy_fd_init()
{
    phy_fd = open(CTC_PHY_DEV_NAME, O_RDWR);
    return phy_fd;
}

int32 get_ctc_phy_fd()
{
    return phy_fd;
}

int32
get_ctc_phy_stat(int32* phy_intr_stat)
{
    return ioctl(phy_fd, RD_PERI_IRQ_STAT, phy_intr_stat);   
}

int32
ctc_en_phy_irq(int32* irq_num)
{
    return ioctl(phy_fd, CTC_CMD_EN_INTERRUPTS, irq_num);   
}

int32 ctc_gpio_fd_init()
{
    gpio_fd = open(CTC_GPIO_DEV_NAME, O_RDWR);
    return gpio_fd;
}

int32 get_ctc_gpio_fd()
{
    return gpio_fd;
}

int32 ctc_reset_fd_init()
{
    reset_fd = open(CTC_RESET_DEV_NAME, O_RDWR);
    return reset_fd;
}

int32 get_ctc_reset_fd()
{
    return reset_fd;
}
/* Added by liuht for bug 27657, 2014-03-25 */
int32 ctc_power_fd_init()
{
    power_fd = open(CTC_POWER_DEV_NAME, O_RDWR);
    return power_fd;
}

int32 get_ctc_power_fd()
{
    return power_fd;
}

/* Merge from openflow by liuht for bug 26911, 2014-03-27 */
/* Added by liuht for bug 27412, 2014-03-27 */
int32 ctc_sys_led_fd_init()
{
#if (defined _CTC_FSCL_P1014_)||(defined _CTC_FSCL_P1010_) ||(defined _CTC_LOONGSON_)
    sys_led_fd = open(CTC_SYS_LED_DEV_NAME, O_RDWR);
#endif
    return sys_led_fd;
}

int32 get_ctc_sys_led_fd()
{
    return sys_led_fd;
}
/* End of Merge */

int32 ctc_dpll_fd_init()
{
    dpll_fd = open(CTC_DPLL_DEV_NAME, O_RDWR);
    return dpll_fd;
}

int32 get_ctc_dpll_fd()
{
    return dpll_fd;
}

int32
ctc_en_dpll_irq(int32* irq_num)
{
    return ioctl(dpll_fd, CTC_EN_GPIO_DPLL_INT, irq_num);   
}

int32 
ctc_disable_all_irqs()
{
return 0;
    return ioctl(hw_fd, CTC_DIS_ALL_IRQS, 0);
}  

