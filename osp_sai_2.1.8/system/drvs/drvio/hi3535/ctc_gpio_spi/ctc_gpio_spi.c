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
#include "ctc_gpio_spi.h"

struct ctc_spi_msg {
    u16 addr;
    u16 alen;
    u16 op_flag;
    u16 len;
    u32 *tx_buf;
    u32 *rx_buf;
};

struct ctc_spi_msg_32 {
    u16 addr;
    u16 alen;
    u16 op_flag;
    u16 len;
    u32 tx_buf;
    u32 rx_buf;
};

struct spi_rdwr_ioctl_data {
    u32 msgs;   /* pointers to i2c_msgs */
};

extern ctc_board_t ctc_bd_info;

int ctc_gpio_spi_major = CTC_GPIO_SPI_MAJOR;   
u16 gpio_date_offset;
u16 gpio_dir_offset;  
static void __iomem *gpio_spi_virtual_addr;

static int gpio_spi_cs_bit;
static int gpio_spi_clk_bit;
static int gpio_spi_data_out_bit;
static int gpio_spi_data_in_bit;
static int gpio_spi_valid = 0;

#define CTC_READL   readw
#define CTC_WRITEL  writew

static void spi_gpio_delay()
{ 
    udelay(50);
}

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

void ctc_gpio_spi_set(int gpio_pin)
{
    //u32 value;
    
    //value = CTC_READL(gpio_spi_virtual_addr + gpio_date_offset);
    //PDEBUG("set pin %d start: val 0x%x\n", gpio_pin, value);

    setbits32(gpio_spi_virtual_addr + gpio_date_offset, 0x1 << gpio_pin);

    //value = CTC_READL(gpio_spi_virtual_addr + gpio_date_offset );
    //PDEBUG("set pin %d end: val 0x%x\n", gpio_pin, value);  
}

void ctc_gpio_spi_clr(int gpio_pin)
{
    //u32 value;
    //value = CTC_READL(gpio_spi_virtual_addr + gpio_date_offset );
    //PDEBUG("clr pin %d start: val 0x%x\n", gpio_pin, value);    

    clrbits32(gpio_spi_virtual_addr + gpio_date_offset, 0x1 << gpio_pin); 

    //value = CTC_READL(gpio_spi_virtual_addr + gpio_date_offset );
    //PDEBUG("clr pin %d end: val 0x%x\n", gpio_pin, value);       
}

int ctc_gpio_get(int gpio_pin)
{
    u32 value;
    value = CTC_READL(gpio_spi_virtual_addr + gpio_date_offset ) ;    
    //PDEBUG("ctc_gpio_get start : %d val %x\n", gpio_pin, value);       
    
    return ((value>> gpio_pin) & 0x1);
}

int spi_cs( u32 assert_data)
{
    //PDEBUG("spi_cs -> %d\n",assert_data);
    if (assert_data)
    {
        ctc_gpio_spi_set(gpio_spi_cs_bit);
    }           
    else
    {
        ctc_gpio_spi_clr(gpio_spi_cs_bit);       
    }
    return 0;
}

int spi_clk( u32 assert_clk)
{
    //PDEBUG("spi_clk -> %d\n",assert_clk);
    if (assert_clk)
    {
        ctc_gpio_spi_set(gpio_spi_clk_bit);
    }           
    else
    {
        ctc_gpio_spi_clr(gpio_spi_clk_bit);       
    }
    return 0;
}

int spi_data_out( u32 assert_data)
{
    //PDEBUG("spi_out-> %d\n",assert_data);
    if (assert_data)
    {
        ctc_gpio_spi_set(gpio_spi_data_out_bit);
    }           
    else
    {
        ctc_gpio_spi_clr(gpio_spi_data_out_bit);       
    }
    return 0;
}

int spi_data_in()
{
    unsigned int val = 0;
    val = ctc_gpio_get(gpio_spi_data_in_bit);
    //PDEBUG("spi_in -> %d\n",val);
    return val;
}

