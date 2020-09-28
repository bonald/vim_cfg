
#ifndef __TBL_USER_DEFINE_H__
#define __TBL_USER_DEFINE_H__

/* TBL_USER field defines */
typedef enum
{
    TBL_USER_FLD_KEY                  = 0 ,  /* READ */
    TBL_USER_FLD_UID                  = 1 ,  /* READ */
    TBL_USER_FLD_PRIVILEGE            = 2 ,  /* RW */
    TBL_USER_FLD_ENC_PASSWD           = 3 ,  /* RW */
    TBL_USER_FLD_PASSWD               = 4 ,  /* RW */
    TBL_USER_FLD_RSAKEY               = 5 ,  /* RW */
    TBL_USER_FLD_SERVICE_TYPE         = 6 ,  /* RW */
    TBL_USER_FLD_MAX                  = 7 
} tbl_user_field_id_t;

/* TBL_USER defines */
typedef struct
{
    char                 name[USERNAME_SIZE+1];
} tbl_user_key_t;

typedef struct
{
    tbl_user_key_t       key;
    uint32               uid;                 /* user's id */
    uint8                privilege;           /* user's privilege <1-4> */
    char                 enc_passwd[PASSWD_SIZE]; /* encrypted password */
    char                 passwd[PASSWD_SIZE]; /* password of this user */
    char                 rsakey[M_RSA_KEY_NAMESZ+1]; /* RSA key name for this user */
    uint8                service_type;        /* service_type */
} tbl_user_t;

typedef struct
{
    ctclib_slist_t       *user_list;
} tbl_user_master_t;

#endif /* !__TBL_USER_DEFINE_H__ */

