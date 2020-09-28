#ifndef _SRAM_MODEL_H_
#define _SRAM_MODEL_H_

#include "greatbelt/include/drv_error.h"
#include "greatbelt/include/drv_enum.h"
#include "greatbelt/include/drv_chip_info.h"
#include "greatbelt/include/drv_common.h"
#include "sal.h"

#define SRAM_MODEL_PTR_VALID_CHECK(ptr) \
    if (NULL == (ptr))                \
    {                                   \
        return DRV_E_INVALID_PTR;       \
    }

#define SRAM_CHIP_ID_VALID_CHECK(chip_id_offset) \
    if ((chip_id_offset) >= (MAX_LOCAL_CHIP_NUM))           \
    {                                         \
        DRV_DBG_INFO("Sram Out of chip_id number\n"); \
        return DRV_E_INVALID_CHIP;            \
    }

/* Record each register sw base address in cmodel stucture */
struct cm_sram_info_s
{
    uint8* sram_model_wbit[MaxTblId_t]; /* not used for tcam ad and dynamic tables */
    uint8 sram_model_initialized_flag;

    uintptr dynamic_mem_base; /* sw mem for all 9 dynamic tables, DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM*12 bytes */
    uintptr tcam_ad_mem_base; /* sw mem for tcam ad, DRV_TCAM_AD_MAX_ENTRY_NUM*12 bytes */
    uintptr lpm_tcam_ad_mem_base; /* sw mem for lpm tcam ad, DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM*12 bytes */
    uint8* dynamic_mem_wbit; /* DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM bytes */
    uint8* tcam_ad_mem_wbit; /* DRV_TCAM_AD_MAX_ENTRY_NUM bytes */
    uint8* lpm_tcam_ad_mem_wbit; /* DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM bytes */

    //uint8 mem_profile_index;
};
typedef struct cm_sram_info_s cm_sram_info_t;

enum sram_tbl_type_e
{
    SRAM_TBL_TYPE_NORMAL = 0,
    SRAM_TBL_TYPE_DYNAMAL,
    SRAM_TBL_TYPE_TCAM_AD,

    SRAM_TBL_TYPE_INVALID,
};
typedef enum sram_tbl_type_e sram_tbl_type_t;

enum sram_dynamic_mem_access_mode_u
{
    SRAM_DYNAMIC_DEFAULT = 0,
    SRAM_DYNAMIC_4W_MODE,
    SRAM_DYNAMIC_8W_MODE,
    SRAM_DYNAMIC_INVALID,
};
typedef enum sram_dynamic_mem_access_mode_u sram_dynamic_mem_access_mode_t;

/* record sw info similar to hw info */
struct sram_dynamic_info_s
{
    uint16 sram_dynamic_bitmap;
    uintptr sram_dynamic_sw_data_base[MAX_DRV_BLOCK_NUM];
    uint32 sram_dynamic_entry_num[MAX_DRV_BLOCK_NUM];
    uint32 sram_dynamic_entry_start_index[MAX_DRV_BLOCK_NUM];
    uint32 sram_dynamic_entry_end_index[MAX_DRV_BLOCK_NUM];
    uintptr sram_dynamic_wbit_sw_data_base[MAX_DRV_BLOCK_NUM];
    sram_dynamic_mem_access_mode_t sram_access_mode;
};
typedef struct sram_dynamic_info_s sram_dynamic_info_t;

/* Record each table sw base address in cmodel stucture */
/* when the table is tcam key, we can get the key database and maskbase
   according to the whole tcam base and the key hw_offset,
   so to tcam key table we do not need the sw_data_base */
struct cmodel_mem_info_s
{
    sram_tbl_type_t sram_tbl_type;
    uintptr  sw_data_base;

    sram_dynamic_info_t* sram_dynamic_info_ptr;
};
typedef struct cmodel_mem_info_s cmodel_tbl_info_t;

extern int32 sram_model_initialize(uint8 chip_id_offset);
extern int32 sram_model_release(uint8 chip_id_offset);
extern int32 sram_model_reset(uint8 chip_id_offset);
extern int32 sram_model_read(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len);
extern int32 sram_model_write(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len);
extern int8 sram_model_is_tcam_ad_table(uint8 chip_id_offset, tbls_id_t tbl_id);
extern int32 sram_model_get_sw_address_by_tbl_id_and_index(uint8 chip_id_offset, tbls_id_t tbl_id,
                                                           uint32 index, uintptr* start_data_addr);

extern int32 sram_model_allocate_sram_tbl_info(void);
/* get SRAM model Address according to chipid+tablid+index */
extern int32
sram_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index, uint32* sw_model_addr);

extern cm_sram_info_t sram_info[MAX_LOCAL_CHIP_NUM];
extern cmodel_tbl_info_t cmodel_tbls_info[MAX_LOCAL_CHIP_NUM][MaxTblId_t];

#define SRAM_TBL_INFO(chip_id_offset, tbl_id) (cmodel_tbls_info[chip_id_offset][tbl_id])
#define SRAM_SW_DATA_BASE(chip_id_offset, tbl_id) (cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
#define SRAM_TBL_TYPE(chip_id_offset, tbl_id) (cmodel_tbls_info[chip_id_offset][tbl_id].sram_tbl_type)

#define SRAM_DYNAMIC_INFO_T (sram_dynamic_info_t*)

#define SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id) ((SRAM_TBL_INFO(chip_id_offset, tbl_id).sram_dynamic_info_ptr))
#define SRAM_BITMAP(chip_id_offset, tbl_id)  (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_bitmap)
#define SRAM_DATA_BASE(chip_id_offset, tbl_id, blk_id) (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_sw_data_base[blk_id])
#define SRAM_ENTRY_NUM(chip_id_offset, tbl_id, blk_id) (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_entry_num[blk_id])
#define SRAM_START_INDEX(chip_id_offset, tbl_id, blk_id) (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_entry_start_index[blk_id])
#define SRAM_END_INDEX(chip_id_offset, tbl_id, blk_id) (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_entry_end_index[blk_id])
#define SRAM_WBIT_DATA_BASE(chip_id_offset, tbl_id, blk_id)(SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_dynamic_wbit_sw_data_base[blk_id])
#define SRAM_ACCESS_MODE(chip_id_offset, tbl_id) (SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id)->sram_access_mode)

#define SRAM_INFO_MODEL_TBL_WBIT_PTR(chip_id_offset, tbl_id) (sram_info[chip_id_offset].sram_model_wbit[tbl_id])
#define SRAM_INFO_MODEL_INIT_FLAG(chip_id_offset) (sram_info[chip_id_offset].sram_model_initialized_flag)
#define SRAM_INFO_DYNAMIC_MEM_BASE(chip_id_offset) (sram_info[chip_id_offset].dynamic_mem_base)
#define SRAM_INFO_DYNAMIC_MEM_WBIT_BASE_PTR(chip_id_offset) (sram_info[chip_id_offset].dynamic_mem_wbit)
#define SRAM_INFO_TCAM_AD_MEM_BASE(chip_id_offset) (sram_info[chip_id_offset].tcam_ad_mem_base)
#define SRAM_INFO_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset) (sram_info[chip_id_offset].tcam_ad_mem_wbit)
#define SRAM_INFO_LPM_TCAM_AD_MEM_BASE(chip_id_offset) (sram_info[chip_id_offset].lpm_tcam_ad_mem_base)
#define SRAM_INFO_LPM_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset) (sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit)

#endif

