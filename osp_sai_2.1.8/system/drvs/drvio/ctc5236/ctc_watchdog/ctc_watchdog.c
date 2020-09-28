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
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/watchdog.h>

extern ctc_board_t ctc_bd_info;
struct file *watchdog_file = NULL;
static struct timer_list feeddog_timer;

static void feed_dog(void)
{
    if (watchdog_file != NULL)
    {
        int arg = 0;
        unsigned long argp = (unsigned long)&arg;
        
        mm_segment_t old_fs = get_fs();
        set_fs(KERNEL_DS);
        watchdog_file->f_op->unlocked_ioctl(watchdog_file, WDIOC_KEEPALIVE, argp);
        set_fs(old_fs);
    }
}

static void enable_dog(u8 enable)
{
    if (enable)
    {
        if (watchdog_file == NULL)
        {
            int timeout = 5;
            unsigned long argp = (unsigned long)&timeout;
            
            watchdog_file = filp_open("/dev/watchdog0", O_RDWR, 0);
            mm_segment_t old_fs = get_fs();
            set_fs(KERNEL_DS);
            watchdog_file->f_op->unlocked_ioctl(watchdog_file, WDIOC_SETTIMEOUT, argp);
            set_fs(old_fs);
        }
    }
    else
    {
        if (watchdog_file != NULL)
        {
            char buf[1] = { 'V' };
            loff_t pos = 0;
        
            mm_segment_t old_fs = get_fs();
            set_fs(KERNEL_DS);
            vfs_write(watchdog_file, buf, 1, &pos);
            set_fs(old_fs);
            filp_close(watchdog_file, 0);
            watchdog_file = NULL;
        }
    }
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
    
    enable_dog(1);
    ctc_bd_info.watchdog_is_feed = 1;
    start_feeddog_timer();
}

static void ctc_watchdog_disable(void)
{
    if(!ctc_bd_info.watchdog_support)
    {
        return;
    }
    
    delete_feeddog_timer();
    ctc_bd_info.watchdog_is_feed = 0;
    enable_dog(0);
}

static void ctc_disable_feed_dog(void)
{
    if(!ctc_bd_info.watchdog_support)
    {
        return;
    }
    
    ctc_bd_info.watchdog_is_feed = 0;
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

