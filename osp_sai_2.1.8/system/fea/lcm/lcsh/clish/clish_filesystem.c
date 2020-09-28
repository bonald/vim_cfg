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
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <a.out.h>
//#include <sys/param.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>

#include "clish_filesystem.h"
#include "param_check.h"
#include "cli.h"
//#include "glb_define.h"
//#include "glb_util.h"
//#include "glb_types.h"
#include "clish.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
extern int check_filename(const char *pszFilename);

#define XMALLOC(type,size)      malloc(size)
#define XCALLOC(type,size)      calloc(1, size)
#define XFREE(type,ptr)         free(ptr)
#define XREALLOC(type,ptr,size) realloc(ptr, size)
#define XSTRDUP(type,str)       strdup(str)

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
rootdir_t actual_root_dir[] = {
#if HAVE_CF /*modified by weij*/
    {E_DISK_FILE_CFCARD, FULL_NAME_CFCARD,  "cf:"},
#endif
    {E_DISK_FILE_FLASH,  FULL_NAME_FLASH,   "flash:"},
    /* for bug 28631 added by liuyang 2014-5-29*/
#ifndef _CTC_LOONGSON_2F_
    {E_DISK_BOOT_FLASH,  FULL_NAME_FLASH_BOOT,   "flash:/boot"},
#endif    
    /*for bug30239 add by guhg to mount udisk,2014-10-27*/
#ifdef   _CTC_BOARD_SUPPORT_UDISK_
    {E_DISK_FILE_UDISK,  FULL_NAME_UDISK,   "udisk:"},
#endif
};

network_dir_t network_root_dir[NETWORK_MAX_TYPE] = {
    {
        "tftp://",
        "tftp://hostname_or_ip:port/file_name",
        NETWORK_TFTP,
        NULL
    },
#if HAVE_FTP
    {
        "ftp://",
        "ftp://username:password@host:port/file_name",
        NETWORK_FTP,
        NULL
    }
#endif
};

char *
gen_recalc_rootdir(rootdir_t *rdir)
{
    return get_current_logindir();
}

/*******************************************************************************
 * Name:    gen_iter_disk_cfg
 * Purpose: iterate disk cfg array and call pfunc to process it
 * Input: 
 *   pfunc: callback function
 *   arg: callback argument
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
gen_iter_disk_cfg(DISK_ITER_FUNC pfunc, void *arg)
{
    int nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    rootdir_t *rdir = NULL;
    int nIdx = 0;
    
    if (NULL == pfunc)
    {
        return;
    }

    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        pfunc(rdir, arg);
    }
    return;
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
    int i, path_len, dir_len;

    path_len = strlen(url);    
    for (i = 0; i < NETWORK_MAX_TYPE; i ++)
    {
        dir_len = strlen(network_root_dir[i].name);
        /* for bug 5133 del by weij
        if ((path_len >= dir_len)
            && !strncmp(network_root_dir[i].name, url, dir_len))
        {
            return &network_root_dir[i];;
        }
         for bug 5133 add by weij */
         if (path_len >= dir_len)
         {
             if (!strncmp(network_root_dir[i].name, url, dir_len))
             {
                 return &network_root_dir[i];
             }
         }
         else
         {
             if (!strncmp(network_root_dir[i].name, url, path_len))
             {
                 return &network_root_dir[i];
             }
         }
    }    

    return NULL;
}

int
network_name2type(const char *str)
{
    int i, path_len, dir_len;

    path_len = strlen(str);    
    for (i = 0; i < NETWORK_MAX_TYPE; i ++)
    {
        dir_len = strlen(network_root_dir[i].name);
        if ((path_len >= dir_len) && !strncmp(network_root_dir[i].name, str, dir_len))
        {
            return i;
        }
    }    

    return NETWORK_MAX_TYPE;
}

char *
syscmd_get_url(char *para, network_file_t type)
{   
    return (para + strlen(network_root_dir[type].name));
}

