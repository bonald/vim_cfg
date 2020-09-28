
#ifndef __TBL_AUTH_SESSION_DEFINE_H__
#define __TBL_AUTH_SESSION_DEFINE_H__

/* TBL_AUTH_SESSION field defines */
typedef enum
{
    TBL_AUTH_SESSION_FLD_KEY                  = 0 ,  /* READ */
    TBL_AUTH_SESSION_FLD_USER                 = 1 ,  /* RW */
    TBL_AUTH_SESSION_FLD_PASSWD               = 2 ,  /* RW */
    TBL_AUTH_SESSION_FLD_STATUS               = 3 ,  /* READ */
    TBL_AUTH_SESSION_FLD_AUTHENTICATOR        = 4 ,  /* RW */
    TBL_AUTH_SESSION_FLD_OAUTHENTICATOR       = 5 ,  /* RW */
    TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX    = 6 ,  /* RW */
    TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR       = 7 ,  /* RW */
    TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS       = 8 ,  /* RW */
    TBL_AUTH_SESSION_FLD_AUTHOR_START         = 9 ,  /* RW */
    TBL_AUTH_SESSION_FLD_ACCT_START           = 10,  /* RW */
    TBL_AUTH_SESSION_FLD_ACCT_STOP            = 11,  /* RW */
    TBL_AUTH_SESSION_FLD_ACCTCMD_START        = 12,  /* RW */
    TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP       = 13,  /* RW */
    TBL_AUTH_SESSION_FLD_MAX                  = 14
} tbl_auth_session_field_id_t;

/* TBL_AUTH_SESSION defines */
typedef auth_session_key_t tbl_auth_session_key_t;

typedef struct
{
    tbl_auth_session_key_t key;
    user_t               user;                /* encrypted username */
    char                 passwd[PASSWD_SIZE*2]; /* encrypted password */
    uint8                status;              /* session's auth status */
    void                 *pHandle;            /* Auth handler */
    uint32               uCurSessId;          /* current session id */
    uint16               is_dot1x_session;
    uint16               eap_length;
    uint8                eap_buf[AUTH_EAPOL_MAX_LEN];
    uint16               radius_state_len;
    uint8                radius_state_buf[RADIUS_MAX_STATE_LEN];
    uint8                authenticator[RADIUS_AUTHENTICATOR_LEN];
    uint8                oauthenticator[RADIUS_AUTHENTICATOR_LEN];
    uint32               dot1x_mac_ifindex;
    mac_addr_t           dot1x_mac_addr;
    uint16               is_auth_bypass;
    uint8                author_start;
    uint8                acct_start;
    uint8                acct_stop;
    uint8                acctcmd_start;
    uint8                next_acct_stop;
} tbl_auth_session_t;

typedef struct
{
    ctclib_hash_t        *auth_sessions_hash;
    ctclib_slist_t       *auth_sessions;
    uint32               session_cnt;         /* session count */
} tbl_auth_session_master_t;

#endif /* !__TBL_AUTH_SESSION_DEFINE_H__ */

