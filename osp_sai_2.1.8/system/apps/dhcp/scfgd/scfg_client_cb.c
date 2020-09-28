/****************************************************************************
* $Id$
*  Message callbacks
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-25 16:39
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "pal.h"
#include "cli.h"
#include "message.h"
#include "glb_hw_define.h"
#include "SCfgMsg.h"
#include "dhc_common.h"
#include "scfg_client_cb.h"
#include "scfg_client.h"
#include "scfg_popen.h"
#include "param_check.h"
#include "thread.h"
#include "SmartDeploy.h"
#include "chsm_client.h"
#include "oem_info.h"
#include "syscmd_readline.h"
#include "glb_smartcfg.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

typedef enum
{
    E_SCFG_INITD_NONE = 0, /* not started */
    E_SCFG_INITD_GET_XML, /* get xml file */
    E_SCFG_INITD_PARSE_XML, /* parse xml */
    E_SCFG_INITD_DOWNLOAD_IMG, /* download image */
    E_SCFG_INITD_DOWNLOAD_CFG, /* download configuration file */
    E_SCFG_INITD_CHECK_IMG, /* check image file checksum */
    E_SCFG_INITD_SUCCESS /* perform init deployment success */
} scfg_init_deploy_state_e;

typedef struct
{
    scfg_init_deploy_state_e state;

    /* vars for E_SCFG_INITD_GET_XML */
    int nTryLoops; /* try loop count */
    int nTryIdx;

    /* vars for server and it vrfid */
    int32_t vrfid; 
    char *pszAddr; /* tftp server address */

    /* vars for DOWNLOAD_xxx */
    char *hostprefix;
    char *imageName;
    char *confName;

    char *imageTmp; /* temp file for image */
    char *confTmp; /* temp file for config */

    void *timer; /* timer for next loop */
    SCFG_EXEC_T *pexec;
} scfg_deploy_ctrl_t;

scfg_deploy_ctrl_t g_ideploy_ctrl;

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

int scfg_try_download_deploy_file();

int64_t check_disk_free_space(char* name);
void scfg_tftp_download_deployfile_done(unsigned short nResult, void *pArg);
void scfg_tftp_download_image_done(unsigned short nResult, void *pArg);
void scfg_tftp_download_conf_done(unsigned short nResult, void *pArg);
void scfg_tftp_release(void *pArg);
void scfg_tftp_proc(char *szBuf, int len);

int scfg_client_tx_msg(SCfgMsg_t *pmsg);

/****************************************************************************
 *
* Function
* 
****************************************************************************/

inline void scfg_log(E_SEVERITY eSeverity, const char *fmt, ...)
{
#define _M_LOG_BUF_SZ   1024
    char szLogBuf[_M_LOG_BUF_SZ];
    va_list ap;
    int bIsProgress = 0;
    int bBlankLine = 0;
    int bStartCR = 0;

    va_start(ap, fmt);
    vsnprintf(szLogBuf, _M_LOG_BUF_SZ, fmt, ap);

    if (!pal_strcasecmp(szLogBuf, "\n") 
        || !pal_strcasecmp(szLogBuf, "\r\n")
        || !pal_strcasecmp(szLogBuf, ""))
    {
        bBlankLine = 1;
    }

    if (!pal_strncasecmp(szLogBuf, "\n", 1) 
        || !pal_strncasecmp(szLogBuf, "\r\n", 2))
    {
        bStartCR = 1;
    }

    if (!pal_strcasecmp(szLogBuf, ".") || !pal_strcasecmp(szLogBuf, "#"))
    {
        bIsProgress = 1;
    }

    if (!bIsProgress)
    {
        if (!bBlankLine)
        {
            log_sys(M_MOD_SCFG, eSeverity, "%s", szLogBuf);
        }
        if (scfg_get_deploy_cfg()->flags & M_SCFG_FLAG_LOG_TO_CONSOLE)
        {
            if (bBlankLine || bStartCR)
            {
                fprintf(stderr, "%s", szLogBuf);
            }
            else
            {
                fprintf(stderr, "SmartConfig: %s", szLogBuf);
            }
        }
    }
    else
    {
        if (scfg_get_deploy_cfg()->flags & M_SCFG_FLAG_LOG_TO_CONSOLE)
        {
            fprintf(stderr, "%s", szLogBuf);
            fflush(stderr);
        }
    }

    va_end(ap);
    return;
}

inline scfg_deploy_ctrl_t *scfg_get_ideploy_ctrl(void)
{
    return &g_ideploy_ctrl;
}

void scfg_check_image_proc(char *szBuf, int len)
{
    scfg_log(E_WARNING, "%s", szBuf);
    return;
}

void scfg_check_image_release(void *p)
{
    return;
}

void scfg_ctrl_free(void)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    if (pctrl->imageTmp)
    {
        unlink(pctrl->imageTmp);
        XFREE(MTYPE_TMP, pctrl->imageTmp);
        pctrl->imageTmp = NULL;
    }

    if (pctrl->confTmp)
    {
        unlink(pctrl->confTmp);
        XFREE(MTYPE_TMP, pctrl->confTmp);
        pctrl->confTmp = NULL;
    }

    if (pctrl->hostprefix)
    {
        XFREE(MTYPE_TMP, pctrl->hostprefix);
        pctrl->hostprefix = NULL;
    }

    if (pctrl->imageName)
    {
        XFREE(MTYPE_TMP, pctrl->imageName);
        pctrl->imageName = NULL;
    }

    if (pctrl->confName)
    {
        XFREE(MTYPE_TMP, pctrl->confName);
        pctrl->confName = NULL;
    }
    if (pctrl->pszAddr)
    {
        XFREE(MTYPE_TMP, pctrl->pszAddr);
        pctrl->pszAddr = NULL;
    }

    if (pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (pctrl->timer)
    {
        thread_cancel(pctrl->timer);
        pctrl->timer = NULL;
    }

    pctrl->state = E_SCFG_INITD_GET_XML;
    return;
}

