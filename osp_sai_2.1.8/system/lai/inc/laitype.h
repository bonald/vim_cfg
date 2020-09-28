/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-11-25
* Reason        : First Create.
****************************************************************************/

#if !defined (__LAITYPES_H_)
#define __LAITYPES_H_


#if defined(_WIN32)

//
// *nix already has lower-case definitions for types.
//
typedef UINT8  uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef INT32  int32_t;
typedef INT64  int64_t;
typedef UINT64 uint64_t;


typedef UINT32  lai_attr_id_t;
typedef UINT8   lai_mac_t[6];

#include <ws2def.h>
#include <ws2ipdef.h>


//
// N.B. Equal to 260 on Windows
//
#define PATH_MAX MAX_PATH



#else  // #if defined(_WIN32)

#include <stdint.h>
#include <sys/types.h>

typedef int32_t  lai_status_t;
typedef uint32_t lai_switch_profile_id_t;
typedef uint32_t lai_attr_id_t;
typedef uint8_t  lai_mac_t[6];

#define _In_
#define _Out_
#define _Inout_
#define _In_reads_z_(_LEN_)
#define _In_reads_opt_z_(_LEN_)

#endif // _WIN32

//
// New common definitions
//
typedef uint64_t lai_uint64_t;
typedef int64_t lai_int64_t;
typedef uint32_t lai_uint32_t;
typedef int32_t lai_int32_t;
typedef uint16_t lai_uint16_t;
typedef int16_t lai_int16_t;
typedef uint8_t lai_uint8_t;
typedef int8_t lai_int8_t;
typedef size_t lai_size_t;
typedef uint64_t lai_object_id_t;


#define LAI_NULL_OBJECT_ID 0L


typedef struct _lai_object_list_t {
    uint32_t count;
    lai_object_id_t *list;
} lai_object_list_t;



#ifndef LAI_CONFIG_INFO_STR_MAX
#define LAI_CONFIG_INFO_STR_MAX 64
#endif

#ifndef LAI_NAME_INFO_STR_MAX
#define LAI_NAME_INFO_STR_MAX 256
#endif

#ifndef LAI_VCM_CHANNEL_NUM
#define LAI_VCM_CHANNEL_NUM 9
#endif

#ifndef LAI_POE_PORT_NUM
#define LAI_POE_PORT_NUM 48
#endif

#ifndef LAI_MAX_SWVER_LEN
#define LAI_MAX_SWVER_LEN          29 
#endif

/**
 * The Platform specification defines the format of the system
 * eeprom and the available fields that may be described there.
 *
 * This structure contains the decoded fields for application
 * and platform use.
 */

/** lai_card_status */
typedef enum lai_card_status_e {
    LAI_CARD_STATUS_PRESENT = (1 << 0),
    LAI_CARD_STATUS_ABSENT = (1 << 1),
    LAI_CARD_STATUS_INITED = (1 << 2),
    LAI_CARD_STATUS_READY = (1 << 3),
    LAI_CARD_STATUS_MAX,
} lai_card_status_t;

/** lai_card_status */
typedef enum lai_card_caps_e {
    LAI_CARD_CAPS_SUPPORT_SAVE_REBOOT_INFO = (1 << 0),
    LAI_CARD_CAPS_SUPPORT_ONIE = (1 << 1),
    LAI_CARD_CAPS_MAX,
} lai_card_caps_t;

