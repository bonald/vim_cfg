
#ifndef __TBL_OPM_GLOBAL_DEFINE_H__
#define __TBL_OPM_GLOBAL_DEFINE_H__

/* TBL_OPM_GLOBAL field defines */
typedef enum
{
    TBL_OPM_GLOBAL_FLD_ERPS_MODE            = 0 ,  /* RW */
    TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD        = 1 ,  /* RW */
    TBL_OPM_GLOBAL_FLD_MAX                  = 2 
} tbl_opm_global_field_id_t;

/* TBL_OPM_GLOBAL defines */
typedef struct
{
    uint32               erps;                /* bitmap of OPMGLB_FLAG_ERPS_ */
} tbl_opm_global_t;

#endif /* !__TBL_OPM_GLOBAL_DEFINE_H__ */