#ifdef _GLB_UML_SYSTEM_
void scfg_do_uml_deployment(void)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();

    scfg_log(E_WARNING, 
            "Execute SmartConfig deployment in UML\n");
    if (pctrl->confTmp)
    {
#if 0
        char szCmd[DHC_ERR_MSG_BUF_LEN];
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN,
                "cp -fr %s %s", pctrl->confTmp, 
                STARTUP_CONFIG_FILE_PATH);
        system(szCmd);
#endif
        scfg_log(E_WARNING, 
                " Deployment startup configuration file\n");
    }
    if (pctrl->imageTmp)
    {
        scfg_log(E_WARNING, 
                " Deployment image file\n");
    }
    scfg_log(E_WARNING, 
            " Please restart UML environment manually\n");
    return;
}
#else
#define HASHBYTES   (50 * 1024)
#define BUFFER_COUNT    8192
int scfg_install_image(void)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    char dst_file[PATH_MAX];
    FILE *src_fp = NULL, *dst_fp = NULL;
    char buf[BUFFER_COUNT];
    int ret = -1, count;
    int amount = 0; 
    int hashbytes = 0;
    struct timeval tstart;
    struct timeval tstop;

    pal_snprintf(dst_file, PATH_MAX, "%s/%s/%s",
            FULL_NAME_FLASH, "boot", pctrl->imageName);

    scfg_log(E_WARNING, 
            "Install image file %s\n", pctrl->imageName);

    src_fp = pal_fopen(pctrl->imageTmp, PAL_OPEN_RO);
    if (NULL == src_fp)
    {
        goto err_out;
    }

    dst_fp = pal_fopen(dst_file, PAL_OPEN_RW);
    if (NULL == dst_fp)
    {
        goto err_out;
    }

    (void)gettimeofday(&tstart, NULL);
    while ((count = pal_fread(buf, sizeof(char), BUFFER_COUNT, src_fp)) > 0)
    {
        ret = pal_fwrite(buf, sizeof(char), count, dst_fp);
        if (ret <= 0)
        {
            (void)gettimeofday(&tstop, NULL);
            goto err_out;
        }
        amount += count;
        while (amount >= hashbytes) 
        {
            hashbytes += HASHBYTES;
            scfg_log(E_WARNING, ".");
        }
    }
    (void)gettimeofday(&tstop, NULL);
    ret = 0;

err_out:
    if (NULL != src_fp)
    {
        pal_fclose(src_fp);
    }
    if (NULL != dst_fp)
    {
        pal_fclose(dst_fp);
    }
    if (amount)
    {
        double delta;

        /* compute delta in 1/10's second units */
        delta = ((tstop.tv_sec * 10.) + (tstop.tv_usec / 100000)) -
            ((tstart.tv_sec * 10.) + (tstart.tv_usec / 100000));
        delta = delta / 10.;    /* back to seconds */
        scfg_log(E_WARNING, 
                "\nImage file %d bytes in %.1f seconds, %.0f kbytes/second\n",
                amount, delta, (amount * 1.) / (1024 * delta));
    }
    return ret;
}

char *scfg_gen_hostname(char *prefix, char *name)
{
    if (NULL == prefix)
    {
        pal_snprintf(name, M_HOSTNAME_MAXSIZE, "Switch");
    }
    else 
    {
        pal_snprintf(name, M_HOSTNAME_MAXSIZE, "%s%s", 
                prefix, scfg_get_sys_mac() + 7);
    }

    return name;
}

int scfg_do_sys_deployment(void)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    scfg_deploy_cfg_t *pcfg = scfg_get_deploy_cfg();
    char szCmd[DHC_ERR_MSG_BUF_LEN];
    char szHostname[M_HOSTNAME_MAXSIZE];
    int ret;

    /* copy files to it destination */
    if (pcfg->flags & M_SCFG_FLAG_HOSTNAME_PREFIX)
    {
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, 
                "echo \"hostname %s\" > %s && cat %s | grep -v \"^hostname \" >> %s", 
                scfg_gen_hostname(pctrl->hostprefix, szHostname), 
                STARTUP_CONFIG_FILE_PATH, 
                pctrl->confTmp, STARTUP_CONFIG_FILE_PATH);
    }
    else
    {
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, 
                "cp -fr %s %s", pctrl->confTmp, STARTUP_CONFIG_FILE_PATH);
    }
    ret = system(szCmd);
    if (0 != ret)
    {
        return -1;
    }

    /*added by liuyang,  add imageName check for we did not download img*/
    if (pctrl->imageName)
    {
        ret = scfg_install_image();
        if (0 != ret)
        {
            return -1;
        }
#if 0
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, 
                "rm -fr %s && ln -s %s/%s %s && sync && sync ", 
                M_CUR_BOOT_IMAGE_LINK_FULLNAME,
                M_BOOT_IMAGE_PATH, pctrl->imageName, 
                M_CUR_BOOT_IMAGE_LINK_FULLNAME);

        ret = system(szCmd);

#else
        /* modified by liuyang 2011-11-4
         *  for uboot can't boot img if we create soft link "uImage" to absolute
         *  path of img . If this is not work also ,we need copy some clauses 
         *  from imish.
         */
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, 
                "rm -fr %s && ln -s %s %s && sync && sync ", 
                M_CUR_BOOT_IMAGE_LINK_FULLNAME,
                pctrl->imageName, 
                M_CUR_BOOT_IMAGE_LINK_FULLNAME);

        ret = system(szCmd);
       
#endif    

#if 0
        scfg_log(E_WARNING, "Not perform REBOOT for debug\n");
#else
        SCfgMsg_t msg;
        /* send reboot message to controller */
        pal_mem_set(&msg, 0, sizeof(SCfgMsg_t));
        msg.op.present = op_PR_rebootCmd;
#define _M_DEPLOY_REBOOT_MSG  "perform initialize deployment success"
        msg.op.rebootCmd.msg.buf = (unsigned char *)_M_DEPLOY_REBOOT_MSG;
        msg.op.rebootCmd.msg.size = pal_strlen(_M_DEPLOY_REBOOT_MSG);

        scfg_client_tx_msg(&msg);
        scfg_log(E_WARNING, 
                "Deployment success, exit...\n");
