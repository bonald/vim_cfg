/****************************************************************************
* $Id$
*  Remote access cmodel
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2016-10-12 09:32
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <assert.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdarg.h>
#include <poll.h>
#include <limits.h>

#include "cm_com_remote.h"
#include "split.h"
#include "duet2/include/drv_error.h"

#include "pktswitch.h"
#include "pktclnt.h"
#include "afx_evtlp.h"
#include "dal.h"
#include "drv_api.h"
#include "chip_agent_dma.h"
/****************************************************************************
 *
* Defines and Macros
*
****************************************************************************/
#define sal_poll        poll
#define sal_pollfd      pollfd
#define sal_strdup      strdup
#define sal_atoi        atoi
//typedef FILE* sal_file_t;
#ifndef sal_calloc
#define sal_calloc      calloc
#endif

/* for packet agent */
typedef struct tagRemoteSess {
    int sock; /* socket id */
    int lchip; /* chip number */
    void *pArg; /* argument for this session */
    int nUse; /* session in use flag */

    int sock_unsync; /* sock for unsync message */
    int sock_server_unsync; /* sock for unsync message proposed by sdk */

    sal_mutex_t *g_pMsgLock;
    sal_mutex_t *g_pNotifyLock;
    sal_mutex_t *g_pNotifyServerLock;

    int g_bExit; /*status check for sock_server_unsync socket*/

} RemoteSess_t;

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/
#define M_SDKNAME_BUFSZ     32
char g_szPathPrefix[PATH_MAX]; /* unix socket path prefix */
char g_szSdkName[M_SDKNAME_BUFSZ]; /* process name */
char g_szTopoCfgFile[PATH_MAX] = {'\0'}; /* topo configuration file */

RemoteSess_t g_RemoteSessArr[M_REMOTE_MAX_SESS];

//static sal_mutex_t *g_pMsgLock = NULL;
//static sal_mutex_t *g_pNotifyLock = NULL;

static u_int32_t g_nReqid = 0;
static u_int32_t g_nReqid_dma = 0;

/* for multi chip */
int g_nRemoteAccSock = -1; /* remote access socket */
afx_mio_t *g_pRemoteAccMio = NULL; /* MIO for remote access socket */
ProcItem_t *g_pInterruptProc = NULL; /* Proc item for interrupt and stats */
ProcItem_t *g_pIOProc = NULL; /* Proc item for IO & DMA */
RemoteAccessCbArg_t g_RemoteAccessCb = { NULL, NULL }; /* remote access callback */
static pthread_mutex_t g_notifyLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_msgLock = PTHREAD_MUTEX_INITIALIZER;

#define M_NOTIFY_LOCK_INIT  pthread_mutex_init(&g_notifyLock, NULL);
#define M_NOTIFY_LOCK       pthread_mutex_lock(&g_notifyLock)
#define M_NOTIFY_UNLOCK     pthread_mutex_unlock(&g_notifyLock)

#define M_MESSAGE_LOCK_INIT pthread_mutex_init(&g_msgLock, NULL);
#define M_MESSAGE_LOCK      pthread_mutex_lock(&g_msgLock)
#define M_MESSAGE_UNLOCK    pthread_mutex_unlock(&g_msgLock)


extern int32 chip_agent_dma_init(uint8 lchip);
int cm_sim_remote_read_cfg(char *pszFilename);

/****************************************************************************
 *
* Function
*
****************************************************************************/

int cm_sim_set_sock_path_prefix(char *pszPathPrefix)
{
    snprintf(g_szPathPrefix, PATH_MAX, "%s", pszPathPrefix);
    return 0;
}

char * cm_sim_get_sock_path_prefix(void)
{
    return g_szPathPrefix;
}

int cm_sim_set_sdk_name(char *pszName)
{
    snprintf(g_szSdkName, M_SDKNAME_BUFSZ, "%s", pszName);
    return 0;
}

char * cm_sim_get_sdk_name(void)
{
    return g_szSdkName;
}

int cm_sim_set_topo_cfg(char *pszFilename)
{
    snprintf(g_szTopoCfgFile, PATH_MAX, "%s", pszFilename);
    return 0;
}

char * cm_sim_get_topo_cfg(void)
{
    return g_szTopoCfgFile;
}

void remote_access_info(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    sal_vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    sal_printf("%s: %s\n", cm_sim_get_sdk_name(), print_buf);
    return;
}

void remote_access_err(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    sal_vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    sal_printf("%s: %s\n", cm_sim_get_sdk_name(), print_buf);
    return;
}

/*SDK-Modify >>*/
int ctc_chip_agent_init(char *pszCfgFile)
{
    char szFile[PATH_MAX];
#if gonghd
    if (NULL == g_RemoteSessArr[0].g_pMsgLock) {
        if (ctc_sal_mutex_create(&g_pMsgLock) != 0) {
            sal_printf("cm_sim_remote_init create message mutex failed\n");
            return -1;
        }
    }
#endif
    if (NULL == pszCfgFile) {
        snprintf(szFile, PATH_MAX, "%s", cm_sim_get_topo_cfg());
    } else {
        snprintf(szFile, PATH_MAX, "%s", pszCfgFile);
    }

    sal_printf("Topo configuraiton file: %s\n", szFile);
    sal_memset(&g_RemoteSessArr, 0x00, sizeof(RemoteSess_t) * M_REMOTE_MAX_SESS);

    return cm_sim_remote_read_cfg(szFile);
}

int ctc_chip_agent_set_env(char *pszName, char *pszPathPrefix, char *pszCfgFile)
{
    cm_sim_set_sdk_name((char *)pszName);
    cm_sim_set_sock_path_prefix((char *)pszPathPrefix);
    cm_sim_set_topo_cfg((char *)pszCfgFile);

    return 0;
}
/*SDK-Modify <<*/

RemoteSess_t *allocRemoteSess()
{
    int i;
    RemoteSess_t *pSess = &g_RemoteSessArr[0];

    for (i = 0; i< M_REMOTE_MAX_SESS; i++) {
        pSess = &g_RemoteSessArr[i];
        if (!pSess->nUse) {
            pSess->nUse = 1;
            return pSess;
        }
    }
    return NULL;
}

