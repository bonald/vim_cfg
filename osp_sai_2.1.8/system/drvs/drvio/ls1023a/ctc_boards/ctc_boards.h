#ifndef __CTC_BOARD_H__
#define __CTC_BOARD_H__

/* board type Defines */
enum glb_board_series_e
{
    GLB_SERIES_GREATBELT_DEMO = 0xc,
    GLB_SERIES_E350 = 0x1,
    GLB_SERIES_E580 = 0x10,
    GLB_SERIES_P580 = 0x11,
    GLB_SERIES_E550 = 0x12,
    GLB_SERIES_E530 = 0x13,
    GLB_SERIES_MAX
};
typedef enum glb_board_series_e glb_board_series_t;

#define GLB_BOARD_GREATBELT_DEMO     0x1

#define GLB_BOARD_E580_24Q           0x1
#define GLB_BOARD_E580_48X2Q4Z       0x2
#define GLB_BOARD_E580_48X6Q         0x3
#define GLB_BOARD_E580_32X2Q         0x4
#define GLB_BOARD_E580_32X           0x5

#define GLB_BOARD_P580_HW_48X2Q4Z   2

#define GLB_BOARD_E550_24X8Y2C              1
#define GLB_BOARD_E550_48T4Y2Q              2
#define GLB_BOARD_E550_28M4Y2C              3
#define GLB_BOARD_E550_48M4Y2C              4
#define GLB_BOARD_E550_24T16Y               5
#define GLB_BOARD_E550_24T16X               6

#define GLB_BOARD_E530_24X2C_D              1
#define GLB_BOARD_E530_48T4X2Q_D              2
#define GLB_BOARD_E530_48M4Y2D              3
#define GLB_BOARD_E530_48X                  4
#define GLB_BOARD_E530_24X2C              5
#define GLB_BOARD_E530_48T4X              6
#define GLB_BOARD_E530_48S4X              7
#define GLB_BOARD_E530_24X2Q              8
#define GLB_BOARD_E530_48P4X              9

enum ctc_reboot_type_e
{
    CTC_REBOOT_TYPE_POWEROFF = 0,
    CTC_REBOOT_TYPE_WD,
    CTC_REBOOT_TYPE_MAX,
} ;
typedef enum ctc_reboot_type_e ctc_reboot_type_t;

#define CTC_WD_CFG    1
#define CTC_WD_EN     0
#define CTC_WD_FEED   1
#define CTC_GPIO_DATA 0x8 /* GPIO data register offset */
#define CTC_GPIO_DIR  0x0 /* GPIO dir register offset */

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
    u8 watchdog_en_gpio;           /*enable watch dog gpio*/
    u8 watchdog_feed_gpio;         /*feed watch dog gpio*/
    u8 watchdog_is_feed;           /*whether to feed watchdog, bug18151*/
    u16 gpio_date_offset;           /* gpio date register offset */	
    u16 gpio_dir_offset;            /* gpio direction register offset */	
    u8 reboot_type;
    u8 led_offset;                 /* kernel monitor led offset */
    u8 rst_irq;                    /* irq0~15 is reserved, so we choose 0 to be invalid irq */
    u8 rst_int_mask;
    u8 rst_int_offset;
    u8 pwr_irq;                    /* irq0~15 is reserved, so we choose 0 to be invalid irq */
    volatile void __iomem *gpio_g0_logic_addr;  /*gpio group 0 logic address*/
    volatile void __iomem *gpio_g1_logic_addr;  /*gpio group 1 logic address*/
    volatile void __iomem *gpio_g7_logic_addr;  /*gpio group 7 logic address*/
    volatile void __iomem *muxctrl_base_addr;
};
typedef struct ctc_board_s ctc_board_t;

#endif
