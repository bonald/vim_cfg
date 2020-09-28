
#ifndef __TBL_AUTH_CFG_DEFINE_H__
#define __TBL_AUTH_CFG_DEFINE_H__

/* TBL_AUTH_CFG field defines */
typedef enum
{
    TBL_AUTH_CFG_FLD_SECRET               = 0 ,  /* RW */
    TBL_AUTH_CFG_FLD_ENC_SECRET           = 1 ,  /* RW */
    TBL_AUTH_CFG_FLD_TIMEOUT              = 2 ,  /* RW */
    TBL_AUTH_CFG_FLD_RETRIES              = 3 ,  /* RW */
    TBL_AUTH_CFG_FLD_DEADTIME             = 4 ,  /* RW */
    TBL_AUTH_CFG_FLD_MAX                  = 5 
} tbl_auth_cfg_field_id_t;

/* TBL_AUTH_CFG defines */
typedef struct
{
    char                 secret[GLB_AUTH_SECRET_SIZE+1]; /* shared secret string */
    char                 enc_secret[PASSWD_SIZE]; /* encrypted secret string */
    uint16               timeout;             /* timeout seconds, default 5 */
    uint16               retries;             /* retransmit counts, default 3 */
    uint16               deadtime;            /* dead time (seconds, [60-1200], default 300) */
} tbl_auth_cfg_t;

#endif /* !__TBL_AUTH_CFG_DEFINE_H__ */

