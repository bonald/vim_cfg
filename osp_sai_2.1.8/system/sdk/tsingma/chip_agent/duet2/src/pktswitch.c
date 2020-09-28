/****************************************************************************
* $Id$
*  Packet Switch Implement
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-10-09 14:55
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
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
#include <stdarg.h>
#include <limits.h>
#include "pktswitch.h"
#include "split.h"
#include "afx_evtlp.h"
#include "sal.h"

/****************************************************************************
 *
* Defines and Macros
*
****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
****************************************************************************/
ProcItem_t *g_pProcs = NULL; /* process array */
int g_nProcsCnt = 0;
unsigned char g_szUnixSockPath[PATH_MAX] = {'\0'}; /* unix socket path */
unsigned char *g_pszTopoCfg = NULL; /* topo configuration file */
int g_nSock = -1; /* socket description */
afx_mio_t *g_pMainMio = NULL; /* main mio for unix socket */

/****************************************************************************
 *
* Function
*
****************************************************************************/

void pktswitch_info(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    fprintf(stdout, "pktswitch: %s\n", print_buf);
    return;
}

void pktswitch_err(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    fprintf(stderr, "pktswitch: %s\n", print_buf);
    return;
}

int locateProcItem(ProcItem_t **pItem, char *pszName, int *pnRecordEmptySlot)
{
    int idx;
    int empty_slot = -1;
    ProcItem_t *pTmp;
    int bFind = 0;

    *pItem = NULL;
    for (idx = 0; idx < g_nProcsCnt; idx++) {
        pTmp = &g_pProcs[idx];

        if ('\0' == pTmp->szName[0]) {
            if (-1 == empty_slot) {
                empty_slot = idx;
            }
            continue;
        }

        if (!strcasecmp(pTmp->szName, pszName)) {
            bFind = 1;
            *pItem = pTmp;
            break;
        }
    }

    if ((!bFind) && (NULL != pnRecordEmptySlot)) {
        *pnRecordEmptySlot = empty_slot; /* maybe it equal -1 */
    }
    return bFind;
}

int recordPortMap(ProcItem_t *pItemSrc, ProcItem_t *pItemDst,
        int nPortSrc, int nPortDst)
{
    int bFind = 0;
    int idx;
    int empty_slot = -1;
    PortMapping_t *pMap;

    if (0 == pItemSrc->nCount) {
        /* new items */
        int alloc_cnt;

        if (nPortSrc <= M_DEFAULT_PORTS_COUNT) {
            alloc_cnt = M_DEFAULT_PORTS_COUNT;
        } else {
            alloc_cnt = M_MAX_PORTS_ID;
        }

        pMap = calloc(sizeof(PortMapping_t), alloc_cnt);
        if (NULL == pMap) {
            pktswitch_err("WARNING: Out of memory!");
            return -1;
        }
        pItemSrc->nCount = alloc_cnt;
        pItemSrc->pPortArr = pMap;
    } else {
        for (idx = 0; idx < pItemSrc->nCount; idx++) {
            pMap = &pItemSrc->pPortArr[idx];
            if (0 == pMap->nPortSrc) {
                if (-1 == empty_slot) {
                    empty_slot = idx;
                    continue;
                }
            }
            if (pMap->pProcess != pItemDst) {
                continue;
            }

            if (pMap->nPortSrc == nPortSrc || pMap->nPortDst == nPortDst) {
                bFind = 1;
                break;
            }
        }

        if (bFind) {
            pktswitch_err("Invalid Topology configuration file, port duplication error.\n  %s/%s:%d/%d %d/%d",
                    pItemSrc->szName, pItemDst->szName,
                    pMap->nPortSrc, pMap->nPortDst,
                    nPortDst, nPortDst);
            return -1;
        }

        if (-1 == empty_slot) {
            assert(M_DEFAULT_PORTS_COUNT == pItemSrc->nCount);
            pMap = realloc(pItemSrc->pPortArr, M_MAX_PORTS_ID * sizeof(PortMapping_t));
            if (NULL == pMap) {
                pktswitch_err("WARNING: Out of memory!");
                return -1;
            }

            pItemSrc->pPortArr = pMap;
            pMap = &pItemSrc->pPortArr[pItemSrc->nCount];
            pItemSrc->nCount = M_MAX_PORTS_ID;
        } else {
            pMap = &pItemSrc->pPortArr[empty_slot];
        }
    }

    pMap->nPortSrc = nPortSrc;
    pMap->nPortDst = nPortDst;
    pMap->pProcess = pItemDst;
    return 0;
}

