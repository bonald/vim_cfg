
#ifndef __TBL_DOT1X_GLOBAL_DEFINE_H__
#define __TBL_DOT1X_GLOBAL_DEFINE_H__

/* TBL_DOT1X_GLOBAL field defines */
typedef enum
{
    TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE   = 0 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS  = 1 ,  /* READ */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY = 2 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT = 3 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME = 4 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX   = 5 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT = 6 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB    = 7 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT = 8 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID = 9 ,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET = 10,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET = 11,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID      = 12,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED = 13,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL            = 14,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET         = 15,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER          = 16,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL       = 17,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT          = 18,  /* RW */
    TBL_DOT1X_GLOBAL_FLD_MAX                  = 19
} tbl_dot1x_global_field_id_t;

/* TBL_DOT1X_GLOBAL defines */
typedef struct
{
    uint32               global_auth_enable;
    mac_addr_t           eapol_group_address;
    uint16               radius_default_retry;
    uint32               radius_default_timeout;
    uint32               radius_default_deadtime;
    uint32               mac_user_count_max;
    uint32               mac_user_count_current;
    uint32               mac_user_count_db;
    uint32               mac_port_count_current;
    uint8                radius_default_shared_secret_valid;
    uint8                radius_default_shared_secret[RADIUS_SHARED_SECRET_LEN+1];
    char                 radius_default_shared_enc_secret[PASSWD_SIZE]; /* encrypted secret string */
    uint8                last_session_id;
    uint8                session_id_allocated[RADIUS_SESSION_BUFFER];
    uint32               dot1x;               /* bitmap of DOT1X_FLAG_ */
} tbl_dot1x_global_t;

#endif /* !__TBL_DOT1X_GLOBAL_DEFINE_H__ */

