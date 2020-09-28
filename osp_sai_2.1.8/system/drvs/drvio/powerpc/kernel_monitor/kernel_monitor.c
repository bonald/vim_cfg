/****************************************************************************
 * kernel_monitor.c :        for kernel monitor
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         caoj
 * Date           :         2014-01-16
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
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include "kernel_monitor.h"

#ifdef _CTC_LOONGSON_2F_
#include <asm/mach-loongson/ctc_ls2f.h>
#else
//#include <asm/ctc_platform.h>
#endif
#include "ctc_boards.h"

extern ctc_board_t ctc_bd_info;
void __iomem *epld_base_addr;

static struct task_struct *monitor_thread;
/* Added by liuht for bug 27412, 2014-03-27 */
static int g_sys_led_alarm = 0;

static uint monitor_led;
static u8 led_ctl_mask;

static void kernel_monitor_thread(void );

/* 
 * 
 */
static void led_ctl_write_mask(u8 led_ctl_status, u8 offset, u8 value, u8 mask)
{
    u8 orig_led_ctl_stat;
    orig_led_ctl_stat = ioread8(epld_base_addr + offset);

    value = ((value<<4)&0xff)|value;
    led_ctl_status = (value & mask) | (orig_led_ctl_stat & ~mask);
    
    iowrite8(led_ctl_status, epld_base_addr + offset);
}

static void start_kernel_monitor_thread(void)
{
	if (monitor_thread)
		return;
	monitor_thread = kthread_run(kernel_monitor_thread, NULL, "kernel_monitor");
	if (IS_ERR(monitor_thread)) {
		printk("Failed to create the kernel monitor thread\n");
		monitor_thread = NULL;
	}
}

static void stop_kernel_monitor_thread(void)
{
    u8 led_ctl = 0;
    
    if (monitor_thread) {
        kthread_stop(monitor_thread);
        monitor_thread = NULL;
        led_ctl = ioread8(epld_base_addr + monitor_led);      
        //led_ctl &= 0xF0;
        //iowrite8(led_ctl, epld_base_addr + monitor_led);
        led_ctl_write_mask(led_ctl, monitor_led, 0x0, led_ctl_mask);
    }
}

static void kernel_monitor_thread(void)
{
    u8 led_ctl = 0;
    printk("Kernel monitor scanning thread starting\n");
    allow_signal(SIGKILL);
    allow_signal(SIGSTOP);

    set_user_nice(current, 10);

    while (!kthread_should_stop()) {
        /* Modified by liuht for bug 27412, 2014-03-27 */
        /* set led on */
        led_ctl = ioread8(epld_base_addr + monitor_led); 
        if(g_sys_led_alarm)
        {
            //led_ctl &= 0x0f;
            //led_ctl |= 0xb0;
            led_ctl_write_mask(led_ctl, monitor_led, 0xb, led_ctl_mask);
        }
        else
        {
            //led_ctl &= 0x0f;
            //led_ctl |= 0x70; 
            led_ctl_write_mask(led_ctl, monitor_led, 0x7, led_ctl_mask);
        }
        //iowrite8(led_ctl, epld_base_addr + monitor_led);
        schedule_timeout_interruptible(HZ);
        /* set led off */
        led_ctl = ioread8(epld_base_addr + monitor_led);
        //led_ctl &= 0xf;
        //iowrite8(led_ctl, epld_base_addr + monitor_led); 
        led_ctl_write_mask(led_ctl, monitor_led, 0x0, led_ctl_mask);
        schedule_timeout_interruptible(HZ);
	}

    printk("Kernel monitor scanning thread ended\n");
}

/* Added by liuht for bug 27412, 2014-03-27 */
static long ctc_sys_led_ioctl (struct file* file,
                unsigned int cmd, unsigned long parameter)                 
{
    int val = 0;
    
    switch (cmd)
    {
        case CTC_LED_RDWR:
            get_user(val, (u32*)parameter);
            if(val)
            {
                g_sys_led_alarm = 1;
            }
            else
            {
                g_sys_led_alarm = 0;
            }
        default:
            break;
    }
    return 0;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_sys_led_ioctl,
#else
    .unlocked_ioctl = ctc_sys_led_ioctl, 
#endif    
};

static int __init kernel_monitor_init(void)
{
    /* Added by liuht for bug 27412, 2014-03-27 */
    int ret = 0;
    ret = register_chrdev(CTC_SYS_LED_DEV_MAJOR, "ctc_sys_led", &fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    epld_base_addr = ctc_bd_info.epld_logic_addr;   
    monitor_led = ctc_bd_info.led_offset;
    if (0x2 == monitor_led)
    {
        led_ctl_mask = 0xf0;
    }
    else if (0x5 == monitor_led)
    {
        led_ctl_mask = 0x0f;
    }
    printk(KERN_WARNING "kernel monitor init\n");
   
    start_kernel_monitor_thread();
    return 0;
}

static void __exit kernel_monitor_exit(void)
{
    /* Added by liuht for bug 27412, 2014-03-27 */
    unregister_chrdev(CTC_SYS_LED_DEV_MAJOR, "ctc_sys_led");
    stop_kernel_monitor_thread();
}


module_init(kernel_monitor_init);                                                                                                                   
module_exit(kernel_monitor_exit);                                                                                                                   
MODULE_LICENSE("GPL");

