
#ifndef __TBL_AUTH_SERVER_DEFINE_H__
#define __TBL_AUTH_SERVER_DEFINE_H__

/* TBL_AUTH_SERVER field defines */
typedef enum
{
    TBL_AUTH_SERVER_FLD_KEY                  = 0 ,  /* READ */
    TBL_AUTH_SERVER_FLD_SECRET               = 1 ,  /* RW */
    TBL_AUTH_SERVER_FLD_ENC_SECRET           = 2 ,  /* RW */
    TBL_AUTH_SERVER_FLD_PORT                 = 3 ,  /* RW */
    TBL_AUTH_SERVER_FLD_TIMEOUT              = 4 ,  /* RW */
    TBL_AUTH_SERVER_FLD_RETRIES              = 5 ,  /* RW */
    TBL_AUTH_SERVER_FLD_DEAD                 = 6 ,  /* RW */
    TBL_AUTH_SERVER_FLD_IS_INBAND            = 7 ,  /* RW */
    TBL_AUTH_SERVER_FLD_REAWAKE_TIME         = 8 ,  /* READ */
    TBL_AUTH_SERVER_FLD_MAX                  = 9 
} tbl_auth_server_field_id_t;

/* TBL_AUTH_SERVER defines */
typedef auth_server_key_t tbl_auth_server_key_t;

typedef struct
{
    tbl_auth_server_key_t key;
    char                 secret[GLB_AUTH_SECRET_SIZE+1]; /* shared secret string */
    char                 enc_secret[PASSWD_SIZE]; /* encrypted secret string */
    uint16               port;                /* port number */
    uint16               timeout;             /* timeout seconds, default 5 */
    uint8                retries;             /* retransmit counts, default 3 */
    uint8                dead;                /* dead flag */
    uint32               is_inband;
    uint32               reawake_time;        /* rewake time after dead */
    uint32               id;                  /* id of this server, may change after reboot */
    int                  fd;
    void                 *pRead;              /* read handler */
    char                 buf[GLB_AUTH_MSG_BUF_SZ]; /* reply buffer */
    uint16               connected;
    uint16               blen;                /* data's in buffer */
} tbl_auth_server_t;

typedef struct
{
    ctclib_slist_t       *auth_servers;
    uint8                radius_cnt;          /* radius server count */
    uint8                tacplus_cnt;         /* tacplus server count */
    uint32               cur_id;              /* auth server item currrent id */
} tbl_auth_server_master_t;

#endif /* !__TBL_AUTH_SERVER_DEFINE_H__ */

