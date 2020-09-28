/**
 @file ctc_chip.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-30

 @version v2.0

   This file contains all chip related data structure, enum, macro and proto.
*/

#ifndef _CTC_CHIP_H
#define _CTC_CHIP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_mix.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/****************************************************************
*
* Data Structures
*
****************************************************************/

/**
 @defgroup chip CHIP
 @{
*/

/**
 @brief define chip type
*/
enum ctc_chip_type_e
{
    CTC_CHIP_NONE,              /**< [GB.GG]indicate this is a invalid chip */
    CTC_CHIP_GREATBELT,         /**< [GB]indicate this is a greatbelt chip */
    CTC_CHIP_GOLDENGATE,        /**< [GG]indicate this is a goldengate chip */
    CTC_CHIP_DUET2,             /**< [D2]indicate this is a duet2 chip */
    CTC_CHIP_TSINGMA,           /**< [TM]indicate this is a tsingma chip */
    MAX_CTC_CHIP_TYPE
};
typedef enum ctc_chip_type_e ctc_chip_type_t;


/**
 @brief define cpumac speed
*/
enum ctc_cpumac_speed_e
{
    CTC_CPUMAC_SPEED_MODE_100M,  /**< [GB] indicate cpumac's speed is 100M  */
    CTC_CPUMAC_SPEED_MODE_1G,      /**< [GB] indicate cpumac's speed is 1G  */
    CTC_CPUMAC_SPEED_MODE_MAX
};
typedef enum ctc_cpumac_speed_e ctc_cpumac_speed_t;


/**
 @brief define chip global configure
*/
struct ctc_chip_global_cfg_s
{
    uint8  lchip;                                       /**< [GB.GG.D2.TM] local chip id */
    uint8  cpu_port_en;                                 /**< [GB.GG.D2.TM] Network port as cpu port*/
    uint8  gb_gg_interconnect_en;                       /**< [GB.GG] Enable: Support linkagg add slice1 ports when GB and GG interconnect */
    uint8  rchain_en;                                   /**< [TM] Route chain mode enable */
    uint8  rchain_gchip;                                /**< [TM] Route chain gchip */
    uint16 tpid;                                        /**< [GB.GG.D2.TM] TpId field in cpu port Tx packet */
    uint16 vlanid;                                      /**< [GB.GG.D2.TM] vlanId filed in cpu port Tx packet */
    mac_addr_t cpu_mac_sa;                              /**< [GB.GG.D2.TM] cpu mac source address */
    mac_addr_t cpu_mac_da[MAX_CTC_CPU_MACDA_NUM];       /**< [GB.GG.D2.TM] cpu mac destination address */
    uint32 cpu_port;                                    /**< [GG.D2.TM] Network global port which is used for cpu port */

    uint8   cut_through_en;                             /**< [GB.GG.D2.TM] Enable cut through. 0: disable; 1: Enable */
    uint8   cut_through_speed;                          /**< [GB.GG.D2.TM] Cut through port speed.
                                                                        [GB] 0:none; 1:1G; 2:10G; 3: mix support 1G and 10G
                                                                        [GG] 1:10/40/100G mixed;2:1/10/100G mixed 3:1/10/40G mixed, else none */
    uint32  cut_through_speed_bitmap;                   /**< [D2.TM] Support Cut through port speed bitmap, the bit value refer to ctc_port_speed_t*/

    uint8  ecc_recover_en;                              /**< [GB.GG.D2.TM] Enable ecc error recover */
    uint8  tcam_scan_en;                                /**< [GG.D2.TM] Enable tcam key scan and recover error */
    uint64 alpm_affinity_mask;                          /**< [TM] Set affinity mask for alpm task */
    uint64 normal_affinity_mask;                        /**< [TM] Set affinity mask for normal task such as DMA,Stats... */
};
typedef struct ctc_chip_global_cfg_s ctc_chip_global_cfg_t;


/**
 @brief define memory scan para
*/
struct ctc_chip_mem_scan_cfg_t
{
    uint32  tcam_scan_interval;  /**<[GG.D2.TM]only used in scan_mode 2, Time interval between every entire tcam scan, unit is minute*/
    uint8   tcam_scan_mode;      /**<[GG.D2.TM] 0:scan once, 1:Continuously scan, 2:means stop scan task or do not set this function*/
    uint32  sbe_scan_interval;   /**<[GG.D2.TM]only used in scan_mode 2, Time interval between every entire single bit error scan, unit is minute*/
    uint8   sbe_scan_mode;       /**<[GG.D2.TM] 0:scan once, 1:Continuously scan, 2:means stop scan task or do not set this function*/
};
typedef struct ctc_chip_mem_scan_cfg_t ctc_chip_mem_scan_cfg_t;

/**
 @brief define ge phy para
*/
struct ctc_chip_gephy_para_s
{
    uint8 reg;            /**< [GB]the offset of the phy internal address */
    uint8 rsv;
    uint16 val;          /**< [GB]the value of the register */
};
typedef struct ctc_chip_gephy_para_s ctc_chip_gephy_para_t;

/**
 @brief define xg phy para
*/
struct ctc_chip_xgphy_para_s
{
    uint8   devno;       /**< [GB]the device NO. of the xg phy*/
    uint16  reg;          /**< [GB]the offset of the phy internal address */
    uint16  val;          /**< [GB]the value of the register */
    uint8   rsv[2];
};
typedef struct ctc_chip_xgphy_para_s ctc_chip_xgphy_para_t;

