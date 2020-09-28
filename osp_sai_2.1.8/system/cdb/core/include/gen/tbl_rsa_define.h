
#ifndef __TBL_RSA_DEFINE_H__
#define __TBL_RSA_DEFINE_H__

/* TBL_RSA field defines */
typedef enum
{
    TBL_RSA_FLD_KEY                  = 0 ,  /* READ */
    TBL_RSA_FLD_TYPE                 = 1 ,  /* READ */
    TBL_RSA_FLD_REFCNT               = 2 ,  /* READ */
    TBL_RSA_FLD_PEM                  = 3 ,  /* RW */
    TBL_RSA_FLD_ENC_PASSWD           = 4 ,  /* RW */
    TBL_RSA_FLD_PASSWD               = 5 ,  /* RW */
    TBL_RSA_FLD_MAX                  = 6 
} tbl_rsa_field_id_t;

/* TBL_RSA defines */
typedef struct
{
    char                 name[M_RSA_KEY_NAMESZ+1];
} tbl_rsa_key_t;

typedef struct
{
    tbl_rsa_key_t        key;
    uint8                type;                /* private or public */
    uint8                refcnt;              /* reference count */
    rsa_keystr_t         pem;                 /* RSA key in pem format (base64 encoding) */
    char                 enc_passwd[PASSWD_SIZE]; /* encrypted password for private key */
    char                 passwd[PASSWD_SIZE]; /* password for private key */
    void                 *pstRsa;             /* RSA key pointer */
} tbl_rsa_t;

typedef struct
{
    ctclib_slist_t       *rsa_list;
} tbl_rsa_master_t;

#endif /* !__TBL_RSA_DEFINE_H__ */

