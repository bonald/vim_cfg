
#include <openssl/evp.h>
#include <netdb.h>
#include <sys/vfs.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#include "proto.h"
#include "param_check.h"
#include "cdb_const.h"
#include "lib_fs.h"
#include "glb_if_define.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"

#ifndef M_FILENAME_MINSIZE
#define M_FILENAME_MINSIZE      1
#define M_FILENAME_MAXSIZE      48
#endif

/* format: address;address2;...;address16 */
int dhcsrvgrp_addr_iter(char *pszAddrs, dhcsrvgrp_iter_fn_t *pCB, void *pArg)
{
    gensock_t su;
    int ret = -1;
    char **ppArr = NULL;
    unsigned long nCnt = 0;
    char szBuf[DHCP_SRV_ADDRS_BUF_SIZE];
    char *p;
    int i;

    if (split_tokens(pszAddrs, strlen(pszAddrs), DHCP_SRV_ADDRS_NUM, 
            ";", &nCnt, &ppArr) != 0) {
        return -1;
    }
    
    assert (NULL != pCB);
    for (i = 0; i < nCnt; i++) {
        memset(&su, 0x00, sizeof(gensock_t));
        p = strchr(ppArr[i], ',');
        if (NULL != p) {
            snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s", p + 1);
            str2gensock(&su, szBuf);
            pCB(i, atoi(ppArr[i]), &su, pArg);
        } else {
            str2gensock(&su, ppArr[i]);
            pCB(i, 0, &su, pArg);
        }
    }
    
    ret = 0;    
    free_tokens(&ppArr);
    return ret;
}

/* format: vrfid,addr;vrfid2,addr2;vrfid3,addr3; */
int logging_server_addr_iter(const char *pszAddrs, addr_iter_fn_t *pCB, void *pArg, void *pArg2)
{
    int ret = -1;
    char **ppArr = NULL;
    unsigned long nCnt = 0;
    char szBuf[M_SRV_ADDR_BUF_SZ];
    char *p;
    int i;

    if (split_tokens((char *)pszAddrs, strlen(pszAddrs), 
            10, ";", &nCnt, &ppArr) != 0) {
        return -1;
    }
    
    assert (NULL != pCB);
    for (i = 0; i < nCnt; i++) {
        p = strchr(ppArr[i], ',');
        snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%s", p + 1);
        pCB(pArg, pArg2, atoi(ppArr[i]), szBuf, i);
    }
    
    ret = 0;    
    free_tokens(&ppArr);
    return ret;
}

char *gensock2str (gensock_t *pgs, char *buf, size_t len)
{
    const char *ret = NULL;

    if (pgs->sa.sa_family == AF_INET) {
        ret = inet_ntop (AF_INET, &pgs->sin.sin_addr, buf, len);
    } else if (pgs->sa.sa_family == AF_INET6) {
        ret = inet_ntop (AF_INET6, &pgs->sin6.sin6_addr, buf, len);
    }
    return ((NULL != ret) ? buf : NULL);
}

int str2gensock(gensock_t *pgs, const char *pszAddr)
{
    int ret;

    memset(pgs, 0x00, sizeof(gensock_t));
    ret = inet_pton (AF_INET, pszAddr, &pgs->sin.sin_addr);
    if (ret > 0) {
        pgs->sin.sin_family = AF_INET;
        //pgs->sin.sin_len = sizeof(struct sockaddr_in);
        return 0;
    }
    
    ret = inet_pton (AF_INET6, pszAddr, &pgs->sin6.sin6_addr);
    if (ret > 0) {
        pgs->sin6.sin6_family = AF_INET6;
        //pgs->sin6.sin6_len = sizeof(struct sockaddr_in6);
        return 0;
    }

    return -1;
}

int gensock_get_family(gensock_t *pgs)
{
    return pgs->sin.sin_family;
}

void *gensock_get_addr(gensock_t *pgs)
{
    if (AF_INET == pgs->sin.sin_family) {
        return &pgs->sin.sin_addr;
    }
    return &pgs->sin6.sin6_addr;
}

int gensock_set_port(gensock_t *pgs, u_int16_t port)
{
    if (AF_INET == pgs->sin.sin_family) {
        pgs->sin.sin_port = port;
    } else {
        pgs->sin6.sin6_port = port;
    }
    return 0;
}

u_int16_t gensock_get_port(gensock_t *pgs) 
{
    if (AF_INET == pgs->sin.sin_family) {
        return pgs->sin.sin_port;
    } 

    return pgs->sin6.sin6_port;
}

int syscmd_file_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}

