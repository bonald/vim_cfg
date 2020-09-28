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
#include <linux/module.h>
#include <linux/version.h>
#include <asm/io.h>
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
#include "ctc_boards.h"
#include "ctc_gpio.h"

struct ctc_gpio_dev {    
	spinlock_t iorw_spinlock;    /* spin lock     */    
	struct cdev cdev;	         /* Char device structure*/
};

extern ctc_board_t ctc_bd_info;

int ctc_gpio_major = CTC_GPIO_MAJOR;
int ctc_gpio_minor = 0;
int ctc_gpio_nr_devs = CTC_GPIO_NR_DEVS;	/* number of bare ctc_gpio devices */
struct ctc_gpio_dev *ctc_gpio_devices;	    
u16 gpio_date_offset;
u16 gpio_dir_offset;  
void* gpio_virtual_addr;

#define GPIO_TDI             0x02  /*TDI, GPIO1*/
#define GPIO_TDO             0x08  /*TDO, GPIO3*/
#define GPIO_TMS             0x04  /*TMS, GPIO2*/
#define GPIO_TCK             0x01  /*TCK, GPIO0*/
#define GPIO_CTRL            0x04000000  /*GPIO ctrl buffer*/

#define CTC_READL   __raw_readl
#define CTC_WRITEL  __raw_writel

static inline void setbits32(void __iomem *addr, int bits)
{
    int val;
    
	val = CTC_READL(addr);
    val |= bits;
    CTC_WRITEL(val, addr);
}

static inline void clrbits32(void __iomem *addr, int bits)
{
    int val;
    
	val = CTC_READL(addr);
    val &= ~(bits);
    CTC_WRITEL(val, addr);
}

void ctc_gpio_init(void)
{
    u32 value;

    gpio_dir_offset = ctc_bd_info.gpio_dir_offset;
    gpio_date_offset= ctc_bd_info.gpio_date_offset;
    gpio_virtual_addr = ctc_bd_info.gpio_g0_logic_addr;
    
    value = CTC_READL(gpio_virtual_addr + gpio_dir_offset);
    PDEBUG("ctc_gpio_init start config: val %x\n", value); 
    /*set gpio dir, 0 input, 1 output*/
    setbits32(gpio_virtual_addr + gpio_dir_offset, GPIO_TDI|GPIO_TMS|GPIO_TCK);    
    clrbits32(gpio_virtual_addr + gpio_dir_offset, GPIO_TDO);  
    value = CTC_READL(gpio_virtual_addr + gpio_date_offset);
    PDEBUG("ctc_gpio_init data: val %x\n", value);    
}

void ctc_gpio_set(int gpio_pin)
{
    u32 value;
    
    value = CTC_READL(gpio_virtual_addr + gpio_date_offset);
    PDEBUG("ctc_gpio_set start: %d val %x\n", gpio_pin, value);

    setbits32(gpio_virtual_addr + gpio_date_offset, 0x1 << gpio_pin);

    value = CTC_READL(gpio_virtual_addr + gpio_date_offset );
    PDEBUG("ctc_gpio_set end: %d val %x\n", gpio_pin, value);  
}

void ctc_gpio_clr(int gpio_pin)
{
    u32 value;
    value = CTC_READL(gpio_virtual_addr + gpio_date_offset );
    PDEBUG("ctc_gpio_clr start: %d val %x\n", gpio_pin, value);    

    clrbits32(gpio_virtual_addr + gpio_date_offset, 0x1 << gpio_pin); 

    value = CTC_READL(gpio_virtual_addr + gpio_date_offset );
    PDEBUG("ctc_gpio_clr end: %d val %x\n", gpio_pin, value);       
}

int ctc_gpio_get(int gpio_pin)
{
    u32 value;
    value = CTC_READL(gpio_virtual_addr + gpio_date_offset ) ;    
    PDEBUG("ctc_gpio_get start : %d val %x\n", gpio_pin, value);       

    value = (CTC_READL(gpio_virtual_addr + gpio_date_offset )>> gpio_pin) & 0x1;  

    PDEBUG("ctc_gpio_get end: %d val %x\n", gpio_pin, value);    
    return value;
}

/*
 * The ioctl() implementation
 */
long ctc_gpio_ioctl(struct file *file, 
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
    .compat_ioctl = ctc_gpio_ioctl,
    .unlocked_ioctl = ctc_gpio_ioctl,
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
        ctc_gpio_devices[i].iorw_spinlock = __SPIN_LOCK_UNLOCKED(ctc_gpio_devices[i].iorw_spinlock);
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

