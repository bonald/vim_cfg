/****************************************************************************
 * param_check.c: Common functions to check parameter
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Alexander Liu
 * Date         : 2006-11-27
 * Reason       : First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <openssl/evp.h>
#include <sys/statfs.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/wait.h>

#include "param_check.h"
/* filename buffer size */
#define M_FILENAME_MAXSIZE      48
#define M_FILENAME_MINSIZE      1

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
#define CTC_IH_NMLEN       32    /* Image Name Length  */
#if (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
#define CTC_IH_MAGIC       0xd00dfeed    /*FDT Image Magic Number  */
#else
#define CTC_IH_MAGIC       0x27051956    /* Image Magic Number  */
#endif
#ifndef O_BINARY
#define O_BINARY    0
#endif

/* crc 32 table */
const u_int32_t crc_table[256] = 
{
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

typedef struct ctc_image_header 
{
    uint32_t    ih_magic;   /* Image Header Magic Number */
    uint32_t    ih_hcrc;    /* Image Header CRC Checksum */
    uint32_t    ih_time;    /* Image Creation Timestamp */
    uint32_t    ih_size;    /* Image Data Size  */
    uint32_t    ih_load;    /* Data     Load  Address  */
    uint32_t    ih_ep;      /* Entry Point Address  */
    uint32_t    ih_dcrc;    /* Image Data CRC Checksum */
    uint8_t     ih_os;      /* Operating System  */
    uint8_t     ih_arch;    /* CPU architecture  */
    uint8_t     ih_type;    /* Image Type   */
    uint8_t     ih_comp;    /* Compression Type  */
    uint8_t     ih_name[CTC_IH_NMLEN];    /* Image Name  */
} ctc_image_header_t;

/*******************************************************************************
 * Name:    check_filename
 * Purpose: checking filename string validity
 *   Only support [0-9A-Za-z.-_], add support[()].
 * Input:  pszFilename: file name string
 * Output: N/A
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: Bug 16587. MAIPU OEM information. jqiu 2011-10-12 add support ( ).
 ******************************************************************************/
int check_filename(const char *pszFilename)
{
    int nLen = 0;
    char *p = NULL;
    
    if (NULL == pszFilename)
    {
        return -1;
    }

    /* check the string length */
    nLen = strlen(pszFilename);
    if (nLen < M_FILENAME_MINSIZE || nLen >= M_FILENAME_MAXSIZE)
    { /*modified by liuyang 2012-4-18*/
        return -2;
    }

    /* check the chars */
    p = (char *)pszFilename;

    /* for bug 28145 added by liuyang 2014-4-17*/
    if ((1 == nLen && '.' == *p)
        || (2 == nLen && '.' == *p && '.' == p[1]))
    {
        return 0;
    }
    
    /*for bug 27538 added by liuyang 2014-3-27*/
    if ('.' == *p)
    {
        return -1;
    }
        
    while ('\0' != *p)
    {
        if ((isdigit(*p)) 
            || (isupper(*p))
            || (islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || ('(' == *p)
            || (')' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }
    
    return 0;
}

#ifndef strtok_r
#define strtok_r strtok_r
#endif
/*******************************************************************************
 * Name:    split_tokens
 * Purpose: split tokens (we only support 100 tokens)
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
#ifdef _CENTEC_   /* Zongyi add for snmp tag split */
        || nMaxToks <= 0 || nMaxToks > 128
#else        
        || nMaxToks <= 0 || nMaxToks > 100
#endif        
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
    ppTokArr = calloc(1, sizeof(char *) * nArrSize);
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

int signal_app(const char *pszPidFile, int signal)
{
#define M_PIDBUF_SIZE 32
    char szBuf[M_PIDBUF_SIZE + 1];
    FILE *pFile = NULL;
    pid_t nPid = 0;

    if (NULL == pszPidFile)
    {
        return -1;
    }
    
    /* open & read pid file */
    pFile = fopen(pszPidFile, "r");
    if (NULL == pFile)
    {
        return -1;
    }
    memset(szBuf, 0x00, M_PIDBUF_SIZE);
    if (NULL == fgets(szBuf, M_PIDBUF_SIZE, pFile))
    {
        /* may be daemon error */
        fclose(pFile);
        return -1;
    }
    
    fclose(pFile);
    if ((nPid = atoi(szBuf)) <= 0)
    {
        return -1;
    }

    /* test daemon */
    if (kill(nPid, 0) != 0)
    {
        /* daemon error */
        return -1;
    }

    /* send signal to app  */
    if (kill(nPid, signal) != 0)
    {
        return -1;
    }

    return 0;
}

u_int32_t
gen_crc32(u_int32_t crc, char *buf, size_t len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;

}

int
check_image_file(char *filename, char *errmsg, size_t size, int bCRC)
{
    int ifd = -1;
    struct stat sbuf;
    ctc_image_header_t header;
    ctc_image_header_t *hdr = &header;
    unsigned char *ptr = NULL;
    int ret = -1;
    int len = 0;
    char *data = NULL;
    uint32_t checksum = 0l;
    
    if (NULL == filename)
    {
        return -1;
    }

    if ((ifd = open(filename, O_RDONLY|O_BINARY)) < 0)
    {
        goto error_out;
    }

    if (fstat(ifd, &sbuf) < 0)
    {
        goto error_out;
    }

    if ((unsigned)sbuf.st_size < sizeof(ctc_image_header_t))
    {
        goto error_out;
    }

    ptr = (unsigned char *)mmap(0, sbuf.st_size,
                                PROT_READ, MAP_SHARED, ifd, 0);
    if ((caddr_t)-1 == (caddr_t)ptr)
    {
        goto error_out;
    }

    /*
     * create copy of header so that we can blank out the
     * checksum field for checking - this can't be done
     * on the PROT_READ mapped data.
     */
    memcpy(hdr, ptr, sizeof(ctc_image_header_t));
    if (ntohl(hdr->ih_magic) != CTC_IH_MAGIC) 
    {
        goto error_out;
    }

    if (ntohl(hdr->ih_size) + sizeof(ctc_image_header_t) != sbuf.st_size)
    {
        goto error_out;
    }

    data = (char *)hdr;
    len  = sizeof(ctc_image_header_t);
    checksum = ntohl(hdr->ih_hcrc);
    hdr->ih_hcrc = ntohl(0);  /* clear for re-calculation */
    if (gen_crc32(0, data, len) != checksum) 
    {
        goto error_out;
    }

    if (bCRC != 0)
    {
        data = (char *)(ptr + sizeof(ctc_image_header_t));
        len  = sbuf.st_size - sizeof(ctc_image_header_t) ;
        if (gen_crc32(0, data, len) != ntohl(hdr->ih_dcrc)) 
        {
            goto error_out;
        }
    }
    ret = 0;

error_out:
    if (NULL != ptr)
    {
        munmap((void *)ptr, sbuf.st_size);
    }
    
    if (ifd >= 0)
    {
        close(ifd);
    }
    return ret;
}

