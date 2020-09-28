/****************************************************************************
 * ctc_gpio_i2c.c :        lkm for gpio_i2c access, 
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
#include <linux/sched.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ctc_boards.h"
#include "ctc_gpio_i2c.h"

extern ctc_board_t ctc_bd_info;

typedef struct gpio_i2c_gen_s
{
    u8 scl_idx;
    u8 sda_idx;
}gpio_i2c_gen_t;

#define E_GPIO_OUT      0
#define E_GPIO_IN       1

#define CLK_DELAY       10
#define START_STOP_DELAY    10
#define ACK_DELAY       10

int _i2c_gpio_set_gpio_value( u8 pin_idx, u8 value)
{
    unsigned long val32;

    val32 = ls2h_readl(LS2H_GPIO_OUT_REG);
    //PDEBUG("_i2c_gpio_set_gpio_value read g_i2c_gpio_out 0x%4lx\n",val32);

    if(value)
        val32 |= (1<<pin_idx);
    else
        val32 &= ~(1<<pin_idx);
    udelay(10);
    ls2h_writel(val32, LS2H_GPIO_OUT_REG);
    //PDEBUG("_i2c_gpio_set_gpio_value write g_i2c_gpio_out 0x%4lx\n",val32);
    
    return 0;
}

int _i2c_gpio_get_gpio_value( u8 pin_idx, u8 *value)
{
    unsigned long val32;

    val32 = ls2h_readl(LS2H_GPIO_IN_REG);
    //PDEBUG("_i2c_gpio_get_gpio_value read g_i2c_gpio_in 0x%4lx\n",val32);
    *value = (val32 & (1<<pin_idx))? 1 : 0;
    
    return 0;
}

int _i2c_gpio_set_gpio_dir(u8 pin_idx, u8 dir)
{
    unsigned long val32;

    /* GPIOOE, 0 as output, 1 as input */
    val32 = ls2h_readl(LS2H_GPIO_OE_REG);
    //PDEBUG("_i2c_gpio_set_gpio_dir read g_i2c_gpio_oe 0x%4lx\n",val32);

    if(dir == E_GPIO_OUT)
        val32 &= ~(1<<pin_idx);
    else if(dir == E_GPIO_IN)
        val32 |= (1<<pin_idx);
    udelay(10);
    ls2h_writel(val32, LS2H_GPIO_OE_REG);
    //PDEBUG("_i2c_gpio_set_gpio_value write g_i2c_gpio_oe 0x%4lx\n",val32);
    
    return 0;
}

int _i2c_gpio_transfer_start(gpio_i2c_gen_t *gpio_i2c_gen )
{
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_OUT);/* set SDA out */
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->scl_idx, E_GPIO_OUT);/* set SCL out */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,1); /* send SDA 1 */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1);/* up SCL */
    udelay(START_STOP_DELAY); /* >4.7 us */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,0); /* send SDA 0 */
    udelay(START_STOP_DELAY); /* >4.7 us */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
    
    return 0;
}

int _i2c_gpio_transfer_stop(gpio_i2c_gen_t *gpio_i2c_gen )
{
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_OUT);/* set SDA out */
    udelay(START_STOP_DELAY); /* >4 us */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,0); /* send SDA 0 */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1);/* up SCL */
    udelay(START_STOP_DELAY); /* >4 us */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,1); /* send SDA 1 */
    udelay(START_STOP_DELAY); /* >4.7 us */
    
    return 0;
}

int _i2c_gpio_transfer_readAck(gpio_i2c_gen_t *gpio_i2c_gen, unsigned char *value)
{
    int timeout_cnt=0;
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_IN);/* set SDA in */
    //_i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0); /* low SCL */
    udelay(ACK_DELAY); /* delay for device preparing the ack data */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1);/* up SCL */
    udelay(ACK_DELAY/2);
    do
    {
        _i2c_gpio_get_gpio_value(gpio_i2c_gen->sda_idx,value); /* read SDA */
        if(!value)
            break;
        udelay(ACK_DELAY/2);
    }
    while(timeout_cnt--);
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0); /* low SCL */
    
    return 0;
}

int _i2c_gpio_transfer_sendAck(gpio_i2c_gen_t *gpio_i2c_gen )
{
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_OUT);/* set SDA out */
    //_i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,0); /* send SDA 0 */
    udelay(ACK_DELAY);
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1); /* up SCL */
    udelay(ACK_DELAY);
    _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
    
    return 0;
}

int _i2c_gpio_transfer_writeByte(gpio_i2c_gen_t *gpio_i2c_gen, unsigned char value )
{
    unsigned char ack=0;
    char i;

    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_OUT);/* set SDA out */
    for(i=7;i>=0;i--)
    {
        //_i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
        udelay(CLK_DELAY/2);
        _i2c_gpio_set_gpio_value(gpio_i2c_gen->sda_idx,value & (1<<i)); /* send SDA 0 */
        udelay(CLK_DELAY/2);
        _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1);/* up SCL */
        udelay(CLK_DELAY);
        _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
    }
    _i2c_gpio_transfer_readAck(gpio_i2c_gen,&ack);
    
    return 0;
}

