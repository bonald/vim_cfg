#ifndef _CTC_DKIT_H_
#define _CTC_DKIT_H_


#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "sal_types.h"
#include "ctc_dkit_error.h"
#include "ctc_cli.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_DKITS_VERSION_STR        "V1.0.0"
#define CTC_DKITS_RELEASE_DATE       "2015-07-23"
#define CTC_DKITS_COPYRIGHT_TIME     "2015"
#ifdef GREATBELT
#define CTC_DKITS_CURRENT_GB_CHIP       "Greatbelt"
#endif
#ifdef GOLDENGATE
#define CTC_DKITS_CURRENT_GG_CHIP       "Goldengate"
#endif
#ifdef DUET2
#define CTC_DKITS_CURRENT_D2_CHIP       "Duet2"
#endif
#ifdef TSINGMA
#define CTC_DKITS_CURRENT_TM_CHIP       "Tsingma"
#endif
#define CTC_DKITS_MAX_LOCAL_CHIP_NUM          30

#define CTC_DKITS_CHIP_DESC       "Chip id on linecard"
#define CTC_DKITS_CHIP_ID_DESC       "Local chip id"

#define CTC_DKIT_LCHIP_CHECK(lchip)  \
    do { \
        if(lchip >=CTC_DKITS_MAX_LOCAL_CHIP_NUM)\
            {\
            if(dkits_debug)\
            ctc_cli_out("Lchip id exceed max chip id %d\n", CTC_DKITS_MAX_LOCAL_CHIP_NUM);\
            return DKIT_E_INVALID_CHIP_ID; \
            }\
    } while (0)

#define DKITS_IS_BIT_SET(flag, bit)        (((flag) & (1 << (bit))) ? 1 : 0)
#define DKITS_BIT_SET(flag, bit)           ((flag) = (flag) | (1 << (bit)))
#define DKITS_BIT_UNSET(flag, bit)         ((flag) = (flag) & (~(1 << (bit))))

#define DKITS_SET_FLAG(VAL, FLAG)          (VAL) = (VAL) | (FLAG)
#define DKITS_UNSET_FLAG(VAL, FLAG)        (VAL) = (VAL)&~(FLAG)
#define DKITS_FLAG_ISSET(VAL, FLAG)        (((VAL)&(FLAG)) == (FLAG))
#define DKITS_PTR_VALID_CHECK(ptr)          \
        if ((ptr) == NULL) \
        { \
            return CLI_ERROR; \
        }
