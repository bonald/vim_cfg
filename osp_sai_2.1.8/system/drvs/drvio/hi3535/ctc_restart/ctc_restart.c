#include <asm/types.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/module.h>
#include "mach/i2c-hi3535.h"
#include "ctc_boards.h"
#include "ctc_restart.h"

#define CTC_EPLD_I2C_ADDR     0x58
#define CTC_EPLD_PWROFF_REG   0x23

extern ctc_board_t ctc_bd_info;
extern void (*arm_pm_restart)(char str, const char *cmd);

u8 ctc_epld_read(u8 reg)
{
    u8 val;
    
    ctc_i2c_read(CTC_EPLD_I2C_ADDR, reg, 1, &val,  1);

    return val;
}

u8 ctc_epld_write(u8 reg, u8 val)
{
    return ctc_i2c_write(CTC_EPLD_I2C_ADDR, reg, 1, val);
}

void ctc_arm_restart(char mode, const char *cmd)
{
	if(0xfe == ctc_epld_read(0xf6))
	{
		printk(KERN_INFO "WRITE 0x08 means ABNORMAL reboot!\n");
		ctc_epld_write(0xf6, 0x08);
	}
    
    ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x0);
    mdelay(200);
    ctc_epld_write(CTC_EPLD_PWROFF_REG, 0x3);

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

