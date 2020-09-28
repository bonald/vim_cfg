/****************************************************************************
* $Id$
*  base64 for URL encoding
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-11-07 21:55
* Reason        : First Create.
****************************************************************************/

#ifndef _SAL_BASE64_H_
#define _SAL_BASE64_H_

#ifdef __cplusplus
extern "C"
{
#endif

int
sal_b64_ntop(unsigned char const *src, size_t srclength, char *target, size_t targsize);

int
sal_b64_pton(char const *src, unsigned char *target, size_t targsize);

#ifdef __cplusplus
}
#endif

#endif /* !_SAL_BASE64_H_ */
