#ifndef __CHSM_LCMTLK_H__
#define __CHSM_LCMTLK_H__

#include <sys/types.h>
#include <netinet/in.h>
#include "glb_hw_define.h"
#include "glb_const.h"
#include "LcmMsg.h"
#include "lcapi_lcm_lcsh_msg.h"
#include "lcm_specific.h"

#ifndef _LCM_OSP_
#define CHSM_LCM_SESS_CLOSE(_LC_)                                                                                        \
do {                                                                                                                \
    CHSM_DBG_WARN(M_MOD_CHSM, E_WARNING, "chsm lcm server close slot %d %s:%d %s", (_LC_)->phy_slot_no, __FUNCTION__, __LINE__, strerror(errno));   \
    chsm_lcm_sess_close((_LC_));                                                                                         \
} while (0)

struct chsm_lcm_sess_s 
{
    int sock; /* socket fd */
    struct thread *t_read; /* input event */
    struct thread *timer_tx_keepalive;    /* send keep-alive timer */
    struct thread *timer_chk_rx_keepalive; /* check keep-alive ack timer */
    uint32 keepalive_to_threshold;   /* keep timeout threshold */
    uint32 max_broken_cnt;
    uint32 max_broken_threshold;
    uint32 keepalive_cnt;
    uint32 recvKaCnt;
    uint32 sendKaCnt; 
    uint8  ibuf[LCM_BUF_SZ]; /* input buffer */
    uint32 ibuf_len; /* input data len */
    LcmMsg_t *msg; /* partially decoded message */
};
typedef struct chsm_lcm_sess_s chsm_lcm_sess_t;
#endif

#endif /* !__CHSM_LCMTLK_H__ */