typedef struct lai_card_info_s {
    /** OID*/
    lai_object_id_t oid;

    /** slot id*/
    uint32_t slot;

    /** board type info*/
    uint8_t board_series;
    uint8_t board_type;
    
    /* System MAC*/
    uint8_t     mac[6];

    /*MAC number*/
    uint32_t mac_num;

    /* Total physical port number*/
    uint32_t port_num;

    /*lai_card_status_t*/
    uint32_t status;
    uint8_t epld_status;
    char product_name[LAI_NAME_INFO_STR_MAX];
    char package_name[LAI_NAME_INFO_STR_MAX];
    char version[LAI_NAME_INFO_STR_MAX];
    char vendor[LAI_NAME_INFO_STR_MAX];
    char serial_number[LAI_NAME_INFO_STR_MAX];
    char hardware_name[LAI_NAME_INFO_STR_MAX];
    char hardware_version[LAI_NAME_INFO_STR_MAX];
    char epld_version[LAI_NAME_INFO_STR_MAX];
    char bootrom_version[LAI_NAME_INFO_STR_MAX];
    char snmp_enterprise_oid[LAI_NAME_INFO_STR_MAX];

    char manufacture_date[LAI_NAME_INFO_STR_MAX];
    char platform_name[LAI_NAME_INFO_STR_MAX];
    char onie_version[LAI_NAME_INFO_STR_MAX];
    char manufacturer[LAI_NAME_INFO_STR_MAX];
    char country_code[LAI_NAME_INFO_STR_MAX];

    char bootcmd[LAI_NAME_INFO_STR_MAX];

    /* for customer requirement */
    uint32_t is_cst;
    char cst_company[LAI_NAME_INFO_STR_MAX];
    char cst_hw_type[LAI_CONFIG_INFO_STR_MAX];
    char cst_hw_info[LAI_CONFIG_INFO_STR_MAX];
    char cst_hw_ver[LAI_CONFIG_INFO_STR_MAX];
    char cst_sw_ver[LAI_CONFIG_INFO_STR_MAX];

    /* SDRAM Size by Mbyte*/
    uint32_t sdram_size;

    /* FLASH Size by Mbyte*/
    uint32_t flash_size;  

    /* Tmpr low threshold for board*/
    int32_t tmpr_low;

    /* Tmpr higt threshold for board*/
    int32_t tmpr_high;

    /* Tmpr cirt threshold for board*/
    int32_t tmpr_crit; 

    /* Tmpr low_alarm threshold for chip*/
    int32_t tmpr_chip_low_alarm;

    /* Tmpr low threshold for chip*/
    int32_t tmpr_chip_low;

    /* Tmpr higt threshold for chip*/
    int32_t tmpr_chip_high;

    /* Tmpr alarm threshold for chip*/
    int32_t tmpr_chip_alarm;

    /* Tmpr cirt threshold for chip*/
    int32_t tmpr_chip_crit;  

    /* Capabilities */
    int32_t caps;
    int32_t platform_type;

    /* added by hansf for save profile type from errprom */
    uint32_t profile_type;

} lai_card_info_t;

typedef struct lai_portname_info_s
{
    /*connection char in interface name, char '-' in eth-0-1*/
    char connection;

    /*connection char in interface name, char '/' in eth-0-1/1*/
    char sub_connection;
    
    char prefix_num;
    char prefix[LAI_CONFIG_INFO_STR_MAX][LAI_CONFIG_INFO_STR_MAX];
    char full_prefix[LAI_CONFIG_INFO_STR_MAX][LAI_CONFIG_INFO_STR_MAX];
    
}lai_portname_info_t;

typedef struct lai_reboot_info_s
{
    /*reboot times*/
    int32_t  reboot_times;

    /*reboot type, like "power", "manual"*/
    char reboot_type[LAI_CONFIG_INFO_STR_MAX][LAI_NAME_INFO_STR_MAX];

    /*reboot time*/
    char reboot_time[LAI_CONFIG_INFO_STR_MAX][LAI_NAME_INFO_STR_MAX];
    
}lai_reboot_info_t;

