/****************************************************************************
 * ctc_power.c :        for immap/ioctl/interrupt
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
#include <linux/interrupt.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_power.h"

extern ctc_board_t ctc_bd_info;

volatile void __iomem *epld_base_addr;
static int power_trigger;

static DECLARE_WAIT_QUEUE_HEAD(power_poll_interrupt); 

static irqreturn_t 
ctc_power_isr(int irq, void *dev_id)
{
    printk(KERN_ERR "ctc_power_isr %d", irq);
    disable_irq_nosync(irq);
    power_trigger = 1;
    wake_up(&power_poll_interrupt);
    return IRQ_HANDLED;
}

static unsigned int ctc_power_poll (struct file *filp, struct poll_table_struct *p)
{
    unsigned int mask = 0;

    unsigned long flags;
    poll_wait(filp, &power_poll_interrupt, p);
    local_irq_save(flags);
    if (power_trigger)
    {
        power_trigger = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    local_irq_restore(flags);

    return mask;
}

static struct file_operations power_fops = 
{
    .owner = THIS_MODULE, 
    .poll = ctc_power_poll,
};

static int __init ctc_power_init(void)
{
    int ret;

    if (ctc_bd_info.pwr_irq == 0)
    {
        return 0;
    }

    ret = register_chrdev(CTC_POWER_DEV_MAJOR, "ctc_power", &power_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the power character device", ret);
        return ret;
    } 	
    
    ret = request_irq(ctc_bd_info.pwr_irq, ctc_power_isr, 0, "ctc_power", ctc_power_isr);
    if (ret)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, requesting irq for power interrupt", ret);
    }

    return 0;
}

static void __exit ctc_power_exit(void)
{
    unregister_chrdev(CTC_POWER_DEV_MAJOR, "ctc_power");	
}

module_init(ctc_power_init);                                                                                                                   
module_exit(ctc_power_exit);                                                                                                                   

MODULE_LICENSE("GPL");

