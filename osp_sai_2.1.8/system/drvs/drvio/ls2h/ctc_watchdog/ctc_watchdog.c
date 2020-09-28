/****************************************************************************
 * ctc_watchdog.c :        for immap/ioctl/interrupt
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R1.0
 * Author         :           liangf
 * Date           :            2019-07-10
 * Reason         :         First Create
 ****************************************************************************/
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
#include <linux/phy.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_watchdog.h"

extern ctc_board_t ctc_bd_info;
#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))
    
static struct timer_list feeddog_timer; 

void feeddog_timer_handle(unsigned long data)
{
    unsigned int val;
    unsigned int mask;
    /*Bug18151. Add flag to control watchdog feed. another control position is octeon_restart().*/
    if(ctc_bd_info.watchdog_is_feed)
    {
        val = ls2h_readl(LS2H_GPIO_OUT_REG);
        mask = 0x1<<(ctc_bd_info.watchdog_feed_gpio);
        if(val & mask)
        {
             ls2h_writel(val & (~mask), LS2H_GPIO_OUT_REG);   
        }
        else
        {
            ls2h_writel(val | mask, LS2H_GPIO_OUT_REG);  
        }

        mod_timer(&feeddog_timer, jiffies + HZ/2);    
    }
}

 static long ctc_watchdog_ioctl (struct file *file, 
            unsigned int cmd, unsigned long parameter) 
{
    unsigned int val, mask;

    switch (cmd)
    {
        case CTC_EN_WATCHDOG:
            /*now just humber demo&seoul g24eu don't support watchdog*/
            if(ctc_bd_info.watchdog_support)
            {
                /*modified by xgu for bug 14828 , 2011-05-09*/
                if(NULL != feeddog_timer.function)
                {
                    return 0;
                }
                
                /* feed dog before enable it */
                {
                    val = ls2h_readl(LS2H_GPIO_OUT_REG);
                    mask = 0x1<<(ctc_bd_info.watchdog_feed_gpio);
                    if(val & mask)
                    {
                         ls2h_writel(val & (~mask), LS2H_GPIO_OUT_REG);   
                    }
                    else
                    {
                        ls2h_writel(val | mask, LS2H_GPIO_OUT_REG);  
                    }
                }
                ctc_bd_info.watchdog_is_feed = 1;
                /* start feed dog timer */
                init_timer(&feeddog_timer);
                feeddog_timer.function = &feeddog_timer_handle;
                feeddog_timer.data = (unsigned long)0; 
                feeddog_timer.expires = jiffies + HZ/2;
                add_timer(&feeddog_timer);

                /* enable GPIO feed dog */
                val = ls2h_readl(LS2H_GPIO_OUT_REG);
                mask = 0x1<<(ctc_bd_info.watchdog_en_gpio);
                ls2h_writel(val & (~mask), LS2H_GPIO_OUT_REG);
            }            
            return 0;
        case CTC_DIS_WATCHDOG:
            /*now just humber demo&seoul g24eu don't support watchdog*/
            if(ctc_bd_info.watchdog_support)
            {
                /* disable GPIO feed dog while enable EPLD feed dog automatically */
                val = ls2h_readl(LS2H_GPIO_OUT_REG);
                mask = 0x1<<(ctc_bd_info.watchdog_en_gpio);
                ls2h_writel(val | mask, LS2H_GPIO_OUT_REG);

                /* stop feed dog timer */
                if(feeddog_timer.function != NULL)
                {
                    del_timer(&feeddog_timer);
                    feeddog_timer.function = NULL;
                }
            }                        
            return 0;
        case CTC_DIS_FEED_WATCHDOG:
            /*now just humber demo&seoul g24eu don't support watchdog*/
            if(ctc_bd_info.watchdog_support)
            {
                /* stop feed dog timer */
                if(feeddog_timer.function != NULL)
                {
                    del_timer(&feeddog_timer);
                    feeddog_timer.function = NULL;
                }
            }                        
            return 0;         
        default:
            break;
    }

    return 0;
} 

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .compat_ioctl = ctc_watchdog_ioctl,
};

static int __init ctc_watchdog_init(void)
{
    int ret;
    
    ret = register_chrdev(CTC_WD_DEV_MAJOR, "ctc_watchdog", &fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }   
    
    if(ctc_bd_info.watchdog_support)
    {
        memset(&feeddog_timer, 0, sizeof(feeddog_timer));
        init_timer(&feeddog_timer);
    }
    
    return 0;
}

static void __exit ctc_watchdog_exit(void)
{

}

module_init(ctc_watchdog_init); 
module_exit(ctc_watchdog_exit);
MODULE_LICENSE("GPL");

