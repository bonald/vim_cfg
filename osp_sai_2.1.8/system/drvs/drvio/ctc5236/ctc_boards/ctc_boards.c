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
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "ctc_boards.h"
#include <linux/gpio.h>

#define CTC_EPLD_I2C_OLD_ADDR     0x58
#define CTC_EPLD_I2C_NEW_ADDR     0x36
#define CTC_GPIO_BASE         496

int xirq_gpio_1 = 0;
int xirq_gpio_0 = 0;
int xirq_gpio_6 = 0;  /* add for E530_48S4X EPLD INT0*/
int xirq_gpio_7 = 0;  /* add for E530_48S4X EPLD INT1*/
int xirq_gpio_15 = 0;

ctc_board_t ctc_bd_info;

static int board_series=GLB_SERIES_E530;
module_param(board_series, int, S_IRUGO);
MODULE_PARM_DESC(board_series, "board series number");

static int board_type=GLB_BOARD_E530_24X2C_D;
module_param(board_type, int, S_IRUGO);
MODULE_PARM_DESC(board_type, "board type number");

static int hardware_version=0x10;
module_param(hardware_version, int, S_IRUGO);
MODULE_PARM_DESC(hardware_version, "hardware version number");

int ctc_i2c_read(unsigned char bus,int devAddr, int regAddr, int regLen, unsigned char *data,  int dataLen)
{
	int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[1];
	unsigned char buf[4];

	adap = i2c_get_adapter(bus);
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

int ctc_i2c_write(unsigned char bus,int devAddr, int regAddr, int regLen, unsigned char data)
{
    int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[1];
	unsigned char buf[4];

	adap = i2c_get_adapter(bus);
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
        if(0)
        {
            epld_addr = CTC_EPLD_I2C_OLD_ADDR;
        }
        else
        {
            epld_addr = CTC_EPLD_I2C_NEW_ADDR;
        }
    }
    ctc_i2c_read(0,epld_addr, reg, 1, &val, 1);

    return val;
}
EXPORT_SYMBOL(ctc_epld_read);

u8 ctc_epld_write(u8 reg, u8 val)
{
    int epld_addr = CTC_EPLD_I2C_NEW_ADDR;
    if(GLB_SERIES_E550 == board_series)
    {
        if(0)
        {
            epld_addr = CTC_EPLD_I2C_OLD_ADDR;
        }
        else
        {
            epld_addr = CTC_EPLD_I2C_NEW_ADDR;
        }
    }
    return ctc_i2c_write(0,epld_addr, reg, 1, val);
    
}
EXPORT_SYMBOL(ctc_epld_write);

u8 ctc_gpio_set(u8 gpio_pin,u8 val)
{
    gpio_set_value_cansleep(gpio_pin+CTC_GPIO_BASE, val);
    return 0;
}
EXPORT_SYMBOL(ctc_gpio_set);

u8 ctc_gpio_get(u8 gpio_pin)
{
    return gpio_get_value_cansleep(gpio_pin+CTC_GPIO_BASE);
}
EXPORT_SYMBOL(ctc_gpio_get);

u8 ctc_gpio_direction_config(u8 gpio_pin,u8 dir,u8 default_out)
{
    return dir?gpio_direction_input(gpio_pin+CTC_GPIO_BASE):gpio_direction_output(gpio_pin+CTC_GPIO_BASE,default_out);
}
EXPORT_SYMBOL(ctc_gpio_direction_config);