/* should be same as glb_phy_type_t */
enum lai_phy_type_e
{
    LAI_PHY_TYPE_UNKNOWN = 0x0,
    LAI_PHY_TYPE_1000BASE_T,    
    LAI_PHY_TYPE_10BASE_T,
    LAI_PHY_TYPE_100BASE_TX,
    LAI_PHY_TYPE_BASE_PX,
    LAI_PHY_TYPE_BASE_BX10,    
    LAI_PHY_TYPE_100BASE_FX,
    LAI_PHY_TYPE_100BASE_LX,
    LAI_PHY_TYPE_1000BASE_T_SFP,
    LAI_PHY_TYPE_1000BASE_CX,
    LAI_PHY_TYPE_1000BASE_LX,
    LAI_PHY_TYPE_1000BASE_SX,
    LAI_PHY_TYPE_10GBASE_ER,
    LAI_PHY_TYPE_10GBASE_LRM,
    LAI_PHY_TYPE_10GBASE_LR,
    LAI_PHY_TYPE_10GBASE_SR,
    LAI_PHY_TYPE_10GBASE_PASSIVE_COPPER, 
    LAI_PHY_TYPE_10GBASE_ACTIVE_COPPER,
    LAI_PHY_TYPE_40GBASE_CR4,
    LAI_PHY_TYPE_40GBASE_SR4,
    LAI_PHY_TYPE_40GBASE_LR4,
    LAI_PHY_TYPE_40GBASE_XLPPI,
    LAI_PHY_TYPE_100G_AOC,
    LAI_PHY_TYPE_100GBASE_SR4,
    LAI_PHY_TYPE_100GBASE_LR4,
    LAI_PHY_TYPE_100GBASE_ER4,
    LAI_PHY_TYPE_100GBASE_SR10,
    LAI_PHY_TYPE_100G_CWDM4_MSA_FEC,
    LAI_PHY_TYPE_100G_PSM4_SMF,
    LAI_PHY_TYPE_100G_ACC,
    LAI_PHY_TYPE_100G_CWDM4_MSA_NOFEC,
    LAI_PHY_TYPE_100GBASE_CR4,
    LAI_PHY_TYPE_40GBASE_ER4,
    LAI_PHY_TYPE_4X10GBASE_SR,
    LAI_PHY_TYPE_40G_PSM4_SMF,
    LAI_PHY_TYPE_MAX,
};
typedef enum lai_phy_type_e lai_phy_type_t;

enum lai_port_speed_e
{
    LAI_PORT_SPEED_100M = (1<<0),
    LAI_PORT_SPEED_1G   = (1<<1),
    LAI_PORT_SPEED_10G  = (1<<2),
    LAI_PORT_SPEED_40G  = (1<<3),
    LAI_PORT_SPEED_100G = (1<<4),
    LAI_PORT_SPEED_2_5G = (1<<5),
    LAI_PORT_SPEED_5G   = (1<<6),
    LAI_PORT_SPEED_25G = (1<<7),
    LAI_PORT_SPEED_50G   = (1<<8),    
    LAI_PORT_SPEED_MAX
};
typedef enum lai_port_speed_e lai_port_speed_t;

enum lai_port_speed_caps_e
{
    LAI_PORT_SPEED_CAPS_100M = (1<<0),
    LAI_PORT_SPEED_CAPS_1G = (1<<1),
    LAI_PORT_SPEED_CAPS_10G = (1<<2),
    LAI_PORT_SPEED_CAPS_40G = (1<<3),
    LAI_PORT_SPEED_CAPS_100G = (1<<4),
    LAI_PORT_SPEED_CAPS_MAX
};
typedef enum lai_port_speed_caps_e lai_port_speed_caps_t;


enum lai_port_media_e
{
    LAI_PORT_MEDIA_AUTO,
    LAI_PORT_MEDIA_COPPER,
    LAI_PORT_MEDIA_FIBER,
    LAI_PORT_MEDIA_SGMII,
    LAI_PORT_MEDIA_MAX,
};
typedef enum lai_port_media_e lai_port_media_t;

enum lai_port_commber_e
{
    LAI_PORT_COMMBER_DEFAULT,
    LAI_PORT_COMMBER_SFP,
    LAI_PORT_COMMBER_QSFP,
    LAI_PORT_COMMBER_COPPER,
    LAI_PORT_COMMBER_MAX,
};
typedef enum lai_port_commber_e lai_port_commber_t;

/** lai_port_status */
typedef enum lai_port_status_e {
    LAI_PORT_STATUS_PRESENT = (1 << 0),
    LAI_PORT_STATUS_ABSENT = (1 << 1),
    LAI_PORT_STATUS_MAX,
} lai_port_status_t;

