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
    GLB_SERIES_SGG = 0xa0,
    GLB_SERIES_MAX
};
typedef enum glb_board_series_e glb_board_series_t;

#define GLB_BOARD_GREATBELT_DEMO     0x1

#define GLB_BOARD_E580_24Q           0x1
#define GLB_BOARD_E580_48X2Q4Z       0x2
#define GLB_BOARD_E580_48X6Q         0x3
#define GLB_BOARD_E580_32X2Q         0x4
#define GLB_BOARD_E580_32X           0x5
#define GLB_BOARD_E580_48XT6Q        0x6
#define GLB_BOARD_E580_WQT48X4Q      0x7
#define GLB_BOARD_E580_SCC48X6Q      0x8
#define GLB_BOARD_E580_WQT48XT4Q     0x9
#define GLB_BOARD_E580_HW_48X6Q      0xa
#define GLB_BOARD_E580_HW_48X4Q4T    0xb

#define GLB_BOARD_P580_HW_48X2Q4Z   2

#define GLB_BOARD_E350_48T4X2Q          0x1
#define GLB_BOARD_E350_48T4XG           0x2
#define GLB_BOARD_E350_PHICOMM_8T12XG   0x3
#define GLB_BOARD_E350_24T4XG           0x4
#define GLB_BOARD_E350_8T12XG           0x5
#define GLB_BOARD_E350_8T4S12XG         0x6
#define GLB_BOARD_E350_8TS12X           0x7
#define GLB_BOARD_E350_24X              0x08
#define GLB_BOARD_E350_PEGATRON         0x10
#define GLB_BOARD_V350_48P4X            0x13
/* added by tongzb for support HF 2018-09-18*/ 
#define GLB_BOARD_E350_HF_12T4S8X       0x14
#define GLB_BOARD_E350_65DV1  0x15
#define GLB_BOARD_V350_24P4X            0x19
 	
#define GLB_BOARD_T65C_8T64X6Q 0x5

enum ctc_reboot_type_e
{
    CTC_REBOOT_TYPE_POWEROFF = 0,
    CTC_REBOOT_TYPE_WD,
    CTC_REBOOT_TYPE_MAX,
} ;
typedef enum ctc_reboot_type_e ctc_reboot_type_t;

#define CTC_INT_NUM 1
#define CTC_INT_0 19
#define CTC_INT0_STA_OFFSET 7
#define CTC_INT0_MSK_OFFSET 9
#define CTC_WD_CFG 1
#define CTC_WD_EN 11
#define CTC_WD_FEED 10
#define CTC_GPIO_DATA 8 /* GPIO data register offset */

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
    u8 dpll_support;               /*whether support dpll*/
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
    void __iomem *epld_logic_addr; /*epld logic address*/
    void __iomem *gpio_logic_addr; /*gpio logic address*/
    u8 normal_irq_type;/* 0 mask by epld, 1 mask by cpu */
};
typedef struct ctc_board_s ctc_board_t;

#endif
