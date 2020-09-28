/****************************************************************************
 * ctc_dpll.c :        for dpll handle
 *
 * Copyright (C) 2019 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R1.0
 * Author         :         liangf
 * Date           :         2019-07-03
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
#include "ctc_dpll.h"

extern ctc_board_t ctc_bd_info;
static int trigger=0;

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); 

static irqreturn_t 
ctc_dpll_int_isr(int irq, void *dev_id)
{
    disable_irq_nosync(irq);
    trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}     
    
static long ctc_dpll_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter)       
{
    switch (cmd)
    {     
        case CTC_EN_GPIO_DPLL_INT:           
            enable_irq(ctc_bd_info.dpll_irq);            
            break;               
         default:
            break;
    }

    return 0;
}

static unsigned int ctc_dpll_poll (struct file *filp, struct poll_table_struct *p)
{
    unsigned int mask = 0;
    unsigned long flags;
    
    poll_wait(filp, &poll_interrupt, p);
    local_irq_save(flags);
    if (trigger)
    {
        trigger = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    local_irq_restore(flags);
    
    return mask;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
#ifdef _KERNEL_VER_2_6_
    #ifdef CONFIG_COMPAT
        .compat_ioctl = ctc_dpll_ioctl,
    #else
        .ioctl = ctc_dpll_ioctl,
    #endif
#else
    .unlocked_ioctl = ctc_dpll_ioctl,
#endif    
    .poll = ctc_dpll_poll,
};

static int __init ctc_dpll_init(void)
{ 
    int ret;
    
    if(ctc_bd_info.dpll_irq == 0)
    {
        return 0;
    }

    ret = register_chrdev(CTC_DPLL_DEV_MAJOR, "ctc_dpll", &fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    ret = request_irq(ctc_bd_info.dpll_irq, ctc_dpll_int_isr, 0, "DPLL interrupt", ctc_dpll_int_isr);
    if (ret)
    {
        printk(KERN_WARNING "Sorry, requesting irq for normal 0 interrupt failed with %d\n", ret);
    }
   
    return 0;
}

static void __exit ctc_dpll_exit(void)
{
    unregister_chrdev(CTC_DPLL_DEV_MAJOR, "ctc_dpll");
}

module_init(ctc_dpll_init);  
module_exit(ctc_dpll_exit); 
MODULE_LICENSE("GPL");

