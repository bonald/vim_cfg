/****************************************************************************
* $Id$
*  some util function
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-30 11:14
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

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
#define _M_CMD_BUFSZ    10240
int sal_cmd_exec(char *pszRetBuf, size_t nBufSize, const char *pszCmdFmt, ...)
{
    va_list ap;
    char *pszCmd = NULL;
    FILE *fp = NULL;
    int nLeft;
    int nOffset;
    char *p;

    pszCmd = malloc(_M_CMD_BUFSZ);
    if (NULL == pszCmd) {
        fprintf(stderr, "%% sal_cmd_exec: out of memory\r\n");
        return -1;
    }

    va_start(ap, pszCmdFmt);
    vsnprintf(pszCmd, _M_CMD_BUFSZ, pszCmdFmt, ap);
    va_end(ap);

    fp = popen(pszCmd, "r");  
    if (NULL == fp) {
        free(pszCmd);
        return -1;
    }

    if (pszRetBuf && nBufSize > 1) {
        pszRetBuf[0] = '\0';
        nLeft = nBufSize;
        nOffset = 0;
        for (;;) {
            p = fgets(pszRetBuf + nOffset, nBufSize - nOffset, fp);
            if (NULL == p) {
                break;
            }

            nLeft -= strlen(p);
            if (nLeft <= 1) {
                break;
            }
        }

        /* delete this tailer \r\n */
        if (nLeft < nBufSize) {
            p = pszRetBuf + (strlen(pszRetBuf) - 1);
            if ('\n' == *p) {
                *p = '\0';
                p--;
            }
            if (p >= pszRetBuf && '\r' == *p) {
                *p = '\0';
            }
        }
    }

    free(pszCmd);
    return pclose(fp);
}

int sal_cmd_exec_file(char *pszDestFile, const char *pszCmdFmt, ...)
{
    va_list ap;
    char *pszCmd = NULL;
    char szBuf[1024];
    FILE *fp = NULL;
    FILE *fpDest = NULL;

    if ((fpDest = fopen(pszDestFile, "w")) == NULL) {
        fprintf(stderr, "%% sal_cmd_exec: can't open file: %s\r\n", strerror(errno));
        goto err_out;
    }

    pszCmd = malloc(_M_CMD_BUFSZ);
    if (NULL == pszCmd) {
        fprintf(stderr, "%% sal_cmd_exec: out of memory\r\n");
        goto err_out;
    }
    
    va_start(ap, pszCmdFmt);
    vsnprintf(pszCmd, _M_CMD_BUFSZ, pszCmdFmt, ap);
    va_end(ap);

    fp = popen(pszCmd, "r");  
    if (NULL == fp) {
        goto err_out;
    }

    while (fgets(szBuf, 1024, fp) != NULL) {
        fwrite(szBuf, strlen(szBuf), 1, fpDest);
    }

err_out:
    if (fpDest) {
        fclose(fpDest);
    }
    if (pszCmd) {
        free(pszCmd);
    }

    return fp ? pclose(fp) : -1;
}

int sal_cmd_get_token(char *pszBuf, char *pszToken, char *pszValue, size_t nValueSize)
{
    char szToken[64];
    int nTokenLen;
    char *p;

    if ((strlen(pszToken) + 2) >= 64) {
        assert(0);
        return -1;
    }
    
    nTokenLen = snprintf(szToken, 64, "/%s=", pszToken);
    
    p = strstr(pszBuf, szToken);
    if (NULL == p) {
        pszValue[0] = '\0';
        return -1;
    }

    if ('\0' == *(p + nTokenLen) || '/' == *(p + nTokenLen)) {
        /* emtpy value */
        pszValue[0] = '\0';
        return 0;
    }

    snprintf(pszValue, nValueSize, "%s", p + nTokenLen);
    p =  strchr(pszValue, '/');
    if (NULL != p && '/' == *p) {
        *p = '\0';
    }
    
    return 0;
}

