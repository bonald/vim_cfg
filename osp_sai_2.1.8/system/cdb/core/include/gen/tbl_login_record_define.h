
#ifndef __TBL_LOGIN_RECORD_DEFINE_H__
#define __TBL_LOGIN_RECORD_DEFINE_H__

/* TBL_LOGIN_RECORD field defines */
typedef enum
{
    TBL_LOGIN_RECORD_FLD_KEY                  = 0 ,  /* READ */
    TBL_LOGIN_RECORD_FLD_LOCKED               = 1 ,  /* RW */
    TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS        = 2 ,  /* RW */
    TBL_LOGIN_RECORD_FLD_LOCK_TS              = 3 ,  /* RW */
    TBL_LOGIN_RECORD_FLD_FAIL_COUNT           = 4 ,  /* RW */
    TBL_LOGIN_RECORD_FLD_INVALID_USER         = 5 ,  /* RW */
    TBL_LOGIN_RECORD_FLD_MAX                  = 6 
} tbl_login_record_field_id_t;

/* TBL_LOGIN_RECORD defines */
typedef struct
{
    char                 name[USERNAME_SIZE+1];
} tbl_login_record_key_t;

typedef struct
{
    tbl_login_record_key_t key;
    uint32               locked;
    uint32               first_fail_ts;       /* first fail timestamp */
    uint32               lock_ts;             /* lock timestamp */
    uint32               fail_count;          /* fail count */
    uint32               invalid_user;        /* is invalid user */
} tbl_login_record_t;

typedef struct
{
    ctclib_slist_t       *user_list;
} tbl_login_record_master_t;

#endif /* !__TBL_LOGIN_RECORD_DEFINE_H__ */

