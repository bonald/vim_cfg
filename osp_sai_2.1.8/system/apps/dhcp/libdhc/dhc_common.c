/****************************************************************************
* $Id$
*  common function library for dhclient messages process
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-03 18:18
* Reason        : First Create.
****************************************************************************/

#include "dhc_common.h"
#include "param_check.h"

int
dhc_comm_encode_bound_msg(nsm_msg_dhc_bound_t *pmsg, u_int8_t **buf, u_int16_t *size,
    int (*pencode_func)(u_int8_t **, u_int16_t *, nsm_msg_dhc_bound_t *))
{
    int nbytes;

    if (NULL == pmsg || NULL == buf || NULL == size)
    {
        return -1;
    }

    pal_assert(NULL != pmsg->ifname);
    pal_assert(NULL != pmsg->ip);
    pal_assert(NULL != pmsg->mask);

    pal_mem_set(&pmsg->cindex, 0x00, sizeof(cindex_t));
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_IFNAME);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_VRFID);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_IPADDR);
    if (NULL != pmsg->tftpsrv_ip)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_TFTPSRV_IPADDR);
    }

    if (NULL != pmsg->routers)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_ROUTERS);
    }
    if (NULL != pmsg->classful_routers)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_CFS_ROUTERS);
    }

    if (NULL != pmsg->classless_routers)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHC_BOUND_CTYPE_CLS_ROUTERS);
    }

    nbytes = pencode_func(buf, size, pmsg);
    if (nbytes < 0)
    {
        return -1;
    }

    return nbytes;
}

int
dhc_comm_send_notify(void *hdl, int what, int result, 
        char *psz_errmsg, size_t errmsg_len, char *ifname, s_int32_t vrfid)
{
    struct nsm_client_handler *nch = (struct nsm_client_handler *)hdl;
    nsm_msg_dhc_notify_t msg;
    u_char *pnt;
    u_int16_t size;
    u_int32_t msg_id;
    int nbytes;

    pal_mem_set(&msg, 0x00, sizeof(nsm_msg_dhc_notify_t));

    /* Init Cindex. */
    NSM_SET_CTYPE (msg.cindex, NSM_DHC_NTF_CTYPE_IFNAME);
    msg.ifname = ifname;
    msg.ifname_len = pal_strlen(ifname);

    NSM_SET_CTYPE (msg.cindex, NSM_DHC_NTF_CTYPE_VRFID);
    msg.vrfid = vrfid;

    NSM_SET_CTYPE (msg.cindex, NSM_DHC_NTF_CTYPE_FLAGS);
    msg.result = result;
    msg.what = what;
    msg.protocol_id = lib_globals_get()->protocol;

    if (NULL != psz_errmsg && '\0' != psz_errmsg[0])
    {
        NSM_SET_CTYPE (msg.cindex, NSM_DHC_NTF_CTYPE_ERRMSG);
        msg.err_msg_len = errmsg_len;
        msg.err_msg = psz_errmsg;
    }

    /* Set pnt and size.  */
    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    nbytes = nsm_encode_dhc_notify(&pnt, &size, &msg);
    if (nbytes < 0)
    {
        return -1;
    }

    nsm_client_send_message(nch, 0, 0, NSM_MSG_DHC_NOTIFY, nbytes, &msg_id);
    return 0;
}

int
dhc_comm_send_cmd(void *hdl, int what, char *psz_reason, 
        size_t reason_len, char *ifname, s_int32_t vrfid)
{
    struct nsm_client_handler *nch = (struct nsm_client_handler *)hdl;
    nsm_msg_dhc_cmd_t msg;
    u_char *pnt;
    u_int16_t size;
    u_int32_t msg_id;
    int nbytes;

    pal_mem_set(&msg, 0x00, sizeof(nsm_msg_dhc_cmd_t));

    /* Init Cindex. */
    NSM_SET_CTYPE (msg.cindex, NSM_DHC_CMD_CTYPE_IFNAME);
    msg.ifname = ifname;
    msg.ifname_len = pal_strlen(ifname);

    NSM_SET_CTYPE (msg.cindex, NSM_DHC_CMD_CTYPE_VRFID);
    msg.vrfid = vrfid;

    NSM_SET_CTYPE (msg.cindex, NSM_DHC_CMD_CTYPE_FLAGS);
    msg.what = what;
    msg.protocol_id = lib_globals_get()->protocol;

    if (NULL != psz_reason && '\0' != psz_reason[0])
    {
        NSM_SET_CTYPE (msg.cindex, NSM_DHC_CMD_CTYPE_REASON);
        msg.reason_len = reason_len;
        msg.reason = psz_reason;
    }

    /* Set pnt and size.  */
    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    nbytes = nsm_encode_dhc_cmd(&pnt, &size, &msg);
    if (nbytes < 0)
    {
        return -1;
    }

    nsm_client_send_message(nch, 0, 0, NSM_MSG_DHC_CMD, nbytes, &msg_id);
    return 0;
}

