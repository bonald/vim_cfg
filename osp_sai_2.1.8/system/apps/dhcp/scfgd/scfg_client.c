/****************************************************************************
* $Id$
*  Smart Config Message Implements
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-23 16:19
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
#include "glb_smartcfg.h"
#include "dhc_common.h"
#include "scfg_client.h"
#include "SCfgMsg.h"
#include "thread.h"

#include "scfg_client_cb.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#ifndef SCFG_MSG_BUF_SZ
#define SCFG_MSG_BUF_SZ     4096
#define SCFG_MSG_PATH       "/tmp/.smart_config"
#endif

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

struct message_handler *g_pScfgClientHandler = NULL;
char *g_pScfgClientBuf = NULL;
int g_pScfgClientBufLen = 0;

scfg_deploy_cfg_t *g_pScfgDeployCfg = NULL;

int scfg_client_start(struct message_handler *pms);

/****************************************************************************
 *
* Function
* 
****************************************************************************/

int scfg_client_rx_msg(struct message_handler *pms, SCfgMsg_t *pmsg)
{
    int ret = 1;
    scfg_msg_cb_t cb;

    if (pmsg->op.present >= op_PR_maxMsg)
    {
        return 0;
    }

    if ((cb = scfg_client_cb[pmsg->op.present]))
    {
        ret = cb(pms, pmsg);
    }
    return ret;
}

static unsigned int g_ulMsgId = 0;

int scfg_client_tx_msg(SCfgMsg_t *pmsg)
{
    uint8_t buf[SCFG_MSG_BUF_SZ];
    size_t len = SCFG_MSG_BUF_SZ;
    asn_enc_rval_t enc_rc;
    int sock;

    if (NULL == g_pScfgClientHandler || -1 == g_pScfgClientHandler->sock)
    {
        return 0;
    }

    pmsg->ver = 1;
    pmsg->id = g_ulMsgId++;

    sock = g_pScfgClientHandler->sock;

#if 0
    fprintf(stderr, "+++ Send msg to sock %d id %ld +++\n", sock, pmsg->id);
    xer_fprint(stderr, &asn_DEF_SCfgMsg, pmsg);
    printf("------------------------\n");
#endif

    enc_rc = der_encode_to_buffer(&asn_DEF_SCfgMsg, pmsg, buf, &len);
    if (enc_rc.encoded < 0)

    {
        log_sys(M_MOD_SCFG, E_ERROR, 
                "encode message %u failed", pmsg->op.present);
        return 0;
    }

    if (write(sock, buf, len) != len)
    {
        log_sys(M_MOD_SCFG, E_ERROR, 
                "send message %u failed", pmsg->op.present);
        return 0;
    }

    return 1;
}

