
#ifndef __TBL_CARD_DEFINE_H__
#define __TBL_CARD_DEFINE_H__

/* TBL_CARD field defines */
typedef enum
{
    TBL_CARD_FLD_KEY                  = 0 ,  /* READ */
    TBL_CARD_FLD_SLOT                 = 1 ,  /* READ */
    TBL_CARD_FLD_PRODUCT_NAME         = 2 ,  /* READ */
    TBL_CARD_FLD_HARDWARE_NAME        = 3 ,  /* READ */
    TBL_CARD_FLD_COMPANY_NAME         = 4 ,  /* READ */
    TBL_CARD_FLD_PACKAGE_NAME         = 5 ,  /* READ */
    TBL_CARD_FLD_ENTERPRISE_OID       = 6 ,  /* READ */
    TBL_CARD_FLD_HARDWARE_TYPE        = 7 ,  /* READ */
    TBL_CARD_FLD_SERIAL_NO            = 8 ,  /* READ */
    TBL_CARD_FLD_BOOTROM_VER          = 9 ,  /* READ */
    TBL_CARD_FLD_EPLD_VER             = 10,  /* RW */
    TBL_CARD_FLD_SW_VER               = 11,  /* READ */
    TBL_CARD_FLD_HARDWARE_VER         = 12,  /* READ */
    TBL_CARD_FLD_BOARD_TYPE           = 13,  /* READ */
    TBL_CARD_FLD_BOARD_SERIES         = 14,  /* READ */
    TBL_CARD_FLD_FLASH_SIZE           = 15,  /* READ */
    TBL_CARD_FLD_DRAM_SIZE            = 16,  /* READ */
    TBL_CARD_FLD_PORT_NUM             = 17,  /* READ */
    TBL_CARD_FLD_UPTIME_DAY           = 18,  /* READ */
    TBL_CARD_FLD_UPTIME_HOUR          = 19,  /* READ */
    TBL_CARD_FLD_UPTIME_MIN           = 20,  /* READ */
    TBL_CARD_FLD_ATTACH_TIME          = 21,  /* RW */
    TBL_CARD_FLD_ATTACH_COUNT         = 22,  /* RW */
    TBL_CARD_FLD_TMPR_LOW             = 23,  /* RW */
    TBL_CARD_FLD_TMPR_HIGH            = 24,  /* RW */
    TBL_CARD_FLD_TMPR_CRITICAL        = 25,  /* RW */
    TBL_CARD_FLD_STATUS               = 26,  /* RW */
    TBL_CARD_FLD_EPLD_STATUS          = 27,  /* RW */
    TBL_CARD_FLD_REBOOT               = 28,  /* READ */
    TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX = 29,  /* RW */
    TBL_CARD_FLD_PLATFORM_TYPE        = 30,  /* RW */
    TBL_CARD_FLD_BOOTUP_DIAG_LEVEL    = 31,  /* RW */
    TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL = 32,  /* RW */
    TBL_CARD_FLD_FAN_SPEED_BOTTOM     = 33,  /* RW */
    TBL_CARD_FLD_FAN_SPEED_LOW        = 34,  /* RW */
    TBL_CARD_FLD_FAN_SPEED_HIGH       = 35,  /* RW */
    TBL_CARD_FLD_FAN_SPEED_FULL       = 36,  /* RW */
    TBL_CARD_FLD_BOOTCMD              = 37,  /* RW */
    TBL_CARD_FLD_UPDATE_EPLD_NAME     = 38,  /* RW */
    TBL_CARD_FLD_UPDATE_BOOTROM_NAME  = 39,  /* RW */
    TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME = 40,  /* RW */
    TBL_CARD_FLD_GLB_POE_ENABLE       = 41,  /* RW */
    TBL_CARD_FLD_GLB_POE_POWER_MAX    = 42,  /* RW */
    TBL_CARD_FLD_MAX                  = 43
} tbl_card_field_id_t;

/* TBL_CARD defines */
typedef struct
{
    uint32               id;
} tbl_card_key_t;

typedef struct
{
    tbl_card_key_t       key;
    uint32               slot;
    char                 product_name[GLB_NAME_INFO_STR_MAX];
    char                 hardware_name[GLB_NAME_INFO_STR_MAX];
    char                 company_name[GLB_NAME_INFO_STR_MAX];
    char                 package_name[GLB_NAME_INFO_STR_MAX];
    char                 enterprise_oid[GLB_NAME_INFO_STR_MAX];
    char                 hardware_type[GLB_NAME_INFO_STR_MAX];
    char                 serial_no[GLB_NAME_INFO_STR_MAX];
    char                 bootrom_ver[GLB_NAME_INFO_STR_MAX];
    char                 epld_ver[GLB_NAME_INFO_STR_MAX];
    char                 sw_ver[GLB_NAME_INFO_STR_MAX];
    char                 hardware_ver[GLB_NAME_INFO_STR_MAX];
    uint8                board_type;
    uint8                board_series;
    int32                flash_size;
    int32                dram_size;
    int32                port_num;
    int32                uptime_day;
    int32                uptime_hour;
    int32                uptime_min;
    sal_time_t           attach_time;
    uint32               attach_count;
    int32                tmpr_low;
    int32                tmpr_high;
    int32                tmpr_critical;
    uint32               status;              /*Card status, 1 means ready, 0 means not ready*/
    uint8                epld_status;
    uint32               reboot;
    int32                update_phy_firmware_port_idx;
    int32                platform_type;
    int32                bootup_diag_level;
    int32                next_bootup_diag_level;
    int32                fan_speed_bottom;
    int32                fan_speed_low;
    int32                fan_speed_high;
    int32                fan_speed_full;
    char                 bootcmd[GLB_NAME_INFO_STR_MAX]; /* move filename may contain '/' to tail */
    char                 update_epld_name[GLB_NAME_INFO_STR_MAX]; /* move filename may contain '/' to tail */
    char                 update_bootrom_name[GLB_NAME_INFO_STR_MAX]; /* move filename may contain '/' to tail */
    char                 update_phy_firmware_name[GLB_NAME_INFO_STR_MAX]; /* move filename may contain '/' to tail */
    char                 glb_poe_enable;
    uint32               glb_poe_power_max;
} tbl_card_t;

typedef struct
{
    tbl_card_t           *card_array[GLB_SLOT_NUM_MAX];
} tbl_card_master_t;

#endif /* !__TBL_CARD_DEFINE_H__ */

