/****************************************************************************
 * syscmd_readline.c:    
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Xiaobo Yan
 * Date         :       2007-1-22
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include <proto.h>
#include <limits.h>
#include <sys/statfs.h>
#include <assert.h>
#include <dirent.h>
#include <a.out.h>
#include <signal.h>
#include <termios.h>
#include <assert.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include "param_check.h"
#include "lib_fs.h"
#include "ctclib_syslimit.h"
#include "genlog.h"
#include "lib_syslimit.h"
#include "glb_tempfile_define.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
#define FLAG_ISSET(V,F)        (((V) & (F)) == (F))

#define CLI_RPP_PASSWORD_LEN      128     /* max length, not counting NUL */

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
/* Modified by liuht for second subarea for bug25870, 2013-11-22 */
rootdir_t actual_root_dir[] = {
#if HAVE_CF /*modified by weij*/
    {E_DISK_FILE_CFCARD, FULL_NAME_CFCARD, "cf:"},
#endif
    {E_DISK_FILE_FLASH,  FULL_NAME_FLASH,  "flash:"},
    /* modified by liuht for bug 29099, 2014-06-23 */
#ifndef _CTC_NO_PARTITION2
#ifndef _CTC_LOONGSON_2F_
    /* for bug 28631 modified by liuyang 2014-5-29*/
    {E_DISK_BOOT_FLASH,  FULL_NAME_FLASH_BOOT,   "flash:/boot"},
#endif    
#endif
    {E_DISK_FILE_UDISK,  FULL_NAME_UDISK,   "udisk:"},
};

int cli_readpassphrase(const char *, char *, size_t, int, int);

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
static int dir_mnt_valid = 0;
static volatile sig_atomic_t signo = 0;
static volatile sig_atomic_t bstop = 0;
static void handler(int);

system_file_t g_system_file_arr[] =
{
    {STARTUP_CONFIG_FILE_PATH, "startup configuration file"},
    {M_CUR_BOOT_IMAGE_LINK_FULLNAME, "boot configuration file"},
    {M_BOOT_IMAGE_PATH, "boot image directory"},
    {M_SYSTEM_LOG_FILE, "system log file"},
    {DHCPSNOOPING_SAVE_BINDING_FILE, "DHCP snooping binding database file"},
#ifdef HAVE_IPV6
    {DHCPSNOOPING_SAVE_BINDING_FILE6, "DHCPv6 snooping binding database file"},
#endif
    {NULL, NULL}
};

/* for xmodem */
network_dir_t xmodem_root_dir =
{
    .name = "xmodem:",
    .usage = "xmodem:",
    .network_file = NETWORK_XMODEM,
    .ops = NULL
};

network_dir_t network_root_dir[] = {
    {
        "tftp://",
        "tftp://hostname_or_ip:port/file_name",
        NETWORK_TFTP,
        NULL
    },
    {
        "ftp://",
        "ftp://username:password@host:port/file_name",
        NETWORK_FTP,
        NULL
    },
    {
    	"scp://",
        "scp://username@host:/directory_name/file_name",
        NETWORK_SCP,
        NULL
    },
};

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
    pTmp = sal_strdup(pszBuf);
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
int
free_tokens(char ***pppTokArr)
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

/*******************************************************************************
 * Name:    lib_filesys_check_filename
 * Purpose: checking filename string validity
 *   Only support [0-9A-Za-z.-_], add support[()].
 * Input:  pszFilename: file name string
 * Output: N/A
 * Return: 
 *   success: 0
 *   fail: -1
 * Note: Bug 16587. MAIPU OEM information. jqiu 2011-10-12 add support ( ).
 ******************************************************************************/
int
lib_filesys_check_filename(const char *pszFilename)
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
        
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
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

int
lib_fs_is_file(char *filename)
{
    struct stat stat_buf;
    
    if (!sal_stat(filename, &stat_buf))
    {
        if (S_ISREG(stat_buf.st_mode))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int
lib_fs_is_directory(char *filename)
{
    struct stat stat_buf;
    
    if (!sal_stat(filename, &stat_buf))
    {
        if (sal_S_ISDIR(stat_buf.st_mode))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int
lib_fs_is_chardev(char *filename)
{
    struct stat stat_buf;
    
    if (!sal_stat(filename, &stat_buf))
    {
        if (S_ISCHR(stat_buf.st_mode))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int
lib_fs_is_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!sal_stat(filename, &stat_buf))
    {
        return TRUE;
    }
    
    return FALSE;
}

/*******************************************************************************
 * Name:    gen_validate_path
 * Purpose: validate full path
 * Input: 
 *   pszPath: path name 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_validate_path(char *pszPath)
{
    char **ppNameArr = NULL;
    unsigned long nCnt = 0;
    int i = 0;
    int nRet = 0;
    
    if (NULL == pszPath)
    {
        return -1;
    }

    if (sal_strlen(pszPath) > M_FULLPATH_MAX_LEN)
    {
        return -1;
    }

    if (NULL != sal_strstr(pszPath, "//"))
    {
        return -1;
    }

    if (split_tokens(pszPath, sal_strlen(pszPath), M_FULLPATH_DEPTH_MAX,
              "/", &nCnt, &ppNameArr) != 0)
    {
        return -1;
    }

    for (i = 0; i < nCnt; i++)
    {
        if (NULL == ppNameArr[i])
        {
            continue;
        }
        if (lib_filesys_check_filename(ppNameArr[i]) != 0)
        {
            nRet = -1;
            break;
        }
    }
    free_tokens(&ppNameArr);
    return nRet;
}

/*******************************************************************************
 * Name:    gen_path_getparents
 * Purpose: parse .. so we don't compromise security
 * Input: 
 *   name: path with .. and .
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
gen_path_getparents(char *name)
{
    int l, w;

    /* Four passes, as per RFC 1808 */
    /* a) remove ./ path segments */
    for (l = 0, w = 0; name[l] != '\0';) 
    {
        if (name[l] == '.' && name[l + 1] == '/' && (l == 0 || name[l - 1] == '/'))
        {
            l += 2;
        }
        else
        {
            name[w++] = name[l++];
        }
    }

    /* b) remove trailing . path, segment */
    if (w == 1 && name[0] == '.')
    {
        w--;
    }
    else if (w > 1 && name[w - 1] == '.' && name[w - 2] == '/')
    {
        w--;
    }
    name[w] = '\0';

    /* c) remove all xx/../ segments. (including leading ../ and /../) */
    l = 0;

    while (name[l] != '\0') 
    {
        if (name[l] == '.' && name[l + 1] == '.' && name[l + 2] == '/' &&
            (l == 0 || name[l - 1] == '/')) 
        {
            register int m = l + 3, n;
            
            l = l - 2;
            if (l >= 0) 
            {
                while (l >= 0 && name[l] != '/')
                {
                    l--;
                }
                l++;
            }
            else
            {
                l = 0;
            }
            n = l;
            while ((name[n] = name[m]))
            {
                (++n, ++m);
            }
        }
        else
        {
            ++l;
        }
    }

    /* d) remove trailing xx/.. segment. */
    if (l == 2 && name[0] == '.' && name[1] == '.')
    {
        name[0] = '\0';
    }
    else if (l > 2 && name[l - 1] == '.' && name[l - 2] == '.' && name[l - 3] == '/') 
    {
        l = l - 4;
        if (l >= 0) 
        {
            while (l >= 0 && name[l] != '/')
            {
                l--;
            }
            l++;
        }
        else
        {
            l = 0;
        }
        name[l] = '\0';
    }
    return;
}

/*******************************************************************************
 * Name:    gen_validate_relative_path
 * Purpose: validate relative path
 * Input: 
 *   pszFullName: path name 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: Created by Percy Wang 2007-12-21 
 ******************************************************************************/
int
gen_validate_relative_path(char *pszFullName)
{
    int nCnt = 0;    
    int nRet = 0;
    int nIdx = 0;
    rootdir_t *rdir = NULL;
    char *pszTmpName;
    int nValLen = 0;
    
    if (NULL == pszFullName)
    {
        return -1;
    }

    if (sal_strlen(pszFullName) > M_FULLPATH_MAX_LEN)
    {
        return -1;
    }

    pszTmpName = pszFullName;
    
    if (sal_strchr(pszFullName, ':') != NULL)
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            nValLen = sal_strlen(rdir->show_name);
            if (!sal_strncmp(pszFullName, rdir->show_name,
                             sal_strlen(rdir->show_name)))
            {
                if (!sal_strcmp(pszFullName, rdir->show_name))
                {
                    return 0;
                }
                else
                {
                    pszTmpName = pszFullName + nValLen;
                    break;
                }
                
            }
        }
   
    }
    
    nRet = gen_validate_path(pszTmpName);
    
    return nRet;
}

/*******************************************************************************
 * Name:    gen_fullname_to_showname
 * Purpose: parse fullname to showname
 * Input: 
 *   fullname: file name start with drive
 *   showname: showname
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_fullname_to_showname(char *fullname, char *showname)
{
    rootdir_t *rdir = NULL;
    char *full_char = fullname;
    char *show_char = showname;
    int nLen = 0;
    int nCnt = 0;
    int nIdx = 0;
    
    if (NULL == fullname || NULL == showname)
    {
        return -1;
    }

    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nLen = sal_strlen(rdir->real_name);
        if (!sal_strncmp(fullname, rdir->real_name, nLen))
        {
            full_char = nLen + full_char;
            nLen = sal_strlen(rdir->show_name);
            sal_snprintf(showname, nLen + 1, "%s", rdir->show_name);
            show_char += nLen;
            sal_snprintf(show_char, M_FULLPATH_MAX_LEN - nLen, "%s", full_char);
            return 0;
        }
    }
    return -1;
}

int
syscmd_check_actual_directory_exist(char *filename)
{
    int nIdx = 0;
    int nValLen = 0;
    int nCnt = sizeof(actual_root_dir)/sizeof(actual_root_dir[0]);
    rootdir_t  * rdir = NULL;

    for(nIdx = 0; nIdx < nCnt; nIdx ++)
    {
        rdir = & actual_root_dir[nIdx];
        nValLen = sal_strlen(filename);
        if (!sal_strncmp(filename, rdir->real_name, nValLen))
        {
            if (lib_fs_is_directory(filename))
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
    }
    return 0;
}

/*******************************************************************************
 * Name:    network_name2ops
 * Purpose: get operation functions for url
 * Input: 
 *   url: 
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
network_dir_t *
network_name2ops(const char *url)
{
    int i = 0;
    int path_len = 0;
    int dir_len = 0;
    int cnt = sizeof(network_root_dir) / sizeof(network_dir_t);

    if (!strcmp(url, "xmodem:")) {
        return &xmodem_root_dir;
    }

    path_len = sal_strlen(url);    
    for (i = 0; i < cnt; i ++) {
        dir_len = sal_strlen(network_root_dir[i].name);
        /* for bug 5133 del by weij
        if ((path_len >= dir_len)
            && !sal_strncmp(network_root_dir[i].name, url, dir_len)) {
            return &network_root_dir[i];;
        }
         for bug 5133 add by weij */
         if (path_len >= dir_len) {
             if (!sal_strncmp(network_root_dir[i].name, url, dir_len)) {
                 return &network_root_dir[i];
             }
         } else {
             if (!sal_strncmp(network_root_dir[i].name, url, path_len)) {
                 return &network_root_dir[i];
             }
         }
    }    

    return NULL;
}