void freeRemoteSess(RemoteSess_t *pSess)
{
    if (pSess->sock > 0) {
        sal_close(pSess->sock);
        pSess->sock = -1;
    }

    if (pSess->sock_unsync > 0) {
        sal_close(pSess->sock_unsync);
        pSess->sock_unsync = -1;
    }

    pSess->nUse = 0;
    return;
}

RemoteSess_t * getRemoteSess(int lchip)
{
    int i;
    RemoteSess_t *pSess = &g_RemoteSessArr[0];

    for (i = 0; i< M_REMOTE_MAX_SESS; i++) {
        pSess = &g_RemoteSessArr[i];
        if (pSess->nUse && lchip == pSess->lchip) {
            return pSess;
        }
    }

    return NULL;
}
/*temply*/
struct chip_agent_pkt_s
{
    uint8 chan_id;
    uint8 chip_id;
    void* pkt;
    uint32 packet_length;

};
typedef struct chip_agent_pkt_s chip_agent_pkt_t;

//static int g_bExit = 0;
extern int32 chip_agent_fifo_push(uint8 lchip, uint8 fifo_type, void* p_info);
extern int32 chip_agent_dma_tx_pkt(chip_agent_pkt_t* p_pkt);
int32 cm_sim_dma_fifo_reply(uint8 lchip, uint32 value)
{
    int sock = g_RemoteSessArr[lchip].sock_server_unsync;
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    RemoteDMA_Reply_t *reply = (RemoteDMA_Reply_t *)phdr->data;
    int len = M_REMOTE_MSG_HDR_LEN + sizeof(RemoteDMA_Reply_t);
    int n_len;
    int ret = -1;

    //remote_access_info("\n\ncm_sim_dma_fifo_reply: RRRRRRRRRRRRRRFFFFFFFFFFFFFFF");

    phdr->nType = htons(M_REMOTE_MSG_DMA_CLIENT_REPLY);
    phdr->nLen = htons(len);

    reply->result = value;

    n_len = writen(sock, buf, len);
    if(n_len != len)
    {
        goto err_out;
    }
    ret = 0;

err_out:
    return ret;
}
//extern int32 drv_model_dma_fifo_is_not_full(uint8 chip_id_offset, uint8 fifo_type);
extern int32 chip_agent_dma_fifo_is_not_full(uint8 chip_id, uint8 fifo_type);
STATIC void
read_msg_from_cmodel_thread(void* param)
{
    RemoteSess_t *pSess = (RemoteSess_t *)param;
    struct sal_pollfd poll_list;
    int ret;
    uint8 fifo_type;
    char buf[M_REMOTE_MSG_MAX_LEN];
    chip_agent_pkt_t* p_pkt;
    uint32 fifo_full = 0;
    #if gonghd
    int i=0;
    #endif

    pSess->g_bExit = 0;
    poll_list.fd = pSess->sock_server_unsync;
    poll_list.events = POLLIN | POLLERR | POLLHUP;

    remote_access_info("read_msg_from_cmodel_thread ready to service. fd %d, lchip %d",
        pSess->sock_server_unsync, pSess->lchip);
    while (!pSess->g_bExit) {
        ret = sal_poll(&poll_list, 1, 1000); /* 1000ms */
        if (ret) {
            if (POLLIN == (poll_list.revents & POLLIN))
            {

                fifo_type = CHIP_AGENT_FIFO_TYPE_MAX;
                //remote_access_info("\n\nread_msg_from_cmodel_thread: SSSSSSSS fifo_type = %d SSSSSSS\n\n", fifo_type);
                sal_memset((uint8 *)buf, 0x00, M_REMOTE_MSG_MAX_LEN);

                cm_sim_dma_fifo(pSess->lchip, &fifo_type, buf);

                //remote_access_err("read_msg_from_cmodel_thread: fifo type = %d!", fifo_type);
                //remote_access_info("\n\nread_msg_from_cmodel_thread: SSSSSSSS fifo_type = %d SSSSSSS\n\n", fifo_type);
                if(fifo_type == CHIP_AGENT_FIFO_TYPE_MAX)
                {
                    remote_access_err("read_msg_from_cmodel_thread: unknown fifo type !");
                    continue;
                }
                #if gonghd
                remote_access_info("\n\nSSSSSSSDATA= ");
                for(i = 0; i < 92; i++)
                {

                    remote_access_info("%02x",*(buf + i));

                }
                remote_access_info(" SSSSSSS\n\n");
                #endif
                if(fifo_type < CHIP_AGENT_FIFO_TYPE_MAX)
                {//dma receive fifo push from cmodel
                    chip_agent_fifo_push(pSess->lchip, fifo_type, buf);
                    cm_sim_dma_fifo_reply(pSess->lchip, 0);
                }
                else if(fifo_type == 10)
                {//dma receive pkt from cmodel
                    p_pkt = sal_malloc(sizeof(chip_agent_pkt_t));
                    sal_memset((uint8 *)p_pkt, 0x00,sizeof(chip_agent_pkt_t));
                    p_pkt = (chip_agent_pkt_t*)buf;
                    p_pkt->pkt = buf + sizeof(chip_agent_pkt_t);
                    ret = chip_agent_dma_tx_pkt(p_pkt);
                    if(ret)
                    {
                        sal_free(p_pkt);
                    }
                    cm_sim_dma_fifo_reply(pSess->lchip, 0);
                }
                else if(fifo_type == 20)
                {
                    fifo_full = (chip_agent_dma_fifo_is_not_full(pSess->lchip, 1) == 0);
                    //remote_access_info("\n\nread_msg_from_cmodel_thread: SSSSSSSS fifo_full = %d SSSSSSS\n\n", fifo_full);
                    cm_sim_dma_fifo_reply(pSess->lchip, fifo_full);
                }
                #if gonghd
                remote_access_info("\n\nread_msg_from_cmodel_thread: SSSSSSSS fifo_type = %d SSSSSSS\n\n", fifo_type);
                remote_access_info("please add read system call........");
                nret = read(pSess->sock_server_unsync, buf, 256);
                buf[nret] = '\0';
                printf("msg: %s\r\n", buf + sizeof(unsigned short) * 3);
                #endif
            }
            if(POLLERR == (poll_list.revents & POLLERR))
            {
                remote_access_info("Remote session closed!!!");
                pSess->g_bExit = 1;
                close(pSess->sock_server_unsync);
                break;
            }
        }
    }
    //return ;
}