typedef struct lai_port_info_s {
    /** OID*/
    lai_object_id_t oid;
    
    /** Port id*/
    uint32_t portid;

    /** Name*/
    char name[LAI_NAME_INFO_STR_MAX];

    /** slot id*/
    int32_t slot_id;

    /** pannel_port_id*/
    int32_t pannel_port_id;

    /** pannel_port_id*/
    int32_t pannel_sub_port_id;

    /** speed,lai_port_speed_t*/
    int32_t speed;

    /** speed_capability, lai_port_speed_caps_t*/
    uint32_t speed_caps;

    /** Media, lai_port_media_t*/
    int32_t media;

    /** commber, lai_port_commber_t, for commber port application*/
    int32_t commber;

    /** Media, if support copper switch to fiber or fiber to copper*/
    int32_t support_media_switch;    

    /** Physical type, lai_phy_type_t*/
    int32_t phy_type;

    /* status, lai_port_status_t */
    int32_t status;

    /** oper duplex */
    int32_t duplex;
    
    /** MAC */
    uint8_t mac[6];

    /** port flag */
    int32_t phyport_flag;

    /** bypass enable */
    uint8_t bypass;

    /** fec status */
    uint32_t fec_status;

    /** cl73 ability */
    uint32_t cl73_ability;

} lai_port_info_t;

#define LAI_FIBER_VAL_TYPE   4  /*high/low alarm, high/low warn*/
#define LAI_FIBER_VAL_TYPE2  4  /*real value, channel num max is 4*/

typedef struct lai_fiber_info_s
{
    /** OID*/
    lai_object_id_t oid;

    /** has updated */
    int32_t updated;
    
    /** slot id*/
    int32_t slot_id;

    /** pannel port id*/
    int32_t pannel_port_id;

    /** pannel sub port id*/
    int32_t pannel_sub_port_id;

    /** port_id*/
    int32_t port_id;

    /** SFP vendor name (ASCII)*/
    char name[LAI_NAME_INFO_STR_MAX];

    /** SFP type name*/
    char fiber_type_name[LAI_NAME_INFO_STR_MAX]; 

    /**SFP vendor IEEE company ID*/
    char oui[LAI_NAME_INFO_STR_MAX]; 

    /** Part number provided by SFP vendor (ASCII)*/
    char pn[LAI_NAME_INFO_STR_MAX]; 

    /** Revision level for part number provided by vendor (ASCII)*/
    char rev[LAI_NAME_INFO_STR_MAX]; 

    /** Serial number provided by vendor (ASCII)*/
    char sn[LAI_NAME_INFO_STR_MAX]; 

    /** complianceCode */
    char compliance_code[LAI_NAME_INFO_STR_MAX]; 

    /** high/low alarm , high/low warn*/
    double tmpr[LAI_FIBER_VAL_TYPE];
    
    double               voltage[LAI_FIBER_VAL_TYPE];
    
    double               bias[LAI_FIBER_VAL_TYPE];
    
    double               tx_pwr[LAI_FIBER_VAL_TYPE];
    
    double               rx_pwr[LAI_FIBER_VAL_TYPE];
    
    /**real value, max 4*/
    double               tmpr2[LAI_FIBER_VAL_TYPE2]; 
    
    double               voltage2[LAI_FIBER_VAL_TYPE2];
    
    double               bias2[LAI_FIBER_VAL_TYPE2];
    
    double               tx_pwr2[LAI_FIBER_VAL_TYPE2];
    
    double               rx_pwr2[LAI_FIBER_VAL_TYPE2];
    
    /**store alarm info*/
    uint32_t               alarm_flag[LAI_FIBER_VAL_TYPE2];

    /**store warn info*/
    uint32_t               warn_flag[LAI_FIBER_VAL_TYPE2]; 
    
    uint32_t               update_cnt;
    
    uint32_t                channel_num;
    
    uint32_t                channel_idx;
    
    uint32_t                is_detail;

    /*lai_phy_type_t*/
    uint32_t                fiber_type;          

    uint32_t                fiber_flag; 
    
    uint32_t                is_copper;   
    
    /*SFP speed = speed * 100Mbit/sec */
    uint32_t                speed;               

    /*is support DDM*/
    uint32_t                ddm_support;    

    /*if support DDM, use internally calibrated or externally calibrated*/
    uint32_t                externally_calibrated; 

    /*output wavelength*/
    uint8_t                wavelength[2];       
    /*length */
    
    uint8_t                length[5];           
} lai_fiber_info_t;

