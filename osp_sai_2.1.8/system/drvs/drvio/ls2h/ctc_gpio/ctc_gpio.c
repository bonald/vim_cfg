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
#include "ctc_boards.h"
#include "ctc_gpio.h"

struct ctc_gpio_dev {    
	spinlock_t iorw_spinlock;    /* spin lock     */    
	struct cdev cdev;	         /* Char device structure*/
};

extern ctc_board_t ctc_bd_info;
#define IS_SERIES_BOARD_OF(boardinfo, series, board)                      \
    ((series == boardinfo.board_series) && (board == boardinfo.board_type))
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 13)
#else
static spinlock_t g_iorw_spinlock = SPIN_LOCK_UNLOCKED;
#endif /* !LINUX_VERSION_CODE */

int ctc_gpio_major = CTC_GPIO_MAJOR;
int ctc_gpio_minor = 0;
int ctc_gpio_nr_devs = CTC_GPIO_NR_DEVS;	/* number of bare ctc_gpio devices */
struct ctc_gpio_dev *ctc_gpio_devices;	    

/*********************************************************************
  For CTC-LS2H board
  +----+---------+---------+--------------------------------------+
  |GPIO|Function |         |                                      |
  | (n)|Selection|Direction| Function Description                 |
  +----+---------+---------+--------------------------------------+
  | 0  |  GPIO   |    IN   | ASIC Intr 1 (while SYS_INT as Intr 0)|
  | 1  |  GPIO   |    IN   | Normal Intr 0 (such as PHY)          |
  | 2  |  GPIO   |    IN   | Normal Intr 1 (such as DPLL)       |
  | 3  |  UART1|     x       |UART1                           |
  | 4  |  GPIO   |   OUT   | PHY reset                     |
  | 5  |  GPIO   |   OUT   | Watchdog Enable        |
  | 6  |  UART1|      x      | UART1                          |
  | 7  |  GPIO   |   OUT   | Watchdog Feed                        |
  | 8  | SPI_CS1 |   x   | SPI CS 1                             |
  | 9  | SPI_CS2 |   x   | SPI CS 2                             |
  | 10 |  GPIO   |   OUT   | LED: RUN                             |
  | 11 |  GPIO   |   OUT   | LED: ALARM                           |
  | 12 |  GPIO   |   OUT   | Reserved                             |
  | 13 |  GPIO   |   OUT   | Reserved                             |
  | 14 |  GPIO   |   OUT   | Reserved                             |
  | 15 |  GPIO   |   OUT   | Reserved                             |
  +----+---------+---------+--------------------------------------+
 ********************************************************************/
void ctc_normal_gpio_init(void)
{
    u32 val32;
    
    /* fix bug49858, enable watchdog, liangf, 2018-11-06 */
    val32 = ls2h_readl(LS2H_GPIO_OUT_REG);
    val32 |= 0xa0;  /* GPIO5/7 */
    ls2h_writel(val32, LS2H_GPIO_OUT_REG);

    /* Step 1: GPIOCFG, 0 as GPIO, 1 as OTHER */
    val32 = ls2h_readl(LS2H_GPIO_CFG_REG);
    val32 &= ~0xa7;     /* GPIO 0/1/2/5/7 */
    val32 |= 0x348;     /* GPIO8 SPI_CSn1; GPIO9, SPI_CSn2; GPIO3,GPIO6 UART*/
    ls2h_writel(val32, LS2H_GPIO_CFG_REG);

    /* Step 2: GPIOOE, 0 as output, 1 as input */
    val32 = ls2h_readl(LS2H_GPIO_OE_REG);
    val32 |= 0x7;     /* GPIO 0/1/2 */
    val32 &= ~0xa0;  /* GPIO 5/7 */
    ls2h_writel(val32, LS2H_GPIO_OE_REG);
    
     /* fix bug51374, support fastboot, qianj, 2019-03-22*/
    if (!ctc_bd_info.fast_boot_en)
    {
        val32 = ls2h_readl(LS2H_GPIO_CFG_REG);
        val32 &= ~0x10;  /* GPIO4*/
        ls2h_writel(val32, LS2H_GPIO_CFG_REG);

        val32 = ls2h_readl(LS2H_GPIO_OE_REG);
        val32 &= ~0x10;  /* GPIO4*/    
        ls2h_writel(val32, LS2H_GPIO_OE_REG);
        
        /*fix bug43760, support syncE, liangf, 2017-04-20*/
        /*CFG GPIO4, 0 as GPIO */
        val32 = ls2h_readl(LS2H_GPIO_OUT_REG);
        //GPIO4, Reset PHY, 0
        val32 &= ~0x10;
        ls2h_writel(val32, LS2H_GPIO_OUT_REG);
        udelay(100*1000); /*Actually,it's about 7ms*/
        //Release reset PHY, 1
        val32 |= 0x10;
        ls2h_writel(val32, LS2H_GPIO_OUT_REG);
    }
}

void ctc_gpio_init(void)
{
    /* set TCK,TMS,TDI,TDO GPIO */
}

void ctc_gpio_set(int gpio_pin)
{
   u32 value;

    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    //PDEBUG("_i2c_gpio_set_gpio_value read g_i2c_gpio_out 0x%4lx\n",value);
    value |= (1<<gpio_pin);
    udelay(10);
    ls2h_writel(value, LS2H_GPIO_OUT_REG);
}

void ctc_gpio_clr(int gpio_pin)
{
   u32 value;

    value = ls2h_readl(LS2H_GPIO_OUT_REG);
    //PDEBUG("_i2c_gpio_set_gpio_value read g_i2c_gpio_out 0x%4lx\n",value);
    value &= ~(1<<gpio_pin);
    udelay(10);
    ls2h_writel(value, LS2H_GPIO_OUT_REG);
}

int ctc_gpio_get(int gpio_pin)
{
   u32 value;
    
    value = ls2h_readl(LS2H_GPIO_IN_REG);
    //PDEBUG("_i2c_gpio_get_gpio_value read g_i2c_gpio_in 0x%4lx\n",val32);
    value = (value & (1<<gpio_pin))? 1 : 0;
    return value;
}

/*
 * The ioctl() implementation
 */
int ctc_gpio_ioctl(struct inode *inode,struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    int val = 0;
    int retval = 0;
    int gpio = 0;
    int high = 0;
    
    switch(cmd) 
    {
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

    ctc_normal_gpio_init();
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
