/****************************************************************************
* $Id$
*  Smart Config Message Define & Macros
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-23 16:21
* Reason        : First Create.
****************************************************************************/

#ifndef _SCFG_MSG_H_
#define _SCFG_MSG_H_

#define M_DEPLOY_MAN_DEV_STR    "management"

typedef struct scfg_deploy_cfg
{
    int bStartupCfgEmpty:1; /* is startup config empty */
    unsigned char flags; /* smartconfig flags */

    int interval;
#define M_DEPLOY_DEFAULT_INTERVAL       60 /* seconds */
    int retryCnt;
#define M_DEPLOY_DEFAULT_TRYCNT         2 /* try it forever */

    char *tftpSrvs[DHC_MAX_TFTPSRV_RECORDS * 2];
    int nAddrCnt; /* total */

    int32_t manVrfid;
    int32_t vrfid;
} scfg_deploy_cfg_t;

int scfg_client_module_init(struct lib_globals *pzg);

scfg_deploy_cfg_t *scfg_get_deploy_cfg(void);

#endif /* !_SCFG_MSG_H_ */