/**
 @brief define mapping para
*/
struct ctc_chip_phy_mapping_para_s
{
    uint8   port_mdio_mapping_tbl[MAX_PORT_NUM_PER_CHIP];       /**< [GB.GG.D2.TM]port and mdio bus mapping, index is local phy port */
    uint8   port_phy_mapping_tbl[MAX_PORT_NUM_PER_CHIP];        /**< [GB.GG.D2.TM]port and mdio phy address mapping, index is local phy port*/
};
typedef struct ctc_chip_phy_mapping_para_s ctc_chip_phy_mapping_para_t;

struct ctc_chip_gpio_params_s
{
    uint8   gpio_id;                         /**< [GB.GG.D2.TM] gpio id
                                                               [TM] gpio id. the normal gpio is 0-16, the Hs gpio, 17-33 */
    uint8   value;                           /**< [GB.GG.D2.TM] input or output value */
    uint8   rsv[2];
};
typedef struct ctc_chip_gpio_params_s ctc_chip_gpio_params_t;

struct ctc_chip_gpio_cfg_s
{
    uint8 gpio_id;                          /**< [TM] gpio id. the normal gpio is 0-16, the Hs gpio, 17-33 */
    uint8 debounce_en;                      /**< [TM] debounce enable */
    uint8 level_edge_val;                   /**< [TM] 0: low level/falling edge effective 1: high level/rising edge effective */
};
typedef struct ctc_chip_gpio_cfg_s ctc_chip_gpio_cfg_t;

/**
 @brief define chip access type
*/
enum ctc_chip_access_type_e
{
    CTC_CHIP_PCI_ACCESS,        /**< [GB.GG.D2.TM] Pcie access type */
    CTC_CHIP_I2C_ACCESS,        /**< [GB.GG.D2.TM] I2C access type */

    CTC_CHIP_MAX_ACCESS_TYPE
};
typedef enum ctc_chip_access_type_e ctc_chip_access_type_t;

/**
 @brief define ge auto scan optinal register
*/
struct ctc_chip_ge_opt_reg_s
{
    uint8 intr_enable;                         /**< [GB] whether to enable interrupt */
    uint8 reg;                                     /**< [GB] special ge phy register for auto scan */
    uint8 bit_ctl;                                /**< [GB] special ge phy register bit for auto scan */
    uint8 lchip;                                    /**< [GB] local chip id */
};
typedef struct ctc_chip_ge_opt_reg_s  ctc_chip_ge_opt_reg_t;

/**
 @brief define xg auto scan optinal register
*/
struct ctc_chip_xg_opt_reg_s
{
    uint8 intr_enable;                     /**< [GB] whether to enable interrupt */
    uint8 dev;                                /**< [GB] special xg phy device type for auto scan */
    uint16 reg;                               /**< [GB] special ge phy register for auto scan */
    uint8 bit_ctl;                            /**< [GB] special ge phy register bit for auto scan */
    uint8 lchip;                                /**< [GB] local chip id */
    uint8 rsv[2];
};
typedef struct ctc_chip_xg_opt_reg_s ctc_chip_xg_opt_reg_t;

/**
 @brief define phy auto scan control parameter
*/
struct ctc_chip_phy_scan_ctrl_s
{
    uint32 op_flag;                             /**< [GB.GG.D2.TM] Flags indicate parameter type, zero means set all parameter */
    uint32 scan_interval;                    /**< [GB.GG.D2.TM] interval for next phy scan, the unit is core frequency */
    uint32 mdio_use_phy0;                 /**< [GB.GG.D2.TM] mac0~31 use phy or not, 1:use phy, link status come from externel phy; 0: not use phy, link status come from mac pcs layer */
    uint32 mdio_use_phy1;                 /**< [GB.GG.D2.TM] mac 32~59, use phy or not */
    uint32 scan_gephy_bitmap0;         /**< [GB.GG] control which ge phy in mdio bus 0 will be scaned */
    uint32 scan_gephy_bitmap1;         /**< [GB.GG] control which ge phy in mdio bus 1 will be scaned */
    uint8  scan_xgphy_bitmap0;          /**< [GB.GG] control which xg phy in mdio bus 3 will be scaned */
    uint8  scan_xgphy_bitmap1;          /**< [GB.GG] control which xg phy in mdio bus 4 will be scaned */
    uint8  xgphy_link_bitmask;            /**< [GB.GG.D2.TM] control whether PMA,PCS or XS link status should mask */
    uint8  xgphy_scan_twice;              /**< [GB.GG.D2.TM] control whether xgphy should scan twice */
    uint8  lchip;                           /**< [GB] local chip id */
    uint8  ctl_id;                                 /**<[GG] choose which slice, 0-slice0; 1-slice1 */
    uint8  mdio_ctlr_id;            /**< [D2.TM] MDIO controller ID */
    uint8  mdio_ctlr_type;          /**< [D2.TM] MDIO controller PHY type: 1G/XG, and etc. refer to ctc_chip_mdio_type_t */
    uint32 scan_phy_bitmap0;        /**< [D2.TM] scan PHY bitmap for MDIO bus #0 per MDIO controller */
    uint32 scan_phy_bitmap1;        /**< [D2.TM] scan PHY bitmap for MDIO bus #1 per MDIO controller */
};
typedef struct ctc_chip_phy_scan_ctrl_s ctc_chip_phy_scan_ctrl_t;

/**
 @brief define phy auto scan control parameter type
*/
enum ctc_chip_phy_scan_type_e
{
    CTC_CHIP_INTERVAL_OP = 0x01,           /**< [GB.GG.D2.TM] to set phy scan interval parameter */
    CTC_CHIP_USE_PHY_OP = 0x02,            /**< [GB.GG.D2] to set mdio use phy parameter */
    CTC_CHIP_GE_BITMAP_OP = 0x04,         /**< [GG] to set ge phy bitmap parameter */
    CTC_CHIP_XG_BITMAP_OP = 0x08,         /**< [GG] to set xg phy bitmap parameter */
    CTC_CHIP_XG_LINK_MASK_OP = 0x10,    /**< [GG.D2.TM] to set xgphy device link mask parameter */
    CTC_CHIP_XG_TWICE_OP = 0x20,            /**< [GG.D2.TM] to set xgphy scan twice parameter */
    CTC_CHIP_PHY_BITMAP_OP = 0x40       /**< [D2.TM] to set phy bitmap parameter */
};
typedef enum ctc_chip_phy_scan_type_e ctc_chip_phy_scan_type_t;