#endif       
 
    }
    else
    {
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN,
                "imish -f %s > /tmp/scfg_startup.log 2>&1 &",
                STARTUP_CONFIG_FILE_PATH);
        ret = system(szCmd);
        if (0 != ret)
        {
            return -1;
        }

    }

    exit (0);

    return 0;
}
#endif /* _GLB_UML_SYSTEM_ */

int scfg_do_deployment(void)
{
#ifdef _GLB_UML_SYSTEM_
    scfg_do_uml_deployment();
#else
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    char szCmd[DHC_ERR_MSG_BUF_LEN];
    int64_t llFree = 0ll;
    int64_t llSize = 0ll;
    struct stat ns;

    /* delete old image file if exist */
    /* modified by liuyang 2012-4-24*/
    if (pctrl->imageName)
    {
        pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, "rm -fr %s/%s",
                M_BOOT_IMAGE_PATH, pctrl->imageName);
        system(szCmd);
    }
    llFree = check_disk_free_space(FULL_NAME_FLASH);

    if (pctrl->imageTmp)
    {
        if (stat(pctrl->imageTmp, &ns) != 0)
        {
            return -1;
        }
        llSize += ns.st_size;
    }
    if (pctrl->confTmp)
    {
        if (stat(pctrl->confTmp, &ns) != 0)
        {
            return -1;
        }
        llSize += ns.st_size;
    }

    /* max leave 100k size */
    scfg_log(E_WARNING, 
            "Currentlly free space is %llu bytes, file size is %llu bytes\n", 
            llFree, llSize);
    if (llFree < (llSize + 100 * 1024))
    {
        scfg_log(E_WARNING, 
                "Perform deploy failed: disk space not enough\n");
        return -1;
    }

    if (-1 == scfg_do_sys_deployment())
    {
        return -1;
    }
#endif /* !_GLB_UML_SYSTEM_ */
    return 0;
}

void scfg_check_image_done(unsigned short nResult, void *pArg)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();

    pal_assert(E_SCFG_INITD_CHECK_IMG == pctrl->state);
    if (NULL != pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (0 == nResult)
    {
        scfg_log(E_WARNING, 
                "Check image file %s success\n", pctrl->imageName);

        if (0 == scfg_do_deployment())
        {
            scfg_ctrl_free();
            return;
        }
    }

    scfg_log(E_WARNING, 
            "Corrupted image file %s\n", pctrl->imageName);

    scfg_ctrl_free();
    pctrl->state = E_SCFG_INITD_GET_XML;
    scfg_try_download_deploy_file();
    return;
}

int scfg_start_validate_image(SCFG_EXEC_T **ppexec, char *szFile)
{
    SCFG_EXEC_T *pexec;
    char szCmd[DHC_ERR_MSG_BUF_LEN];
    SCFG_SYSTEM_OPS_T ops = 
    {
        .release = scfg_check_image_release,
        .proc = scfg_check_image_proc,
        .done = scfg_check_image_done
    };

    if (NULL != *ppexec)
    {
        scfg_cancel_exec(*ppexec);
        *ppexec = NULL;
    }

    pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN, 
            "check_image %s 2>/dev/null", szFile);
    scfg_log(E_WARNING, 
            "Try validate image file: %s\n", szFile);
    pexec = scfg_system(szCmd, "r", &ops, NULL);
    if (NULL == pexec)
    {
        return 0;
    }

    pexec->refcnt++;
    *ppexec = pexec;
    return 1;
}

void scfg_tftp_proc(char *szBuf, int len)
{
    scfg_log(E_WARNING, "%s", szBuf);
    return;
}

void scfg_tftp_release(void *p)
{
    SCFG_TFTP_ARG_T *pArg = (SCFG_TFTP_ARG_T *)p;
    if (NULL != pArg)
    {
        if (NULL != pArg->pszDest)
        {
            unlink(pArg->pszDest);
            XFREE(MTYPE_TMP, pArg->pszDest);
        }

        if (NULL != pArg->pszSrc)
        {
            XFREE(MTYPE_TMP, pArg->pszSrc);
        }

        XFREE(MTYPE_TMP, pArg);
        pArg = NULL;
    }
    return;
}

int scfg_start_download_file(SCFG_EXEC_T **ppexec, char *szFile, 
        void (*fdone)(unsigned short nResult, void *pArg), size_t maxsize)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    SCFG_EXEC_T *pexec;
    SCFG_TFTP_ARG_T *pArg;
    char szCmd[DHC_ERR_MSG_BUF_LEN];
    SCFG_SYSTEM_OPS_T ops = 
    {
        .release = scfg_tftp_release,
        .proc = scfg_tftp_proc,
        .done = fdone
    };

    if (NULL != *ppexec)
    {
        scfg_cancel_exec(*ppexec);
        *ppexec = NULL;
    }

    if (NULL == (pArg = scfg_tftp_init(szFile)))
    {
        scfg_log(E_ERROR, "Warning: out of memory\n");
        return 0;
    }

    pal_snprintf(szCmd, DHC_ERR_MSG_BUF_LEN,
            "chvrf %d tftp -T 5 -w 1 -s 2048 -m binary -v -S %d -Z %s -c get -r %s -l %s",
            pctrl->vrfid, maxsize, pctrl->pszAddr, pArg->pszSrc, pArg->pszDest);
    scfg_log(E_WARNING, 
            "Try download file %s from server %s vrf %d\n", 
            szFile, pctrl->pszAddr, pctrl->vrfid);
    pexec = scfg_system(szCmd, "r", &ops, pArg);
    if (NULL == pexec)
    {
        scfg_tftp_release(pArg);
        return 0;
    }

    pexec->refcnt++;
    *ppexec = pexec;
    return 1;
}

int scfg_restart_download_deployfile(struct thread *th)
{
    scfg_deploy_ctrl_t *pctrl = (scfg_deploy_ctrl_t *)THREAD_ARG(th);

    pctrl->timer = NULL;
    scfg_try_download_deploy_file();
    return 0;
}