int
syscmd_check_actual_directory_exist(struct cli *cli, char * filename)
{
    int nIdx = 0;
    int nValLen = 0;
    int nCnt = sizeof(actual_root_dir)/sizeof(actual_root_dir[0]);
    rootdir_t  * rdir = NULL;

    for(nIdx = 0; nIdx < nCnt; nIdx ++)
    {
        rdir = & actual_root_dir[nIdx];
        nValLen = strlen(filename);
        if(!strncmp(filename, gen_recalc_rootdir(rdir), nValLen))
        {
            if(syscmd_is_directory(filename))
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

int
syscmd_mkdir_protect_root_dir(struct cli * cli, char * filename)
{
    int nIdx = 0;
    int nValLen = 0;
    int nCnt = sizeof(actual_root_dir)/sizeof(actual_root_dir[0]);
    rootdir_t * rdir = NULL;

    for(nIdx = 0; nIdx < nCnt; nIdx ++)
    {
        rdir= & actual_root_dir[nIdx];
        nValLen = strlen(filename);
        if(!strncmp(filename, gen_recalc_rootdir(rdir), nValLen))
        {
            return -1;
        }

    }
    return 0;
}

#define BUFFER_COUNT  4096
int
syscmd_copy_file(char *src_file, char *dst_file)
{
    FILE *src_fp = NULL, *dst_fp = NULL;
    char buf[BUFFER_COUNT];
    int ret = 0, count;
    int w_count = 0;

    if (!strcmp(src_file, dst_file))
    {
        return 0;
    }

    src_fp = fopen(src_file, "r");
    if (NULL == src_fp)
    {
        ret = -1;
        goto error;
    }

    dst_fp = fopen(dst_file, "w+");
    if (NULL == dst_fp)
    {
        ret = -2;
        goto error;
    }

    while ((count = fread(buf, sizeof(char), BUFFER_COUNT, src_fp)) > 0)
    {
        w_count = fwrite(buf, sizeof(char), count, dst_fp);
        /*add by weij for bug 5059*/
        if (w_count < count)
        {
            ret = -2;
            goto error;
        }
    }
error:
    if (NULL != src_fp)
    {
        fclose(src_fp);
    }
    if (NULL != dst_fp)
    {
        fclose(dst_fp);
    }

    return ret;
}

/*******************************************************************************
 * Name:    copy_file_with_progress
 * Purpose: copy file and show progress
 * Input: 
 *   cli:  client pointer
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
copy_file_with_progress(struct cli *cli, char *src_file, char *dst_file)
{
    FILE *src_fp = NULL, *dst_fp = NULL;
    char buf[BUFFER_COUNT];
    int ret = 0, count;
    int amount = 0; 
    int hashbytes = 0;
    struct timeval tstart;
    struct timeval tstop;

    if (!strcmp(src_file, dst_file))
    {
        return 0;
    }

    src_fp = fopen(src_file, "r");
    if (NULL == src_fp)
    {
        ret = -1;
        goto error;
    }

    dst_fp = fopen(dst_file, "w+");
    if (NULL == dst_fp)
    {
        ret = -2;
        goto error;
    }

    (void)gettimeofday(&tstart, NULL);
    while ((count = fread(buf, sizeof(char), BUFFER_COUNT, src_fp)) > 0)
    {
        ret = fwrite(buf, sizeof(char), count, dst_fp);
        if (ret <= 0)
        {
            (void)gettimeofday(&tstop, NULL);
            goto error;
        }
        amount += count;
        while (amount >= hashbytes) 
        {
            cli_out(cli, ".");
            (void) fflush(stdout);
            hashbytes += HASHBYTES;
        }
    }
    (void)gettimeofday(&tstop, NULL);
    
error:
    if (NULL != src_fp)
    {
        fclose(src_fp);
    }
    if (NULL != dst_fp)
    {
        fclose(dst_fp);
    }
    if (amount)
    {
        double  delta;
        
        /* compute delta in 1/10's second units */
        delta = ((tstop.tv_sec * 10.) + (tstop.tv_usec / 100000)) -
                ((tstart.tv_sec * 10.) + (tstart.tv_usec / 100000));
        delta = delta / 10.;    /* back to seconds */
        cli_out(cli, "\n%lu bytes in %.1f seconds, %.0f kbytes/second\n",
                amount, delta, (amount * 1.) / (1024 * delta));
    }
    return ret;
}

/*******************************************************************************
 * Name:    syscmd_is_file
 * Purpose: is this file ?
 * Input: 
 *   filename: file name
 * Output: 
 * Return: 
 *   success: 1
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
syscmd_is_file(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        if (S_ISREG (stat_buf.st_mode))
        {
            return 1;
        }
    }

    return 0;
}

int
syscmd_is_directory(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        if (S_ISDIR (stat_buf.st_mode))
        {
            return 1;
        }
    }
    return 0;
}

int
syscmd_file_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}

int
syscmd_file_can_read(char *filename)
{
    struct stat stat_buf;

    if (!stat(filename, &stat_buf))
    {
        if (stat_buf.st_mode & S_IRUSR)
        {
            return 1;
        }       
    }    
    return 0;
}

int
syscmd_file_can_write(char *filename)
{
    struct stat stat_buf;

    if (!stat(filename, &stat_buf))
    {
        if (stat_buf.st_mode & S_IWUSR)
        {
            return 1;
        }       
    }    
    return 0;
}

char *
syscmd_get_filename(char *path)
{
    char *tmp;

    tmp = strrchr((const char *)path, '/');
    if (NULL == tmp)
    {
        return path;
    }
    else
    {
        return tmp + 1;
    }    
}

/* Added by Alexander Liu 2007-02-28 09:19 */
char *g_gen_last_token = NULL;
int g_gen_partly_match = 0;

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
 * Name:    gen_get_match_status
 * Purpose: get match status
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 1
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
gen_partly_match(void)
{
    return (g_gen_partly_match == 1);
}

void
gen_set_partly_match(int on)
{
    g_gen_partly_match = on;
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
        case cli_token_gflashname:
            if (E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            break;
        case cli_token_gcfcardname:
            if (E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }
            break;
        case cli_token_gfilename:
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
        if (!strncmp(ndir->name, cp, nLen))
        {
            if (1 == child_node)
            {
                /*child_node == 1 means that we need not to match url*/
                return E_GEN_MATCH_ALL;
            }
            return E_GEN_MATCH_NONE;
        }
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
 * Name:    gen_output_helpstr_by_width
 * Purpose: output help string with width limitiation
 * Input: 
 *   cli: client pointer
 *   node: node
 *   max: max width 
 *   width: width
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
gen_output_helpstr_by_width(struct cli *cli, char *cmd, char *help, int max, int width)
{
    char *buf = NULL;
    char *p = NULL;
    int pos = 0;
 
    /* Allocate a temporally help string.  */
    width -= (max + 6);
    buf = XMALLOC(MTYPE_TMP, strlen(help) + 1);

    if (!buf)
        return;

    for (p = help; strlen(p) > width; p += pos + 1)
    {
        for (pos = width; pos > 0; pos--)
        {
            if (' ' == p[pos])
            {
                break;
            }
        }

        if (0 == pos)
        {
            break;
        }

        strncpy (buf, p, pos);
        buf[pos] = '\0';
        
        cli_out (cli, "  %-*s  %s\n", max, cmd, buf);
        cmd = "";
    }

    cli_out (cli, "  %-*s  %s\n", max, cmd, p);
    XFREE (MTYPE_TMP, buf);
    return;
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
int
gen_match_path_callback_output(char *dir, char *str, void *arg)
{
    char szFullName[M_FULLPATH_MAX_LEN];
    
    gen_path_match_arg_t *pmatch = (gen_path_match_arg_t *)arg;
    if (NULL == str || NULL == pmatch || NULL == dir)
    {
        return -1;
    }

    /* trim last '/' */
    if (dir[strlen(dir) - 1] == '/')
    {
        dir[strlen(dir) - 1] = '\0';
    }

    if (!strcmp(dir, gen_recalc_rootdir(pmatch->rdir)))
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", 
                pmatch->rdir->show_name, str);
    }
    else
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s%s/%s", 
                pmatch->rdir->show_name,
                dir + strlen(gen_recalc_rootdir(pmatch->rdir)),
                str);
    }
    pmatch->ptr[*pmatch->pcnt] = XSTRDUP(MTYPE_TMP, szFullName);
    /* added by tanggang 20080109 */
    if (!pmatch->ptr[*pmatch->pcnt])
        return -1;
    *pmatch->pcnt += 1;
    return 0;
}

/*******************************************************************************
 * Name:    gen_match_path_callback_helpstr
 * Purpose: callback to match path and output help string
 * Input: 
 *   arg: count pointer
 * Output: 
 * Return: 
 *   success: 0
 * Note: 
 ******************************************************************************/
int
gen_match_path_callback_helpstr(char *dir, char *str, void *arg)
{
    char szFullName[M_FULLPATH_MAX_LEN];
    gen_path_match_arg_t *pmatch = (gen_path_match_arg_t *)arg;
    int bDir = 0;
    
    if (NULL == str || NULL == pmatch || NULL == dir)
    {
        return -1;
    }

    snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", dir, str);
    bDir = syscmd_is_directory(szFullName);

    /* trim last '/' */
    if (dir[strlen(dir) - 1] == '/')
    {
        dir[strlen(dir) - 1] = '\0';
    }

    if (!strcmp(dir, gen_recalc_rootdir(pmatch->rdir)))
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", 
                pmatch->rdir->show_name, str);
    }
    else
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s%s/%s", 
                pmatch->rdir->show_name,
                dir + strlen(gen_recalc_rootdir(pmatch->rdir)),
                str);
    }
    
    if (pmatch->width <= 0)
    {
        cli_out(pmatch->cli, "  %-*s  %s\n", 
                pmatch->max, szFullName, bDir ? "Directory" : "File");
    }
    else
    {
        gen_output_helpstr_by_width(pmatch->cli, szFullName, 
                bDir ? "Directory" : "File",
                pmatch->max, pmatch->width);
    }

    return 0;
}

/*******************************************************************************
 * Name:    gen_match_path_callback_helpstr_short
 * Purpose: callback to match path and output help string (short format)
 * Input: 
 *   arg: count pointer
 * Output: 
 * Return: 
 *   success: 0
 * Note: 
 ******************************************************************************/
