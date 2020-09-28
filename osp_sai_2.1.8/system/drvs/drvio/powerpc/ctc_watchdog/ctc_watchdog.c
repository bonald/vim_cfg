/****************************************************************************
 * ctc_watchdog.c :        for immap/ioctl/interrupt
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         liuht
 * Date           :         2014-12-25
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
//#ifdef _CTC_ARM_HI3535_
//#include <mach/ctc_platform.h>
//#else
//#include <asm/ctc_platform.h>
//#endif
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_watchdog.h"

extern ctc_board_t ctc_bd_info;
void __iomem *epld_base_addr;

#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))
    
#if defined _CTC_ARM_HI3535_
#define CTC_READW   __raw_readl
#define CTC_WRITEW  __raw_writel
#elif defined _CTC_X86_64_
/* added by kcao for X86TODO */
#define CTC_READW
#define CTC_WRITEW
#else
#define CTC_READW in_be32
#define CTC_WRITEW  out_be32
#endif

static struct timer_list feeddog_timer;
#define WATCHDOG_DYING_TIME_COUNT      120    /* Time = 0.5s*Count */ 
static u32 g_dying_watchdog_cnt=0;

void feeddog_timer_handle(unsigned long data)
{
    /* Added by liuht for bug 27693, 2014-03-31 */
    unsigned int val;
    unsigned int mask;

    /*Bug18151. Add flag to control watchdog feed. another control position is octeon_restart().*/
    if(ctc_bd_info.watchdog_is_feed)
    {
        val = CTC_READW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
        mask = 0x80000000 >> ctc_bd_info.watchdog_feed_gpio;
        if(val & mask)
            CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
        else
            CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val | mask);

        mod_timer(&feeddog_timer, jiffies + HZ/2);  
    }
    /*modified by jqiu for bug46425, 2018-02-12*/
    if(g_dying_watchdog_cnt)
    {
        g_dying_watchdog_cnt--;
        
        if(g_dying_watchdog_cnt == 0)
        {
            /*power down the whole board*/
            out_8(ctc_bd_info.epld_logic_addr+0x23 , 0);
            mdelay(200); /* Delay 200ms */
            out_8(ctc_bd_info.epld_logic_addr+0x23 , 3);
            
            ctc_bd_info.watchdog_is_feed = 0;

            val = CTC_READW(ctc_bd_info.gpio_logic_addr);
            mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;
            CTC_WRITEW(ctc_bd_info.gpio_logic_addr, val | mask);
            val = CTC_READW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
            mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;

            if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4XG)
              ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_48P4X)
              ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_24P4X)
              ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_HF_12T4S8X))
            {
                CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val | mask);
            }
            else
            {
                CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
            }
        }
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
                /* Added by liuht for bug 27693, 2014-03-31 */
                val = CTC_READW(ctc_bd_info.gpio_logic_addr);
                mask = 0x80000000 >> ctc_bd_info.watchdog_feed_gpio;
                CTC_WRITEW(ctc_bd_info.gpio_logic_addr , val | mask);

                /* feed dog before enable it */
                {
                    val = CTC_READW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
                    mask = 0x80000000 >> ctc_bd_info.watchdog_feed_gpio;
                    if(val & mask)
                        CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
                    else
                        CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val | mask);
                }
                ctc_bd_info.watchdog_is_feed = 1;
                /* start feed dog timer */
                init_timer(&feeddog_timer);
                feeddog_timer.function = &feeddog_timer_handle;
                feeddog_timer.data = (unsigned long)0; 
                feeddog_timer.expires = jiffies + HZ/2;
                add_timer(&feeddog_timer);
                /* Added by liuht for bug 28365, 2014-04-27 */
                /* delay 20ms to ensure feed watchdog signal out before enable watchdog signal */
                mdelay(20);
                /* enable GPIO feed dog */
                /* Added by liuht for bug 27693, 2014-03-31 */
                val = CTC_READW(ctc_bd_info.gpio_logic_addr);
                mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;
                CTC_WRITEW(ctc_bd_info.gpio_logic_addr, val | mask);
                val = CTC_READW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
                mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;

                if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4XG)
                   ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_48P4X)
                   ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_24P4X)
                   ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_HF_12T4S8X))
                {
                    CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val | mask);
                }
                else
                {
                    CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
                }
            }
            
            return 0;
        case CTC_DIS_WATCHDOG:
            if(ctc_bd_info.watchdog_support)
            {
                /* disable GPIO feed dog while enable EPLD feed dog automatically */
                val = CTC_READW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset);
                mask = 0x80000000 >> ctc_bd_info.watchdog_en_io_offset;
                if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4XG)
                  ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_48P4X)
                  ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_24P4X)
                  ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_HF_12T4S8X))
                {
                    CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val & (~mask));
                }
                else
                {

                    CTC_WRITEW(ctc_bd_info.gpio_logic_addr + ctc_bd_info.gpio_date_offset, val | mask);
                }

                /* stop feed dog timer */
                if(feeddog_timer.function != NULL)
                {
                    del_timer(&feeddog_timer);
                    feeddog_timer.function = NULL;
                }
            }            
            
            return 0;
        case CTC_DIS_FEED_WATCHDOG:
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
         /*modified by jqiu for bug46425, 2018-02-12*/   
         case CTC_DYING_WATCHDOG:
            g_dying_watchdog_cnt = WATCHDOG_DYING_TIME_COUNT;
            return 0;

         default:
            break;
    }

    return 0;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
/* Modified by qicx, add kernel_ver_2_6 to support new kernel 3.8.13. */   
#ifdef _KERNEL_VER_2_6_
    #ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_watchdog_ioctl,
    #else
     .ioctl = ctc_watchdog_ioctl,
    #endif
#else
    .unlocked_ioctl = ctc_watchdog_ioctl,
#endif
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

