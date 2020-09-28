
#ifndef _DRV_SER_DB_H_
#define _DRV_SER_DB_H_

#ifdef __cplusplus
extern "C" {
#endif


#define DRV_GET_TBL_INFO_ECC(tbl_id)       TABLE_INFO(lchip, tbl_id).ecc
#define DRV_GET_TBL_INFO_DB(tbl_id)        TABLE_INFO(lchip, tbl_id).store
#define DRV_GET_TBL_INFO_RWRITE(tbl_id)    0


#define DRV_MEM_DB_TBL_ID_SIZE 2
#define DRV_MEM_DB_LPM_TCAM_BLOCK_NUM 4
#define DRV_MEM_DB_LPM_AD_BLOCK_NUM 2
#define DRV_MEM_DB_LPM_KEY_BLOCK_NUM 4
#define DRV_MEM_DB_MPLS_HS_KEY_ECC_SIZE  64
#define DRV_MEM_DB_SHARE_BUFFER_ECC_SIZE 48

enum drv_ser_db_recover_mode_e
{
    DRV_MEM_DB_RECOVER_STATIC_TBL,
    DRV_MEM_DB_RECOVER_DYNAMIC_TBL,
    DRV_MEM_DB_RECOVER_TCAM_KEY,
    DRV_MEM_DB_RECOVER_ALL,
    DRV_MEM_DB_RECOVER_MAX_NUM
};
typedef enum drv_ser_db_recover_mode_e drv_ser_db_recover_mode_t;


struct drv_ser_db_dma_rw_s
{
    uint32  tbl_addr;
    uint32* buffer;

    uint16  entry_num;
    uint16  entry_len;

    uint8   rflag;
    uint8   is_pause;
    uint8   user_dma_mode; /*use the user alloced dma memory*/
    uint8   rsv;
};
typedef struct drv_ser_db_dma_rw_s drv_ser_db_dma_rw_t;


struct drv_ser_db_err_mem_info_s
{
   uint8  err_mem_type;
   uint8  err_mem_order;
   uint32 err_mem_offset;
   uint32 err_entry_size;
};
typedef struct drv_ser_db_err_mem_info_s drv_ser_db_err_mem_info_t;

 /*#include "drv_api.h"*/
enum drv_ser_db_mask_tcam_fld_type_e
{
    DRV_MEM_DB_MASK_TCAM_BY_STORE,
    DRV_MEM_DB_MASK_TCAM_BY_READ,
    DRV_MEM_DB_MASK_TCAM_BY_INIT,
    DRV_MEM_DB_MASK_TCAM_MAX_NUM
};
typedef enum drv_ser_db_mask_tcam_fld_type_e drv_ser_db_mask_tcam_fld_type_t;

struct drv_ser_db_mask_tcam_fld_s
{
   uint8  type; /*drv_ser_db_mask_tcam_fld_type_t*/
   uint8  ram_id;
   uint8  entry_valid;
   uint32* p_data;
   uint32* p_mask;
};
typedef struct drv_ser_db_mask_tcam_fld_s drv_ser_db_mask_tcam_fld_t;

typedef uint32 (* mem_db_key_fn) (void* p_data);
typedef bool (* mem_db_cmp_fn) (void* p_data1, void* p_data2);
typedef int32 (* mem_db_traversal_fn)(void* bucket_data, void* user_data);
typedef void* (*hash_create_cb)(uint16 num, uint16 size, mem_db_key_fn key_cb, mem_db_cmp_fn cmp_cb);
typedef void* (*hash_insert_cb)(void* p_data0, void* p_data1);
typedef void (*hash_free_cb)(void* p_data0);
typedef void* (*hash_lookup_cb)(void* p_data0, void* p_data1);
typedef int32 (*hash_traverse_cb)(void* p_data1, mem_db_traversal_fn traverse_fn, void* p_data2);
typedef int32 (*drv_ser_db_dma_rw_cb)(uint8 lchip, uint32 tbl_addr, uint32* buffer, uint16 entry_num, uint16 entry_len, uint8 user_dma_mode);


struct drv_ser_db_s
{
    uint8*      dynamic_tbl[128];
    uint32      dma_scan_timestamp[128][2];/*for dma tcam scan*/
    uint32*     p_reset_hw_alloc_mem[2];/*0:malloc normal cpu memory; 1:malloc dma memory*/
    void* p_static_tbl_hash;

    uint32      catch_mem_static_size;
    uint32      catch_mem_dynamic_size;
    uint32      catch_mem_tcam_key_size;

    uint32      drv_ser_db_en          :1;          /*overall control */
    uint32      static_tbl_backup_en   :1;          /**< enable static table backup */
    uint32      dynamic_tbl_backup_en  :1;          /**< enable dynamic table backup */
    uint32      tcam_key_backup_en     :1;          /**< enable tcam key backup */
    uint32      stop_store_en          :1;          /*if set,will stop ecc backup store .it is used for test*/
    uint32      db_reset_hw_doing      :1;
    uint32      rsv                    :26;
    drv_ser_hw_reset_cb  reset_hw_cb[DRV_SER_HW_RESET_CB_TYPE_NUM];
    drv_ser_db_dma_rw_cb dma_rw_cb;

    hash_create_cb   create_cb;
    hash_insert_cb   insert_cb;
    hash_free_cb     free_cb;
    hash_lookup_cb   lookup_cb;
    hash_traverse_cb travese_cb;

};
typedef struct drv_ser_db_s drv_ser_db_t;



extern int32
drv_ser_db_init(uint8 lchip ,drv_ser_global_cfg_t* p_cfg);
extern int32
drv_ser_db_deinit(uint8 lchip);
extern int32
drv_ser_db_set_hw_reset_en(uint8 lchip, uint8 enable);

extern int32
drv_ser_db_rewrite_tcam(uint8 lchip, uint8 mem_id, uint8 mem_type, uint8 mem_order, uint32 entry_idx, void* p_tbl_entry);

extern int32
drv_ser_db_store(uint8 lchip, uint32 mem_id, uint32 entry_idx,  void* p_tbl_entry);

extern int32
drv_ser_db_map_lpm_tcam(uint8 lchip, uint8* p_mem_id, uint8* p_mem_order, uint16* p_entry_idx);

extern int32
drv_ser_db_unmap_lpm_tcam(uint8 lchip, uint8* p_mem_id, uint8* p_mem_order, uint16* p_entry_idx);

extern int32
drv_ser_db_get_tcam_local_entry_size(uint8 lchip, uint8 mem_id, uint32*p_entry_size);

extern int32
drv_ser_db_rewrite_dyn(uint8 lchip, uint8 err_mem_id, uint32 err_mem_offset, uint32 err_entry_size, uint8* p_data, uint8 mode, uint32 dma_per_size);
extern int32
drv_ser_db_set_tcam_mask_fld(uint8 lchip, drv_ser_db_mask_tcam_fld_t * p_void);
extern uint8
drv_ser_db_get_tcam_entry_size_aline(uint8 lchip, uint8 ram_id);
extern int32
drv_ser_deinit(uint8 lchip );
extern int32
drv_ser_check_correct_tcam(uint8 lchip, uint32 mem_id, uint32 tbl_idx, void* p_void, uint8 rd_empty);

#ifdef __cplusplus
}
#endif

#endif
