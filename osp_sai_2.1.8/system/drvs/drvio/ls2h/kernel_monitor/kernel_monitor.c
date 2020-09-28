/****************************************************************************
 * kernel_monitor.c :        for ls2h kernel monitor
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.1
 * Author         :         liangf
 * Date           :         2016-03-28
 * Reason         :         First Create, bug37788
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
#include "ctc_boards.h"

#ifdef CTC_DEBUG
#ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_DEBUG "kernel_monitor: " fmt, ## args)
#else
     /* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

extern ctc_board_t ctc_bd_info;

static struct task_struct *monitor_thread;
/* Added by liuht for bug 27412, 2014-03-27 */
static int g_sys_led_alarm = -1;
static u8 monitor_run_led;
static u8 monitor_alarm_led;

static int kernel_monitor_thread(void *data);

void ctc_gpio_set_led_on(u8 gpio_pin)
{
    u32 value;
    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    PDEBUG("ctc_gpio_clr start: %d val %x\n", gpio_pin, value);    

    value &= ~(0x1 << gpio_pin);
    ls2h_writel(value, LS2H_GPIO_OUT_REG);

#ifdef CTC_DEBUG
    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    PDEBUG("ctc_gpio_clr end: %d val %x\n", gpio_pin, value);    
#endif   
}

void ctc_gpio_set_led_off(u8 gpio_pin)
{
    u32 value;
    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    PDEBUG("ctc_gpio_set start: %d val %x\n", gpio_pin, value);

    value |= 0x1 << gpio_pin; 
    ls2h_writel(value, LS2H_GPIO_OUT_REG);
    
#ifdef CTC_DEBUG
    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    PDEBUG("ctc_gpio_set end: %d val %x\n", gpio_pin, value);  
#endif     
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
    if (monitor_thread) {
        kthread_stop(monitor_thread);
        monitor_thread = NULL;
        ctc_gpio_set_led_off(monitor_run_led);
    }
}

static int kernel_monitor_thread(void *data)
{
    printk(KERN_EMERG "Kernel monitor scanning thread starting\n");
    allow_signal(SIGKILL);
    allow_signal(SIGSTOP);
    set_user_nice(current, 10);
    static unsigned int count = 0;
    while (!kthread_should_stop()) {
        count++;
        /*g_sys_led_alarm is equal to init vlaue -1 mean that the system is in initiation*/
        if(-1 == g_sys_led_alarm)
        {
            ctc_gpio_set_led_off(monitor_alarm_led);
            if(count % 2)
                ctc_gpio_set_led_on(monitor_run_led);
            else
                ctc_gpio_set_led_off(monitor_run_led);
        }
        else
        {
            if(0 == count % 8)
                ctc_gpio_set_led_on(monitor_run_led);
            else if(0 == count % 4)
                ctc_gpio_set_led_off(monitor_run_led);
            if(g_sys_led_alarm)
            {
                ctc_gpio_set_led_on(monitor_alarm_led);
            }
            else
            {
                ctc_gpio_set_led_off(monitor_alarm_led);
            }
        }
        schedule_timeout_interruptible(HZ/4);
    }
    printk("Kernel monitor scanning thread ended\n");
    return 0;
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
    int ret = 0;
    
    ret = register_chrdev(CTC_SYS_LED_DEV_MAJOR, "ctc_sys_led", &fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    if(ctc_bd_info.kernel_monitor_en)
    {
        monitor_run_led = ctc_bd_info.run_led_gpio;  
        monitor_alarm_led = ctc_bd_info.alarm_led_gpio;
        printk(KERN_INFO "kernel monitor init\n");
       
        start_kernel_monitor_thread();
    }

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

