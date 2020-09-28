
#ifndef __TBL_DOT1X_MAC_DEFINE_H__
#define __TBL_DOT1X_MAC_DEFINE_H__

/* TBL_DOT1X_MAC field defines */
typedef enum
{
    TBL_DOT1X_MAC_FLD_KEY                  = 0 ,  /* READ */
    TBL_DOT1X_MAC_FLD_USER_NAME            = 1 ,  /* RW */
    TBL_DOT1X_MAC_FLD_SESSION_ID           = 2 ,  /* RW */
    TBL_DOT1X_MAC_FLD_CURRENT_RADIUS       = 3 ,  /* RW */
    TBL_DOT1X_MAC_FLD_T_WAIT               = 4 ,  /* READ */
    TBL_DOT1X_MAC_FLD_T_REAUTH             = 5 ,  /* READ */
    TBL_DOT1X_MAC_FLD_T_REJECT             = 6 ,  /* READ */
    TBL_DOT1X_MAC_FLD_T_DELETE             = 7 ,  /* READ */
    TBL_DOT1X_MAC_FLD_ENTRY_ID             = 8 ,  /* RW */
    TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS       = 9 ,  /* RW */
    TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT      = 10,  /* RW */
    TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT      = 11,  /* RW */
    TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS      = 12,  /* RW */
    TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT = 13,  /* RW */
    TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH = 14,  /* RW */
    TBL_DOT1X_MAC_FLD_MAX                  = 15
} tbl_dot1x_mac_field_id_t;

/* TBL_DOT1X_MAC defines */
typedef dot1x_mac_key_t tbl_dot1x_mac_key_t;

typedef struct
{
    tbl_dot1x_mac_key_t  key;
    uint8                user_name[AUTH_USER_NAMLEN+1];
    uint32               session_id;
    void                 *current_radius;
    ctc_task_t           *t_wait;
    ctc_task_t           *t_reauth;           /*timer for user reauth*/
    ctc_task_t           *t_reject;           /*timer for user reject*/
    ctc_task_t           *t_delete;           /*timer for delete unprocessed */
    uint32               entry_id;
    uint32               is_auth_bypass;
    void                 *dot1x_port;
    uint32               auth_mac_flag;       /* bitmap of AUTHD_FLAG_DOT1X */
} tbl_dot1x_mac_t;

typedef struct
{
    ctclib_hash_t        *dot1x_mac_hash;
    ctclib_slist_t       *dot1x_mac_list;
} tbl_dot1x_mac_master_t;

#endif /* !__TBL_DOT1X_MAC_DEFINE_H__ */

