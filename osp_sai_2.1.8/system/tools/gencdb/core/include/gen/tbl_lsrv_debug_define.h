
#ifndef __TBL_LSRV_DEBUG_DEFINE_H__
#define __TBL_LSRV_DEBUG_DEFINE_H__

/* TBL_LSRV_DEBUG field defines */
typedef enum
{
    TBL_LSRV_DEBUG_FLD_CARD                 = 0 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_PORT                 = 1 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_FIBER                = 2 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_FAN                  = 3 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_PSU                  = 4 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_LED                  = 5 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_THERMAL              = 6 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_VCM                  = 7 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_POE                  = 8 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_SYNCE                = 9 ,  /* RW */
    TBL_LSRV_DEBUG_FLD_MAX                  = 10
} tbl_lsrv_debug_field_id_t;

/* TBL_LSRV_DEBUG defines */
typedef struct
{
    uint32               card;                /* bitmap of LSRVDBG_FLAG_CARD_ */
    uint32               synce;               /* bitmap of LSRVDBG_FLAG_SYNCE_ */
} tbl_lsrv_debug_t;

#endif /* !__TBL_LSRV_DEBUG_DEFINE_H__ */

