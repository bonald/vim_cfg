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
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_watchdog.h"
#include <linux/spinlock.h>

static volatile void __iomem *wd_gpio_logic_addr;

extern ctc_board_t ctc_bd_info;
extern spinlock_t ctc_gpio_g1_dir_lock;
extern spinlock_t ctc_gpio_g1_data_lock;
    
#define CTC_READL   ioread32be
#define CTC_WRITEL  iowrite32be

static struct timer_list feeddog_timer;

static void feed_dog(void)
{
    unsigned int val;
    unsigned int mask;

    spin_lock_irq(&ctc_gpio_g1_data_lock);
    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    mask = 1 << ctc_bd_info.watchdog_feed_gpio;
    if(val & mask)
    {
        CTC_WRITEL(val & (~mask), wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    }
    else
    {
        CTC_WRITEL(val | mask, wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    }

    /* 1 not work; >= 5 work; set udelay to 50 */
    udelay(50);
    spin_unlock_irq(&ctc_gpio_g1_data_lock);
}

static void feeddog_signal_init(void)
{
    unsigned int val;
    unsigned int mask;

    spin_lock_irq(&ctc_gpio_g1_dir_lock);
    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);
    mask = 1 << ctc_bd_info.watchdog_feed_gpio;
    
    CTC_WRITEL(val | mask, wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);	
    spin_unlock_irq(&ctc_gpio_g1_dir_lock);
    
    feed_dog();
}

static void enable_dog(u8 enable)
{
    unsigned int val;
    unsigned int mask;

    spin_lock_irq(&ctc_gpio_g1_data_lock);
    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    mask = 1 << ctc_bd_info.watchdog_en_gpio;
    if(enable)
    {
        CTC_WRITEL(val & (~mask), wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    }
    else
    {
        CTC_WRITEL(val | mask, wd_gpio_logic_addr + ctc_bd_info.gpio_date_offset);
    }
    spin_unlock_irq(&ctc_gpio_g1_data_lock);
}

static void dog_enable_signal_init(void)
{
    unsigned int val;
    unsigned int mask;

    spin_lock_irq(&ctc_gpio_g1_dir_lock);
    val = CTC_READL(wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);
    mask = 1 << ctc_bd_info.watchdog_en_gpio;
    CTC_WRITEL(val | mask, wd_gpio_logic_addr + ctc_bd_info.gpio_dir_offset);
    spin_unlock_irq(&ctc_gpio_g1_dir_lock);
    
    enable_dog(1);
}

static void feeddog_timer_handle(unsigned long data)
{
    if(ctc_bd_info.watchdog_is_feed)
    {
        feed_dog();
    }
    mod_timer(&feeddog_timer, jiffies + HZ/2);  
}

static void start_feeddog_timer(void)
{
    init_timer(&feeddog_timer);
    feeddog_timer.function = &feeddog_timer_handle;
    feeddog_timer.data = (unsigned long)0; 
    feeddog_timer.expires = jiffies + HZ/2;
    add_timer_on(&feeddog_timer,0);
    printk(KERN_WARNING "start feeddog_time on cpu : 0x%x\n",feeddog_timer.flags&TIMER_CPUMASK);
}

static void delete_feeddog_timer(void)
{
    if(feeddog_timer.function != NULL)
    {
        del_timer(&feeddog_timer);
        feeddog_timer.function = NULL;
    }
}

void ctc_watchdog_enable(void)
{
    if(!ctc_bd_info.watchdog_support)
    {
        return;
    }

    if(NULL != feeddog_timer.function)
    {
        return;
    }
    
    feeddog_signal_init();
    
    ctc_bd_info.watchdog_is_feed = 1;
    start_feeddog_timer();
    
    /* Added by liuht for bug 28365, 2014-04-27 */
    /* delay 20ms to ensure feed watchdog signal out before enable watchdog signal */
    mdelay(20);
    
    dog_enable_signal_init();
    
}

static void ctc_watchdog_disable(void)
{
    if(!ctc_bd_info.watchdog_support)
    {
        return;
    }
    
    enable_dog(0);
    delete_feeddog_timer();
}

static void ctc_disable_feed_dog(void)
{
    if(!ctc_bd_info.watchdog_support)
    {
        return;
    }
    
    delete_feeddog_timer();
}

 static long ctc_watchdog_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter) 
 {
    switch (cmd)
    {
    case CTC_EN_WATCHDOG:
        ctc_watchdog_enable();
        return 0;
    case CTC_DIS_WATCHDOG:
        ctc_watchdog_disable();
        return 0;
    case CTC_DIS_FEED_WATCHDOG:
        ctc_disable_feed_dog();
        return 0;
    }
    
    return 0;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .compat_ioctl = ctc_watchdog_ioctl,
    .unlocked_ioctl = ctc_watchdog_ioctl,
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
        wd_gpio_logic_addr = ctc_bd_info.gpio_g1_logic_addr; 
        memset(&feeddog_timer, 0, sizeof(feeddog_timer));
        init_timer(&feeddog_timer);
    }
    
    return 0;
}

static void __exit ctc_watchdog_exit(void)
{

}
EXPORT_SYMBOL(wd_gpio_logic_addr);

module_init(ctc_watchdog_init);                                                                                                                   
module_exit(ctc_watchdog_exit);                                                                                                                   

MODULE_LICENSE("GPL");