extern sal_file_t dkits_log_file;
#define CTC_DKIT_PRINT(X, ...)  \
    do { \
            if (dkits_log_file){\
                sal_fprintf(dkits_log_file, X, ##__VA_ARGS__);}\
            else{\
                ctc_cli_out(X, ##__VA_ARGS__);}\
    } while (0)

extern bool dkits_debug;
#define CTC_DKIT_PRINT_DEBUG(X, ...)  \
    do { \
            if(dkits_debug) ctc_cli_out(X, ##__VA_ARGS__);\
    } while (0)

#define CTC_DKITS_HEX_FORMAT(S, F, V, W)                        \
                            ((sal_sprintf(F, "0x%%0%dX", W))   \
                             ? ((sal_sprintf(S, F, V)) ? S : NULL) : NULL)

#define CTC_DKITS_PRINT_FILE(FILE, X, ...)\
    do { \
        if(NULL == FILE){CTC_DKIT_PRINT(X, ##__VA_ARGS__);}\
        else{sal_fprintf(FILE, X, ##__VA_ARGS__);}\
    } while (0)

#define CTC_DKIT_PTR_VALID_CHECK(ptr)           \
        if ((ptr) == NULL)                      \
        {                                       \
            return DKIT_E_INVALID_PTR;          \
        }


#define CTC_DKIT_SERDES_CTL_PARAMETER_NUM  5

/**
 @brief define chip type
*/
enum ctc_dkit_chip_type_e
{
    CTC_DKIT_CHIP_NONE,       /**< [GB.GG]indicate this is a invalid chip */
    CTC_DKIT_CHIP_GREATBELT,  /**< [GB]indicate this is a greatbelt chip */
    CTC_DKIT_CHIP_GOLDENGATE, /**< [GG]indicate this is a goldengate chip */
    CTC_DKIT_CHIP_DUET2,      /**< [D2]indicate this is a duet2 chip */
    CTC_DKIT_CHIP_TSINGMA,      /**< [TM]indicate this is a tsingma chip */
    MAX_CTC_DKIT_CHIP_TYPE
};
typedef enum ctc_dkit_chip_type_e ctc_dkit_chip_type_t;

enum ctc_dkit_direction_e
{
    CTC_DKIT_INGRESS,        /**< Ingress direction */
    CTC_DKIT_EGRESS,         /**< Egress direction */
    CTC_DKIT_BOTH_DIRECTION  /**< Both Ingress and Egress direction */
};
typedef enum ctc_dkit_direction_e ctc_dkit_direction_t;

/**
 @defgroup api DKIT
 @{
*/
struct ctc_dkit_api_s
{
    /*1. Basic*/
    int32(*memory_process)(void*);
    int32(*show_interface_status)(void*);
    int32(*show_device_info)(void*);

    /*2. Normal*/
    int32(*install_captured_flow)(void*);
    int32(*clear_captured_flow)(void*);
    int32(*show_discard)(void*);
    int32(*show_discard_type)(void*);
    int32(*show_discard_stats)(void*);
    int32(*discard_to_cpu_en)(void*);
    int32(*discard_display_en)(void*);
    int32(*show_path)(void*);
    int32(*show_tcam_key_type)(uint8);
    int32(*tcam_capture_start)(void*);
    int32(*tcam_capture_stop)(void*);
    int32(*tcam_capture_show)(void*);
    int32(*tcam_scan)(void*);
    int32(*cfg_dump)(uint8 lchip, void*);
    int32(*cfg_decode)(uint8 lchip, void*);
    int32(*cfg_cmp)(uint8 lchip, void*, void*);
    int32(*cfg_usage)(uint8, void*);

    int32(*packet_dump)(uint8, void*, void*, void*);

    /*3. Advanced*/
    int32(*show_queue_id)(void*);
    int32(*show_queue_depth)(void*);
    int32(*show_sensor_result)(void*);
    int32(*serdes_ctl)(void*);
    int32(*integrity_check)(uint8, void*, void*, void*);

    /*4. dkit api for user*/
    int32(*read_table)(uint8 lchip, void*);
    int32(*write_table)(uint8 lchip, void*);
};
typedef struct ctc_dkit_api_s ctc_dkit_api_t;

/**@} end of @defgroup  api DKIT */

/**
 @defgroup discard DKIT
 @{
*/
struct ctc_dkit_discard_para_s
{
    uint8 on_line; /**<1: can not use bus info; 0:can use bus info*/
    uint8 captured;  /**<1: can use bus info; 0:can not use bus info*/
    uint8 captured_session;/**< captured session id*/
    uint8 history;

    uint32 reason_id;

    uint8 lchip;
    uint8 match_port;
    uint16 gport;

    uint8 discard_to_cpu_en;
    uint8 discard_display_en;
    uint8 rsv[2];

    sal_file_t p_wf;
};
typedef struct ctc_dkit_discard_para_s ctc_dkit_discard_para_t;
/**@} end of @defgroup  discard DKIT */

/**
 @defgroup memory DKIT
 @{
*/
#define CTC_DKITS_RW_BUF_SIZE             256
#define CTC_DKITS_MAX_ENTRY_WORD          128
#define CTC_DKITS_INVALID_TBL_ID          0xFFFF

enum ctc_dkit_memory_rw_type_e
{
    CTC_DKIT_MEMORY_LIST = 0,
    CTC_DKIT_MEMORY_READ,
    CTC_DKIT_MEMORY_WRITE,
    CTC_DKIT_MEMORY_RESET,
    CTC_DKIT_MEMORY_SHOW_TBL_BY_ADDR,
    CTC_DKIT_MEMORY_SHOW_TBL_BY_DATA,
    CTC_DKIT_MEMORY_CHECK,
    CTC_DKIT_MEMORY_MAX
} ;
typedef enum ctc_dkit_memory_rw_type_e ctc_dkit_memory_rw_type_t;
struct ctc_dkit_memory_para_s
{
    ctc_dkit_memory_rw_type_t type;
    uintptr  param[8];/*  0      1      2      3        4        5       6   */
                          /*read:  opcode, chip, index, tbl-id,  count    step*/
                          /*write: opcode, chip, index, tbl-id,  fld-id,  value, count */
    uint32 value[CTC_DKITS_MAX_ENTRY_WORD];
    uint32 mask[CTC_DKITS_MAX_ENTRY_WORD];

    uint8    buf[CTC_DKITS_RW_BUF_SIZE];/*table name*/
    uint8    buf2[CTC_DKITS_RW_BUF_SIZE];/*field name*/
    uint8    detail;
    uint8    grep;

    uint8    direct_io;
    uint8    rev[3];
};
typedef struct ctc_dkit_memory_para_s ctc_dkit_memory_para_t;
/**@} end of @defgroup  memory DKIT */

/**
 @defgroup tcam DKIT
 @{
*/
typedef uint32 ds_t[32];

enum ctc_dkits_tcam_type_flag_e
{
    CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL          = 1,
    CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL          = 2,
    CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL          = 4,
    CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX         = 8,
    CTC_DKITS_TCAM_TYPE_FLAG_IPNAT            = 0x10,
    CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID       = 0x20,
    CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE    = 0x40,
    CTC_DKITS_TCAM_TYPE_FLAG_ALL              = ((1U << 7) - 1)
};
typedef enum ctc_dkits_tcam_type_flag_e ctc_dkits_tcam_type_flag_t;

enum ctc_dkits_dump_packet_type_e
{
    CTC_DKITS_DUMP_PACKET_TYPE_CAPTURE,
    CTC_DKITS_DUMP_PACKET_TYPE_NETTX,
    CTC_DKITS_DUMP_PACKET_TYPE_NETRX,
    CTC_DKITS_DUMP_PACKET_TYPE_BSR,
    CTC_DKITS_DUMP_PACKET_TYPE_NUM
};
typedef enum ctc_dkits_dump_packet_type_e ctc_dkits_dump_packet_type_t;

enum ctc_dkits_tcam_bist_size_e
{
    CTC_DKITS_TCAM_BIST_SIZE_HALF,
    CTC_DKITS_TCAM_BIST_SIZE_SINGLE,
    CTC_DKITS_TCAM_BIST_SIZE_DOUBLE,
    CTC_DKITS_TCAM_BIST_SIZE_QUAD,
    CTC_DKITS_TCAM_BIST_SIZE_MAX
};
typedef enum ctc_dkits_tcam_bist_size_e ctc_dkits_tcam_bist_size_t;

struct ctc_dkits_tcam_info_s
{
    uint32 tcam_type;          /* CTC_DKITS_TCAM_TYPE_FLAG_XXX */
    uint32 key_type;
    uint32 priority;           /* tcam key lookup sequence */
    uint32 index;
    uint8 lchip;
    uint8 is_stress;
    uint8 key_size;
    uint8 mismatch_cnt;
};
typedef struct ctc_dkits_tcam_info_s ctc_dkits_tcam_info_t;

/**@} end of @defgroup tcam DKIT */

/**
 @defgroup tcam DKIT
 @{
*/
enum ctc_dkits_dump_func_flag_e
{
    CTC_DKITS_DUMP_FUNC_STATIC_IPE,
    CTC_DKITS_DUMP_FUNC_STATIC_BSR,
    CTC_DKITS_DUMP_FUNC_STATIC_EPE,
    CTC_DKITS_DUMP_FUNC_STATIC_ALL,
    CTC_DKITS_DUMP_FUNC_STATIC_HSS,
    CTC_DKITS_DUMP_FUNC_STATIC_SERDES,
    CTC_DKITS_DUMP_FUNC_DYN_FDB,
    CTC_DKITS_DUMP_FUNC_DYN_SCL,
    CTC_DKITS_DUMP_FUNC_DYN_ACL,
    CTC_DKITS_DUMP_FUNC_DYN_OAM,
    CTC_DKITS_DUMP_FUNC_DYN_IPUC,
    CTC_DKITS_DUMP_FUNC_DYN_IPMC,
    CTC_DKITS_DUMP_FUNC_DYN_IPFIX,
    CTC_DKITS_DUMP_FUNC_DYN_CID,
    CTC_DKITS_DUMP_FUNC_DYN_QUE,
    CTC_DKITS_DUMP_FUNC_ALL,
    CTC_DKITS_DUMP_FUNC_MAX
};
typedef enum ctc_dkits_dump_func_flag_e ctc_dkits_dump_func_flag_t;

enum ctc_dkits_dump_usage_type_e
{
    CTC_DKITS_DUMP_USAGE_TCAM_KEY,
    CTC_DKITS_DUMP_USAGE_HASH_BRIEF,
    CTC_DKITS_DUMP_USAGE_HASH_DETAIL,
    CTC_DKITS_DUMP_USAGE_HASH_MEM,
    CTC_DKITS_DUMP_USAGE_NUM
};
typedef enum ctc_dkits_dump_usage_type_e ctc_dkits_dump_usage_type_t;

struct ctc_dkits_dump_cfg_s
{
    uint32 func_flag;        /* ctc_dkits_dump_func_flag_t */
    char   file[256];          /* file name */
    uint8 lchip;
    uint8 data_translate;
    uint8 rsv[2];
};
typedef struct ctc_dkits_dump_cfg_s ctc_dkits_dump_cfg_t;

/**@} end of @defgroup tcam DKIT */

/**
 @defgroup captured DKIT
 @{
*/
enum ctc_dkit_captured_flow_type_e
{
    CTC_DKIT_CAPTURED_NONE = 0,
    CTC_DKIT_CAPTURED_IPV4,
    CTC_DKIT_CAPTURED_IPV6,
    CTC_DKIT_CAPTURED_MPLS,
    CTC_DKIT_CAPTURED_SLOW_PROTOCOL,
    CTC_DKIT_CAPTURED_ETHOAM,
    CTC_DKIT_CAPTURED_L4_SOURCE_PORT,
    CTC_DKIT_CAPTURED_L4_DEST_PORT
} ;
typedef enum ctc_dkit_captured_flow_type_e ctc_dkit_captured_flow_type_t;


struct ctc_dkit_captured_flow_info_s
{
     uint16 port;              /** source port*/
     uint16 dest_port;
     uint8  is_mcast;
     uint16 group_id;
     uint8  is_debugged_pkt;
     uint16 channel_id_0;
     uint16 channel_id_1;      /** Use channel_id 1~3 to debug, when from_cflex_channel is set to 1*/
     uint16 channel_id_2;
     uint16 channel_id_3;
     uint8  from_cflex_channel;
     uint32 mep_index;
     uint8  oam_dbg_en;
     mac_addr_t mac_da;
     mac_addr_t mac_sa;
     uint16 svlan_id;
     uint16 cvlan_id;
     uint16 ether_type;
     uint8 l3_type;
     uint8 l4_type;
     union
     {
         struct
         {
             ipv6_addr_t ip_da;
             ipv6_addr_t ip_sa;
         }ipv6;
         struct
         {
             ip_addr_t ip_da;
             ip_addr_t ip_sa;
             uint16 ip_check_sum;
         }ipv4;
         struct
         {
             uint32 mpls_label0;
             uint32 mpls_label1;
             uint32 mpls_label2;
             uint32 mpls_label3;
             uint32 mpls_label4;
             uint32 mpls_label5;
             uint32 mpls_label6;
             uint32 mpls_label7;
         }mpls;
         struct
         {
             uint8 sub_type;
             uint16 flags;
             uint8 code;
         }slow_proto;
         struct
         {
             uint8 level;
             uint8 version;
             uint8 opcode;
             uint32 rx_fcb;
             uint32 tx_fcf;
             uint32 tx_fcb;
         }ether_oam;
         struct
         {

         }ptp;
         struct
         {

         }fcoe;
         struct
         {

         }trill;
     }u_l3;
     union
     {
         struct
         {
           uint16 source_port;
           uint16 dest_port;
         }l4port;
     }u_l4;
};
typedef struct ctc_dkit_captured_flow_info_s ctc_dkit_captured_flow_info_t;

enum ctc_dkit_captured_flag_e
{
    CTC_DKIT_CAPTURED_CLEAR_NONE = 0,
    CTC_DKIT_CAPTURED_CLEAR_ALL,
    CTC_DKIT_CAPTURED_CLEAR_RESULT,
    CTC_DKIT_CAPTURED_CLEAR_FLOW,
    CTC_DKIT_CAPTURED_MAX
} ;
typedef enum ctc_dkit_captured_flag_e ctc_dkit_captured_flag_t;

enum ctc_dkit_flow_mode_e
{
    CTC_DKIT_FLOW_MODE_ALL = 0,
    CTC_DKIT_FLOW_MODE_IPE,
    CTC_DKIT_FLOW_MODE_EPE,
    CTC_DKIT_FLOW_MODE_BSR,
    CTC_DKIT_FLOW_MODE_OAM_RX,
    CTC_DKIT_FLOW_MODE_OAM_TX,
    CTC_DKIT_FLOW_MODE_METFIFO,
    CTC_DKIT_FLOW_MODE_MAX
};
typedef enum ctc_dkit_flow_mode_e ctc_dkit_flow_mode_t;

struct ctc_dkit_captured_s
{
      uint8 captured_session;/**< captured session id*/
      uint8 lchip;

      uint8 path_sel; /*flags for debug path, 0 is forwarding path, 1 is exception path, 2 is log path*/
      uint8 to_cpu_en;

      uint32 flag;        /**< flag for clear*/
      uint32 mode;       /** mode for capture, refer to ctc_dkit_flow_mode_t*/
      ctc_dkit_captured_flow_info_t flow_info;
      ctc_dkit_captured_flow_info_t flow_info_mask;
};
typedef struct ctc_dkit_captured_s ctc_dkit_captured_t;
/**@} end of @defgroup  captured DKIT */

/**
 @defgroup path DKIT
 @{
*/
struct ctc_dkit_path_para_s
{
    uint8 on_line; /**<1: can not use bus info; 0:can use bus info*/
    uint8 captured;  /**<1: can use bus info; 0:can not use bus info*/
    uint8 captured_session;/**< captured session id*/
    uint8 detail; /**< 1: detail display; 0: brief display*/

    uint32 flag;     /**< flag for clear*/
    uint8  lchip;
    uint8  rsv[3];
    uint32 mode;     /** mode for show path, refer to ctc_dkit_flow_mode_t*/
};
typedef struct ctc_dkit_path_para_s ctc_dkit_path_para_t;
/**@} end of @defgroup  path DKIT */

/**
 @defgroup monitor DKIT
 @{
*/
enum ctc_dkit_monitor_sensor_e
{
    CTC_DKIT_MONITOR_SENSOR_TEMP,
    CTC_DKIT_MONITOR_SENSOR_TEMP_NOMITOR,
    CTC_DKIT_MONITOR_SENSOR_VOL,
    CTC_DKIT_MONITOR_SENSOR_MAX
} ;
typedef enum ctc_dkit_monitor_sensor_e ctc_dkit_monitor_sensor_t;

struct ctc_dkit_monitor_para_s
{
    uint16 gport;  /**< 0xFFFF means show all port*/
    uint8 dir;    /**< ingress or egress queue monitor */
    uint8 sensor_mode;  /**< refer to ctc_dkit_monitor_sensor_t*/

    uint8 interval;/**< monitor temperature interval, unit:second*/
    uint8 enable;  /**< for temperature monitor*/
    uint8 temperature;/**< for temperature monitor*/
    uint8 log;/**< for temperature monitor*/

    uint8 power_off_temp;/**< for temperature monitor*/
    uint8 lchip;
    uint8 rsv[2];

    char str[32];/**< for temperature monitor*/
    sal_file_t p_wf;
};
typedef struct ctc_dkit_monitor_para_s ctc_dkit_monitor_para_t;
/**@} end of @defgroup  monitor DKIT */

/**
 @defgroup interface DKIT
 @{
*/
enum ctc_dkit_interface_type_e
{
    CTC_DKIT_INTERFACE_ALL = 0,
    CTC_DKIT_INTERFACE_MAC_ID,
    CTC_DKIT_INTERFACE_GPORT,
    CTC_DKIT_INTERFACE_MAC_EN,
    CTC_DKIT_INTERFACE_LINK_STATUS,
    CTC_DKIT_INTERFACE_SPEED,
    CTC_DKIT_INTERFACE_MAX
} ;
typedef enum ctc_dkit_interface_type_e ctc_dkit_interface_type_t;

enum ctc_dkit_interface_mac_mode_e
{
    CTC_DKIT_INTERFACE_SGMII,         /**<[GB.GG.D2] config serdes to SGMII mode */
    CTC_DKIT_INTERFACE_XFI,           /**<[GB.GG.D2] config serdes to XFI mode */
    CTC_DKIT_INTERFACE_XAUI,          /**<[GB.GG.D2] config serdes to XAUI mode */
    CTC_DKIT_INTERFACE_XLG,           /**<[GG.D2] config serdes to xlg mode */
    CTC_DKIT_INTERFACE_CG,            /**<[GG.D2] config serdes to cg mode */
    CTC_DKIT_INTERFACE_QSGMII,        /**<[GB.D2] config serdes to QSGMII mode */
    CTC_DKIT_INTERFACE_DXAUI,         /**<[GG.D2] config serdes to D-XAUI mode */
    CTC_DKIT_INTERFACE_2DOT5G,        /**<[GB.GG.D2] config serdes to 2.5g mode */
    CTC_DKIT_INTERFACE_USXGMII0,      /**<[D2] config serdes to USXGMII single mode */
    CTC_DKIT_INTERFACE_USXGMII1,      /**<[D2] config serdes to USXGMII multi 1G/2.5G mode */
    CTC_DKIT_INTERFACE_USXGMII2,      /**<[D2] config serdes to USXGMII multi 5G mode */
    CTC_DKIT_INTERFACE_XXVG,          /**<[D2] config serdes to XXVG mode */
    CTC_DKIT_INTERFACE_LG,            /**<[D2] config serdes to LG mode */
    CTC_DKIT_INTERFACE_MAC_MAX
};
typedef enum ctc_dkit_interface_mac_mode_e ctc_dkit_interface_mac_mode_t;

struct ctc_dkit_interface_para_s
{
    ctc_dkit_interface_type_t type ;  /**< 0xFFFF means all gport*/
    uint32 value;
    uint8 lchip;
    uint8 rsv[3];
};
typedef struct ctc_dkit_interface_para_s ctc_dkit_interface_para_t;
/**@} end of @defgroup  interface DKIT */



/**
 @defgroup serdes DKIT
 @{
*/
enum ctc_dkit_serdes_ctl_type_e
{
    CTC_DKIT_SERDIS_CTL_NONE = 0,
    CTC_DKIT_SERDIS_CTL_LOOPBACK,
    CTC_DKIT_SERDIS_CTL_PRBS,
    CTC_DKIT_SERDIS_CTL_EYE,
    CTC_DKIT_SERDIS_CTL_EYE_SCOPE,
    CTC_DKIT_SERDIS_CTL_FFE,
    CTC_DKIT_SERDIS_CTL_STATUS,
    CTC_DKIT_SERDIS_CTL_POLR_CHECK,
    CTC_DKIT_SERDIS_CTL_DFE,
    CTC_DKIT_SERDIS_CTL_GET_DFE
} ;
typedef enum ctc_dkit_serdes_ctl_type_e ctc_dkit_serdes_ctl_type_t;

enum ctc_dkit_serdes_loopback_e
{
    CTC_DKIT_SERDIS_LOOPBACK_INTERNAL = 0,
    CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL
} ;
typedef enum ctc_dkit_serdes_loopback_e ctc_dkit_serdes_loopback_t;

enum ctc_dkit_serdes_external_loopback_e
{
    CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE0,
    CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE1,
    CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE2
} ;
typedef enum ctc_dkit_serdes_external_loopback_e ctc_dkit_serdes_external_loopback_t;

enum ctc_dkit_serdes_ctl_e
{
    CTC_DKIT_SERDIS_CTL_ENABLE = 0,
    CTC_DKIT_SERDIS_CTL_DISABLE,
    CTC_DKIT_SERDIS_CTL_CEHCK,
    CTC_DKIT_SERDIS_CTL_MONITOR
} ;
typedef enum ctc_dkit_serdes_ctl_e ctc_dkit_serdes_ctl_t;

enum ctc_dkit_serdes_dfe_stat_e
{
    CTC_DKIT_SERDIS_CTL_DFE_OFF = 0,
    CTC_DKIT_SERDIS_CTL_DFE_ON
};
typedef enum ctc_dkit_serdes_dfe_stat_e ctc_dkit_serdes_dfe_stat_t;

enum ctc_dkit_serdes_eye_e
{
    CTC_DKIT_SERDIS_EYE_HEIGHT = 0,
    CTC_DKIT_SERDIS_EYE_WIDTH,
    CTC_DKIT_SERDIS_EYE_WIDTH_SLOW,
    CTC_DKIT_SERDIS_EYE_ALL
};
typedef enum ctc_dkit_serdes_eye_e ctc_dkit_serdes_eye_t;

enum ctc_dkit_eye_metric_precision_type_e
{
    CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3,
    CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E6,
    CTC_DKIT_EYE_METRIC_PRECISION_TYPE_NUM
};
typedef enum ctc_dkit_eye_metric_precision_type_e ctc_dkit_eye_metric_precision_type_t;

struct ctc_dkit_serdes_para_ffe_s
{
    uint16 coefficient0_min;
    uint16 coefficient0_max;
    uint16 coefficient1_min;
    uint16 coefficient1_max;
    uint16 coefficient2_min;
    uint16 coefficient2_max;
    uint16 coefficient3_min;
    uint16 coefficient3_max;
    uint16 coefficient4_min;
    uint16 coefficient4_max;
};
typedef struct ctc_dkit_serdes_para_ffe_s ctc_dkit_serdes_para_ffe_t;

struct ctc_dkit_serdes_ctl_para_s
{
    uint32 serdes_id ;
    uint8 lchip;
    uint8 log;
    uint8 rsv[2];
    ctc_dkit_serdes_ctl_type_t type;
    ctc_dkit_eye_metric_precision_type_t precision;

    uint32 para[CTC_DKIT_SERDES_CTL_PARAMETER_NUM]; /*CTC_DKIT_SERDIS_CTL_LOOPBACK : para[0] = ctc_dkit_serdes_ctl_t;   para[1] = ctc_dkit_serdes_loopback_t*/
                                   /*CTC_DKIT_SERDIS_CTL_PRBS     : para[0] = ctc_dkit_serdes_ctl_t;   para[1] = prbs pattern
                                                   para[2] = is_keep;  para[3] = delay before check;  para[4] = error count*/
                                   /*CTC_DKIT_SERDIS_CTL_EYE      : para[0] = ctc_dkit_serdes_eye_t;   para[1] = read times*/
                                   /*CTC_DKIT_SERDIS_CTL_FFE      : para[0] = dest serdes id;          para[1] = prbs pattern
                                                   para[2] = coefficient sum threshold        para[3] = delay*/
                                   /*CTC_DKIT_SERDIS_CTL_POLR_CHECK  : para[0] = paired_serdes_id */
                                   /*CTC_DKIT_SERDIS_CTL_DFE  : para[0] = dfe enable status
                                     para[1] bit 0~7 = tap1, para[1] bit 8~15 = tap2, para[1] bit 16~23 = tap3, para[1] bit 24~31 = tap4
                                     para[2] bit 0~7 = tap5*/
    char* str;
    ctc_dkit_serdes_para_ffe_t ffe;
};
typedef struct ctc_dkit_serdes_ctl_para_s ctc_dkit_serdes_ctl_para_t;
/**@} end of @defgroup  serdes DKIT */

extern uint8 g_api_lchip;
extern uint8 g_ctcs_api_en;

#ifdef __cplusplus
}
#endif

#endif