/*******************************************************************************
 * Name:    gen_network_get_errmsg
 * Purpose: get network error message
 * Input: 
 *   type: error type
 * Output: 
 * Return: error string
 * Note: 
 ******************************************************************************/
char *
gen_network_get_errmsg(int type)
{
    switch (type)
    {
    case E_GEN_NETWORK_TIMEOUT:
        return "timeouted";
    case E_GEN_NETWORK_ACCESSDENY:
        return "access denied";
    case E_GEN_NETWORK_INVALID:
        return "invalid parameter";
    case E_GEN_NETWORK_FAILED:
        break;
    case E_GEN_NETWORK_SUCCESS:
    default:
        return "successed";
    }
    return "failed";
}

static int32 
gen_get_memory_check_state()
{
    char strLine[M_FILENAME_MAXSIZE];
    uint32 mem_states = 0;
    FILE *p_file = NULL;
    
    /*read the system memory check state from the file*/
    p_file = sal_fopen(GLB_MEMCHK_STATE_FILE, "r");
    if ((NULL == p_file) || (sal_feof(p_file)))
    {
        printf("%% system memory file is NULL\n");
    }
    else
    {
        while (sal_fgets(strLine, M_FILENAME_MAXSIZE, p_file) != NULL)
        {
            sal_sscanf(strLine, "%d", &mem_states);
        }
        sal_fclose(p_file);
    }
    return mem_states;
    /*end*/
}
    

/*******************************************************************************
 * Name:    copy_file_with_progress
 * Purpose: copy file and show progress
 * Input: 
 *   src_file: source file
 *   dst_file: dest file
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
#define HASHBYTES (50 * 1024)
int
copy_file_with_progress(char *src_file, char *dst_file, int need_progress)
{
    FILE *src_fp = NULL;
    FILE *dst_fp = NULL;
    char buf[BUFFER_COUNT];
    int ret = 0;
    int count = 0; 
    int amount = 0; 
    int hashbytes = 0;
    struct sal_timeval tstart;
    struct sal_timeval tstop;
    struct sigaction sa, saveint, saveterm;
    long filelength = 0;
    int memchek_state = 0;

    if (!sal_strcmp(src_file, dst_file)) {
        return 0;
    }

    /* Catch signals */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;        /* don't restart system calls */
    sa.sa_handler = handler;
    (void)sigaction(SIGINT, &sa, &saveint);
    (void)sigaction(SIGTERM, &sa, &saveterm);
    errno = 0;

    src_fp = sal_fopen(src_file, SAL_OPEN_RO);
    if (NULL == src_fp)
    {
        ret = -1;
        goto error;
    }
    fseek(src_fp, 0L, SEEK_END); 
    filelength = (int64_t)ftell(src_fp);
    fseek(src_fp, 0L, SEEK_SET); 

#ifndef _GLB_UML_SYSTEM_
    match_boot_and_remount(dst_file, MS_REMOUNT | MS_SILENT);
#endif
    dst_fp = sal_fopen(dst_file, SAL_OPEN_RW);
    if (NULL == dst_fp)
    {
        ret = -2;
        goto error;
    }

    (void)gettimeofday(&tstart, NULL);
    while (!bstop) {
        ret = -4;
        count = sal_fread(buf, sizeof(char), BUFFER_COUNT, src_fp);
        if (count < 0) {
            switch(errno) {
            case EIO:
                ret = -4;
                break;
            case EINTR:
               if (signo == SIGINT || signo == SIGTERM) {
                   ret = -5; /* Interruppted by user */
                   break;
               }
            case EAGAIN:
            case EINPROGRESS:
                continue;
            }
        } else if (count == 0) {
            if (filelength != amount) {
                ret = -6;
            } else {
                ret = 0;
            }
            break; /* end of file */
        }

        while (1) {
            ret = sal_fwrite(buf, sizeof(char), count, dst_fp);
            if (ret <= 0) {
                switch(errno) {
                case EIO:
                    ret = -4;
                    goto error;
                case EINTR:
                   if (signo == SIGINT || signo == SIGTERM) {
                       ret = -5; /* Interruppted by user */
                       goto error;
                   }
                case EAGAIN:
                case EINPROGRESS:
                    continue;
                }
            }
            
            amount += count;
            break;
        }

        /* check the memory on 1M boundary */
        memchek_state = gen_get_memory_check_state();
        if ((amount & 0x100000) && (memchek_state >= SYSLIMIT_MEMCHECK_STATE_WARNING)) {
            ret = -3;
            goto error;
        }
        
        while (amount >= hashbytes) {
            if (need_progress) {
                sal_printf(".");
                (void)sal_fflush(stdout);
            }
            hashbytes += HASHBYTES;
        }
    }

    if (bstop) {
        ret = -5; /* Interruppted by user */
    }