int
gen_match_path_callback_helpstr_short(char *dir, char *str, void *arg)
{
    char szFullName[M_FULLPATH_MAX_LEN];
    gen_path_match_arg_t *pmatch = (gen_path_match_arg_t *)arg;
    
    if (NULL == str || NULL == pmatch || NULL == dir)
    {
        return -1;
    }

    /* trim last '/' */
    if (dir[strlen(dir) - 1] == '/')
    {
        dir[strlen(dir) - 1] = '\0';
    }

    if (!strcmp(dir, gen_recalc_rootdir(pmatch->rdir)))
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s/%s", 
                pmatch->rdir->show_name, str);
    }
    else
    {
        snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s%s/%s", 
                pmatch->rdir->show_name,
                dir + strlen(gen_recalc_rootdir(pmatch->rdir)),
                str);
    }
    
    cli_out(pmatch->cli, "%-*s", 
            pmatch->maxarr[pmatch->firstcnt % pmatch->maxsize] + 2,
            szFullName);
    if (((*pmatch->curcnt) % pmatch->maxsize) == (pmatch->maxsize - 1))
    {
        cli_out(pmatch->cli, "\n");
    }
    *pmatch->curcnt += 1;

    return 0;
}

/*******************************************************************************
 * Name:    gen_match_path_callback_length
 * Purpose: callback to match path and count items
 * Input: 
 *   arg: count pointer
 * Output: 
 * Return: 
 *   success: 0
 * Note: 
 ******************************************************************************/
int
gen_match_path_callback_length(char *dir, char *str, void *arg)
{
    return -1;
}

/*******************************************************************************
 * Name:    gen_match_path_callback_count
 * Purpose: callback to match path and count items
 * Input: 
 *   arg: count pointer
 * Output: 
 * Return: 
 *   success: 0
 * Note: 
 ******************************************************************************/
int
gen_match_path_callback_count(char *dir, char *str, void *arg)
{
    int *cnt = (int *)arg;

    *cnt += 1;
    return 0;
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
void gen_path_getparents(char *name)
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
        if (NULL != (p = strrchr(pathstr, '/')))
        {
            p++;
            if ('\0' != *p)
            {
                snprintf(szSuffix, M_FULLPATH_MAX_LEN, "%s", p);
            }
        }
        else
        {
            snprintf(szSuffix, M_FULLPATH_MAX_LEN, "%s", pathstr);
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
        
        if (strcmp(szFullName, FULL_NAME_FLASH) == 0
        && strcmp(ent->d_name, ".users") == 0)
        {
            /*skip /mnt/flash/.users*/
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
                gen_set_partly_match(1);
                func(szFullName, ent->d_name, pArg);
            }
        }
    }
    
    closedir(dir);
    return 0;
}

/*******************************************************************************
 * Name:    gen_helpstr_len_match_path
 * Purpose: calculate help string length to match path item
 * Input: 
 *   rdir: root item
 * Output: 
 * Return: 
 *   success: length
 *   failed : length for show name of the root item 
 * Note: 
 ******************************************************************************/