/* Base on RFC 3442 */
int
_get_classless_router_dest_bytes(int mask_width, int *bytes)
{
    if (0 == mask_width)
    {
        *bytes = 0;
    }
    else if (mask_width >= 1 && mask_width <= 8)
    {
        *bytes = 1;
    }
    else if (mask_width >= 9 && mask_width <= 16)
    {
        *bytes = 2;
    }
    else if (mask_width >= 17 && mask_width <= 24)
    {
        *bytes = 3;
    }
    else if (mask_width >= 25 && mask_width <= 32)
    {
        *bytes = 4;
    }
    else
    {
        return 0;
    }

    return 1;
}

/* Base on RFC 3442
 * format: mask_width:dest-bytes:A:B:C:D 
 */
int 
dhc_comm_install_classless_routers(nsm_msg_dhc_bound_t *pbound, 
        dhc_comm_rt_cb_arg_t *proc)
{
    dhc_comm_routers_rec_t item;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nRet = CLI_ERROR;
    int cnt, idx, records;
    char *psz_routers = (char *)pbound->classless_routers;
    char *psz_errmsg = proc->errmsg;
    unsigned char distance = pbound->distance;

    if (NULL == psz_routers)
    {
        return CLI_SUCCESS;
    }

    if (pal_strspn(psz_routers, "0123456789abcdefABCDEF:")
             != pal_strlen(psz_routers))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid classless-static-routers string %s", psz_routers);
        goto err_out;
    }

    if (split_tokens(psz_routers, pal_strlen(psz_routers), DHC_MAX_OPTION_121_BYTES,
                ":", &nToks, &ppTokArr) != 0 || nToks < 5)
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid classless-static-routers string %s", psz_routers);
        goto err_out;
    }

    cnt = nToks;
    idx = 0;
    records = 0;
    do
    {
        int addr[4];
        char szBuf[DHC_PREFIX_BUF_LEN];
        int mask_width;
        int i, bytes;

        pal_mem_set(&item, 0x00, sizeof(dhc_comm_routers_rec_t));

        /* get mask width */
        sscanf(ppTokArr[idx], "%x", &mask_width);
        cnt--;
        bytes = 0;
        if (!_get_classless_router_dest_bytes(mask_width, &bytes))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classless-static-routers string at byte %d, bad mask width %s",
                    idx + 1, ppTokArr[idx]);
            goto err_out;
        }

        item.mask_width = mask_width;

        /* check & get subnet number */
        if ((idx + 1 + bytes) > nToks)
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classless-static-routers string at byte %d, miss subnet mask number",
                    idx + 1);
            goto err_out;
        }

        addr[0] = addr[1] = addr[2] = addr[3] = 0;
        for (i = 0; i < bytes; i++)
        {
            idx++;
            cnt--;
            sscanf(ppTokArr[idx], "%x", &addr[i]);
        }
        pal_snprintf(szBuf, DHC_PREFIX_BUF_LEN, "%d.%d.%d.%d/%d",
                addr[0], addr[1], addr[2], addr[3], mask_width);
        if (!str2prefix_ipv4(szBuf, &item.dest))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classless-static-routers destination descriptor %s",
                    szBuf);
            goto err_out;
        }

        /* check & get router address, 4 bytes */
        if ((idx + 1 + 4) > nToks)
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classless-static-routers string at byte %d, miss router address",
                    idx + 1);
            goto err_out;
        }

        addr[0] = addr[1] = addr[2] = addr[3] = 0;
        for (i = 0; i < 4; i++)
        {
            idx++;
            cnt--;
            sscanf(ppTokArr[idx], "%x", &addr[i]);
        }

        snprintf(szBuf, DHC_PREFIX_BUF_LEN, "%d.%d.%d.%d",
                addr[0], addr[1], addr[2], addr[3]);
        if (!str2prefix_ipv4(szBuf, &item.router))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classless-static-routers router address %s",
                    szBuf);
            goto err_out;
        }

        idx++;
        if (CLI_SUCCESS != proc->cb_install_static_router(proc, &item, distance))
        {
            goto err_out;
        }
        if (distance < DHC_MAX_DISTANCE)
        {
            distance++;
        }
        records++;
        if (DHC_MAX_ROUTERS_RECORDS <= records)
        {
            break;
        }
    } while (cnt >= 5);

    if (cnt > 0)
    {
        ;
    }

    nRet = CLI_SUCCESS;