/** lai_fan_mode */
typedef enum lai_fan_mode_e {
    LAI_FAN_MODE_OFF,
    LAI_FAN_MODE_SLOW,
    LAI_FAN_MODE_NORMAL,
    LAI_FAN_MODE_FAST,
    LAI_FAN_MODE_MAX,
    LAI_FAN_MODE_LAST = LAI_FAN_MODE_MAX,
    LAI_FAN_MODE_COUNT,
    LAI_FAN_MODE_INVALID = -1,
} lai_fan_mode_t;

/** lai_fan_status */
typedef enum lai_fan_status_e {
    LAI_FAN_STATUS_PRESENT = (1 << 0),
    LAI_FAN_STATUS_ABSENT = (1 << 1),
    LAI_FAN_STATUS_OK = (1 << 2),
    LAI_FAN_STATUS_FAILED = (1 << 3),
    LAI_FAN_STATUS_B2F = (1 << 4),
    LAI_FAN_STATUS_F2B = (1 << 5),
    LAI_FAN_STATUS_AUTO = (1 << 6),
    LAI_FAN_STATUS_FORCE = (1 << 7),
   
} lai_fan_status_t;

/** lai_fan_caps */
typedef enum lai_fan_caps_e {
    LAI_FAN_CAPS_B2F = (1 << 0),
    LAI_FAN_CAPS_F2B = (1 << 1),
    LAI_FAN_CAPS_SET_RPM = (1 << 2),
    LAI_FAN_CAPS_SET_PERCENTAGE = (1 << 3),
    LAI_FAN_CAPS_GET_RPM = (1 << 4),
    LAI_FAN_CAPS_GET_PERCENTAGE = (1 << 5),
} lai_fan_caps_t;

/** lai_fan_dir */
typedef enum lai_fan_dir_e {
    LAI_FAN_DIR_B2F,
    LAI_FAN_DIR_F2B,
    LAI_FAN_DIR_LAST = LAI_FAN_DIR_F2B,
    LAI_FAN_DIR_COUNT,
    LAI_FAN_DIR_INVALID = -1,
} lai_fan_dir_t;

typedef struct lai_fan_info_s {
    /** OID*/
    lai_object_id_t oid;
    
    /** slot id*/
    int32_t slot_id;

    /* Tray*/
    int32_t tray;

    /* Status */
    int32_t status;

    /* Capabilities */
    int32_t caps;

    /* Current fan speed, in RPM, if available */
    int32_t rpm;

    /* Current fan percentage, if available */
    int32_t percentage;

    /* Current fan mode, if available  */
    lai_fan_mode_t mode;

    /* Model (if applicable) */
    char model[LAI_CONFIG_INFO_STR_MAX];

    /* Serial Number (if applicable) */
    char serial[LAI_CONFIG_INFO_STR_MAX];

} lai_fan_info_t;


/** lai_psu_caps */
typedef enum lai_psu_caps_e {
    LAI_PSU_CAPS_AC = (1 << 0),
    LAI_PSU_CAPS_DC12 = (1 << 1),
    LAI_PSU_CAPS_DC48 = (1 << 2),
    LAI_PSU_CAPS_VIN = (1 << 3),
    LAI_PSU_CAPS_VOUT = (1 << 4),
    LAI_PSU_CAPS_IIN = (1 << 5),
    LAI_PSU_CAPS_IOUT = (1 << 6),
    LAI_PSU_CAPS_PIN = (1 << 7),
    LAI_PSU_CAPS_POUT = (1 << 8),
} lai_psu_caps_t;

