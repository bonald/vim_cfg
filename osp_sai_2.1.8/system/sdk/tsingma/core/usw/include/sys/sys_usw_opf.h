/**
 @file sys_usw_opf.h

 @date 2009-10-22

 @version v2.0

 opf  -offset pool freelist
*/

#ifndef _SYS_USW_OPF_H_
#define _SYS_USW_OPF_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "ctc_debug.h"
#include "sal.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define START_OPF_TBL_NUM     1
#define MAX_OPF_TBL_NUM     255

#define SYS_OPF_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(opf, opf, NH_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

struct sys_usw_opf_s
{
    uint8 pool_type; /*enum sys_usw_opf_type*/
    uint8 pool_index;
    uint8 multiple;   /* [alloc only] */
    uint8 reverse;    /* [alloc only] */
};
typedef  struct sys_usw_opf_s sys_usw_opf_t;

struct sys_usw_opf_entry_s
{
    struct sys_usw_opf_entry_s* prev;
    struct sys_usw_opf_entry_s* next;
    uint32 size;
    uint32 offset;
};
typedef struct sys_usw_opf_entry_s sys_usw_opf_entry_t;

struct sys_usw_opf_master_s
{
    sys_usw_opf_entry_t*** ppp_opf_forward;
    sys_usw_opf_entry_t*** ppp_opf_reverse;
    uint32* start_offset_a[MAX_OPF_TBL_NUM];
    uint32* max_size_a[MAX_OPF_TBL_NUM];
    uint32* forward_bound[MAX_OPF_TBL_NUM];             /*forward_bound is max_offset + 1*/
    uint32* reverse_bound[MAX_OPF_TBL_NUM];             /*reverse_bound is min_offset. no -1. prevent to be negative */
    uint8* is_reserve[MAX_OPF_TBL_NUM];
    uint8  pool_num[MAX_OPF_TBL_NUM];
    uint8  count;

    char* type_desc[MAX_OPF_TBL_NUM];
    uint32* alloced_cnt[MAX_OPF_TBL_NUM];               /*store alloced offset counter*/
    uint32  type_bmp[8];
    sal_spinlock_t* p_mutex;

};
typedef struct sys_usw_opf_master_s sys_usw_opf_master_t;
typedef struct sys_usw_opf_used_node_s
{
    uint32 start;
    uint32 end;
    uint16 node_idx;
    uint8 reverse;
}sys_usw_opf_used_node_t;

typedef int32 (*OPF_TRAVERSE_FUNC)(uint8 lchip, sys_usw_opf_used_node_t* p_node, void* user_data);


/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_opf_init(uint8 lchip, uint8* pool_type, uint8 pool_num,const char* str);
extern int32
sys_usw_opf_free(uint8 lchip, uint8 pool_type, uint8 pool_index);
extern int32
sys_usw_opf_deinit(uint8 lchip, uint8 type);

extern int32
sys_usw_opf_init_offset(uint8 lchip, sys_usw_opf_t* opf, uint32 start_offset, uint32 max_size);
extern int32
sys_usw_opf_init_reverse_size(uint8 lchip, sys_usw_opf_t* opf, uint32 block_size);

extern int32
sys_usw_opf_alloc_offset(uint8 lchip, sys_usw_opf_t* opf, uint32 block_size, uint32* offset);
extern int32
sys_usw_opf_free_offset(uint8 lchip, sys_usw_opf_t* opf, uint32 block_size, uint32 offset);

extern int32
sys_usw_opf_get_bound(uint8 lchip, sys_usw_opf_t* opf, uint32* forward_bound, uint32* reverse_bound);

extern int32
sys_usw_opf_print_sample_info(uint8 lchip, sys_usw_opf_t* opf);

extern int32
sys_usw_opf_alloc_offset_from_position(uint8 lchip, sys_usw_opf_t* opf, uint32 block_size, uint32 begin);
extern int32
sys_usw_opf_alloc_offset_last(uint8 lchip, sys_usw_opf_t* opf, uint32 block_size, uint32* offset);
extern int32
sys_goldentgate_opf_print_type(uint8 lchip, uint8 opf_type, uint8 is_all);

extern uint32
sys_usw_opf_get_alloced_cnt(uint8 lchip, sys_usw_opf_t* opf);

extern int32
sys_usw_opf_traverse(uint8 lchip, sys_usw_opf_t* p_opf, OPF_TRAVERSE_FUNC cb, void* user_data, uint8 dir);

extern int32
sys_usw_opf_fprint_alloc_used_info(uint8 lchip, uint8 pool_type, sal_file_t p_f);
#ifdef __cplusplus
}
#endif

#endif /*_SYS_USW_OPF_H_*/