int setPortMapping(ProcItem_t *pItemSrc, ProcItem_t *pItemDst,
        int nStartSrc, int nStartDst, int nCount)
{
    int idx;

    for (idx = 0; idx < nCount; idx++) {
        recordPortMap(pItemSrc, pItemDst, nStartSrc + idx, nStartDst + idx);
        /* revert it */
        recordPortMap(pItemDst, pItemSrc, nStartDst + idx, nStartSrc + idx);
    }

    return 0;
}

ProcItem_t *recordProcItem(char *pszName)
{
    int bFind = 0;
    int empty_slot = -1;
    ProcItem_t *pItem = NULL;
    ProcItem_t *pTmp;

    bFind = locateProcItem(&pItem, pszName, &empty_slot);
    if (!bFind) {
        if (-1 != empty_slot) {
            pItem = &g_pProcs[empty_slot];
        } else {
            pTmp = realloc(g_pProcs, (g_nProcsCnt + 2) * sizeof(ProcItem_t));
            if (NULL == pTmp) {
                return NULL;
            }

            g_pProcs = pTmp;
            pItem = &g_pProcs[g_nProcsCnt];
            memset(pItem, 0x00, sizeof(ProcItem_t) * 2);
            g_nProcsCnt += 2;
        }
        snprintf(pItem->szName, M_PROCESS_NAME_LEN, "%s", pszName);
    }
    return pItem;
}

int procTopoCfgLine(char **ppTokArr)
{
    ProcItem_t *pItemSrc = NULL;
    ProcItem_t *pItemDst = NULL;

    if (NULL == ppTokArr) {
        return -1;
    }

    pItemSrc = recordProcItem(ppTokArr[0]);
    pItemDst = recordProcItem(ppTokArr[1]);
    if (NULL == pItemSrc || NULL == pItemDst) {
        pktswitch_err("WARNING: Out of memory!");
        return -1;
    }

    return setPortMapping(pItemSrc, pItemDst,
            atoi(ppTokArr[2]), atoi(ppTokArr[3]), atoi(ppTokArr[4]));
}

int loadTopoCfg(unsigned char *pszCfg)
{
    FILE *fp = NULL;
    char szLine[256];
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    char *p;

    fp = fopen(pszCfg, "r");
    if (NULL == fp) {
        pktswitch_err("Open topology configuration file failed: %s",
                strerror(errno));
        return -1;
    }

    while (fgets(szLine, 256, fp) != NULL) {
        /* delete whitespace */
        p = szLine;
        while ('\0' != *p && sal_isspace ((int) *p)) {
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

        /* format:
         *   NAME-A  NAME-B  A-Start-Port-Id  B-Start-Port-Id  Port-Counts
         */
        if (split_tokens(p, strlen(p), 5, " \t\r\n", &nToks, &ppTokArr) != 0
                || (5 != nToks)) {
            if ((3 == nToks) && (NULL != strstr(ppTokArr[0], "CTP"))) {
                /* ignore SDK configuration line */
                free_tokens(&ppTokArr);
                continue;
            }
            pktswitch_err("Invalid topology configuration line: '%s'", p);
            free_tokens(&ppTokArr);
            continue;
        }

        /* check port id and port counts */
        if (strspn(ppTokArr[2], "0123456789") != strlen(ppTokArr[2])
            || strspn(ppTokArr[3], "0123456789") != strlen(ppTokArr[3])
            || strspn(ppTokArr[4], "0123456789") != strlen(ppTokArr[4])) {
            pktswitch_err("Invalid topology configuration line: '%s'", p);
            free_tokens(&ppTokArr);
            continue;
        }
        if (atoi(ppTokArr[2]) > M_MAX_PORTS_ID
            || atoi(ppTokArr[3]) > M_MAX_PORTS_ID
            || atoi(ppTokArr[4]) > M_MAX_PORTS_ID) {
            pktswitch_err("Invalid topology configuration line: '%s   '", p);
            free_tokens(&ppTokArr);
            continue;
        }

        procTopoCfgLine(ppTokArr);
        free_tokens(&ppTokArr);
    }

    fclose(fp);
    return 0;

    if (NULL != fp) {
        fclose(fp);
    }
    return -1;
}

void close_client_session(ProcItem_t *pProc)
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

    pktswitch_info("Connection for client %s closed",
            pProc->szName);
    return;
}