/**
 @brief define sfp auto scan control parameter type
*/
enum ctc_chip_sfp_scan_op_e
{
    CTC_CHIP_SFP_INTERVAL_OP = 0x01,         /**< [GB] to set i2c scan interval parameter */
    CTC_CHIP_SFP_BITMAP_OP = 0x02,            /**< [GB] to set i2c device biitmap parameter */
    CTC_CHIP_SFP_SCAN_REG_OP = 0x04         /**< [GB] to set which space to scan for i2c device */
};
typedef enum ctc_chip_sfp_scan_op_e ctc_chip_sfp_scan_op_t;

#define CTC_CHIP_MAC_BITMAP     2
/**
 @brief define sfp auto scan control parameter
*/
struct ctc_chip_i2c_scan_s
{
    uint8   ctl_id;                                                   /**< [GG.D2.TM] i2c master control id <0-1>  */
    uint32  op_flag;                                                /**< [GB.GG.D2.TM] Flags indicate parameter type, zero means set all parameter */
    uint32  slave_bitmap[CTC_CHIP_MAC_BITMAP];     /**< [GB.GG.D2.TM] bit map for slave device to scan */
    uint16  dev_addr;                                              /**< [GB.GG.D2.TM] slave device address */
    uint8   offset;                                                    /**< [GG.D2.TM] offset in slave devie for scan */
    uint8   length;                                                   /**< [GG.D2.TM] scan length */
    uint32  interval;                                                /**< [GG.D2.TM] interval for next round scan, the unit is sup clock cycle */
    uint8   lchip;                                                     /**< [GB.D2.TM] local chip id */
    uint8   i2c_switch_id;                                        /**< [GG.D2.TM] interval for next round scan, the unit is sup clock cycle */
    uint8   rsv[2];
};
typedef struct ctc_chip_i2c_scan_s ctc_chip_i2c_scan_t;

/**
 @brief define sfp read para
*/
struct ctc_chip_i2c_read_s
{
    uint8 ctl_id;                                                      /**< [GG.D2.TM] i2c master control id <0-1>  */
    uint8 slave_dev_id;                                           /**< [GB.GG.D2.TM] slave device id */
    uint16 dev_addr;                                               /**< [GB.GG.D2.TM] slave device address */
    uint32 slave_bitmap;                                         /**< [GB.GG.D2.TM] bit map for slave device to scan */
    uint8  offset;                                                     /**< [GG.D2.TM] offset in slave devie for scan */
    uint8  length;                                                    /**< [GG.D2.TM] scan length */
    uint8 lchip;                                                       /**< [GB.GG.D2.TM] local chip id */
    uint8 i2c_switch_id;                                          /**< [GG.D2.TM] for dev like pca9548a, set 0 mean disable */
    uint8* p_buf;                                                    /**< [GB.GG.D2.TM] buffer for store read data result */
    uint32 buf_length;                                             /**< [GG.D2.TM] buffer length */
    uint8  access_switch;                                        /**< [GG.D2.TM] access I2C switch */
    uint8  rsv[3];
};
typedef struct ctc_chip_i2c_read_s ctc_chip_i2c_read_t;

/**
 @brief define polling read
*/
struct ctc_chip_i2c_scan_read_s
{
    uint8 ctl_id;                                                     /**< [GG.D2.TM] i2c master control id <0-1> */
    uint8 gchip;                                                     /**< [GB.GG.D2.TM] global chip id */
    uint8 rsv[2];
    uint32 len;                                                       /**< [GB.GG.D2.TM] buffer length */
    uint8* p_buf;                                                   /**< [GB.GG.D2.TM] buffer pointer */
};
typedef struct ctc_chip_i2c_scan_read_s ctc_chip_i2c_scan_read_t;

/**
 @brief define sfp write para
*/
struct ctc_chip_i2c_write_s
{
    uint8 ctl_id;                                                    /**< [GG.D2.TM] i2c master control id <0-1> */
    uint8  offset;                                                   /**< [GG.D2.TM] offset in slave devie for scan */
    uint16 dev_addr;                                             /**< [GB.GG.D2.TM] slave device address */
    uint8  slave_id;                                               /**< [GB.GG.D2.TM] slave device id */
    uint8  data;                                                     /**< [GB.GG.D2.TM] data to write */
    uint8  lchip;                                                     /**< [GB.GG.D2.TM] local chip id */
    uint8  i2c_switch_id;                                        /**< [GG.D2.TM] for dev like pca9548a, set 0 mean disable other equal ((addr>>4)&0xF+1)*/
    uint8  access_switch;                                        /**< [GG.D2.TM] access I2C switch */
    uint8  rsv[3];
};
typedef struct ctc_chip_i2c_write_s ctc_chip_i2c_write_t;

/**
 @brief define mac led type
*/
enum ctc_chip_mac_led_type_e
{
    CTC_CHIP_USING_ONE_LED,                         /**< [GG.D2.TM] using one led for mac status */
    CTC_CHIP_USING_TWO_LED,                        /**< [GG.D2.TM] using two led for mac status */

    CTC_CHIP_MAX_LED_TYPE
};
typedef enum ctc_chip_mac_led_type_e ctc_chip_mac_led_type_t;

