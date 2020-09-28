/****************************************************************************
 * ctc_phy.c :        for immap/ioctl/interrupt
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
#ifdef _CTC_ARM_HI3535_
#include <mach/ctc_platform.h>
#else
//#include <asm/ctc_platform.h>
#endif
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_phy.h"

extern ctc_board_t ctc_bd_info;
void __iomem *epld_base_addr;
static int phy_trigger;

#define CTC_PHY_MAX_INTR_NUM 3

typedef irqreturn_t (*p_func) (int irq, void* dev_id);
p_func ctc_normal_int_isr[CTC_PHY_MAX_INTR_NUM];
static uint normal_irq[CTC_PHY_MAX_INTR_NUM];
static char normal_irq_str[CTC_PHY_MAX_INTR_NUM][25] = 
{
    "Normal 0 interrupt",
    "Normal 1 interrupt",
    "Normal 2 interrupt",
};

u32 phy_int_val = 0xffffffff;
 
static uint normal_int0_offset;
static uint normal_int0_mask_offset;
static uint normal_int1_offset;
static uint normal_int1_mask_offset;
static uint normal_int2_offset;
static uint normal_int2_mask_offset;
static u8 normal_irq_count;
static DECLARE_WAIT_QUEUE_HEAD(poll_interrupt); 

static irqreturn_t 
ctc_normal_int0_isr(int irq, void *dev_id)
{
    if(ctc_bd_info.normal_irq_type==1)
    {
        disable_irq_nosync(irq);
    }
    else
    {
        u8 tmp_val = 0;
        tmp_val = ioread8(epld_base_addr + normal_int0_offset);
        phy_int_val = (0xffff00 | tmp_val) & 0xffffff;
        iowrite8(0xff, epld_base_addr + normal_int0_mask_offset);
    }
    phy_trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}   

static irqreturn_t 
ctc_normal_int1_isr(int irq, void *dev_id)
{
    if(ctc_bd_info.normal_irq_type==1)
    {
        disable_irq_nosync(irq);
    }
    else
    {
        u16 tmp_val = 0;
        tmp_val = ioread8(epld_base_addr + normal_int1_offset);
        phy_int_val = (0xff00ff | (tmp_val<<8)) & 0xffffff;
        iowrite8(0xff, epld_base_addr + normal_int1_mask_offset);
    }
    phy_trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}   

static irqreturn_t 
ctc_normal_int2_isr(int irq, void *dev_id)
{
    if(ctc_bd_info.normal_irq_type==1)
    {
        disable_irq_nosync(irq);
    }
    else
    {
        u32 tmp_val = 0;
        tmp_val = ioread8(epld_base_addr + normal_int2_offset);
        phy_int_val = (0x00ffff | (tmp_val<<16)) & 0xffffff;
        iowrite8(0xff, epld_base_addr + normal_int2_mask_offset);
    }
    phy_trigger = 1;
    wake_up(&poll_interrupt);
    return IRQ_HANDLED;
}   


 static long ctc_phy_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter) 
 {
    int ret;

    switch (cmd)
    {
        case RD_PERI_IRQ_STAT:
            ret = put_user(phy_int_val , (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");
            return ret;

        case CTC_CMD_EN_INTERRUPTS:
            enable_irq(ctc_bd_info.normal_irq_0);
            return 0;
            
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
/* Modified by qicx, add kernel_ver_2_6 to support new kernel 3.8.13. */   
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
    int i;
    
    ret = register_chrdev(CTC_PHY_DEV_MAJOR, "ctc_phy", &phy_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }   	

    epld_base_addr = ctc_bd_info.epld_logic_addr;        
    printk(KERN_WARNING "EPLD base address is 0x%x, value 0x%x\n", (unsigned int)epld_base_addr, ioread8(epld_base_addr + 0xf1)); 
    normal_int0_offset = ctc_bd_info.normal_int0_offset;
    normal_int0_mask_offset = ctc_bd_info.normal_int0_mask_offset;
    normal_int1_offset = ctc_bd_info.normal_int1_offset;
    normal_int1_mask_offset = ctc_bd_info.normal_int1_mask_offset;
    normal_int2_offset = ctc_bd_info.normal_int2_offset;
    normal_int2_mask_offset = ctc_bd_info.normal_int2_mask_offset;

    normal_irq_count = ctc_bd_info.normal_irq_count;
    
    ctc_normal_int_isr[0] = ctc_normal_int0_isr;
    ctc_normal_int_isr[1] = ctc_normal_int1_isr;
    ctc_normal_int_isr[2] = ctc_normal_int2_isr;
    normal_irq[0] = ctc_bd_info.normal_irq_0;
    normal_irq[1] = ctc_bd_info.normal_irq_1;
    normal_irq[2] = ctc_bd_info.normal_irq_2;
 
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