int cm_send_register_message(int fd, char *pszProcName, char cType)
{
    char szBuf[256];
    RemoteMsgHdr_t *pHdr = (RemoteMsgHdr_t *)szBuf;
    int len = M_REMOTE_MSG_HDR_LEN + strlen(pszProcName) + sizeof(char);
    int ret;

    pHdr->nType = htons(M_REMOTE_MSG_REGISTER);
    pHdr->nLen = htons(len);
    pHdr->data[0] = cType;
    snprintf(pHdr->data + 1, 256 - M_REMOTE_MSG_HDR_LEN, "%s", pszProcName);

    ret = write(fd, szBuf, len);
    if (ret != len) {
        remote_access_err("Close connection for send register message failed: %s",
                strerror(errno));
        return -1;
    }

    return 0;
}

int cm_recv_message(int sock, int nCurReqId,
    char *pmsg, int msglen, int *pnReplySize)
{
    char szBuf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *pHdr = (RemoteMsgHdr_t *)szBuf;
    int len;
    int ret = -1;

    len = read(sock, szBuf, M_REMOTE_MSG_MAX_LEN);
    if (len < M_REMOTE_MSG_HDR_LEN) {
        remote_access_err("Recv remote message failed: %s, ret = %d, len = %d",
                strerror(errno), ret, len);
        goto err_out;
    }
#if 0
    if (nCurReqId != sal_ntohl(pHdr->nReqId)) {
        remote_access_err("Remote message unordered: request id %d, reply id %d",
                nCurReqId, pHdr->nReqId);
        goto err_out;
    }
#endif
    if (len != sal_ntohs(pHdr->nLen)) {
        remote_access_err("Invalid remote message: msg len %d, read len %d",
                msglen, len);
        goto err_out;
    }

    if (msglen < (len - M_REMOTE_MSG_HDR_LEN)) {
        remote_access_err("cm_recv_message: buffer overflow, buffer size %d, msg size %d",
                msglen, (len - M_REMOTE_MSG_HDR_LEN));
        goto err_out;

    }

    sal_memcpy(pmsg, pHdr->data, (len - M_REMOTE_MSG_HDR_LEN));
    *pnReplySize = (len - M_REMOTE_MSG_HDR_LEN);
    ret = 0;
err_out:
    return ret;
}

int cm_send_message(int sock, unsigned short usMsgType,
    char *pmsg, int msglen, int nCurReqId)
{
    char szBuf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *pHdr = (RemoteMsgHdr_t *)szBuf;
    int len = M_REMOTE_MSG_HDR_LEN + msglen + sizeof(unsigned short);
    int ret = -1;
    int nBytes;

    if (len > M_REMOTE_MSG_MAX_LEN) {
        remote_access_err("Send remote message failed: message too large %d",
                len);
        goto err_out;
    }

    pHdr->nReqId = sal_htonl(nCurReqId);
    pHdr->nType = htons(usMsgType);
    pHdr->nLen = htons(len);

    sal_memcpy(pHdr->data, pmsg, msglen);
    //gonghd add
    //remote_access_info("cm_send_message pHdr->data value = %u \n", pHdr->data);

    nBytes = writen(sock, szBuf, len);
    if (nBytes != len) {
        remote_access_err("Send remote message failed: %s, ret = %d, len = %d",
                strerror(errno), ret, len);
        goto err_out;
    }

    ret = 0;
err_out:
    return ret;
}


int connect_remote_cmodel(RemoteSess_t *pSess, char *pszDutName, int bSync)
{
    int sock;
    struct sockaddr_un addr;
    int len;
    char szPath[PATH_MAX];
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    sal_task_t *pTask = NULL;

    if (bSync == 1) {
        if (pSess->sock > 0) {
            remote_access_err("connect_remote_cmodel: internal error, main sock already opened");
            return -1;
        }
    } else if(bSync == 2)
    {
        if (pSess->sock_server_unsync > 0) {
            remote_access_err("connect_remote_cmodel: internal error, unsync server sock already opened");
            return -1;
        }
    } else {
        if (pSess->sock_unsync > 0) {
            remote_access_err("connect_remote_cmodel: internal error, unsync sock already opened");
            return -1;
        }
    }

    sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0 ) {
        remote_access_err("connect_remote_cmodel: Create socket failed: %s",
            sal_strerror(errno));
        return -1;
    }

    sal_snprintf(szPath, PATH_MAX, "%s-%s-RMTACC",
            cm_sim_get_sock_path_prefix(), pszDutName);
    memset(&addr, 0x00, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    remote_access_info("try connect to %s(%s)",
        szPath, (bSync == 1) ? "Main" : ((bSync == 0) ? "Interrupt" : "Interrupt Client"));
    do {
        if (connect (sock, (struct sockaddr *)&addr, len) == 0) {
            /* connection established */
            break;
        }
        remote_access_err("connect_remote_cmodel: Connect to %s failed: %s",
                pszDutName, sal_strerror(errno));
        sleep(1);
    } while (1);

    if (cm_send_register_message(sock, cm_sim_get_sdk_name(),
            (bSync == 1) ? 'M' : ((bSync == 0) ? 'I' : 'S')) != 0) {
        close(sock);
        return -1;
    }
    /* read reigster reply from server */
    len = read(sock, buf, M_REMOTE_MSG_MAX_LEN);
    if (len <= 0) {
        remote_access_err("Connection closed!");
        close(sock);
        return -1;
    }
    phdr->nType = ntohs(phdr->nType);
    if (M_REMOTE_MSG_REGISTER_REPLY != phdr->nType) {
        remote_access_err("Invalid reply message!");
        close(sock);
        return -1;
    }
    unsigned short *pus = (unsigned short *)phdr->data;
    if (0 == *pus) {
        phdr->nLen = ntohs(phdr->nLen);
        buf[phdr->nLen] = '\0';
        remote_access_err("%s", phdr->data + sizeof(unsigned short));
        close(sock);
        return -1;
    }

    if (bSync == 1) {
        pSess->sock = sock;
    } else if(bSync == 0)
    {
        pSess->sock_unsync = sock;
    } else {
        sal_task_create(&pTask,
                        pszDutName,
                        SAL_DEF_TASK_STACK_SIZE,
                        SAL_TASK_PRIO_DEF,
                        read_msg_from_cmodel_thread,
                        (void*)pSess);
        pSess->sock_server_unsync = sock;
    }

    remote_access_info("Connect to %s(%s) success, fd %d",
        szPath, (bSync == 1) ? "Main" : ((bSync == 0) ? "Interrupt" : "Interrupt Client"), sock);
    pSess->pArg = (void *)pTask;
    set_sock_recvbuf(sock, M_REMOTE_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_REMOTE_SOCK_BUF_SZ);
    return 0;
}