int send_packet_to_client(ProcItem_t *pProc, unsigned short nPortDst, char *buf, size_t len)
{
    char szBuf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *pHdr = (MsgHdr_t *)szBuf;
    int nTotalLen = M_MSG_HDR_LEN + len + sizeof(unsigned short);
    int ret;
    unsigned short *pus;

    if ((len + M_MSG_HDR_LEN + sizeof(unsigned short)) > M_MSG_PKT_MAX_LEN) {
        /* pkt too large */
        return -1;
    }

    pHdr->nType = htons(M_MSG_PKT);
    pHdr->nLen = htons(nTotalLen);
    pus = (unsigned short *)pHdr->data;
    *pus = ntohs(nPortDst);
    memcpy(pHdr->data + sizeof(unsigned short), buf, len);

    ret = writen(pProc->sock, szBuf, nTotalLen);
    if (ret != nTotalLen) {
        /* send packet failed */
        pktswitch_err("Transmit packet to %s port %d length %d failed: %s",
                pProc->szName, nPortDst, len, strerror(errno));
        return -1;
    }
    return 0;
}

int transmit_packet(ProcItem_t *pProc, unsigned short nPortSrc, char *buf, size_t len)
{
    int idx;
    int bFind = 0;
    PortMapping_t *pMap = NULL;
    int i = 0;

    pktswitch_info("Process packet from client %s, length %d from port %d",
            pProc->szName, len, nPortSrc);

    if (0 == pProc->nCount) {
        pktswitch_info("Ignore packet from %s port %d len %d",
                pProc->szName, nPortSrc, len);
        return -1;
    }

    for (idx = 0; idx < pProc->nCount; idx++) {
        pMap = &pProc->pPortArr[idx];

        if (pMap->nPortSrc == nPortSrc) {
            bFind = 1;
            break;
        }
    }

    if (!bFind) {
        pktswitch_info("Ignore packet from %s port %d len %d",
                pProc->szName, nPortSrc, len);
        return -1;
    }

    ProcItem_t *pDst = pMap->pProcess;
    if (pDst->sock <= 0) {
        pktswitch_info("Ignore packet from %s port %d len %d, destination %s disconnected",
                pProc->szName, nPortSrc, len, pDst->szName);
        return -1;
    }

    if (!strncmp(pProc->szName, "DUT", 3) && !strncmp(pDst->szName, "DUT", 3))
    {
        pktswitch_info("%s %d send to %s %d, len:%d",pProc->szName,nPortSrc,pDst->szName, pMap->nPortDst,len);
        for(i=0;i<len;i++)
        {
            printf("%02x ",(unsigned int)buf[i] & 0xff);
            if((i+1)%16 == 0)
            {
                printf("\n");
            }
        }
    }

    return send_packet_to_client(pDst, pMap->nPortDst, buf, len);
}

