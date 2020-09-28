
#ifndef __TBL_LOGIN_SECURITY_GLB_DEFINE_H__
#define __TBL_LOGIN_SECURITY_GLB_DEFINE_H__

/* TBL_LOGIN_SECURITY_GLB field defines */
typedef enum
{
    TBL_LOGIN_SECURITY_GLB_FLD_ENABLE               = 0 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM         = 1 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD          = 2 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION        = 3 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER    = 4 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER     = 5 ,  /* RW */
    TBL_LOGIN_SECURITY_GLB_FLD_MAX                  = 6 
} tbl_login_security_glb_field_id_t;

/* TBL_LOGIN_SECURITY_GLB defines */
typedef struct
{
    uint32               enable;              /* login security enable */
    uint32               max_fail_num;        /* maxinum fail number */
    uint32               fail_period;         /* lock duration*/
    uint32               lock_duration;       /* record time */
    uint32               curr_invalid_user;   /* current invalid user count */
    uint32               max_invalid_user;    /* maximum invalid user number */
} tbl_login_security_glb_t;

#endif /* !__TBL_LOGIN_SECURITY_GLB_DEFINE_H__ */

