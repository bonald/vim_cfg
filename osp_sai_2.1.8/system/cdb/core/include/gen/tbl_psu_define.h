
#ifndef __TBL_PSU_DEFINE_H__
#define __TBL_PSU_DEFINE_H__

/* TBL_PSU field defines */
typedef enum
{
    TBL_PSU_FLD_KEY                  = 0 ,  /* READ */
    TBL_PSU_FLD_SLOT                 = 1 ,  /* READ */
    TBL_PSU_FLD_NAME                 = 2 ,  /* READ */
    TBL_PSU_FLD_ABSENT               = 3 ,  /* RW */
    TBL_PSU_FLD_RUN_STATUS           = 4 ,  /* RW */
    TBL_PSU_FLD_ALERT_STATUS         = 5 ,  /* RW */
    TBL_PSU_FLD_MODE                 = 6 ,  /* RW */
    TBL_PSU_FLD_FIXED_STATUS         = 7 ,  /* RW */
    TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE = 8 ,  /* RW */
    TBL_PSU_FLD_SUPPORT_PMBUS        = 9 ,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_BYTE    = 10,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_WORD    = 11,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_VOUT    = 12,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_IOUT    = 13,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_INPUT   = 14,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE = 15,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_CML     = 16,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_OTHER   = 17,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC = 18,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2 = 19,  /* RW */
    TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4 = 20,  /* RW */
    TBL_PSU_FLD_MAX                  = 21
} tbl_psu_field_id_t;

/* TBL_PSU defines */
typedef struct
{
    int32                id;
} tbl_psu_key_t;

typedef struct
{
    tbl_psu_key_t        key;
    int32                slot;
    char                 name[GLB_NAME_INFO_STR_MAX];
    int32                absent;
    char                 run_status[GLB_NAME_INFO_STR_MAX];
    char                 alert_status[GLB_NAME_INFO_STR_MAX];
    char                 mode[GLB_NAME_INFO_STR_MAX];
    uint8                fixed_status;
    uint8                psu_health_status_change;
    uint8                support_pmbus;
    uint8                pmbus_status_byte;
    uint16               pmbus_status_word;
    uint8                pmbus_status_vout;
    uint8                pmbus_status_iout;
    uint8                pmbus_status_input;
    uint8                pmbus_status_temperature;
    uint8                pmbus_status_cml;
    uint8                pmbus_status_other;
    uint8                pmbus_status_mfr_specific;
    uint8                pmbus_status_fans_1_2;
    uint8                pmbus_status_fans_3_4;
} tbl_psu_t;

typedef struct
{
    ctclib_hash_t        *psu_hash;
    ctclib_slist_t       *psu_list;
} tbl_psu_master_t;

#endif /* !__TBL_PSU_DEFINE_H__ */

