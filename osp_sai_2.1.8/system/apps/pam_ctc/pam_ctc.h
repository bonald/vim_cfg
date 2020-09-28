/****************************************************************************
* $Id$
*  pam module for centec switch
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-28 15:44
* Reason        : First Create.
****************************************************************************/
#ifndef _PAM_CTC_H
#define _PAM_CTC_H

/* Header files */
#include <security/pam_appl.h>

/* argument parsing */
#define PAM_DEBUG_ARG		0x0001
#define PAM_ICASE_ARG		0x0002
#define PAM_DUMP_ARG		0x0004
#define PAM_UNKNOWN_OK_ARG	0x0010
#define PAM_USE_FPASS_ARG	0x0040
#define PAM_TRY_FPASS_ARG	0x0080
#define PAM_SERVICE_TYPE_TELNET	     (1 << 0)
#define PAM_SERVICE_TYPE_SSH	     (1 << 1)
#define PAM_SERVICE_TYPE_WEB  	     (1 << 2)
#define PAM_SERVICE_TYPE_RPC_API	 (1 << 3)
/* Useful macros */
#define  x_strdup(s)  ( (s) ? strdup(s):NULL )

/* The name of the module we are compiling */
#ifndef MODULE_NAME
#define MODULE_NAME	"pam_ctc"
#endif							/* MODULE_NAME */

#endif							/* _PAM_CTC_H */
