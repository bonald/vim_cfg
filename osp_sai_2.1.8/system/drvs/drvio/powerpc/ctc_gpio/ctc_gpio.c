/****************************************************************************
 * ctc_gpio.c :        lkm for gpio access
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision         :         R0.01
 * Author           :         Zhu Jian
 * Date             :         2010-1-30
 * Reason         :         First Create
 ****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/types.h>	/* size_t */
#include <linux/fs.h>		/* everything... */
#include <linux/mm.h>
#include <asm/uaccess.h>   /*copy_from_user */
#include <linux/cdev.h>
#include <linux/spinlock_types.h>
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/delay.h>
#include <linux/slab.h>
//#include <asm/ctc_platform.h>
#include "ctc_boards.h"
#include "ctc_gpio.h"

struct ctc_gpio_dev {    
	spinlock_t iorw_spinlock;    /* spin lock     */    
	struct cdev cdev;	         /* Char device structure*/
};

extern ctc_board_t ctc_bd_info;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 13)
#else
static spinlock_t g_iorw_spinlock = SPIN_LOCK_UNLOCKED;
#endif /* !LINUX_VERSION_CODE */

int ctc_gpio_major = CTC_GPIO_MAJOR;
int ctc_gpio_minor = 0;
int ctc_gpio_nr_devs = CTC_GPIO_NR_DEVS;	/* number of bare ctc_gpio devices */
struct ctc_gpio_dev *ctc_gpio_devices;	    

void* gpio_virtual_addr;

#define GPIO_TRST_N          0x08000000  /*TRST_N,GPIO4*/
#define GPIO_TDI             0x40000000  /*TDI, GPIO1*/
#define GPIO_TDO             0x80000000  /*TDO, GPIO0*/
#define GPIO_TMS             0x20000000  /*TMS, GPIO2*/
#define GPIO_TCK             0x10000000  /*TCK, GPIO3*/
#define GPIO_CTRL            0x04000000  /*GPIO ctrl buffer*/

void ctc_gpio_init(void)
{
#ifndef _CTC_X86_64_
/* added by kcao for X86TODO */

    u32 value;
    gpio_virtual_addr = ctc_bd_info.gpio_logic_addr;
    
    value = in_be32(gpio_virtual_addr );
    PDEBUG("ctc_gpio_init start config: val %x\n", value); 
#ifdef _CTC_FSCL_P1010_    
    /* although pegatron board use p1014 CPU, but it can be compatible with p1010 CPU */
    if((ctc_bd_info.board_series == GLB_SERIES_E350)
        && (GLB_BOARD_E350_PEGATRON == ctc_bd_info.board_type))
    {
    }
    /* Modified by liuht for bu40949 to support update epld online for E350-48T4XG, 2016-1015  */
    else if((ctc_bd_info.board_series == GLB_SERIES_E350)
        && ((GLB_BOARD_E350_48T4XG == ctc_bd_info.board_type)
        ||(GLB_BOARD_V350_48P4X == ctc_bd_info.board_type)
        ||(GLB_BOARD_V350_24P4X == ctc_bd_info.board_type)))
    {
        /*set gpio dir, 0 input, 1 output*/
        setbits32(gpio_virtual_addr, GPIO_TRST_N |GPIO_TDI|
            GPIO_TMS|GPIO_TCK|GPIO_CTRL);    
        clrbits32(gpio_virtual_addr, GPIO_TDO);    
        setbits32(gpio_virtual_addr + 8, GPIO_TRST_N );
        clrbits32(gpio_virtual_addr + 8, GPIO_CTRL);
    }
    else
    {
        /*set gpio dir, 0 input, 1 output*/
        setbits32(gpio_virtual_addr, GPIO_TDI|GPIO_TMS|GPIO_TCK);    
        clrbits32(gpio_virtual_addr, GPIO_TDO); 
    } 
#else
    /*set gpio dir, 0 input, 1 output*/
    setbits32(gpio_virtual_addr, GPIO_TDI|GPIO_TMS|GPIO_TCK);
    clrbits32(gpio_virtual_addr, GPIO_TDO);
#endif
    value = in_be32(gpio_virtual_addr +8);
    PDEBUG("ctc_gpio_init data: val %x\n", value);
    
#endif /*!_CTC_X86_64_ */
}

void ctc_gpio_set(int gpio_pin)
{
#ifndef _CTC_X86_64_
/* added by kcao for X86TODO */

    u32 value;
    value = in_be32(gpio_virtual_addr + 8 );
    PDEBUG("ctc_gpio_set start: %d val %x\n", gpio_pin, value);

    setbits32(gpio_virtual_addr + 8, 0x1 << (31-gpio_pin));

    value = in_be32(gpio_virtual_addr + 8 );
    PDEBUG("ctc_gpio_set end: %d val %x\n", gpio_pin, value);
    
#endif /*!_CTC_X86_64_ */
}

void ctc_gpio_clr(int gpio_pin)
{
#ifndef _CTC_X86_64_
/* added by kcao for X86TODO */

    u32 value;
    value = in_be32(gpio_virtual_addr + 8 );
    PDEBUG("ctc_gpio_clr start: %d val %x\n", gpio_pin, value);    

    clrbits32(gpio_virtual_addr + 8, 0x1 << (31-gpio_pin)); 

    value = in_be32(gpio_virtual_addr + 8 );
    PDEBUG("ctc_gpio_clr end: %d val %x\n", gpio_pin, value);
    
#endif /*!_CTC_X86_64_ */
}

