
#ifndef __TBL_LOG_GLOBAL_DEFINE_H__
#define __TBL_LOG_GLOBAL_DEFINE_H__

/* TBL_LOG_GLOBAL field defines */
typedef enum
{
    TBL_LOG_GLOBAL_FLD_LOG_TO_CDB           = 0 ,  /* READ */
    TBL_LOG_GLOBAL_FLD_MAX                  = 1 
} tbl_log_global_field_id_t;

/* TBL_LOG_GLOBAL defines */
typedef struct
{
    logid_bmp_t          log_to_cdb;
} tbl_log_global_t;

#endif /* !__TBL_LOG_GLOBAL_DEFINE_H__ */

