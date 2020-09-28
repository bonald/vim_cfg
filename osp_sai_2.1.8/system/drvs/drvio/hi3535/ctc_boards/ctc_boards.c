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
#include <asm/delay.h>
#include <linux/delay.h>
#include "ctc_boards.h"
#include "mach/i2c-hi3535.h"

#define CTC_EPLD_I2C_ADDR     0x58

ctc_board_t ctc_bd_info;
void __iomem *gpio_g0_logic_addr;
void __iomem *gpio_g1_logic_addr;
void __iomem *gpio_g7_logic_addr;
void __iomem *muxctrl_base_addr;

static int board_series=GLB_SERIES_E580;
module_param(board_series, int, S_IRUGO);
MODULE_PARM_DESC(board_series, "board series number");

static int board_type=GLB_BOARD_E580_24Q;
module_param(board_type, int, S_IRUGO);
MODULE_PARM_DESC(board_type, "board type number");

u8 ctc_epld_read(u8 reg)
{
    u8 val;
    
    ctc_i2c_read(CTC_EPLD_I2C_ADDR, reg, 1, &val,  1);

    return val;
}
EXPORT_SYMBOL(ctc_epld_read);

u8 ctc_epld_write(u8 reg, u8 val)
{
    return ctc_i2c_write(CTC_EPLD_I2C_ADDR, reg, 1, val);
}
EXPORT_SYMBOL(ctc_epld_write);

static void set_normal_irq_info(void)
{
    if((GLB_SERIES_E550 == ctc_bd_info.board_series) && 
        (GLB_BOARD_E550_48T4Y2Q == ctc_bd_info.board_type))
    {
        ctc_bd_info.normal_irq_count = 1;
        ctc_bd_info.normal_irq_0 = 92;
    }
    else
    {
        ctc_bd_info.normal_irq_count = 0;
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
    if((GLB_SERIES_E550 == ctc_bd_info.board_series) &&
        (GLB_BOARD_E550_24X8Y2C== ctc_bd_info.board_type))
    {
        ctc_bd_info.dpll_irq = 92;
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

static void ctc_intr_init(void)
{
    /* init gpio interrupt */
    #define HI3535_GPIO_IS    0x404
    #define HI3535_GPIO_IBE   0x408
    #define HI3535_GPIO_IEV   0x40c
    #define HI3535_GPIO_IE    0x410
    #define HI3535_GPIO_IC    0x41c
    
    void __iomem *base;
    unsigned int val;
    
    base = gpio_g1_logic_addr;
    
    val = readw(base + HI3535_GPIO_IS);
    val |= (3<<6);
    writew(val, base + HI3535_GPIO_IS);

    val = readw(base + HI3535_GPIO_IEV);
    val &= ~(3<<6);
    writew(val, base + HI3535_GPIO_IEV);

    msleep(100);

    writew(0xff, base + HI3535_GPIO_IC);

    val = readw(base + HI3535_GPIO_IE);
    val |= (3<<6);
    writew(val, base + HI3535_GPIO_IE);
}

static void ctc_phy_intr_init(void)
{
    /* init gpio interrupt */
    #define HI3535_GPIO_IS    0x404
    #define HI3535_GPIO_IBE   0x408
    #define HI3535_GPIO_IEV   0x40c
    #define HI3535_GPIO_IE    0x410
    #define HI3535_GPIO_IC    0x41c
    #define muxctrl_reg67        0x10c
    
    void __iomem *base;
    unsigned int val;
    
    base = muxctrl_base_addr;
    
    val = readw(base + muxctrl_reg67);
    val &= ~1;
    writew(val, base + muxctrl_reg67);

    base = gpio_g7_logic_addr;
    
    val = readw(base + HI3535_GPIO_IS);
    val |= 1;
    writew(val, base + HI3535_GPIO_IS);

    val = readw(base + HI3535_GPIO_IEV);
    val &= ~1;
    writew(val, base + HI3535_GPIO_IEV);

    msleep(100);

    writew(0xff, base + HI3535_GPIO_IC);

    val = readw(base + HI3535_GPIO_IE);
    val |= (0x1);
    writew(val, base + HI3535_GPIO_IE);
}

static void ctc_dpll_intr_init(void)
{
    /* init gpio interrupt */
    #define HI3535_GPIO_IS    0x404
    #define HI3535_GPIO_IBE   0x408
    #define HI3535_GPIO_IEV   0x40c
    #define HI3535_GPIO_IE    0x410
    #define HI3535_GPIO_IC    0x41c
    #define muxctrl_reg67        0x10c
    
    void __iomem *base;
    unsigned int val;
    
    base = muxctrl_base_addr;
    
    val = readw(base + muxctrl_reg67);
    val &= ~1;
    writew(val, base + muxctrl_reg67);

    base = gpio_g7_logic_addr;
    
    val = readw(base + HI3535_GPIO_IS);
    val |= 1;
    writew(val, base + HI3535_GPIO_IS);

    val = readw(base + HI3535_GPIO_IEV);
    val &= ~1;
    writew(val, base + HI3535_GPIO_IEV);

    msleep(100);

    writew(0xff, base + HI3535_GPIO_IC);

    val = readw(base + HI3535_GPIO_IE);
    val |= (0x1);
    writew(val, base + HI3535_GPIO_IE);
}

static int __init ctc_boards_init(void)
{ 
    gpio_g0_logic_addr = ioremap(0x20150000, 0x1000) ;

    if (gpio_g0_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.gpio_g0_logic_addr = gpio_g0_logic_addr;
    }

    gpio_g1_logic_addr = ioremap(0x20160000, 0x1000) ;

    if (gpio_g1_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.gpio_g1_logic_addr = gpio_g1_logic_addr;
    }
    
    gpio_g7_logic_addr = ioremap(0x201c0000, 0x1000) ;
    
    if (gpio_g7_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.gpio_g7_logic_addr = gpio_g7_logic_addr;
    }
    
    muxctrl_base_addr = ioremap(0x200f0000, 0x1000);
    
    if (muxctrl_base_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.muxctrl_base_addr = muxctrl_base_addr;
    }

    ctc_bd_info.board_series = board_series;
    ctc_bd_info.board_type   = board_type;

    ctc_set_board_info();
    ctc_ctl_board_led();
    ctc_release_device();
    ctc_intr_init();

    if((GLB_SERIES_E550 == ctc_bd_info.board_series) &&         ///TODO:
        (GLB_BOARD_E550_48T4Y2Q == ctc_bd_info.board_type))
    {
        ctc_phy_intr_init();
    }
    
    if((GLB_SERIES_E550 == ctc_bd_info.board_series) &&
        (GLB_BOARD_E550_24X8Y2C== ctc_bd_info.board_type))
    {
        ctc_dpll_intr_init();
    }
        
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
    iounmap(gpio_g0_logic_addr);
    iounmap(gpio_g1_logic_addr);
    iounmap(gpio_g7_logic_addr);
    iounmap(muxctrl_base_addr);
}

EXPORT_SYMBOL(ctc_bd_info);

module_init(ctc_boards_init);                                                                                                                   
module_exit(ctc_boards_exit);                                                                                                                   

MODULE_LICENSE("GPL");