error:
    if (NULL != src_fp) {
        sal_fclose(src_fp);
    }
    if (NULL != dst_fp) {
        sal_fclose(dst_fp);
    }
    
    (void)sigaction(SIGINT, &saveint, NULL);
    (void)sigaction(SIGTERM, &saveterm, NULL);

    if (need_progress) {
        sal_printf("\nFile system synchronization. Please waiting...");
        (void)sal_fflush(stdout);
    }
    system("sync && echo 3 > /proc/sys/vm/drop_caches");
    (void)gettimeofday(&tstop, NULL);
    if (need_progress && amount && ( ret >= 0)) {
        double  delta;
        
        /* compute delta in 1/10's second units */
        delta = ((tstop.tv_sec * 10.) + (tstop.tv_usec / 100000)) -
                ((tstart.tv_sec * 10.) + (tstart.tv_usec / 100000));
        delta = delta / 10.;    /* back to seconds */
        sal_printf("\n%u bytes in %.1f seconds, %.0f kbytes/second\n",
                amount, delta, (amount * 1.) / (1024 * delta));
    } else if (need_progress) {
        sal_printf("\n");
    }

#ifndef _GLB_UML_SYSTEM_
    match_boot_and_remount(dst_file, MS_RDONLY | MS_REMOUNT | MS_SILENT);
#endif
    
    return ret;
}

