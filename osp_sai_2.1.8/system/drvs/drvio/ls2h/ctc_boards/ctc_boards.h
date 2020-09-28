#ifndef __CTC_BOARD_H__
#define __CTC_BOARD_H__

#include <ls2h/ls2h.h>
#include <ls2h/ls2h_int.h>

#define CTC_HW_NORMAL_IRQ_0     (LS2H_GPIO0_IRQ + 1)     /*GPIO1 for PHY intr */
#define CTC_HW_NORMAL_IRQ_1     (LS2H_GPIO0_IRQ + 2)     /*GPIO2 for DPLL intr */

#define CTC_WD_CFG            1
#define CTC_WD_EN               5    /* enable GPIO feed dog */
#define CTC_WD_FEED          7    /* feed dog GPIO */

#define RUN_LED_GPIO_N         10  /* RUN LED, 0:on, 1:off*/
#define ALARM_LED_GPIO_N    11  /* Alarm LED, 0:on, 1:off*/

#define LED_ON                  0
#define LED_OFF                1

/* board type Defines */
enum glb_board_series_e
{
    GLB_SERIES_SG8348C = 0x82, /* SG8348C */
    GLB_SERIES_VPX=0x84,/* RZVP-SW3 */
    GLB_SERIES_SGG = 0xa0,
    GLB_SERIES_MAX
};
typedef enum glb_board_series_e glb_board_series_t;

#define GLB_BOARD_SG8348C           1

enum ctc_reboot_type_e
{
    CTC_REBOOT_TYPE_POWEROFF = 0,
    CTC_REBOOT_TYPE_WD,
    CTC_REBOOT_TYPE_MAX,
} ;
typedef enum ctc_reboot_type_e ctc_reboot_type_t;

enum ctc_watchdog_en_type_e
{
    CTC_WATCHDOG_EPLD = 0,
    CTC_WATCHDOG_GPIO,
    CTC_WATCHDOG_MAX,
} ;
typedef enum ctc_watchdog_en_type_e ctc_watchdog_en_type_t;

struct ctc_board_s
{
    u8 board_series ;              /*board series*/
    u8 board_type ;                /*board type*/
    u8 board_ver ;                 /*board version*/
    u8 nor_type;                   /*nor flash type*/
    u8 nand_type;                  /*nor flash type*/
    u8 ddr_type;                   /*ddr type*/
    u8 asic_chip_num;              /*asic chip number*/
    u8 normal_irq_count;           /*normal irq count*/    
    u8 normal_irq_0;               /*normal irq 0 number*/    
    u8 normal_irq_1;               /*normal irq 1 number*/    
    u8 normal_irq_2;               /*normal irq 2 number*/    
    u8 dpll_irq;                   /* dpll irq number */
    u8 normal_int0_offset;         /*normal interrupt 0 offset*/    
    u8 normal_int0_mask_offset;    /*normal interrupt 0 mask offset*/    
    u8 normal_int1_offset;         /*normal interrupt 1 offset*/    
    u8 normal_int1_mask_offset;    /*normal interrupt 1 mask offset*/
    u8 normal_int2_offset;         /*normal interrupt 2 offset*/    
    u8 normal_int2_mask_offset;    /*normal interrupt 2 mask offset*/
    u8 dpll_int_offset;            /*normal interrupt 2 offset*/    
    u8 dpll_int_mask_offset;       /*normal interrupt 2 mask offset*/
    u8 watchdog_support;           /*whether support watch dog*/  
    u8 watchdog_en_type;           /*different type to enable the watchdog */
    u8 watchdog_en_io_offset;      /*the offset of port or epld reg*/
    u8 watchdog_en_gpio;             /*enable watch dog gpio*/
    u8 watchdog_feed_gpio;         /*feed watch dog gpio*/
    u8 watchdog_en_reverse; /* reverse the enable output */
    u8 watchdog_is_feed;           /*whether to feed watchdog, bug18151*/
    u8 gpio_date_offset;           /* gpio date register offset */	
    u8 gpio_dir_offset;            /* gpio direction register offset */	
    u8 reboot_type;
    u8 pre_reset_en;               /* 1 means need reset the others before reboot system, 0 means no pre-reset and the value of epld_rst_reg is invalid */
    u8 epld_rst_reg;               /* epld reset register, 0xff means invalid */
    u8 led_offset;                 /* kernel monitor led offset */
    u8 rst_irq;                    /* irq0~15 is reserved, so we choose 0 to be invalid irq */
    u8 rst_int_mask;
    u8 rst_int_offset;
    u8 pwr_irq;                    /* irq0~15 is reserved, so we choose 0 to be invalid irq */
    u8 kernel_monitor_en;
    u8 run_led_gpio;  /*run led gpio, 0:on*/
    u8 alarm_led_gpio; /*alarm led gpio, 0:on*/
    u8 normal_irq_type;/* 0 mask by epld, 1 mask by cpu */
    u8 gpio_i2c_scl;
    u8 gpio_i2c_sda;
    u8 fast_boot_en;   /* fix bug47412, support fastboot, liangf, 2018-05-26*/
};
typedef struct ctc_board_s ctc_board_t;

#endif
