#ifndef __DOT1X_H__
#define __DOT1X_H__

#define DOT1X_LOG_EMERG(fmt, args...)    log_sys(M_MOD_AUTHD, E_EMERGENCY, fmt, ##args)
#define DOT1X_LOG_ALERT(fmt, args...)    log_sys(M_MOD_AUTHD, E_ALERT, fmt, ##args)
#define DOT1X_LOG_CRIT(fmt, args...)     log_sys(M_MOD_AUTHD, E_CRITICAL, fmt, ##args)
#define DOT1X_LOG_ERR(fmt, args...)      log_sys(M_MOD_AUTHD, E_ERROR, fmt, ##args)
#define DOT1X_LOG_WARN(fmt, args...)     log_sys(M_MOD_AUTHD, E_WARNING, fmt, ##args)
#define DOT1X_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_AUTHD, E_NOTICE, fmt, ##args)
#define DOT1X_LOG_INFO(fmt, args...)     log_sys(M_MOD_AUTHD, E_INFORMATIONAL, fmt, ##args)

#define DOT1X_EAPOL_ETHERTYPE    0x888E

#define RADIUS_SHARE_KEY_MIN_LEN 1
#define RADIUS_SHARE_KEY_MAX_LEN 64

#define INVALID_RADIUS_MSG_ID    0xffffffff

#define DOT1X_PORT_TIMER_INTERVAL    1

#define AUTH_SUPPLICANT_LOGOFF         1
#define AUTH_PORT_FAILURE              2
#define AUTH_SUPPLICANT_RESTART        3
#define AUTH_REAUTH_FAILURE            4
#define AUTH_SET_TO_FORCE_UNAUTHORIZED 5
#define AUTH_PORT_REINIT               6
#define AUTH_PORT_ADMIN_DISABLE        7
#define AUTH_NOT_TERMINATED_YET        8

#define DEFAULT_MAC_USER_COUNT_MAX    256

#define AUTH_MAC_ENTRY_WAITING_TIME  60

#define AUTH_NO_KEY_XMIT             0
#define AUTH_KEY_XMIT                1

#define AUTH_CTRL_DIR_BOTH           0
#define AUTH_CTRL_DIR_IN             1

#define FORCE_BOTH 1
#define IN_OR_BOTH 2

#define AUTH_BE_STATE_INVALID        0
#define AUTH_BE_STATE_REQUEST        1
#define AUTH_BE_STATE_RESPONSE       2
#define AUTH_BE_STATE_SUCCESS        3
#define AUTH_BE_STATE_FAIL           4
#define AUTH_BE_STATE_TIMEOUT        5
#define AUTH_BE_STATE_IDLE           6
#define AUTH_BE_STATE_INITIALIZE     7
#define AUTH_BE_STATE_IGNORE         8
#define AUTH_BE_STATE_MAX            9

/* 802.1X-2001 8.5.4 Authenticator PAE state.  */
#define AUTH_PAE_STATE_DOWN             0
#define AUTH_PAE_STATE_INIT             1
#define AUTH_PAE_STATE_DISCONNECTED     2
#define AUTH_PAE_STATE_CONNECTING       3
#define AUTH_PAE_STATE_AUTHENTICATING   4
#define AUTH_PAE_STATE_AUTHENTICATED    5
#define AUTH_PAE_STATE_ABORTING         6
#define AUTH_PAE_STATE_HELD             7
#define AUTH_PAE_STATE_FORCE_AUTH       8
#define AUTH_PAE_STATE_FORCE_UNAUTH     9
#define AUTH_PAE_STATE_RESTART          10
#define AUTH_PAE_STATE_MAX              11

#define AUTH_LOCAL_AUTHN_SERVER  0
#define AUTH_REMOTE_AUTHN_SERVER 1

#define AUTH_CTRL_DIR_BOTH       0
#define AUTH_CTRL_DIR_IN         1

#define RADIUS_ATTR_HDR_LEN        2
#define RADIUS_FRAMED_MTU_DEFAULT  1000
#define RADIUS_AUTHENTICATOR_LEN   16

#define AUTH_MAC_ENTRY_WAITING_TIME  60

#define AUTH_MAC_SESSION_HEADER    0x100

/* after time out, clear pSess */
#define AUTH_WAIT_TIMER_TIMEOUT    1

#define RADIUS_ATTR_MAX_LEN   253

bool dot1x_debug_is_on(uint32 flag);

#define DOT1X_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (dot1x_debug_is_on(flag)) \
        log_sys(M_MOD_AUTHD, E_DEBUG, fmt, ##args); \
} while (0)

#define DOT1X_PTR_CHECK(ptr) \
    do { \
        if (NULL == ptr) \
        {\
            sal_printf("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
            return(PM_E_INVALID_PARAM); \
        }\
    }while(0)

#define DEC(x) { if(x !=0) x--; }

#define INC(x)  x = (((x)+1) & 0xff)

int dot1x_start();
int32 dot1x_init(void);

char *get_intf_print_name(char *pszIfname);

#endif /* !__DOT1X_H__ */
