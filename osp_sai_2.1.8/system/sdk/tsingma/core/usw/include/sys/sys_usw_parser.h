/**
 @file sys_usw_parser.h

 @date 2009-12-22

 @version v2.0

---file comments----
*/

#ifndef _SYS_USW_PARSER_H
#define _SYS_USW_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_const.h"
#include "ctc_parser.h"
#include "ctc_debug.h"

#define SYS_PARSER_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(parser, parser, PARSER_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

#define SYS_PARSER_DBG_PARAM(FMT, ...) SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ##__VA_ARGS__)
#define SYS_PARSER_DBG_INFO(FMT, ...) SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)

/*set tpid macro*/
#define SYS_PAS_ETH_CVLAN_TPID          0x8100
#define SYS_PAS_ETH_BVLAN_TPID          0x88a8
#define SYS_PAS_ETH_SVLAN_TPID0         0x88a8
#define SYS_PAS_ETH_SVLAN_TPID1         0x9100
#define SYS_PAS_ETH_SVLAN_TPID2         0x8100
#define SYS_PAS_ETH_SVLAN_TPID3         0x88a8
#define SYS_PAS_ETH_ITAG_TPID           0x88e7

#define SYS_PAS_ETH_MAX_LENGTH          1536
#define SYS_PAS_ETH_VLAN_PAS_NUM        2

#define SYS_PAS_PBB_VLAN_PAS_NUM        2

/**
 @brief  calculate and generate hash key method type
*/
enum sys_usw_parser_gen_hash_type_e
{
    SYS_USW_PARSER_GEN_HASH_TYPE_CRC,
    SYS_USW_PARSER_GEN_HASH_TYPE_XOR,
    SYS_USW_PARSER_GEN_HASH_TYPE_NUM
};
typedef enum sys_usw_parser_gen_hash_type_e ctc_usw_parser_gen_hash_type_t;

/**
 @brief  parser ecmp/linkagg/flowid hash type
*/
enum sys_parser_hash_type_e
{
    SYS_PARSER_HASH_TYPE_CRC,
    SYS_PARSER_HASH_TYPE_XOR,
    SYS_PARSER_HASH_TYPE_NUM
};
typedef enum sys_parser_hash_type_e sys_parser_hash_type_t;

enum sys_parser_hash_usage_e
{
    SYS_PARSER_HASH_USAGE_ECMP,     /**< hash application for ecmp */
    SYS_PARSER_HASH_USAGE_LINKAGG,  /**< hash application for linkagg */
    SYS_PARSER_HASH_USAGE_FLOW,     /**< hash application for dlb and efd */
    SYS_PARSER_HASH_USAGE_NUM,      /**< hash application num */
};
typedef enum sys_parser_hash_usage_e sys_parser_hash_usage_t;

enum sys_parser_layer4_udf_field_e
{
    SYS_PARSER_LAYER4_UDF_FIELD_LAYER4_TYPE,
    SYS_PARSER_LAYER4_UDF_FIELD_DEST_PORT,
    SYS_PARSER_LAYER4_UDF_FIELD_SRC_PORT,
    SYS_PARSER_LAYER4_UDF_FIELD_FRAGINFO_0,
    SYS_PARSER_LAYER4_UDF_FIELD_FRAGINFO_1,
    SYS_PARSER_LAYER4_UDF_FIELD_IP_VERSION,
    SYS_PARSER_LAYER4_UDF_FIELD_NUM
};
typedef enum sys_parser_layer4_udf_field_e sys_parser_layer4_udf_field_t;

enum sys_parser_type_flag_e
{
    SYS_PARSER_TYPE_FLAG_IGS_OUTER = 1U << 0,
    SYS_PARSER_TYPE_FLAG_IGS_INNER = 1U << 1,
    SYS_PARSER_TYPE_FLAG_EGS_OUTER = 1U << 2,
};
typedef enum sys_parser_type_flag_e sys_parser_type_flag_t;

/**
 @brief  layer4 flag option ctl fields
*/
struct sys_parser_range_op_ctl_s
{
    uint8  type;
    uint32 min_value;
    uint32 max_value;
};
typedef struct sys_parser_range_op_ctl_s sys_parser_range_op_ctl_t;

