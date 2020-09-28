
#ifndef __AUTHD_H__
#define __AUTHD_H__

#define M_TRIM_CR(szLine) \
do { \
    char *_p = szLine + strlen(szLine) - 1; \
    if (*_p == '\n') { \
        *_p = '\0'; \
        _p--; \
    } \
    if (*_p == '\r') { \
        *_p = '\0'; \
    } \
} while (0); \

    
int32 authd_tbl_init();

int32 authd_proc_startup_done();

struct auth_be_op_s;

typedef struct {
    ctc_task_t *pConnect; /* read / write task */
    ctc_task_t *pTimeout; /* timeout task */

    void *pObj; /* object for auth backend */

    u_int32_t nServerId; /* server index for retry */

    u_int32_t nRetries; /* retry round */

    struct auth_be_op_s *pCBS; /* callback functions */
} auth_handle_t;

typedef void (*AUTH_RESULT_CB)(tbl_auth_session_t *pSess, int nRet, char *pszFmt, ...);

/* auth backend operation callbacks */
struct auth_be_op_s
{
    /* init handle */
    int (*init)(auth_handle_t *h); /* 0 for success */

    /* connect to auth server */
#define M_AUTH_CONNECT_FAILED       0x01 /* connect failed */
#define M_AUTH_CONNECT_INPROGRESS   0x02 /* connect in progress */
#define M_AUTH_CONNECT_SUCCESS      0x04 /* connect success */
    int (*do_connect)(auth_handle_t *h, tbl_auth_server_t *pSrv);
    
    /* release handle */
    void (*release)(auth_handle_t *h);

#define M_AUTH_SEND_FAILED          0x01
#define M_AUTH_SEND_SUCCESS         0x02
    /* send auth request */
    int (*send_request)(auth_handle_t *h, tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv);

    /* process auth reply
     *  ppSess is return var
     */
#define M_AUTH_ERROR            0x01    /* auth operation error */
#define M_AUTH_FAILED           0x02    /* auth failed */
#define M_AUTH_INPROGRESS       0x04    /* auth in progress */
#define M_AUTH_SUCCESS          0x08    /* auth success */
    int (*process_reply)(tbl_auth_server_t *pSrv, AUTH_RESULT_CB pCB);

    void (*get_error_msg)(auth_handle_t *h, char *pszErrBuf, size_t nBufSz);
};

typedef struct auth_be_op_s auth_be_op_t;

uint8 
aaa_map_privilege(uint32 privilege);

uint8 
aaa_remap_privilege(uint8 privilege);

#endif /* !__AUTHD_H__ */

