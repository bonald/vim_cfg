/****************************************************************************
 * ctc_reset.c :        for immap/ioctl/interrupt
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
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_reset.h"

extern ctc_board_t ctc_bd_info;
volatile void __iomem *epld_base_addr;
static int reset_trigger;

#define CTC_RESET_IRQ_MASK  0x9

#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))

static DECLARE_WAIT_QUEUE_HEAD(reset_poll_interrupt); 

static irqreturn_t 
ctc_reset_isr(int irq, void *dev_id)
{
    disable_irq_nosync(irq);
    
    reset_trigger = 1;
    wake_up(&reset_poll_interrupt);
    return IRQ_HANDLED;
}   

static unsigned int ctc_reset_poll (struct file *filp, struct poll_table_struct *p)
{
    unsigned int mask = 0;
    unsigned long flags;
    
    poll_wait(filp, &reset_poll_interrupt, p);
    local_irq_save(flags);
    if (reset_trigger)
    {
        reset_trigger = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    local_irq_restore(flags);
    
    return mask;
}

static struct file_operations reset_fops = 
{
    .owner = THIS_MODULE,   
    .poll = ctc_reset_poll,
};

static int __init ctc_reset_init(void)
{

    int ret;

    if(ctc_bd_info.rst_irq == 0)
    {
        return 0;
    }

    ret = register_chrdev(CTC_RESET_DEV_MAJOR, "ctc_reset", &reset_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the reset character device", ret);
        return ret;
    } 
    ret = request_irq(ctc_bd_info.rst_irq, ctc_reset_isr, 0, "ctc_reset", ctc_reset_isr);
    if (ret)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, requesting irq for reset interrupt", ret);
    }

    return 0;
}

static void __exit ctc_reset_exit(void)
{
    unregister_chrdev(CTC_RESET_DEV_MAJOR, "ctc_reset");	
}


module_init(ctc_reset_init);                                                                                                                   
module_exit(ctc_reset_exit);                                                                                                                   

MODULE_LICENSE("GPL");
