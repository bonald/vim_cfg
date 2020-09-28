#include <linux/sched.h>
#include <linux/module.h>
#include <asm/irq.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/phy.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <asm/reboot.h>

#include "ctc_boards.h"
#include "ctc_restart.h"

extern ctc_board_t ctc_bd_info;

#ifdef _CTC_LOONGSON_2H_
void ctc_restart(char *cmd)
{
    int tmp;

    /* Using Watchdog to reboot PMON, qicx, 20160202 */
    /* GPIO 5: enable watchdog
       GPIO 7: feed dog
     */
    /* step 1: set "gpiocfg" bit5 & bit7 as GPIO function, value is 0'b */
    tmp = ls2h_readl(LS2H_GPIO_CFG_REG);
    tmp &= 0xffffff5f;
    ls2h_writel(tmp, LS2H_GPIO_CFG_REG);
    
    /* step2: set "gpiooe" bit5 & bit7 as OUTPUT, value is 0'b */
    tmp = ls2h_readl(LS2H_GPIO_OE_REG);
    tmp &= 0xffffff5f;
    ls2h_writel(tmp, LS2H_GPIO_OE_REG);
    
    /* step3: set "gpioout" bit5 as LOW electric level */
    tmp = ls2h_readl(LS2H_GPIO_OUT_REG);
    tmp &= 0xffffffdf;
    ls2h_writel(tmp, LS2H_GPIO_OUT_REG);

    /* fix bug37918, disable feed watchdog.*/
    ctc_bd_info.watchdog_is_feed = 0;
    
    /*delay 2s for watchdog to reboot, otherwise cpu reset will disable gpio output and disable watchdog*/
    msleep(2*1000);

    while (1) {
        tmp = ls2h_readl(LS2H_RST_CNT_REG);
        tmp |= 1;
        ls2h_writel(tmp, LS2H_RST_CNT_REG);
        msleep(100);
    }   
}
#elif defined _CTC_LOONGSON_2K_
void ctc_restart(char *cmd)
{

}
#endif

static int __init ctc_restart_init(void)
{ 
#ifdef _CTC_LOONGSON_2H_
    _machine_restart = ctc_restart;
#elif defined _CTC_LOONGSON_2K_

#endif

    return 0;
}

static void __exit ctc_restart_exit(void)
{

}

EXPORT_SYMBOL(ctc_restart);

module_init(ctc_restart_init);
module_exit(ctc_restart_exit); 
MODULE_LICENSE("GPL");

