#include <asm/types.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_restart.h"

#define CTC_EPLD_PWROFF_REG   0x23

#define CTC_READL   ioread32be
#define CTC_WRITEL  iowrite32be

extern ctc_board_t ctc_bd_info;

extern int ctc_i2c_read(int devAddr, int regAddr, int regLen, unsigned char *data,  int dataLen);
extern int ctc_i2c_write(int devAddr, int regAddr, int regLen, unsigned char data);
extern u8 ctc_epld_read(u8 reg);
extern u8 ctc_epld_write(u8 reg, u8 val);
extern volatile void __iomem *wd_gpio_logic_addr;

extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);

void ctc_arm_restart(enum reboot_mode mode, const char *cmd)
{
    unsigned int val;
    unsigned int mask;    
	if(0xfe == ctc_epld_read(0xf6))
	{
        /* for bug 48447, remove invalid log, the is not usefull on e550 board */
		//printk(KERN_INFO "WRITE 0x08 means ABNORMAL reboot!\n");
		ctc_epld_write(0xf6, 0x08);
	}
cpu_reset:
    ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x0);
    mdelay(200);
    ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x3);

    /* for bug 52303, use watchdog for ensure reboot,when get here, the reboot info is prepared */
    if(!ctc_bd_info.watchdog_support)
    {
        mdelay(1000);
        goto cpu_reset;
    }

    ctc_bd_info.watchdog_is_feed = 0;

    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);
    mask = 1 << ctc_bd_info.watchdog_en_gpio;
    CTC_WRITEL(val | mask, wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);

    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    mask = 1 << ctc_bd_info.watchdog_en_gpio;
    CTC_WRITEL(val & (~mask), wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);

    while(1);
}

static int __init ctc_restart_init(void)
{ 
    arm_pm_restart = ctc_arm_restart;

    return 0;
}

static void __exit ctc_restart_exit(void)
{

}

module_init(ctc_restart_init);                                                                                                                   
module_exit(ctc_restart_exit); 
MODULE_LICENSE("GPL");

