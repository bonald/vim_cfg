/****************************************************************************
 * ctc_misc.c :        for immap/ioctl/interrupt
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
#include <linux/delay.h>
#include "ctc_boards.h"
#include "ctc_hw.h"

extern void si_meminfo(struct sysinfo *val);
extern ctc_board_t ctc_bd_info;

static int memusage_proc(long *val)
{
    unsigned long flags;
    unsigned long freeram;
#define K(x) ((x) << (PAGE_SHIFT - 10))

    local_irq_save(flags);
    freeram = global_page_state(NR_FREE_PAGES);
    *val = K(freeram);
    local_irq_restore(flags);
    
    return 0;
}

int hw_cpu_mdio_read(unsigned int phy_addr, unsigned int offset, unsigned int *value)
{
#if 0 /* not support in kernel v3.8 only GB demo need this func */
    phy_addr |= 0xff0000; /* add flag to indicate PHY VSC8512, qicx, bug25630, 2013-11-04 */
    /* *value = ctc_p1020_new_bus->read(ctc_p1020_new_bus, phy_addr, offset); */
    *value = mdiobus_read(ctc_p1020_new_bus, phy_addr, offset);
#endif
    return 0;
}

int hw_cpu_mdio_write(unsigned int phy_addr, unsigned int offset, unsigned int value)
{
#if 0 /* not support in kernel v3.8 only GB demo need this func */
    phy_addr |= 0xff0000; /* add flag to indicate PHY VSC8512, qicx, bug25630, 2013-11-04 */
    /* ctc_p1020_new_bus->write(ctc_p1020_new_bus, phy_addr, offset, value); */
    mdiobus_write(ctc_p1020_new_bus, phy_addr, offset, value);
#endif
    return 0;
}


static int
hw_user_read_cpu_mdio(unsigned long arg)
{
    hw_cpu_mdio_cfg_ioctl_t hw_cpu_mdio_cfg;
    hw_cpu_mdio_cfg_ioctl_t* tmp;
    if(copy_from_user(&hw_cpu_mdio_cfg, (void*)arg, sizeof(hw_cpu_mdio_cfg_ioctl_t)))
    {
        return -EFAULT;
    }

    if(hw_cpu_mdio_read(hw_cpu_mdio_cfg.phy_addr, hw_cpu_mdio_cfg.reg, &hw_cpu_mdio_cfg.value))
    {
        printk("hw_user_read_cpu_mdio failed.\n");
        return -EFAULT;
    }

    if(copy_to_user((hw_cpu_mdio_cfg_ioctl_t*)arg, (void*)&hw_cpu_mdio_cfg, sizeof(hw_cpu_mdio_cfg_ioctl_t)))
    {
        return -EFAULT;
    }
    tmp = (hw_cpu_mdio_cfg_ioctl_t*)arg;
    
    return 0;
}

static int
hw_user_write_cpu_mdio(unsigned long arg)
{
    hw_cpu_mdio_cfg_ioctl_t hw_cpu_mdio_cfg;
    if(copy_from_user(&hw_cpu_mdio_cfg, (void*)arg, sizeof(hw_cpu_mdio_cfg_ioctl_t)))
    {
        return -EFAULT;
    }

    return hw_cpu_mdio_write(hw_cpu_mdio_cfg.phy_addr, hw_cpu_mdio_cfg.reg, hw_cpu_mdio_cfg.value);
}



 static long ctc_hw_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter) 
 {
    int ret;
    long val;

    switch (cmd)
    {
         case CTC_GET_MEM_FREE:
            memusage_proc(&val);
            ret = put_user(val, (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");
            
            return ret;
         case CTC_CPU_MDIO_READ:
            return hw_user_read_cpu_mdio(parameter);

         case CTC_CPU_MDIO_WRITE:
            return hw_user_write_cpu_mdio(parameter);            
#if 0
         case CTC_HW_GET_PLATFORM_TYPE:
            ret = put_user(ctc_bd_info.platform_type, (u32*)parameter);
            if (ret<0)
                printk(KERN_WARNING"invalid user space pointer");
            return ret;
#endif
         default:
            break;
    }

    return 0;
}

static struct file_operations hw_fops = 
{
    .owner = THIS_MODULE,
#ifdef _KERNEL_VER_2_6_
    #ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_hw_ioctl,
    #else
     .ioctl = ctc_hw_ioctl,
    #endif
#else
    .unlocked_ioctl = ctc_hw_ioctl,
#endif
};

static int __init ctc_hw_init(void)
{
    int ret;
    
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

MODULE_LICENSE("GPL");

