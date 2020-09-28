/****************************************************************************
* $Id$
*  some util function
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-30 11:11
* Reason        : First Create.
****************************************************************************/

#ifndef _SAL_UTIL_H_
#define _SAL_UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

int sal_cmd_exec(char *pszRetBuf, size_t nBufSize, const char *pszCmdFmt, ...);
int sal_cmd_exec_file(char *pszDestFile, const char *pszCmdFmt, ...);
int sal_cmd_get_token(char *pszBuf, char *pszToken, char *pszValue, size_t nValueSize);

#ifdef __cplusplus
}
#endif

#endif /* !_SAL_UTIL_H_ */
