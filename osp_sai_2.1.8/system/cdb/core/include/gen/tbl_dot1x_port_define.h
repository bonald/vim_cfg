
#ifndef __TBL_DOT1X_PORT_DEFINE_H__
#define __TBL_DOT1X_PORT_DEFINE_H__

/* TBL_DOT1X_PORT field defines */
typedef enum
{
    TBL_DOT1X_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_DOT1X_PORT_FLD_CURRENT_RADIUS       = 1 ,  /* RW */
    TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID        = 2 ,  /* RW */
    TBL_DOT1X_PORT_FLD_RETRY                = 3 ,  /* RW */
    TBL_DOT1X_PORT_FLD_T_WAIT               = 4 ,  /* READ */
    TBL_DOT1X_PORT_FLD_SESSIONTYPE          = 5 ,  /* RW */
    TBL_DOT1X_PORT_FLD_NAME                 = 6 ,  /* READ */
    TBL_DOT1X_PORT_FLD_BUF                  = 7 ,  /* RW */
    TBL_DOT1X_PORT_FLD_DEST_MAC             = 8 ,  /* RW */
    TBL_DOT1X_PORT_FLD_SRC_MAC              = 9 ,  /* RW */
    TBL_DOT1X_PORT_FLD_USER_NAME            = 10,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL = 11,  /* RW */
    TBL_DOT1X_PORT_FLD_T_TIMER              = 12,  /* READ */
    TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION     = 13,  /* RW */
    TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID     = 14,  /* RW */
    TBL_DOT1X_PORT_FLD_GUEST_VLAN           = 15,  /* RW */
    TBL_DOT1X_PORT_FLD_CURRENT_USER         = 16,  /* RW */
    TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB = 17,  /* RW */
    TBL_DOT1X_PORT_FLD_MAX_USER             = 18,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_MODE            = 19,  /* RW */
    TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS      = 20,  /* RW */
    TBL_DOT1X_PORT_FLD_SHARE_REAUTH         = 21,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHWHILE            = 22,  /* RW */
    TBL_DOT1X_PORT_FLD_AWHILE               = 23,  /* RW */
    TBL_DOT1X_PORT_FLD_QUIETWHILE           = 24,  /* RW */
    TBL_DOT1X_PORT_FLD_REAUTHWHEN           = 25,  /* RW */
    TBL_DOT1X_PORT_FLD_TXWHEN               = 26,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHABORT            = 27,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHFAIL             = 28,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHSTART            = 29,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHTIMEOUT          = 30,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTHSUCCESS          = 31,  /* RW */
    TBL_DOT1X_PORT_FLD_INITIALIZE           = 32,  /* RW */
    TBL_DOT1X_PORT_FLD_PORTENABLED          = 33,  /* RW */
    TBL_DOT1X_PORT_FLD_REAUTHENTICATE       = 34,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPNOREQ             = 35,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPRESTART           = 36,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPRESP              = 37,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPREQ               = 38,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPSUCCESS           = 39,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPFAIL              = 40,  /* RW */
    TBL_DOT1X_PORT_FLD_RETRANSMIT           = 41,  /* RW */
    TBL_DOT1X_PORT_FLD_CURRENTID            = 42,  /* RW */
    TBL_DOT1X_PORT_FLD_PORTCONTROL          = 43,  /* RW */
    TBL_DOT1X_PORT_FLD_PORTSTATUS           = 44,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF   = 45,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART    = 46,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE    = 47,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT = 48,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID    = 49,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE       = 50,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD = 51,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX   = 52,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD    = 53,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING = 54,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING = 55,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING = 56,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING = 57,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING = 58,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING = 59,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING = 60,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING = 61,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED = 62,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED = 63,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED = 64,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING = 65,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE = 66,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED = 67,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN = 68,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE = 69,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE  = 70,  /* RW */
    TBL_DOT1X_PORT_FLD_REAUTHPERIOD         = 71,  /* RW */
    TBL_DOT1X_PORT_FLD_REAUTHENABLED        = 72,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_STATE        = 73,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT     = 74,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER = 75,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT  = 76,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT = 77,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ       = 78,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES = 79,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES = 80,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT = 81,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS = 82,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS = 83,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT = 84,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE  = 85,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS = 86,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS = 87,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED = 88,  /* RW */
    TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY  = 89,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX      = 90,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX      = 91,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX = 92,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX = 93,  /* RW */
    TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX = 94,  /* RW */
    TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX   = 95,  /* RW */
    TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX  = 96,  /* RW */
    TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX    = 97,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX = 98,  /* RW */
    TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX = 99,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION = 100,  /* RW */
    TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE = 101,  /* RW */
    TBL_DOT1X_PORT_FLD_SESSION_TIME         = 102,  /* RW */
    TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD = 103,  /* RW */
    TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE = 104,  /* RW */
    TBL_DOT1X_PORT_FLD_SESSION_USER_NAME    = 105,  /* RW */
    TBL_DOT1X_PORT_FLD_CLASS_ID             = 106,  /* RW */
    TBL_DOT1X_PORT_FLD_MAX                  = 107
} tbl_dot1x_port_field_id_t;

/* TBL_DOT1X_PORT defines */
typedef struct
{
    uint32               ifindex;
} tbl_dot1x_port_key_t;