err_out:
    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return nRet;
}

/* format: A.B.C.D A.B.C.D, A.B.C.D A.B.C.D */
int
dhc_comm_install_classful_routers(nsm_msg_dhc_bound_t *pbound, 
        dhc_comm_rt_cb_arg_t *proc)
{
    dhc_comm_routers_rec_t item;
    char *psz_errmsg = proc->errmsg;
    char *psz_routers = (char *)pbound->classful_routers;
    int nRet = CLI_ERROR;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    char **ppSubTokArr = NULL;
    unsigned long nSubToks = 0;
    int nProcToks;
    int idx;
    unsigned char distance = pbound->distance;

    if (NULL == psz_routers)
    {
        return CLI_SUCCESS;
    }

    if (pal_strspn(psz_routers, "0123456789., ") != pal_strlen(psz_routers))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid classful-static-routers string %s", psz_routers);
        goto err_out;
    }

    /* for bug 17502 modified by liuyang for one route 2011-12-21*/
    if (split_tokens(psz_routers, pal_strlen(psz_routers), DHC_MAX_OPTION_33_BYTES,
                ",", &nToks, &ppTokArr) != 0)
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid classful-static-routers string %s", psz_routers);
        goto err_out;
    }

    nProcToks = (nToks < DHC_MAX_ROUTERS_RECORDS) ? 
        nToks : DHC_MAX_ROUTERS_RECORDS;
    for (idx = 0; idx < nProcToks; idx++)
    {
        if (split_tokens(ppTokArr[idx], pal_strlen(ppTokArr[idx]), 2, 
                    " ", &nSubToks, &ppSubTokArr) != 0 || (2 != nSubToks))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classful-static-routers string: %s",
                    ppTokArr[idx]);
            goto err_out;
        }

        pal_mem_set(&item, 0x00, sizeof(dhc_comm_routers_rec_t));

        /* check router address */
        if (!str2prefix_ipv4(ppSubTokArr[1], &item.router))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classfull-static-routers router address %s",
                    ppSubTokArr[1]);
            goto err_out;
        }

        /* get subnet number */
        if (!str2prefix_ipv4(ppSubTokArr[0], &item.dest)) /* no subnet mask /m */
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid classfull-static-routers subnet number %s",
                    ppSubTokArr[0]);
            goto err_out;
        }

        item.mask_width = item.dest.prefixlen;
        if (CLI_SUCCESS != proc->cb_install_static_router(proc, &item, distance))
        {
            goto err_out;
        }
        
        if (distance < DHC_MAX_DISTANCE)
        {
            distance++;
        }

        free_tokens(&ppSubTokArr);
        ppSubTokArr = NULL;
    }

    nRet = CLI_SUCCESS;

err_out:
    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    if (NULL != ppSubTokArr)
    {
        free_tokens(&ppSubTokArr);
    }
    return nRet;
}

