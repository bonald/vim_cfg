#ifndef __LCM_CHSMTLK_H__
#define __LCM_CHSMTLK_H__

#include "glb_hw_define.h"
struct vct_msg_s
{
    uint32 port_id;
};
typedef struct vct_msg_s vct_msg_t; 

int32 lcm_chsm_msg_tx_lcAttach(lcm_clnt_t *clnt);
/* Added by liuht for bug27036, 2014-02-27 */
int32 lcm_chsm_msg_tx_lcRebootInfo(lcm_clnt_t *clnt);
int32 lcm_chsm_msg_tx_lcDeattach(lcm_clnt_t *clnt);
int32 lcm_chsm_msg_tx_lcOemInfo(lcm_clnt_t* clnt);


#endif
