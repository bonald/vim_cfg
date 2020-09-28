
#ifndef __TBL_CHSM_DEBUG_DEFINE_H__
#define __TBL_CHSM_DEBUG_DEFINE_H__

/* TBL_CHSM_DEBUG field defines */
typedef enum
{
    TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS         = 0 ,  /* RW */
    TBL_CHSM_DEBUG_FLD_MAX                  = 1 
} tbl_chsm_debug_field_id_t;

/* TBL_CHSM_DEBUG defines */
typedef struct
{
    uint32               chsm;                /* bitmap of CHSMDBG_FLAG_CHSM_ */
} tbl_chsm_debug_t;

#endif /* !__TBL_CHSM_DEBUG_DEFINE_H__ */