/**
 @brief define mac led mode
*/
enum ctc_chip_led_mode_e
{
    CTC_CHIP_RXLINK_MODE,                           /**< [GB.GG.D2.TM]  led on when rx link up, led off when rx link down, not blind */
    CTC_CHIP_TXLINK_MODE,                           /**< [GB.GG.D2.TM]  led on when tx link up, led off when tx link down, not blind, only for 10g mac */
    CTC_CHIP_RXLINK_RXACTIVITY_MODE,        /**< [GB.GG.D2.TM]  led on when rx link up, led off when rx link down, led blind when rx activity */
    CTC_CHIP_TXLINK_TXACTIVITY_MODE,        /**< [GB.GG.D2.TM]  led on when tx link up, led off when tx link down, led blind when tx activity, only for 10g */
    CTC_CHIP_RXLINK_BIACTIVITY_MODE,        /**< [GB.GG.D2.TM]  led on when rx link up, off when rx link down, led blind when rx or tx activity */
    CTC_CHIP_TXACTIVITY_MODE,                    /**< [GB.GG.D2.TM]  led always off, led blind when tx activity */
    CTC_CHIP_RXACTIVITY_MODE,                    /**< [GB.GG.D2.TM]  led always off, led blind when rx activity */
    CTC_CHIP_BIACTIVITY_MODE,                     /**< [GB.GG.D2.TM]  led always off, led blind when rx or tx activity */
    CTC_CHIP_FORCE_ON_MODE,                      /**< [GB.GG.D2.TM]  led always on, not blind  */
    CTC_CHIP_FORCE_OFF_MODE,                     /**< [GB.GG.D2.TM]  led always off, not blind  */
    CTC_CHIP_FORCE_ON_TXACTIVITY_MODE,          /**< [GB.GG.D2.TM]  led always on, led blind when tx activity  */
    CTC_CHIP_FORCE_ON_RXACTIVITY_MODE,          /**< [GB.GG.D2.TM]  led always on, led blind when rx activity  */
    CTC_CHIP_FORCE_ON_BIACTIVITY_MODE,          /**< [GB.GG.D2.TM]  led always on, led blind when rx or tx activity  */
    CTC_CHIP_MAC_LED_MODE
};
typedef enum ctc_chip_led_mode_e ctc_chip_led_mode_t;

/**
 @brief define mac led para
*/
struct ctc_chip_led_para_s
{
    uint32 op_flag;                                       /**<[GB.GG.D2.TM] Flags indicate parameter type, zero means set all parameter */
    uint8  lport_en;                                      /**<[D2.TM] if set,will use lport as key,else use mac id */
    uint16 port_id;                                       /**<[GB.GG.D2.TM] When lport_en enable, it means local phy port id; else means mac id(default)*/
    uint8  lchip;                                         /**< [GB.GG.D2.TM] local chip id */

    uint8 ctl_id;                                        /**<[GG] mac led control id <0-1> */
    uint32 polarity;                                      /**<[GB.GG.D2.TM] config led driver polarity */
    ctc_chip_led_mode_t  first_mode;            /**<[GB.GG.D2.TM] first mac led mode */
    ctc_chip_led_mode_t  sec_mode;             /**<[GB.GG.D2.TM] second mac led mode, only use when using two leds */
};
typedef struct ctc_chip_led_para_s ctc_chip_led_para_t;

/**
 @brief define mac led para set type
*/
enum ctc_chip_mac_led_op_e
{
    CTC_CHIP_LED_MODE_SET_OP = 0x01,             /**<[GB.GG.D2.TM]  to set mac led mode parameter */
    CTC_CHIP_LED_POLARITY_SET_OP = 0x02        /**<[GB.GG.D2.TM]  to set mac led polarity parameter, must acording hardware design */
};
typedef enum ctc_chip_mac_led_op_e ctc_chip_mac_led_op_t;

/**
 @brief define mac led mapping
*/
struct ctc_chip_mac_led_mapping_s
{
    uint8* p_mac_id;                             /**<[GB.GG.D2] mac list for mac led sequence, the size must be adjusted to mac_led_num parameter */
    uint8  mac_led_num;                         /**<[GB.GG.D2] how many mac led should be used */
    uint16 *port_list;                           /**<[D2.TM] port list for mac led sequence*/
    uint8  lport_en;                              /**<[D2.TM] if set,will use lport as key,else use mac id (default) */

    uint8  lchip;                                /**< [GB.GG.D2.TM] local chip id */
    uint8 ctl_id;                                 /**<[GG] mac led control id <0-1> */
    uint8 rsv[1];
};
typedef struct ctc_chip_mac_led_mapping_s ctc_chip_mac_led_mapping_t;

/**
 @brief define gpio mode
*/
enum ctc_chip_gpio_mode_e
{
    CTC_CHIP_INPUT_MODE,                     /**<[GG.D2.TM] gpio used as input */
    CTC_CHIP_OUTPUT_MODE,                  /**<[GG.D2.TM] gpio used as output */
    CTC_CHIP_SPECIAL_MODE,                 /**<[GB] gpio used as specail function */
    CTC_CHIP_EDGE_INTR_INPUT_MODE,         /**<[TM] gpio used as edge interrupt input mode*/
    CTC_CHIP_LEVEL_INTR_INPUT_MODE,        /**<[TM] gpio used as level interrupt input mode*/
    CTC_CHIP_MAX_GPIO_MODE
};
typedef enum ctc_chip_gpio_mode_e ctc_chip_gpio_mode_t;

