
#ifndef _DRV_SER_H
#define _DRV_SER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"

enum drv_ser_action_e
{
    DRV_SER_ACTION_NULL       = 0,
    DRV_SER_ACTION_LOG,
    DRV_SER_ACTION_RESET_CHIP,
    DRV_SER_ACTION_RESET_PORT
};
typedef enum drv_ser_action_e drv_ser_action_t;

enum drv_ser_type_e
{
   DRV_SER_TYPE_SBE,
   DRV_SER_TYPE_MBE,
   DRV_SER_TYPE_PARITY_ERROR,
   DRV_SER_TYPE_TCAM_ERROR,
   DRV_SER_TYPE_OTHER,
   DRV_SER_TYPE_NUM
};
typedef enum drv_ser_type_e drv_ser_type_t;

typedef int32 (* drv_ser_event_fn)(uint8 gchip, void* p_data);

struct drv_ser_cb_info_s
{
   uint8  type;  /*drv_ser_type_t*/
   uint32 tbl_id;
   uint32 tbl_index;
   uint8  action;   /*drv_ser_action_t*/
   uint8  recover;
};
typedef struct drv_ser_cb_info_s drv_ser_cb_info_t;

enum drv_ser_cfg_type_e
{
    DRV_SER_CFG_TYPE_SCAN_MODE,
    DRV_SER_CFG_TYPE_TCAM_SCAN_INFO,
    DRV_SER_CFG_TYPE_ECC_INTERRUPT_INFO,
    DRV_SER_CFG_TYPE_HW_RESER_EN,
    DRV_SER_CFG_TYPE_SHOW_ECC_STAUTS,
    DRV_SER_CFG_TYPE_ECC_EVENT_CB,
    DRV_SER_CFG_TYPE_DMA_RECOVER_TCAM
};
typedef enum drv_ser_cfg_type_e drv_ser_cfg_type_t;

enum drv_ser_hw_reset_cb_tyep_e
{
    DRV_SER_HW_RESET_CB_TYPE_DATAPATH,
    DRV_SER_HW_RESET_CB_TYPE_INTERRUPT,
    DRV_SER_HW_RESET_CB_TYPE_DMA,
    DRV_SER_HW_RESET_CB_TYPE_OAM,
    DRV_SER_HW_RESET_CB_TYPE_FDB,
    DRV_SER_HW_RESET_CB_TYPE_MAC,
    DRV_SER_HW_RESET_CB_TYPE_NUM
};
typedef enum drv_ser_hw_reset_cb_tyep_e drv_ser_hw_reset_cb_tyep_t;

struct drv_ser_global_cfg_s
{
    uint32  tcam_scan_en     :1;        /**< enable tcam scan */
    uint32  sbe_scan_en      :1;        /**< enable single bit error scan */
    uint32  ecc_recover_en   :1;        /**< enable ecc/parity error recover */
    uint8   hw_reset_en:1;               /*overall control */
    uint32  rsv              :29;
    uint32  tcam_scan_burst_entry_num;  /**< tcam key scan entries num per burst */
    uint32  tcam_scan_interval;         /**< tcam key scan all time interval, unit is ms */
    uint32  sbe_scan_interval;          /**< single bit error scan all time interval, unit is ms */
    uint64  cpu_mask;

    void *    dma_rw_cb;
    void *    create_cb;
    void *    insert_cb;
    void *    free_cb;
    void *    lookup_cb;
    void *    caculate_cb;
    void *    travese_cb;

};
typedef struct drv_ser_global_cfg_s drv_ser_global_cfg_t;

enum drv_ser_scan_type_e
{
    DRV_SER_SCAN_TYPE_TCAM,
    DRV_SER_SCAN_TYPE_SBE,
    DRV_SER_SCAN_TYPE_NUM
};
typedef enum drv_ser_scan_type_e drv_ser_scan_type_t;

/* tcam/sbe scan param*/
struct drv_ser_scan_info_s
{
    drv_ser_scan_type_t type;
    uint8     mode;
    uint32   scan_interval;
    uint32   burst_entry_num;
    uint32   burst_interval;
};
typedef struct drv_ser_scan_info_s drv_ser_scan_info_t;

struct drv_ecc_intr_tbl_s
{
    uint32              intr_tblid;
    uint8               intr_bit;
    uint8               intr_fld;
    uint8               action;
    uint8               err_type;
    uint32              rec_id;
    uint32              rec_id_fld;
    uint32              mem_id;
    uint8               tbl_type;
};
typedef struct drv_ecc_intr_tbl_s drv_ecc_intr_tbl_t;

struct drv_ecc_intr_param_s
{
    uint32     intr_tblid;       /**< interrupt tbl id */
    uint32*    p_bmp;            /**< bitmap of interrupt status */
    uint16     total_bit_num;    /**< total bit num */
    uint16     valid_bit_count;  /**< valid bit count */
};
typedef struct drv_ecc_intr_param_s drv_ecc_intr_param_t;

typedef int32 (* drv_ser_hw_reset_cb)(uint8 lchip, void* p_data);

struct drv_ser_dma_tcam_param_s
{
    uint16 mem_id;
    uint16 sub_mem_id;
    uint32 entry_size;
    uint32 time_stamp[2];
    uint32* p_memory;
};
typedef struct drv_ser_dma_tcam_param_s drv_ser_dma_tcam_param_t;
extern int32
drv_ser_init(uint8 lchip ,drv_ser_global_cfg_t* p_cfg);
extern int32
drv_ser_register_hw_reset_cb(uint8 lchip ,uint32 cb_type,drv_ser_hw_reset_cb cb);
extern int32
drv_ser_restore(uint8 lchip, drv_ecc_intr_param_t* p_intr_param);
extern int32
drv_ser_set_cfg(uint8 lchip, uint32 type, void* p_cfg);
extern int32
drv_ser_get_cfg(uint8 lchip, uint32 type, void* p_cfg);

#ifdef __cplusplus
}
#endif

#endif


