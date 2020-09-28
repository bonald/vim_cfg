/****************************************************************************
 * ctc_hw.c :        for immap/ioctl/interrupt
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         caoj
 * Date           :         2011-05-05
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
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/vmstat.h>
#include "ctc_boards.h"
#include "ctc_hw.h"

extern ctc_board_t ctc_bd_info; 
static int trigger;
volatile ulong* pci_logic_addr;


static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); /*for supervisor card or linecard phy*/


extern atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];

static int memusage_proc(unsigned int *val)
{
    unsigned long flags;
    unsigned long freeram;
#define K(x) ((x) << (PAGE_SHIFT - 10))

    local_irq_save(flags);
    freeram = atomic_long_read(&vm_stat[NR_FREE_PAGES]);
    *val = K(freeram);
    local_irq_restore(flags);
    
    return 0;
}

static long ctc_hw_ioctl (struct file *file, 
            unsigned int cmd, unsigned long parameter) 
{
    int ret;
    unsigned int val, mask;

    switch (cmd)
    {
        case CTC_GET_MEM_FREE:
            memusage_proc(&val);
            ret = put_user(val, (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");

            return ret;
        default:
            break;
    }

    return 0;
} 

static unsigned int ctc_hw_poll (struct file *filp, struct poll_table_struct *p)
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
 
static struct file_operations hw_fops = 
{
    .owner = THIS_MODULE,
    .compat_ioctl = ctc_hw_ioctl,   
    .poll = ctc_hw_poll,
    .mmap = NULL
};

static int __init ctc_hw_init(void)
{
    int ret = 0;

    ret = register_chrdev(CTC_HW_DEV_MAJOR, "ctc_hw", &hw_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    return 0;
}

static void __exit ctc_hw_exit(void)
{
    unregister_chrdev(CTC_HW_DEV_MAJOR, "ctc_hw"); 
}

module_init(ctc_hw_init);                                                                                                                   
module_exit(ctc_hw_exit);                                                                                                                   

EXPORT_SYMBOL(pci_logic_addr);
MODULE_LICENSE("GPL");

