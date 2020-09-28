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

ctc_board_t ctc_bd_info;
void __iomem *epld_logic_addr;
void __iomem *gpio_logic_addr;

static int board_series=GLB_SERIES_E580;
module_param(board_series, int, S_IRUGO);
MODULE_PARM_DESC(board_series, "board series number");

static int board_type=GLB_BOARD_E580_24Q;
module_param(board_type, int, S_IRUGO);
MODULE_PARM_DESC(board_type, "board type number");

static void set_normal_irq_info(void)
{
    /* for e580 series */
    if(GLB_SERIES_E580 == ctc_bd_info.board_series) 
    {
        if(GLB_BOARD_E580_48XT6Q == ctc_bd_info.board_type)
        {
            ctc_bd_info.normal_irq_count= 1;  
            ctc_bd_info.normal_irq_0 = 18;
            ctc_bd_info.normal_int0_offset = 0xa;       
            ctc_bd_info.normal_int1_offset = 0xc;       
            ctc_bd_info.normal_int0_mask_offset = 0xb;           
            ctc_bd_info.normal_int1_mask_offset = 0xd;   
        }
        else if (GLB_BOARD_E580_WQT48XT4Q == ctc_bd_info.board_type)
        {
            ctc_bd_info.normal_irq_count= 1;  
            ctc_bd_info.normal_irq_0 = 18;
            ctc_bd_info.normal_irq_type = 1;
        }
        else
        {
            ctc_bd_info.normal_irq_count= 0;  
        }
    }
    else if(GLB_SERIES_P580 == ctc_bd_info.board_series)
    {
        if(GLB_BOARD_T65C_8T64X6Q == ctc_bd_info.board_type)
        {
            ctc_bd_info.normal_irq_count= 1;  
            ctc_bd_info.normal_irq_0 = 18;
            ctc_bd_info.normal_int0_offset = 0xa;       
            ctc_bd_info.normal_int0_mask_offset = 0xb;           
            
            ctc_bd_info.dpll_support = 1;              
            ctc_bd_info.dpll_irq = 19;     
            ctc_bd_info.dpll_int_offset = 0xc;       
            ctc_bd_info.dpll_int_mask_offset = 0xd;  
        }
        else
        {
            ctc_bd_info.normal_irq_count= 0;  
        }
    }
    /* for e350 series */
    else if(GLB_SERIES_E350 == ctc_bd_info.board_series)
    {
        if(GLB_BOARD_E350_PEGATRON == ctc_bd_info.board_type)
        {
            ctc_bd_info.normal_irq_count = 1;  
            ctc_bd_info.normal_irq_0 = 22;
            ctc_bd_info.normal_irq_type = 1;            
        }
        else if ((GLB_BOARD_E350_48T4X2Q == ctc_bd_info.board_type)
            || (GLB_BOARD_E350_48T4XG == ctc_bd_info.board_type)
            || (GLB_BOARD_V350_48P4X == ctc_bd_info.board_type)
            || (GLB_BOARD_E350_HF_12T4S8X == ctc_bd_info.board_type)
            || (GLB_BOARD_V350_24P4X == ctc_bd_info.board_type))
        {
            ctc_bd_info.normal_int0_offset = CTC_INT0_STA_OFFSET;       
            ctc_bd_info.normal_int0_mask_offset = CTC_INT0_MSK_OFFSET;           
            ctc_bd_info.normal_irq_count= 1;  
            ctc_bd_info.normal_irq_0 = CTC_INT_0;  
            ctc_bd_info.normal_irq_type = 0;
        }
        else if ((GLB_BOARD_E350_8TS12X == ctc_bd_info.board_type)
            ||  (GLB_BOARD_E350_65DV1 == ctc_bd_info.board_type))
        {            
            ctc_bd_info.normal_int0_offset = 0x10;                   
            ctc_bd_info.normal_int0_mask_offset = 0x13;                       
            ctc_bd_info.normal_irq_count= 1;              
            ctc_bd_info.normal_irq_0 = 18;
            ctc_bd_info.normal_irq_type = 0;
        } 
        else
        {
            ctc_bd_info.normal_irq_count= 0;  
        }
    }
}