enum ctc_chip_serdes_mode_e
{
    CTC_CHIP_SERDES_NONE_MODE,                   /**<[GG.D2.TM] serdes is useless */
    CTC_CHIP_SERDES_XFI_MODE,                    /**<[GB.GG.D2.TM] config serdes to XFI mode */
    CTC_CHIP_SERDES_SGMII_MODE,                  /**<[GB.GG.D2.TM] config serdes to SGMII mode */
    CTC_CHIP_SERDES_XSGMII_MODE,                 /**<[GB] config serdes to XSGMII mode */
    CTC_CHIP_SERDES_QSGMII_MODE,                 /**<[GB.D2.TM] config serdes to QSGMII mode */
    CTC_CHIP_SERDES_XAUI_MODE,                   /**<[GB.GG.D2.TM] config serdes to XAUI mode */
    CTC_CHIP_SERDES_DXAUI_MODE,                  /**<[GG.D2.TM] config serdes to D-XAUI mode */
    CTC_CHIP_SERDES_XLG_MODE,                    /**<[GG.D2.TM] config serdes to xlg mode */
    CTC_CHIP_SERDES_CG_MODE,                     /**<[GG.D2.TM] config serdes to cg mode */
    CTC_CHIP_SERDES_2DOT5G_MODE,                 /**<[GB.GG.D2.TM] config serdes to 2.5g mode */
    CTC_CHIP_SERDES_USXGMII0_MODE,               /**<[D2.TM] config serdes to USXGMII single mode */
    CTC_CHIP_SERDES_USXGMII1_MODE,               /**<[D2.TM] config serdes to USXGMII multi 1G/2.5G mode */
    CTC_CHIP_SERDES_USXGMII2_MODE,               /**<[D2.TM] config serdes to USXGMII multi 5G mode */
    CTC_CHIP_SERDES_XXVG_MODE,                   /**<[D2.TM] config serdes to XXVG mode */
    CTC_CHIP_SERDES_LG_MODE,                     /**<[D2.TM] config serdes to LG mode */
    CTC_CHIP_SERDES_100BASEFX_MODE,              /**<[TM] config serdes to 100BASEFX mode */

    CTC_CHIP_MAX_SERDES_MODE
};
typedef enum ctc_chip_serdes_mode_e ctc_chip_serdes_mode_t;

enum ctc_chip_serdes_ocs_mode_e
{
    CTC_CHIP_SERDES_OCS_MODE_NONE = 0, /**<[GG.D2.TM] normal serdes speed, not overclocking */
    CTC_CHIP_SERDES_OCS_MODE_11_06G, /**<[GG.D2.TM] serdes overclocking to 11.06G speed */
    CTC_CHIP_SERDES_OCS_MODE_12_12G, /**<[GG.D2.TM] serdes overclocking to 12.12G speed */
    CTC_CHIP_SERDES_OCS_MODE_12_58G, /**<[GG.D2.TM] serdes overclocking to 12.58G speed */
    CTC_CHIP_SERDES_OCS_MODE_27_27G, /**<[TM] serdes overclocking to 27.27G speed */

    CTC_CHIP_MAX_SERDES_OCS_MODE
};
typedef enum ctc_chip_serdes_ocs_mode_e ctc_chip_serdes_ocs_mode_t;

struct ctc_chip_serdes_info_s
{
    uint8 serdes_id;                              /**<[GB.GG.D2.TM] serdes id */
    uint8 lport_valid;                            /**<[GB] If the lport_valid is true, will swith to the lport */
    uint16 overclocking_speed;                    /**<[GG.D2.TM] overclocking speed value, refer to ctc_chip_serdes_overclocking_speed_mode_t*/
    ctc_chip_serdes_mode_t serdes_mode;           /**<[GB.GG.D2.TM] serdes mode */
    uint16 lport;                                 /**<[GB] lport value */
};
typedef struct ctc_chip_serdes_info_s ctc_chip_serdes_info_t;

enum ctc_chip_mdio_type_e
{
    CTC_CHIP_MDIO_GE,                                   /**<[GB.GG.D2.TM] 1G phy */
    CTC_CHIP_MDIO_XG,                                   /**<[GB.GG.D2.TM] 10G phy */
    CTC_CHIP_MDIO_XGREG_BY_GE                   /**<[GB.GG] accecc 10 phy by 1G bus */
};
typedef enum ctc_chip_mdio_type_e ctc_chip_mdio_type_t;

struct ctc_chip_mdio_para_s
{
    uint8 ctl_id;                                                 /**<[GG] mdio slice id <0-1> */
    uint8 bus;                                                    /**<[GB.GG.D2.TM] bus id */
    uint8 phy_addr;                                            /**<[GB.GG.D2.TM] phy address */
    uint8 rsv;
    uint16 reg;                                                   /**<[GB.GG.D2.TM] register address */
    uint16 value;                                                /**<[GB.GG.D2.TM] config value */
    uint16 dev_no;                                              /**<[GB.GG.D2.TM] only usefull for xgphy */
    uint8 rsv1[2];
};
typedef struct ctc_chip_mdio_para_s ctc_chip_mdio_para_t;

enum ctc_chip_sensor_type_e
{
    CTC_CHIP_SENSOR_TEMP,                            /**<[GB.GG.D2.TM] sensor chip temperature */
    CTC_CHIP_SENSOR_VOLTAGE                        /**<[GB.GG.D2.TM] sensor chip voltage */
};
typedef enum ctc_chip_sensor_type_e ctc_chip_sensor_type_t;

