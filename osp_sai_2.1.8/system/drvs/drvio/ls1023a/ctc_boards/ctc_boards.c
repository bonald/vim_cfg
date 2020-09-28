#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include "ctc_boards.h"
//#include "mach/i2c-hi3535.h"

#define CTC_EPLD_I2C_OLD_ADDR     0x58
#define CTC_EPLD_I2C_NEW_ADDR     0x36

ctc_board_t ctc_bd_info;
spinlock_t ctc_gpio_g1_dir_lock;
spinlock_t ctc_gpio_g1_data_lock;

volatile void __iomem *gpio_g1_logic_addr;
volatile void __iomem *muxctrl_base_addr;

extern int xirq_gpio_1_23;
extern int xirq_gpio_1_24;
extern int xirq_gpio_1_25;

static int board_series=GLB_SERIES_E580;
module_param(board_series, int, S_IRUGO);
MODULE_PARM_DESC(board_series, "board series number");

static int board_type=GLB_BOARD_E580_24Q;
module_param(board_type, int, S_IRUGO);
MODULE_PARM_DESC(board_type, "board type number");

static int hardware_version=0x30;
module_param(hardware_version, int, S_IRUGO);
MODULE_PARM_DESC(hardware_version, "hardware version number");

int ctc_i2c_read(int devAddr, int regAddr, int regLen, unsigned char *data,  int dataLen)
{
	int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[1];
	unsigned char buf[4];

	adap = i2c_get_adapter(1);
	if (!adap)
		return -ENODEV;

	msg->addr = devAddr;	/* I2C address of chip */
	msg->flags = 0;
	msg->len = regLen;
	msg->buf = buf;
    if(regLen==1)
    {
        buf[0] = (unsigned char)(regAddr & 0xff);
    }
    else if(regLen==2)
    {
        buf[0] = (unsigned char)((regAddr >> 8) & 0xff);
        buf[1] = (unsigned char)((regAddr) & 0xff);
    }
    else if(regLen==3)
    {
        buf[0] = (unsigned char)((regAddr >> 16) & 0xff);
        buf[1] = (unsigned char)((regAddr >> 8) & 0xff);
        buf[2] = (unsigned char)((regAddr) & 0xff);
    }
    else
    {
        printk(KERN_ALERT "[%s:%d]: %s\n", __func__, __LINE__,"Invalid reg length!");
    }
	ret = i2c_transfer(adap, msg, 1);

	msg->addr = devAddr;	/* I2C address */
	msg->flags = I2C_M_RD;
	msg->len = dataLen;
	msg->buf = data;
	ret = i2c_transfer(adap, msg, 1);
	i2c_put_adapter(adap);
    
	return ret;
}
EXPORT_SYMBOL(ctc_i2c_read);

int ctc_i2c_write(int devAddr, int regAddr, int regLen, unsigned char data)
{
    int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[1];
	unsigned char buf[4];

	adap = i2c_get_adapter(1);
	if (!adap)
		return -ENODEV;
	msg->addr = devAddr;	/* I2C address of chip */
	msg->flags = 0;
	msg->len = regLen+1;
	msg->buf = buf;
    if(regLen==1)
    {
        buf[0] = (unsigned char)(regAddr & 0xff);
        buf[1] = data;
    }
    else if(regLen==2)
    {
        buf[0] = (unsigned char)((regAddr >> 8) & 0xff);
        buf[1] = (unsigned char)((regAddr) & 0xff);
        buf[2] = data;
    }
    else if(regLen==3)
    {
        buf[0] = (unsigned char)((regAddr >> 16) & 0xff);
        buf[1] = (unsigned char)((regAddr >> 8) & 0xff);
        buf[2] = (unsigned char)((regAddr) & 0xff);
        buf[3] = data;
    }
    else
    {
        printk(KERN_ALERT "[%s:%d]: %s\n", __func__, __LINE__,"Invalid reg length!");
    }
	ret = i2c_transfer(adap, msg, 1);
	i2c_put_adapter(adap);
	return ret;
}
EXPORT_SYMBOL(ctc_i2c_write);

u8 ctc_epld_read(u8 reg)
{
    u8 val;
    int epld_addr = CTC_EPLD_I2C_NEW_ADDR;
    if(GLB_SERIES_E550 == board_series)
    {
        if((GLB_BOARD_E550_24T16Y == board_type)||(GLB_BOARD_E550_24T16X == board_type))
        {
            if (hardware_version < 0x30)
            {
                epld_addr = CTC_EPLD_I2C_OLD_ADDR;
            }
        }
        else if((GLB_BOARD_E550_24X8Y2C == board_type)||(GLB_BOARD_E550_48T4Y2Q == board_type)
            ||(GLB_BOARD_E550_28M4Y2C == board_type)||(GLB_BOARD_E550_48M4Y2C == board_type))
        {
            epld_addr = CTC_EPLD_I2C_OLD_ADDR;
        }
    }
    ctc_i2c_read(epld_addr, reg, 1, &val, 1);

    return val;
}
EXPORT_SYMBOL(ctc_epld_read);