static void set_watchdog_info(void)
{
    if((GLB_SERIES_P580 == ctc_bd_info.board_series)||(GLB_SERIES_E580 == ctc_bd_info.board_series))
    {
        if((GLB_BOARD_P580_HW_48X2Q4Z == ctc_bd_info.board_type)
          ||(GLB_BOARD_E580_HW_48X4Q4T == ctc_bd_info.board_type)
          ||(GLB_BOARD_E580_HW_48X6Q == ctc_bd_info.board_type))
        {
            ctc_bd_info.watchdog_support = CTC_WD_CFG;    
            ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_EPLD;
            ctc_bd_info.watchdog_en_io_offset = 0x35;
            ctc_bd_info.watchdog_feed_gpio = CTC_WD_FEED;  
            ctc_bd_info.gpio_date_offset = CTC_GPIO_DATA;
            ctc_bd_info.watchdog_en_reverse = 0;
        }
        else
        {
            ctc_bd_info.watchdog_support = CTC_WD_CFG; 
            ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_GPIO;
            ctc_bd_info.watchdog_en_io_offset = CTC_WD_EN;  
            ctc_bd_info.watchdog_feed_gpio = CTC_WD_FEED;  
            ctc_bd_info.gpio_date_offset = CTC_GPIO_DATA;
            ctc_bd_info.watchdog_en_reverse = 0;
        }
    }
    else if(GLB_SERIES_E350 == ctc_bd_info.board_series)
    {
        if(GLB_BOARD_E350_PEGATRON == ctc_bd_info.board_type)
        {
            ctc_bd_info.watchdog_support = 0;
        }
        else
        {
            ctc_bd_info.watchdog_support = CTC_WD_CFG; 
            ctc_bd_info.watchdog_en_type = CTC_WATCHDOG_GPIO;
            ctc_bd_info.watchdog_en_io_offset = CTC_WD_EN;  
            ctc_bd_info.watchdog_feed_gpio = CTC_WD_FEED;  
            ctc_bd_info.gpio_date_offset = CTC_GPIO_DATA;
            ctc_bd_info.watchdog_en_reverse = 1;
            if((GLB_BOARD_E350_8TS12X == ctc_bd_info.board_type)
                ||(GLB_BOARD_E350_65DV1 == ctc_bd_info.board_type))
            {
                ctc_bd_info.watchdog_en_reverse = 0;
            }
        }        
    }
}

static void set_reboot_info(void)
{
    if(((GLB_SERIES_P580 == ctc_bd_info.board_series)
        && (GLB_BOARD_P580_HW_48X2Q4Z == ctc_bd_info.board_type))
        || ((GLB_SERIES_E580 == ctc_bd_info.board_series)
        && (GLB_BOARD_E580_HW_48X4Q4T == ctc_bd_info.board_type))
        || ((GLB_SERIES_E580 == ctc_bd_info.board_series)
        && (GLB_BOARD_E580_HW_48X6Q == ctc_bd_info.board_type)))
    {
        ctc_bd_info.pre_reset_en = 1; 
        ctc_bd_info.epld_rst_reg = 0x34;
        ctc_bd_info.reboot_type = CTC_REBOOT_TYPE_WD;
    }
    else if(((GLB_SERIES_E580 == ctc_bd_info.board_series)
            && (GLB_BOARD_E580_WQT48X4Q == ctc_bd_info.board_type))
            || ((GLB_SERIES_E350 == ctc_bd_info.board_series)
            && (GLB_BOARD_E350_PEGATRON != ctc_bd_info.board_type))) 
    {   
        ctc_bd_info.pre_reset_en = 0; 
        ctc_bd_info.epld_rst_reg = 0xff;
        ctc_bd_info.reboot_type = CTC_REBOOT_TYPE_WD;
    }
    else
    {
        ctc_bd_info.pre_reset_en = 0; 
        ctc_bd_info.epld_rst_reg = 0xff;
        ctc_bd_info.reboot_type = CTC_REBOOT_TYPE_POWEROFF;
    }
}

static void set_reset_irq_info(void)
{
#define WQT_RESET_IRQ             19
#define WQT_RESET_INTR_MASK_REG   0xd

    if((GLB_SERIES_E580 == ctc_bd_info.board_series) && 
        (GLB_BOARD_E580_WQT48X4Q == ctc_bd_info.board_type))
    {   
        ctc_bd_info.rst_irq = WQT_RESET_IRQ;
        ctc_bd_info.rst_int_mask = WQT_RESET_INTR_MASK_REG;
        ctc_bd_info.rst_int_offset = 4;
    }
    else if((GLB_SERIES_E580 == ctc_bd_info.board_series) &&
        (GLB_BOARD_E580_WQT48XT4Q == ctc_bd_info.board_type))
    {
        ctc_bd_info.rst_irq = WQT_RESET_IRQ;
        ctc_bd_info.rst_int_mask = WQT_RESET_INTR_MASK_REG;
        ctc_bd_info.rst_int_offset = 5;
    }
    else
    {
        ctc_bd_info.rst_irq = 0;
    }
}

static void set_pwr_down_irq_info(void)
{
#define HW_PWR_DOWN_IRQ     20

    if(((GLB_SERIES_P580 == ctc_bd_info.board_series)
        && (GLB_BOARD_P580_HW_48X2Q4Z == ctc_bd_info.board_type))
        || ((GLB_SERIES_E580 == ctc_bd_info.board_series)
        && (GLB_BOARD_E580_HW_48X4Q4T == ctc_bd_info.board_type))
        || ((GLB_SERIES_E580 == ctc_bd_info.board_series)
        && (GLB_BOARD_E580_HW_48X6Q == ctc_bd_info.board_type)))
    {
        ctc_bd_info.pwr_irq = HW_PWR_DOWN_IRQ;
    }
    else
    {
        ctc_bd_info.pwr_irq = 0;
    }
}

static void ctc_set_board_info(void)
{
    set_normal_irq_info();
    set_watchdog_info();
    set_reboot_info();
    set_reset_irq_info();
    set_pwr_down_irq_info();
}