enum ctc_chip_serdes_prbs_polynome_e
{
    CTC_CHIP_SERDES_PRBS7_PLUS,      /**<[GB.GG.D2] prbs mode PRBS7+ */
    CTC_CHIP_SERDES_PRBS7_SUB,       /**<[GB.GG.D2] prbs mode PRBS7- */
    CTC_CHIP_SERDES_PRBS15_PLUS,    /**<[GB.GG.D2] prbs mode PRBS15+ */
    CTC_CHIP_SERDES_PRBS15_SUB,     /**<[GB.GG.D2] prbs mode PRBS15- */
    CTC_CHIP_SERDES_PRBS23_PLUS,    /**<[GB.GG.D2] prbs mode PRBS23+ */
    CTC_CHIP_SERDES_PRBS23_SUB,     /**<[GB.GG.D2] prbs mode PRBS23- */
    CTC_CHIP_SERDES_PRBS31_PLUS,    /**<[GB.GG.D2] prbs mode PRBS31+ */
    CTC_CHIP_SERDES_PRBS31_SUB      /**<[GB.GG.D2] prbs mode PRBS31- */
};
typedef enum ctc_chip_serdes_prbs_polynome_e ctc_chip_serdes_polynome_type_t;

struct ctc_chip_serdes_prbs_s
{
    uint8 serdes_id;                         /**<[GB.GG.D2.TM] serdes ID */
    uint8 polynome_type;                /**<[GB.GG.D2.TM] relate to ctc_chip_serdes_polynome_type_t */
    uint8 mode;                               /**<[GB.GG.D2.TM] 0--Rx, 1--Tx */
    uint8 value;                               /**<[GB.GG.D2.TM] used for tx, 0--tx disable, 1--tx enable; used for rx, is check result, 1 is "ok", other value is "fail" */
};
typedef struct ctc_chip_serdes_prbs_s ctc_chip_serdes_prbs_t;

struct ctc_chip_serdes_loopback_s
{
   uint8 serdes_id;                     /**<[GB.GG.D2.TM] serdes ID */
   uint8 enable;                         /**<[GB.GG.D2.TM] 1--enable, 0--disable */
   uint8 mode;                          /**<[GB.GG.D2.TM] loopback mode, 0--interal, 1--external */
};
typedef struct ctc_chip_serdes_loopback_s ctc_chip_serdes_loopback_t;

struct ctc_chip_mem_bist_s
{
   uint32 status;                         /**<[GB.GG.D2.TM] check result, 0--PASS, 1--FAIL */
};
typedef struct ctc_chip_mem_bist_s ctc_chip_mem_bist_t;

#define CTC_CHIP_CTLE_PARAM_NUM 3
struct ctc_chip_serdes_ctle_s
{
    uint8 serdes_id;                            /**<[TM] serdes id */
    uint8 auto_en;                              /**<[TM] 1: enable auto, 0: diable auto */
    uint16 value[CTC_CHIP_CTLE_PARAM_NUM];      /**<[TM] ctle parameter value*/
};
typedef struct ctc_chip_serdes_ctle_s ctc_chip_serdes_ctle_t;

struct ctc_phy_driver_s
{
  int32 (* init)(uint8 lchip, ctc_chip_mdio_para_t* param);
  int32 (* deinit)(uint8 lchip, ctc_chip_mdio_para_t* param);
  int32 (* set_port_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 enable);
  int32 (* get_port_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_enable);
  int32 (* set_port_duplex_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 enable);
  int32 (* get_port_duplex_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_enable);
  int32 (* set_port_speed)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 value);
  int32 (* get_port_speed)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_value);
  int32 (* set_port_auto_neg_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 enable);
  int32 (* get_port_auto_neg_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_enable);
  int32 (* set_port_loopback)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 enable);
  int32 (* get_port_loopback)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_enable);
  int32 (* get_link_up_status)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* p_link_up);
  int32 (* linkup_event)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* user_data);
  int32 (* linkdown_event)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32* user_data);
  int32 (* set_port_unidir_en)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 enable);
  int32 (* get_port_unidir_en)(uint8 lchip, ctc_chip_mdio_para_t* param,  uint32* p_enable);
  int32 (* set_port_medium)(uint8 lchip, ctc_chip_mdio_para_t* param,  uint32 value);
  int32 (* get_port_medium)(uint8 lchip, ctc_chip_mdio_para_t* param,  uint32* p_value);
  int32 (* set_ext_attr)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 type, uint32 value);
  int32 (* get_ext_attr)(uint8 lchip, ctc_chip_mdio_para_t* param, uint32 type, uint32* p_value);
};
typedef struct ctc_phy_driver_s ctc_phy_driver_t;

#define CTC_CHIP_PHY_NAME_MAX_LEN 30
#define CTC_CHIP_PHY_NULL_PHY_ID 0xFFFFFFFFL
enum ctc_chip_phy_shim_event_mode_e
{
    CTC_CHIP_PHY_SHIM_EVENT_MODE_SCAN,      /**<[D2.TM] get phy port link up/down, by mdio scan */
    CTC_CHIP_PHY_SHIM_EVENT_MODE_IRQ,       /**<[D2.TM] get phy port link up/down, by irq */
    CTC_CHIP_PHY_SHIM_EVENT_MODE_POLLING,   /**<[D2.TM] get phy port link up/down, by polling */
    CTC_CHIP_PHY_SHIM_MAX_EVENT_MODE
};
typedef enum ctc_chip_phy_shim_event_mode_e ctc_chip_phy_shim_event_mode_t;

struct ctc_chip_phy_shim_s
{
    uint32 phy_id;                              /**< [D2.TM] 32bit phy id */
    char phy_name[CTC_CHIP_PHY_NAME_MAX_LEN];   /**< [D2.TM] Phy Name */
    ctc_phy_driver_t driver;                   /**< [D2.TM] the driver for this phy */
    void *user_data;                            /**< [D2.TM] user_data for callback function param.  */
    uint32 event_mode;                          /**< [D2.TM] linkup or linkdown event mode. refer to ctc_chip_phy_shim_event_mode_t */
    uint16 irq;                                 /**< [D2.TM] user_data for callback function param.  */
    uint16 poll_interval;                       /**< [D2.TM] poll interval time, equal zero: disable, other:enable£¬unit: ms*/
};
typedef struct ctc_chip_phy_shim_s ctc_chip_phy_shim_t;