int check_ipv6(const char *hostip)
{
    int ret;
    struct sal_in6_addr addr;

    if (NULL == hostip)
    {
        return -1;
    }

    ret = sal_inet_pton (AF_INET6, hostip, &addr);    
    if (!ret)     
    {         
        return -1;
    }                

    return 0;
}


/*******************************************************************************
 * Name:    check_hostname
 * Purpose: check hostname
 *   Only support [0-9A-Za-z.-_]
 * Input:  pszHostname: host name string
 * Output: N/A
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: N/A
 ******************************************************************************/
int check_hostname(const char *pszHostname)
{
    int nLen = 0;
    char *p = NULL;
    
    if (NULL == pszHostname)
    {
        return -1;
    }

    /* check the string length */
    nLen = sal_strlen(pszHostname);
    if ((nLen < M_HOSTNAME_MINSIZE) || (nLen > M_HOSTNAME_MAXSIZE))
    {
        return -1;
    }

    /*check the first character*/    
    if(!sal_char_isalpha(*pszHostname))
    {
        return -1;
    }
    
    /* check the chars */
    p = (char *)(pszHostname + 1);
    while ('\0' != *p)
    {
        if ((sal_char_isdigit(*p)) 
            || (sal_char_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }

    /*check the last character*/
    p--;
    if(!(sal_char_isdigit(*p) || sal_char_isalpha(*p)))
    {
        return -1;
    }
    
    return 0;
}


/*******************************************************************************
 * Name:    check_ipaddr
 * Purpose: checking p string validity
 *   Only support [0-9.]
 * Input:  ipaddr : ip address
 * Output: N/A
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: N/A
 ******************************************************************************/
int check_ip(const char *ipaddr)
{
    int nLen = 0;
    char *p = NULL;
    
    if (NULL == ipaddr)
    {
        return -1;
    }

    /* check the string length */
    nLen = sal_strlen(ipaddr);
    if (nLen < M_HOSTNAME_MINSIZE || nLen > M_HOSTNAME_MAXSIZE)
    {
        return -1;
    }

    /* check the chars */
    p = (char *)ipaddr;
    while ('\0' != *p)
    {
        if ((sal_char_isdigit(*p)) 
            || ('.' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }
    
    return 0;
}


/*******************************************************************************
 * Name:    check_hostip
 * Purpose: checking hostname or ip string validity
 *   Only support [0-9A-Za-z.-_]
 * Input:  pszHostname: host name string
 * Output: N/A
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: N/A
 ******************************************************************************/
int check_hostip(const char *pszHostname)
{
    if (!check_hostname(pszHostname))
    {
        return 0;
    }
    if (!check_ip(pszHostname))
    {
        return 0;
    }

    if (!check_ipv6(pszHostname))
    {
        return 0;
    }
    
    return -1;
}

/*******************************************************************************
 * Name:    check_username
 * Purpose: check username
 *   it length range is [1, M_MAX_USERNAME_LEN),
 *   Only support [0-9A-Za-z.-_], and start with alpha
 * Input: 
 *   username: username string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int32
check_username(char *username)
{
    char *p = username;

    if (NULL == p || !sal_isalpha(*p))
    {
        return -1;
    }

    if (sal_strlen(username) >= M_MAX_USERNAME_LEN)
    {
        return -1;
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }

        return -1;
    }

    return 0;
}

/*******************************************************************************
 * Name:    check_password
 * Purpose: check password string, 
 *    limit it length not overflow, and it must being printable
 * Input: 
 *   pin: password string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int32
check_password(char *pin)
{
    char *p = pin;
    
    if (NULL == pin)
    {
        return -1;
    }
    
    if (sal_strlen(pin) >= M_MAX_PASSWORD_LEN)
    {
        return -1;
    }

    while ('\0' != *p)
    {
        if (!sal_isprint(*p)) 
        {
            return -1;
        }

        p++;
    }
    return 0;
}

/* check XML's in PM
    <PTYPE name="PASSWORD" 
        pattern="^([0-9A-Za-z._-])+$"
 */
int32
check_user_password(char *pin)
{
    char              *p = pin;
    int               password_max_len = M_MAX_PASSWORD_LEN;
    tbl_sys_global_t  *pGlobal = tbl_sys_global_get_sys_global();
    char              passwd[PASSWD_SIZE];
    char              *q = passwd;

    if (NULL == pin)
    {
        return -1;
    }

    if (sal_strlen(pin) >= PASSWD_SIZE)
    {  
        return -1;
    }

    while ('\0' != *p)
    {
        if ('\\' != *p)
        {
            *q = *p;
            p++;
            q++;
        }
        else
        {
            p++;
        }
    }
    *q = '\0';

    /* modified by liwh for bug 44024, 2017-05-15 */
    if (pGlobal->startup_done != 0 && sal_strlen(passwd) >= password_max_len)
    /* liwh end */
    {
        return -1;
    }

    p = pin;
    while ('\0' != *p)
    {
        if (('`' == *p)
            || ('\'' == *p)
            || ('$' == *p)
            || ('%' == *p))
        {
            return -1;
        }
        
        p++;
    }

    sal_memset(pin, 0, sal_strlen(pin));
    sal_memcpy(pin, passwd, sal_strlen(passwd));

    return 0;
}

/*******************************************************************************
 * Name:    check_intnumber
 * Purpose: checking integer number, range is [min, max]
 * Input:  
 *    pszNumber: number string
 *    min: min value
 *    max: max value
 * Output:
 *    nRet: number value if valid
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: N/A
 ******************************************************************************/
int32
check_intnumber(const char *pszNumber, int min, int max, int *nRet)
{
    char *p = NULL;
    int32 num = 0;
    
    if (NULL == pszNumber || min > max || NULL == nRet)
    {
        return -1;
    }

    p = (char*)pszNumber;
    if ('+' == *p || '-' == *p)
    {
        p++;
        if ('\0' == *p)
        {
            return -1;
        }
    }    
    
    if (sal_strspn(p, "0123456789") != sal_strlen(p))
    {
        return -1;
    }

    num = sal_atoi(pszNumber);
    if (num < min || num > max)
    {
        return -1;
    }
    
    *nRet = num;
    return 0;
}

/*******************************************************************************
 * Name:    check_tcpudp_portnumber
 * Purpose: checking port number for tcp / udp, range is [0, 65535]
 * Input:  pszPort: port number string
 * Output: N/A
 * Return: 
 *   success: port number
 *   fail: -1
 * Note: N/A
 ******************************************************************************/
int32
check_tcpudp_portnumber(const char *pszPort)
{
    int nRet = 0;
    
    if (check_intnumber(pszPort, 0, 65535, &nRet) != 0)
    {
        return -1;
    }

    return nRet;
}

/*******************************************************************************
 * Name:    check_disk_free space
 * Purpose: check disk free space
 * Input: 
 *   pszName: key name string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
 int64_t
 check_disk_free_space(char* name)
 {
     struct statfs st;
     int64_t llSize = 0ll;
     
     if (statfs(name, &st) != 0)
     {
         return -1;
     }

     llSize = (u_int64_t)st.f_bavail * st.f_bsize;
     return llSize;    
 }
 /*end add */

/*******************************************************************************
 * Name:    check_space_available
 * Purpose: check disk is available for copy or not
 * Input: 
 *   pszName: key name string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
 int
 check_space_available(char * src_filename, char * dst_filename, int64_t size)
 {
    int64_t llsize = 0ll;
    int64_t rsize = 0ll;
    FILE* fp = NULL;
    char *dst_dir = NULL;
    char *second_subarea = NULL;	
    char *p_save_ptr = NULL;
    char *p_tmp = NULL;
    int ret = 0;

    /* getting dst_filename string copy */
    /*modified by liul for 17954, 2012-02-16. Fix MIB copy invalid problem*/
    p_tmp = strdup(dst_filename);
    if (NULL == p_tmp)
    {
        errno = ENOMEM;
        ret =  -1;
        goto out;
    }
    
    /* remove /mnt from string */
    dst_dir = strtok_r(p_tmp, "/", &p_save_ptr);
    if(NULL == dst_dir)
    {
        ret = -1;
        goto out;
    }

    if(!strncmp(dst_dir, "tmp", 3))    /* Added by Percy Wang for bug 15315, 2011-08-11 */
    {
        ret = 0;
        goto out;
    }

    /* get the disk name */
    dst_dir = strtok_r(NULL, "/", &p_save_ptr);
    if(NULL == dst_dir)
    {
        ret = -1;
        goto out;
    }
    if (size == 0 && syscmd_file_exist(dst_filename))   /* Added by Percy Wang for bug 15959, 2011-08-18 */
    {
        fp = fopen(dst_filename, "r" );
        if (fp == NULL) 
        {
            ret = -1; 
            goto out;
        }
  
        fseek(fp, 0L, SEEK_END ); 
        size = (int64_t)ftell(fp);
        fclose(fp);
        fp = NULL;
    }
    if(!strncmp(dst_dir, "flash", 5))
    {
        /* Modified by liuht to check different subareas depand on destination for bug25870, 2013-11-22 */
        second_subarea = strtok_r(NULL, "/", &p_save_ptr);
        if(NULL == second_subarea)
        {
            ret = -1;
            goto out;
        }
        if(!strncmp(second_subarea, "boot", 4))
        {
            llsize = check_disk_free_space("/mnt/flash/boot");
            /* reserve 1M space for important manipulation */
            rsize = 1*1024*1024;
        }
        else
        {
            llsize = check_disk_free_space("/mnt/flash");
            /* reserve 50M space for important manipulation */
            rsize = 50*1024*1024;
        }
    }
    else if(!strncmp(dst_dir, "udisk", 5))
    {
        llsize = check_disk_free_space("/mnt/udisk");
    }
    else if(!strncmp(dst_dir, "cf", 2))
    {
        llsize = check_disk_free_space("/mnt/cf");
    }
    else
    {
        ret = -1;
        goto out;
    }
    
    if (llsize == -1)
    {
        ret = -1;       
        goto out;
    }
    
    fp = fopen(src_filename, "r" );
    if (fp == NULL) 
    {
        ret = -2; 
        goto out;
    }
      
    fseek(fp, 0L, SEEK_END );
    ftell(fp);

    /* modified by wus for bug15285, 2011-07-21 */
    if(!strncmp(dst_dir, "udisk", 5))
    {
      /*add by guhg for bug 33833,2015-07-17*/
        if((llsize + size) <= (int64_t)ftell(fp))
        {
            ret = -3;
            goto out;
        }

        ret =  0;
        goto out;
    }
    /* we left some space in flash for important manipulation */
    /* Modified by liuht for bug 33608, 2015-06-02 */
    if ((llsize + size) < (int64_t)ftell(fp)
        || (llsize + size - (int64_t)ftell(fp)) < rsize)
    {
        ret = -3;
        goto out;
    }

    /*modified by xgu for bug 15839, 2011-08-10, fix memory leak problem*/
out:
    if (p_tmp)
    {
        /*modified by liul for 17954, 2012-02-16. Fix MIB copy invalid problem*/
        free(p_tmp);
    }
    if (fp)
    {
        fclose(fp); 
    }
    return ret;
 }


/* include ipv6 and ipv4 and hostname */
int check_ipaddress(char *p)
{
    int32 len;
    int32 error;
    struct sal_addrinfo hints, *res, *res_save;
    
    if (NULL == p || '\0' == *p)
    {
        return -1;
    }
    len = sal_strlen(p);

    if (len >= MAXHOSTNAMELEN)
    {
        return -1;
    }

    sal_memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    error = sal_getaddrinfo(p, NULL, &hints, &res_save);
    if (error)
    {
        return -1;
    }
    res = res_save;
    if (!res)
    {
        return -1;
    }

    sal_freeaddrinfo(res_save);
    return 0;
}

/*******************************************************************************
 * Name:    check_filename
 * Purpose: checking filename string validity
 *   Only support [0-9A-Za-z.-_()]
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
    { /* modified by liuyang 2012-4-18*/
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

#define SCALE_LENGTH (sizeof(units)/sizeof(units[0]))
/* These three arrays MUST be in sync!  XXX make a struct */
static unit_type units[] = { NONE, KILO, MEGA, GIGA, TERA, PETA, EXA };
static char scale_chars[] = "BKMGTPE";
static long long scale_factors[] = {
    1LL,
    1024LL,
    1024LL*1024,
    1024LL*1024*1024,
    1024LL*1024*1024*1024,
    1024LL*1024*1024*1024*1024,
    1024LL*1024*1024*1024*1024*1024,
};

/*******************************************************************************
 * Name:    fmt_scaled
 * Purpose: 
 *   Format the given "number" into human-readable form in "result".
 *   Result must point to an allocated buffer of length FMT_SCALED_STRSIZE.
 * Input:  
 *   number: number value
 *   result: convert result buffer
 * Output: N/A
 *   result: convert result
 * Return: 
 *   success: 0
 *   failed: -1
 * Note: N/A
 ******************************************************************************/
int fmt_scaled(long long number, char *result)
{
    long long abval, fract = 0;
    unsigned int i;
    unit_type unit = NONE;

    abval = (number < 0LL) ? -number : number;    /* no long long_abs yet */

    /* Not every negative long long has a positive representation.
     * Also check for numbers that are just too darned big to format
     */
    if (abval < 0 || abval / 1024 >= scale_factors[SCALE_LENGTH-1]) {
        errno = ERANGE;
        return -1;
    }

    /* scale whole part; get unscaled fraction */
    for (i = 0; i < SCALE_LENGTH; i++) {
        if (abval/1024 < scale_factors[i]) {
            unit = units[i];
            fract = (i == 0) ? 0 : abval % scale_factors[i];
            number /= scale_factors[i];
            if (i > 0)
                fract /= scale_factors[i - 1];
            break;
        }
    }

    fract = (10 * fract + 512) / 1024;
    /* if the result would be >= 10, round main number */
    if (fract == 10) {
        if (number >= 0)
            number++;
        else
            number--;
        fract = 0;
    }

    if (number == 0)
        strncpy(result, "0B", FMT_SCALED_STRSIZE);
    else if (unit == NONE || number >= 100 || number <= -100) {
        if (fract >= 5) {
            if (number >= 0)
                number++;
            else
                number--;
        }
        (void)snprintf(result, FMT_SCALED_STRSIZE, "%lld%c",
            number, scale_chars[unit]);
    } else
        (void)snprintf(result, FMT_SCALED_STRSIZE, "%lld.%1lld%c",
            number, fract, scale_chars[unit]);

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

int
check_macaddr(char *pszMac)
{
    int nLen = 0;
    int idx = 0;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nRet = -1;
    
    if (NULL == pszMac)
    {
        return -1;
    }
    
    nLen = strlen(pszMac);
    if (strspn(pszMac, "0123456789AaBbCcDdEeFf.") != nLen)
    {
        return -1;
    }

    if (split_tokens(pszMac, nLen, 3, ".", &nToks, &ppTokArr) != 0)
    {
        return -1;
    }

    if (3 == nToks)
    {
        for (idx = 0; idx < nToks; idx++)
        {
            if (strlen(ppTokArr[idx]) > 4)
            {
                break;
            }       
        }

        if (idx >= nToks)
        {
            nRet = 0;
        }
    }

    free_tokens(&ppTokArr);
    return nRet;
}

/*filter the kernel name, refter to netlink_check_unsupport_link*/
char *
check_prefix_dup(char *prifix)
{
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_ETH, sal_strlen(GLB_KERNEL_PORT_ETH)))
    {
        return GLB_KERNEL_PORT_ETH;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_LO, sal_strlen(GLB_KERNEL_PORT_LO)))
    {
        return GLB_KERNEL_PORT_LO;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_GRE0, sal_strlen(GLB_KERNEL_PORT_GRE0)))
    {
        return GLB_KERNEL_PORT_GRE0;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_IP6TNL0, sal_strlen(GLB_KERNEL_PORT_IP6TNL0)))
    {
        return GLB_KERNEL_PORT_IP6TNL0;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_SIT0, sal_strlen(GLB_KERNEL_PORT_SIT0)))
    {
        return GLB_KERNEL_PORT_SIT0;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_TUNL0, sal_strlen(GLB_KERNEL_PORT_TUNL0)))
    {
        return GLB_KERNEL_PORT_TUNL0;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_PIMREG, sal_strlen(GLB_KERNEL_PORT_PIMREG)))
    {
        return GLB_KERNEL_PORT_PIMREG;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_PIM6REG, sal_strlen(GLB_KERNEL_PORT_PIM6REG)))
    {
        return GLB_KERNEL_PORT_PIM6REG;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_DUMMY, sal_strlen(GLB_KERNEL_PORT_DUMMY)))
    {
        return GLB_KERNEL_PORT_DUMMY;
    }
    if (!sal_strncmp(prifix, GLB_KERNEL_PORT_STACKING, sal_strlen(GLB_KERNEL_PORT_STACKING)))
    {
        return GLB_KERNEL_PORT_STACKING;
    }
    return NULL;
}

int
check_prefix_string(char *prifix)
{
    char *p = prifix;

    if (NULL == p)
    {
        return -1;
    }

    if (sal_strlen(prifix) >= GLB_PHY_PORT_PREFIX_SZ)
    {
        return -1;
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ( sal_isupper(*p) || sal_islower(*p))
        {
            p++;
            continue;
        }
        return -1;
    }
    return 0;
}

int
check_prefix_full_string(char *prifix)
{
    char *p = prifix;

    if (NULL == p)
    {
        return -1;
    }

    if (sal_strlen(prifix) >= GLB_PHY_PORT_FULL_PREFIX_SZ)
    {
        return -1;
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ( sal_isupper(*p) || sal_islower(*p))
        {
            p++;
            continue;
        }
        return -1;
    }
    return 0;
}

