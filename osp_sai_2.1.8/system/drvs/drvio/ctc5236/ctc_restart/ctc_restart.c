#include <asm/types.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include "ctc_boards.h"
#include "ctc_restart.h"

#define CTC_EPLD_PWROFF_REG   0x23

extern ctc_board_t ctc_bd_info;
extern int ctc_i2c_read(unsigned char bus,int devAddr, int regAddr, int regLen, unsigned char *data,  int dataLen);
extern int ctc_i2c_write(unsigned char bus,int devAddr, int regAddr, int regLen, unsigned char data);
extern u8 ctc_epld_read(u8 reg);
extern u8 ctc_epld_write(u8 reg, u8 val);
extern u8 ctc_epld_write(u8 reg, u8 val);
extern u8 ctc_gpio_direction_config(u8 gpio_pin,u8 dir,u8 default_out);


extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);
extern void do_kernel_restart(char *cmd);

void ctc_arm_restart(enum reboot_mode mode, const char *cmd)
{
    u8 val;
cpu_reset:    
    if(GLB_SERIES_E530 == ctc_bd_info.board_series)
    {
        if (GLB_BOARD_E530_48T4X2Q_D== ctc_bd_info.board_type)
        {
            printk(KERN_INFO "Reboot system by writing I2C\n");
            ctc_i2c_read(0,0x22, 0x2, 1, &val, 1);
            val &= 0xf7;
            ctc_i2c_write(0,0x22, 0x2, 1, val);
            mdelay(1000);
            val |= 0x08;
            ctc_i2c_write(0,0x22, 0x2, 1, val);
            mdelay(1000);
        }
        else if((GLB_BOARD_E530_24X2C_D == ctc_bd_info.board_type)
            ||(GLB_BOARD_E530_48X == ctc_bd_info.board_type)
            ||(GLB_BOARD_E530_48S4X == ctc_bd_info.board_type))
        {
            printk(KERN_INFO "Reboot system by writing EPLD\n");
        	if(0xfe == ctc_epld_read(0xf6))
        	{
                /* for bug 48447, remove invalid log, the is not usefull on e550 board */
        		//printk(KERN_INFO "WRITE 0x08 means ABNORMAL reboot!\n");
        		ctc_epld_write(0xf6, 0x08);
        	}  
            ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x0);
            mdelay(200);
            ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x3);
        }
        else
        {
            printk(KERN_INFO "Reboot system by writing GPIO\n");
            ctc_gpio_direction_config(6, 0, 1);
            mdelay(200);
        }
    }
    /* for bug 52303, use watchdog for ensure reboot,when get here, the reboot info is prepared */
    if(!ctc_bd_info.watchdog_support)
    {
        mdelay(1000);
        goto cpu_reset;
    }    
    /* call kernel function */
    printk(KERN_INFO "Reboot system by watchdog\n");
    do_kernel_restart(cmd);
    
    while(1);
}

static int __init ctc_restart_init(void)
{ 
    
    arm_pm_restart = ctc_arm_restart;

    return 0;
}

static void __exit ctc_restart_exit(void)
{

}

module_init(ctc_restart_init);                                                                                                                   
module_exit(ctc_restart_exit); 
MODULE_LICENSE("GPL");