/** lai_psu_mode */
typedef enum lai_psu_mode_e {
    LAI_PSU_MODE_AC = (1 << 0),
    LAI_PSU_MODE_DC12 = (1 << 1),
    LAI_PSU_MODE_DC48 = (1 << 2),
    LAI_PSU_MODE_DC12_POE = (1 << 3),
    LAI_PSU_MODE_DC48_POE = (1 << 4),
} lai_psu_mode_t;

/** lai_psu_status */
typedef enum lai_psu_status_e {
    LAI_PSU_STATUS_PRESENT = (1 << 0),
    LAI_PSU_STATUS_ABESENT = (1 << 1),
    LAI_PSU_STATUS_OK = (1 << 2),
    LAI_PSU_STATUS_FAILED = (1 << 3),
    LAI_PSU_STATUS_ALERT = (1 << 4),
} lai_psu_status_t;

/**
 * PSU Information Structure
 */
typedef struct lai_psu_info_t {
    /** OID Header */
    lai_object_id_t oid;

    /** slot id*/
    int32_t slot_id;

    /* Model */
    char model[LAI_CONFIG_INFO_STR_MAX];

    /* Serial Number */
    char serial[LAI_CONFIG_INFO_STR_MAX];

    /* Status */
    uint32_t status;

    /* Capabilities */
    uint32_t caps;

    /* Mode,  */
    uint32_t mode;    

    /* millivolts */
    int mvin;
    int mvout;

    /* milliamps */
    int miin;
    int miout;

    /* milliwatts */
    int mpin;
    int mpout;

    uint8_t fixed_status;
    uint8_t psu_health_status_change;
    uint8_t support_pmbus;
    uint8_t status_byte;
    uint16_t status_word;
    uint8_t status_vout;
    uint8_t status_iout;
    uint8_t status_input;
    uint8_t status_temperature;
    uint8_t status_cml;
    uint8_t status_other;
    uint8_t status_mfr_specific;
    uint8_t status_fans_1_2;
    uint8_t status_fans_3_4;

} lai_psu_info_t;

/**
 * VCM Information Structure
 */
typedef struct lai_vcm_info_t {
    /** OID Header */
    lai_object_id_t oid;

    /** slot id*/
    int32_t slot_id;

    uint8_t vcm_type;
    uint8_t vcm_channel_nums;
    uint8_t channel_name[LAI_VCM_CHANNEL_NUM][LAI_NAME_INFO_STR_MAX];
    uint32_t flag[LAI_VCM_CHANNEL_NUM];
    uint8_t curr_depend_pin[LAI_VCM_CHANNEL_NUM];
    double	 volt_split_factor[LAI_VCM_CHANNEL_NUM];
    double	 low[LAI_VCM_CHANNEL_NUM];
    double	 high[LAI_VCM_CHANNEL_NUM];
    double	 volt[LAI_VCM_CHANNEL_NUM];
    double	 curr[LAI_VCM_CHANNEL_NUM];
    double	 tmpr[LAI_VCM_CHANNEL_NUM];
    
} lai_vcm_info_t;

/**
 * POE Information Structure
 */
typedef struct lai_poe_info_t {
    /** OID Header */
    lai_object_id_t oid;

    /** slot id*/
    int32_t slot_id;

    uint8_t                glb_enable;
    uint32_t               glb_power_limit;
    uint32_t               glb_status;
    uint32_t               glb_cur_consump;
    uint32_t               glb_aver_consump;
    uint32_t               glb_peak_consump;
    uint32_t               pse_channel_num;
    uint32_t               const_power_limit;
    uint32_t               pse_channel_id[LAI_POE_PORT_NUM];
    uint32_t               panel_slot_no[LAI_POE_PORT_NUM];
    uint32_t               panel_port_no[LAI_POE_PORT_NUM];
    uint32_t               panel_sub_port_no[LAI_POE_PORT_NUM];
    uint32_t               enable[LAI_POE_PORT_NUM];
    uint32_t               force_power[LAI_POE_PORT_NUM];
    uint32_t               mode[LAI_POE_PORT_NUM];
    uint32_t               priority[LAI_POE_PORT_NUM];
    uint32_t               power_limit[LAI_POE_PORT_NUM];
    uint32_t               class_level[LAI_POE_PORT_NUM];
    uint32_t               status[LAI_POE_PORT_NUM];
    uint32_t               cur_consump[LAI_POE_PORT_NUM];
    uint32_t               aver_consump[LAI_POE_PORT_NUM];
    uint32_t               peak_consump[LAI_POE_PORT_NUM];
    uint32_t               current[LAI_POE_PORT_NUM];
    uint32_t               voltage[LAI_POE_PORT_NUM];
    uint32_t               temper[LAI_POE_PORT_NUM];    
} lai_poe_info_t;

