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
#ifdef _CTC_ARM_HI3535_
#include <mach/ctc_platform.h>
#else
//#include <asm/ctc_platform.h>
#endif
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_power.h"

extern ctc_board_t ctc_bd_info;

void __iomem *epld_base_addr;
static int power_trigger;

#define CTC_POWER_INTR_NUM      18       /* For E350-48T4XG2Q/E350-48T4XG/E350-24T4XG */
#define CTC_POWER_INTR_MASK     0xa      /* For E350-48T4XG2Q/E350-48T4XG/E350-24T4XG */

#define CTC_POWER_INTR_NUM2     19       /* For E350-8TS12X */
#define CTC_POWER_INTR_MASK2    0x15     /* For E350-8TS12X */

#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))

static DECLARE_WAIT_QUEUE_HEAD(power_poll_interrupt); 

static irqreturn_t 
ctc_power_isr(int irq, void *dev_id)
{
    u8 tmp_val = 0;
    if ((GLB_BOARD_E350_48T4X2Q == ctc_bd_info.board_type)
        || (GLB_BOARD_E350_48T4XG == ctc_bd_info.board_type)
        || (GLB_BOARD_E350_24T4XG == ctc_bd_info.board_type)
        || (GLB_BOARD_V350_48P4X == ctc_bd_info.board_type)
        || (GLB_BOARD_V350_24P4X == ctc_bd_info.board_type))
    {
        tmp_val = ioread8(epld_base_addr + CTC_POWER_INTR_MASK);
        tmp_val |= (1<<3);
        iowrite8(tmp_val, epld_base_addr + CTC_POWER_INTR_MASK);
    }
    else if ((GLB_BOARD_E350_8TS12X == ctc_bd_info.board_type)
        ||(GLB_BOARD_E350_65DV1 == ctc_bd_info.board_type))
    {
        iowrite8(0x1, epld_base_addr + CTC_POWER_INTR_MASK2);
    }
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

    if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4XG)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4X2Q)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_24T4XG)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_8TS12X)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_65DV1)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_48P4X)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_24P4X))
    {
        ret = register_chrdev(CTC_POWER_DEV_MAJOR, "ctc_power", &power_fops);
        if (ret<0)
        {
            printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the power character device", ret);
            return ret;
        } 
    }	
    if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4XG)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_48T4X2Q)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_24T4XG)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_48P4X)
        || IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_V350_24P4X))
    {
        ret = request_irq(CTC_POWER_INTR_NUM, ctc_power_isr, IRQF_DISABLED, "ctc_power", ctc_power_isr);
        if (ret)
        {
            printk(KERN_WARNING "%s failed with %d\n", "Sorry, requesting irq for power interrupt", ret);
        }
    }
    else if(IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_8TS12X)
        ||IS_SERIES_BOARD_OF(ctc_bd_info, GLB_SERIES_E350, GLB_BOARD_E350_65DV1))
    {
        ret = request_irq(CTC_POWER_INTR_NUM2, ctc_power_isr, IRQF_DISABLED, "ctc_power", ctc_power_isr);
        if (ret)
        {
            printk(KERN_WARNING "%s failed with %d\n", "Sorry, requesting irq for power interrupt", ret);
        }
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