extern dal_op_t g_dal_op;

int procRemoteAccessCfgLine(char **ppTokArr)
{
    char *pszSdkName = ppTokArr[0];
    char *pszDutName = ppTokArr[1];
    int nChipId = sal_atoi(ppTokArr[2]);
    char *pszName = cm_sim_get_sdk_name();
    RemoteSess_t *pSess = NULL;

    if (sal_strcasecmp(pszSdkName, pszName)) {
        return -1;
    }

    remote_access_info("Process remote access configuration line: %s %s %d",
        pszName, pszDutName, nChipId);
    if ((pSess = allocRemoteSess()) == NULL) {
        remote_access_info("Process remote access configuration line: %s %s %d failed: too many sessions",
            pszName, pszDutName, nChipId);
        return -1;
    }
    // gonghd add
    if (ctc_sal_mutex_create(&pSess->g_pMsgLock) != 0) {
        sal_printf("procRemoteAccessCfgLine create message mutex failed\n");
        return -1;
    }
    if (ctc_sal_mutex_create(&pSess->g_pNotifyLock) != 0) {
        sal_printf("procRemoteAccessCfgLine create dma mutex failed\n");
        return -1;
    }
    if (ctc_sal_mutex_create(&pSess->g_pNotifyServerLock) != 0) {
        sal_printf("procRemoteAccessCfgLine create dma server mutex failed\n");
        return -1;
    }

    /* 3 socket for each cmodel */
    if (connect_remote_cmodel(pSess, pszDutName, 1)
            || connect_remote_cmodel(pSess, pszDutName, 0) || connect_remote_cmodel(pSess, pszDutName, 2)) {
        freeRemoteSess(pSess);
        return -1;
    }

    extern void cm_sim_register_remote_dal_op(dal_op_t *pop);
    cm_sim_register_remote_dal_op(&g_dal_op);

    pSess->lchip = nChipId;
    sal_printf("\n\n procRemoteAccessCfgLine chipid = %d\n", nChipId);

    chip_agent_dma_init(nChipId);
    return 0;
}

int cm_sim_remote_read_cfg(char *pszFilename)
{
    sal_file_t fp = NULL;
    char szLine[256];
    char *p = NULL;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int ret = -1;

    fp = sal_fopen(pszFilename, "r");
    if (NULL == fp) {
        remote_access_err("cm_sim_remote_read_cfg: open file %s failed: %s",
            pszFilename, sal_strerror(errno));
        goto err_out;
    }

    while (fgets(szLine, 256, fp) != NULL) {
        /* delete whitespace */
        p = szLine;
        while ('\0' != *p && isspace ((int) *p)) {
            p++;
        }

        if ('\r' == *p) {
            p++;
        }
        if ('\n' == *p) {
            p++;
        }

        /* skip empty or comment lines */
        if ('\0' == *p || '#' == *p) {
            continue;
        }

        /* # Format: SDKxx DUTyy CHIP-ID  */
        if (split_tokens(p, strlen(p), 5, " \t\r\n", &nToks, &ppTokArr) != 0
                || (3 != nToks)) {
            if ((5 == nToks) && (NULL != strstr(ppTokArr[0], "CTP")
                                  || NULL != strstr(ppTokArr[0], "DUT"))) {
                /* ignore networking configuration line */
                free_tokens(&ppTokArr);
                continue;
            }

            remote_access_err("Invalid remote access configuration line: '%s'", p);
            free_tokens(&ppTokArr);
            continue;
        }

        /* check chip id */
        if (sal_strspn(ppTokArr[2], "0123456789") != strlen(ppTokArr[2])
                || sal_atoi(ppTokArr[2]) >= M_REMOTE_MAX_SESS) {
            remote_access_err("Invalid remote access configuration line: '%s'", p);
            free_tokens(&ppTokArr);
            continue;
        }

        procRemoteAccessCfgLine(ppTokArr);
        free_tokens(&ppTokArr);
    }

    ret = 0;

err_out:
    if (NULL != fp) {
        sal_fclose(fp);
    }
    return ret;
}

int cm_sim_get_socket(uint8 lchip, int bSync)
{
    RemoteSess_t *pSess = NULL;
    int nSock = -1;

    // gonghd ctc_sal_mutex_lock(g_pMsgLock);
    if (NULL != (pSess = getRemoteSess(lchip))) {
        nSock = bSync ? pSess->sock : pSess->sock_unsync;
    }
    // gonghd ctc_sal_mutex_unlock(g_pMsgLock);
    return nSock;
}

static u_int32_t g_RemoteReqId = 0;

void close_remote_access_session(ProcItem_t *pProc)
{
    afx_mio_t *pMio = (afx_mio_t *)pProc->pArg;

    if (pProc->sock > 0) {
        close(pProc->sock);
        pProc->sock = -1;
        pProc->connected = 0;
    }

    if (NULL != pMio) {
        afx_mio_destroy(pMio);
        pProc->pArg = NULL;
    }

    remote_access_info("Connection for remote access client %s closed (%s)",
            pProc->szName, ('M' == pProc->cType) ? "Main channel" : "Interrupt channel");
    return;
}