/** lai_thermal_status */
typedef enum lai_thermal_status_e {
    LAI_THERMAL_STATUS_PRESENT = (1 << 0),
    LAI_THERMAL_STATUS_ABESENT = (1 << 1),
    LAI_THERMAL_STATUS_FAILED = (1 << 2),
    LAI_THERMAL_STATUS_OK = (1 << 3),
} lai_thermal_status_t;

/** lai_thermal_caps */
typedef enum lai_thermal_caps_e {
    LAI_THERMAL_CAPS_GET_TEMPERATURE = (1 << 0),
} lai_thermal_caps_t;

/**
 * Thermal sensor information structure.
 */
typedef struct lai_thermal_info_s {

    /** OID Header */
    lai_object_id_t oid;

    /** slot id*/
    int32_t slot_id;

    char posision[LAI_NAME_INFO_STR_MAX];

    /** Status */
    uint32_t status;

    /** Capabilities */
    uint32_t caps;

    /** Current temperature in milli-celsius */
    int32_t tempr;

    /** 1: chip, 0: board */
    int32_t is_chip;

} lai_thermal_info_t;

typedef enum lai_led_location_e {
    LAI_LED_LOCATION_CHASSIS,
    LAI_LED_LOCATION_CARD,
    LAI_LED_LOCATION_FAN,
    LAI_LED_LOCATION_PSU,
    LAI_LED_LOCATION_THERMAL,
    LAI_LED_LOCATION_SYNCE_DPLL_STATE,

} lai_led_location_t;

/** lai_led_status */
typedef enum lai_led_status_e {
    LAI_LED_STATUS_INIT,
    LAI_LED_STATUS_RUN,
    LAI_LED_STATUS_ABNORMAL,    
    LAI_LED_STATUS_ALARM,
    LAI_LED_STATUS_NO_ALARM,
    LAI_LED_STATUS_ABSENT,
    LAI_LED_STATUS_OFF,
    LAI_LED_STATUS_ON,

    LAI_LED_STATUS_PRESENT,
    LAI_LED_STATUS_FAILED,
} lai_led_status_t;

/** lai_led_mode */
typedef enum lai_led_mode_e {
    LAI_LED_MODE_OFF,
    LAI_LED_MODE_ON,
    LAI_LED_MODE_BLINKING,
    LAI_LED_MODE_RED = 10,
    LAI_LED_MODE_RED_BLINKING = 11,
    LAI_LED_MODE_ORANGE = 12,
    LAI_LED_MODE_ORANGE_BLINKING = 13,
    LAI_LED_MODE_YELLOW = 14,
    LAI_LED_MODE_YELLOW_BLINKING = 15,
    LAI_LED_MODE_GREEN = 16,
    LAI_LED_MODE_GREEN_BLINKING = 17,
    LAI_LED_MODE_BLUE = 18,
    LAI_LED_MODE_BLUE_BLINKING = 19,
    LAI_LED_MODE_PURPLE = 20,
    LAI_LED_MODE_PURPLE_BLINKING = 21,
    LAI_LED_MODE_AUTO = 22,
} lai_led_mode_t;