typedef struct
{
    tbl_dot1x_port_key_t key;
    void                 *current_radius;
    uint32               radius_msg_id;
    uint32               retry;
    ctc_task_t           *t_wait;
    uint32               sessiontype;
    char                 name[IFNAME_SIZE];
    void                 *p_if;               /* interface pointer */
    void                 *p_buf;
    uint8                buf[AUTH_EAPOL_MAX_LEN];
    mac_addr_t           dest_mac;
    mac_addr_t           src_mac;
    uint8                user_name[USERNAME_SIZE+1];
    uint8                authControlledPortControl;
    ctc_task_t           *t_timer;
    uint8                protocol_version;
    uint8                guest_vlan_valid;
    uint16               guest_vlan;
    uint32               current_user;
    uint32               current_user_count_db; /*software table*/
    uint32               max_user;
    uint8                auth_mode;
    uint8                mac_auth_bypass;
    uint16               share_reauth;
    uint32               authWhile;
    uint32               aWhile;
    uint32               quietWhile;
    uint32               reAuthWhen;
    uint32               txWhen;
    uint8                authAbort;
    uint8                authFail;
    uint8                authStart;
    uint8                authTimeout;
    uint8                authSuccess;
    uint8                initialize;
    uint8                portEnabled;
    uint8                reAuthenticate;
    uint8                eapNoReq;
    uint8                eaprestart;
    uint8                eapResp;
    uint8                eapReq;
    uint8                eapSuccess;
    uint8                eapFail;
    uint16               retransmit;
    uint8                currentId;
    uint8                portControl;
    uint16               portStatus;
    uint8                auth_pae_eapLogoff;  /* Authenticator PAE variables.  */
    uint8                auth_pae_eapStart;
    uint8                auth_pae_portMode;
    uint8                auth_pae_reAuthCount;
    uint8                auth_pae_rxRespId;
    uint32               auth_pae_state;      /* The authenticator PAE state.  */
    uint32               auth_pae_quietPeriod; /* Default 60s.  */
    uint32               auth_pae_reAuthMax;  /* Default 2 attempts.  */
    uint32               auth_pae_txPeriod;   /* Default 30s.  */
    uint32               auth_pae_enter_connecting; /* Authenticator PAE Counters */
    uint32               auth_pae_eaplogoff_while_connecting;
    uint32               auth_pae_enter_authenticating;
    uint32               auth_pae_success_while_authenticating;
    uint32               auth_pae_timeout_while_authenticating;
    uint32               auth_pae_fail_while_authenticating;
    uint32               auth_pae_eapstart_while_authenticating;
    uint32               auth_pae_eaplogoff_while_authenticating;
    uint32               auth_pae_reauths_while_authenticated;
    uint32               auth_pae_eapstart_while_authenticated;
    uint32               auth_pae_eaplogoff_while_authenticated;
    uint32               auth_pae_reauths_while_authenticating;
    uint32               auth_key_xmit_keyAvailable; /* 8.5.5 auth_key_xmit */
    uint32               auth_key_xmit_keyTxEnabled;
    uint8                auth_key_xmit_KeyRun;
    uint8                auth_key_xmit_KeyDone;
    uint16               auth_key_xmit_state;
    uint32               reAuthPeriod;        /* 8.5.7 reauth_timer */
    uint32               reAuthEnabled;
    uint32               auth_be_state;       /* 8.5.8 auth_be */
    uint8                auth_be_reqCount;
    uint8                auth_be_idFromServer;
    uint32               auth_be_suppTimeout; /* Default 30s.  */
    uint32               auth_be_serverTimeout; /* Default 30s.  */
    uint16               auth_be_maxReq;      /* Default 2 times.  */
    uint32               auth_be_backend_responses; /* Backend state machine Counters */
    uint32               auth_be_backend_access_challenges;
    uint32               auth_be_backend_otherrequest_to_supplicant;
    uint32               auth_be_backend_auth_success;
    uint32               auth_be_backend_auth_fails;
    uint32               auth_be_backend_nonnak_responses_from_supplicant;
    uint8                auth_ctrl_dir_state; /* 8.5.9 auth_ctrl_dir */
    uint8                auth_ctrl_dir_adminControlledDirections;
    uint8                auth_ctrl_dir_operControlledDirections;
    uint8                auth_ctrl_dir_bridgeDetected;
    uint8                auth_key_recv_rxKey; /* 8.5.11 auth_key_recv */
    uint32               eapol_frames_rx;     /* auth_port_stats 9.4.2.1.3 */
    uint32               eapol_frames_tx;
    uint32               eapol_start_frames_rx;
    uint32               eapol_logoff_frames_rx;
    uint32               eap_respid_frames_rx;
    uint32               eap_resp_frames_rx;
    uint32               eap_reqid_frames_tx;
    uint32               eap_req_frames_tx;
    uint32               eapol_invalid_frames_rx;
    uint32               eap_len_error_frames_rx;
    uint32               eapol_last_frame_version;
    uint8                eapol_last_frame_source[GLB_ETH_ADDR_LEN];
    uint32               session_time;        /* session statistics variables */
    uint8                session_authentication_method;
    uint8                session_terminate_cause;
    uint8                session_user_name[AUTH_USER_NAMLEN+1];
    uint32               class_id;
} tbl_dot1x_port_t;

typedef struct
{
    ctclib_hash_t        *dot1x_port_hash;
    ctclib_slist_t       *dot1x_port_list;
} tbl_dot1x_port_master_t;

#endif /* !__TBL_DOT1X_PORT_DEFINE_H__ */