int
gen_helpstr_len_match_path(rootdir_t *rdir)
{
    char *pTokStr = gen_get_last_token_str();
    int nMaxLen = 0;
    int nValLen = 0;
    gen_path_match_arg_t matcharg = 
    { 
        .rdir = rdir,
        .plen = &nMaxLen,
    };
    char *pPathStr = NULL;

    if (NULL == rdir)
    {
        return 0;
    }
    nValLen = strlen(rdir->show_name);
    nMaxLen = nValLen;

    if (NULL == pTokStr)
    {
        return nMaxLen;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        return gen_get_last_token_len();
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
    
    gen_match_path_main(gen_recalc_rootdir(rdir), pPathStr,
                        gen_match_path_callback_length, (void *)&matcharg);
    return nMaxLen;
}

/*******************************************************************************
 * Name:    gen_helpstr_output_match_path
 * Purpose: output help string length to match path item
 * Input: 
 *   cli: client pointer
 *   rdir: root item
 *   max: max witdh
 *   width: width
 * Output: 
 * Return: 
 *   success: length
 *   failed : length for show name of the root item 
 * Note: 
 ******************************************************************************/
int
gen_helpstr_output_match_path(struct cli *cli, rootdir_t *rdir, int max, int width)
{
    char *pTokStr = gen_get_last_token_str();
    int nMaxLen = 0;
    int nValLen = 0;
    gen_path_match_arg_t matcharg = 
    { 
        .rdir = rdir,
        .cli = cli,
        .max = max, 
        .width = width,
    };
    char *pPathStr = NULL;

    if (NULL == rdir)
    {
        return 0;
    }
    nValLen = strlen(rdir->show_name);
    nMaxLen = nValLen;

    if (NULL == pTokStr)
    {
        return nMaxLen;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        return gen_get_last_token_len();
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
    
    gen_match_path_main(gen_recalc_rootdir(rdir), pPathStr,
                        gen_match_path_callback_helpstr, (void *)&matcharg);
    return nMaxLen;
}

/*******************************************************************************
 * Name:    gen_helpstr_output_match_path_short
 * Purpose: output help string length to match path item (short format)
 * Input: 
 *   cli: client pointer
 *   rdir: root item
 *   max: max witdh array
 *   maxsize: current array index
 * Output: 
 * Return: 
 *   success: length
 *   failed : length for show name of the root item 
 * Note: 
 ******************************************************************************/
int
gen_helpstr_output_match_path_short(struct cli *cli, rootdir_t *rdir, 
                                    int *cnt, int *max, int maxsize)
{
    char *pTokStr = gen_get_last_token_str();
    int nMaxLen = 0;
    int nValLen = 0;
    gen_path_match_arg_t matcharg = 
    { 
        .rdir = rdir,
        .cli= cli,
        .firstcnt = *cnt,
        .curcnt = cnt,
        .maxarr = max,
        .maxsize = maxsize,
    };
    char *pPathStr = NULL;

    if (NULL == rdir)
    {
        return 0;
    }
    nValLen = strlen(rdir->show_name);
    nMaxLen = nValLen;

    if (NULL == pTokStr)
    {
        return nMaxLen;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        return gen_get_last_token_len();
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
    
    gen_match_path_main(gen_recalc_rootdir(rdir), pPathStr,
                        gen_match_path_callback_helpstr_short, (void *)&matcharg);
    return nMaxLen;
}

/*******************************************************************************
 * Name:    gen_refine_helpstr_len
 * Purpose: recalculate help string length
 * Input: 
 *   str:  keyword string
 * Output: 
 * Return: 
 *   success: modified length
 *   failed : 0
 * Note: 
 ******************************************************************************/
int gen_refine_helpstr_len(char *str)
{
    return -1;
}
   
/*******************************************************************************
 * Name:    gen_output_helpstr
 * Purpose: check and output help string
 * Input: 
 *   cli: client pointer
 *   node: command node
 *   max: max length
 *   width: current width
 * Output: 
 * Return:
 *   success: 0
 *   failed:  -1
 * Note: 
 ******************************************************************************/
int
gen_output_helpstr(struct cli *cli, struct cli_node *node,
                   int max, int width)
{
    int nTokenLen = gen_get_last_token_len();
    rootdir_t *rdir = NULL;
    int nValLen = 0;
    int nCnt = 0;
    int nIdx = 0;

    if (NULL == cli || NULL == node)
    {
        return -1;
    }

    if (!strcmp(node->str, M_GEN_FILENAME_STR)
        || !strcmp(node->str, M_GEN_FLASHNAME_STR)
        || !strcmp(node->str, M_GEN_CFCARDNAME_STR))
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            if (!strcmp(node->str, M_GEN_FLASHNAME_STR)
                 && E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            if (!strcmp(node->str, M_GEN_CFCARDNAME_STR) 
                && E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }

            if (nTokenLen)
            {
                if (gen_match_last_token(rdir->show_name) != 0)
                {
                    continue;
                }
                nValLen = strlen(rdir->show_name);
                if (nValLen <= nTokenLen)
                {
                    gen_helpstr_output_match_path(cli, &actual_root_dir[nIdx], 
                            max, width);
                    continue;
                }
            }
            
            if (width <= 0)
            {
                cli_out(cli, "  %-*s  %s\n", max, 
                        rdir->show_name, node->help);
                continue;
            }

            gen_output_helpstr_by_width(cli, rdir->show_name,
                                        node->help, max, width);
        }
        return 0;
    }
    
    if (!strcmp(node->str, M_GEN_URLNAME_STR))
    {
        nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            if (nTokenLen)
            {
                if (gen_match_last_token(network_root_dir[nIdx].name) != 0)
                {
                    continue;
                }
                nValLen = strlen(network_root_dir[nIdx].name);
                if (nValLen <= nTokenLen)
                {
                    continue;
                }
            }

            if (width <= 0)
            {
                cli_out(cli, "  %-*s  %s\n", max, 
                        network_root_dir[nIdx].name, node->help);
                continue;
            }

            gen_output_helpstr_by_width(cli, network_root_dir[nIdx].name,
                                        node->help, max, width);
        }
        return 0;
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_output_helpstr_short
 * Purpose:  check and output help string (short format)
 * Input: 
 *   cli: client
 *   str: string
 *   max: max array
 *   maxsize: max array size
 * Output: 
 *   cnt: cnt pointer
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
gen_output_helpstr_short(struct cli *cli, char *str, int *cnt, int *max, int maxsize)
{
    int nTokenLen = gen_get_last_token_len();
    rootdir_t *rdir = NULL;
    int nValLen = 0;
    int nCnt = 0;
    int nIdx = 0;
    int nFirstCnt = *cnt;

    if (NULL == cli)
    {
        return -1;
    }

    if (!strcmp(str, M_GEN_FILENAME_STR)
        || !strcmp(str, M_GEN_FLASHNAME_STR)
        || !strcmp(str, M_GEN_CFCARDNAME_STR))
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            if (!strcmp(str, M_GEN_FLASHNAME_STR)
                 && E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            if (!strcmp(str, M_GEN_CFCARDNAME_STR) 
                && E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }

            if (nTokenLen)
            {
                if (gen_match_last_token(rdir->show_name) != 0)
                {
                    continue;
                }

                nValLen = strlen(rdir->show_name);
                if (nValLen <= nTokenLen)
                {
                    gen_helpstr_output_match_path_short(cli, &actual_root_dir[nIdx], 
                            cnt, max, maxsize);
                    continue;
                }
            }
            cli_out (cli, "%-*s", max[nFirstCnt % maxsize] + 2,
                     rdir->show_name);
            if ((*cnt % maxsize) == (maxsize - 1))
            {
                cli_out (cli, "\n");
            }
            *cnt += 1;
        }
        return 0;
    }
    
    if (!strcmp(str, M_GEN_URLNAME_STR))
    {
        nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            if (nTokenLen)
            {
                if (gen_match_last_token(network_root_dir[nIdx].name) != 0)
                {
                    continue;
                }
                nValLen = strlen(network_root_dir[nIdx].name);
                if (nValLen <= nTokenLen)
                {
                    continue;
                }
            }
            cli_out (cli, "%-*s", max[nFirstCnt % maxsize] + 2,
                     network_root_dir[nIdx].name);
            if ((*cnt % maxsize) == (maxsize - 1))
            {
                cli_out (cli, "\n");
            }
            *cnt += 1;
        }
        return 0;
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_match_path_count
 * Purpose: get matched count for path
 * Input: 
 *   rdir: path
 * Output: 
 * Return: 
 *   success: count
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
gen_match_path_count(rootdir_t *rdir)
{
    int nRetCnt = 0;
    char *pTokStr = gen_get_last_token_str();
    char *pPathStr = NULL;

    if (NULL == rdir || NULL == pTokStr)
    {
        return 0;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        return 1;
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

    gen_match_path_main(gen_recalc_rootdir(rdir), pPathStr,
                        gen_match_path_callback_count, (void *)&nRetCnt);
    if (0 == nRetCnt)
    {
        return 0;
    }

    return nRetCnt;
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
gen_output_match_path(rootdir_t *rdir, char **ptr, int *pnItems)
{
    char *pTokStr = gen_get_last_token_str();
    char *pPathStr = NULL;
    gen_path_match_arg_t matcharg = 
    { 
        .ptr = ptr,
        .pcnt = pnItems,
        .rdir = rdir,
    };
    int nOldCnt = 0;

    if (NULL == rdir || NULL == pTokStr)
    {
        return -1;
    }
    
    if (strlen(pTokStr) < strlen(rdir->show_name))
    {
        /* let just match pTokStr */
        ptr[*pnItems] = XSTRDUP (MTYPE_TMP, pTokStr);
        if( !ptr[*pnItems] )
          return -1;	
        *pnItems += 1;
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

    nOldCnt = *matcharg.pcnt;
    gen_match_path_main(gen_recalc_rootdir(rdir), pPathStr,
                        gen_match_path_callback_output, (void *)&matcharg);
    if (nOldCnt >= *matcharg.pcnt)
    {
        /* let just match pTokStr */
        ptr[*pnItems] = XSTRDUP (MTYPE_TMP, pTokStr);
        if( !ptr[*pnItems] )
          return -1;	
        *pnItems += 1;
        return 0;
    }

    return 0;
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
gen_output_matchstr(char **ptr, char *oldtext, char *type, int *nCntRet)
{
    int nCnt = 0;
    int nIdx = 0;
    int nTokenLen = gen_get_last_token_len();
    int nValLen = 0;
    rootdir_t *rdir = NULL;

    *nCntRet = 0;
    if (!strcmp(type, M_GEN_FILENAME_STR)
        || !strcmp(type, M_GEN_FLASHNAME_STR)
        || !strcmp(type, M_GEN_CFCARDNAME_STR))
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            if (!strcmp(type, M_GEN_FLASHNAME_STR)
                 && E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            if (!strcmp(type, M_GEN_CFCARDNAME_STR) 
                && E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }
            
            if (gen_match_last_token(rdir->show_name) == 0)
            {
                nValLen = strlen(rdir->show_name);
                if (nValLen > nTokenLen)
                {
                    ptr[*nCntRet] = XSTRDUP(MTYPE_TMP, rdir->show_name);
                    if( !ptr[*nCntRet] )
                      continue ;	
                    *nCntRet += 1;
                    gen_set_partly_match(1);
                }
                else
                {
                    /* search the path and output matched items */
                    gen_output_match_path(&actual_root_dir[nIdx], ptr, nCntRet);
                }
            }
        }
        
        return;
    }

    if (!strcmp(type, M_GEN_URLNAME_STR))
    {
        nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            if (gen_match_last_token(network_root_dir[nIdx].name) == 0)
            {
                nValLen = strlen(network_root_dir[nIdx].name);
                if ((nValLen >= nTokenLen) || nTokenLen <= 0)
                {
                    ptr[*nCntRet] = XSTRDUP (MTYPE_TMP, network_root_dir[nIdx].name);
                    if( !ptr[*nCntRet] )
                      continue ;	
                    *nCntRet += 1;
                    gen_set_partly_match(1);
                }
                else
                {
                    ptr[*nCntRet] = XSTRDUP(MTYPE_TMP, gen_get_last_token_str());
                    if( !ptr[*nCntRet] )
                      continue ;	
                    *nCntRet += 1;
                    gen_set_partly_match(1);
                    return;
                }
            }
        }

        return;
    }

    ptr[0] = XSTRDUP (MTYPE_TMP, oldtext);
    if( ! ptr[0] )
      return ;	
    *nCntRet = 1;
    assert(0);
    return;
}

/*******************************************************************************
 * Name:    gen_get_match_count
 * Purpose: get match count for last token
 * Input: 
 *   type: type string
 * Output: 
 * Return: 
 *   success: count
 *   failed : 0
 * Note: 
 ******************************************************************************/
int
gen_get_match_count(char *type)
{
    int nMatchCnt = 0;
    int nCnt = 0;
    int nIdx = 0;
    int nTokenLen = gen_get_last_token_len();
    int nValLen = 0;
    rootdir_t *rdir = NULL;

    if (!strcmp(type, M_GEN_FILENAME_STR)
        || !strcmp(type, M_GEN_FLASHNAME_STR)
        || !strcmp(type, M_GEN_CFCARDNAME_STR))
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            if (!strcmp(type, M_GEN_FLASHNAME_STR)
                && E_DISK_FILE_FLASH != rdir->etype)
            {
                continue;
            }
            if (!strcmp(type, M_GEN_CFCARDNAME_STR) 
                && E_DISK_FILE_CFCARD != rdir->etype)
            {
                continue;
            }

            if (gen_match_last_token(rdir->show_name) == 0)
            {
                nValLen = strlen(rdir->show_name);
                if (nValLen > nTokenLen)
                {
                    nMatchCnt++;
                }
                else
                {
                    /* search the path and count items */
                    nMatchCnt += gen_match_path_count(&actual_root_dir[nIdx]);
                }
            }
        }
        
        return nMatchCnt;
    }
    
    if (!strcmp(type, M_GEN_URLNAME_STR))
    {
        nCnt = sizeof(network_root_dir) / sizeof(network_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            if (gen_match_last_token(network_root_dir[nIdx].name) == 0)
            {
                nMatchCnt++;
            }
        }

        return nMatchCnt;
    }

    assert(0);
    return nMatchCnt;
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

    if (strlen(pszPath) > M_FULLPATH_MAX_LEN)
    {
        return -1;
    }

    if (NULL != strstr(pszPath, "//"))
    {
        return -1;
    }

    if (split_tokens(pszPath, strlen(pszPath), M_FULLPATH_DEPTH_MAX,
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
        if (check_filename(ppNameArr[i]) != 0)
        {
            nRet = -1;
            break;
        }
    }
    free_tokens(&ppNameArr);
    return nRet;
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

    if (strlen(pszFullName) > M_FULLPATH_MAX_LEN)
    {
        return -1;
    }

    pszTmpName = pszFullName;
    
    if (strchr (pszFullName, ':') != NULL)
    {
        nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
        for (nIdx = 0; nIdx < nCnt; nIdx++)
        {
            rdir = &actual_root_dir[nIdx];
            nValLen = strlen(rdir->show_name);
            if (!strncmp(pszFullName, rdir->show_name,
                             strlen(rdir->show_name)))
            {
                if (!strcmp(pszFullName, rdir->show_name))
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
 * Name:    gen_check_and_get_filename
 * Purpose: check and return absoulte filename 
 * Input: 
 *   cli: client pointer
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
gen_check_and_get_filename(struct cli *cli, char *filename, char *outfile, size_t outsize)
{
    struct line *line = NULL;
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
    if (NULL == cli)
    {
        return -1;
    }

    if (strlen(filename) >= M_FULLPATH_MAX_LEN)
    {
        cli_out(cli, "%% File or directory name length overflow.\n");
        return -1;
    }

    line = cli->line;
    
    snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s", filename);
    
    if (gen_validate_relative_path(szFullName) != 0)/* Modified by Percy Wang 2007-12-21 */
    {
        return -1;
    }
    
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
                        gen_recalc_rootdir(rdir));
            }
            else if ('/' == szFullName[nValLen])
            {
                snprintf(outfile, outsize, "%s%s",
                        gen_recalc_rootdir(rdir), szFullName + nValLen);
                if ((strlen(gen_recalc_rootdir(rdir)) + strlen(szFullName + nValLen)) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    cli_out(cli, "%% File or directory name length overflow.\n");
                    return -1;
                }
            }
            else
            {
                snprintf(outfile, outsize, "%s/%s",
                        gen_recalc_rootdir(rdir), szFullName + nValLen);
                if ((strlen(gen_recalc_rootdir(rdir)) + strlen(szFullName + nValLen) + 1) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    cli_out(cli, "%% File or directory name length overflow.\n");
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
    if (NULL == line)
    {
        if (getcwd(szFullName, M_FULLPATH_MAX_LEN) == NULL)
        {
            if (NULL != cli)
            {
                cli_out(cli, "%% Get current working directory failed: %s\n",
                        strerror(errno));
            }
            return -1;
        }
    }

    nLen = strlen(szFullName);
    if ('/' != filename[0])
    {
        snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "/%s", filename);
        if ((nLen + strlen(filename) + 1) >= M_FULLPATH_MAX_LEN)
        {
            cli_out(cli, "%% File or directory name length overflow.\n");
            return -1;
        }
    }
    else
    {
        snprintf(szFullName + nLen, M_FULLPATH_MAX_LEN - nLen, "%s", filename);
        if ((nLen + strlen(filename)) >= M_FULLPATH_MAX_LEN)
        {
            cli_out(cli, "%% File or directory name length overflow.\n");
            return -1;
        }
    }
    gen_path_getparents(szFullName);
    nLen = strlen(szFullName);
    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        nValLen = strlen(gen_recalc_rootdir(rdir));
        if (!strncmp(szFullName, gen_recalc_rootdir(rdir),
                         strlen(gen_recalc_rootdir(rdir))))
        {
            snprintf(outfile, outsize, "%s", szFullName);

            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
        if (!strncmp(szFullName, gen_recalc_rootdir(rdir), nLen))
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
 *   cli: client pointer
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
gen_check_and_get_filename_bytype(struct cli *cli, char *filename, char *outfile,
                                  size_t outsize, int type)
{
    rootdir_t *rdir = NULL;
    int nCnt = 0;
    int nIdx = 0;
    
    if (NULL == cli || NULL == filename || NULL == outfile || 0 > outsize)
    {
        return -1;
    }

    switch (type)
    {
    case E_DISK_FILE_CFCARD:
        break;
    case E_DISK_FILE_FLASH:
        break;
    case E_DISK_FILE_UDISK:
    default:
        assert(0);
        break;
    }

    if (gen_check_and_get_filename(cli, filename, outfile, outsize) != 0)
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

        if (!strncmp(outfile, gen_recalc_rootdir(rdir), strlen(gen_recalc_rootdir(rdir))))
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
        if (!strncmp(szOrig, gen_recalc_rootdir(rdir),
                         strlen(gen_recalc_rootdir(rdir))))
        {
            if (!strcmp(szOrig, gen_recalc_rootdir(rdir)))
            {
                snprintf(szShow, M_FULLPATH_MAX_LEN,
                        "%s/", rdir->show_name);
            }
            else if ('/' == szOrig[strlen(gen_recalc_rootdir(rdir))])
            {
                snprintf(szShow, M_FULLPATH_MAX_LEN, "%s%s",
                        rdir->show_name,
                        szOrig + strlen(gen_recalc_rootdir(rdir)));
            }
            else
            {
                snprintf(szShow, M_FULLPATH_MAX_LEN, "%s/%s",
                        rdir->show_name,
                        szOrig + strlen(gen_recalc_rootdir(rdir)));
            }
            return 0;
        }
    }

    return -1;

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

/*******************************************************************************
 * Name:    gen_send_to_url
 * Purpose: send file to url
 * Input: 
 *   cli: client pointer
 *   szSrc: file to send
 *   url: url
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
int
gen_send_to_url(struct cli *cli, char *szSrc, char *url)
{
    network_dir_t *ndir = NULL;
    network_op_t *ops = NULL;
    network_handle_t hdl;
    int ret = CLI_SUCCESS;
    int sret = E_GEN_NETWORK_SUCCESS;

    GEN_NETWORK_HANDLE_INIT(&hdl);
    if ((ndir = network_name2ops(url)) == NULL)
    {
        cli_out(cli, "%% Invalid URL: %s\n", url);
        return CLI_ERROR;
    }

    if (NULL == ndir->ops)
    {
        cli_out(cli, "%% Unimplemented method: %s\n", ndir->name);
        return CLI_ERROR;
    }
    ops = ndir->ops;
    hdl.ndir = ndir;
    
    /* init handle for this process */
    if (ops->init && ops->init(&hdl) != 0)
    {
        cli_out(cli, "%% Init %s failed\n", ndir->name);
        return CLI_ERROR;
    }

    /* parse url to get it parameters */
    if (ops->parseurl)
    {
        if (ops->parseurl(&hdl, url) != 0)
        {
            /* parse url failed, we will get it interactive */
            ret = CLI_ERROR;
        }
    }
    else
    {
        ret = CLI_ERROR;
    }

    /* get parameters interactive if needed */
    if (CLI_SUCCESS != ret)
    {
        if (ops->interactive_get_param 
            && ops->interactive_get_param(&hdl, cli) == 0)
        {
            ret = CLI_SUCCESS;
        }
    }
    if (CLI_SUCCESS != ret)
    {
        cli_out(cli, "%% Get parameters failed. The format is: %s\n",
                ndir->usage);
        goto error_out;
    }

    /* let's do sendto */
    if (NULL == ops->sendto)
    {
        cli_out(cli, "%% Incomplete implement %s\n", ndir->name);
        ret = CLI_ERROR;
        goto error_out;
    }

    if ((sret = ops->sendto(&hdl, szSrc, cli)) != E_GEN_NETWORK_SUCCESS)
    {
        cli_out(cli, "%% Send file %s\n", gen_network_get_errmsg(sret));
        ret = CLI_ERROR;
        goto error_out;
    }
    
error_out:
    if (ops->release)
    {
        ops->release(&hdl);
    }
    return ret;
}

/*******************************************************************************
 * Name:    gen_recv_from_url
 * Purpose: recv file from url
 * Input: 
 *   cli: client pointer
 *   szDst: dest file
 *   url: url
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS 
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
int
gen_recv_from_url(struct cli *cli, char *szDst, char *url)
{
    network_dir_t *ndir = NULL;
    network_op_t *ops = NULL;
    network_handle_t hdl;
    int ret = CLI_SUCCESS;
    int sret = E_GEN_NETWORK_SUCCESS;
    char szTmpName[] = "/tmp/recvXXXXXX";

    GEN_NETWORK_HANDLE_INIT(&hdl);
    if ((ndir = network_name2ops(url)) == NULL)
    {
        cli_out(cli, "%% Invalid URL: %s\n", url);
        return CLI_ERROR;
    }

    if (NULL == ndir->ops)
    {
        cli_out(cli, "%% Unimplemented method: %s\n", ndir->name);
        return CLI_ERROR;
    }
    ops = ndir->ops;
    hdl.ndir = ndir;

    if (ctc_create_tempfile(szTmpName) != 0)
    {
        cli_out (cli, "%% Generate temporary file failed: %s\n", strerror(errno));
        unlink(szTmpName);
        return CLI_ERROR;
    }
    
    /* init handle for this process */
    if (ops->init && ops->init(&hdl) != 0)
    {
        cli_out(cli, "%% Init %s failed\n", ndir->name);
        return CLI_ERROR;
    }

    /* parse url to get it parameters */
    if (ops->parseurl)
    {
        if (ops->parseurl(&hdl, url) != 0)
        {
            /* parse url failed, we will get it interactive */
            ret = CLI_ERROR;
        }
    }
    else
    {
        ret = CLI_ERROR;
    }

    /* get parameters interactive if needed */
    if (CLI_SUCCESS != ret)
    {
        if (ops->interactive_get_param 
            && ops->interactive_get_param(&hdl, cli) == 0)
        {
            ret = CLI_SUCCESS;
        }
    }
    if (CLI_SUCCESS != ret)
    {
        cli_out(cli, "%% Get parameters failed. The format is: %s\n",
                ndir->usage);
        goto error_out;
    }
    /* delete by weij for bug 4662 */
    if (ops->append_dir && (ops->append_dir(&hdl, cli, szDst) == 0))
    {
        ret = CLI_SUCCESS;
    }
    else
    {
        ret = CLI_ERROR;
    }

    if (CLI_SUCCESS != ret)
    {
        goto error_out;
    }

    /* let's do recvfrom*/
    if (NULL == ops->recvfrom)
    {
        cli_out(cli, "%% Incomplete implement %s\n", ndir->name);
        ret = CLI_ERROR;
        goto error_out;
    }

    cli_out(cli, "Download from URL to temporary file.\n");
    if ((sret = ops->recvfrom(&hdl, szTmpName, cli)) != E_GEN_NETWORK_SUCCESS)
    {
        cli_out(cli, "%% Get file %s\n", gen_network_get_errmsg(sret));
        ret = CLI_ERROR;
        goto error_out;
    }
/* begin added by zongyi for bug 5409 */
    {
        int nRet = 0;       
        int64_t llsize = 0ll;
        
        /* zongyi add for existent file check */
        if (syscmd_file_exist(szDst))
        {
            FILE* fp = fopen(szDst, "r" );
            if (fp == NULL) 
            {
                cli_out(cli, "%% Open file failed \n");
                ret = CLI_ERROR;        
                goto error_out;        
            }
      
            fseek(fp, 0L, SEEK_END ); 
            llsize = (int64_t)ftell(fp);
            fclose(fp);
        }
        
        nRet = check_space_available(szTmpName, llsize);
        if (nRet == -1)
        {
            cli_out(cli, "%% Get free space on device failed.\n");
            ret = CLI_ERROR;        
            goto error_out;         
        }
        else if (nRet == -2)
        {
            cli_out(cli, "%% Open temporary file failed\n");
            ret = CLI_ERROR;        
            goto error_out;         
        }
        else if (nRet == -3)
        {
            cli_out(cli, "%% No enough space left on device.\n"); 
            ret = CLI_ERROR;        
            goto error_out;   
        }
    }
/* end add */
    cli_out(cli, "Copy the temporary file to its destination.\n");
    if (copy_file_with_progress(cli, szTmpName, szDst) < 0)
    {
        cli_out(cli, "%% Copy file failed %s\n", strerror(errno));
        ret = CLI_ERROR;
    }
    
error_out:
    if (ops->release)
    {
        ops->release(&hdl);
    }
    unlink(szTmpName);
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
    
    for (idx = 0; idx < nCnt; idx++)
    {
        ndir = &network_root_dir[idx];
        if (etype == ndir->network_file)
        {
            ndir->ops = ops;
            return 0;
        }
    }
    return -1;
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
    if (fseek(fp, (off_t)0, SEEK_SET))
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
    
    if (strlen(filename) >= M_FULLPATH_MAX_LEN)
    {
        return -1;
    }
    
    snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s", filename);
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
                        gen_recalc_rootdir(rdir));
            }
            else if ('/' == szFullName[nValLen])
            {
                snprintf(outfile, outsize, "%s%s",
                        gen_recalc_rootdir(rdir), szFullName + nValLen);
                if ((strlen(gen_recalc_rootdir(rdir)) + strlen(szFullName + nValLen) + 1) 
                        >= M_FULLPATH_MAX_LEN)
                {
                    return -1;
                }
            }
            else
            {
                snprintf(outfile, outsize, "%s/%s",
                        gen_recalc_rootdir(rdir), szFullName + nValLen);
                if ((strlen(gen_recalc_rootdir(rdir)) + strlen(szFullName + nValLen)) 
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
        nValLen = strlen(gen_recalc_rootdir(rdir));
        if (!strncmp(szFullName, gen_recalc_rootdir(rdir),
                         strlen(gen_recalc_rootdir(rdir))))
        {
            snprintf(outfile, outsize, "%s", szFullName);
            if (gen_validate_path(outfile) != 0)
            {
                return -1;
            }
            return 0;
        }
        if (!strncmp(szFullName, gen_recalc_rootdir(rdir), nLen))
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
 * Name:    gen_is_file_system_root
 * Purpose: check to see if this directory is file system root
 * Input: 
 *   pszName: directory name
 * Output: 
 * Return: 
 *   yes: 0
 *   no : -1
 * Note: 
 ******************************************************************************/
int
gen_is_file_system_root(char *pszName)
{
    rootdir_t *rdir = NULL;
    char szFullName[M_FULLPATH_MAX_LEN];
    int nCnt = 0;
    int nIdx = 0;
    int nLen = 0;
    char *p = NULL;

    if (NULL == pszName)
    {
        return 0;
    }

    snprintf(szFullName, M_FULLPATH_MAX_LEN, "%s", pszName);
    nLen = strlen(szFullName);
    p = szFullName + nLen - 1;
    while ('/' == *p && p >= szFullName)
    {
        *p = '\0';
        p--;
    }
    if ('\0' == szFullName[0])
    {
        return 0;
    }
    gen_path_getparents(szFullName);

    nCnt = sizeof(actual_root_dir) / sizeof(actual_root_dir[0]);
    for (nIdx = 0; nIdx < nCnt; nIdx++)
    {
        rdir = &actual_root_dir[nIdx];
        if (!strcasecmp(szFullName, gen_recalc_rootdir(rdir)))
        {
            return 1;
        }
    }

    return 0;
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
        nLen = strlen(gen_recalc_rootdir(rdir));
        if (!strncmp(fullname, gen_recalc_rootdir(rdir), nLen))
        {
            full_char = nLen + full_char;
            nLen = strlen(rdir->show_name);
            snprintf(showname, nLen + 1, "%s", rdir->show_name);
            show_char += nLen;
            snprintf(show_char, M_FULLPATH_MAX_LEN - nLen, "%s", full_char);
            return 0;
        }
    }
    return -1;
}

/* Modified by Percy Wang 2007-12-27 */
   
/* Return 0 if dequoted pattern matches S in the current locale. */
static int 
strcompare (char *p, char *pe, char *s, char * se)
{
    int ret;
    char c1, c2;

    c1 = *pe;
    c2 = *se;

    *pe = *se = '\0';
    ret = strcmp (p, s);

    *pe = c1;
    *se = c2;

    return (ret == 1 ? ret : 0);
}

static char *
patscan (char *string, char *end, int delim)
{
    int pnest, bnest;
    char *s, c;
    c = 0;
    pnest = bnest = 0;
    for (s = string; c !='a'; s++)
    {
        c = *s;
        switch (c)
        {
        case '\0':
            return ((char *)0);
        case '[':
            bnest++;
            break;
        case ']':
            if (bnest)
            {
                bnest--;
            }
            break;
        case '(':
            if (bnest == 0)
            {
                pnest++;
            }
            break;
        case ')':
            if (bnest == 0)
            {
                pnest--;
            }
            if (pnest <= 0)
            {
                return ++s;
            }
            break;
        case '|':
            if (bnest == 0 && pnest == 0 && delim == '|')
            {
                return ++s;
            }
            break;
        }
    }
    return (char *)0;
}

static int extmatch (int xc, char *s, char *se, char *p, char *pe, int flags);

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, FALSE if not.  */
static int
gmatch ( char *string,  char *se,  char *pattern,  char *pe, int flags)
{
  register char *p, *n;		/* pattern, string */
  register char c;		/* current pattern character */
  register char sc;		/* current string character */

  p = pattern;
  n = string;

  if (string == 0 || pattern == 0)
    return 0;

  while (p < pe)
    {
      c = *p++;
      c = FOLD (c);

      sc = n < se ? *n : '\0';

    /* extmatch () will handle recursively calling gmatch, so we can
    just return what extmatch() returns. */
    if ((flags & FNM_EXTMATCH) && *p == '(' &&
    (c == '+' || c == '*' || c == '?' || c == '@' || c == '!')) /* ) */
    {
        int lflags;
        /* If we're not matching the start of the string, we're not
         concerned about the special cases for matching `.' */
        lflags = (n == string) ? flags : (flags & ~FNM_PERIOD);
        return (extmatch (c, n, se, p, pe, lflags));
    }

      switch (c)
	{
	case '\\':		/* backslash escape removes special meaning */
	  if (p == pe)
	    return 0;

	  if ((flags & FNM_NOESCAPE) == 0)
	    {
	      c = *p++;
	      /* A trailing `\' cannot match. */
	      if (p > pe)
		return 0;
	      c = FOLD (c);
	    }
	  if (FOLD (sc) != (unsigned char)c)
	    return 0;
	  break;

	case '*':		/* Match zero or more characters */
	  if (p == pe)
	    return 1;
	  
//	  if ((flags & FNM_PERIOD) && sc == '.' &&
//	      (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
	    /* `*' cannot match a `.' if it is the first character of the
	       string or if it is the first character following a slash and
	       we are matching a pathname. */
//	    return 0;

	  /* Collapse multiple consecutive, `*' and `?', but make sure that
	     one character of the string is consumed for each `?'. */
	  for (c = *p++; (c == '?' || c == '*'); c = *p++)
	    {
	      if ((flags & FNM_PATHNAME) && sc == '/')
		/* A slash does not match a wildcard under FNM_PATHNAME. */
		return 0;
	      else if (c == '?')
		{
		  if (sc == '\0')
		    return 0;
		  /* One character of the string is consumed in matching
		     this ? wildcard, so *??? won't match if there are
		     fewer than three characters. */
		  n++;
		  sc = n < se ? *n : '\0';
		}

	      /* Handle ******(patlist) */
	      if ((flags & FNM_EXTMATCH) && c == '*' && *p == '(')  /*)*/
		{
		  char *newn;
		  /* We need to check whether or not the extended glob
		     pattern matches the remainder of the string.
		     If it does, we match the entire pattern. */
		  for (newn = n; newn < se; ++newn)
		    {
		      if (extmatch (c, newn, se, p, pe, flags) == 0)
			return (1);
		    }
		  /* We didn't match the extended glob pattern, but
		     that's OK, since we can match 0 or more occurrences.
		     We need to skip the glob pattern and see if we
		     match the rest of the string. */
		  newn = patscan (p, pe, 0);
		  p = newn;
		}
	      if (p == pe)
	        break;
	    }

	  /* If we've hit the end of the pattern and the last character of
	     the pattern was handled by the loop above, we've succeeded.
	     Otherwise, we need to match that last character. */
	  if (p == pe && (c == '?' || c == '*'))
	    return (1);

	  /* General case, use recursion. */
	  {
	    unsigned char c1;

	    c1 = (unsigned char)((flags & FNM_NOESCAPE) == 0 && c == '\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; n < se; ++n)
	      {
		/* Only call fnmatch if the first character indicates a
		   possible match.  We can check the first character if
		   we're not doing an extended glob match. */
		if ((flags & FNM_EXTMATCH) == 0 && c != '[' && FOLD (*n) != c1)
		  continue;

		/* If we're doing an extended glob match and the pattern is not
		   one of the extended glob patterns, we can check the first
		   character. */
		if ((flags & FNM_EXTMATCH) && p[1] != '(' && /*)*/
		    strchr ("?*+@!", *p) == 0 && c != '[' && FOLD (*n) != c1)
		  continue;

		/* Otherwise, we just recurse. */
		if (gmatch (n, se, p, pe, flags & ~FNM_PERIOD) == 1)
		  return (1);
	      }
	    return 0;
	  }
	default:
	  if ((unsigned char)c != FOLD (sc))
	    return (0);
	}

      ++n;
    }

  if (n == se)
    return (1);

  if ((flags & FNM_LEADING_DIR) && *n == '/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 1;
          
  return (0);
}

static int
extmatch (int xc, char *s, char *se, char *p, char *pe, int flags)
{
  char *prest;			/* pointer to rest of pattern */
  char *psub;			/* pointer to sub-pattern */
  char *pnext;			/* pointer to next sub-pattern */
  char *srest;			/* pointer to rest of string */
  int m1=0;
  int m2=0;
  
  switch (xc)
    {
    case '+':			/* match one or more occurrences */
    case '*':			/* match zero or more occurrences */
      prest = patscan (p, pe, 0);
      if (prest == 0)
	/* If PREST is 0, we failed to scan a valid pattern.  In this
	   case, we just want to compare the two as strings. */
	return (strcompare (p - 1, pe, s, se));

      /* If we can get away with no matches, don't even bother.  Just
	 call gmatch on the rest of the pattern and return success if
	 it succeeds. */
      if ((xc == '*') && (gmatch (s, se, prest, pe, flags) == 1))
	return 0;

      /* OK, we have to do this the hard way.  First, we make sure one of
         the subpatterns matches, then we try to match the rest of the
         string. */
      for (psub = p + 1; ; psub = pnext)
	{
	  pnext = patscan (psub, pe, '|');
	  for (srest = s; srest <= se; srest++)
	    {
	      /* Match this substring (S -> SREST) against this
		 subpattern (psub -> pnext - 1) */
	      m1 = gmatch (s, srest, psub, pnext - 1, flags) == 1;
	      /* OK, we matched a subpattern, so make sure the rest of the
		 string matches the rest of the pattern.  Also handle
		 multiple matches of the pattern. */
	      if (m1)
		m2 = (gmatch (srest, se, prest, pe, flags) == 1) ||
		      (s != srest && gmatch (srest, se, p - 1, pe, flags) == 1);
	      if (m1 && m2)
	        return (1);
	    }
	  if (pnext == prest)
	    break;
	}
      return (0);

    case '?':		/* match zero or one of the patterns */
    case '@':		/* match exactly one of the patterns */
      prest = patscan (p, pe, 0);
      if (prest == 0)
	return (strcompare (p - 1, pe, s, se));
      
      /* If we can get away with no matches, don't even bother.  Just
	 call gmatch on the rest of the pattern and return success if
	 it succeeds. */
      if (xc == '?' && (gmatch (s, se, prest, pe, flags) == 1))
	return 0;

      /* OK, we have to do this the hard way.  First, we see if one of
	 the subpatterns matches, then, if it does, we try to match the
	 rest of the string. */
      for (psub = p + 1; ; psub = pnext)
	{
	  pnext = patscan (psub, pe, '|');
	  srest = (prest == pe) ? se : s;
	  for ( ; srest <= se; srest++)
	    {
	      if (gmatch (s, srest, psub, pnext - 1, flags) == 1 &&
		  gmatch (srest, se, prest, pe, flags) == 1)
		return (1);
	    }
	  if (pnext == prest)
	    break;
	}
      return (0);

    case '!':		/* match anything *except* one of the patterns */
      prest = patscan (p, pe, 0);
      if (prest == 0)
	return (strcompare (p - 1, pe, s, se));

      for (srest = s; srest <= se; srest++)
	{
	  m1 = 0;
	  for (psub = p + 1; ; psub = pnext)
	    {
	      pnext = patscan (psub, pe, '|');
	      /* If one of the patterns matches, just bail immediately. */
	      if ((m1 = (gmatch (s, srest, psub, pnext - 1, flags)) == 1))
		break;
	      if (pnext == prest)
		break;
	    }
	  if (m1 == 0 && gmatch (srest, se, prest, pe, flags) == 1)
	    return (1);	
	}
      return (0);
    }

  return (0);
}




int file_name_match (char *pattern, char *string, int flags)
{
    char *se, *pe;

    if (string == 0 || pattern == 0)
    {
        return 0;
    }

    se = string + strlen (string);
    pe = pattern + strlen (pattern);

    return (gmatch (string, se, pattern, pe, flags));
}

/*******************************************************************************
 * Name:    check_and_get_genfile
 * Purpose: check and get genral filename 
 * Input: 
 *   cli: cli pointer
 *   filename: file name start with drive type
 *   outsize:  out buffer size
 * Output: 
 *   outfile: out file
 * Return: 
 *   success: E_GEN_FILE_SUCCESS
 *   failed : E_GEN_FILE_INALID
 * Note: 
 ******************************************************************************/
int
check_and_get_genfile(struct cli *cli, char *filename, char *outfile, size_t outsize)
{
    if (NULL == filename || NULL == outfile || 0 > outsize)
    {
        return E_GEN_FILE_INALID;
    }

    if (gen_check_and_get_filename(cli, filename, outfile, outsize) != 0)
    {
        return E_GEN_FILE_INALID;
    }
    return E_GEN_FILE_SUCCESS;
}

