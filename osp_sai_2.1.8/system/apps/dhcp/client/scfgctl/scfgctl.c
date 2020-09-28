/****************************************************************************
* $Id$
*  Smart Config Controller Implements
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-22 14:53
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#if 0
#include "dhcpd.h"

#include "pal.h"
#include "cli.h"
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "glb_smartcfg.h"
#include "chsm_port.h"
#include "param_check.h"
#include "message.h"
#include "SCfgMsg.h"
#include "scfgctl.h"
#include "chsm_client.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#define SCFG_MSG_PATH       "/tmp/.smart_config"

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
extern int dhclient_foreach_bounded_intf(scfgctl_bound_cb_t pFunc, void *pArg);
extern void dhclient_do_reboot(char *, size_t);

static struct message_handler *g_pSCfgMsgHandler = NULL;
typedef int (*scfgctl_cb_t)(scfgctl_sess_t *psess, SCfgMsg_t *);
int scfgctl_send_msg(SCfgMsg_t *pmsg);
int scfgctl_rx_initNotify(scfgctl_sess_t *psess, SCfgMsg_t *);
int scfgctl_rx_rebootCmd(scfgctl_sess_t *psess, SCfgMsg_t *);

scfgctl_cb_t scfgctl_cb[op_PR_maxMsg] =
{
    [op_PR_initNotify] = scfgctl_rx_initNotify,
    [op_PR_rebootCmd] = scfgctl_rx_rebootCmd,
};

/****************************************************************************
 *
* Function
* 
****************************************************************************/

int scfgctl_send_unbound(char *ifname, int32_t vrfid)
{
    SCfgMsg_t req;

    pal_mem_set(&req, 0, sizeof(SCfgMsg_t));
    req.op.present = op_PR_unBoundNotify;
    req.op.unBoundNotify.ifname.buf = (unsigned char *)ifname;
    req.op.unBoundNotify.ifname.size = pal_strlen(ifname);
    req.op.unBoundNotify.vrfid = vrfid;

    scfgctl_send_msg(&req);
    return 0;
}

int scfgctl_send_bound(char *ifname, int32_t vrfid, char *tftpaddrs, size_t len)
{
    SCfgMsg_t req;

    pal_mem_set(&req, 0, sizeof(SCfgMsg_t));
    req.op.present = op_PR_boundNotify;
    req.op.boundNotify.ifname.buf = (unsigned char *)ifname;
    req.op.boundNotify.ifname.size = pal_strlen(ifname);
    req.op.boundNotify.vrfid = vrfid;
    req.op.boundNotify.tftpaddrs.buf = (unsigned char *)tftpaddrs;
    req.op.boundNotify.tftpaddrs.size = len;

    scfgctl_send_msg(&req);
    return 0;
}

int scfgctl_send_msg_by_session(scfgctl_sess_t *psess, SCfgMsg_t *pmsg)
{
    uint8_t buf[SCFG_MSG_BUF_SZ];
    size_t len = SCFG_MSG_BUF_SZ;
    asn_enc_rval_t enc_rc;

    pmsg->ver = 1;
    pmsg->id = psess->id++;

#if 0
    fprintf(stderr, "+++ Send msg to sock %d id %ld +++\n", psess->sock, pmsg->id);
    xer_fprint(stderr, &asn_DEF_SCfgMsg, pmsg);
    printf("------------------------\n");
#endif

    enc_rc = der_encode_to_buffer(&asn_DEF_SCfgMsg, pmsg, buf, &len);
    if (enc_rc.encoded < 0)

    {
        log_sys(M_MOD_SCFG, E_ERROR, "encode message failed");
        return 0;
    }

    if (write (psess->sock, buf, len) != len)
    {
        log_sys(M_MOD_SCFG, E_ERROR, "send message failed");
        return 0;
    }
    return 1;
}

int scfgctl_send_bound_bysession(char *ifname, int32_t vrfid, 
        char *tftpaddrs, size_t len, void *pArg)
{
    SCfgMsg_t msg;

    pal_mem_set(&msg, 0, sizeof(SCfgMsg_t));
    msg.op.present = op_PR_boundNotify;
    msg.op.boundNotify.ifname.buf = (unsigned char *)ifname;
    msg.op.boundNotify.ifname.size = pal_strlen(ifname);
    msg.op.boundNotify.vrfid = vrfid;
    msg.op.boundNotify.tftpaddrs.buf = (unsigned char *)tftpaddrs;
    msg.op.boundNotify.tftpaddrs.size = len;

    scfgctl_send_msg_by_session((scfgctl_sess_t *)pArg, &msg);
    return 0;
}


