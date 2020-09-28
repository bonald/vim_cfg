/****************************************************************************
* $Id$
*  netconf for switch
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2018-04-24 16:33
* Reason        : First Create.
****************************************************************************/

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 16:32 */

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "proto.h"
#include "swth_sys.h"
#include "ncutil.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/****************************************************************************
 *
* Function
* 
****************************************************************************/

void swth_nc_reinit_cb(void);
int swth_nc_init(void);

void swth_nc_reinit(void *th)
{
    static int last_warn = 0;
    if (swth_nc_init() != 0) {
        /* reconnect failed */
        time_t now = time(NULL);
        if (now > last_warn) {
            last_warn = now + 60;
            log_sys(M_MOD_INTERNAL, E_CRITICAL, "swth_nc_reinit failed");
        }
        ctc_task_add_timer(TASK_PRI_NORMAL, swth_nc_reinit, NULL, 1);
        return;
    }
    return;
}

void swth_nc_reinit_cb(void)
{
    ctc_task_add_timer(TASK_PRI_NORMAL, swth_nc_reinit, NULL, 1);
    return;
}

int swth_nc_init(void)
{
    /* init netconf client */
    if (ncutil_init("switch", NULL, swth_nc_reinit_cb) != 0) {
        log_sys(M_MOD_INTERNAL, E_CRITICAL, "swth_nc_init failed");
        goto err_out;
    }

    ctc_nc_session_switch_ds(NULL, SR_DS_RUNNING);
    return 0;

err_out:
    ncutil_deinit();
    return -1;
}

#endif
