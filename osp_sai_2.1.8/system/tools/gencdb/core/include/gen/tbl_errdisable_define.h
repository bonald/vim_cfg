
#ifndef __TBL_ERRDISABLE_DEFINE_H__
#define __TBL_ERRDISABLE_DEFINE_H__

/* TBL_ERRDISABLE field defines */
typedef enum
{
    TBL_ERRDISABLE_FLD_KEY                  = 0 ,  /* READ */
    TBL_ERRDISABLE_FLD_ERRDISABLE_EN        = 1 ,  /* RW */
    TBL_ERRDISABLE_FLD_RECOVERY_EN          = 2 ,  /* RW */
    TBL_ERRDISABLE_FLD_MAX                  = 3 
} tbl_errdisable_field_id_t;

/* TBL_ERRDISABLE defines */
typedef struct
{
    errdis_rsn_t         reason;
} tbl_errdisable_key_t;

typedef struct
{
    tbl_errdisable_key_t key;
    uint8                errdisable_en;
    uint8                recovery_en;
} tbl_errdisable_t;

typedef struct
{
    tbl_errdisable_t     *errdisable_array[ERRDIS_RSN_MAX];
} tbl_errdisable_master_t;

#endif /* !__TBL_ERRDISABLE_DEFINE_H__ */