int ctc_gpio_get(int gpio_pin)
{
    u32 value = 0;
#ifndef _CTC_X86_64_
/* added by kcao for X86TODO */

    value = in_be32(gpio_virtual_addr + 8 ) ;    
    PDEBUG("ctc_gpio_get start : %d val %x\n", gpio_pin, value);       

    value = (in_be32(gpio_virtual_addr + 8 )>> (31-gpio_pin)) & 0x1;  

    PDEBUG("ctc_gpio_get end: %d val %x\n", gpio_pin, value);

#endif /*!_CTC_X86_64_ */
    return value;
}

/*
 * The ioctl() implementation
 */
 /*fix bug31456, update epld failed, liangf, 2015-04-24*/
int ctc_gpio_ioctl(struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	//int err = 0;
    int val = 0;
	int retval = 0;
    int gpio;
    int high;
    
	switch(cmd) {
	  case CTC_GPIO_WRITE: 
		if (! capable (CAP_SYS_ADMIN))
			return -EPERM;
		retval = __get_user(val, (int __user *)arg);
            //printk("CTC_GPIO_WRITE: write val %x\n", val);
        gpio = val&0xff;
        high = (val>>8)&0xff;
        if(high)
            ctc_gpio_set(gpio);
        else
            ctc_gpio_clr(gpio);
		break;
	  case CTC_GPIO_READ:
		retval = __get_user(val, (int __user *)arg);
            //printk("CTC_GPIO_READ: 111 read val %x\n", val);
        gpio = val&0xff;
        high = ctc_gpio_get(gpio);
        if(high)
            val |= 0x1<<8;
        else
            val &= ~0x1<<8;
            //printk("CTC_GPIO_READ: 222 read val %x\n", val);
		retval = __put_user(val, (int __user *)arg);
		break;
      case CTC_GPIO_GPIO_INIT:
            //printk("CTC_GPIO_GPIO_INIT: gpio init\n");
        ctc_gpio_init();
            //printk("CTC_GPIO_GPIO_INIT: gpio init end\n");
        break;
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

	return 0;
}

/*
 * Open and close
 */
int ctc_gpio_open(struct inode *inode, struct file *filp)
{
	struct ctc_gpio_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct ctc_gpio_dev, cdev);
	filp->private_data = dev; /* for other methods */
    
	return 0;          /* success */
}

int ctc_gpio_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations ctc_gpio_fops = {
	.owner =    THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_gpio_ioctl,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 13)
    .unlocked_ioctl = ctc_gpio_ioctl, 
#else
    .ioctl =    ctc_gpio_ioctl,
#endif /* !LINUX_VERSION_CODE */
#endif
	.open =     ctc_gpio_open,
	.release =  ctc_gpio_release,
};

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void ctc_gpio_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(ctc_gpio_major, ctc_gpio_minor);

	/* Get rid of our char dev entries */
	if (ctc_gpio_devices) {
		for (i = 0; i < ctc_gpio_nr_devs; i++) {
			cdev_del(&ctc_gpio_devices[i].cdev);
		}
		kfree(ctc_gpio_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, ctc_gpio_nr_devs);
}


/*
 * Set up the char_dev structure for this device.
 */
static void ctc_gpio_setup_cdev(struct ctc_gpio_dev *dev, int index)
{
	int err, devno = MKDEV(ctc_gpio_major, ctc_gpio_minor + index);

	cdev_init(&dev->cdev, &ctc_gpio_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &ctc_gpio_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding ctc_gpio%d", err, index);
}


int ctc_gpio_init_module(void)
{
	int result, i;
	dev_t dev = 0;

/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	dev = MKDEV(ctc_gpio_major, ctc_gpio_minor);
	result = register_chrdev_region(dev, ctc_gpio_nr_devs, "ctc_gpio");
    
	if (result < 0) {
		printk(KERN_WARNING "ctc_gpio: can't get major %d\n", ctc_gpio_major);
		return result;
	}
    
/* 
 * allocate the devices -- we can't have them static, as the number
 * can be specified at load time
 */
	ctc_gpio_devices = kmalloc(ctc_gpio_nr_devs * sizeof(struct ctc_gpio_dev), GFP_KERNEL);
	if (!ctc_gpio_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
    
	memset(ctc_gpio_devices, 0, ctc_gpio_nr_devs * sizeof(struct ctc_gpio_dev));

    /* Initialize each device. */
	for (i = 0; i < ctc_gpio_nr_devs; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 13)
        ctc_gpio_devices[i].iorw_spinlock = __SPIN_LOCK_UNLOCKED(ctc_gpio_devices[i].iorw_spinlock);
#else
        ctc_gpio_devices[i].iorw_spinlock = g_iorw_spinlock;
#endif /* !LINUX_VERSION_CODE */
        ctc_gpio_setup_cdev(&ctc_gpio_devices[i], i);
	}
  
	return 0; /* succeed */
    
fail:
	ctc_gpio_cleanup_module();
	return result;
}

module_init(ctc_gpio_init_module);
module_exit(ctc_gpio_cleanup_module);
MODULE_LICENSE("GPL");