int scfgctl_send_msg(SCfgMsg_t *pmsg)
{
    scfgctl_sess_t *psess;
    int ret = 1;
    int i;

    VECTOR_LOOP(g_pSCfgMsgHandler->clist, psess, i)
    {
        if (!scfgctl_send_msg_by_session(psess, pmsg))
        {
            ret = 0;
        }
    }

    return ret;
}

int scfgctl_rx_msg(scfgctl_sess_t *psess, SCfgMsg_t *pmsg)
{
    int ret = 1;
    scfgctl_cb_t cb;

    if (pmsg->op.present >= op_PR_maxMsg)
    {
        return 0;
    }

    if ((cb = scfgctl_cb[pmsg->op.present]))
    {
        ret = cb(psess, pmsg);
    }
    return ret;
}

int scfgctl_rx_rebootCmd(scfgctl_sess_t *psess, SCfgMsg_t *pmsg)
{
    dhclient_do_reboot((char *)pmsg->op.rebootCmd.msg.buf, 
            pmsg->op.rebootCmd.msg.size);
    return 0;
}

int scfgctl_rx_initNotify(scfgctl_sess_t *psess, SCfgMsg_t *pmsg)
{
    dhclient_foreach_bounded_intf(scfgctl_send_bound_bysession, psess);
    return 0;
}

int scfgctl_proc_msg(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    SCfgMsg_t *pmsg = NULL;
    asn_dec_rval_t dec_rc;
    scfgctl_sess_t *psess = (scfgctl_sess_t *)pme->info;
    char *p = psess->buf;
    int left = psess->len;
    int ret;

    for (;;)
    {
        dec_rc = ber_decode(0, &asn_DEF_SCfgMsg, (void **)&pmsg, p, left);
        switch (dec_rc.code) {
        case RC_OK:
#if 0
            xer_fprint(stdout, &asn_DEF_SCfgMsg, pmsg);
#endif
            ret = scfgctl_rx_msg(psess, pmsg);
            asn_DEF_SCfgMsg.free_struct(&asn_DEF_SCfgMsg, pmsg, 0);
            pmsg = NULL;
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left == 0) 
            {
                psess->len = 0;
                return 0;
            }
            break;

        case RC_WMORE:
            if (left > 0)
            {
                pal_mem_move(psess->buf, p, left);
            }
            psess->len = left;
            if (pmsg)
            {
                asn_DEF_SCfgMsg.free_struct(&asn_DEF_SCfgMsg, pmsg, 0);
                pmsg = NULL;
            }
            return 0;

        case RC_FAIL:
            /* XXX */
            pal_assert(0);
            /* close connection */
            return -1;
        }
    }

    return 0;
}

int scfgctl_read(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    int nRead = 0;
    scfgctl_sess_t *psess = (scfgctl_sess_t *)pme->info;
    int len = psess->len;

    while (1)
    {
        nRead = pal_sock_read(sock, psess->buf + len, SCFG_MSG_BUF_SZ - len);
        if (nRead > 0)
        {
            psess->len += nRead;
            break;
        }

        if (0 == nRead)
        {
            /* Connection closed */
            return -1;
        }

        switch (errno)
        {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)        
            case EWOULDBLOCK:
#endif
                pal_delay(0);
                continue;
        }

        return nRead;
    }


    return scfgctl_proc_msg(pms, pme, sock);
}

int scfgctl_disconnect(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    void *data;
    int i;

    VECTOR_LOOP(pms->clist, data, i)
    {
        if (data == pme->info)
        {
            vector_unset(pms->clist, i);
            break;
        }
    }

    if (NULL != pme->info)
    {
        XFREE(MTYPE_TMP, pme->info);
        pme->info = NULL;
    }
    return 0;
}

int scfgctl_connect(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    scfgctl_sess_t *psess = XCALLOC(MTYPE_TMP, sizeof(scfgctl_sess_t));

    if (NULL == psess)
    {
        log_sys(M_MOD_DHCLIENT, E_ERROR, "Warning: Out of memory!");
        return -1;
    }

    pme->info = (void *)psess;
    psess->sock = sock;
    vector_set(pms->clist, psess);
    return 0;
}

