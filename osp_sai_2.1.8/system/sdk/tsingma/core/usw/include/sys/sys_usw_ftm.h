/**
 @file sys_usw_ftm.h

 @date 2011-11-16

 @version v2.0

 alloc memory and offset
*/

#ifndef _SYS_GREAT_BELT_FTM_H
#define _SYS_GREAT_BELT_FTM_H
#ifdef __cplusplus
extern "C" {
#endif
#include "ctc_ftm.h"


#define SYS_FTM_DS_STATS_HW_INDIRECT_ADDR 0x1600000
#define SYS_FTM_CHANGE_TABLE_NUM 18

#define SYS_FTM_GET_TCAM_AD_IDX( key_tblid, key_idx)  sys_usw_ftm_get_tcam_ad_index(key_tblid, key_idx)

#define SYS_FTM_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(ftm, ftm, FTM_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_FTM_SPEC(lchip, spec_id)   sys_usw_ftm_get_spec(lchip, spec_id)

enum sys_ftm_dyn_entry_type_e
{
    SYS_FTM_DYN_ENTRY_GLB_MET,
    SYS_FTM_DYN_ENTRY_VSI,
    SYS_FTM_DYN_ENTRY_ECMP,
    SYS_FTM_DYN_ENTRY_LOGIC_MET,
    SYS_FTM_DYN_ENTRY_MAX
};
typedef enum sys_ftm_dyn_entry_type_e sys_ftm_dyn_entry_type_t;

enum sys_ftm_dyn_nh_type_e
{
    SYS_FTM_DYN_NH0,
    SYS_FTM_DYN_NH1,
    SYS_FTM_DYN_NH2,
    SYS_FTM_DYN_NH3,
    SYS_FTM_DYN_NH_MAX
};

typedef struct sys_ftm_specs_info_s
{
    uint32 used_size;
    uint8 type;
}sys_ftm_specs_info_t;

typedef int32 (*sys_ftm_callback_t)(uint8 lchip, sys_ftm_specs_info_t* specs_info);

enum sys_ftm_hash_key_e
{
    SYS_FTM_HASH_KEY_MPLS,
    SYS_FTM_HASH_KEY_OAM,
    SYS_FTM_HASH_KEY_MAX
};
typedef int32 (*sys_ftm_rsv_hash_key_cb_t)(uint8, uint8);

struct sys_ftm_lpm_pipeline_info_s
{
    uint32 pipeline0_size;
    uint32 pipeline1_size;
};
typedef struct sys_ftm_lpm_pipeline_info_s sys_ftm_lpm_pipeline_info_t;
typedef int32 (*sys_ftm_change_mem_cb_t)(uint8 lchip, void* user_data);
extern int32
sys_usw_ftm_tcam_io(uint8 lchip, uint32 index, uint32 cmd, void* val);

extern uint32
sys_usw_ftm_tcam_map_index(uint32 tbl_id, uint32 index);

extern int32
sys_usw_ftm_register_callback(uint8 lchip, uint32 spec_type, sys_ftm_callback_t func);

extern int32
sys_usw_ftm_register_rsv_key_cb(uint8 lchip, uint32 hash_key_type, sys_ftm_rsv_hash_key_cb_t func);

extern int32
sys_usw_ftm_get_dyn_entry_size(uint8 lchip, sys_ftm_dyn_entry_type_t type,
                                     uint32* p_size);

extern int32
sys_usw_ftm_lkp_register_init(uint8 lchip);

extern int32
sys_usw_ftm_get_tcam_ad_index(uint8 lchip, uint32 key_tbl_id,  uint32 key_index);

extern int32
sys_usw_ftm_query_table_entry_num(uint8 lchip, uint32 table_id, uint32* entry_num);
#if 0
extern int32
sys_usw_ftm_query_lpm_model(uint8 lchip, uint32* lpm_model);
#endif
extern int32
sys_usw_ftm_get_dynamic_table_info(uint8 lchip, uint32 tbl_id,
                                         uint8* dyn_tbl_idx,
                                         uint32* entry_enum,
                                         uint32* nh_glb_entry_num);

extern  int32
sys_usw_ftm_alloc_table_offset(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint8 multi, uint32* offset);

extern  int32
sys_usw_ftm_free_table_offset(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);

extern int32
sys_usw_ftm_alloc_table_offset_from_position(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);

extern  int32
sys_usw_ftm_get_alloced_table_count(uint8 lchip, uint32 table_id, uint32* count);

extern int32
sys_usw_ftm_mem_alloc(uint8 lchip, ctc_ftm_profile_info_t* profile_info);

extern int32
sys_usw_ftm_show_alloc_info(uint8 lchip);

extern int32
sys_usw_ftm_get_spec(uint8 lchip, uint32 spec);

extern int32
sys_usw_ftm_show_alloc_info_detail(uint8 lchip);

extern int32
sys_usw_ftm_query_table_entry_size(uint8 lchip, uint32 table_id, uint32* entry_size);

/*init size is an array[2][3]
  init_size[0][0]   private ipv6 double key size;
  init_size[0][1]   private ipv6 single key size;
  init_size[0][2]   private ipv4 key size;

  init_size[1][0]   private ipv6 double key size;
  init_size[1][1]   private ipv6 single key size;
  init_size[1][2]   private ipv4 key size;
*/
extern int32
sys_usw_ftm_query_lpm_tcam_init_size(uint8 lchip, uint32** init_size);

extern int32
sys_usw_ftm_get_lpm_pipeline_info(uint8 lchip, sys_ftm_lpm_pipeline_info_t* p_pipeline_info);

extern int32
sys_usw_ftm_get_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly);

extern int32
sys_usw_ftm_set_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly);

extern int32
sys_usw_ftm_set_profile_specs(uint8 lchip, uint32 spec_type, uint32 value);

extern int32
sys_usw_ftm_get_profile_specs(uint8 lchip, uint32 spec_type, uint32* value);

extern int32
sys_usw_ftm_mem_free(uint8 lchip);
extern int32
sys_usw_ftm_adjust_flow_tcam(uint8 lchip, uint8 expand_blocks, uint8 compress_blocks);
extern int32
sys_usw_ftm_get_working_status(uint8 lchip, uint8* p_status);
extern int32
sys_usw_ftm_mem_change(uint8 lchip, ctc_ftm_change_profile_t* p_profile);
extern int32
sys_usw_ftm_set_working_status(uint8 lchip, uint8 status);
extern int32
sys_usw_ftm_register_change_mem_cb(uint8 lchip, sys_ftm_change_mem_cb_t cb);
#define SYS_USW_FTM_CHECK_NEED_SYNC(lchip) \
do{\
    uint8 work_status = 0;\
    sys_usw_ftm_get_working_status(lchip, &work_status);\
    if(3 ==  work_status)\
    {\
        return CTC_E_NONE;\
    }\
}while(0)
#ifdef __cplusplus
}
#endif

#endif

