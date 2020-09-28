/****************************************************************************
* $Id$
*  split string as tokens
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-10-09 16:29
* Reason        : First Create.
****************************************************************************/

#ifndef _SPLIT_H_
#define _SPLIT_H_

#ifdef __cplusplus
extern "C" {
#endif

int split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
        char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);
int free_tokens(char ***pppTokArr);

int writen (int fd, char *ptr, size_t nbytes);
int readn (int fd, char *ptr, size_t nbytes);

int set_sock_nonblocking(int fd);
int set_sock_recvbuf (int sock, size_t size);
int set_sock_sendbuf (int sock, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* !_SPLIT_H_ */