void read_msg_from_client(int fd, afx_io_dir_t dir, void *arg)
{
    int nbytes,nbytes1=0;
    char buf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *phdr = (MsgHdr_t *)buf;
    ProcItem_t *pProc = (ProcItem_t *)arg;
    int onepktlen=0;

    for (;;) {
	 while (nbytes1!=M_MSG_HDR_LEN)
	 {
	  nbytes = recv(fd,buf,M_MSG_HDR_LEN-nbytes1,MSG_PEEK);
	   nbytes1=nbytes1+nbytes;
	   if (nbytes==0) {
		close_client_session(pProc);
        return;
	   }
	 }
	 onepktlen=ntohs(phdr->nLen);
	 nbytes=readn(fd,buf,onepktlen);
	 if (nbytes!=onepktlen)
	 {
	  pktswitch_err("PacketSwitch receive pkts from  client failed! explen = %d , reallen=%d",onepktlen,nbytes);
	  close_client_session(pProc);
	  return ;
	 }

        #if 0

        nbytes = read(fd, buf, M_MSG_PKT_MAX_LEN);
        if (nbytes < 0) {
            switch (errno) {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
                continue;
            }

            pktswitch_err("Read message from client %s failed: %s",
                    pProc->szName, strerror(errno));
            close_client_session(pProc);
            return;
        } else if (0 == nbytes) {
            /* connection closed */
            close_client_session(pProc);
            return;
        }
       #endif
        break;
    }


    phdr->nType = ntohs(phdr->nType);
    phdr->nLen = ntohs(phdr->nLen) - M_MSG_HDR_LEN - sizeof(unsigned short);
    if (M_MSG_PKT == phdr->nType) {
        unsigned short *pus = (unsigned short *)phdr->data;
        *pus = ntohs(*pus);
        transmit_packet(pProc, *pus, phdr->data + sizeof(unsigned short), phdr->nLen);
        return;
    }

    pktswitch_err("Ignore message from client %s, message type %d",
            pProc->szName, phdr->nType);
    return;
}

int send_register_reply(int sock, int bSuccess, char *pszMsg)
{
    char buf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *phdr = (MsgHdr_t *)buf;
    int len = strlen(pszMsg) + M_MSG_HDR_LEN + sizeof(unsigned short);
    unsigned short *pusResult = (unsigned short *)phdr->data;

    phdr->nType = htons(M_MSG_REGISTER_REPLY);
    phdr->nLen = htons(len);
    *pusResult = bSuccess;
    snprintf(phdr->data + sizeof(unsigned short),
            M_MSG_PKT_MAX_LEN - M_MSG_HDR_LEN - sizeof(unsigned short),
            "%s", pszMsg);
    writen(sock, buf, len);
    if (!bSuccess) {
        pktswitch_err("%s", pszMsg);
        close(sock);
    }
    return 0;
}

void accept_client(int fd, afx_io_dir_t dir, void *arg)
{
    struct sockaddr_un addr;
    socklen_t len = sizeof(struct sockaddr_un);
    int sock;
    MsgHdr_t *pmsg;
    char szBuf[256];
    int nbytes;
    char szName[M_PROCESS_NAME_LEN];
    char szErrMsg[256];
    ProcItem_t *pItem = NULL;

    sock = accept(fd, (struct sockaddr *)&addr, &len);
    if (-1 == sock) {
        pktswitch_err("Accept client failed: %s", strerror(errno));
        return;
    }
    /* pktswitch_err("Accpet connection sock %d", sock); */

    snprintf(szErrMsg, 256, "%s", "internal error");
    /* read register message */
    do {
        nbytes = read(sock, szBuf, 256);
    } while (-1 == nbytes && EINTR == errno);
    if (-1 == nbytes) {
        snprintf(szErrMsg, 256, "Read register message failed: %s",
                strerror(errno));
        goto err_out;
    }

    pmsg = (MsgHdr_t *)&szBuf;
    if (M_MSG_REGISTER != ntohs(pmsg->nType)) {
        snprintf(szErrMsg, 256, "Invalid register message.");
        goto err_out;
    }

    int plen = ntohs(pmsg->nLen) - M_MSG_HDR_LEN;
    if (plen >= M_PROCESS_NAME_LEN) {
        snprintf(szErrMsg, 256, "Process name too long.");
        goto err_out;
    }

    memcpy(szName, pmsg->data, plen);
    szName[plen] = '\0';

    if (0 == locateProcItem(&pItem, szName, NULL)) {
        snprintf(szErrMsg, 256, "Can't locate %s from topology configuration!",
                szName);
        goto err_out;
    }

    afx_mio_t *pMio = NULL;
    if (afx_mio_create(&pMio, sock, AFX_IO_IN,
                read_msg_from_client, pItem) != 0) {
        snprintf(szErrMsg, 256, "WARNING: Out of memory!");
        goto err_out;
    }

    pItem->pArg = pMio;
    if (pItem->sock > 0) {
        close(pItem->sock);
    }
    pItem->sock = sock;
    pItem->connected = 1;
    set_sock_nonblocking(sock);
    set_sock_recvbuf(sock, M_PKT_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_PKT_SOCK_BUF_SZ);
    send_register_reply(sock, 1, "success");
    pktswitch_info("Accpet connection %s success", szName);
    return;

err_out:
    if (sock > 0) {
        send_register_reply(sock, 0, szErrMsg);
    }
    return;
}