void read_msg_from_remote_access_client(int fd, afx_io_dir_t dir, void *arg)
{
    int nbytes,nbytes1=0;
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    ProcItem_t *pProc = (ProcItem_t *)arg;
    int onepktlen=0;

    for (;;) {
        while (nbytes1 != M_REMOTE_MSG_HDR_LEN) {
            nbytes = recv(fd, buf, M_REMOTE_MSG_HDR_LEN - nbytes1, MSG_PEEK);
            nbytes1 = nbytes1 + nbytes;
            if (nbytes == 0) {
                close_remote_access_session(pProc);
                return;
            }
        }
        onepktlen = ntohs(phdr->nLen);
        nbytes = readn(fd, buf, onepktlen);
        if (nbytes != onepktlen) {
            remote_access_err("pktclnt remote access receive pkts from client failed! explen = %d , reallen=%d",
                    onepktlen, nbytes);
            close_remote_access_session(pProc);
            return ;
        }

        break;
    }

    phdr->nType = ntohs(phdr->nType);
    phdr->nLen = ntohs(phdr->nLen) - M_REMOTE_MSG_HDR_LEN - sizeof(unsigned short);
    remote_access_info("Message from remote access client %s, message type %d",
            pProc->szName, phdr->nType);
    if (g_RemoteAccessCb.cb) {
        g_RemoteAccessCb.cb(phdr->nType, phdr->data, phdr->nLen, g_RemoteAccessCb.parg);
    }
    return;
}

int send_remote_access_register_reply(int sock, int bSuccess, char *pszMsg)
{
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    int len = strlen(pszMsg) + M_REMOTE_MSG_HDR_LEN + sizeof(unsigned short);
    unsigned short *pusResult = (unsigned short *)phdr->data;

    phdr->nType = htons(M_REMOTE_MSG_REGISTER_REPLY);
    phdr->nLen = htons(len);
    *pusResult = bSuccess;
    snprintf(phdr->data + sizeof(unsigned short),
            M_REMOTE_MSG_MAX_LEN - M_REMOTE_MSG_HDR_LEN - sizeof(unsigned short),
            "%s", pszMsg);
    writen(sock, buf, len);
    if (!bSuccess) {
        remote_access_err("%s", pszMsg);
        close(sock);
    }
    return 0;
}


void accept_remote_access_client(int fd, afx_io_dir_t dir, void *arg)
{
    struct sockaddr_un addr;
    socklen_t len = sizeof(struct sockaddr_un);
    int sock;
    RemoteMsgHdr_t *pmsg;
    char szBuf[256];
    int nbytes;
    char szName[M_PROCESS_NAME_LEN];
    char szErrMsg[256];
    ProcItem_t *pItem = NULL;
    char cType = 'M'; /* Main channel */

    sock = accept(fd, (struct sockaddr *)&addr, &len);
    if (-1 == sock) {
        remote_access_err("Accept remote access client failed: %s", strerror(errno));
        return;
    }

    remote_access_info("Accpet remote access connection sock %d", sock);

    snprintf(szErrMsg, 256, "%s", "internal error");
    /* read register message */
    do {
        nbytes = read(sock, szBuf, 256);
    } while (-1 == nbytes && EINTR == errno);
    if (-1 == nbytes) {
        snprintf(szErrMsg, 256, "Read remote access register message failed: %s",
                strerror(errno));
        goto err_out;
    }

    pmsg = (RemoteMsgHdr_t *)&szBuf;
    if (M_REMOTE_MSG_REGISTER != ntohs(pmsg->nType)) {
        snprintf(szErrMsg, 256, "Invalid remote access register message.");
        goto err_out;
    }

    cType = pmsg->data[0];
    if ('M' == cType) {
        /* Main channel */
    } else if ('I' == cType) {
        /* channel for interrupt and stats */
    } else {
        snprintf(szErrMsg, 256, "Remote access register message unknown, type '%c'!",
            cType);
        goto err_out;
    }

    int plen = ntohs(pmsg->nLen) - M_REMOTE_MSG_HDR_LEN - 1;
    if (plen >= M_PROCESS_NAME_LEN) {
        snprintf(szErrMsg, 256, "SDK name too long.");
        goto err_out;
    }
    memcpy(szName, pmsg->data + 1, plen);
    szName[plen] = '\0';

    pItem = calloc(sizeof(ProcItem_t), 1);
    if (NULL == pItem) {
        snprintf(szErrMsg, 256, "WARNING: Out of memory!");
        goto err_out;
    }

    snprintf(pItem->szName, M_PROCESS_NAME_LEN, "%s", szName);

    afx_mio_t *pMio = NULL;
    if (afx_mio_create(&pMio, sock, AFX_IO_IN,
                read_msg_from_remote_access_client, pItem) != 0) {
        snprintf(szErrMsg, 256, "WARNING: Out of memory!");
        goto err_out;
    }

    pItem->pArg = pMio;
    pItem->sock = sock;
    pItem->connected = 1;
    pItem->cType = cType;
    set_sock_nonblocking(sock);
    set_sock_recvbuf(sock, M_REMOTE_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_REMOTE_SOCK_BUF_SZ);
    send_remote_access_register_reply(sock, 1, "success");
    if ('I' == cType) {
        g_pInterruptProc = pItem;
    } else {
        g_pIOProc = pItem;
    }
    remote_access_info("Accept remote access connection %s success(channel %s)",
        szName, ('M' ==  cType) ? "Main" : "Interrupt");
    return;

err_out:
    if (sock > 0) {
        send_remote_access_register_reply(sock, 0, szErrMsg);
    }
    if (NULL != pItem) {
        free(pItem);
    }
    return;
}

int start_remote_access_agent(void)
{
    struct sockaddr_un addr;
    char szPath[PATH_MAX];
    int len;

    if (-1 != g_nRemoteAccSock) {
        return -1;
    }

    g_nRemoteAccSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == g_nRemoteAccSock) {
        remote_access_err("Create remote access socket failed: %s", strerror(errno));
        return -1;
    }

    snprintf(szPath, PATH_MAX, "%s-%s-RMTACC",
            get_sock_path_prefix(), get_dut_name());
    unlink(szPath);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    if (bind(g_nRemoteAccSock, (struct sockaddr *)&addr, len) < 0) {
        remote_access_err("Bind remote access socket failed: %s", strerror(errno));
        goto err_out;
    }

    listen(g_nRemoteAccSock, 3);

    if (afx_mio_create(&g_pRemoteAccMio, g_nRemoteAccSock, AFX_IO_IN,
                accept_remote_access_client, g_pRemoteAccMio) != 0) {
        remote_access_err("Create remote access MIO failed: Out of memory");
        goto err_out;
    }

    remote_access_info("Start remote access socket success");
    M_MESSAGE_LOCK_INIT;
    M_NOTIFY_LOCK_INIT;
    return 0;