/*******************************************************************************
 * Name:    gen_recv_from_url
 * Purpose: recv file from url
 * Input: 
 *   szDst: dest file
 *   url: url
 * Output: 
 * Return: 
 *   success: 0 
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_recv_from_url(char *errmsg, uint size, char *szDst, char *url, char *vrf)
{
    network_dir_t *ndir = NULL;
    network_op_t *ops = NULL;
    network_handle_t hdl;
    int ret = 0;
    int sret = E_GEN_NETWORK_SUCCESS;
    char szTmpName[M_FULLPATH_MAX_LEN];
    char szPPID[32];
    char *pszPPID = getenv("FNPPID");
    int memchek_state = 0;
    if (NULL == pszPPID) {
        pszPPID = szPPID;
        snprintf(szPPID, 32, "%u", getppid());
    }

    GEN_NETWORK_HANDLE_INIT(&hdl);
    if ((ndir = network_name2ops(url)) == NULL)
    {
        sal_snprintf(errmsg, size, "%%%% Invalid URL: %s\n", url);
        return -1;
    }

    if (NULL == ndir->ops)
    {
        sal_snprintf(errmsg, size, "%%%% Unimplemented method: %s\n", ndir->name);
        return -1;
    }
    ops = ndir->ops;
    hdl.ndir = ndir;

    sal_snprintf(szTmpName, M_FULLPATH_MAX_LEN, "/tmp/recv%s", pszPPID);

    /* init handle for this process */
    if (ops->init && ops->init(&hdl) != 0)
    {
        sal_snprintf(errmsg, size, "%%%% Init %s failed\n", ndir->name);
        return -1;
    }

    /* parse url to get it parameters */
    if (ops->parseurl)
    {
        if (ops->parseurl(&hdl, url) != 0)
        {
            /* parse url failed, we will get it interactive */
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    /* get parameters interactive if needed */
    if (0 != ret)
    {
        if (ops->interactive_get_param 
            && ops->interactive_get_param(&hdl) == 0)
        {
            ret = 0;
        }
    }
    if (0 != ret)
    {
        sal_snprintf(errmsg, size, "%%%% Get parameters failed. The format is: %s\n",
                ndir->usage);
        goto error_out;
    }
    /* delete by weij for bug 4662 */
    if (ops->append_dir && (ops->append_dir(&hdl, szDst) == 0))
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    if (0 != ret)
    {
        goto error_out;
    }

    /* let's do recvfrom*/
    if (NULL == ops->recvfrom)
    {
        sal_sprintf(errmsg, "%%%% Incomplete implement %s\n", ndir->name);
        ret = -1;
        goto error_out;
    }

    sal_printf("Download from URL to temporary file.\n");
    if ((sret = ops->recvfrom(&hdl, szTmpName, vrf)) != E_GEN_NETWORK_SUCCESS)
    {
        sal_snprintf(errmsg, size, "%%%% Get file %s\n", gen_network_get_errmsg(sret));
        memchek_state = gen_get_memory_check_state();
        if (memchek_state >= SYSLIMIT_MEMCHECK_STATE_WARNING)
        {
            sal_snprintf(errmsg, size, "%%%% There is not enough memory.\n");
        }
        ret = -1;
        goto error_out;
    }

    sal_printf("Copy the temporary file to its destination.\n");
    {
        char szCmd[1024];

        sal_snprintf(szCmd, 1024, "fnconvert -c filecopy -p %s -o %s",
                     szTmpName, szDst);
        ret = system(szCmd);
        ret = (WEXITSTATUS(ret) & 0xff);
        if (0 != ret) {
            sal_unlink(szDst);
        }
    }

error_out:
    if (ops->release)
    {
        ops->release(&hdl);
    }
    sal_unlink(szTmpName);
#ifdef TODO
    memmgr_free_cached_mem();
#endif
    return ret;
}

/*******************************************************************************
 * Name:    gen_network_register_ops
 * Purpose: register operations for network method
 * Input: 
 *   etype: network method type
 *   ops: callback functions
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_network_register_interactive_func(network_file_t etype, network_op_t *ops)
{
    int nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
    network_dir_t *ndir = NULL;
    int idx = 0;

    if (NETWORK_XMODEM == etype) {
        xmodem_root_dir.ops = ops;
        return 0;
    }
    
    for (idx = 0; idx < nCnt; idx++) {
        ndir = &network_root_dir[idx];
        if (etype == ndir->network_file) {
            ndir->ops = ops;
            return 0;
        }
    }
    
    return -1;
}

/*******************************************************************************
 * Name:    gen_check_and_get_filename_bycwd
 * Purpose: check and get file name by current working directory
 * Input: 
 *   filename: filename
 *   outsize: out buffer size
 * Output: 
 *   outfile:  out buffer
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_check_and_get_filename_bycwd(char *filename, char *outfile, size_t outsize)
{
    char szFullName[M_FULLPATH_MAX_LEN];    
    rootdir_t *rdir = NULL;
    int nValLen = 0;
    int nLen = 0;
    int nCnt = 0;
    int nIdx = 0;
    
    if (NULL == filename || NULL == outfile || 0 > outsize)
    {
        return -1;
    }

    /* Added by kcao for bug 38687, check filename start with '-' */
    if (sal_strstr (filename, "/-"))
    {
        return -1;
    }
    if ('-' == filename[0])
    {
        return -1;
    }
    /* Added by tongzb for bug 45876, check filename start with '.' */
    if (sal_strstr (filename, "/."))
    {
        return -1;
    }
    if ('.' == filename[0])
    {
        return -1;
    }
    /* Added by kcao for bug 38394, 38395, check char '%' for will mask PM crash */
    if (sal_strchr (filename, '%'))
    {
        return -1;
    }
    
    if (strlen(filename) >= M_FULLPATH_MAX_LEN)
    {
        return -1;
    }
    
    snprintf(szFullName, M_FULLPATH_MAX_LEN, filename);
    gen_path_getparents(szFullName);
    nLen = strlen(szFullName);

    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nValLen = strlen(rdir->show_name);
        if (!strncmp(szFullName, rdir->show_name,
                         strlen(rdir->show_name)))
        {
            if (!strcmp(szFullName, rdir->show_name))
            {
                snprintf(outfile, outsize, "%s",
                        rdir->real_name);
            }
            else if ('/' == szFullName[nValLen])
            {
                snprintf(outfile, outsize, "%s%s",
                        rdir->real_name, szFullName + nValLen);
                if ((strlen(rdir->real_name) + strlen(szFullName + nValLen) + 1) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    return -1;
                }
            }
            else
            {
                snprintf(outfile, outsize, "%s/%s",
                        rdir->real_name, szFullName + nValLen);
                if ((strlen(rdir->real_name) + strlen(szFullName + nValLen)) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    return -1;
                }
            }

            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
    }

    /* is it .. or . or raw path 
     * let connect it with current working directory and check it
     */
    szFullName[0] = '\0';
    szFullName[M_FULLPATH_MAX_LEN - 1] = '\0';
    if (getcwd(szFullName, M_FULLPATH_MAX_LEN) == NULL)
    {
        printf("%% Get current working directory failed: %s\n",
                strerror(errno));
        return -1;
    }
    
    nLen = strlen(szFullName);
    if ('/' != filename[0])
    {
        snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "/%s", filename);
        if ((nLen + strlen(filename) + 1) >= M_FULLPATH_MAX_LEN)
        {
            return -1;
        }
    }
    else
    {
        snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "%s", filename);
        if ((nLen + strlen(filename)) >= M_FULLPATH_MAX_LEN)
        {
            return -1;
        }
    }
    gen_path_getparents(szFullName);
    nLen = strlen(szFullName);
    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nValLen = strlen(rdir->real_name);
        if (!strncmp(szFullName, rdir->real_name,
                         strlen(rdir->real_name)))
        {
            snprintf(outfile, outsize, szFullName);
            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
        if (!strncmp(szFullName, rdir->real_name, nLen))
        {
            /* this directory not showing for user
             * setting it to root directory of this part
             */
            return -1;
        }
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_check_and_get_filename
 * Purpose: check and return absoulte filename 
 * Input: 
 *   filename: file name start with drive
 *   outsize: out buffer size
 * Output: 
 *   outfile: output file name
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_check_and_get_filename(char *filename, char *outfile, size_t outsize)
{
    char szFullName[M_FULLPATH_MAX_LEN];    
    rootdir_t *rdir = NULL;
    int nValLen = 0;
    int nLen = 0;
    int nCnt = 0;
    int nIdx = 0;
    
    if (NULL == filename || NULL == outfile || 0 > outsize)
    {
        return -1;
    }

    /* Added by kcao for bug 38687, check filename start with '-' */
    if (sal_strstr (filename, "/-"))
    {
        return -1;
    }
    if ('-' == filename[0])
    {
        return -1;
    }
    /* Added by tongzb for bug 45876, check filename start with '.' */
    if (sal_strstr (filename, "/."))
    {
        return -1;
    }
    if ('.' == filename[0])
    {
        return -1;
    }    
    /* Added by kcao for bug 38394, 38395, check char '%' for will mask PM crash */
    if (sal_strchr (filename, '%'))
    {
        return -1;
    }
    
    if (sal_strlen(filename) >= M_FULLPATH_MAX_LEN)
    {
        return -1;
    }

    sal_snprintf(szFullName, M_FULLPATH_MAX_LEN, filename);
    
    if (gen_validate_relative_path(szFullName) != 0)/* Modified by Percy Wang 2007-12-21 */
    {
        return -1;
    }
    
    gen_path_getparents(szFullName);
    nLen = sal_strlen(szFullName);

    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nValLen = sal_strlen(rdir->show_name);
        if (!sal_strncmp(szFullName, rdir->show_name,
                         sal_strlen(rdir->show_name)))
        {
            if (!sal_strcmp(szFullName, rdir->show_name))
            {
                sal_snprintf(outfile, outsize, "%s",
                        rdir->real_name);
            }
            else if ('/' == szFullName[nValLen])
            {
                sal_snprintf(outfile, outsize, "%s%s",
                        rdir->real_name, szFullName + nValLen);
                if ((sal_strlen(rdir->real_name) + sal_strlen(szFullName + nValLen)) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    return -1;
                }
            }
            else
            {
                sal_snprintf(outfile, outsize, "%s/%s",
                        rdir->real_name, szFullName + nValLen);
                if ((sal_strlen(rdir->real_name) + sal_strlen(szFullName + nValLen) + 1) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    return -1;
                }
            }
            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
    }

    /* is it .. or . or raw path 
     * let connect it with current working directory and check it
     */
    szFullName[0] = '\0';
    szFullName[M_FULLPATH_MAX_LEN - 1] = '\0';
    if (getcwd(szFullName, M_FULLPATH_MAX_LEN) == NULL)
    {
        return -1;
    }

    nLen = sal_strlen(szFullName);
    if ('/' != filename[0])
    {
        sal_snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "/%s", filename);
        if ((nLen + sal_strlen(filename) + 1) >= M_FULLPATH_MAX_LEN)
        {
            return -1;
        }
    }
    else
    {
        sal_snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "%s", filename);
        if ((nLen + sal_strlen(filename)) >= M_FULLPATH_MAX_LEN)
        {
            return -1;
        }
    }
    gen_path_getparents(szFullName);
    nLen = sal_strlen(szFullName);
    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    if (dir_mnt_valid)
    {
        if (!sal_strncmp(szFullName, FULL_NAME_MNT,
                         sal_strlen(FULL_NAME_MNT)))
        {
            sal_snprintf(outfile, outsize, szFullName);
            return 0;
        }
    }
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nValLen = sal_strlen(rdir->real_name);
        if (!sal_strncmp(szFullName, rdir->real_name,
                         sal_strlen(rdir->real_name)))
        {
            sal_snprintf(outfile, outsize, szFullName);

            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
        if (!sal_strncmp(szFullName, rdir->real_name, nLen))
        {
            /* this directory not showing for user
             * setting it to root directory of this part
             */
            return -1;
        }
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_check_and_get_filename_bytype
 * Purpose: check and return absoulte filename by disk type
 * Input: 
 *   filename: file name start with drive
 *   outsize: out buffer size
 *   type: disk type
 * Output: 
 *   outfile: output file name
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_check_and_get_filename_bytype(char *filename, char *outfile,
                                  size_t outsize, int type)
{
    rootdir_t *rdir = NULL;
    int nCnt = 0;
    int nIdx = 0;
    
    if (NULL == filename || NULL == outfile || 0 > outsize)
    {
        return -1;
    }

    switch (type)
    {
    case E_DISK_FILE_CFCARD:
    case E_DISK_FILE_FLASH:
    case E_DISK_FILE_UDISK:
        break;
    default:
        SAL_ASSERT(0);
        break;
    }

    if (gen_check_and_get_filename(filename, outfile, outsize) != 0)
    {
        return -1;
    }
    
    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        if (type != rdir->etype)
        {
            continue;
        }

        if (!sal_strncmp(outfile, rdir->real_name, sal_strlen(rdir->real_name)))
        {
            return 0;
        }
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_check_and_gen_showname
 * Purpose: check orignal path and build showing string
 * Input: 
 *   szOrig: absolute path
 *   szShow: showing path
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_check_and_gen_showname(char *szOrig, char *szShow)
{
    int nCnt = 0;
    int nIdx = 0;
    rootdir_t *rdir = NULL;
    
    if (NULL == szOrig || NULL == szShow)
    {
        return -1;
    }
    
    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        if (!sal_strncmp(szOrig, rdir->real_name,
                         sal_strlen(rdir->real_name)))
        {
            if (!sal_strcmp(szOrig, rdir->real_name))
            {
                sal_snprintf(szShow, M_FULLPATH_MAX_LEN,
                        "%s/", rdir->show_name);
            }
            else if ('/' == szOrig[sal_strlen(rdir->real_name)])
            {
                sal_snprintf(szShow, M_FULLPATH_MAX_LEN, "%s%s",
                        rdir->show_name,
                        szOrig + sal_strlen(rdir->real_name));
            }
            else
            {
                sal_snprintf(szShow, M_FULLPATH_MAX_LEN, "%s/%s",
                        rdir->show_name,
                        szOrig + sal_strlen(rdir->real_name));
            }
            return 0;
        }
    }

    return -1;
}

param_token_e gen_parse_token(const char *token)
{
    param_token_e type = E_TOKEN_UNKNOWN;
    
    if (strcmp(token, M_GEN_FILENAME_STR) == 0) {
        type = E_TOKEN_GFILENAME;
    }
    else if (strcmp(token, M_GEN_URLNAME_STR) == 0) {
        type = E_TOKEN_GURLNAME;
    }
    else if (strcmp(token, M_GEN_FLASHNAME_STR) == 0)
    {
        type = E_TOKEN_GFLASHNAME;
    }
    else if (strcmp(token, M_GEN_CFCARDNAME_STR) == 0)
    {
        type = E_TOKEN_GCFCARDNAME;
    }
    
    return type;
}

/* Added by Alexander Liu 2007-02-28 09:19 */
char *g_gen_last_token = NULL;
int g_gen_match_status = 0;

/*******************************************************************************
 * Name:    gen_set_last_token
 * Purpose: set last token of the command line
 * Input: 
 *   str: command line str
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int 
gen_set_last_token(char *str)
{
    if (NULL == str)
    {
        return -1;
    }

    g_gen_last_token = strrchr(str, ' ');
    if (NULL != g_gen_last_token)
    {
        g_gen_last_token++;
    }

    if (NULL == g_gen_last_token)
    {
        g_gen_last_token = str;
    }       
    return 0;
}

/*******************************************************************************
 * Name:    gen_match_last_token
 * Purpose: match last token of the command line
 * Input: 
 *   value: value to match
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_match_last_token(char *value)
{
    int nValLen = 0;
    int nTokLen = 0;
    
    if (NULL == g_gen_last_token || NULL == value)
    {
        return -1;
    }

    nValLen = strlen(value);
    nTokLen = strlen(g_gen_last_token);
    if (nTokLen > nValLen)
    {
        /* we must match the prefix */
        if (!strncmp(value, g_gen_last_token, nValLen))
        {
            return 0;
        }
        return -1;
    }
    
    /* we may match at header */
    if (!strncmp(value, g_gen_last_token, nTokLen))
    {
        return 0;
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_get_last_token_len
 * Purpose: get last token length
 * Input: N/A
 * Output: 
 * Return: 
 *   success: length
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
gen_get_last_token_len(void)
{
    if (NULL == g_gen_last_token)
    {
        return 0;
    }

    return strlen(g_gen_last_token);
}

/*******************************************************************************
 * Name:    gen_get_last_token_str
 * Purpose: get last token string
 * Input: N/A
 * Output: 
 * Return: 
 *   success: token str
 *   failed : NULL
 * Note: 
 ******************************************************************************/
char *
gen_get_last_token_str(void)
{
    return g_gen_last_token;
}

/*******************************************************************************
 * Name:    gen_filename_match
 * Purpose: match general filename
 * Input: 
 *   cp: filename string
 *   token: token type
 * Output: 
 * Return: 
 *   full match: E_GEN_MATCH_ALL
 *   none match: E_GEN_MATCH_NONE
 *   incomplete: E_GEN_MATCH_INCOMPLETE
 * Note: 
 ******************************************************************************/
gen_match_e gen_filename_match(char *cp, int token, int child_node)
{
    int nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    int nLen = strlen(cp);
    int nRootLen = 0;
    int idx = 0;
    char *file_end  = NULL;
    rootdir_t *rdir = NULL;
    network_dir_t *ndir = NULL;

    /*for bug 5133 add by weij */
    file_end = strchr(cp, ' ');
    if (file_end)
    {
        nLen = file_end - cp;
    }

    for (idx = 0; idx < nCnt; idx++)
    {
        rdir = &actual_root_dir[idx];

        switch (token)
        {
        case E_TOKEN_GFLASHNAME:
            if (E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            break;
        case E_TOKEN_GCFCARDNAME:
            if (E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }
            break;
        case E_TOKEN_GFILENAME:
            break;
        default:
            assert(0);
            break;
        }
        
        nRootLen = strlen(rdir->show_name);
        if (nRootLen > nLen
            && !strncmp(rdir->show_name, cp, nLen))
        {
            return E_GEN_MATCH_INCOMPLETE;
        }

        if (!strncmp(rdir->show_name, cp, nRootLen))
        {
            return E_GEN_MATCH_ALL;
        }
    }

    /* we can't match URL */
    nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
    for (idx = 0; idx < nCnt; idx++)
    {
        ndir = &network_root_dir[idx];
        if (!strncmp(ndir->name, cp, strlen(ndir->name)))
        {
            if (1 == child_node)
            {
                /*child_node == 1 means that we need not to match url*/
                return E_GEN_MATCH_ALL;
            }
            return E_GEN_MATCH_NONE;
        }
    }

    /* we can't match xmodem: */
    if (!strncmp("xmodem:", cp, strlen("xmodem:"))) {
        if (1 == child_node) {
            /*child_node == 1 means that we need not to match url*/
            return E_GEN_MATCH_ALL;
        }
        return E_GEN_MATCH_NONE;
    }

    /* match .. and .. and raw name */
    if (nLen > 0)
    {
        return E_GEN_MATCH_ALL;
    }

    return E_GEN_MATCH_NONE;
}

/*******************************************************************************
 * Name:    gen_url_match
 * Purpose: match general url
 * Input: 
 *   cp: url string
 * Output: 
 * Return: 
 *   full match: E_GEN_MATCH_ALL
 *   none match: E_GEN_MATCH_NONE
 *   incomplete: E_GEN_MATCH_INCOMPLETE
 * Note: 
 ******************************************************************************/
gen_match_e gen_url_match(char *cp)
{
    int nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
    int nLen = strlen(cp);
    int nRootLen = 0;
    int idx = 0;
    char *url_end = NULL;
    
    /*for bug 5133 add by weij */
    url_end = strchr(cp, ' ');
    if (url_end)
    {
        nLen = url_end - cp;
    }

    for (idx = 0; idx < nCnt; idx++)
    {
        nRootLen = strlen(network_root_dir[idx].name);
        if (!strncmp(network_root_dir[idx].name, cp, nRootLen)
            && nLen >= nRootLen)
        {
            return E_GEN_MATCH_ALL;
        }

        if (nRootLen > nLen
            && !strncmp(network_root_dir[idx].name, cp, nLen))
        {
            return E_GEN_MATCH_INCOMPLETE;
        }
    }

    return E_GEN_MATCH_NONE;
}

/*******************************************************************************
 * Name:    gen_match_path_main
 * Purpose: match path and call callback function to process it
 * Input: 
 *   pathprefix: path prefix
 *   pathstr: path to match
 *   func: callback func
 *   pArg: callback argument
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_match_path_main(char *pathprefix, char *pathstr,
                    int (*func)(char *, char *, void *), void *pArg)
{
    char szFullName[M_FULLPATH_MAX_LEN];
    char szSuffix[M_FULLPATH_MAX_LEN] = {'\0'};
    char *p = NULL;
    struct dirent *ent = NULL;
    DIR *dir = NULL;
    int filename_len = 0;
    int prefix_len = strlen(pathprefix);

    if (NULL == pathprefix || NULL == func)
    {
        return -1;
    }

    if (NULL == pathstr)
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s", pathprefix);
        szSuffix[0] = '\0';
    }
    else
    {
        if (NULL != strstr(pathstr, "%"))
        {
            return -1;
        }
        if (NULL != (p = strrchr(pathstr, '/')))
        {
            p++;
            if ('\0' != *p)
            {
                snprintf(szSuffix, M_FULLPATH_MAX_LEN, p);
            }
        }
        else
        {
            snprintf(szSuffix, M_FULLPATH_MAX_LEN, pathstr);
        }
        filename_len = strlen(szSuffix);
 
        /* split path from full path */
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", pathprefix, pathstr);
        /* check security for .. & . */
        gen_path_getparents(szFullName);
        if (strncmp(pathprefix, szFullName, prefix_len))
        {
            return -1;
        }
        if (NULL != (p = strrchr(szFullName + prefix_len, '/')))
        {
            *p = '\0';
        }
        else
        {
            szFullName[prefix_len] = '\0';
        }
    }
       
    /* search dir */
    dir = opendir(szFullName);
    if (NULL == dir)
    {
        return -1;
    }

    /* find the partial match item */
    rewinddir(dir);
    while ((ent = readdir(dir)) != NULL)
    {
        if (!strcmp(ent->d_name, ".")
            || !strcmp(ent->d_name, ".."))
        {
            continue;
        }

        /*Can't match hidden file, add by Percy Wang, bug 12643*/
        if (ent->d_name[0] == '.')
        {
            continue;
        }
        
        if (('\0' == szSuffix[0])
            || (ent->d_name[0] == szSuffix[0]
#if defined(__SVR4) || defined(__linux__)
                && strlen(ent->d_name) >= filename_len
#else
                && ent->d_namlen >= filename_len
#endif
                && strncmp(ent->d_name, szSuffix, filename_len) == 0))
        {
            if (NULL != func)
            {
                func(szFullName, ent->d_name, pArg);
            }
        }
    }
    
    closedir(dir);
    return 0;
}