/* format: A.B.C.D,A.B.C.D */
int
dhc_comm_install_static_routers(nsm_msg_dhc_bound_t *pbound, 
        dhc_comm_rt_cb_arg_t *proc)
{
    unsigned char distance = pbound->distance;
    dhc_comm_routers_rec_t item;
    char *psz_errmsg = proc->errmsg;
    char *psz_routers = (char *)pbound->routers;
    int nRet = CLI_ERROR;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nProcToks;
    int idx;

    if (NULL == psz_routers)
    {
        return CLI_SUCCESS;
    }

    if (pal_strspn(psz_routers, "0123456789.,")
             != pal_strlen(psz_routers))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid routers string %s", psz_routers);
        goto err_out;
    }

    if (split_tokens(psz_routers, pal_strlen(psz_routers), 
                DHC_MAX_OPTION_3_BYTES, ",", &nToks, &ppTokArr) != 0)
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid routers string %s", psz_routers);
        goto err_out;
    }

    nProcToks = (nToks < DHC_MAX_ROUTERS_RECORDS) ? 
        nToks : DHC_MAX_ROUTERS_RECORDS;
    for (idx = 0; idx < nProcToks; idx++)
    {
        pal_mem_set(&item, 0x00, sizeof(dhc_comm_routers_rec_t));

        /* check router address */
        if (!str2prefix_ipv4(ppTokArr[idx], &item.router))
        {
            pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid routers router address %s", ppTokArr[idx]);
            goto err_out;
        }
        str2prefix_ipv4("0.0.0.0/0", &item.dest);
        if (CLI_SUCCESS != proc->cb_install_static_router(proc, &item, distance))
        {
            goto err_out;
        }

        if (distance < DHC_MAX_DISTANCE)
        {
            distance++;
        }
    }

    nRet = CLI_SUCCESS;

err_out:
    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return nRet;
}

int
dhc_comm_install_ip_address(nsm_msg_dhc_bound_t *pbound, dhc_comm_ip_cb_arg_t *proc)
{
    char *psz_ip = NULL;
    char *psz_mask = NULL;
    int ret = CLI_ERROR;
    char *psz_errmsg = proc->errmsg;
    struct prefix_ipv4 p;
    struct sal_in4_addr mask;

    if (NULL == pbound || NULL == pbound->ip || NULL == pbound->mask)
    {
        return -1;
    }

    if (NULL == (psz_ip = sockunion_su2str((union sockunion *)pbound->ip)))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, "Invalid ip address");
        goto err_out;
    }
    if (NULL == (psz_mask = sockunion_su2str((union sockunion *)pbound->mask)))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, "Invalid subnet mask");
        goto err_out;
    }

    if (!pal_inet_pton (AF_INET, psz_ip, &p.prefix) 
            || !pal_inet_pton (AF_INET, psz_mask, &mask))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, "Invalid ip address or mask");
        goto err_out;
    }

    if (!cli_mask_check (mask))
    {
        pal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, "Invalid subnet mask");
        goto err_out;
    }
    p.family = AF_INET;
    p.prefixlen = ip_masklen (mask);

    ret = proc->cb_install_ip(proc, &p, &mask);

err_out:
    if (CLI_SUCCESS != ret)
    {
        /* modified by liuyang 2011-11-24 */
        log_sys(M_MOD_DHCLIENT, E_ERROR, "install/uninstall address %s/%s failed: %s", 
                psz_ip ? psz_ip : "N/A", psz_mask ? psz_mask : "N/A",
                psz_errmsg);
    }
    if (NULL != psz_ip)
    {
        XFREE(MTYPE_TMP, psz_ip);
    }

    if (NULL != psz_mask)
    {
        XFREE(MTYPE_TMP, psz_mask);
    }
    return ret;
}

int
dhc_comm_install_ip_routers(nsm_msg_dhc_bound_t *pbound,
        dhc_comm_rt_cb_arg_t *proc)
{
    int ret;
    int failed = 0;

    if (NULL != pbound->classless_routers)
    {
        ret = dhc_comm_install_classless_routers(pbound, proc);
        if (CLI_SUCCESS != ret)
        {
            failed = 1;
        }
    }
    else if (NULL != pbound->classful_routers)
    {
        if (CLI_SUCCESS != dhc_comm_install_classful_routers(pbound, proc))
        {
            failed = 1;
        }
    }

    if (NULL != pbound->routers)
    {
        if (CLI_SUCCESS != dhc_comm_install_static_routers(pbound, proc))
        {
            failed = 1;
        }
    }

    return failed ? CLI_ERROR : CLI_SUCCESS;
}


