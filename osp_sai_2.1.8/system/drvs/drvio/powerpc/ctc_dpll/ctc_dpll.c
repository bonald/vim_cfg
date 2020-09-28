/****************************************************************************
 * ctc_dpll.c :        for dpll handle
 *
 * Copyright (C) 2015 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         qianj
 * Date           :         2019-06-03
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
void __iomem *epld_base_addr;

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); 

static irqreturn_t 
ctc_dpll_int_isr(int irq, void *dev_id)
{
    u8 tmp_val = 0;
    tmp_val = ioread8(epld_base_addr + ctc_bd_info.dpll_int_offset);
    iowrite8(0xff, epld_base_addr + ctc_bd_info.dpll_int_mask_offset);
    trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
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
    .poll = ctc_dpll_poll,
};

static int __init ctc_dpll_init(void)
{
    int ret;
    epld_base_addr = ctc_bd_info.epld_logic_addr;        

    if (ctc_bd_info.dpll_support)
    {
        ret = register_chrdev(CTC_DPLL_DEV_MAJOR, "ctc_dpll", &fops);
        if (ret<0)
        {
            printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
            return ret;
        }

        ret = request_irq(ctc_bd_info.dpll_irq, ctc_dpll_int_isr, IRQF_DISABLED, "DPLL interrupt", ctc_dpll_int_isr);
        if (ret)
        {
            printk(KERN_WARNING "Sorry, requesting irq for normal 0 interrupt failed with %d\n", ret);
        }
    }
    return 0;
}

static void __exit ctc_dpll_exit(void)
{
    if(ctc_bd_info.dpll_support)
    {
        unregister_chrdev(CTC_DPLL_DEV_MAJOR, "ctc_dpll");
    }
}


module_init(ctc_dpll_init);                                                                                                                   
module_exit(ctc_dpll_exit);                                                                                                                   

MODULE_LICENSE("GPL");