void printTopo(void)
{
    int idx;
    int i;

    pktswitch_info("Topology Port Mapping:");
    pktswitch_info("==========================================");
    for (idx = 0; idx < g_nProcsCnt; idx++) {
        ProcItem_t *pProc = &g_pProcs[idx];

        if ('\0' == pProc->szName[0]) {
            continue;
        }

        pktswitch_info("%s:", pProc->szName);

        if (NULL == pProc->pPortArr) {
            continue;
        }

        for (i = 0; i < pProc->nCount; i++) {
            PortMapping_t *pMap = &pProc->pPortArr[i];
            if (!pMap->pProcess) {
                continue;
            }
            pktswitch_info("  %3d -- %3d %s",
                    pMap->nPortSrc, pMap->nPortDst, pMap->pProcess->szName);
        }
    }
    return;
}

int initialize(unsigned char *pszPath, unsigned char *pszCfg)
{
    struct sockaddr_un addr;
    int len;

    if (NULL != g_pProcs) {
        return 0;
    }

    g_pProcs = calloc(sizeof(ProcItem_t), M_DEFAULT_PROCESS_COUNT);
    if (NULL == g_pProcs) {
        pktswitch_err("WARNING: Out of memory!");
        goto err_out;
    }
    g_nProcsCnt = M_DEFAULT_PROCESS_COUNT;
    if (loadTopoCfg(pszCfg) != 0) {
        goto err_out;
    }

#if 1
    pktswitch_info("Topolog file: %s", pszCfg);
#else
    printTopo();
#endif

    g_nSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == g_nSock) {
        pktswitch_err("Create socket failed: %s", strerror(errno));
        return -1;
    }

    unlink(pszPath);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", pszPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    if (bind(g_nSock, (struct sockaddr *)&addr, len) < 0) {
        pktswitch_err("Bind socket failed: %s", strerror(errno));
        goto err_out;
    }

      listen(g_nSock, 3);

    if (afx_mio_create(&g_pMainMio, g_nSock, AFX_IO_IN,
                accept_client, g_pMainMio) != 0) {
        pktswitch_err("Create MIO failed: Out of memory");
        goto err_out;
    }
    return 0;

err_out:
    if (NULL != g_pProcs) {
        free(g_pProcs);
        g_pProcs = NULL;
    }

    if (-1 != g_nSock) {
        close(g_nSock);
        g_nSock = -1;
    }

    return -1;
}

void usage(void)
{
    printf("Usage:");
    printf("\n  pktsd -p unix-socket-path-prefix -t topo-cfg-file [-d]");
    printf("\n        -d for running program as daemon\n");
    return;
}

void signal_init(void)
{
    signal (SIGHUP, SIG_IGN);
    signal (SIGUSR1, SIG_IGN);
    signal (SIGUSR2, SIG_IGN);
    signal (SIGPIPE, SIG_IGN);
    return;
}

/*SDK Comment*/
#if 0
int main(int argc, char **argv)
{
    int opt;
    int bForeground = 1;

    while (1)
    {
        opt = getopt(argc, argv, "p:t:d");

        if (EOF == opt) {
            break;
        }

        switch (opt) {
        case 0:
            break;
        case 'd':
            bForeground = 0;
            break;
        case 'p':
            snprintf(g_szUnixSockPath, PATH_MAX,
                    "%s-%s", optarg, M_UNIX_SOCK_SUFFIX_PS);
            break;
        case 't':
            g_pszTopoCfg = optarg;
            break;
        default:
            usage ();
            exit (-1);
            break;
        }
    }

    if ('\0' == g_szUnixSockPath[0] || NULL == g_pszTopoCfg) {
        usage();
        exit (-1);
    }


    signal_init();
    openlog("pktsd", LOG_PID, LOG_USER);
    afx_event_loop_create();
    initialize(g_szUnixSockPath, g_pszTopoCfg);
    pktswitch_info("Ready to service!");
    if (!bForeground) {
        daemon(0, 0);
    }
    afx_event_loop_run();

    return 0;
}
#endif