int spi_read(u16 addr,u32 alen, u32 *val, u32 len)
{
    u32 i;
    *val = 0x0;
    spi_cs (0);
    
    for (i = 0; i < alen * 8; i++ )
    {
        //clk(0);
        spi_clk(0);        
        spi_gpio_delay();
        
        if(addr&(1<<(alen*8-1)))
        {
            //date_out(1);
            spi_data_out(1);
            spi_gpio_delay();
        }
        else
        {
            //date_out(0);
            spi_data_out(0);
            spi_gpio_delay();
        }
        //clk(1);
        spi_clk(1);
        spi_gpio_delay();
        spi_gpio_delay();
        addr <<= 1;
    }/*write  addr*/
    //PDEBUG("#########finish addr wirte , continue data #########\n");
    for (i=0; i<len*8; i++)
    {
        //clk(0);
        spi_clk(0);        
        spi_gpio_delay();
        
        *val <<= 1;
        
        //if(*di_addr)
        if(spi_data_in())
        {
            *val |= 0x1; 
            spi_gpio_delay();
        }
        else
        {
            *val &= (~0x1);
            spi_gpio_delay();
        }
        //clk(1);
        spi_clk(1);
        spi_gpio_delay();
        spi_gpio_delay();
    }/*read value*/
    
    //*en_addr = ~0x0;
    spi_cs(1);
    PDEBUG("#########value 0x%x#########\n",*val);
    return 0;
}

int spi_write(u16 addr,u32 alen, u32 *val, u32 len)
{
    u32 i;
    u32 tmp;
    
    tmp = *val;

    //*en_addr = 0x0;
    spi_cs(0);
   
    for (i=0; i<alen*8; i++)
    {
        //clk(0);        
       
        spi_clk(0);        
        spi_gpio_delay();
        
        if((addr)&(1<<(alen*8-1)))
        {
            //date_out(1);
            spi_data_out(1);
            spi_gpio_delay();
        }
        else
        {
            //date_out(0);
            spi_data_out(0);
            spi_gpio_delay();
        }
        
        //clk(1);
        spi_clk(1);        
        spi_gpio_delay();
        spi_gpio_delay();
        addr <<= 1;
    }/*write  addr*/   
    //PDEBUG("#########finish addr , continue data #########\n");
    for (i=0; i<len*8; i++)
    {
        //clk(0);
        spi_clk(0);
        
        spi_gpio_delay();
        
        if(tmp&(1<<(len*8-1)))
        {
            //date_out(1);
            spi_data_out(1);
            spi_gpio_delay();
        }
        else
        {
            //date_out(0);
            spi_data_out(0);
            spi_gpio_delay();
        }
        
        //clk(1);
        spi_clk(1);
        spi_gpio_delay();
        spi_gpio_delay();
        tmp <<= 1;
    }/*write value*/
    
    //*en_addr = ~0x0;
    spi_cs(1);
    
    return 0;
}
static int gpio_spi_ioctl_rdrw(struct ctc_spi_msg_32 *spi_msg_32)
{
    int ret = 0;
    struct ctc_spi_msg *spi_msg;

    spi_msg = (struct ctc_spi_msg *)kmalloc(sizeof(struct ctc_spi_msg),GFP_KERNEL);
    if (!spi_msg )
        return -ENOMEM;
    PDEBUG("gpio_spi_ioctl_rdrw 1\n");

    spi_msg->addr = spi_msg_32->addr;
    spi_msg->alen = spi_msg_32->alen;
    spi_msg->op_flag = spi_msg_32->op_flag;
    spi_msg->len = spi_msg_32->len;
    spi_msg->tx_buf = kmalloc(spi_msg->len, GFP_KERNEL);
    if (spi_msg->tx_buf == NULL) {
        kfree(spi_msg);
        return -ENOMEM;
    }
    PDEBUG("gpio_spi_ioctl_rdrw 2\n");
    spi_msg->rx_buf = kmalloc(spi_msg->len, GFP_KERNEL);
    if (spi_msg->rx_buf == NULL) {
        kfree(spi_msg->tx_buf);
        kfree(spi_msg);
        return -ENOMEM;
    }
    PDEBUG("gpio_spi_ioctl_rdrw 3\n");

    /* move data to kernel space*/
    if (copy_from_user(spi_msg->tx_buf, (u32 __user *)spi_msg_32->tx_buf,spi_msg->len)) ///TODO:modified by tongzb
        ret = -EFAULT;
    PDEBUG("gpio_spi_ioctl_rdrw 4\n");

    if(spi_msg->op_flag == 0)//write
    {
        PDEBUG("gpio_spi_ioctl_rdrw 51:addr--0x%x;alen--%d;len--%d\n",spi_msg->addr,spi_msg->alen,spi_msg->len);
        spi_write(spi_msg->addr,spi_msg->alen, spi_msg->tx_buf, spi_msg->len);
        PDEBUG("gpio_spi_ioctl_rdrw 61\n");
    }
    else //read
    {
        PDEBUG("gpio_spi_ioctl_rdrw 52:addr--0x%x;alen--%d;len--%d\n",spi_msg->addr,spi_msg->alen,spi_msg->len);
        spi_read(spi_msg->addr,spi_msg->alen, spi_msg->rx_buf, spi_msg->len);
        /* move data to user space */
        PDEBUG("gpio_spi_ioctl_rdrw 62\n");
        PDEBUG("#########rx_buf 0x%x#########\n",*spi_msg->rx_buf);
        if (copy_to_user((u32 __user *)spi_msg_32->rx_buf, spi_msg->rx_buf,spi_msg->len))///TODO:modified by tongzb
            ret = -EFAULT;
        PDEBUG("gpio_spi_ioctl_rdrw 7\n");
    }
    kfree(spi_msg->rx_buf );
    PDEBUG("free spi_msg->rx_buf\n");
    kfree(spi_msg->tx_buf );
    PDEBUG("free spi_msg->tx_buf\n");
    kfree(spi_msg);
    PDEBUG("free spi_msg\n");

    return ret;
}

