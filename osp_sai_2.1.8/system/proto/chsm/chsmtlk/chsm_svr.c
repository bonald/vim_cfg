
/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"
#include "chsm_svr.h"

int32
chsm_svr_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CFG:
        return cfg_cmd_process(sock, p_msg);

    case MSG_OPER_DATA:
        return ipc_pm_process_data_sync(sock, p_msg);
#if 0 
    case MSG_OPER_LCM:
        return _chsm_srv_process_lcm_msg(sock, p_msg);
#endif
    default:
        return -1;
    }
    
    return 0;
}