enum ctc_chip_property_e
{
    CTC_CHIP_PROP_SERDES_FFE,               /**<[GB.GG.D2.TM] config serdes ffe parameter, refer to ctc_chip_serdes_ffe_t */
    CTC_CHIP_PEOP_SERDES_POLARITY,      /**<[GG.D2.TM] config serdes polarity */
    CTC_CHIP_PROP_SERDES_PRBS,             /**<[GB.GG.D2.TM] config serdes prbs */
    CTC_CHIP_PROP_SERDES_LOOPBACK,     /**<[GB.GG.D2.TM] config serdes loopback */
    CTC_CHIP_PROP_SERDES_P_FLAG,   /**<[GG.D2.TM] when optical using Pre-emphasis mode, for trace length
                                                 too short serdes, need config this mode to reduce p-flag
                                                 value: 0:disable, 1:enable */
    CTC_CHIP_PROP_SERDES_PEAK,         /**<[GB.GG.D2.TM] peaking value, when dpc is true, the peak is invalid. refer to ctc_chip_serdes_cfg_t
                                                   value: HSS28G:0-31 other:0-7 */
    CTC_CHIP_PROP_SERDES_DPC,          /**<[GB.GG.D2.TM] enable or disable dynamic Peaking Control, refer to ctc_chip_serdes_cfg_t
                                                   value: 0:disable, 1:enable */
    CTC_CHIP_PROP_SERDES_SLEW_RATE,    /**<[GB.GG.D2.TM] config serdes slew rate, refert to ctc_chip_serdes_cfg_t,
                                                   value: [GB]  0:>4.5Gbps 1:3.126-4.25Gbps 2:<3.126Gbps
                                                          [GG]  HSS28G 0:full rate 1:10G
                                                                HSS15G 0:full rate 1:full rate for 802.3ap kr
                                                                       2:medium 3:slow */

    CTC_CHIP_PROP_EEE_EN,                       /**<[GB.GG.D2.TM] enable eee globally */
    CTC_CHIP_PROP_SERDES_ID_TO_GPORT,           /**<[GB.GG.D2.TM] get gport with serdes_id, refer to ctc_port_serdes_info_t */
    CTC_CHIP_PHY_SCAN_EN,                       /**<[GB.D2.TM] enable phy scan */
    CTC_CHIP_PHY_SCAN_PARA,                     /**<[D2.TM] set phy scan para, refer to ctc_chip_phy_scan_ctrl_t */
    CTC_CHIP_I2C_SCAN_EN,                       /**<[GB.D2.TM] enable I2C Master scan */
    CTC_CHIP_I2C_SCAN_PARA,                     /**<[D2.TM] set i2c scan para, refer to ctc_chip_i2c_scan_t */
    CTC_CHIP_MAC_LED_EN,                        /**<[GB.GG.D2.TM] enable Mac Led */
    CTC_CHIP_PROP_MEM_SCAN,                     /**<[GG.D2.TM] enable tcam key and single bit error scan */
    CTC_CHIP_PROP_GPIO_MODE,                    /**<[GB.GG.D2.TM] set GPIO working mode ctc_chip_gpio_mode_t */
    CTC_CHIP_PROP_GPIO_OUT,                     /**<[GB.GG.D2.TM] write Value onto the GPIO with ctc_chip_gpio_params_t */
    CTC_CHIP_PROP_GPIO_IN,                      /**<[GB.GG.D2.TM] read Value from the GPIO with ctc_chip_gpio_params_t */
    CTC_CHIP_PROP_GPIO_CFG,                     /**<[TM] set GPIO config with ctc_chip_gpio_cfg_t */
    CTC_CHIP_PROP_PHY_MAPPING,                  /**<[GB.GG.D2.TM] set mdio and phy mapping relation, Value is a pointer to ctc_chip_phy_mapping_para_t */

    CTC_CHIP_PROP_DEVICE_INFO,                  /**<[GB.GG.D2.TM] chip device information */
    CTC_CHIP_PROP_MEM_BIST,                     /**<[GB.GG.D2.TM] memory BIST, Value is a pointer to ctc_chip_mem_bist_t, GB will take a few seconds */
    CTC_CHIP_PROP_RESET_HW,                     /**<[D2.TM] Reset chip hardware,recover chip data*/
    CTC_CHIP_PROP_PERI_CLOCK,                   /**<[TM] set peri clock, refer to ctc_chip_peri_clock_t */
    CTC_CHIP_PROP_MAC_LED_BLINK_INTERVAL,       /**<[D2.TM] set Mac Led blink interval,  time cycle(ns) = 1000*interval/coreclock(MHZ)£¬default£º0x0d1cef00*/
    CTC_CHIP_PROP_SERDES_CTLE,                  /**<[TM] changle ctle ,  refer to ctc_chip_serdes_ctle_t */
    CTC_CHIP_PROP_REGISTER_PHY,                 /**<[D2.TM] phy regiseter,  refer to ctc_chip_phy_shim_t*/
    CTC_CHIP_PROP_MAX_TYPE
};
typedef enum ctc_chip_property_e ctc_chip_property_t;

enum ctc_chip_serdes_ffe_mode_e
{
    CTC_CHIP_SERDES_FFE_MODE_TYPICAL,    /**<[GB.GG.D2.TM] config ffe by motherboard material and trace length */
    CTC_CHIP_SERDES_FFE_MODE_DEFINE,      /**<[GG.D2.TM] config ffe by user define value, this mode is traditional mode, support copper and fiber */
    CTC_CHIP_SERDES_FFE_MODE_3AP            /**<[GG.D2.TM] config ffe by user define value, this mode is 802.3ap mode, only support copper */
};
typedef enum ctc_chip_serdes_ffe_mode_e ctc_chip_serdes_ffe_mode_t;