int _i2c_gpio_transfer_readByte(gpio_i2c_gen_t *gpio_i2c_gen ,unsigned char *value)
{
    unsigned int tmp=0;
    unsigned char result=0;
    char i;
    
    _i2c_gpio_set_gpio_dir(gpio_i2c_gen->sda_idx, E_GPIO_IN);/* set SDA out */
    for(i=7;i>=0;i--)
    {   
        udelay(CLK_DELAY);
        _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,1);/* up SCL */
        udelay(CLK_DELAY/2);
        _i2c_gpio_get_gpio_value(gpio_i2c_gen->sda_idx,&result); /* read SDA */
        udelay(CLK_DELAY/2);
        tmp = (tmp | result) << 1;
        _i2c_gpio_set_gpio_value(gpio_i2c_gen->scl_idx,0);/* low SCL */
        
    }
    _i2c_gpio_transfer_sendAck(gpio_i2c_gen);
    *value = (tmp >> 1);
    
    return 0;
}

int ctc_i2c_gpio_transfer(struct i2c_msg *msgs,u32 nmsgs)
{
    int ret=0;
    int i = 0;
    long data_len = 0;
    int reg_len = 0;
    gpio_i2c_gen_t gpio_i2c_gen;
    PDEBUG("ctc_i2c_gpio_transfer %d nmsgs\n",nmsgs);

    gpio_i2c_gen.scl_idx = ctc_bd_info.gpio_i2c_scl;
    gpio_i2c_gen.sda_idx = ctc_bd_info.gpio_i2c_sda;

    if(gpio_i2c_gen.scl_idx ==0xff || gpio_i2c_gen.sda_idx == 0xff)
    {
        return 0;
    }

    for (i = 0; i < nmsgs; i++)
    {
        if(msgs[i].flags == 0x0)//write
        {
            _i2c_gpio_transfer_start(&gpio_i2c_gen);
            _i2c_gpio_transfer_writeByte(&gpio_i2c_gen,((msgs[i].addr & 0xff)<<1)|0);/* write op */
            for(reg_len = 0; reg_len < msgs[i].len; reg_len++) 
            {
                _i2c_gpio_transfer_writeByte(&gpio_i2c_gen, msgs[i].buf[reg_len]);
            }
            _i2c_gpio_transfer_stop(&gpio_i2c_gen); 
            udelay(START_STOP_DELAY*100);
        }
        else //read
        {
            _i2c_gpio_transfer_start(&gpio_i2c_gen);
            _i2c_gpio_transfer_writeByte(&gpio_i2c_gen,((msgs[i].addr & 0xff)<<1)|1);/* read op */ 

            for(data_len = 0; data_len < msgs[i].len; data_len++) 
            {
                _i2c_gpio_transfer_readByte(&gpio_i2c_gen,&msgs[i].buf[data_len]);/* read op */
            }
            _i2c_gpio_transfer_stop(&gpio_i2c_gen);
            udelay(START_STOP_DELAY*100);
        }
    }    
    return ret;

}


/*
 * Open and close
 */
int ctc_gpio_i2c_open(struct inode *inode, struct file *filp)
{
    unsigned long val32 = 0;
    unsigned long tmp = 0;
    gpio_i2c_gen_t gpio_i2c_gen;
    gpio_i2c_gen.scl_idx = ctc_bd_info.gpio_i2c_scl;
    gpio_i2c_gen.sda_idx = ctc_bd_info.gpio_i2c_sda;

    if(gpio_i2c_gen.scl_idx ==0xff || gpio_i2c_gen.sda_idx == 0xff)
    {
        return 0;
    }
    
    val32 = ls2h_readl(LS2H_GPIO_CFG_REG);
    tmp = val32;
    /* if scl is not GPIO */
    if(val32 & (1<<gpio_i2c_gen.scl_idx))
        val32 &= ~(1<<gpio_i2c_gen.scl_idx);
    /* if sda is not GPIO */
    if(val32 & (1<<gpio_i2c_gen.sda_idx))
        val32 &= ~(1<<gpio_i2c_gen.sda_idx);

    if(tmp != val32)
        ls2h_writel(val32, LS2H_GPIO_CFG_REG);
    
	return 0;          /* success */
}

struct i2c_msg_32{
    u16 addr;     
    u16 flags;           
    u16 len;              
    u32 buf;/* compat to (u8 *) */       
};

struct i2c_rdwr_ioctl_data_32{
    u32 msgs;   /* compat to (struct i2c_msg *) */
    u32 nmsgs;              /* number of i2c_msgs */
};