/*******************************************************************************
 * Name:    gen_match_path_callback_output
 * Purpose: callback to match path and output item
 * Input: 
 *   dir: directory
 *   str: matched string
 *   arg: callback argument
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int gen_match_path_callback_output(char *dir, char *str, void *arg)
{
    char szFullName[M_FULLPATH_MAX_LEN];    
    gen_path_match_arg_t *pmatch = (gen_path_match_arg_t *)arg;
    int bDir = 0;
    
    if (NULL == str || NULL == pmatch || NULL == dir)
    {
        return -1;
    }

    snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", dir, str);
    bDir = lib_fs_is_directory(szFullName);    

    /* trim last '/' */
    if (dir[strlen(dir) - 1] == '/')
    {
        dir[strlen(dir) - 1] = '\0';
    }

    if (!strcmp(dir, pmatch->rdir->real_name))
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", 
                pmatch->rdir->show_name, str);
    }
    else
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s%s/%s", 
                pmatch->rdir->show_name,
                dir + strlen(pmatch->rdir->real_name),
                str);
    }

    pmatch->pCB(pmatch->pArg, szFullName, 
        bDir ? "Directory" : "File");
    pmatch->nMatched += 1;
    return 0;
}


/*******************************************************************************
 * Name:    gen_output_match_path
 * Purpose: output matched items for path
 * Input: 
 *   rdir: path root
 * Output: 
 *   ptr: output buffer
 *   pnItems: items 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_output_match_path(rootdir_t *rdir, HELP_OUT_FUNC *pCB, void *pArg, char *oldtext)
{
    char *pTokStr = gen_get_last_token_str();
    char *pPathStr = NULL;
    gen_path_match_arg_t matcharg = 
    { 
        .pCB = pCB,
        .pArg = pArg,
        .oldtext = oldtext,
        .rdir = rdir,
        .nMatched = 0,
    };

    if (NULL == rdir || NULL == pTokStr)
    {
        return -1;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        pCB(pArg, pTokStr, oldtext);
        return 0;
    }
    pPathStr = pTokStr + strlen(rdir->show_name);
    while ('\0' != *pPathStr)
    {
        if ('/' != *pPathStr)
        {
            break;
        }
        pPathStr++;
    }

    gen_match_path_main(rdir->real_name, pPathStr,
                        gen_match_path_callback_output, (void *)&matcharg);
    if (0 == matcharg.nMatched)
    {
        /* let just match pTokStr */
        pCB(pArg, pTokStr, oldtext);
        return 0;
    }

    return 0;
}