struct ctc_chip_serdes_ffe_s
{
    uint8  rsv;
    uint8  serdes_id;                                          /**<[GB.GG.D2.TM] serdes id */
    uint8  mode;                                                /**<[GB.GG.D2.TM] relate to ctc_chip_serdes_ffe_mode_t */
    uint8  status;                                               /**<[GG.D2.TM] mode status: 0--inactive, 1-active */
    uint16 board_material;                                 /**<[GB.GG.D2.TM] motherboard material: 0--FR4, 1--M4, 2--M6 */
    uint16 trace_len;                                          /**<[GB.GG.D2.TM] trace length: 0-- (0~4)inch, 1--(4~7)inch, 2--(7~10)inch */
                                                           /**<[D2.TM]M4 trace length: 0-- (0~4)inch, 1--(4~7)inch, 2--(7~10)inch */
                                                           /**<[D2.TM]FR4 trace length: 0-- (0~4)inch, 1--(4~7)inch, 2--(7~8)inch, 3--(8-9)inch, 4--(>9)inch*/
    uint16 coefficient[CTC_CHIP_FFE_PARAM_NUM]; /**<[GB.GG.D2.TM] coefficient value */
};
typedef struct ctc_chip_serdes_ffe_s ctc_chip_serdes_ffe_t;

struct ctc_chip_serdes_polarity_s
{
    uint8 dir;                                                     /**<[GG.D2.TM] 0:rx, 1:tx */
    uint8 serdes_id;                                           /**<[GG.D2.TM] serdes id */
    uint16 polarity_mode;                                   /**<[GG.D2.TM] 0:normal, 1:Reverse*/
};
typedef struct ctc_chip_serdes_polarity_s ctc_chip_serdes_polarity_t;

struct ctc_chip_serdes_cfg_s
{
    uint8 serdes_id;            /**<[GB.GG.D2.TM] serdes id */
    uint8 rsv;
    uint16 value;           /**<[GB.GG.D2.TM] serdes value */
};
typedef struct ctc_chip_serdes_cfg_s ctc_chip_serdes_cfg_t;

struct ctc_chip_gpio_event_s
{
    uint8 gpio_id;                              /**< [TM] gpio id. the normal gpio is 0-16, the Hs gpio, 17-33 */
    uint8 rsv[3];
};
typedef struct ctc_chip_gpio_event_s ctc_chip_gpio_event_t;

enum ctc_chip_peri_type_e
{
    CTC_CHIP_PERI_MDIO_TYPE,                   /**< [TM] indicate mdio peri*/
    CTC_CHIP_PERI_I2C_TYPE,                    /**< [TM] indicate i2c peri*/
    CTC_CHIP_PERI_MAC_LED_TYPE,                /**< [TM] indicate mac led peri*/
};
typedef enum ctc_chip_peri_type_e ctc_chip_peri_type_t;

struct ctc_chip_peri_clock_s
{
    uint8 type;                               /**< [TM] peri type, refer to ctc_chip_peri_type_t */
    uint8 mdio_type;                          /**< [TM] mdio type, refer to ctc_chip_mdio_type_t */
    uint8 ctl_id;                             /**< [TM] i2c master control id <0-1> */
    uint8 rsv;
    uint16 clock_val;                         /**< [TM] clock val, unit KHZ */
};
typedef struct ctc_chip_peri_clock_s ctc_chip_peri_clock_t;

typedef int32 (* ctc_chip_reset_cb)(uint8 type, uint32 flag);
/**@} end of @defgroup chip  */

/**
 @defgroup datapath
 @{
*/

struct ctc_datapath_serdes_prop_s
{
    uint8 mode;                    /**<[GG.D2.TM]refer to ctc_chip_serdes_mode_t */
    uint8 is_dynamic;            /**<[GG.D2.TM]serdes support dynamic switch */
    uint8 rx_polarity;             /**<[GG.D2.TM]serdes rx polarity is reversed or not,  0:normal, 1:Reverse */
    uint8 tx_polarity;             /**<[GG.D2.TM]serdes tx polarity is reversed or not,  0:normal, 1:Reverse */
    uint8 group;                    /**<[D2.TM]serdes group info, used by LG mode*/
    uint8 is_xpipe;                 /*<[TM]serdes support X-PIPE or not */
};
typedef struct ctc_datapath_serdes_prop_s ctc_datapath_serdes_prop_t;

struct ctc_datapath_global_cfg_s
{
    uint16 core_frequency_a;                                                            /**<[GG.D2.TM]for core plla */
    uint16 core_frequency_b;                                                            /**<[GG.D2.TM]for core pllb */
    ctc_datapath_serdes_prop_t serdes[CTC_DATAPATH_SERDES_NUM];    /**<[GG.D2.TM] serdes property array */
    uint8 wlan_enable;                                             /**<[D2.TM]for wlan security */
    uint8 dot1ae_enable;                                           /**<[D2.TM]for dot1ae security */
    char* file_name;                                               /**<[GB]have no meaning, useless */
};
typedef struct ctc_datapath_global_cfg_s ctc_datapath_global_cfg_t;

struct ctc_chip_device_info_s
{
    uint16 device_id;                                            /**<[GB.GG.D2.TM]chip device ID */
    uint16 version_id;                                           /**<[GB.GG.D2.TM]chip version ID */

    char chip_name[CTC_MAX_CHIP_NAME_LEN];      /**<[GB.GG.D2.TM]chip name */
};
typedef struct ctc_chip_device_info_s ctc_chip_device_info_t;

/**@} end of @defgroup datapath  */

#ifdef __cplusplus
}
#endif

#endif

