#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "split.h"

/*******************************************************************************
 * Name:    split_tokens
 * Purpose: split tokens (we only support 128 tokens)
 * Input:  
 *   pszBuf: string buffer
 *   nLen: string length
 *   nMaxToks: limit token numbers
 *   pszDelimiters: delimiters
 *   pfCheck: validate function. return 0 for success, others for fail
 * Output: 
 *   pnToks: token numbers
 *   ppTokArr: token array
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int 
split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
        char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr)
{
    char *pTmp = NULL;
    char *pTok = NULL;
    char **ppTokArr = NULL;
    char *pSavePtr = NULL;
    unsigned long nArrSize = nMaxToks + 1;
    unsigned long nCurTok = 0;

    /* validate params */
    if (NULL == pszBuf || nLen <= 0 
            || nMaxToks <= 0 || nMaxToks > 128
            || NULL == pszDelimiters || NULL == pnToks
            || NULL == pppTokArr)
    {
        return -1;
    }

    /* getting string copy */
    pTmp = strdup(pszBuf);
    if (NULL == pTmp)
    {
        errno = ENOMEM;
        return -1;
    }

    /* calloc token array */
    ppTokArr = calloc(sizeof(char *), nArrSize);
    if (NULL == ppTokArr)
    {
        errno = ENOMEM;
        goto error_out;
    }

    pTok = strtok_r(pTmp, pszDelimiters, &pSavePtr);
    while (NULL != pTok)
    {
        nCurTok++;
        if (nCurTok > nMaxToks)
        {
            errno = EINVAL;
            goto error_out;
        }

        /* save current token */
        ppTokArr[nCurTok - 1] = strdup(pTok);
        if (NULL == ppTokArr[nCurTok - 1])
        {
            errno = ENOMEM;
            goto error_out;
        }

        /* process next token */
        pTok = strtok_r(NULL, pszDelimiters, &pSavePtr);
    }

    /* success */
    *pnToks = nCurTok;
    *pppTokArr = ppTokArr;

    /* free memory */
    if (NULL != pTmp)
    {
        free(pTmp);
    }
    return 0;

error_out:
    if (NULL != pTmp)
    {
        free(pTmp);
    }

    free_tokens(&ppTokArr);
    return -1;
}

/*******************************************************************************
 * Name:    free_tokens
 * Purpose: free split result
 * Input:  
 *   pppTokArr: token array 
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int free_tokens(char ***pppTokArr)
{
    char **ppTokArr = NULL;
    int i = 0;

    if (NULL == pppTokArr || NULL == *pppTokArr)
    {
        return -1;
    }
    ppTokArr = *pppTokArr;

    i = 0;
    while (1)
    {
        if (NULL == ppTokArr[i])
        {
            break;
        }

        free(ppTokArr[i]);
        i++;
    }

    free(ppTokArr);
    return 0;
}

int set_sock_nonblocking(int fd)
{
    int val;

    val = fcntl(fd, F_GETFL, 0);
    if (val < 0) {
        return -1;
    }
    if (val & O_NONBLOCK) {
        return 0;
    }
    val |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, val) == -1) {
        return -1;
    }
    return 0;
}

/* Read nbytes from fd and store into ptr. */
int readn (int fd, char *ptr, size_t nbytes)
{
    size_t nleft;
    size_t nread;

    nleft = nbytes;
    while (nleft > 0) 
    {
        nread = read (fd, ptr, nleft);
        if (nread < 0)
        {
            switch (errno)
            {
                case EINTR:
                case EAGAIN:
                case EINPROGRESS:
                    continue;
            }

            return nread;
        }
        else
            if (nread == 0) 
                break;

        nleft -= nread;
        ptr += nread;
    }

    return nbytes - nleft;
}  

/* Write nbytes from ptr to fd. */
int writen (int fd, char *ptr, size_t nbytes)
{
    size_t nleft;
    size_t nwritten;

    nleft = nbytes;
    while (nleft > 0) 
    {
        nwritten = write (fd, ptr, nleft);
        if (nwritten <= 0) 
        {
            switch (errno)
            {
                case EINTR:
                case EAGAIN:
                case EINPROGRESS:
                    continue;
            }

            return nwritten;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return nbytes - nleft;
}

int set_sock_recvbuf (int sock, size_t size)
{
    return setsockopt (sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size));
}

int set_sock_sendbuf (int sock, size_t size)
{
    return setsockopt (sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size));
}