void gen_clear_match_status(void)
{
    g_gen_match_status = E_GEN_MATCH_NONE;
}

int gen_get_match_status(void)
{
    return g_gen_match_status;
}

void gen_set_match_status(int status)
{
    g_gen_match_status = status;
}


/*******************************************************************************
 * Name:    gen_output_matchstr
 * Purpose: output matched string
 * Input: 
 *   oldtext: old text
 *   type: matched type string
 * Output: 
 *   ptr: return pointer
 *   nCntRet: match item count
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
gen_output_matchstr(HELP_OUT_FUNC *pCB, void *pArg, char *oldtext, char *type)
{
    int nCnt = 0;
    int nIdx = 0;
    int nTokenLen = gen_get_last_token_len();
    int nValLen = 0;
    rootdir_t *rdir = NULL;

    if (!strcmp(type, M_GEN_FILENAME_STR) || !strcmp(type, M_GEN_FLASHNAME_STR)
            || !strcmp(type, M_GEN_CFCARDNAME_STR)) {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++) {
            rdir = &actual_root_dir[nIdx];
            if (!strcmp(type, M_GEN_FLASHNAME_STR)
                 && E_DISK_FILE_FLASH != rdir->etype) {
                continue;
            }
            if (!strcmp(type, M_GEN_CFCARDNAME_STR) 
                && E_DISK_FILE_CFCARD != rdir->etype) {
                continue;
            }
            
            if (gen_match_last_token(rdir->show_name) == 0) {
                nValLen = strlen(rdir->show_name);
                if (nValLen > nTokenLen) {
                    pCB(pArg, rdir->show_name, oldtext);
                } else {
                    /* search the path and output matched items */
                    gen_output_match_path(&actual_root_dir[nIdx], pCB, pArg, oldtext);
                }
            }
        }
        
        return;
    }

    if (!strcmp(type, M_GEN_URLNAME_STR)) {
        nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++) {
            if (gen_match_last_token(network_root_dir[nIdx].name) == 0) {
                nValLen = strlen(network_root_dir[nIdx].name);
                if ((nValLen >= nTokenLen) || nTokenLen <= 0) {
                    pCB(pArg, network_root_dir[nIdx].name, oldtext);
                } else {
                    pCB(pArg, gen_get_last_token_str(), oldtext);
                    return;
                }
            }
        }

        return;
    }

    assert(0);
    return;
}

/* Added by kcao for allow /mnt/ directory */
int
gen_set_dir_mnt_valid(int valid)
{
    dir_mnt_valid = valid;
    return 0;
}

/* readline */
int cli_readline(char *pszBuf, size_t bufsz, char *pszFmt, ...)
{
    char szMsg[M_FULLPATH_MAX_LEN];
    int timeout = 300;
    int nr = 0;
    va_list ap;
    
    va_start(ap, pszFmt);
    vsnprintf(szMsg, M_FULLPATH_MAX_LEN, pszFmt, ap);
    va_end(ap);

    if (timeout > 0) {
        nr = cli_readpassphrase(szMsg, pszBuf, bufsz, 
                CLI_RPP_ECHO_ON | CLI_RPP_WITH_TIMEOUT | CLI_RPP_LINE_MODE, timeout);
    } else {
        nr = cli_readpassphrase(szMsg, pszBuf, bufsz, 
                CLI_RPP_ECHO_ON | CLI_RPP_LINE_MODE, 0);
    }

#if 0
    if ('\0' != pszBuf[0]) {
        printf("\n");
    }
#endif

    switch (nr) {
    case -4:
        printf("\n");
        return -1;
    case -2:
        printf("\n%% Key input is timed out.\n");
        return -1;
    case -1:
        printf("\n%% Read input failed: %s\n", strerror(errno));
        return -1;
    case -3:
        printf("\n%% Message too long.\n");
        return -1;
    default:
        break;
    }

    return 0;
}

/* default yes */
int cli_confirm(char *pszFmt, ...)
{
    char szMsg[M_FULLPATH_MAX_LEN];
    char g_buf[CLI_RPP_PASSWORD_LEN + 1];
    int timeout = 300;
    int nr = 0;
    va_list ap;
    
    va_start(ap, pszFmt);
    vsnprintf(szMsg, M_FULLPATH_MAX_LEN, pszFmt, ap);
    va_end(ap);

    if (timeout > 0) {
        nr = cli_readpassphrase(szMsg, g_buf, CLI_RPP_PASSWORD_LEN, 
                CLI_RPP_ECHO_ON | CLI_RPP_WITH_TIMEOUT | CLI_RPP_CONFIRM_MODE,
                timeout);
    } else {
        nr = cli_readpassphrase(szMsg, g_buf, CLI_RPP_PASSWORD_LEN, 
                CLI_RPP_ECHO_ON | CLI_RPP_CONFIRM_MODE,
                0);
    }

    if ('\0' != g_buf[0]) {
        printf("\n");
    }

    switch (nr) {
    case -4:
        printf("\n");
        return -1;
    case -2:
        printf("\n%% Key input is timed out.\n");
        return -1;
    case -1:
        printf("\n%% Get confirm failed: %s\n", strerror(errno));
        return -1;
    case -3:
        printf("\n%% Confirm message too long.\n");
        return -1;
    default:
        break;
    }

    if ('\0' == g_buf[0] || !strcasecmp(g_buf, "y")
        || !strcasecmp(g_buf, "ye") || !strcasecmp(g_buf, "yes")) {       
        return 1;
    }
    
    return 0;
}