u8 ctc_epld_write(u8 reg, u8 val)
{
    int epld_addr = CTC_EPLD_I2C_NEW_ADDR;
    if(GLB_SERIES_E550 == board_series)
    {
        if((GLB_BOARD_E550_24T16Y == board_type)||(GLB_BOARD_E550_24T16X == board_type))
        {
            if (hardware_version < 0x30)
            {
                epld_addr = CTC_EPLD_I2C_OLD_ADDR;
            }
        }
        else if((GLB_BOARD_E550_24X8Y2C == board_type)||(GLB_BOARD_E550_48T4Y2Q == board_type)
            ||(GLB_BOARD_E550_28M4Y2C == board_type)||(GLB_BOARD_E550_48M4Y2C == board_type))
        {
            epld_addr = CTC_EPLD_I2C_OLD_ADDR;
        }        
    }
    return ctc_i2c_write(epld_addr, reg, 1, val);
    
}
EXPORT_SYMBOL(ctc_epld_write);

static void set_normal_irq_info(void)
{

    ctc_bd_info.normal_irq_count = 0;
    if((GLB_SERIES_E550 == ctc_bd_info.board_series) &&
        ((GLB_BOARD_E550_24T16X== ctc_bd_info.board_type)
        ||(GLB_BOARD_E550_24T16Y== ctc_bd_info.board_type)))
    {
        ctc_bd_info.normal_irq_count = 1;
        ctc_bd_info.normal_irq_0 = xirq_gpio_1_25;
    }
    else if ((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
                (GLB_BOARD_E530_48T4X2Q_D == ctc_bd_info.board_type))
    {
        ctc_bd_info.normal_irq_count = 2;
        ctc_bd_info.normal_irq_0 = xirq_gpio_1_24;
        ctc_bd_info.normal_irq_1 = xirq_gpio_1_25;
    }
}

static void set_watchdog_info(void)
{
    ctc_bd_info.watchdog_support = CTC_WD_CFG; 
    ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_GPIO;
    ctc_bd_info.watchdog_en_gpio = CTC_WD_EN;  
    ctc_bd_info.watchdog_feed_gpio = CTC_WD_FEED;  
    ctc_bd_info.gpio_date_offset = CTC_GPIO_DATA;
    ctc_bd_info.gpio_dir_offset  = CTC_GPIO_DIR;
}

static void set_reboot_info(void)
{
    ctc_bd_info.reboot_type = CTC_REBOOT_TYPE_POWEROFF;
}

static void set_reset_irq_info(void)
{
    ctc_bd_info.rst_irq = 0;
}

static void set_pwr_down_irq_info(void)
{
    ctc_bd_info.pwr_irq = 0;
}

static void set_dpll_irq_info(void)
{
    ctc_bd_info.dpll_irq = 0;
    if((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
        (GLB_BOARD_E530_24X2C_D== ctc_bd_info.board_type))
    {
        ctc_bd_info.dpll_irq = xirq_gpio_1_25;
    }    
}

static void ctc_set_board_info(void)
{
    set_normal_irq_info();
    set_watchdog_info();
    set_reboot_info();
    set_reset_irq_info();
    set_pwr_down_irq_info();
    set_dpll_irq_info();
}

static void ctc_ctl_board_led(void)
{
    u8 led_ctl = 0;

    ctc_bd_info.led_offset = 2;
    led_ctl = ctc_epld_read(ctc_bd_info.led_offset);
    led_ctl &= ~0xf0;
    led_ctl |= 0x90;
    ctc_epld_write(ctc_bd_info.led_offset, led_ctl);
}

static void ctc_release_device(void)
{
    u8 val;

    val = ctc_epld_read(0x8);
    val |= 0x3f;
    ctc_epld_write(0x8, val);

    val = ctc_epld_read(0x9);
    val |= 0xf;
    ctc_epld_write(0x9, val);
    
    val = ctc_epld_read(0x1d);
    val |= 0x3;
    ctc_epld_write(0x1d, val);
}

#if 0
static void ctc_intr_init(void)
{
    /* init gpio interrupt */
    #define GPIO_GPIER    0xc
    #define GPIO_GPIMR    0x10
    #define GPIO_GPICR    0x14
    
    volatile void __iomem *base;
    unsigned int val;
    
    base = gpio_g1_logic_addr;

    /* enable intr of gpio1-23/24/25 */
    val = readw(base);
    val &= ~(7<<23);
    writew(val, base);
    
    val = readw(base + GPIO_GPICR);
    val |= (7<<23);
    writew(val, base + GPIO_GPICR);

    val = readw(base + GPIO_GPIER);
    val |= (7<<23);
    writew(val, base + GPIO_GPIER);

    msleep(100);

    val = readw(base + GPIO_GPIMR);
    val |= (7<<23);
    writew(val, base + GPIO_GPIMR);
}
#endif
static int __init ctc_boards_init(void)
{ 
    gpio_g1_logic_addr = ioremap_nocache(0x2300000, 0x1000) ;

    if (gpio_g1_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.gpio_g1_logic_addr = gpio_g1_logic_addr;
    }

    spin_lock_init(&ctc_gpio_g1_dir_lock);
    spin_lock_init(&ctc_gpio_g1_data_lock);
    
    #if 0
    muxctrl_base_addr = ioremap(0x200f0000, 0x1000);
    
    if (muxctrl_base_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.muxctrl_base_addr = muxctrl_base_addr;
    }
    #endif
    
    ctc_bd_info.board_series = board_series;
    ctc_bd_info.board_type   = board_type;

    //printk(KERN_INFO "ctc-irq GPIO1_23 IRQ3 is %d\n", xirq_gpio_1_23);
    //printk(KERN_INFO "ctc-irq GPIO1_24 IRQ4 is %d\n", xirq_gpio_1_24);
    //printk(KERN_INFO "ctc-irq GPIO1_25 IRQ5 is %d\n", xirq_gpio_1_25);
    
    ctc_set_board_info();
    ctc_ctl_board_led();
    ctc_release_device();
    //ctc_intr_init();

    return 0;
}

static void __exit ctc_boards_exit(void)
{
    printk(KERN_ERR "ctc_bd_info.board_series              %d\n", ctc_bd_info.board_series           );
    printk(KERN_ERR "ctc_bd_info.board_type                %d\n", ctc_bd_info.board_type             );
    printk(KERN_ERR "ctc_bd_info.board_ver                 %d\n", ctc_bd_info.board_ver              );
    printk(KERN_ERR "ctc_bd_info.asic_chip_num             %d\n", ctc_bd_info.asic_chip_num          );
    printk(KERN_ERR "ctc_bd_info.normal_irq_count          %d\n", ctc_bd_info.normal_irq_count       );
    printk(KERN_ERR "ctc_bd_info.normal_irq_0              %d\n", ctc_bd_info.normal_irq_0           );
    printk(KERN_ERR "ctc_bd_info.normal_irq_1              %d\n", ctc_bd_info.normal_irq_1           );
    printk(KERN_ERR "ctc_bd_info.normal_irq_2              %d\n", ctc_bd_info.normal_irq_2           );
    printk(KERN_ERR "ctc_bd_info.dpll_irq                  %d\n", ctc_bd_info.dpll_irq               );
    printk(KERN_ERR "ctc_bd_info.normal_int0_offset        %d\n", ctc_bd_info.normal_int0_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int0_mask_offset   %d\n", ctc_bd_info.normal_int0_mask_offset);
    printk(KERN_ERR "ctc_bd_info.normal_int1_offset        %d\n", ctc_bd_info.normal_int1_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int1_mask_offset   %d\n", ctc_bd_info.normal_int1_mask_offset);
    printk(KERN_ERR "ctc_bd_info.normal_int2_offset        %d\n", ctc_bd_info.normal_int2_offset     );
    printk(KERN_ERR "ctc_bd_info.normal_int2_mask_offset   %d\n", ctc_bd_info.normal_int2_mask_offset);
    printk(KERN_ERR "ctc_bd_info.dpll_int_offset           %d\n", ctc_bd_info.dpll_int_offset        );
    printk(KERN_ERR "ctc_bd_info.dpll_int_mask_offset      %d\n", ctc_bd_info.dpll_int_mask_offset   );
    printk(KERN_ERR "ctc_bd_info.watchdog_support          %d\n", ctc_bd_info.watchdog_support       );
    printk(KERN_ERR "ctc_bd_info.watchdog_en_type          %d\n", ctc_bd_info.watchdog_en_type       );
    printk(KERN_ERR "ctc_bd_info.watchdog_en_gpio     %d\n", ctc_bd_info.watchdog_en_gpio  );
    printk(KERN_ERR "ctc_bd_info.watchdog_feed_gpio        %d\n", ctc_bd_info.watchdog_feed_gpio     );
    printk(KERN_ERR "ctc_bd_info.watchdog_is_feed          %d\n", ctc_bd_info.watchdog_is_feed       );
    printk(KERN_ERR "ctc_bd_info.gpio_date_offset          %d\n", ctc_bd_info.gpio_date_offset       );
    printk(KERN_ERR "ctc_bd_info.reboot_type               %d\n", ctc_bd_info.reboot_type            );
    printk(KERN_ERR "ctc_bd_info.led_offset                %d\n", ctc_bd_info.led_offset             );
    printk(KERN_ERR "ctc_bd_info.rst_irq                   %d\n", ctc_bd_info.rst_irq                );
    printk(KERN_ERR "ctc_bd_info.rst_int_mask              %d\n", ctc_bd_info.rst_int_mask           );
    printk(KERN_ERR "ctc_bd_info.rst_int_offset            %d\n", ctc_bd_info.rst_int_offset         );
    printk(KERN_ERR "ctc_bd_info.pwr_irq                   %d\n", ctc_bd_info.pwr_irq                );
    iounmap(gpio_g1_logic_addr);
}

EXPORT_SYMBOL(ctc_bd_info);
EXPORT_SYMBOL(ctc_gpio_g1_dir_lock);
EXPORT_SYMBOL(ctc_gpio_g1_data_lock);

module_init(ctc_boards_init);                                                                                                                   
module_exit(ctc_boards_exit);                                                                                                                   

MODULE_LICENSE("GPL");
