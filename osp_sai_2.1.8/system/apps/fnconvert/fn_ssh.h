/****************************************************************************
* $Id$
*  implement ssh client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2015-06-16 11:06
* Reason        : First Create.
****************************************************************************/

#ifndef _FN_SSH_H_
#define _FN_SSH_H_

#define M_SSH_DEFAULT_PORT              "22"
#define M_SSH_DEFAULT_PROTOCOL_VERSION  "2"

#define M_SSH_CIPHER_BUFSIZE            128
#define M_SSH_HMAC_BUFSIZE              128
#define M_SSH_CLIENT_CMD_BUFSIZE        128
#define M_SSH_OPTIONS_BUFSIZE           64

#define M_SSH_CLIENT_MAX_ARGS           64
#define M_SSH_MAX_CIPHER_ARG_NUM        32
#define M_SSH_MAX_HMAC_ARR_NUM          32

typedef enum {
	E_TYPE_PEM,			/* pem format */
	E_TYPE_DER,			/* der format */
	E_TYPE_DERHEX,		/* der-hex format */
	//E_TYPE_SSH1,		/* SSH1 format */
	E_TYPE_SSH2,		/* SSH2 format */
	E_TYPE_MAX,
} key_type_e;

int ssh_login(char *argv[], int argc);
int do_rsa_convert(const char *pszMsg, const char *pszPath, key_type_e otype);

#endif /* !_FN_SSH_H_ */