/* default is no */
int cli_yes_no_default_no(char *pszFmt, ...)
{
    char szMsg[M_FULLPATH_MAX_LEN];
    char g_buf[CLI_RPP_PASSWORD_LEN + 1];
    int timeout = 300;
    int nr = 0;
    va_list ap;

    va_start(ap, pszFmt);
    vsnprintf(szMsg, M_FULLPATH_MAX_LEN, pszFmt, ap);
    va_end(ap);

again:    
    if (timeout > 0) {
        nr = cli_readpassphrase(szMsg, g_buf, CLI_RPP_PASSWORD_LEN, 
                CLI_RPP_ECHO_ON | CLI_RPP_WITH_TIMEOUT | CLI_RPP_LINE_MODE, timeout);
    } else {
        nr = cli_readpassphrase(szMsg, g_buf, CLI_RPP_PASSWORD_LEN, 
                CLI_RPP_ECHO_ON | CLI_RPP_LINE_MODE, 0);
    }

    switch (nr) {
    case -4:
        return -1;
    case -2:
        printf("%% Key input is timed out.\n");
        return -1;
    case -1:
        printf("%% Get confirm failed: %s\n", strerror(errno));
        return -1;
    case -3:
        printf("%% Confirm message too long.\n");
        return -1;
    default:
        break;
    }

    if ('\0' == g_buf[0]) {
        printf("%% Please answer 'yes' or 'no'.\n");
        goto again;
    }

    if (!strcasecmp(g_buf, "y") || !strcasecmp(g_buf, "ye") || !strcasecmp(g_buf, "yes")) {       
        return 1;
    }
    if (!strcasecmp(g_buf, "n") || !strcasecmp(g_buf, "no")) {       
        return 0;
    }

    printf("%% Please answer 'yes' or 'no'.\n");
    goto again;
    
    return 0;
}

