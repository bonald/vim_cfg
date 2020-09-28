#ifndef _CTC_DKIT_GOLDENGATE_HASH_DB_H
#define _CTC_DKIT_GOLDENGATE_HASH_DB_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"

#define CTC_DKITS_TBL_INVALID_FLDID                   0xFFFFFFFF
#define CTC_DKITS_HASH_DB_MODULE_MAX_HASH_TYPE        53

#define CTC_DKITS_HASH_DB_KEY_NUM(HASH_MOD)                 ctc_dkits_hash_db[HASH_MOD].key_num
#define CTC_DKITS_HASH_DB_LEVE_NUM(HASH_MOD)                ctc_dkits_hash_db[HASH_MOD].level_num
#define CTC_DKITS_HASH_DB_DEPTH(HASH_MOD, LEVEL_IDX)        ctc_dkits_hash_db[HASH_MOD].p_level[LEVEL_IDX].depth
#define CTC_DKITS_HASH_DB_BUCKET_NUM(HASH_MOD, LEVEL_IDX)   ctc_dkits_hash_db[HASH_MOD].p_level[LEVEL_IDX].bucket_num
#define CTC_DKITS_HASH_DB_HASH_TYPE(HASH_MOD, KEY_TYPE)     ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].hash_type
#define CTC_DKITS_HASH_DB_KEY_TBLID(HASH_MOD, KEY_TYPE)     ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].key_tblid
#define CTC_DKITS_HASH_DB_AD_TBLID(HASH_MOD, KEY_TYPE)      ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].ad_tblid
#define CTC_DKITS_HASH_DB_ADIDX_FLDID(HASH_MOD, KEY_TYPE)   ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].ad_idx_fldid
#define CTC_DKITS_HASH_DB_CAM_TBLID(HASH_MOD)               ctc_dkits_hash_db[HASH_MOD].cam_tblid
#define CTC_DKITS_HASH_DB_KEY_UNIT(HASH_MOD, KEY_TYPE)      ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].bits_per_unit
#define CTC_DKITS_HASH_DB_KEY_ALIGN(HASH_MOD, KEY_TYPE)     ctc_dkits_hash_db[HASH_MOD].p_hash_db_info[KEY_TYPE].align_unit
#define CTC_DKITS_HASH_DB_BYTE2WORD(BYTE)                   (((BYTE) + 3) / 4)
#define CTC_DKITS_HASH_DB_BIT2BYTE(BIT)                     (((BIT) + 7) / 8)

enum ctc_dkits_hash_db_module_e
{
    CTC_DKITS_HASH_DB_MODULE_EGRESS_XC,
    CTC_DKITS_HASH_DB_MODULE_FIB_HOST0,
    CTC_DKITS_HASH_DB_MODULE_FIB_HOST1,
    CTC_DKITS_HASH_DB_MODULE_FLOW,
    CTC_DKITS_HASH_DB_MODULE_IPFIX,
    CTC_DKITS_HASH_DB_MODULE_USERID,
    CTC_DKITS_HASH_DB_MODULE_NUM,
    CTC_DKITS_HASH_DB_MODULE_INVALID = CTC_DKITS_HASH_DB_MODULE_NUM
};
typedef enum ctc_dkits_hash_db_module_e ctc_dkits_hash_db_module_t;

struct ctc_dkits_hash_level_s
{
    uint32 depth;
    uint32 bucket_num;
};
typedef struct ctc_dkits_hash_level_s ctc_dkits_hash_level_t;

struct ctc_dkits_hash_db_info_s
{
    uint8     hash_type;
    tbls_id_t key_tblid;
    tbls_id_t ad_tblid;
    fld_id_t  ad_idx_fldid;
    uint32    bits_per_unit;
    uint32    align_unit;
};
typedef struct ctc_dkits_hash_db_info_s ctc_dkits_hash_db_info_t;

struct ctc_dkits_hash_db_s
{
    uint8 key_num;
    uint8 level_num;
    ctc_dkits_hash_db_info_t* p_hash_db_info;
    ctc_dkits_hash_level_t* p_level;
    tbls_id_t cam_tblid;
};
typedef struct ctc_dkits_hash_db_s  ctc_dkits_hash_db_t;

extern ctc_dkits_hash_db_t ctc_dkits_hash_db[];

extern int32
ctc_usw_dkits_hash_db_level_en(uint8, ctc_dkits_hash_db_module_t, uint32*);

extern int32
ctc_usw_dkits_hash_db_get_couple_mode(uint8, ctc_dkits_hash_db_module_t, uint32*);

extern int32
ctc_usw_dkits_hash_db_key_index_base(uint8, ctc_dkits_hash_db_module_t, uint32, uint32*);

extern int32
ctc_usw_dkits_hash_db_entry_valid(uint8, ctc_dkits_hash_db_module_t, uint8, uint32*, uint32*);

#ifdef __cplusplus
}
#endif

#endif

