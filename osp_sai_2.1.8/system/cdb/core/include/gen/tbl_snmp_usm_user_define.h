
#ifndef __TBL_SNMP_USM_USER_DEFINE_H__
#define __TBL_SNMP_USM_USER_DEFINE_H__

/* TBL_SNMP_USM_USER field defines */
typedef enum
{
    TBL_SNMP_USM_USER_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_REMO_ENGID           = 2 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_AUTH_TYPE            = 3 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_AUTH_PWD             = 4 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_ENC_AUTH_PWD         = 5 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_PRIV_TYPE            = 6 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_PRIV_PWD             = 7 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_ENC_PRIV_PWD         = 8 ,  /* RW */
    TBL_SNMP_USM_USER_FLD_MAX                  = 9 
} tbl_snmp_usm_user_field_id_t;

/* TBL_SNMP_USM_USER defines */
typedef struct
{
    char                 usm_user_name[SNMP_NAME_SIZE];
} tbl_snmp_usm_user_key_t;

typedef struct
{
    tbl_snmp_usm_user_key_t key;
    uint8                flags;               /* delete entry 0xFF */
    char                 remo_engid[SNMP_ENGINEID_SIZE]; /* remote engine id hex string */
    uint8                auth_type;           /* authentication type md5 or sha */
    char                 auth_pwd[PASSWD_SIZE]; /* authentication password 8-64 */
    char                 enc_auth_pwd[PASSWD_SIZE]; /* authentication enc password */
    uint8                priv_type;           /* privacy type des or aes */
    char                 priv_pwd[PASSWD_SIZE]; /* privacy password 8-64 */
    char                 enc_priv_pwd[PASSWD_SIZE]; /* privacy enc password */
} tbl_snmp_usm_user_t;

typedef struct
{
    ctclib_hash_t        *snmp_usm_user_hash;
    ctclib_slist_t       *snmp_usm_user_list;
} tbl_snmp_usm_user_master_t;

#endif /* !__TBL_SNMP_USM_USER_DEFINE_H__ */