int scfg_try_download_deploy_file()
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    scfg_deploy_cfg_t  *pcfg;
    char *pszAddr = NULL;
    int vrfid = 0;
    int next_loop = 0;

    pal_assert(NULL != pctrl);
    pal_assert(E_SCFG_INITD_GET_XML == pctrl->state);

    pcfg = scfg_get_deploy_cfg();
    pal_assert(NULL != pcfg);

    scfg_log(E_WARNING, 
            "Try download deploy XML file. Index %d, loops %d.\n",
            pctrl->nTryIdx, pctrl->nTryLoops);

    if (!pcfg->nAddrCnt)
    {
        /* no tftp server address */
        return 1;
    }

    if (pctrl->nTryIdx < (DHC_MAX_TFTPSRV_RECORDS * 2))
    {
        /* try other address */
        do 
        {
            pszAddr = pcfg->tftpSrvs[pctrl->nTryIdx++];
        } while (NULL == pszAddr 
                && pctrl->nTryIdx < (DHC_MAX_TFTPSRV_RECORDS * 2));

        if (NULL == pszAddr)
        {
            next_loop = 1;
        }
        else 
        {
            if (pctrl->nTryIdx < DHC_MAX_TFTPSRV_RECORDS)
            {
                vrfid = pcfg->manVrfid;
            }
            else
            {
                vrfid = pcfg->vrfid;
            }
        }
    }
    else
    {
        next_loop = 1;
    }

    if (next_loop)
    {
        /* try next loop */
        pctrl->nTryIdx = 0;
        if (0 != pcfg->retryCnt && pctrl->nTryLoops >= pcfg->retryCnt)
        {
            /* give up for exceed retry count */
            scfg_log(E_ERROR, 
                    "Download deployment XML configuration file failed and give up.");
            return 0;
        }

        pctrl->nTryLoops++;
        scfg_log(E_WARNING, 
                "Start retry timer (delay %d seconds), retry count %d\n",
                pcfg->interval, pctrl->nTryLoops);
        pctrl->timer = thread_add_timer(lib_globals_get(),
                scfg_restart_download_deployfile, pctrl, pcfg->interval);
        return 1;
    }

    pctrl->vrfid = vrfid;
    if (pctrl->pszAddr)
    {
        XFREE(MTYPE_TMP, pctrl->pszAddr);
    }
    pctrl->pszAddr = XSTRDUP(MTYPE_TMP, pszAddr);
    if (NULL == pctrl->pszAddr)
    {
        scfg_log(E_ERROR, "Warning: out of memory\n");
        return 0;
    }

    return scfg_start_download_file(&pctrl->pexec,
            "smartconfig/smartdeploy.xml", 
            scfg_tftp_download_deployfile_done, M_SCFG_DEPLOY_FILE_MAX_SZ);
}

inline scfg_deploy_e scfg_get_deploy_type(Item_t *pitem)
{
    if (!pitem->type.size)
    {
        return E_SCFG_DEPLOY_NONE;
    }

    if (!pal_strcasecmp((char *)pitem->type.buf, M_SCFG_DEPLOY_MAC_STR))
    {
        return E_SCFG_DEPLOY_MAC;
    }

    if (!pal_strcasecmp((char *)pitem->type.buf, M_SCFG_DEPLOY_SN_STR))
    {
        return E_SCFG_DEPLOY_SN;
    }

    if (!pal_strcasecmp((char *)pitem->type.buf, M_SCFG_DEPLOY_PRODUCT_STR))
    {
        return E_SCFG_DEPLOY_PRODUCT;
    }

    return E_SCFG_DEPLOY_NONE;
}

static int scfg_extract_item_vars(unsigned char *image, unsigned char *config)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();

    pal_assert(NULL == pctrl->imageName);
    pal_assert(NULL == pctrl->confName);

    /* check image & config */
    if (!image && !config)
    {
        /* ignore empty item */
        return 0;
    }
    /* for bug 19100 modified by liuyang 2012-4-24*/
    if (image && pal_strlen((char *)image) > 0)
    {
        pctrl->imageName = XSTRDUP(MTYPE_TMP, (char *)image);
        if (NULL == pctrl->imageName)
        {
            scfg_log(E_ERROR, "Parse by MAC failed: out of memory\n");
            return 0;
        }
    }

    if (config && pal_strlen((char *)config) > 0)
    {
        pctrl->confName = XSTRDUP(MTYPE_TMP, (char *)config);
        if (NULL == pctrl->confName)
        {
            scfg_log(E_ERROR, "Parse by MAC failed: out of memory\n");
            if (pctrl->imageName)
            {
                XFREE(MTYPE_TMP, pctrl->imageName);
                pctrl->imageName = NULL;
            }
            return 0;
        }
    }

    return 1;
}

int scfg_parse_by_mac(Item_t *pitem)
{
    unsigned char *pmac;

    if (!pitem->value.size)
    {
        /* ignore empty item */
        return 0;
    }

    pmac = scfg_get_sys_mac();
    pal_assert(NULL != pmac && '\0' != pmac[0]);
    if (pal_strcasecmp((char *)pmac, (char *)pitem->value.buf))
    {
        /* ignore mac mismatch item */
        return 0;
    }

    scfg_log(E_WARNING, 
            "Find item by MAC address: %s\n", pmac);
    return scfg_extract_item_vars(pitem->image.buf, pitem->config.buf);
}

int scfg_parse_by_sn(Item_t *pitem)
{
    char *psn;

    if (!pitem->value.size)
    {
        /* ignore empty item */
        return 0;
    }

    psn = scfg_get_serial_no();
    pal_assert(NULL != psn && '\0' != psn[0]);
    if (pal_strcasecmp(psn, (char *)pitem->value.buf))
    {
        /* ignore serial no mismatch item */
        return 0;
    }

    scfg_log(E_WARNING, 
            "Find item by serial number: %s\n", psn);
    return scfg_extract_item_vars(pitem->image.buf, pitem->config.buf);
}

int scfg_parse_by_product(Item_t *pitem)
{
    char *ptmp;

    if (!pitem->value.size)
    {
        /* ignore empty item */
        return 0;
    }

    ptmp = oem_info_get_hardware_type(NULL);
    pal_assert(NULL != ptmp && '\0' != ptmp[0]);
    if (pal_strcasecmp(ptmp, (char *)pitem->value.buf))
    {
        /* ignore product mismatch item */
        return 0;
    }

    scfg_log(E_WARNING, 
            "Find item by product id: %s\n", ptmp);
    return scfg_extract_item_vars(pitem->image.buf, pitem->config.buf);
}

