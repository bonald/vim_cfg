
#ifndef __TBL_PTP_RESIDENCE_TIME_CACHE_DEFINE_H__
#define __TBL_PTP_RESIDENCE_TIME_CACHE_DEFINE_H__

/* TBL_PTP_RESIDENCE_TIME_CACHE field defines */
typedef enum
{
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_KEY                  = 0 ,  /* READ */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID          = 1 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS = 2 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN               = 3 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER   = 4 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME       = 5 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED             = 6 ,  /* RW */
    TBL_PTP_RESIDENCE_TIME_CACHE_FLD_MAX                  = 7 
} tbl_ptp_residence_time_cache_field_id_t;

/* TBL_PTP_RESIDENCE_TIME_CACHE defines */
typedef ptp_residence_time_cache_t tbl_ptp_residence_time_cache_key_t;

typedef struct
{
    tbl_ptp_residence_time_cache_key_t key;
    uint16               sequence_id;         /*sequence id*/
    uint32               ifindex_port_ingress; /*ingress port ifindex*/
    uint32               domain;              /*domain id*/
    ctc_task_t           *ptm_age_task_timer; /*cache agging*/
    glb_ptp_timeinterval_t residence_time;      /*packet residence time*/
    uint32               outdated;
} tbl_ptp_residence_time_cache_t;

typedef struct
{
    ctclib_hash_t        *ptp_residence_time_cache_hash;
    ctclib_slist_t       *ptp_residence_time_cache_list;
} tbl_ptp_residence_time_cache_master_t;

#endif /* !__TBL_PTP_RESIDENCE_TIME_CACHE_DEFINE_H__ */