/** lai_led_caps */
typedef enum lai_led_caps_e {
    LAI_LED_CAPS_ON_OFF = (1 << 0),
    LAI_LED_CAPS_RED = (1 << 10),
    LAI_LED_CAPS_RED_BLINKING = (1 << 11),
    LAI_LED_CAPS_ORANGE = (1 << 12),
    LAI_LED_CAPS_ORANGE_BLINKING = (1 << 13),
    LAI_LED_CAPS_YELLOW = ( 1 << 14),
    LAI_LED_CAPS_YELLOW_BLINKING = (1 << 15),
    LAI_LED_CAPS_GREEN = (1 << 16),
    LAI_LED_CAPS_GREEN_BLINKING = (1 << 17),
    LAI_LED_CAPS_BLUE = (1 << 18),
    LAI_LED_CAPS_BLUE_BLINKING = (1 << 19),
    LAI_LED_CAPS_PURPLE = (1 << 20),
    LAI_LED_CAPS_PURPLE_BLINKING = (1 << 21),
    LAI_LED_CAPS_AUTO = (1 << 22),
} lai_led_caps_t;
/* <auto.end.enum(tag:led).define> */


/**
 * LED information structure.
 */
typedef struct lai_led_info_s {
    /** Header */
    lai_object_id_t oid;

    /** slot id*/
    int32_t slot_id;

    char name[LAI_CONFIG_INFO_STR_MAX];

    /** Status */
    uint32_t status;

    /** Capabilities */
    uint32_t caps;

    /** Current mode, if capable. */
    lai_led_mode_t mode;
} lai_led_info_t;


typedef enum _lai_synce_attribute_t
{
    /** Set synce global enable*/
    LAI_SYNCE_GLOBAL_ENBALE,

    /** Select synce ifindex*/
    LAI_SYNCE_SELECTED_IFINDEX,
    
    /** Set synce clock recovery select enable*/
    LAI_SYNCE_CLOCK_RECOVERY_SELECT_ENABLE,

} lai_synce_attribute_t;

/**
 * LED information structure.
 */
typedef struct lai_synce_info_s {    
    /** Header */
    lai_synce_attribute_t id;

    /** global synce enable*/
    uint32_t synce_enable;

    /** device holdover enable*/
    uint32_t holdover_en;

    /** device clock recovery select enable*/
    uint32_t clock_recovery_select_en;

    /** status*/
    uint32_t status;

    /** slot id*/
    uint8_t slot_idx;
    
    /*from 1 to max_port_num, eth-X-X*/
    uint8_t port_idx;
    
    uint8_t subport_idx;

    uint16_t gport;

    uint32_t dpll_state;
    
} lai_synce_info_t;

typedef struct lai_oem_info_s
{
    char company_name[LAI_NAME_INFO_STR_MAX];
    char package_name[LAI_NAME_INFO_STR_MAX];
    char product_name[LAI_NAME_INFO_STR_MAX];
    char hardware_type[LAI_NAME_INFO_STR_MAX];
    char snmp_enterprise_oid[LAI_NAME_INFO_STR_MAX];
    char chassis_type[LAI_NAME_INFO_STR_MAX];
    char four_sfp_name[LAI_NAME_INFO_STR_MAX]; /*4SFP card name*/
    char two_sfpp_name[LAI_NAME_INFO_STR_MAX];/*2SFP+ card name*/
    char four_sfpp_name[LAI_NAME_INFO_STR_MAX];/*4SFP+ card name*/
} lai_oem_info_t;

typedef struct lai_sw_version_info_s
{
    char sw_version[LAI_MAX_SWVER_LEN+1];
} lai_sw_version_info_t;

typedef enum
{
    LAI_OEM_TYPE_COMPANY_NAME = 1,
    LAI_OEM_TYPE_PACKAGE_NAME,
    LAI_OEM_TYPE_PRODUCT_NAME,
    LAI_OEM_TYPE_HARDWARE_TYPE,
    LAI_OEM_TYPE_SNMP_ENTERPRISE_OID,
    LAI_OEM_TYPE_CHASSIS_TYPE,
    LAI_OEM_TYPE_4SFP_CARD_NAME,
    LAI_OEM_TYPE_2SFPP_CARD_NAME,
    LAI_OEM_TYPE_4SFPP_CARD_NAME,
    LAI_OEM_TYPE_MAX
} lai_oem_info_type_e;

#endif // __LAITYPES_H_

