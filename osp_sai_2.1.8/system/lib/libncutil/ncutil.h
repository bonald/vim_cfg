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

#ifndef _NCUTIL_H_
#define _NCUTIL_H_

#include "sysrepo.h"
#include "sysrepo/values.h"
#include "sysrepo/xpath.h"

#include "ctc_sr.h"

#define ncutil_do_commit ctc_nc_do_commit
int ncutil_init(char *appname, void *pArg, ctc_nc_reinit_cb ricb);
void ncutil_deinit(void);

#endif /* !_NCUTIL_H_ */