int scfg_client_proc_msg(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    SCfgMsg_t *pmsg = NULL;
    asn_dec_rval_t dec_rc;
    char *p = g_pScfgClientBuf;
    int left = g_pScfgClientBufLen;
    int ret;

    for (;;)
    {
        dec_rc = ber_decode(0, &asn_DEF_SCfgMsg, (void **)&pmsg, p, left);
        switch (dec_rc.code) {
        case RC_OK:
#if 0
            fprintf(stderr, "+++ recv msg from sock %d id %ld +++\n", sock, pmsg->id);
            xer_fprint(stderr, &asn_DEF_SCfgMsg, pmsg);
            printf("------------------------\n");
#endif
            ret = scfg_client_rx_msg(pms, pmsg);
            asn_DEF_SCfgMsg.free_struct(&asn_DEF_SCfgMsg, pmsg, 0);
            pmsg = NULL;
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left == 0) 
            {
                g_pScfgClientBufLen = 0;
                return 0;
            }
            break;

        case RC_WMORE:
            if (left > 0)
            {
                pal_mem_move(g_pScfgClientBuf, p, left);
            }
            g_pScfgClientBufLen = left;
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

int scfg_client_read(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    int nRead = 0;
    char *p = g_pScfgClientBuf;
    int len = g_pScfgClientBufLen;

    while (1)
    {
        nRead = pal_sock_read(sock, p + len , SCFG_MSG_BUF_SZ - len);
        if (nRead > 0)
        {
            g_pScfgClientBufLen += nRead;
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


    return scfg_client_proc_msg(pms, pme, sock);
}

int scfg_client_reconnect(struct thread *th)
{
    struct message_handler *pms = (struct message_handler *)THREAD_ARG(th);

    pms->t_connect = NULL;
    scfg_client_start(pms);
    return 0;
}

int scfg_client_start(struct message_handler *pms)
{
    int ret;
    
    ret = message_client_start(pms);
    if (ret < 0)
    {
        if (NULL == pms->t_connect)
        {
            pms->t_connect = thread_add_timer(lib_globals_get(),
                    scfg_client_reconnect, pms, 5);
        }
    }
    return 0;
}

int scfg_client_disconnect(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    log_sys(M_MOD_SCFG, E_ERROR, "Discnnect from dhclient");
    if (NULL == pms->t_connect)
    {
        pms->t_connect = thread_add_timer(pms->zg, scfg_client_reconnect,
                pms, 5);
    }
    return 0;
}

int scfg_client_connect(struct message_handler *pms,
        struct message_entry *pme, pal_sock_handle_t sock)
{
    log_sys(M_MOD_SCFG, E_ERROR, "Connect to dhclient success");
    if (0 == scfg_client_tx_initNotify(sock))
    {
        log_sys(M_MOD_SCFG, E_ERROR, "Send initialize notify to dhclient failed\n");
    }
    if (NULL != pms->t_read)
    {
        thread_cancel(pms->t_read);
    }
    pms->t_read = thread_add_read (pms->zg, message_client_read, pms, sock);
    return 0;
}

int scfg_client_init(struct lib_globals *pzg)
{
    pal_assert(NULL != pzg);
    if (NULL == pzg)
    {
        return 0;
    }

    char *pmem;
    pmem = XMALLOC(MTYPE_TMP, sizeof(char) * SCFG_MSG_BUF_SZ);
    if (NULL == pmem)
    {
        return 0;
    }

    struct message_handler *pmh;
    if ((pmh = message_client_create(pzg, MESSAGE_TYPE_ASYNC)) == NULL)
    {
        XFREE(MTYPE_TMP, pmem);
        return 0;
    }

    message_client_set_style_domain(pmh, SCFG_MSG_PATH);
    message_client_set_callback(pmh, MESSAGE_EVENT_CONNECT,
            scfg_client_connect);
    message_client_set_callback(pmh, MESSAGE_EVENT_DISCONNECT,
            scfg_client_disconnect);
    message_client_set_callback(pmh, MESSAGE_EVENT_READ_MESSAGE,
            scfg_client_read);
    g_pScfgClientHandler = pmh;
    g_pScfgClientBuf = pmem;
    g_pScfgClientBufLen = 0;

    scfg_client_start(pmh);
    return 1;
}

scfg_deploy_cfg_t *scfg_get_deploy_cfg(void)
{
    pal_assert(NULL != g_pScfgDeployCfg);
    return g_pScfgDeployCfg;
}

int scfg_file_exist(char *filename)
{
    struct stat stat_buf;

    if (!pal_stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}

void scfg_client_deploy_init(scfg_deploy_cfg_t *pCfg)
{
    int flags = scfg_get_scfg_flag();

    pCfg->interval = M_DEPLOY_DEFAULT_INTERVAL;
    pCfg->retryCnt = M_DEPLOY_DEFAULT_TRYCNT;
    pCfg->bStartupCfgEmpty = scfg_file_exist("/tmp/glb_empty_startup_cfg");
    pCfg->flags = (unsigned char)(flags & 0xff);
    return;
}

int scfg_client_module_init(struct lib_globals *pzg)
{
    pal_assert(NULL != pzg);
    if (NULL != g_pScfgClientHandler) /* Initialized */
    {
        return 1;
    }

    if (!scfg_client_init(pzg))
    {
        return 0;
    }

    g_pScfgDeployCfg = XCALLOC(MTYPE_TMP, sizeof(scfg_deploy_cfg_t));
    if (NULL == g_pScfgDeployCfg)
    {
        return 0;
    }

    scfg_client_deploy_init(g_pScfgDeployCfg);
    return 1;
}
