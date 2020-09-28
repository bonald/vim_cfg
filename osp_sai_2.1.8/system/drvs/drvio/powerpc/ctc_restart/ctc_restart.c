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
#include <asm/machdep.h>
#include "ctc_boards.h"
#include "ctc_restart.h"

extern ctc_board_t ctc_bd_info;
extern struct machdep_calls ppc_md;

void ctc_restart(char *cmd)
{
	//u8 reset_ctl; 
    /* added by tongzb for bug 40791, 2016-09-18 */
    unsigned int val, mask;
	/* added by tongzb for bug 40719, 2016-09-12 */
	if(0xfe == in_8(ctc_bd_info.epld_logic_addr+ 0xf6))
	{
		printk(KERN_INFO "WRITE 0x08 means ABNORMAL reboot!\n");
		out_8(ctc_bd_info.epld_logic_addr+ 0xf6, 0x08);
	}

	local_irq_disable();
    
	//reset_ctl = 0xaa;
	//iowrite8(reset_ctl, ctc_bd_info.epld_logic_addr+ 0xf7);	
  
    /* modified by tongzb for bug 40791, 2016-09-18 */
    if(ctc_bd_info.reboot_type == CTC_REBOOT_TYPE_WD)
    {
        if(ctc_bd_info.pre_reset_en == 1 )
        {
            iowrite8(0x00, ctc_bd_info.epld_logic_addr+ctc_bd_info.epld_rst_reg);
            iowrite8(0xff, ctc_bd_info.epld_logic_addr+ctc_bd_info.epld_rst_reg);
        }
        if(ctc_bd_info.watchdog_en_type == CTC_WATCHDOG_GPIO)
        {
            val = in_be32(ctc_bd_info.gpio_logic_addr);
            mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;
            out_be32(ctc_bd_info.gpio_logic_addr, val | mask);
            val = in_be32(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
            mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;
            if(ctc_bd_info.watchdog_en_reverse == 0)
                out_be32(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
            else
                out_be32(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val |mask);
        }
        else if(ctc_bd_info.watchdog_en_type == CTC_WATCHDOG_EPLD)
        {
            iowrite8(0xff, ctc_bd_info.epld_logic_addr+ctc_bd_info.watchdog_en_io_offset);
        }
        ctc_bd_info.watchdog_is_feed = 0;
    }
    /* modified end by tongzb */
    else if (ctc_bd_info.reboot_type == CTC_REBOOT_TYPE_POWEROFF)
    {
        /* fix bug32975, reboot cannot powerdown bug, qicx, 2015-04-23 */
        iowrite8(0x0, ctc_bd_info.epld_logic_addr + 0x23);	
        mdelay(200); /* Delay 200ms */
        iowrite8(0x3, ctc_bd_info.epld_logic_addr + 0x23);	
    }

	while (1) ;
}

static int __init ctc_restart_init(void)
{ 
    ppc_md.restart = ctc_restart;

    return 0;
}

static void __exit ctc_restart_exit(void)
{

}

EXPORT_SYMBOL(ctc_restart);

module_init(ctc_restart_init);                                                                                                                   
module_exit(ctc_restart_exit);                                                                                                                   

MODULE_LICENSE("GPL");

