#ifndef __LCSH_CLIENT_H__
#define __LCSH_CLIENT_H__

#ifdef CNOS_OS
#include "sal.h"
#else
#include "sal_common.h"
#endif
#include "glb_const.h"
#include "lcapi_lcm_lcsh_msg.h"

#define LCSH_CLIENT_MSG_BUF_SIZE     4096

struct lcsh_clnt_s
{
    int32 sock;
    uint32 is_connected;
    uint8  ibuf[LCSH_CLIENT_MSG_BUF_SIZE];       /* input buffer */
    uint32 ibuf_len;                             /* input data len */
    void *msg;
};
typedef struct lcsh_clnt_s lcsh_clnt_t;

typedef int (*lcsh_clnt_cb_t)(lcsh_clnt_t* pst_lcsh_clnt, lcapi_lcm_lcsh_msg_t* pst_lcapi_lcm_lcsh_msg);

extern int32 
lcsh_clnt_send_msg(lcapi_lcm_lcsh_msg_t* pst_req, lcapi_lcm_lcsh_msg_t* pst_res);

extern int32 
lcsh_clnt_start(void);

#endif /* !__LCSH_CLIENT_H__ */