static void set_normal_irq_info(void)
{

    ctc_bd_info.normal_irq_count = 0;
    if ((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
        ((GLB_BOARD_E530_48T4X2Q_D == ctc_bd_info.board_type) || 
            (GLB_BOARD_E530_48T4X == ctc_bd_info.board_type) ||
             (GLB_BOARD_E530_48P4X == ctc_bd_info.board_type)))
    {
        ctc_bd_info.normal_irq_count = 2;
        ctc_bd_info.normal_irq_0 = xirq_gpio_0;
        ctc_bd_info.normal_irq_1 = xirq_gpio_1;
    } 
     if ((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
        (GLB_BOARD_E530_48S4X== ctc_bd_info.board_type ))
    {
        ctc_bd_info.normal_irq_count = 1;
        ctc_bd_info.normal_irq_0 = xirq_gpio_0;
    }    
}

static void set_watchdog_info(void)
{
    ctc_bd_info.watchdog_support = CTC_WD_CFG; 
    ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_SP805;
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
        ctc_bd_info.dpll_irq = xirq_gpio_1;
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

static void ctc_pincrtl_init(void)
{
    /* do tsingma soc gpio init */
    if((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
        ((GLB_BOARD_E530_48T4X == ctc_bd_info.board_type)
        ||(GLB_BOARD_E530_48T4X2Q_D == ctc_bd_info.board_type)
        ||(GLB_BOARD_E530_48P4X== ctc_bd_info.board_type)))
    {
         /* configure mgmt-phy reset-pin output on product, mgmt-phy release must before this */
        ctc_gpio_direction_config(4, 0, 1);
        /* configure power-up pin output on product */
        ctc_gpio_direction_config(6, 0, 0);
        /* configure phy interrupt pin input */
        ctc_gpio_direction_config(0, 1, 0);
        ctc_gpio_direction_config(1, 1, 0);
        /* configure phy reset-pin output, for release phy */
        ctc_gpio_direction_config(5, 0, 1);
    }

    if((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
    (GLB_BOARD_E530_48S4X == ctc_bd_info.board_type))
    {
        /* configure phy interrupt pin input */
        ctc_gpio_direction_config(0, 1, 0);
        /* configure epld interrupt pin input */
        ctc_gpio_direction_config(6, 1, 0); 
        ctc_gpio_direction_config(7, 1, 0);
    }
    return;
}

static int ctc_irq_init(void)
{
    struct device_node *xnp;
    for_each_node_by_type(xnp, "ctc-irq") {
        if (of_device_is_compatible(xnp, "centec,ctc-irq"))
        {
            xirq_gpio_0 = irq_of_parse_and_map(xnp, 0);
            printk(KERN_INFO "ctc-irq GPIO0 IRQ is %d\n", xirq_gpio_0);  
            xirq_gpio_1 = irq_of_parse_and_map(xnp, 1);
            printk(KERN_INFO "ctc-irq GPIO1 IRQ is %d\n", xirq_gpio_1);             
            xirq_gpio_15 = irq_of_parse_and_map(xnp, 2);
            printk(KERN_INFO "ctc-irq GPIO15 IRQ is %d\n", xirq_gpio_15);

            if((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
              (GLB_BOARD_E530_48S4X == ctc_bd_info.board_type))
            {
                xirq_gpio_6 = irq_of_parse_and_map(xnp, 3);
                printk(KERN_INFO "ctc-irq GPIO6 IRQ is %d\n", xirq_gpio_6);
                xirq_gpio_7 = irq_of_parse_and_map(xnp, 4);
                printk(KERN_INFO "ctc-irq GPIO7 IRQ is %d\n", xirq_gpio_7);
            }
        }
    }
    return 0;
}
static int __init ctc_boards_init(void)
{ 
    ctc_bd_info.board_series = board_series;
    ctc_bd_info.board_type   = board_type;

    ctc_irq_init();
    ctc_set_board_info();
    
    /* Only board GLB_BOARD_E530_24X2C_D and GLB_BOARD_E530_48X have epld */
    if((GLB_SERIES_E530 == ctc_bd_info.board_series) &&
        ((GLB_BOARD_E530_24X2C_D== ctc_bd_info.board_type)
        ||(GLB_BOARD_E530_48X== ctc_bd_info.board_type))) 
    {
        ctc_ctl_board_led();
        ctc_release_device();
    }
    
    ctc_pincrtl_init();

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
}

EXPORT_SYMBOL(ctc_bd_info);
EXPORT_SYMBOL(xirq_gpio_1);
EXPORT_SYMBOL(xirq_gpio_15);

module_init(ctc_boards_init);                                                                                                                   
module_exit(ctc_boards_exit);                                                                                                                   

MODULE_LICENSE("GPL");