int scfgctl_init(struct lib_globals *pzg)
{
    pal_assert(NULL != pzg);
    if (NULL == pzg)
    {
        return 0;
    }

    struct message_handler *pmh;
    if ((pmh = message_server_create(pzg)) == NULL)
    {
        return 0;
    }

    message_server_set_style_domain(pmh, SCFG_MSG_PATH);
    message_server_set_callback(pmh, MESSAGE_EVENT_CONNECT,
            scfgctl_connect);
    message_server_set_callback(pmh, MESSAGE_EVENT_DISCONNECT,
            scfgctl_disconnect);
    message_server_set_callback(pmh, MESSAGE_EVENT_READ_MESSAGE,
            scfgctl_read);
    message_server_start(pmh);
    g_pSCfgMsgHandler = pmh;
    return 1;
}

unsigned char _get_smartcfg_config(void)
{
    FILE *fp;
    unsigned int flags;

    flags = 0;
    fp = pal_fopen(GLB_SMARTCFG_FILE, "r");
    if (NULL != fp)
    {
        fscanf(fp, "0x%x", &flags);
        pal_fclose(fp);
    }
    return (unsigned char)(flags & 0xff);
}

CLI (show_smartcfg_config,
     show_smartcfg_config_cmd,
     "show smart-config config",
     CLI_SHOW_STR,
     "SmartConfig",
     "Configuration")
{
    int flags = _get_smartcfg_config();

    cli_out(cli, "Smart-Config config: \n");
    cli_out(cli, " initial-switch-deployment: %s\n",
            (flags & M_SCFG_FLAG_INIT_DEPLOYMENT) ? "on" : "off");
    cli_out(cli, " hostname-prefix: %s\n",
            (flags & M_SCFG_FLAG_HOSTNAME_PREFIX) ? "on" : "off");
    cli_out (cli, "\n");
    cli_out(cli, " Send log message to console: %s\n",
            (flags & M_SCFG_FLAG_LOG_TO_CONSOLE) ? "on" : "off");
    return CLI_SUCCESS;
}

CLI (smartcfg_config_set,
     smartcfg_config_set_cmd,
     "smart-config (initial-switch-deployment|hostname-prefix)",
     "SmartConfig",
     "Enable initial switch automatically deployment",
     "Enable hostname prefix feature"
     )
{
    int ret = CLI_SUCCESS;
    int flags = _get_smartcfg_config();

    if (!pal_strcasecmp(argv[0], "initial-switch-deployment"))
    {
        flags |= M_SCFG_FLAG_INIT_DEPLOYMENT;
    }
    else
    {
        flags |= M_SCFG_FLAG_HOSTNAME_PREFIX;
    }

    if (0 != chsm_client_set_scfg_flag(flags))
    {
        cli_out(cli, "%% Save SmartConfig flags failed\n");
        ret = CLI_ERROR;
    }
    return ret;
}

CLI (no_smartcfg_config_set,
     no_smartcfg_config_set_cmd,
     "no smart-config (initial-switch-deployment|hostname-prefix)",
     CLI_NO_STR,
     "SmartConfig",
     "Enable initial switch automatically deployment",
     "Enable hostname prefix feature"
     )
{
    int ret = CLI_SUCCESS;
    int flags = _get_smartcfg_config();

    if (!pal_strcasecmp(argv[0], "initial-switch-deployment"))
    {
        flags &= ~M_SCFG_FLAG_INIT_DEPLOYMENT;
    }
    else
    {
        flags &= ~M_SCFG_FLAG_HOSTNAME_PREFIX;
    }

    if (0 != chsm_client_set_scfg_flag(flags))
    {
        cli_out(cli, "%% Save SmartConfig flags failed\n");
        ret = CLI_ERROR;
    }
    return ret;
}

void scfgctl_smartcfg_init(struct cli_tree *ctree)
{
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_MAX, 0, &show_smartcfg_config_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_MAX, 0, &smartcfg_config_set_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_MAX, 0, &no_smartcfg_config_set_cmd);
    return;
}

int scfgctl_module_init(struct lib_globals *pzg)
{
    pal_assert(NULL != pzg && NULL != pzg->ctree);
    if (NULL != g_pSCfgMsgHandler) /* Initialized */
    {
        return 1;
    }

    if (!scfgctl_init(pzg))
    {
        return 0;
    }

    if (0 != chsm_client_init(pzg))
    {
        return 0;
    }
    scfgctl_smartcfg_init(pzg->ctree);
    return 1;
}
#endif