int scfg_restart_download_img_conf(struct thread *th)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    char fname[PATH_MAX];
    int ret = 0;

    pal_assert(E_SCFG_INITD_DOWNLOAD_IMG == pctrl->state);

    pctrl->timer = NULL;
    if (NULL != pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (pctrl->imageName)
    {
        scfg_log(E_WARNING, 
                "Try download image file %s\n", pctrl->imageName);
        pal_snprintf(fname, PATH_MAX, "%s/%s",
                M_SCFG_DEPLOY_IMAGE_PREFIX, pctrl->imageName);
        ret = scfg_start_download_file(&pctrl->pexec,
                fname, scfg_tftp_download_image_done, M_MAX_FILE_SIZE);
    }
    else
    {
        pctrl->state = E_SCFG_INITD_DOWNLOAD_CFG;
        scfg_log(E_WARNING, 
                "Try download config file %s\n", pctrl->confName);
        pal_snprintf(fname, PATH_MAX, "%s/%s",
                M_SCFG_DEPLOY_CONF_PREFIX, pctrl->confName);
        ret = scfg_start_download_file(&pctrl->pexec,
                fname, scfg_tftp_download_conf_done, M_SCFG_DEPLOY_FILE_MAX_SZ);
    }

    /* XXX: */
    return 0;
}

int scfg_parse_deploy_file(SmartDeploy_t *pDeploy, SCFG_TFTP_ARG_T *pTftp)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    Item_t *pitem;
    int idx;
    int ret = 0;
    int find = 0;

    if (!pDeploy->ftype.size 
        || pal_strcasecmp((char *)pDeploy->ftype.buf, M_SCFG_DEPLOY_INIT_NAME))
    {
        scfg_log(E_ERROR,
                "Invalid file type string (%s) in deploy file",
                pDeploy->ftype.size ? pDeploy->ftype.buf : (uint8_t *)"NULL");
        goto err_out;
    }
 
    if (pDeploy->hostprefix.size)
    {
        if (check_hostname((char *)pDeploy->hostprefix.buf) == -1
                || pDeploy->hostprefix.size >= (M_HOSTNAME_MAXSIZE - MAC_ADDR_LEN))
        {
            scfg_log(E_ERROR, 
                    "Ignore invalid hostname prefix (%s) in deploy file", 
                    pDeploy->hostprefix.buf);
        }
        else
        {
            if (pctrl->hostprefix)
            {
                XFREE(MTYPE_TMP, pctrl->hostprefix);
                pctrl->hostprefix = NULL;
            }
            pctrl->hostprefix = XMALLOC(MTYPE_TMP, pDeploy->hostprefix.size + 1);
            if (NULL != pctrl->hostprefix)
            {
                pal_mem_cpy(pctrl->hostprefix, pDeploy->hostprefix.buf,
                        pDeploy->hostprefix.size);
                pctrl->hostprefix[pDeploy->hostprefix.size] = '\0';
            }
        }
    }

    if (pctrl->imageName)
    {
        XFREE(MTYPE_TMP, pctrl->imageName);
        pctrl->imageName = NULL;
    }
    if (pctrl->confName)
    {
        XFREE(MTYPE_TMP, pctrl->confName);
        pctrl->confName = NULL;
    }
    /* deploy by mac */
    for (idx = 0; idx < pDeploy->groups.list.count; idx++)
    {
        pitem = (Item_t *)pDeploy->groups.list.array[idx];
        scfg_deploy_e type = scfg_get_deploy_type(pitem);
        if (E_SCFG_DEPLOY_MAC != type)
        {
            continue;
        }
        ret = scfg_parse_by_mac(pitem);
        if (ret)
        {
            find = 1;
        }
    }

    /* deploy by SN */
    for (idx = 0; !find && (idx < pDeploy->groups.list.count); idx++)
    {
        pitem = (Item_t *)pDeploy->groups.list.array[idx];
        scfg_deploy_e type = scfg_get_deploy_type(pitem);
        if (E_SCFG_DEPLOY_SN != type)
        {
            continue;
        }
        ret = scfg_parse_by_sn(pitem);
        if (ret)
        {
            find = 1;
        }
    }

    /* deploy by product */
    for (idx = 0; !find && (idx < pDeploy->groups.list.count); idx++)
    {
        pitem = (Item_t *)pDeploy->groups.list.array[idx];
        scfg_deploy_e type = scfg_get_deploy_type(pitem);
        if (E_SCFG_DEPLOY_PRODUCT != type)
        {
            continue;
        }
        ret = scfg_parse_by_product(pitem);
        if (ret)
        {
            find = 1;
        }
    }

    /* deploy if default config item enabled */
    if (!find)
    {
        Default_t *pdef = &pDeploy->defItem;
        if (pdef->option.buf 
                && !pal_strcasecmp((char *)pdef->option.buf, "enable"))
        {
            scfg_log(E_WARNING, 
                    "Deployment default item\n");
            ret = scfg_extract_item_vars(pdef->image.buf, pdef->config.buf);
            if (ret)
            {
                find = 1;
            }
        }
    }
    /*modified by liuyang 2012-4-24*/
    if (find && pctrl->confName)
    {
        pal_assert(NULL == pctrl->timer);
        pctrl->state = E_SCFG_INITD_DOWNLOAD_IMG;
        if (pctrl->imageTmp)
        {
            XFREE(MTYPE_TMP, pctrl->imageTmp);
            pctrl->imageTmp = NULL;
        }
        if (pctrl->confTmp)
        {
            XFREE(MTYPE_TMP, pctrl->confTmp);
            pctrl->confTmp = NULL;
        }
        pctrl->timer = thread_add_event(lib_globals_get(),
                scfg_restart_download_img_conf, NULL, 
                scfg_get_deploy_cfg()->interval);
    }
    else
    {
        scfg_log(E_WARNING, 
                "Can't find config item or config file from deployment XML file\n");
    }

err_out:
    return ret;
}

