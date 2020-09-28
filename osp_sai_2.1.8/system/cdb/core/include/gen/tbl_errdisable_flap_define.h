
#ifndef __TBL_ERRDISABLE_FLAP_DEFINE_H__
#define __TBL_ERRDISABLE_FLAP_DEFINE_H__

/* TBL_ERRDISABLE_FLAP field defines */
typedef enum
{
    TBL_ERRDISABLE_FLAP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN       = 1 ,  /* RW */
    TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD   = 2 ,  /* RW */
    TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD  = 3 ,  /* RW */
    TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT        = 4 ,  /* RW */
    TBL_ERRDISABLE_FLAP_FLD_MAX                  = 5 
} tbl_errdisable_flap_field_id_t;

/* TBL_ERRDISABLE_FLAP defines */
typedef struct
{
    uint32               flap_rsn;
} tbl_errdisable_flap_key_t;

typedef struct
{
    tbl_errdisable_flap_key_t key;
    uint32               errdisable_rsn;
    uint32               flap_cnt_threshold;
    uint32               flap_time_threshold;
    uint32               flap_time_cnt;
} tbl_errdisable_flap_t;

typedef struct
{
    tbl_errdisable_flap_t *errdisable_flap_array[ERRDIS_FLAP_REASON_MAX];
} tbl_errdisable_flap_master_t;

#endif /* !__TBL_ERRDISABLE_FLAP_DEFINE_H__ */