int ctc_gpio_spi_open(struct inode *inode, struct file *filp)
{
    if(((GLB_SERIES_E550==ctc_bd_info.board_series) && (GLB_BOARD_E550_24X8Y2C==ctc_bd_info.board_type))
        ||((GLB_SERIES_E550==ctc_bd_info.board_series) && (GLB_BOARD_E550_28M4Y2C==ctc_bd_info.board_type))
        ||((GLB_SERIES_E550==ctc_bd_info.board_series) && (GLB_BOARD_E550_48M4Y2C==ctc_bd_info.board_type)))
    {
        gpio_spi_virtual_addr = ctc_bd_info.gpio_g1_logic_addr;
        gpio_spi_cs_bit = 4;
        gpio_spi_clk_bit = 5;
        gpio_spi_data_out_bit = 3;
        gpio_spi_data_in_bit = 2;
        gpio_spi_valid = 1;
    }
    else
    {
        return 0;
    }
    gpio_dir_offset = ctc_bd_info.gpio_dir_offset;
    gpio_date_offset= ctc_bd_info.gpio_date_offset;

    /*set gpio dir, 0 input, 1 output*/
    setbits32(gpio_spi_virtual_addr + gpio_dir_offset, (1<<gpio_spi_cs_bit)|(1<<gpio_spi_clk_bit)|(1<<gpio_spi_data_out_bit));    
    clrbits32(gpio_spi_virtual_addr + gpio_dir_offset, (1<<gpio_spi_data_in_bit));  

    return 0;
}

/*
 * The ioctl() implementation
 */
long ctc_gpio_spi_ioctl(struct file *file, 
             unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct spi_rdwr_ioctl_data rdwr_arg;
    struct ctc_spi_msg_32 *spi_msg;

    if(!gpio_spi_valid)
    {
        printk( KERN_INFO "%%This board do not support gpio_spi\n%%please check the board info\n");
        return 0;
    }
	switch(cmd) {
	  case CTC_GPIO_SPI_RW: 
    	if (copy_from_user(&rdwr_arg,(struct spi_rdwr_ioctl_data __user *)arg, sizeof(rdwr_arg)))
		    return -EFAULT;
        spi_msg = (struct ctc_spi_msg_32 *)kmalloc(sizeof(struct ctc_spi_msg_32),GFP_KERNEL);
        if (!spi_msg )
            return -ENOMEM;
        if (copy_from_user(spi_msg, (struct ctc_spi_msg_32 *)rdwr_arg.msgs,sizeof(struct ctc_spi_msg_32)))
        {
            kfree(spi_msg);
            return -EFAULT;
        }
        ret = gpio_spi_ioctl_rdrw(spi_msg);
        kfree(spi_msg);

		break;
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

	return 0;
}

int ctc_gpio_spi_release(struct inode *inode, struct file *filp)
{
    gpio_spi_valid = 0;
	return 0;
}

struct file_operations ctc_gpio_spi_fops = {
	.owner =    THIS_MODULE,
    .compat_ioctl = ctc_gpio_spi_ioctl,
    .unlocked_ioctl = ctc_gpio_spi_ioctl,
	.open =     ctc_gpio_spi_open,
	.release =  ctc_gpio_spi_release,
};

static int __init  ctc_gpio_spi_init(void)
{
	int ret;

    ret = register_chrdev(CTC_GPIO_SPI_MAJOR, "ctc_gpio_spi", &ctc_gpio_spi_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    return 0;
}
static void __exit ctc_gpio_spi_exit(void)
{
    unregister_chrdev(CTC_GPIO_SPI_MAJOR, "ctc_gpio_spi");
}

module_init(ctc_gpio_spi_init);
module_exit(ctc_gpio_spi_exit);
MODULE_LICENSE("GPL");