err_out:
    if (-1 != g_nRemoteAccSock) {
        close(g_nRemoteAccSock);
        g_nRemoteAccSock = -1;
    }
    return -1;
}

int start_sdk_remote_access_agent(uint8 lchip)
{
    struct sockaddr_un addr;
    char szPath[PATH_MAX];
    int len;

    if (-1 != g_nRemoteAccSock) {
        return -1;
    }

    g_nRemoteAccSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == g_nRemoteAccSock) {
        remote_access_err("Create remote access socket failed: %s", strerror(errno));
        return -1;
    }

    snprintf(szPath, PATH_MAX, "%s-%s-RMTACC",
            get_sock_path_prefix(), get_dut_name());
    unlink(szPath);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    if (bind(g_nRemoteAccSock, (struct sockaddr *)&addr, len) < 0) {
        remote_access_err("Bind remote access socket failed: %s", strerror(errno));
        goto err_out;
    }

    listen(g_nRemoteAccSock, 3);

    if (afx_mio_create(&g_pRemoteAccMio, g_nRemoteAccSock, AFX_IO_IN,
                accept_remote_access_client, g_pRemoteAccMio) != 0) {
        remote_access_err("Create remote access MIO failed: Out of memory");
        goto err_out;
    }

    remote_access_info("Start remote access socket success");
    M_MESSAGE_LOCK_INIT;
    M_NOTIFY_LOCK_INIT;
    return 0;

err_out:
    if (-1 != g_nRemoteAccSock) {
        close(g_nRemoteAccSock);
        g_nRemoteAccSock = -1;
    }
    return -1;
}

int send_notify_to_sdk(char *pmsg, int bufsz, u_int16_t type)
{
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    u_int16_t *pnType;
    int len;
    int nRet;
    int fRet = -1;

    M_NOTIFY_LOCK;
    if (NULL == g_pInterruptProc) {
        remote_access_err("Send notify to SDK failed: SDK not connected");
        goto err_out;
    }

#if 0
    if (SDK_NOTIFY_TYPE_INTERRUPT != type &&
        SDK_NOTIFY_TYPE_STATS != type) {
        remote_access_err("Send notify to %s failed: unknown message type %d",
            g_pInterruptProc->szName, type);
        return -1;
    }
#endif

    len = M_REMOTE_MSG_HDR_LEN + sizeof(u_int16_t) + bufsz;
    if (len > M_REMOTE_MSG_MAX_LEN) {
        remote_access_err("Send notify to %s failed: message too large %d",
            g_pInterruptProc->szName, len);
        goto err_out;
    }

    pnType = (u_int16_t *)phdr->data;
    phdr->nType = htons(M_MSG_NOTIFY);
    phdr->nReqId = htonl(++g_RemoteReqId);
    phdr->nLen = htons(len);
    *pnType = htons(type);
    memcpy(phdr->data + sizeof(u_int16_t), pmsg, bufsz);
    nRet = writen(g_pInterruptProc->sock, buf, len);
    if (nRet != len) {
        remote_access_err("Send notify to %s failed: size %d, write %d",
            g_pInterruptProc->szName, len, nRet);
        goto err_out;
    }

    remote_access_info("Send notify to %s success: size %d",
        g_pInterruptProc->szName, nRet);
    fRet = 0;

err_out:
    M_NOTIFY_UNLOCK;
    return fRet;
}

int send_message_to_sdk(char *pmsg, int bufsz, u_int16_t type)
{
    char buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *phdr = (RemoteMsgHdr_t *)buf;
    int len;
    int nRet;
    int fRet = -1;

    M_MESSAGE_LOCK;
    if (NULL == g_pIOProc) {
        remote_access_err("Send message to SDK failed: SDK not connected");
        goto err_out;
    }

    len = M_REMOTE_MSG_HDR_LEN + bufsz;
    if (len > M_REMOTE_MSG_MAX_LEN) {
        remote_access_err("Send message to %s failed: message too large %d",
            g_pIOProc->szName, len);
        goto err_out;
    }

    phdr->nType = htons(type);
    phdr->nLen = htons(len);
    phdr->nReqId = htonl(++g_RemoteReqId); /* XXX */
    memcpy(phdr->data, pmsg, bufsz);
    nRet = writen(g_pIOProc->sock, buf, len);
    if (nRet != len) {
        remote_access_err("Send message to %s failed: size %d, write %d",
            g_pIOProc->szName, len, nRet);
        goto err_out;
    }

    remote_access_info("Send message to %s success: size %d",
        g_pIOProc->szName, nRet);
    fRet = 0;

err_out:
    M_MESSAGE_UNLOCK;
    return fRet;
}

extern int32
drv_ioctl_api(uint8 chip_id, int32 index, uint32 cmd, void* val);
int dummy_remote_access_cb(unsigned short nType, char *buf, size_t len, void *arg)
{
    RemoteMsgHdr_t *pMsgHdr = (RemoteMsgHdr_t *)buf;
    RemoteIO_Req_t *pIOReq = NULL;
    RemoteIO_Reply_t stReply;
    int ret;

    remote_access_info("dummy_remote_access_cb type %d, size %d", nType, len);
    sal_memset(&stReply, 0x00, sizeof(RemoteIO_Reply_t));
    if (M_REMOTE_MSG_IO == nType) {
        pIOReq = (RemoteIO_Req_t *)pMsgHdr->data;
        //ret = DRV_E_NO_MEMORY;gonghd,cmodel加函数处理
        ret = DRV_E_NONE;
        stReply.result = ret;
        if (DRV_E_NONE == ret) {
            stReply.value = pIOReq->value;
        }

        remote_access_info("  %s chip %d offset %u value %u",
            pIOReq->rw ? "write" : "read", pIOReq->lchip,
            pIOReq->offset, pIOReq->value);
    } else {
        stReply.result = DRV_E_NO_MEMORY; /* XXX */
    }

    send_message_to_sdk((char *)&stReply, sizeof(RemoteIO_Reply_t), nType);
    return 0;
}

