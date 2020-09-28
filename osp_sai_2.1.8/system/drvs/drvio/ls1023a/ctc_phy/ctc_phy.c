/****************************************************************************
 * ctc_phy.c :        for phy handle
 *
 * Copyright (C) 2015 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         liuht
 * Date           :         2015-10-21
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
#include "ctc_phy.h"

extern ctc_board_t ctc_bd_info;
static int trigger=0;

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
p_func ctc_normal_int_isr[CTC_PHY_MAX_INTR_NUM];
static uint normal_irq[CTC_PHY_MAX_INTR_NUM];
static char normal_irq_str[CTC_PHY_MAX_INTR_NUM][25] = 
{
    "PHY interrupt1",
    "PHY interrupt2",
};
static int depth_intr0=0;
static int depth_intr1=0;

static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); 

static irqreturn_t 
ctc_normal_int0_isr(int irq, void *dev_id)
{
    disable_irq_nosync(irq);
    trigger = 1;
    depth_intr0 = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}   

static irqreturn_t 
ctc_normal_int1_isr(int irq, void *dev_id)
{
    disable_irq_nosync(irq);
    trigger = 1;
    depth_intr1 = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}   

 static long ctc_phy_ioctl(struct file *file, 
             unsigned int cmd, unsigned long parameter)        
{
    int ret = 0;
    switch (cmd)
    {     
        case RD_PERI_IRQ_STAT:
            ret = put_user(0 , (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");
            return ret;        
        case CTC_CMD_EN_INTERRUPTS: 
            if (depth_intr0)
            {
                depth_intr0 = 0;
                enable_irq(ctc_bd_info.normal_irq_0);
            }
            if (depth_intr1)
            {
                depth_intr1 = 0;
                enable_irq(ctc_bd_info.normal_irq_1);
            }
            break;
         default:
            break;
    }
    return 0;
}

static unsigned int ctc_phy_poll (struct file *filp, struct poll_table_struct *p)
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
        .compat_ioctl = ctc_phy_ioctl,
    #else
        .ioctl = ctc_phy_ioctl,
    #endif
#else
    .unlocked_ioctl = ctc_phy_ioctl,
#endif    
    .poll = ctc_phy_poll,
};

static int __init ctc_phy_init(void)
{
    int ret;
    
    ret = register_chrdev(CTC_PHY_DEV_MAJOR, "ctc_phy", &fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }
    
    if(ctc_bd_info.normal_irq_count == 1)
    {
        normal_irq[0]         = ctc_bd_info.normal_irq_0;
        ctc_normal_int_isr[0] = ctc_normal_int0_isr;

        ret = request_irq(normal_irq[0], ctc_normal_int_isr[0], 0, normal_irq_str[0], ctc_normal_int_isr[0]);
        if (ret)
        {
            printk(KERN_WARNING "Sorry, requesting irq %d for normal 0 interrupt failed with %d\n",normal_irq[0], ret);
        }
    }
    else if (ctc_bd_info.normal_irq_count == 2)
    {
        normal_irq[0]         = ctc_bd_info.normal_irq_0;
        ctc_normal_int_isr[0] = ctc_normal_int0_isr;

        ret = request_irq(normal_irq[0], ctc_normal_int_isr[0], 0, normal_irq_str[0], ctc_normal_int_isr[0]);
        if (ret)
        {
            printk(KERN_WARNING "Sorry, requesting irq %d for normal 0 interrupt failed with %d\n",normal_irq[0], ret);
        }

        normal_irq[1]         = ctc_bd_info.normal_irq_1;
        ctc_normal_int_isr[1] = ctc_normal_int1_isr;

        ret = request_irq(normal_irq[1], ctc_normal_int_isr[1], 0, normal_irq_str[1], ctc_normal_int_isr[1]);
        if (ret)
        {
            printk(KERN_WARNING "Sorry, requesting irq %d for normal 1 interrupt failed with %d\n",normal_irq[1], ret);
        }
    }

    return 0;
}

static void __exit ctc_phy_exit(void)
{
    unregister_chrdev(CTC_PHY_DEV_MAJOR, "ctc_phy");
}


module_init(ctc_phy_init);                                                                                                                   
module_exit(ctc_phy_exit);                                                                                                                   

MODULE_LICENSE("GPL");