int scfg_decode_deploy_file(char *pszFile, SCFG_TFTP_ARG_T *pTftp)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    SmartDeploy_t *pDeploy = NULL;
    struct stat ns;
    char *pbuf = NULL;
    FILE *fp = NULL;
    size_t size;
    asn_dec_rval_t rval;
    int ret = 0;

    pal_assert(NULL != pszFile);
    pal_assert(E_SCFG_INITD_PARSE_XML == pctrl->state);
    if (stat(pszFile, &ns) != 0)
    {
        scfg_log(E_ERROR, 
                "Check deploy file size failed: %s", pal_strerror(errno));
        pal_assert(0);
        return 0;
    }

    if (ns.st_size > M_SCFG_DEPLOY_FILE_MAX_SZ)
    {
        scfg_log(E_ERROR, 
                "Deploy file too large, should less than %d bytes",
                M_SCFG_DEPLOY_FILE_MAX_SZ);
        pal_assert(0);
        return 0;
    }
    /*for bug 19114 use malloc to ignore memory limitation by liuyang 2012-4-25*/
    pbuf = malloc(ns.st_size + 1);
    if (NULL == pbuf)
    {
        scfg_log(E_ERROR, 
                "Parse deploy file failed: out of memory\n");
        return 0;
    }

    if (NULL == (fp = fopen(pszFile, "r")))
    {
        scfg_log(E_ERROR,
                "Parse deploy file failed: %s", pal_strerror(errno));
        goto err_out;
    }
    
    size = fread(pbuf, 1, ns.st_size, fp);
    if (size != ns.st_size)
    {
        scfg_log(E_ERROR,
                "Read deploy file failed: %s", pal_strerror(errno));
        goto err_out;
    }
    pbuf[ns.st_size] = '\0';

    scfg_log(E_WARNING, "XML file content: %s\n", pbuf);

    rval = xer_decode(0, &asn_DEF_SmartDeploy, (void **)&pDeploy, pbuf, size);
    if (RC_OK != rval.code)
    {
        scfg_log(E_ERROR, 
                "Invalid deploy file (failed at %d bytes)", rval.consumed + 1);
        goto err_out;
    }

    /* xer_fprint(stderr, &asn_DEF_SmartDeploy, pDeploy); */
    scfg_log(E_WARNING, 
            "Parse deploy XML file success, perform check & deployment\n");

    ret = scfg_parse_deploy_file(pDeploy, pTftp);

err_out:
    if (NULL != fp)
    {
        fclose(fp);
    }

    if (NULL != pbuf)
    {/*for bug 19114 modified by liuyang 2012-4-25*/
        free(pbuf);
    }

    if (pDeploy)
    {
        asn_DEF_SmartDeploy.free_struct(&asn_DEF_SmartDeploy, pDeploy, 0);
    }
    return ret;
}

int scfg_tftp_perform_deploy()
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    int ret;

    scfg_log(E_WARNING, 
            "Process image/conf file downloaded\n");
    if (pctrl->imageTmp)
    {
        pctrl->state = E_SCFG_INITD_CHECK_IMG;

        ret = scfg_start_validate_image(&pctrl->pexec, 
                pctrl->imageTmp);
        if (1 != ret)
        {
            goto err_out;
        }
    }
    else
    {
        scfg_log(E_WARNING, 
                "No image file specified and perform deployment\n");
        scfg_do_deployment();
    }
    return 1;

err_out:
    scfg_ctrl_free();
    scfg_log(E_WARNING, 
            "Validate image file failed\n");
    pctrl->state = E_SCFG_INITD_GET_XML;
    scfg_try_download_deploy_file();
    return 0;
}

void scfg_tftp_download_conf_done(unsigned short nResult, void *pArg)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    SCFG_TFTP_ARG_T *pTftp = (SCFG_TFTP_ARG_T *)pArg;

    pal_assert(E_SCFG_INITD_DOWNLOAD_CFG == pctrl->state);
    pal_assert(pTftp);

    if (NULL != pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (0 == nResult)
    {
        pctrl->confTmp = pTftp->pszDest;
        pTftp->pszDest = NULL;
        scfg_log(E_WARNING, 
                "Download configuration file %s success\n", pctrl->confName);
        scfg_tftp_perform_deploy();
        return;
    }

    scfg_log(E_WARNING, 
            "Download config file %s from %s failed\n",
            pTftp->pszSrc, pctrl->pszAddr);
    pctrl->state = E_SCFG_INITD_GET_XML;
    scfg_try_download_deploy_file();
    return;
}

void scfg_tftp_download_image_done(unsigned short nResult, void *pArg)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    SCFG_TFTP_ARG_T *pTftp = (SCFG_TFTP_ARG_T *)pArg;
    int ret = 0;

    pal_assert(E_SCFG_INITD_DOWNLOAD_IMG == pctrl->state);
    pal_assert(pTftp);

    if (NULL != pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (0 == nResult)
    {
        scfg_log(E_WARNING, 
                "Download image file %s success\n", pctrl->imageName);
        if (pctrl->confName)
        {
            char fname[PATH_MAX];

            pctrl->imageTmp = pTftp->pszDest;
            pTftp->pszDest = NULL;

            pctrl->state = E_SCFG_INITD_DOWNLOAD_CFG;
            scfg_log(E_WARNING, 
                    "Try download config file %s\n", pctrl->confName);
            pal_snprintf(fname, PATH_MAX, "%s/%s",
                    M_SCFG_DEPLOY_CONF_PREFIX, pctrl->confName);
            ret = scfg_start_download_file(&pctrl->pexec,
                    fname, scfg_tftp_download_conf_done, M_SCFG_DEPLOY_FILE_MAX_SZ);
            if (ret)
            {
                return;
            }
        }
        else
        {
            scfg_tftp_perform_deploy();
            return;
        }
    }

    scfg_log(E_WARNING, 
            "Download image file %s from %s failed\n",
            pTftp->pszSrc, pctrl->pszAddr);
    pctrl->state = E_SCFG_INITD_GET_XML;
    scfg_try_download_deploy_file();
    return;
}

