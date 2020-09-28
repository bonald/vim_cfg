#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"

ctc_board_t ctc_bd_info;

#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))

static int board_series=GLB_SERIES_SG8348C;
module_param(board_series, int, S_IRUGO);
MODULE_PARM_DESC(board_series, "board series number");

static int board_type=GLB_BOARD_SG8348C;
module_param(board_type, int, S_IRUGO);
MODULE_PARM_DESC(board_type, "board type number");

static void set_normal_irq_info(void)
{
    if (IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_SG8348C, GLB_BOARD_SG8348C))
    {
        ctc_bd_info.normal_irq_count = 1;  
        ctc_bd_info.normal_irq_0 = CTC_HW_NORMAL_IRQ_0;  /* PHY intr */
        ctc_bd_info.normal_int0_offset = 0xff;       
        ctc_bd_info.normal_int0_mask_offset = 0xff;
    }
    else
    {
        ctc_bd_info.normal_irq_count = 0;  
    }
}

static void set_watchdog_info(void)
{
    ctc_bd_info.watchdog_support = CTC_WD_CFG; 
    ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_GPIO;
    ctc_bd_info.watchdog_en_gpio = CTC_WD_EN;  
    ctc_bd_info.watchdog_feed_gpio = CTC_WD_FEED;  
    ctc_bd_info.watchdog_en_reverse = 0;
}

static void set_reboot_info(void)
{ 
    ctc_bd_info.pre_reset_en = 0; 
    ctc_bd_info.epld_rst_reg = 0xff;
    ctc_bd_info.reboot_type = CTC_REBOOT_TYPE_POWEROFF;
 }

static void set_reset_irq_info(void)
{
    ctc_bd_info.rst_irq = 0;
 }

static void set_pwr_down_irq_info(void)
{
    ctc_bd_info.pwr_irq = 0;
 }

static void set_dpll_irq_info(void)
{
    ctc_bd_info.dpll_irq = 0;   
}

static void set_fast_reboot_info(void)
{
    ctc_bd_info.fast_boot_en = 0;
}

static void ctc_set_board_info(void)
{
    set_normal_irq_info();
    set_watchdog_info();
    set_reboot_info();
    set_reset_irq_info();
    set_pwr_down_irq_info();
    set_dpll_irq_info();
    set_fast_reboot_info();
}

static void ctc_ctl_board_led(void)
{
    ctc_bd_info.kernel_monitor_en = 1;
    ctc_bd_info.run_led_gpio = RUN_LED_GPIO_N;
    ctc_bd_info.alarm_led_gpio = ALARM_LED_GPIO_N;
}

static int __init ctc_boards_init(void)
{ 
    ctc_bd_info.board_series = board_series;
    ctc_bd_info.board_type   = board_type;

    ctc_set_board_info();
    ctc_ctl_board_led();
    
    return 0;
}

static void __exit ctc_boards_exit(void)
{
    printk(KERN_ERR "ctc_bd_info.board_series              %d\n", ctc_bd_info.board_series    );
    printk(KERN_ERR "ctc_bd_info.board_type                 %d\n", ctc_bd_info.board_type        );
    printk(KERN_ERR "ctc_bd_info.board_ver                   %d\n", ctc_bd_info.board_ver          );
    printk(KERN_ERR "ctc_bd_info.nor_type                     %d\n", ctc_bd_info.nor_type             );
    printk(KERN_ERR "ctc_bd_info.nand_type                  %d\n", ctc_bd_info.nand_type          );
    printk(KERN_ERR "ctc_bd_info.ddr_type                     %d\n", ctc_bd_info.ddr_type             );
    printk(KERN_ERR "ctc_bd_info.asic_chip_num          %d\n", ctc_bd_info.asic_chip_num  );
    printk(KERN_ERR "ctc_bd_info.normal_irq_count      %d\n", ctc_bd_info.normal_irq_count);
    printk(KERN_ERR "ctc_bd_info.normal_irq_0             %d\n", ctc_bd_info.normal_irq_0       );
    printk(KERN_ERR "ctc_bd_info.normal_irq_1             %d\n", ctc_bd_info.normal_irq_1       );
    printk(KERN_ERR "ctc_bd_info.normal_irq_2             %d\n", ctc_bd_info.normal_irq_2       );
    printk(KERN_ERR "ctc_bd_info.dpll_irq                       %d\n", ctc_bd_info.dpll_irq               );
    printk(KERN_ERR "ctc_bd_info.normal_int0_offset    %d\n", ctc_bd_info.normal_int0_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int0_mask_offset   %d\n", ctc_bd_info.normal_int0_mask_offset);
    printk(KERN_ERR "ctc_bd_info.normal_int1_offset        %d\n", ctc_bd_info.normal_int1_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int1_mask_offset   %d\n", ctc_bd_info.normal_int1_mask_offset);
    printk(KERN_ERR "ctc_bd_info.normal_int2_offset        %d\n", ctc_bd_info.normal_int2_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int2_mask_offset   %d\n", ctc_bd_info.normal_int2_mask_offset);
    printk(KERN_ERR "ctc_bd_info.dpll_int_offset              %d\n", ctc_bd_info.dpll_int_offset        );
    printk(KERN_ERR "ctc_bd_info.dpll_int_mask_offset  %d\n", ctc_bd_info.dpll_int_mask_offset   );
    printk(KERN_ERR "ctc_bd_info.watchdog_support      %d\n", ctc_bd_info.watchdog_support       );
    printk(KERN_ERR "ctc_bd_info.watchdog_en_type     %d\n", ctc_bd_info.watchdog_en_type       );
    printk(KERN_ERR "ctc_bd_info.watchdog_en_gpio     %d\n", ctc_bd_info.watchdog_en_gpio           );
    printk(KERN_ERR "ctc_bd_info.watchdog_feed_gpio %d\n", ctc_bd_info.watchdog_feed_gpio     );
    printk(KERN_ERR "ctc_bd_info.watchdog_is_feed  %d\n", ctc_bd_info.watchdog_is_feed       );
    printk(KERN_ERR "ctc_bd_info.gpio_date_offset    %d\n", ctc_bd_info.gpio_date_offset       );
    printk(KERN_ERR "ctc_bd_info.reboot_type             %d\n", ctc_bd_info.reboot_type            );
    printk(KERN_ERR "ctc_bd_info.pre_reset_en          %d\n", ctc_bd_info.pre_reset_en           );
    printk(KERN_ERR "ctc_bd_info.epld_rst_reg           %d\n", ctc_bd_info.epld_rst_reg           );
    printk(KERN_ERR "ctc_bd_info.led_offset                %d\n", ctc_bd_info.led_offset             );
    printk(KERN_ERR "ctc_bd_info.rst_irq                      %d\n", ctc_bd_info.rst_irq                );
    printk(KERN_ERR "ctc_bd_info.rst_int_mask          %d\n", ctc_bd_info.rst_int_mask           );
    printk(KERN_ERR "ctc_bd_info.rst_int_offset          %d\n", ctc_bd_info.rst_int_offset         );
    printk(KERN_ERR "ctc_bd_info.pwr_irq                   %d\n", ctc_bd_info.pwr_irq                );
}

EXPORT_SYMBOL(ctc_bd_info);

module_init(ctc_boards_init);                                                                                                                   
module_exit(ctc_boards_exit);                                                                                                                   

MODULE_LICENSE("GPL");