int gen_file_proc_confirm(char *action, char *file, char *origfile)
{
     char szFile[M_FULLPATH_MAX_LEN];
    char *p = NULL;

    if (NULL == file || NULL == action) {
        return -1;
    }

    snprintf(szFile, M_FULLPATH_MAX_LEN, file);
    /* trim tail '/' */
    p = &szFile[strlen(szFile) - 1];
    while ((p >= szFile) && ('/' == *p)) {
        *p = '\0';
        p--;
    }
    if (p < szFile) {
        return -1;
    } 
    
    if (lib_fs_is_exist(szFile)) {
        if (cli_yes_no_default_no("Are you sure to %s %s? [no]",
                action, origfile) <= 0) {
            printf("%% Aborted!\n");
            return -1;
        }
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    readpassphrase
 * Purpose: read username/pass word with timeout
 * Input: 
 *   prompt: 
 *   buf: 
 *   bufsiz: 
 *   flags: 
 *   timeout: 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1: error
 *            -2: timeout
 *            -3: too long
 *            -4: interrupted
 * Note: 
 ******************************************************************************/
int cli_readpassphrase(const char *prompt, char *buf, size_t bufsiz, int flags, int timeout)
{
    ssize_t nr;
    int input, output, save_errno;
    char ch, *p, *end;
    struct termios term, oterm;
    struct sigaction sa, savealrm, saveint, savehup, savequit, saveterm;
    struct sigaction savetstp, savettin, savettou, savepipe;
    struct timeval tv = {0, 0};
    struct timeval *ptv = NULL;
    fd_set rset;

    /* I suppose we could alloc on demand in this case (XXX). */
    if (bufsiz == 0) {
        errno = EINVAL;
        return -2;
    }

restart:
    signo = 0;
    input = STDIN_FILENO;
    output = STDERR_FILENO;

    /*
     * Catch signals that would otherwise cause the user to end
     * up with echo turned off in the shell.  Don't worry about
     * things like SIGXCPU and SIGVTALRM for now.
     */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;        /* don't restart system calls */
    sa.sa_handler = handler;
    (void)sigaction(SIGALRM, &sa, &savealrm);
    (void)sigaction(SIGHUP, &sa, &savehup);
    (void)sigaction(SIGINT, &sa, &saveint);
    (void)sigaction(SIGPIPE, &sa, &savepipe);
    (void)sigaction(SIGQUIT, &sa, &savequit);
    (void)sigaction(SIGTERM, &sa, &saveterm);
    (void)sigaction(SIGTSTP, &sa, &savetstp);
    (void)sigaction(SIGTTIN, &sa, &savettin);
    (void)sigaction(SIGTTOU, &sa, &savettou);

    /* Turn off echo if possible. */
    tcgetattr(input, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    if (flags & CLI_RPP_ECHO_ON)
    {
        term.c_lflag |= (ECHO | ECHONL);
    }
    else
    {
        term.c_lflag &= ~(ECHO | ECHONL);
    }

    if (!(flags & CLI_RPP_LINE_MODE)) {
        term.c_lflag &= ~(ICANON);
    }
    (void)tcsetattr(input, TCSANOW, &term);

    /* output prompt */
    if (prompt && prompt[0] != '\0')
        (void)write(output, prompt, strlen(prompt));

    FD_ZERO(&rset);
    FD_SET(input, &rset);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    end = buf + bufsiz - 1;
    p = buf;
    while (1)
    {
        if (flags & CLI_RPP_WITH_TIMEOUT)
        {
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            ptv = &tv;
        }
        nr = select(input + 1, &rset, NULL, NULL, ptv);
        if (nr == 0)
        {
            /* timeout */
            nr = -2;
            break;
        }

        if (nr < 0)
        {
            switch(errno)
            {
            case EINTR:
               if (signo == SIGINT || signo == SIGTSTP)
               {
                   nr = -4;
                   break;
               }
            case EAGAIN:
            case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
            case EWOULDBLOCK:
#endif /* (EWOULDBLOCK != EAGAIN) */
                continue;
            }

            nr = -1;
            break;
        }

        nr = read(input, &ch, 1);
        if (1 != nr)
        {
            nr = -1;
            break;
        }
        
        if (ch == '\n' || ch == '\r')
        {
            nr = 0;
            break;
        }

        /* ignore NUL for console */
        if (0 == ch) {
            continue;
        }

        /* fix bug 48807, only print alpha */
        if (!isprint(ch)) {
            continue;
        }

        if (p < end) 
        {
            *p++ = ch;
            if (flags & CLI_RPP_CONFIRM_MODE)
            {
                break;
            }
        }
        else
        {
            nr = -3;
            break;
        }
    }

    *p = '\0';
    save_errno = errno;
    if (!(term.c_lflag & ECHO))
    {
        (void)write(output, "\n", 1);
    }

    /* Restore old terminal settings and signals. */
    if (memcmp(&term, &oterm, sizeof(term)) != 0) 
    {
        while (tcsetattr(input, TCSANOW, &oterm) == -1 &&
            errno == EINTR)
        {
            continue;
        }
    }
    (void)sigaction(SIGALRM, &savealrm, NULL);
    (void)sigaction(SIGHUP, &savehup, NULL);
    (void)sigaction(SIGINT, &saveint, NULL);
    (void)sigaction(SIGQUIT, &savequit, NULL);
    (void)sigaction(SIGPIPE, &savepipe, NULL);
    (void)sigaction(SIGTERM, &saveterm, NULL);
    (void)sigaction(SIGTSTP, &savetstp, NULL);
    (void)sigaction(SIGTTIN, &savettin, NULL);
    (void)sigaction(SIGTTOU, &savettou, NULL);

    /*
     * If we were interrupted by a signal, resend it to ourselves
     * now that we have restored the signal handlers.
     */
    if (signo) 
    {
        kill(getpid(), signo);
        switch (signo) 
        {
        case SIGTTIN:
        case SIGTTOU:
            goto restart;
        }
    }

    errno = save_errno;
    return nr;
}

static void handler(int s)
{
    signo = s;
    bstop = 1;
}


/*******************************************************************************
 * Name:    gen_is_bin_file
 * Purpose: check to determine if this file is bin file
 * Input: 
 *   file: file name
 * Output: 
 * Return: 
 *   success: 1
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
gen_is_bin_file(char *file)
{
    char twobytes[2];
    FILE *fp = NULL;

    if (NULL == file)
    {
        return 0;
    }

    if ((fp = fopen(file, "r")) == NULL)
    {
        return 0;
    }
    
    /* don't try to look ahead if the input is unseekable */
    if (fseeko(fp, (off_t)0, SEEK_SET))
    {
        fclose(fp);
        return 0;
    }

    if (fread(twobytes, 2, 1, fp) == 1) 
    {
        switch((twobytes[0] + (twobytes[1]<<8)) & 0xFFFF) 
        {            
        case OMAGIC:
        case NMAGIC:
        case ZMAGIC:
        case 0405:
        case 0411:
        case 0x457f:
        case 0177545:
        case 0x8b1f: /* it's gzip file */
        case 0x1f8b: 
        case 0x0527: /* it's image file */
        case 0x2705:
            (void)fclose(fp);
            return 1;
        }
    }
    
    fclose(fp);
    return 0;
}

/*******************************************************************************
 * Name:    gen_send_to_url
 * Purpose: send file to url
 * Input: 
 *   cli: client pointer
 *   szSrc: file to send
 *   url: url
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_send_to_url(char *szSrc, char *url, char *vrf)
{
    network_dir_t *ndir = NULL;
    network_op_t *ops = NULL;
    network_handle_t hdl;
    int ret = 0;
    int sret = E_GEN_NETWORK_SUCCESS;

    GEN_NETWORK_HANDLE_INIT(&hdl);
    if ((ndir = network_name2ops(url)) == NULL)
    {
        printf("%% Invalid URL: %s\n", url);
        return -1;
    }

    if (NULL == ndir->ops)
    {
        printf("%% Unimplemented method: %s\n", ndir->name);
        return -1;
    }
    ops = ndir->ops;
    hdl.ndir = ndir;
    
    /* init handle for this process */
    if (ops->init && ops->init(&hdl) != 0)
    {
        printf("%% Init %s failed\n", ndir->name);
        return -1;
    }

    /* parse url to get it parameters */
    if (ops->parseurl)
    {
        if (ops->parseurl(&hdl, url) != 0)
        {
            /* parse url failed, we will get it interactive */
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    /* get parameters interactive if needed */
    if (0 != ret)
    {
        if (ops->interactive_get_param 
            && ops->interactive_get_param(&hdl) == 0)
        {
            ret = 0;
        }
    }
    if (0 != ret)
    {
        printf("%% Get parameters failed. The format is: %s\n",
                ndir->usage);
        goto error_out;
    }

    /* let's do sendto */
    if (NULL == ops->sendto)
    {
        printf("%% Incomplete implement %s\n", ndir->name);
        ret = -1;
        goto error_out;
    }

    if ((sret = ops->sendto(&hdl, szSrc, vrf)) != E_GEN_NETWORK_SUCCESS)
    {
        printf("%% Send file %s\n", gen_network_get_errmsg(sret));
        ret = -1;
        goto error_out;
    }
    
error_out:
    if (ops->release)
    {
        ops->release(&hdl);
    }
#ifdef TODO
    memmgr_free_cached_mem();
#endif
    return ret;
}

int gen_execvp(int argc, char **argv)
{
    char *pszBuf = NULL;
    int ret = -1;
    int nLen = 0;
    int nTotal = 0;
    int i;

    if (NULL == (pszBuf = malloc(2048))) {
        fprintf(stderr, "%% Warning: Out of memory\r\n");
        goto err_out;
    }

    for (i = 0; i < argc; i++) {
        nLen = strlen(argv[i]);
        if ((nLen + nTotal + 1) >= 2048) {
            fprintf(stderr, "%% Parameter too long\r\n");
            goto err_out;
        }
        nTotal += snprintf(pszBuf + nTotal, 2048 - nTotal, "%s%s", 
                           i ? " " : "", argv[i]);
    }
    ret = system(pszBuf);

    ret = (WEXITSTATUS(ret) & 0xff);
    
err_out:
    if (NULL != pszBuf) {
        free(pszBuf);
    }
    return ret;
}

void
gen_dbg_print_hex_emacs(int modid, const u_char *cp, unsigned int length, unsigned int offset)
{
#define DEBUG_LINEBUF_SIZE  256
    char ln[DEBUG_LINEBUF_SIZE];
    unsigned int i, j, jm;
    char *p = NULL;
    int nLeft = 0;
    int nRet = 0;
    char buf[2];
    int c;

    for (i = 0; i < length; i += 0x10) {
        p = ln;
        nLeft = DEBUG_LINEBUF_SIZE;
        nRet = snprintf(p, nLeft, "  %04x: ", (unsigned int)(i + offset));
        p += nRet;
        nLeft -= nRet;
        jm = length - i;
        jm = jm > 16 ? 16 : jm;

        for (j = 0; j < jm; j++) {
            if ((j % 2) == 1) {
                nRet = snprintf(p, nLeft, "%02x ", (unsigned int)cp[i+j]);
            } else {
                nRet = snprintf(p, nLeft, "%02x", (unsigned int)cp[i+j]);
            }
            p += nRet;
            nLeft -= nRet;
        }
        
        for (; j < 16; j++) {
            if ((j % 2) == 1) {
                nRet = snprintf(p, nLeft, "   ");
            } else {
                nRet = snprintf(p, nLeft, "  ");
            }
            p += nRet;
            nLeft -= nRet;
        }

        nRet = snprintf(p, nLeft, " ");
        p += nRet;
        nLeft -= nRet;
        for (j = 0; j < jm; j++) {
            c = cp[i+j];
            c = isprint(c) ? c : '.';
            buf[0] = c;
            buf[1] = '\0';
            nRet = snprintf(p, nLeft, buf);
            p += nRet;
            nLeft -= nRet;
        }
        log_sys(modid, E_DEBUG, "%s", ln);
    }

    return;
}

const char *gen_get_url_helpstr(const char *pszUrl)
{
    network_dir_t *ndir = NULL;

    if (NULL != (ndir = network_name2ops(pszUrl))) {
        return ndir->usage;
    }
    
    return NULL;
}

/* Added by liuht for bug 42307, 2016-12-28 */
/*******************************************************************************
 * Name:    match_boot_and_remount
 * Purpose: if matchdir is included in boot partition, then remount boot 
 *          partition with vfsflags
 * Input: 
 *   matchdir: the directory to match with boot partition
 *   vfsflags: the boot partition's remount flags
 * Note: 
 ******************************************************************************/
void 
match_boot_and_remount(char *matchdir, int vfsflags)
{
    uint8 len;

    if (sal_strlen(matchdir) > sal_strlen("/mnt/flash/boot"))
        len = sal_strlen("/mnt/flash/boot") + 1;
    else
        len = sal_strlen("/mnt/flash/boot");

#ifdef _CTC_ARM_HI3535_
    if(!sal_strncmp(matchdir, "/mnt/flash/boot/", len))
        mount("/dev/sda2", "/mnt/flash/boot", "ext4", vfsflags, NULL);
#elif defined _CTC_ARM_LS1023A_
    if(!sal_strncmp(matchdir, "/mnt/flash/boot/", len))
        mount("/dev/mmcblk0p2", "/mnt/flash/boot", "ext4", vfsflags, NULL);
#elif defined _CTC_ARM_CTC5236_
    if(!sal_strncmp(matchdir, "/mnt/flash/boot/", len))
        mount("/dev/mmcblk0p2", "/mnt/flash/boot", "ext4", vfsflags, NULL);
#else
    if(!sal_strncmp(matchdir, "/mnt/flash/boot/", len))
        mount("ubi1:boot", "/mnt/flash/boot", "ubifs", vfsflags, NULL);
#endif

}

/* Added by liuht for bug 42307, 2016-12-28 */
/*******************************************************************************
 * Name:    match_boot_and_refuse
 * Purpose: if matchdir is included in boot partition, then refuse
 * Input: 
 *   matchdir: the directory to match with boot partition
 * Return: 
 *   refuse    : 1
 *   no refuse : 0
 * Note: 
 ******************************************************************************/
int
match_boot_and_refuse(char *matchdir)
{
    uint8 len;

    if (sal_strlen(matchdir) > sal_strlen("/mnt/flash/boot"))
        len = sal_strlen("/mnt/flash/boot") + 1;
    else
        len = sal_strlen("/mnt/flash/boot");
    
    if(!sal_strncmp(matchdir, "/mnt/flash/boot/", len))
        return 1;
    else
        return 0;
    
    return 0;
}

int
lib_fs_file_large(char *filename)
{
    struct stat stat_buf;
    
    if (stat(filename, &stat_buf) < 0)
    {
        if(errno == EOVERFLOW)
        {
           /*
             EOVERFLOW
             (stat()) path refers to a file whose size cannot be represented in the type       off_t.
             This can occur when an application compiled on a 32-bit  platform  without  -D_FILE_OFF-
             SET_BITS=64 calls stat() on a file whose size exceeds (2<<31)-1 bits.
           */
            return TRUE;
        }
    }    
    return FALSE;
}