/**
 @brief set tpid
*/
extern int32
sys_usw_parser_set_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16 tpid);

/**
 @brief get tpid with some type
*/
extern int32
sys_usw_parser_get_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16* tpid);

/**
 @brief set max_length,based on the value differentiate type or length
*/
extern int32
sys_usw_parser_set_max_length_filed(uint8 lchip, uint16 max_length);

/**
 @brief get max_length value
*/
extern int32
sys_usw_parser_get_max_length_filed(uint8 lchip, uint16* max_length);

/**
 @brief set vlan parser num
*/
extern int32
sys_usw_parser_set_vlan_parser_num(uint8 lchip, uint8 vlan_num);

/**
 @brief get vlan parser num
*/
extern int32
sys_usw_parser_get_vlan_parser_num(uint8 lchip, uint8* vlan_num);

/**
 @brief set pbb header info
*/
extern int32
sys_usw_parser_set_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* pbb_header);

/**
 @brief get pbb header info
*/
extern int32
sys_usw_parser_get_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* pbb_header);

/**
 @brief mapping l3type
*/
extern int32
sys_usw_parser_mapping_l3_type(uint8 lchip, uint8 index, ctc_parser_l2_protocol_entry_t* entry);

/**
 @brief set the entry invalid based on the index
*/
extern int32
sys_usw_parser_unmapping_l3_type(uint8 lchip, uint8 index);

/**
 @brief enable or disable parser layer3 type
*/
extern int32
sys_usw_parser_enable_l3_type(uint8 lchip, ctc_parser_l3_type_t l3_type, bool enable);

/**
 @brief set parser trill header info
*/
extern int32
sys_usw_parser_set_trill_header(uint8 lchip, ctc_parser_trill_header_t* trill_header);

/**
 @brief get parser trill header info
*/
extern int32
sys_usw_parser_get_trill_header(uint8 lchip, ctc_parser_trill_header_t* trill_header);

/**
 @brief add l3type,can add a new l3type,addition offset for the type,can get the layer4 type
*/
extern int32
sys_usw_parser_mapping_l4_type(uint8 lchip, uint8 index, ctc_parser_l3_protocol_entry_t* entry);

/**
 @brief set the entry invalid based on the index
*/
extern int32
sys_usw_parser_unmapping_l4_type(uint8 lchip, uint8 index);

/**
 @brief set range op ctl
*/
extern int32
sys_usw_parser_set_range_op_ctl(uint8 lchip, uint8 range_id, sys_parser_range_op_ctl_t* p_range_ctl);

/**
 @brief set layer4 flex header
*/
extern int32
sys_usw_parser_set_l4flex_header(uint8 lchip, ctc_parser_l4flex_header_t* l4flex_header);

/**
 @brief get layer4 flex header
*/
extern int32
sys_usw_parser_get_l4flex_header(uint8 lchip, ctc_parser_l4flex_header_t* l4flex_header);

/**
 @brief set layer4 app ctl
*/
extern int32
sys_usw_parser_set_l4app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* l4app_ctl);

/**
 @brief get layer4 app ctl
*/
extern int32
sys_usw_parser_get_l4app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* l4app_ctl);

/**
 @brief set ecmp hash field
*/
extern int32
sys_usw_parser_set_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* p_hash_ctl);

/**
 @brief set ecmp hash field
*/
extern int32
sys_usw_parser_set_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl);

/**
 @brief get ecmp hash field
*/
extern int32
sys_usw_parser_get_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* p_hash_ctl);

/**
 @brief get ecmp hash field
*/
extern int32
sys_usw_parser_get_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl);

/**
 @brief set parser global config info
*/
extern int32
sys_usw_parser_set_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* global_cfg);

/**
 @brief get parser global config info
*/
extern int32
sys_usw_parser_get_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* global_cfg);

/**
 @brief init parser module
*/
extern int32
sys_usw_parser_init(uint8 lchip, void* parser_global_cfg);

extern int32
sys_usw_parser_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

