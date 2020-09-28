#if (FEATURE_MODE == 0)
/**
 @file sys_usw_ipfix.h

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-17

 @version v3.0
*/
#ifndef _SYS_USW_IPFIX_H
#define _SYS_USW_IPFIX_H


/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_ipfix.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define IPFIX_IPV4_MASK(ip, len)                              \
{  \
    uint32 temp;  \
    temp = (ip) >> (31 - (len-1));  \
    (ip) = temp << (31 - (len-1));  \
}

#define IPFIX_IPV6_MASK(ip, len)                              \
    {                                                       \
        uint8 feedlen = CTC_IPV6_ADDR_LEN_IN_BIT - (len);   \
        uint8 sublen = feedlen % 32;                        \
        int8 index = feedlen / 32;                          \
        if (sublen)                                          \
        {                                                   \
            uint32 mask = ~((1 << sublen) - 1);             \
            (ip)[index] &= mask;                            \
        }                                                   \
        index--;                                            \
        for (; index > 0; index--)                          \
        {                                                   \
            (ip)[4-index] = 0;                                \
        }                                                   \
    }

#define SYS_IPFIX_CHECK_DIR(dir) \
    if ((dir) >= CTC_BOTH_DIRECTION) \
    { \
        return CTC_E_INVALID_PARAM; \
    }

#define IPFIX_IPV4_MASK_LEN_CHECK(len)         \
    {                                                 \
        if ((len > CTC_IPV4_ADDR_LEN_IN_BIT) || (len < 1)){  \
            return CTC_E_INVALID_PARAM; }               \
    }

#define IPFIX_IPV6_MASK_LEN_CHECK(len)         \
    {                                                 \
        if (len > CTC_IPV6_ADDR_LEN_IN_BIT){  \
            return CTC_E_INVALID_PARAM; }               \
    }

enum sys_ipfix_traverse_action_e
{
    SYS_IPFIX_DUMP_ENYRY_INFO = 0x0011,
};
typedef enum sys_ipfix_traverse_action_e sys_ipfix_traverse_action_t;

enum sys_ipfix_hash_type_e
{
    SYS_IPFIX_HASH_TYPE_INVALID                     = 0x0,
    SYS_IPFIX_HASH_TYPE_L2                          = 0x1,
    SYS_IPFIX_HASH_TYPE_L2L3                        = 0x2,
    SYS_IPFIX_HASH_TYPE_IPV4                      = 0x3,
    SYS_IPFIX_HASH_TYPE_IPV6                      = 0x4,
    SYS_IPFIX_HASH_TYPE_MPLS                      = 0x5,
};
typedef enum sys_ipfix_hash_type_e sys_ipfix_hash_type_t;

enum sys_ipfix_dest_type_e
{
    SYS_IPFIX_UNICAST_DEST=1,
    SYS_IPFIX_L2MC_DEST=2,
    SYS_IPFIX_L3MC_DEST=3,
    SYS_IPFIX_BCAST_DEST=4,
    SYS_IPFIX_UNKNOW_PKT_DEST=5,
    SYS_IPFIX_UNION_DEST = 6
};
typedef enum sys_ipfix_dest_type_e sys_ipfix_dest_type_t;

enum sys_ipfix_sub_dest_type_e
{
    SYS_IPFIX_SUB_APS_DEST=0,
    SYS_IPFIX_SUB_ECMP_DEST,
    SYS_IPFIX_SUB_LINKAGG_DEST
};
typedef enum sys_ipfix_sub_dest_type_e sys_ipfix_sub_dest_type_t;

#define SYS_IPFIX_MAX_IP_MASK_LEN 31
#define SYS_IPFIX_BUF_MAX     32
#define IPFIX_SET_HW_IP6(d, s)    SYS_USW_REVERT_IP6(d, s)

enum sys_ipfix_export_reason_e
{
    SYS_IPFIX_EXPORTREASON_NO_EXPORT,
    SYS_IPFIX_EXPORTREASON_EXPIRED,
    SYS_IPFIX_EXPORTREASON_TCP_SESSION_CLOSE,
    SYS_IPFIX_EXPORTREASON_PACKET_COUNT_OVERFLOW  ,
    SYS_IPFIX_EXPORTREASON_BYTE_COUNT_OVERFLOW ,
    SYS_IPFIX_EXPORTREASON_NEW_FLOW_EXPORT   ,
    SYS_IPFIX_EXPORTREASON_DESTINATION_CHANGE   ,
    SYS_IPFIX_EXPORTREASON_TS_COUNT_OVERFLOW
};

typedef enum sys_ipfix_export_reason_e sys_ipfix_export_reason_t;
/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_ipfix_init(uint8 lchip, void* p_global_cfg);
extern int32
sys_usw_ipfix_sync_data(uint8 lchip, void* p_dma_info);
extern int32
sys_usw_ipfix_set_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel);
extern int32
sys_usw_ipfix_set_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg);
extern int32
sys_usw_ipfix_get_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg);
extern int32
sys_usw_ipfix_set_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg);
extern int32
sys_usw_ipfix_get_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg);
extern int32
sys_usw_ipfix_register_cb(uint8 lchip, ctc_ipfix_fn_t callback,void *userdata);
int32
sys_usw_ipfix_get_cb(uint8 lchip, void **cb, void** user_data);

extern int32
sys_usw_ipfix_get_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key, uint32* p_rst_hit, uint32* p_rst_key_index);
extern int32
sys_usw_ipfix_add_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key);
extern int32
sys_usw_ipfix_add_entry_by_index(uint8 lchip, ctc_ipfix_data_t* p_key, uint32 index);
extern int32
sys_usw_ipfix_add_ad_by_index(uint8 lchip, ctc_ipfix_data_t* p_key, uint32 index);
extern int32
sys_usw_ipfix_delete_ad_by_index(uint8 lchip, uint32 index, uint8 dir);
extern int32
sys_usw_ipfix_delete_entry_by_index(uint8 lchip, uint8 type, uint32 index, uint8 dir);
extern int32
sys_usw_ipfix_delete_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key);
extern uint32
sys_usw_ipfix_get_flow_cnt(uint8 lchip, uint32 detail);
extern int32
sys_usw_ipfix_get_ad_by_index(uint8 lchip, uint32 index, ctc_ipfix_data_t* p_out);
extern int32
sys_usw_ipfix_get_entry_by_index(uint8 lchip, uint32 index, uint8 dir, uint8 key_type, ctc_ipfix_data_t* p_out);
extern int32
sys_usw_ipfix_traverse(uint8 lchip, void* fn, ctc_ipfix_traverse_t* p_data, uint8 is_remove);
extern int32
sys_usw_ipfix_show_status(uint8 lchip, void *user_data);
extern int32
sys_usw_ipfix_show_stats(uint8 lchip, void *user_data);
extern int32
sys_usw_ipfix_clear_stats(uint8 lchip, void * user_data);
extern int32
sys_usw_ipfix_show_entry_info(ctc_ipfix_data_t* p_info, void* user_data);
extern int32
_sys_usw_ipfix_export_stats(uint8 lchip, uint8 exp_reason, uint64 pkt_cnt, uint64 byte_cnt);
extern int32
sys_usw_ipfix_deinit(uint8 lchip);
#endif

#endif

