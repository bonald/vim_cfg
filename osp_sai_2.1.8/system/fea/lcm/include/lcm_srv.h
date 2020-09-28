#ifndef __LCM_SERVER_H__
#define __LCM_SERVER_H__

#include "sal_common.h"
#include "lcapi_lcm_lcsh_msg.h"
#include "LcmMsg.h"
#include "lcm_error.h"
#include "lcm_specific.h"

#define LCM_SRV_MSG_BUF_SIZE     1024

struct lcm_sess_s {
    int32 sock; /* socket fd */
    uint32 recvKaCnt;
    uint32 sendKaCnt; 
    uint8 ibuf[LCM_SRV_MSG_BUF_SIZE]; /* input buffer */
    int32 ibuf_len; /* input data len */
#ifdef CNOS_OS
    ctc_task_t *t_read; /* input event */
#else
    ctclib_thread_t *t_read; /* input event */
#endif
};
typedef struct lcm_sess_s lcm_sess_t;

typedef int32 (*lcm_srv_cb_t)(lcm_sess_t* pst_sess, lcapi_lcm_lcsh_msg_t* pst_lcapi_lcm_lcsh_msg);

int32 lcm_svr_init();
int32 lcm_sess_send(lcm_sess_t* pst_sess, lcapi_lcm_lcsh_msg_t* pst_lcapi_lcm_lcsh_msg);
int32 lcm_srv_start(void);
LCM_TASK_FUNC(_lcm_sess_read);
LCM_TASK_FUNC(_lcm_srv_accept);
LCM_DECLARE_TASK_ENCAP(_lcm_sess_read)
LCM_DECLARE_TASK_ENCAP(_lcm_srv_accept)

#endif /* !__LCM_SERVER_H__ */