void scfg_tftp_download_deployfile_done(unsigned short nResult, void *pArg)
{
    scfg_deploy_ctrl_t *pctrl = scfg_get_ideploy_ctrl();
    SCFG_TFTP_ARG_T *pTftp = (SCFG_TFTP_ARG_T *)pArg;

    pal_assert(E_SCFG_INITD_GET_XML == pctrl->state);
    pal_assert(pTftp);

    if (NULL != pctrl->pexec)
    {
        scfg_cancel_exec(pctrl->pexec);
        pctrl->pexec = NULL;
    }
    if (0 == nResult)
    {
        scfg_log(E_WARNING, 
                "Download deployment XML file success\n");

        /* process XML file */
        pctrl->state = E_SCFG_INITD_PARSE_XML;
        if (scfg_decode_deploy_file(pTftp->pszDest, pTftp))
        {
            return;
        }

        scfg_log(E_WARNING, 
                "Process deployment XML file failed\n");
    }

    scfg_log(E_WARNING, 
            "Download deployment XML file failed\n");
    pctrl->state = E_SCFG_INITD_GET_XML;
    scfg_try_download_deploy_file();
    return;
}

SCFG_TFTP_ARG_T *scfg_tftp_init(char *pszSrc)
{
    SCFG_TFTP_ARG_T *pArg = NULL;
    int fd = -1;

    pArg = XCALLOC(MTYPE_TMP, sizeof(SCFG_TFTP_ARG_T));
    if (NULL == pArg)
    {
        scfg_log(E_ERROR, "Warning: out of memory\n");
        return NULL;
    }

    pArg->pszDest = XSTRDUP(MTYPE_TMP, M_TMPFILE_TEMPLATE);
    if (NULL == pArg->pszDest)
    {
        scfg_log(E_ERROR, "Warning: out of memory\n");
        goto err_out;
    }

    fd = mkstemp(pArg->pszDest);
    if (-1 == fd)
    {
        scfg_log(E_ERROR, 
                "Init tftp temp file faild: %s\n", pal_strerror(errno));
        goto err_out;
    }
    close(fd);

    pArg->pszSrc  = XSTRDUP(MTYPE_TMP, pszSrc);
    if (NULL == pArg->pszSrc)
    {
        scfg_log(E_ERROR, "Warning: out of memory\n");
        goto err_out;
    }

    return pArg;

err_out:
    if (-1 != fd)
    {
        close(fd);
    }
    scfg_tftp_release(pArg);
    return NULL;
}

int __parse_tftp_address(char *paddrs, size_t size, char ***pppaddrs, int *pnToks)
{
    char buf[DHC_MAX_TFTPSRV_BUF_LEN];
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nProcToks;
    int idx;

    if (size >= DHC_MAX_TFTPSRV_BUF_LEN)
    {
        return 0;
    }
    
    pal_mem_cpy(buf, paddrs, size);
    buf[size] = '\0';

    if (pal_strspn(buf, "0123456789.,") != size)
    {
        scfg_log(E_ERROR, 
                "Invalid tftp address '%s'", buf);
        return 0;
    }

    if (split_tokens(buf, size, DHC_MAX_OPTION_150_BYTES, ",", &nToks, &ppTokArr) != 0)
    {
        scfg_log(E_ERROR, 
                "Parse tftp address '%s' failed", buf);
        goto err_out;
    }

    nProcToks = (nToks < *pnToks) ? nToks : *pnToks;
    for (idx = 0; idx < nProcToks; idx++)
    {
        struct in_addr ina;
        if (!inet_aton(ppTokArr[idx], &ina) || check_host_ip(ina) == -1)
        {
            goto err_out;
        }
    }

    *pppaddrs = ppTokArr;
    *pnToks = nProcToks;
    return 1;

err_out:
    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return 0;
}

int scfg_setting_tftp_address_byname(scfg_deploy_cfg_t *pcfg,
        SCfgTftpAddr_t *pmsg, char ***pppToks, int nToks)
{
    char *ifname = (char *)pmsg->ifname.buf;
    int is_man_dev = 0;
    char **ppToks;
    int idx;
    int mymax;
    int mymin;

    if (!pal_strncasecmp(ifname, M_DEPLOY_MAN_DEV_STR, pmsg->ifname.size))
    {
        is_man_dev = 1;
    }

    /* clear old settings */
    if (is_man_dev)
    {
        mymin = 0;
        mymax = DHC_MAX_TFTPSRV_RECORDS;
    }
    else
    {
        mymin = DHC_MAX_TFTPSRV_RECORDS;
        mymax = DHC_MAX_TFTPSRV_RECORDS * 2;
    }

    for (idx = mymin; idx < mymax; idx++)
    {
        if (NULL == pcfg->tftpSrvs[idx])
        {
            break;
        }

        XFREE(MTYPE_TMP, pcfg->tftpSrvs[idx]);
        pcfg->tftpSrvs[idx] = NULL;
        pcfg->nAddrCnt--;
    }

    ppToks = *pppToks;
    for (idx = 0; idx < nToks; idx++)
    {
        if (NULL == ppToks[idx])
        {
            break;
        }

        pcfg->tftpSrvs[idx + mymin] = XSTRDUP(MTYPE_TMP, ppToks[idx]);
        if (NULL == pcfg->tftpSrvs[idx + mymin])
        {
            scfg_log(E_ERROR, "Warning: out of memory\n");
            goto err_out;
        }
        pcfg->nAddrCnt++;
    }

    if (is_man_dev)
    {
        pcfg->manVrfid = pmsg->vrfid;
    }
    else
    {
        pcfg->vrfid = pmsg->vrfid;
    }
    return 1;

err_out:
    return 0;
}

int scfg_fire_deploy_procedures(scfg_deploy_cfg_t *pcfg)
{
    scfg_deploy_ctrl_t *pctrl;
    int ret;
#if 0
    if (!pcfg->bStartupCfgEmpty || !(pcfg->flags & M_SCFG_FLAG_INIT_DEPLOYMENT))
    {
        return 1;
    }
#endif

    pctrl = scfg_get_ideploy_ctrl();
    scfg_ctrl_free();
    pctrl->nTryLoops = 0;
    pctrl->nTryIdx = 0;
    pctrl->state = E_SCFG_INITD_GET_XML;
    ret = scfg_try_download_deploy_file();
    return ret;
}