static void ctc_ctl_board_led(void)
{
    u8 led_ctl = 0;

    ctc_bd_info.led_offset = 0x2;
    /* system led, amber blinkingSlow 0.5Hz*/
    led_ctl = ioread8(epld_logic_addr + ctc_bd_info.led_offset);
    led_ctl &= ~0xf0;
    led_ctl |= 0x90;
    iowrite8(led_ctl, epld_logic_addr + ctc_bd_info.led_offset);
}


#if 0
static void ctc_release_device(void)
{
    u8 val;

    val = ioread8(epld_logic_addr + 0x8);
    val |= 0x3f;
    iowrite8(val, epld_logic_addr + 0x8);

    val = ioread8(epld_logic_addr + 0x9);
    val |= 0xf;
    iowrite8(val, epld_logic_addr + 0x9);

    val = ioread8(epld_logic_addr + 0x1d);
    val |= 0x3;
    iowrite8(val, epld_logic_addr + 0x1d);
}
#endif

static int __init ctc_boards_init(void)
{ 
    epld_logic_addr = ioremap(0xffb00000, 0x1000) ;

    if (epld_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map epld space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.epld_logic_addr = epld_logic_addr;
    }
    gpio_logic_addr = ioremap(0xffe0f000, 0x1000) ;

    if (gpio_logic_addr == NULL)
    {
        printk(KERN_ERR "#########Can't map gpio space!##########\n");              
    }
    else 
    {          
        ctc_bd_info.gpio_logic_addr = gpio_logic_addr;
    }

    ctc_bd_info.board_series = board_series;
    ctc_bd_info.board_type   = board_type;

    /* modified by liuht for bug 40668, 2016-09-09 */
    if((ctc_bd_info.board_series == GLB_SERIES_E350)&&(GLB_BOARD_E350_PEGATRON == ctc_bd_info.board_type))
    {
        void __iomem *cfg_addr;
        /* select i2c channel 0 */
        cfg_addr = ioremap(0xffee0064, 0x1000);
        out_be32(cfg_addr, 0x4000000);
        out_be32(gpio_logic_addr, 0xd8280000);
        /* modified by liuht for bug 40668, 2016-09-09 */
        /* disable i2c mux and enable usb */
        out_be32(gpio_logic_addr+8, 0x00280000);
        iounmap(cfg_addr);
    }

    ctc_set_board_info();
    ctc_ctl_board_led();
    //ctc_release_device(); //do it in lcm board init
    
    return 0;
}

static void __exit ctc_boards_exit(void)
{
    printk(KERN_ERR "ctc_bd_info.board_series              %d\n", ctc_bd_info.board_series           );
    printk(KERN_ERR "ctc_bd_info.board_type                %d\n", ctc_bd_info.board_type             );
    printk(KERN_ERR "ctc_bd_info.board_ver                 %d\n", ctc_bd_info.board_ver              );
    printk(KERN_ERR "ctc_bd_info.nor_type                  %d\n", ctc_bd_info.nor_type               );
    printk(KERN_ERR "ctc_bd_info.nand_type                 %d\n", ctc_bd_info.nand_type              );
    printk(KERN_ERR "ctc_bd_info.ddr_type                  %d\n", ctc_bd_info.ddr_type               );
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
    printk(KERN_ERR "ctc_bd_info.watchdog_en_io_offset     %d\n", ctc_bd_info.watchdog_en_io_offset  );
    printk(KERN_ERR "ctc_bd_info.watchdog_feed_gpio        %d\n", ctc_bd_info.watchdog_feed_gpio     );
    printk(KERN_ERR "ctc_bd_info.watchdog_is_feed          %d\n", ctc_bd_info.watchdog_is_feed       );
    printk(KERN_ERR "ctc_bd_info.gpio_date_offset          %d\n", ctc_bd_info.gpio_date_offset       );
    printk(KERN_ERR "ctc_bd_info.reboot_type               %d\n", ctc_bd_info.reboot_type            );
    printk(KERN_ERR "ctc_bd_info.pre_reset_en              %d\n", ctc_bd_info.pre_reset_en           );
    printk(KERN_ERR "ctc_bd_info.epld_rst_reg              %d\n", ctc_bd_info.epld_rst_reg           );
    printk(KERN_ERR "ctc_bd_info.led_offset                %d\n", ctc_bd_info.led_offset             );
    printk(KERN_ERR "ctc_bd_info.rst_irq                   %d\n", ctc_bd_info.rst_irq                );
    printk(KERN_ERR "ctc_bd_info.rst_int_mask              %d\n", ctc_bd_info.rst_int_mask           );
    printk(KERN_ERR "ctc_bd_info.rst_int_offset            %d\n", ctc_bd_info.rst_int_offset         );
    printk(KERN_ERR "ctc_bd_info.pwr_irq                   %d\n", ctc_bd_info.pwr_irq                );
    iounmap(epld_logic_addr);
    iounmap(gpio_logic_addr);
}

EXPORT_SYMBOL(ctc_bd_info);

module_init(ctc_boards_init);                                                                                                                   
module_exit(ctc_boards_exit);                                                                                                                   

MODULE_LICENSE("GPL");