static int gpio_i2c_ioctl_rdrw(unsigned long arg)
{
    struct i2c_rdwr_ioctl_data_32 rdwr_arg;
    struct i2c_msg_32 *rdwr_pa_32;
    struct i2c_msg *rdwr_pa;
    u8 __user **data_ptrs;
    int i, res;
    
    if(ctc_bd_info.gpio_i2c_scl ==0xff || ctc_bd_info.gpio_i2c_sda == 0xff)
    {
        return 0;
    }

    if (copy_from_user(&rdwr_arg,
    		   (struct i2c_rdwr_ioctl_data_32 __user *)arg,
    		   sizeof(rdwr_arg)))
    	return -EFAULT;
    
    /* Put an arbitrary limit on the number of messages that can
     * be sent at once */
#if 1
    if (rdwr_arg.nmsgs > I2C_RDRW_IOCTL_MAX_MSGS)
    	return -EINVAL;

    rdwr_pa = (struct i2c_msg *)
        kmalloc(rdwr_arg.nmsgs * sizeof(struct i2c_msg),
        GFP_KERNEL);
    rdwr_pa_32 = (struct i2c_msg_32 *)
        kmalloc(rdwr_arg.nmsgs * sizeof(struct i2c_msg_32),
        GFP_KERNEL);
    if ((!rdwr_pa )|| (!rdwr_pa_32))
    	return -ENOMEM;
    
    if (copy_from_user(rdwr_pa_32, (struct i2c_msg_32 *)rdwr_arg.msgs,
    		   rdwr_arg.nmsgs * sizeof(struct i2c_msg_32))) {
    	kfree(rdwr_pa);
    	return -EFAULT;
    }
    data_ptrs = kmalloc(rdwr_arg.nmsgs * sizeof(u8 __user *), GFP_KERNEL);
    if (data_ptrs == NULL) {
    	kfree(rdwr_pa);
    	return -ENOMEM;
    }
    res = 0;
    for (i = 0; i < rdwr_arg.nmsgs; i++) {
        rdwr_pa[i].addr = rdwr_pa_32[i].addr;
        rdwr_pa[i].flags = rdwr_pa_32[i].flags;
        rdwr_pa[i].len = rdwr_pa_32[i].len;

	/* Limit the size of the message to a sane amount;
	 * and don't let length change either. */
	if ((rdwr_pa[i].len > 8192) ||
	    (rdwr_pa[i].flags & I2C_M_RECV_LEN)) {
		res = -EINVAL;
		break;
	}
	data_ptrs[i] = (u8 __user *)rdwr_pa_32[i].buf;
	rdwr_pa[i].buf = kmalloc(rdwr_pa[i].len, GFP_KERNEL);
	if (rdwr_pa[i].buf == NULL) {
		res = -ENOMEM;
		break;
	}
	if (copy_from_user(rdwr_pa[i].buf, data_ptrs[i],
			   rdwr_pa[i].len)) {
			++i; /* Needs to be kfreed too */
			res = -EFAULT;
		break;
	}
    }
    if (res < 0) {
    	int j;
    	for (j = 0; j < i; ++j)
    		kfree(rdwr_pa[j].buf);
    	kfree(data_ptrs);
    	kfree(rdwr_pa);
    	return res;
    }

    res = ctc_i2c_gpio_transfer(rdwr_pa, rdwr_arg.nmsgs);
    while (i-- > 0) {
    	if (res >= 0 && (rdwr_pa[i].flags & I2C_M_RD)) {
    		if (copy_to_user(data_ptrs[i], rdwr_pa[i].buf,
    				 rdwr_pa[i].len))
    			res = -EFAULT;
    	}
    	kfree(rdwr_pa[i].buf);
    }
    kfree(data_ptrs);
    kfree(rdwr_pa);
    return res;
#endif
}

/*
 * The ioctl() implementation
 */
 #ifdef CONFIG_COMPAT
 static long ctc_gpio_i2c_ioctl (struct file *file, 
             unsigned int cmd, unsigned long parameter) 
#else            
static int ctc_gpio_i2c_ioctl (struct inode* inode, struct file* file,
                unsigned int cmd, unsigned long parameter)            
#endif
{
    int retval = 0;

    switch(cmd) {
        case I2C_RDWR: 
            retval = gpio_i2c_ioctl_rdrw(parameter);
            if(retval)
            {
                PDEBUG("gpio_i2c_ioctl_rdrw:retval %d\n", retval); 
                return retval;
            }
            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            break;
    }

    return 0;
}

int ctc_gpio_i2c_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations ctc_gpio_i2c_fops = {
	.owner =    THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ctc_gpio_i2c_ioctl,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 13)
    .unlocked_ioctl = ctc_gpio_i2c_ioctl, 
#else
    .ioctl =    ctc_gpio_i2c_ioctl,
#endif /* !LINUX_VERSION_CODE */
#endif
    .open =     ctc_gpio_i2c_open,
    .release =  ctc_gpio_i2c_release,
};

static int __init  ctc_gpio_i2c_init(void)
{
    int ret = 0;

    ret = register_chrdev(CTC_GPIO_I2C_MAJOR, "ctc_gpio_i2c", &ctc_gpio_i2c_fops);
    if (ret<0)
    {
        printk(KERN_WARNING "%s failed with %d\n", "Sorry, registering the character device", ret);
        return ret;
    }

    return 0;
}

static void __exit ctc_gpio_i2c_exit(void)
{
    unregister_chrdev(CTC_GPIO_I2C_MAJOR, "ctc_gpio_i2c");
}

module_init(ctc_gpio_i2c_init);
module_exit(ctc_gpio_i2c_exit);
MODULE_LICENSE("GPL");
