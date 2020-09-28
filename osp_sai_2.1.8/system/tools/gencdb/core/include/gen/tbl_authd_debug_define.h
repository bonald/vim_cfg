
#ifndef __TBL_AUTHD_DEBUG_DEFINE_H__
#define __TBL_AUTHD_DEBUG_DEFINE_H__

/* TBL_AUTHD_DEBUG field defines */
typedef enum
{
    TBL_AUTHD_DEBUG_FLD_AUTH_EVENT           = 0 ,  /* RW */
    TBL_AUTHD_DEBUG_FLD_AUTH_PACKET          = 1 ,  /* RW */
    TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL        = 2 ,  /* RW */
    TBL_AUTHD_DEBUG_FLD_AUTH_TIMER           = 3 ,  /* RW */
    TBL_AUTHD_DEBUG_FLD_MAX                  = 4 
} tbl_authd_debug_field_id_t;

/* TBL_AUTHD_DEBUG defines */
typedef struct
{
    uint32               flags;               /* bitmap of AUTHD_DBG_FLAG_ */
} tbl_authd_debug_t;

#endif /* !__TBL_AUTHD_DEBUG_DEFINE_H__ */