void register_remote_access_callback(remote_access_cb_t cb, void *parg)
{
    g_RemoteAccessCb.cb = cb;
    g_RemoteAccessCb.parg = parg;
}
uint32 static_offset = 0;
int32 cm_sim_remote_pci_read(uint8 lchip, uint32 offset, uint32* value)
{

    RemoteIO_Req_t rio;
    RemoteIO_Reply_t reply;
    int nCurReqId;
    int nReplySize = 0;

    if(offset == 0)
    {
        sal_memset(&reply, 0x00, sizeof(RemoteIO_Reply_t));
        *value = (1 << 16);
        return DRV_E_NONE;
    }
    if(offset >= 8)
    {
        offset = static_offset + offset - 8;
    }

    //sal_printf("\n\ncm_sim_remote_pci_write: RRRRRRRRRRRRRRRRRRRR\n\n");


    nCurReqId = ++g_nReqid;
    sal_memset(&rio, 0x00, sizeof(RemoteIO_Req_t));
    rio.offset = offset;
    rio.lchip = lchip;
    sal_memset(&reply, 0x00, sizeof(RemoteIO_Reply_t));
    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pMsgLock);
    int sock = cm_sim_get_socket(lchip, 1);
    if (-1 == sock) {
        sal_printf("\n\ncm_sim_remote_pci_read: remote socket not opened ERROR.\n\n");
        return DRV_E_TIME_OUT;
    }
    if (cm_send_message(sock, M_REMOTE_MSG_IO,
            (char *)&rio, sizeof(RemoteIO_Req_t), nCurReqId) != 0) {
        /* fake timeout */
        return DRV_E_TIME_OUT;
    }
    if (cm_recv_message(sock, nCurReqId,
            (char *)&reply, sizeof(RemoteIO_Reply_t), &nReplySize) != 0
            || nReplySize != sizeof(RemoteIO_Reply_t)) {
        /* fake timeout */
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pMsgLock);

    if (reply.result != DRV_E_NONE) {
        return reply.result;
    }

    *value = reply.value;
    return DRV_E_NONE;
}

int32 cm_sim_remote_pci_write(uint8 lchip, uint32 offset, uint32 value)
{

    RemoteIO_Req_t rio;
    RemoteIO_Reply_t reply;
    int nCurReqId;
    int nReplySize = 0;
    //static uint32 static_offset = 0;

    if(offset == 0 || offset == 4)
    {
        if(offset == 0)
        {
            static_offset = 0;
        }

        if(offset == 4)
        {
            static_offset = value;
        }
        sal_memset(&reply, 0x00, sizeof(RemoteIO_Reply_t));
        return reply.result;
    }

    if(offset >= 8)
    {
        offset = static_offset + offset - 8;
    }
    //sal_printf("\n\ncm_sim_remote_pci_write: WWWWWWWWWWWWWWWWWWW\n\n");

    nCurReqId = ++g_nReqid;
    sal_memset(&rio, 0x00, sizeof(RemoteIO_Req_t));
    rio.offset = offset;
    rio.lchip = lchip;
    rio.rw = 1; /* write */
    rio.value = value;
    sal_memset(&reply, 0x00, sizeof(RemoteIO_Reply_t));

    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pMsgLock);
    int sock = cm_sim_get_socket(lchip, 1);
    if (-1 == sock) {
        sal_printf("\n\nm_sim_remote_pci_write: remote socket not opened ERROR.\n\n");
        return DRV_E_TIME_OUT;
    }


    if (cm_send_message(sock, M_REMOTE_MSG_IO,
    (char *)&rio, sizeof(RemoteIO_Req_t), nCurReqId) != 0) {
    /* fake timeout */
        return DRV_E_TIME_OUT;
    }
    if (cm_recv_message(sock, nCurReqId,
            (char *)&reply, sizeof(RemoteIO_Reply_t), &nReplySize) != 0
            || nReplySize != sizeof(RemoteIO_Reply_t)) {
        /* fake timeout */
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pMsgLock);


    return reply.result;
}


void cm_sim_register_remote_dal_op(dal_op_t *pop)
{
    pop->pci_read = cm_sim_remote_pci_read;
    pop->pci_write = cm_sim_remote_pci_write;
    return;
}



//gonghd dma >>>>
int32 cm_sim_dma_fifo(uint8 lchip, uint8* fifo_type, uint8* buf)
{
    char tmp_buf[M_REMOTE_MSG_MAX_LEN];
    RemoteMsgHdr_t *pHdr = (RemoteMsgHdr_t *)tmp_buf;
    RemoteDMA_Req_t *pDMAReq = NULL;
    int len = 0;
    int ret = -1;
    chip_agent_pkt_t *p_pkt;

    len = read(g_RemoteSessArr[lchip].sock_server_unsync, tmp_buf, M_REMOTE_MSG_MAX_LEN);
    if (len < M_REMOTE_MSG_HDR_LEN) {
        remote_access_err("Recv remote message failed: %s, ret = %d, len = %d",
                strerror(errno), ret, len);
        goto err_out;
    }
    if (len != sal_ntohs(pHdr->nLen)) {
        remote_access_err("Invalid remote message: msg len %d, read len %d",
                sal_ntohs(pHdr->nLen), len);
        goto err_out;
    }

    if (sal_ntohs(pHdr->nLen) < (len - M_REMOTE_MSG_HDR_LEN)) {
        remote_access_err("cm_recv_message: buffer overflow, buffer size %d, msg size %d",
                sal_ntohs(pHdr->nLen), (len - M_REMOTE_MSG_HDR_LEN));
        goto err_out;

    }

    pDMAReq = (RemoteDMA_Req_t *)pHdr->data;

    *fifo_type = pDMAReq->type;
    if(pDMAReq->type < 10)
    {
        sal_memcpy(buf, (uint8* )pHdr->data + sizeof(RemoteDMA_Req_t), (len - M_REMOTE_MSG_HDR_LEN - sizeof(RemoteDMA_Req_t)));
    }
    else if(pDMAReq->type == 10)
    {
        p_pkt = sal_malloc(sizeof(chip_agent_pkt_t));
        sal_memset((uint8 *)p_pkt, 0x00,sizeof(chip_agent_pkt_t));
        p_pkt->chan_id = pDMAReq->index;
        p_pkt->chip_id = lchip;
        p_pkt->packet_length = len - M_REMOTE_MSG_HDR_LEN - sizeof(RemoteDMA_Req_t);
        sal_memcpy(buf, (uint8* )p_pkt, sizeof(chip_agent_pkt_t));
        sal_memcpy(buf + sizeof(chip_agent_pkt_t), (uint8* )pHdr->data + sizeof(RemoteDMA_Req_t), (len - M_REMOTE_MSG_HDR_LEN - sizeof(RemoteDMA_Req_t)));
        sal_free(p_pkt);
    }

    ret = 0;

err_out:
    return ret;
}


