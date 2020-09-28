
#ifndef __TBL_DOT1X_RADIUS_DEFINE_H__
#define __TBL_DOT1X_RADIUS_DEFINE_H__

/* TBL_DOT1X_RADIUS field defines */
typedef enum
{
    TBL_DOT1X_RADIUS_FLD_KEY                  = 0 ,  /* READ */
    TBL_DOT1X_RADIUS_FLD_IDENTIFIER           = 1 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SERVER_STRING        = 2 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SERVER_ADDR          = 3 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN     = 4 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_RADIUS_STATE         = 5 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_MAX_RETRY            = 6 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT = 7 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT = 8 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY    = 9 ,  /* RW */
    TBL_DOT1X_RADIUS_FLD_TIMEOUT              = 10,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID  = 11,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SHARED_SECRET        = 12,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET    = 13,  /* RW */
    TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR        = 14,  /* RW */
    TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR       = 15,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER       = 16,  /* RW */
    TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX      = 17,  /* RW */
    TBL_DOT1X_RADIUS_FLD_REACTIVATE           = 18,  /* RW */
    TBL_DOT1X_RADIUS_FLD_IS_INBAND            = 19,  /* RW */
    TBL_DOT1X_RADIUS_FLD_MAX                  = 20
} tbl_dot1x_radius_field_id_t;

/* TBL_DOT1X_RADIUS defines */
typedef dot1x_radius_key_t tbl_dot1x_radius_key_t;

typedef struct
{
    tbl_dot1x_radius_key_t key;
    uint32               identifier;
    uint8                server_string[RADIUS_MAX_HOSTNAME_LEN];
    addr_ipv4_t          server_addr;
    uint8                radius_state_len;
    uint8                radius_state[RADIUS_MAX_STATE_LEN];
    uint32               max_retry;
    uint32               radius_config;       /* bitmap of AUTH_RADIUS_CONFIG_DEFAULT_ */
    uint32               timeout;
    uint8                shared_secret_valid;
    uint8                shared_secret[RADIUS_SHARED_SECRET_LEN+1];
    char                 shared_enc_secret[PASSWD_SIZE]; /* encrypted secret string */
    uint8                authenticator[RADIUS_AUTHENTICATOR_LEN];
    uint8                oauthenticator[RADIUS_AUTHENTICATOR_LEN];
    uint8                session_number;
    uint32               session_ifindex[RADIUS_SESSION_BUFFER];
    uint32               reactivate;
    uint32               is_inband;
} tbl_dot1x_radius_t;

typedef struct
{
    ctclib_slist_t       *radius_list;
} tbl_dot1x_radius_master_t;

#endif /* !__TBL_DOT1X_RADIUS_DEFINE_H__ */