int scfg_setting_tftp_address(struct message_handler *pms, SCfgTftpAddr_t *pmsg,
        char ***pppToks, int nToks)
{
    scfg_deploy_cfg_t *pcfg;
    int ret;

    if (NULL == (pcfg = scfg_get_deploy_cfg()))
    {
        pal_assert(0);
        return 0;
    }

    /* save config */
    ret = scfg_setting_tftp_address_byname(pcfg, pmsg, pppToks, nToks);
    if (0 == ret)
    {
        goto err_out;
    }

    /* try startup deployment */
    ret = scfg_fire_deploy_procedures(pcfg);
    free_tokens(pppToks);
    *pppToks = NULL;
    return ret;

err_out:
    free_tokens(pppToks);
    *pppToks = NULL;
    return 0;
}

int scfg_client_rx_bound(struct message_handler *pms, SCfgMsg_t *pmsg)
{
    SCfgTftpAddr_t *req;
    int ret = 1;
    char **ppTokArr = NULL;
    int nToks = DHC_MAX_TFTPSRV_RECORDS;

    req = &pmsg->op.boundNotify;
    if (NULL == req->tftpaddrs.buf || NULL == req->ifname.buf)
    {
        scfg_log(E_ERROR, "Invalid bound message (NULL string)");
        return 0;
    }

    if (!__parse_tftp_address((char *)req->tftpaddrs.buf, req->tftpaddrs.size,
                &ppTokArr, &nToks))
    {
        return 0;
    }

    if (!scfg_setting_tftp_address(pms, req, &ppTokArr, nToks))
    {
        ret = 0;
    }

    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return ret;
}


int scfg_client_rx_unbound(struct message_handler *pms, SCfgMsg_t *pmsg)
{
    scfg_deploy_cfg_t *pcfg = scfg_get_deploy_cfg();
    SCfgUnbound_t *req;
    char *ifname;
    int mymax;
    int mymin;
    int idx;

    req = &pmsg->op.unBoundNotify;
    if (NULL == req->ifname.buf)
    {
        scfg_log(E_ERROR, "Invalid unbound message (NULL string)");
        return 0;
    }

    ifname = (char *)req->ifname.buf;
    if (!pal_strncasecmp(ifname, M_DEPLOY_MAN_DEV_STR, req->ifname.size))
    {
        mymin = 0;
        mymax = DHC_MAX_TFTPSRV_RECORDS;
        pcfg->manVrfid = 0;
    }
    else
    {
        mymin = DHC_MAX_TFTPSRV_RECORDS;
        mymax = DHC_MAX_TFTPSRV_RECORDS * 2;
        pcfg->vrfid = 0;
    }

    for (idx = mymin; idx < mymax; idx++)
    {
        if (NULL == pcfg->tftpSrvs[idx])
        {
            break;
        }

        XFREE(MTYPE_TMP, pcfg->tftpSrvs[idx]);
        pcfg->tftpSrvs[idx] = NULL;
        pcfg->nAddrCnt--;
    }

    return 0;
}

int scfg_client_rx_initAck(struct message_handler *pms, SCfgMsg_t *pmsg)
{
    return 0;
}

int scfg_client_tx_initNotify(int sock)
{
    SCfgMsg_t msg;

    pal_mem_set(&msg, 0, sizeof(SCfgMsg_t));
    msg.op.present = op_PR_initNotify;
    msg.op.initNotify.code = 0;
#define _M_INIT_NOTIFY_MSG  "connect success"
    msg.op.initNotify.msg.buf = (unsigned char *)_M_INIT_NOTIFY_MSG;
    msg.op.initNotify.msg.size = pal_strlen(_M_INIT_NOTIFY_MSG);

    return scfg_client_tx_msg(&msg);
}

scfg_msg_cb_t scfg_client_cb[op_PR_maxMsg] =
{
    [op_PR_boundNotify] = scfg_client_rx_bound,
    [op_PR_unBoundNotify] = scfg_client_rx_unbound,
    [op_PR_initAck] = scfg_client_rx_initAck,
};

static unsigned char g_sys_mac[MAC_ADDR_LEN * 3];
static chsm_sys_info_t g_sys_info;

inline unsigned char *scfg_get_sys_mac(void)
{
    return g_sys_mac;
}

inline char *scfg_get_serial_no(void)
{
    return (char *)g_sys_info.serial_no;
}

inline uint8_t scfg_get_hw_ver(void)
{
    return g_sys_info.hw_ver;
}

inline uint8_t scfg_get_epld_ver(void)
{
    return g_sys_info.epld_ver;
}

inline uint8_t scfg_get_scfg_flag(void)
{
    return g_sys_info.scfgFlags;
}

int scfg_init_sys_env(void)
{
    unsigned char sys_mac[MAC_ADDR_LEN];
    chsm_client_get_sys_mac(sys_mac);
    pal_snprintf((char *)g_sys_mac, MAC_ADDR_LEN * 3,
            "%02X%02X.%02X%02X.%02X%02X",
            sys_mac[0], sys_mac[1], sys_mac[2],
            sys_mac[3], sys_mac[4], sys_mac[5]);
    chsm_client_get_sys_info(&g_sys_info);

    log_sys(M_MOD_SCFG, E_WARNING, 
            "Sys-MAC: %s\n", g_sys_mac);
    log_sys(M_MOD_SCFG, E_WARNING, 
            "System Serial NO: %s\n", g_sys_info.serial_no);
    log_sys(M_MOD_SCFG, E_WARNING,
            "System Product ID: %s\n", oem_info_get_hardware_type(NULL));
    log_sys(M_MOD_SCFG, E_WARNING, 
            "System hardware version: %u\n", g_sys_info.hw_ver);
    log_sys(M_MOD_SCFG, E_WARNING, 
            "System epld version: %u\n", g_sys_info.epld_ver);
    log_sys(M_MOD_SCFG, E_WARNING, 
            "SmartConfig flag: 0x%x\n", g_sys_info.scfgFlags);
    return 1;
}

int scfg_init_deploy_start(void)
{
    pal_mem_set(&g_ideploy_ctrl, 0x00, sizeof(scfg_deploy_ctrl_t));
    g_ideploy_ctrl.state = E_SCFG_INITD_NONE;

    if (!scfg_init_sys_env())
    {
        return 0;
    }
    return 1;
}