int32 cm_sim_send_pkt2dma(uint8 lchip, uint8* packet, uint32 packet_len)
{
    int nCurReqId;
    RemoteDMA_Req_t rdma;
	RemoteDMA_Reply_t reply;
    int nReplySize = 0;
    uint8* tmp_buf = NULL;
    sal_memset(&rdma, 0x00, sizeof(RemoteDMA_Req_t));

    //sal_printf("\n\ncm_sim_send_pkt2dma: SSSSSSSSSSSSSSS\n\n");

    rdma.type = 0;
    rdma.len = packet_len;
    nCurReqId = ++g_nReqid_dma;
    tmp_buf = (uint8*)sal_malloc(packet_len + sizeof(RemoteDMA_Req_t));
    sal_memset(tmp_buf, 0x00, packet_len + sizeof(RemoteDMA_Req_t));
    sal_memcpy(tmp_buf, &rdma, sizeof(RemoteDMA_Req_t));
    sal_memcpy(tmp_buf + sizeof(RemoteDMA_Req_t), packet, packet_len);
    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pNotifyLock);
    int sock = cm_sim_get_socket(lchip, 0);
    if (-1 == sock) {
        sal_printf("\n\ncm_sim_remote_pci_read: remote socket not opened ERROR.\n\n");
        return DRV_E_TIME_OUT;
    }

    if (cm_send_message(sock, M_REMOTE_MSG_DMA,
            (char *)tmp_buf, packet_len + sizeof(RemoteDMA_Req_t), nCurReqId) != 0) {
        /* fake timeout */
        sal_free(tmp_buf);
        tmp_buf = NULL;
        ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
    sal_free(tmp_buf);
    tmp_buf = NULL;

    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pNotifyLock);
    sal_memset(&reply, 0x00, sizeof(RemoteDMA_Reply_t));
    if (cm_recv_message(sock, nCurReqId,
            (char *)&reply, sizeof(RemoteDMA_Reply_t), &nReplySize) != 0
            || nReplySize != sizeof(RemoteDMA_Reply_t)) {
        /* fake timeout */
        ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
    if (reply.result != DRV_E_NONE) {
        return reply.result;
    }
    return DRV_E_NONE;
}



int32 cm_sim_reg_memory_dump(uint8 lchip, uint32 table_id, uint32 entry_index, uint32 entry_size, uint8* buf)
{
    int nCurReqId;
    RemoteDMA_Req_t rdma;
	RemoteDMA_Reply_t reply;
    int nReplySize = 0;
    uint8* tmp_buf = NULL;
    sal_memset(&rdma, 0x00, sizeof(RemoteDMA_Req_t));

    sal_printf("\n\ncm_sim_reg_memory_dump: SSSSSSSSSSSSSSS\n\n");

    rdma.type = 1;
    rdma.len = entry_size;
    rdma.tbid = table_id;
    rdma.index = entry_index;

    nCurReqId = ++g_nReqid_dma;

    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pNotifyLock);
    int sock = cm_sim_get_socket(lchip, 0);
    if (-1 == sock) {
        sal_printf("\n\ncm_sim_remote_pci_read: remote socket not opened ERROR.\n\n");
        return DRV_E_TIME_OUT;
    }

    if (cm_send_message(sock, M_REMOTE_MSG_DMA,
            (char *)&rdma, sizeof(RemoteDMA_Req_t), nCurReqId) != 0) {
        /* fake timeout */
        ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);

    sal_memset(&reply, 0x00, sizeof(RemoteDMA_Reply_t));
    tmp_buf = (uint8*)sal_malloc(sizeof(RemoteDMA_Reply_t)+entry_size);
    sal_memset(tmp_buf, 0x00, entry_size + sizeof(RemoteDMA_Reply_t));

    ctc_sal_mutex_lock(g_RemoteSessArr[lchip].g_pNotifyLock);
    if (cm_recv_message(sock, nCurReqId,
            (char *)&reply, sizeof(RemoteDMA_Reply_t)+entry_size, &nReplySize) != 0
            || nReplySize != sizeof(RemoteDMA_Reply_t)) {
        /* fake timeout */
        ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
        return DRV_E_TIME_OUT;
    }
    ctc_sal_mutex_unlock(g_RemoteSessArr[lchip].g_pNotifyLock);
    if (reply.result != DRV_E_NONE) {
        sal_free(tmp_buf);
        tmp_buf = NULL;
        return reply.result;
    }
    sal_memcpy(tmp_buf, &reply, sizeof(RemoteDMA_Reply_t)+entry_size);
    sal_memcpy(buf, tmp_buf + sizeof(RemoteDMA_Reply_t), entry_size);
    sal_free(tmp_buf);
    tmp_buf = NULL;
    return DRV_E_NONE;
}

//gonghd dma <<<<

#ifdef TEST_MAIN
int main(int argc, char **argv)
{
    char *pszSockPathPrefix = argv[1];
    char *pszSdkName = argv[2];
    char *pszCfgFile = argv[3];
    char szBuf[256];

    printf("Sock Path Prefix: %s\r\n", pszSockPathPrefix);
    printf("SDK name: %s\r\n", pszSdkName);

    cm_sim_set_sock_path_prefix(pszSockPathPrefix);
    cm_sim_set_sdk_name(pszSdkName);

    cm_sim_remote_init(pszCfgFile);

    sal_snprintf(szBuf, 256, "Hello main channel!");
    cm_send_message(0, M_MSG_NOTIFY, szBuf, strlen(szBuf), 1);
    sal_snprintf(szBuf, 256, "Hello interrupt channel!");
    cm_send_message(0, M_MSG_NOTIFY, szBuf, strlen(szBuf), 0);
    sleep(4);
    cm_send_message(0, M_MSG_NOTIFY, szBuf, strlen(szBuf), 0);
    sleep(2);
    return 0;
}
#endif

