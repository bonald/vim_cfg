/****************************************************************************
* $Id$
*  Smart Config Controller Macros & structs
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-22 14:54
* Reason        : First Create.
****************************************************************************/

#ifndef _SMART_CONFIG_FUNC_H_
#define _SMART_CONFIG_FUNC_H_

#ifndef SCFG_MSG_BUF_SZ
#define SCFG_MSG_BUF_SZ     4096
#endif

typedef struct scfgctl_sess
{
    char buf[SCFG_MSG_BUF_SZ];
    int len;

    int sock;
    int id;
} scfgctl_sess_t;

int scfgctl_module_init(struct lib_globals *);

typedef int (*scfgctl_bound_cb_t)(char *ifname, 
        int32_t vrfid, char *tftpaddrs, size_t len, void *pArg);

int scfgctl_send_bound(char *ifname, int32_t vrfid, char *tftpaddrs, size_t len);
int scfgctl_send_unbound(char *ifname, int32_t vrfid);

#endif /* !_SMART_CONFIG_FUNC_H_ */
