#ifndef __LCM_CLIENT_H__
#define __LCM_CLIENT_H__

#include "sal_common.h"
#include "LcmMsg.h"
#include "glb_const.h"
#include "lcm_specific.h"

struct lcm_clnt_s
{
    int32 sock; /* client connection sock fd */
#ifndef _LCM_OSP_
    uint32 seq;
    uint32 is_connected;
    uint32 connecting_in_progress;
#endif
    uint32 logic_slot;
    uint32 keepalive_cnt;          /* The count of keepalive lost expected msg */
    uint32 recv_keepalive_cnt;     /* The count of receive keepalive msg */
    uint32 send_keepalive_cnt;     /* The count of send keepalive ACK msg */
#ifndef _LCM_OSP_
    uint8  ibuf[LCM_BUF_SZ];       /* input buffer */
    uint32 ibuf_len; /* input data len */
    void *msg;
#endif
};
typedef struct lcm_clnt_s lcm_clnt_t;

typedef int (*lcm_clnt_cb_t)(lcm_clnt_t *, LcmMsg_t *);

extern lcm_clnt_cb_t lcm_clnt_cb_tbl[LCM_SRV_CB_TBL_SZ];

void *lcm_get_lcm_client();
int32 lcm_clnt_send(lcm_clnt_t *clnt, LcmMsg_t *req);
int32 lcm_clnt_start(void);
void lcm_clnt_close(lcm_clnt_t *p_lcm_clnt);
int32 lcm_clnt_init(lcm_clnt_t *clnt, uint8 slot);
void lcm_clnt_fini(lcm_clnt_t *clnt);
#endif /* !__LCM_CLIENT_H__ */

