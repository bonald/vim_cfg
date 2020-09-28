#ifndef _TCAM_MODEL_H_
#define _TCAM_MODEL_H_

#include "goldengate/include/drv_lib.h"

#define TCAM_MODEL_PTR_VALID_CHECK(ptr) \
    if ( NULL == (ptr) )                \
    {                                   \
        return DRV_E_INVALID_PTR;       \
    }

/* chip id check macro define */
#define TCAM_CHIP_ID_VALID_CHECK(chip_id_offset)\
if ( (chip_id_offset) >= (MAX_LOCAL_CHIP_NUM))                   \
{                                        \
    SAL_LOG_DEBUG("Tcam Out of chip_id number\n");\
    return DRV_E_INVALID_CHIP;           \
}

typedef enum tcam_data_mask_e
{
    TCAM_DATA,
    TCAM_MASK
}tcam_data_mask_t;

struct cm_tcam_info_s
{
    bool tcam_model_inited[MAX_LOCAL_CHIP_NUM];
    uint32* int_tcam_data_base[MAX_LOCAL_CHIP_NUM];
    uint32* int_tcam_mask_base[MAX_LOCAL_CHIP_NUM];
    uint32* int_tcam_wbit[MAX_LOCAL_CHIP_NUM];
    uint32* int_lpm_tcam_data_base[MAX_LOCAL_CHIP_NUM];
    uint32* int_lpm_tcam_mask_base[MAX_LOCAL_CHIP_NUM];
    uint32* int_lpm_tcam_wbit[MAX_LOCAL_CHIP_NUM];
    uint32* ext_tcam_data_base[MAX_LOCAL_CHIP_NUM];
    uint32* ext_tcam_mask_base[MAX_LOCAL_CHIP_NUM];
    uint32* ext_tcam_wbit[MAX_LOCAL_CHIP_NUM];
    /* Embeded TCAM (8K * 80bits) */
    uint32 max_int_tcam_data_base;
    uint32 max_int_tcam_mask_base;
    /* LPM and userId hash resolve conflict embeded TCAM (256 * 80bits) */
    uint32 max_int_lpm_tcam_data_base;
    uint32 max_int_lpm_tcam_mask_base;
    /* External TCAM (TBD), current none, code reserved (0 * 80bits)*/
    uint32 max_ext_tcam_data_base;
    uint32 max_ext_tcam_mask_base;

    uint32 tcam_block_entry_num[MAX_LOCAL_CHIP_NUM][MAX_NOR_TCAM_NUM];
    uint32 lpm_tcam_block_entry_num[MAX_LOCAL_CHIP_NUM][MAX_LPM_TCAM_NUM];
};
typedef struct cm_tcam_info_s cm_tcam_info_t;

/**the following give a min abstract tcam api**/
extern int32 tcam_model_initialize(uint8 chip_id_offset);

extern int32 tcam_model_release(uint8 chip_id_offset);

extern int32 tcam_model_write(uint8 chip_id_offset, uintptr sw_model_addr, uint32 *data, int32 len);

extern int32 tcam_model_read(uint8 chip_id_offset, uintptr sw_model_addr, uint32 *data, int32 len);

extern int32 tcam_model_lookup(uint8 chip_id_offset, uint32 tbl_id, uint32* data, uint32* idx);

extern int32 tcam_model_remove(uint8 chip_id_offset, uintptr sw_model_addr, uint32 entry_word_len);

extern void tcam_model_get_tcam_key_list(uint32 *key_list_ptr);

extern void tcam_model_get_tcam_key_num(uint32 *tcam_key_num);

extern int32
tcam_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index,
                                    uintptr*sw_model_addr, bool is_data);

extern int32
tcam_model_get_blk_sw_addr_start_index(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index,
                                        uintptr *sw_model_addr,uint32 *start_index, bool is_data);

extern cm_tcam_info_t tcam_info;

#endif
