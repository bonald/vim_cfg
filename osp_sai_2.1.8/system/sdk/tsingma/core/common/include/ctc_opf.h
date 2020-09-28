/**
 @file ctc_opf.h

 @date 2009-10-22

 @version v2.0

 opf  -offset pool freelist
*/

#ifndef _CTC_OPF_H_
#define _CTC_OPF_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "common/include/ctc_debug.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_OPF_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(opf, opf, NH_SYS, level, FMT, ## __VA_ARGS__);  \
    } while (0)

enum ctc_opf_type
{
    /*module ower self-define,format OPF_TYPE_XXXX*/
   CTC_OPF_MPLS_TUNNEL_ID,
   CTC_OPF_NHID,
   CTC_OPF_FID,
   CTC_OPF_L3IF_ID,
   CTC_OPF_MCAST_GROUP_ID,
   CTC_OPF_GLOBAL_DSNH_OFFSET,
   CTC_OPF_POLICER_ID,
   CTC_OPF_STATSID_ID,
   CTC_OPF_VLAN_PORT,
   CTC_OPF_LOGIC_PORT,
   CTC_OPF_APS_GROUP_ID,
   CTC_OPF_ARP_ID,
   CTC_OPF_ACL_ENTRY_ID,
   CTC_OPF_ACL_GROUP_ID,
   CTC_OPF_SCL_ENTRY_ID,
   CTC_OPF_SCL_GROUP_ID,
   CTC_OPF_CUSTOM_ID_START,
   CTC_OPF_CUSTOM_ID_END = CTC_OPF_CUSTOM_ID_START + 63,
   CTC_MAX_OPF_TBL_NUM
};

struct ctc_opf_s
{
    uint8 pool_type; /*enum ctc_opf_type*/
    uint8 pool_index;
    uint8 multiple;   /* [alloc only] */
    uint8 reverse;    /* [alloc only] */
};
typedef  struct ctc_opf_s ctc_opf_t;

struct ctc_opf_entry_s
{
    struct ctc_opf_entry_s* prev;
    struct ctc_opf_entry_s* next;
    uint32 size;
    uint32 offset;
};
typedef struct ctc_opf_entry_s ctc_opf_entry_t;

struct ctc_opf_master_s
{
    ctc_opf_entry_t*** ppp_opf_forward;
    ctc_opf_entry_t*** ppp_opf_reverse;
    uint32* start_offset_a[CTC_MAX_OPF_TBL_NUM];
    uint32* max_size_a[CTC_MAX_OPF_TBL_NUM];
    uint32* forward_bound[CTC_MAX_OPF_TBL_NUM];             /*forward_bound is max_offset + 1*/
    uint32* reverse_bound[CTC_MAX_OPF_TBL_NUM];             /*reverse_bound is min_offset. no -1. prevent to be negative */
    uint8* is_reserve[CTC_MAX_OPF_TBL_NUM];
    uint8              pool_num[CTC_MAX_OPF_TBL_NUM];
    uint8  count;
};
typedef struct ctc_opf_master_s ctc_opf_master_t;

/****************************************************************************
 *
* Function
*
*****************************************************************************/

extern int32
ctc_opf_init(uint8 pool_type, uint8 pool_num);
extern int32
ctc_opf_deinit(uint8 lchip, uint8 type);
extern int32
ctc_opf_free(uint8 pool_type, uint8 pool_index);

extern int32
ctc_opf_init_offset(ctc_opf_t* opf, uint32 start_offset, uint32 max_size);
extern int32
ctc_opf_init_reverse_size(ctc_opf_t* opf, uint32 block_size);

extern int32
ctc_opf_alloc_offset(ctc_opf_t* opf, uint32 block_size, uint32* offset);
extern int32
ctc_opf_free_offset(ctc_opf_t* opf, uint32 block_size, uint32 offset);

extern int32
ctc_opf_get_bound(ctc_opf_t* opf, uint32* forward_bound, uint32* reverse_bound);

extern int32
ctc_opf_print_alloc_info(ctc_opf_t* opf);
extern int32
ctc_opf_print_sample_info(ctc_opf_t* opf);

extern int32
ctc_opf_alloc_offset_from_position(ctc_opf_t* opf, uint32 block_size, uint32 begin);
extern int32
ctc_opf_alloc_offset_last(ctc_opf_t* opf, uint32 block_size, uint32* offset);

#ifdef __cplusplus
}
#endif

#endif /*_ctc_OPF_H_*/

