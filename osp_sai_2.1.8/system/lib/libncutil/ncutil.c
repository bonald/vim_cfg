/****************************************************************************
* $Id$
*  netconf util library for CentecOS
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2018-04-12 08:21
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/

#include "proto.h"

#include "ncutil.h"
#include "ctc_sr.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
int32 ctc_task_unset_write(ctc_task_t *p_task);

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
int ncutil_event_cb(int fd, void *pArg, ctc_nc_event_e event, ctc_nc_event_arg_t *earg);

/****************************************************************************
 *
* Function
* 
****************************************************************************/
int ncutil_line_out (void *pArg, const char *format, ...)
{
#if 0
    char szBuf[256];
    va_list args;
    va_start (args, format);
    vsnprintf(szBuf, 256, format, args);
    va_end (args);
#endif
    return 0;
}


int ncutil_init(char *appname, void *pArg,
        ctc_nc_reinit_cb ricb)
{
    return ctc_nc_init(appname, ncutil_event_cb, pArg, ricb);
}

void ncutil_deinit(void)
{
    ctc_nc_deinit();
    return;
}

void ncutil_fd_read(void *pTh)
{
    ctc_nc_event_arg_t *pArg = (ctc_nc_event_arg_t *)pTh;

    ctc_nc_sess_process(pArg->fd, CTC_NC_EVENT_READ);
    return;
}

void ncutil_fd_write(void *pTh)
{
    ctc_nc_event_arg_t *pArg = (ctc_nc_event_arg_t *)pTh;

    ctc_nc_sess_process(pArg->fd, CTC_NC_EVENT_WRITE);
    return;
}


int ncutil_event_cb(int fd, void *pArg, ctc_nc_event_e event, ctc_nc_event_arg_t *eArg)
{
    ctc_task_t *pTh = NULL;
    assert(NULL != eArg);

    if (CTC_NC_EVENT_READ == event || CTC_NC_EVENT_WRITE == event) {
        if (CTC_NC_EVENT_READ == event) {
            pTh = ctc_task_add_read(TASK_PRI_NORMAL, 
                    ncutil_fd_read, eArg, eArg->fd); 
            eArg->pRead = (void *)pTh;
        } else {
            if (NULL == eArg->pArgRead) {
                pTh = ctc_task_add_write(TASK_PRI_NORMAL, 
                        ncutil_fd_write, eArg, eArg->fd);
            } else {
                pTh = eArg->pArgRead;
                ctc_task_set_write(pTh, ncutil_fd_write);
            }
            eArg->pWrite = (void *)pTh;
        }

        if (NULL == pTh) {
            ctc_nc_err("ncutil_event_cb create thread failed event: %d", event);
            return -1;
        }
        return 0;
    } 

    if (CTC_NC_EVENT_STOP_READ == event || CTC_NC_EVENT_STOP_WRITE == event) {
        if (CTC_NC_EVENT_STOP_READ == event) {
            if (eArg->pRead) {
                ctc_task_delete_read((ctc_task_t *)eArg->pRead);
                eArg->pRead = NULL;
                eArg->pWrite = NULL;
            }
        } else {
            if (eArg->pRead) {
                ctc_task_unset_write((ctc_task_t *)eArg->pWrite);
            } else if (eArg->pWrite) {
                ctc_task_delete_write((ctc_task_t *)eArg->pWrite);
            }
            eArg->pWrite = NULL;
        }
        return 0;
    }

    ctc_nc_err("ncutil_event_cb unknown event: %d", event);
    return -1;
}


