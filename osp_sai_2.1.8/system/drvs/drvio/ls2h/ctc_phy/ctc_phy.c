/****************************************************************************
 * ctc_phy.c :        for immap/ioctl/interrupt
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
#include <asm/mach-loongson2/ls2h/ls2h.h>
#include <asm/mach-loongson/loongson.h>
#include "ctc_phy.h"
#include "ctc_boards.h"

extern ctc_board_t ctc_bd_info;
void __iomem *epld_base_addr;
static int phy_trigger;

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
p_func ctc_normal_int_isr[CTC_PHY_MAX_INTR_NUM];
static uint normal_irq[CTC_PHY_MAX_INTR_NUM];
static char normal_irq_str[CTC_PHY_MAX_INTR_NUM][25] = 
{
    "Normal 0 interrupt",
};

u32 phy_int_val = 0xffffffff;
 
static uint normal_int0_offset;
static uint normal_int0_mask_offset;
static u8 normal_irq_count;
static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); 


static irqreturn_t 
ctc_normal_int0_isr(int irq, void *dev_id)
{
    disable_irq_nosync(irq);
    /*enable irq by pic of cpu, add by guhg for bug 38203,2016-04025*/
    phy_int_val = 0;
    phy_trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
} 

#ifdef CONFIG_COMPAT
 static long ctc_phy_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter) 
#else            
static int ctc_phy_ioctl (struct inode* inode, struct file* file,
                unsigned int cmd, unsigned long parameter)            
#endif         
{
    int ret;
    u32 val32;

    switch (cmd)
    {
        case RD_PERI_IRQ_STAT:
            ret = put_user(phy_int_val , (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");
            return ret;       
        case CTC_CMD_EN_INTERRUPTS:
            enable_irq(ctc_bd_info.normal_irq_0);            
            break;
        case CTC_RET_GPIO_PHY:
            /*CFG GPIO4, 0 as GPIO */
            val32 = ls2h_readl(LS2H_GPIO_OUT_REG);
            //GPIO4, Reset PHY, 0
            val32 &= ~0x10;
            ls2h_writel(val32, LS2H_GPIO_OUT_REG);
            //Release reset PHY, 1
            val32 |= 0x10;
            udelay(1000*200);
            ls2h_writel(val32, LS2H_GPIO_OUT_REG);
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
    if (phy_trigger)
    {
        phy_trigger = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    local_irq_restore(flags);
    
    return mask;
}

static struct file_operations phy_fops = 
{
    .owner = THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_phy_ioctl,
#else
    .ioctl = ctc_phy_ioctl,
#endif    
    .poll = ctc_phy_poll,
};

static int __init ctc_phy_init(void)
{
    int ret;
    int i;
    
    ret = register_chrdev(CTC_PHY_DEV_MAJOR, "ctc_phy", &phy_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    normal_int0_offset = ctc_bd_info.normal_int0_offset;
    normal_int0_mask_offset = ctc_bd_info.normal_int0_mask_offset;
    normal_irq_count = ctc_bd_info.normal_irq_count;
    ctc_normal_int_isr[0] = ctc_normal_int0_isr;
    normal_irq[0] = ctc_bd_info.normal_irq_0;
 
    for (i = 0; i < normal_irq_count; i++ )
    {
        ret = request_irq(normal_irq[i], ctc_normal_int_isr[i], IRQF_DISABLED, normal_irq_str[i], ctc_normal_int_isr[i]);
        if (ret)
        {
            printk(KERN_WARNING "Sorry, requesting irq for normal %d interrupt, failed with %d\n", i, ret);
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

