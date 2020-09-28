
#ifndef __TBL_FEA_TIME_DEFINE_H__
#define __TBL_FEA_TIME_DEFINE_H__

/* TBL_FEA_TIME field defines */
typedef enum
{
    TBL_FEA_TIME_FLD_SYNC_TYPE            = 0 ,  /* RW */
    TBL_FEA_TIME_FLD_SYNC_COUNT           = 1 ,  /* READ */
    TBL_FEA_TIME_FLD_LAST_SYNC_TIME       = 2 ,  /* READ */
    TBL_FEA_TIME_FLD_MAX                  = 3 
} tbl_fea_time_field_id_t;

/* TBL_FEA_TIME defines */
typedef struct
{
    uint32               sync_type;           /* Timestamp sync source, glb_time_sync_type_t */
    uint32               sync_count;          /* Timestamp sync count */
    sal_time_t           last_sync_time;      /* Last sync time */
} tbl_fea_time_t;

#endif /* !__TBL_FEA_TIME_DEFINE_H__ */

