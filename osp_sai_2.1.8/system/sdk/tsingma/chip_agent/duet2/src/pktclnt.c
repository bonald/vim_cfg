/****************************************************************************
* $Id$
*  packet switch client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-10-10 09:06
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
#include <arpa/inet.h>
#include <pthread.h>
#include <stdarg.h>
#include <limits.h>

#include "pktswitch.h"
#include "pktclnt.h"
#include "split.h"
#include "afx_evtlp.h"

#ifndef SDK_IN_USERMODE
#include "cts_xr_types.h"
#include "cts_xr_cmodel_pkt.h"
#endif

#define MY_DEBUG    0

/****************************************************************************
 *
* Defines and Macros
*
****************************************************************************/
#include "bsd_queue.h"

/* agent type */
typedef enum enAgtType {
    E_AGT_WRITER = 1,
    E_AGT_READER = 2,
    E_AGT_RW     = 3, /* reader and writer */
} AgtType_e;

/* for packet agent */
typedef struct tagAgtSess {
    TAILQ_ENTRY(tagAgtSess) entry;

    AgtType_e eType; /* agent type */
    unsigned short nPort; /* port id */
    void *pArg; /* argument for this session */
    int sock; /* socket id */
} AgtSess_t;

typedef TAILQ_HEAD(tagAgtSessHdr, tagAgtSess) AgtSessHdr_t;

typedef struct tagAgtMsg {
    unsigned short nType; /* agent type */
    unsigned short nPort; /* port id */
} AgtMsg_t;

typedef struct tagCtsPktAgt {
    pthread_t thread;
} CtsPktAgt_t;

/****************************************************************************
 *
* Global and Declaration
*
****************************************************************************/
char g_szPathPrefix[PATH_MAX]; /* unix socket path prefix */
char g_szDutName[32]; /* process name */

int g_nClntSock = -1; /* socket description for packet switch */
afx_mio_t *g_pClntMio = NULL; /* Client mio for unix socket */
PktCbArg_t g_pktCb = { NULL, NULL };

int g_nAgtSock = -1; /* socket description for packet agent */
afx_mio_t *g_pAgtMio = NULL; /* Agent mio */

CtsPktAgt_t g_ctsPktAgt; /* hold packet thread params */

AgtSessHdr_t g_agtSessHdr;

char g_sendPktFileName[256]; /* process name */


/* Macro #define*/

/****************************************************************************
 *
* Function
*
****************************************************************************/
void pktclnt_info(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    fprintf(stdout, "%s: %s\n", get_dut_name(), print_buf);
    return;
}

void pktclnt_err(const char *fmt, ...)
{
    va_list ap;
#define __M_LOG_BUF_SIZE    1024
    char print_buf[__M_LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(print_buf, __M_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s: %s\n", get_dut_name(), print_buf);
    return;
}


int set_send_pkt_file_name(char *sendpktfilename)
{
    snprintf(g_sendPktFileName, PATH_MAX, "%s", sendpktfilename);
    return 0;
}

char * get_send_pkt_file_name(void)
{
    return g_sendPktFileName;
}

int set_sock_path_prefix(char *pszPathPrefix)
{
    snprintf(g_szPathPrefix, PATH_MAX, "%s", pszPathPrefix);
    return 0;
}

char * get_sock_path_prefix(void)
{
    return g_szPathPrefix;
}

int set_dut_name(char *pszName)
{
    snprintf(g_szDutName, 32, "%s", pszName);
    return 0;
}

char * get_dut_name(void)
{
    return g_szDutName;
}

void close_connect(void)
{
    if (g_nClntSock > 0) {
        close(g_nClntSock);
        g_nClntSock = -1;
    }
    if (g_pClntMio) {
        afx_mio_destroy(g_pClntMio);
        g_pClntMio = NULL;
    }
    return;
}

void read_msg_from_server(int fd, afx_io_dir_t dir, void *arg)
{
    int nbytes,nbytes1=0;
    char buf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *phdr = (MsgHdr_t *)buf;
    int onepktlen=0;


    for (;;) {
	 while (nbytes1!=M_MSG_HDR_LEN)
	 {
	  nbytes = recv(fd,buf,M_MSG_HDR_LEN-nbytes1,MSG_PEEK);
	  if (nbytes<=0) {
	  	close_connect();
		return;
	  	}
	  nbytes1=nbytes1+nbytes;
	 }
	 onepktlen=ntohs(phdr->nLen);
	 nbytes=readn(fd,buf,onepktlen);
	  if (nbytes<=0) {
	  	close_connect();
		return;
	  }
	 if (nbytes!=onepktlen)
	 {

	  pktclnt_err("%s receive pkts from pktswitch failed! explen = %d , reallen=%d",get_dut_name(),onepktlen,nbytes);
	 close_connect();
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

            pktclnt_err("Read message from server failed: %s",
                    strerror(errno));
            close_connect();
            return;
        } else if (0 == nbytes) {
            /* connection closed */
            close_connect();
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
        if (g_pktCb.cb) {
            g_pktCb.cb(*pus, phdr->data + sizeof(unsigned short), phdr->nLen,
                    g_pktCb.parg);
        }
        return;
    }

    pktclnt_err("Ignore message from server message type %d",
            phdr->nType);
    return;
}

int connect_pktswitch(void)
{
    int sock;
    struct sockaddr_un addr;
    int len;
    char szPath[PATH_MAX];
    char buf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *phdr = (MsgHdr_t *)buf;

    sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0 ) {
        pktclnt_err("Create socket failed: %s", strerror(errno));
        return -1;
    }


    snprintf(szPath, PATH_MAX, "%s-%s",
            get_sock_path_prefix(), M_UNIX_SOCK_SUFFIX_PS);
    memset(&addr, 0x00, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    do
    {
        if (connect (sock, (struct sockaddr *)&addr, len) == 0) {
            /* connection established */
            break;
        }
        pktclnt_err("Connect to server failed: %s",
                strerror(errno));
        sleep(2);
    } while (1);


    if (send_register_message(sock, get_dut_name()) != 0) {
        close(sock);
        return -1;
    }
    /* read reigster replay from server */
    len = read(sock, buf, M_MSG_PKT_MAX_LEN);
    if (len <= 0) {
        pktclnt_err("Connection closed!");
        close(sock);
        return -1;
    }
    phdr->nType = ntohs(phdr->nType);
    if (M_MSG_REGISTER_REPLY != phdr->nType) {
        pktclnt_err("Invalid replay message!");
        close(sock);
        return -1;
    }
    unsigned short *pus = (unsigned short *)phdr->data;
    if (0 == *pus) {
        phdr->nLen = ntohs(phdr->nLen);
        buf[phdr->nLen] = '\0';
        pktclnt_err("%s", phdr->data + sizeof(unsigned short));
        close(sock);
        return -1;
    }

    if (afx_mio_create(&g_pClntMio, sock, AFX_IO_IN,
                read_msg_from_server, g_pClntMio) != 0) {
        pktclnt_err("WARNING: out of memory! Close connection");
        close(sock);
        return -1;
    }
    g_nClntSock = sock;
    set_sock_recvbuf(sock, M_PKT_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_PKT_SOCK_BUF_SZ);
    return 0;
}

/* return 0 for success
 * return -1 for failed
 */
int send_packet(int nSrcPortId, char *pbuf, size_t len)
{
    char szBuf[M_MSG_PKT_MAX_LEN];
    MsgHdr_t *pHdr = (MsgHdr_t *)szBuf;
    int nTotalLen = M_MSG_HDR_LEN + len + sizeof(unsigned short);
    int ret;
    unsigned short *pus;
    pktclnt_info("Start send packet to %s port %d, length %d",
            get_dut_name(), nSrcPortId, (int)len);
    if ((len + M_MSG_HDR_LEN + sizeof(unsigned short)) > M_MSG_PKT_MAX_LEN) {
        /* pkt too large */
        return -1;
    }

    pHdr->nType = htons(M_MSG_PKT);
    pHdr->nLen = htons(nTotalLen);
    pus = (unsigned short *)pHdr->data;
    *pus = ntohs(nSrcPortId);
    memcpy(pHdr->data + sizeof(unsigned short), pbuf, len);

    ret = writen(g_nClntSock, szBuf, nTotalLen);
    if (ret != nTotalLen) {
        /* send packet failed */
        return -1;
    }

    pktclnt_err("Send packet to %s port %d success, length %d",
            get_dut_name(), nSrcPortId, (int)len);
    return 0;
}

int send_register_message(int fd, char *pszProcName)
{
    char szBuf[256];
    MsgHdr_t *pHdr = (MsgHdr_t *)szBuf;
    int len = M_MSG_HDR_LEN + strlen(pszProcName);
    int ret;

    pHdr->nType = htons(M_MSG_REGISTER);
    pHdr->nLen = htons(len);
    snprintf(pHdr->data, 256 - M_MSG_HDR_LEN, "%s", pszProcName);

    ret = write(fd, szBuf, len);
    if (ret != len) {
        pktclnt_err("Close connection for send register message failed: %s",
                strerror(errno));
        return -1;
    }

    return 0;
}

void register_pkt_callback(pkt_cb_t cb, void *parg)
{
    g_pktCb.cb = cb;
    g_pktCb.parg = parg;
    return;
}

#ifndef SDK_IN_USERMODE
int write_pkt_to_file(int nSrcPortId, char *pbuf, size_t len)
{
    FILE *fp = NULL;
    int i = 0;
    unsigned int pkt_crc;
    char temp_pkt[M_MSG_PKT_MAX_LEN] = {0};
    char pktpath[256];

    memcpy(temp_pkt, pbuf, len);

    ctcutil_crc32(0xFFFFFFFF, temp_pkt,len, &pkt_crc);
    memcpy(temp_pkt+len, &pkt_crc, 4);

    snprintf(pktpath, 256, "%s",
            get_send_pkt_file_name());
/*
    if (g_ports_transmit_done[nSrcPortId] == 0)
    {
        fp = fopen(SENDPKTFILE, "w");
    }
    else*/
    {
        fp = fopen(pktpath, "a");
    }



    if (!fp)
    {
        printf("Open file failed!!!");
        return -1;
    }

    fprintf(fp, "PKT: packet_length=%d chip_id= 2 src_channel=%d\n", (len+4), nSrcPortId);

    while (i < (len+4))
    {
        fprintf(fp, "%02x", temp_pkt[i]);
        i++;

        if ((i%16) == 0)
        {
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "\n");

    fclose(fp);
    fp = NULL;

    return 0;

}
#endif

void close_agent_client_session(AgtSess_t *psess)
{
    afx_mio_t *pMio = (afx_mio_t *)psess->pArg;

#if 0
    pktclnt_err("=====================Connection for agent client port %d closed",
            psess->nPort);
#endif
    if (psess->sock > 0) {
        close(psess->sock);
    }

    if (NULL != pMio) {
        afx_mio_destroy(pMio);
    }

    TAILQ_REMOVE(&g_agtSessHdr, psess, entry);
    free(psess);
    return;
}

void read_msg_from_agent_client(int fd, afx_io_dir_t dir, void *arg)
{
    int nbytes;
    char buf[M_MSG_PKT_MAX_LEN];
    AgtSess_t *psess = (AgtSess_t *)arg;
    AgtHdr_t *phdr = (AgtHdr_t *)buf;

    for(;;) {
		nbytes=readn(fd,buf,M_AGT_HDR_LEN);
		 if (nbytes<=0)
              {

				close_agent_client_session(psess);
				return;
              }
		if (nbytes!= M_AGT_HDR_LEN)
		{
			pktclnt_err("Read message header from client failed nbytes=%d.",nbytes);
			close_agent_client_session(psess);
			return ;
		}
		int pktlen=ntohs(phdr->nLen);

		nbytes=readn(fd,buf,pktlen);
		 if (nbytes<=0)
              {
				close_agent_client_session(psess);
				return;
              }
		if (nbytes!=pktlen)
		{
			pktclnt_err("Read message from client failed.");
			close_agent_client_session(psess);
			return ;
		}
           break;
	}


    if (E_AGT_READER == psess->eType) {
        /* drop message from reader */
        pktclnt_err("Drop message from reader agent client");
        return;
    }


    if (g_nClntSock > 0) {
        #ifndef SDK_IN_USERMODE
        if (!strcmp(get_dut_name(),"CTP"))
        {
            write_pkt_to_file(psess->nPort, buf, nbytes);
        }
        #endif

        /* transmit message from writer */
        if (send_packet(psess->nPort, buf, nbytes) != 0) {
            pktclnt_err("Send message from agent client %d to packet switch failed: %s",
                    psess->nPort, strerror(errno));
        }
        return;
    }

    pktclnt_err("Drop message from agent client %d", psess->nPort);
    return;
}

void accept_agent_client(int fd, afx_io_dir_t dir, void *arg)
{
    struct sockaddr_un addr;
    socklen_t len;
    int sock;
    char szBuf[256];
    int nbytes;
    char szErrMsg[256];
    AgtSess_t *psess = NULL;
    AgtMsg_t *pmsg = (AgtMsg_t *)szBuf;

    sock = accept(fd, (struct sockaddr *)&addr, &len);
    if (-1 == sock) {
	 if (EINTR == errno || EINPROGRESS == errno || EAGAIN == errno) {
	 	return;
	 }

        pktclnt_err("Agent accept client failed: %s", strerror(errno));
	 close(g_nAgtSock);
	 g_nAgtSock = -1;
        return;
    }

    snprintf(szErrMsg, 256, "%s", "Agent internal error");
    do {
        nbytes = read(sock, szBuf, 256);
    } while (-1 == nbytes && EINTR == errno);
    if (-1 == nbytes) {
        snprintf(szErrMsg, 256, "Agent Read register message failed: %s",
                strerror(errno));
        goto err_out;
    }

    if (NULL == (psess = calloc(sizeof(AgtSess_t), 1))) {
        snprintf(szErrMsg, 256, "WARNING: Out of memory!");
        goto err_out;
    }

    if (E_AGT_WRITER == ntohs(pmsg->nType)) {
        psess->eType = E_AGT_WRITER;
        psess->nPort = ntohs(pmsg->nPort);
    } else if (E_AGT_READER == ntohs(pmsg->nType)) {
        psess->eType = E_AGT_READER;
        psess->nPort = ntohs(pmsg->nPort);
    } else if (E_AGT_RW == ntohs(pmsg->nType)) {
        psess->eType = E_AGT_RW;
        psess->nPort = ntohs(pmsg->nPort);
    } else {
        snprintf(szErrMsg, 256, "Agent invalid type %d",
                ntohs(pmsg->nType));
        goto err_out;
    }

    if (afx_mio_create((afx_mio_t **)(&psess->pArg), sock, AFX_IO_IN,
                read_msg_from_agent_client, psess) != 0) {
        snprintf(szErrMsg, 256, "WARNING: Out of memory!");
        goto err_out;
    }

    psess->sock = sock;
    set_sock_nonblocking(sock);
    set_sock_recvbuf(sock, M_PKT_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_PKT_SOCK_BUF_SZ);
    TAILQ_INSERT_TAIL(&g_agtSessHdr, psess, entry);
#if MY_DEBUG
    pktclnt_info("Agent accept client port %d success\r", psess->nPort);
#endif

    /* send reply */
    writen(sock, (char *)pmsg, sizeof(AgtMsg_t));
    return;

err_out:
    pktclnt_err("Agent accept client failed: %s", szErrMsg);
    if (NULL != psess) {
        free(psess);
    }
    if (sock > 0) {
        close(sock);
    }
    return;
}

int pktagt_cb(unsigned short nPortId, char *buf, size_t len, void *arg)
{
    AgtSess_t *psess = NULL;
    int nbytes = 0;
    int bTrans = 0;
    char szbuf[M_MSG_PKT_MAX_LEN];
    AgtHdr_t *phdr = (AgtHdr_t *)szbuf;

    TAILQ_FOREACH(psess, &g_agtSessHdr, entry) {
        if (E_AGT_WRITER == psess->eType) {
            continue;
        }

        if (psess->sock > 0 && nPortId == psess->nPort) {
            /* forward packet to capture thread */
   		 phdr->nType=htons(M_AGT_PKT);
   		 phdr->nLen=htons(len);
   		 memcpy(szbuf+M_AGT_HDR_LEN,buf,len);
              nbytes = writen(psess->sock, szbuf, len+M_AGT_HDR_LEN);
              if (nbytes != len+M_AGT_HDR_LEN) {
                pktclnt_err("%s packet agent transmit packet to capture thread failed. port %d len %d",
                        get_dut_name(), nPortId, (int)(len+M_AGT_HDR_LEN));
            } else {
                bTrans = 1;
            }
        }
    }

    if (!bTrans) {
        pktclnt_err("%s packet agent can't transmit packet port %d len %d",
                get_dut_name(), nPortId, (int)len);
    }
    return 0;
}

int initialize_pktagent(void)
{
    struct sockaddr_un addr;
    char szPath[PATH_MAX];
    int len;
    int sock;
    if (g_nAgtSock > 0) {
        return 0;
    }

    TAILQ_INIT(&g_agtSessHdr);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == sock) {
        pktclnt_err("Create packet agent socket failed: %s", strerror(errno));
        return -1;
    }

    snprintf(szPath, PATH_MAX, "%s-%s",
            get_sock_path_prefix(), get_dut_name());
    unlink(szPath);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    if (bind(sock, (struct sockaddr *)&addr, len) < 0) {
        pktclnt_err("Bind packet agent socket failed: %s", strerror(errno));
        goto err_out;
    }

    listen(sock, 10);

    if (afx_mio_create(&g_pAgtMio, sock, AFX_IO_IN,
                accept_agent_client, g_pAgtMio) != 0) {
        pktclnt_err("Create MIO for packet agent failed: Out of memory");
        goto err_out;
    }

    g_nAgtSock = sock;
    set_sock_nonblocking(sock);
    register_pkt_callback(pktagt_cb, NULL);
    pktclnt_info("Initialize packet agent socket success");
    return 0;

err_out:
    if (sock > 0) {
        close(sock);
    }
    return -1;
}

int connect_pktagent(int *psock, int nPort, int nRW)
{
    int sock;
    struct sockaddr_un addr;
    int len;
    char szPath[PATH_MAX];
    AgtMsg_t hdr;
    int loops = 10;

    sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0 ) {
        pktclnt_err("Create socket failed: %s", strerror(errno));
        return -1;
    }

    snprintf(szPath, PATH_MAX, "%s-%s",
            get_sock_path_prefix(), get_dut_name());
    memset(&addr, 0x00, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", szPath);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
    do
    {
        if (connect (sock, (struct sockaddr *)&addr, len) == 0) {
            /* connection established */
            break;
        }
        pktclnt_err("Connect to packet agent failed: %s",
                strerror(errno));
        sleep(2);
    } while ((loops--) > 0);

    /* send register message */
    hdr.nType = htons(nRW);
    hdr.nPort = htons(nPort);
    len = writen(sock, (char *)&hdr, sizeof(AgtMsg_t));
    if (len != sizeof(AgtMsg_t)) {
        pktclnt_err("Connection to packet agent closed!");
        close(sock);
        return -1;
    }

    /* read reply and drop it */
    len = readn(sock, (char *)&hdr, sizeof(AgtMsg_t));
    if (len != sizeof(AgtMsg_t)) {
        pktclnt_err("Connection to packet agent closed! Read reply failed");
        close(sock);
        return -1;
    }

    set_sock_recvbuf(sock, M_PKT_SOCK_BUF_SZ);
    set_sock_sendbuf(sock, M_PKT_SOCK_BUF_SZ);
    *psock = sock;
#if MY_DEBUG
    pktclnt_info("Connect to packet agent success. Port %d sock %d",
            ntohs(hdr.nPort), sock);
#endif
    return 0;
}

/* pthread for monitor packet agent sockets */
STATIC void *run_pktagent_engine(void *param)
{
    afx_event_loop_run();
    return NULL;
}

/* return 0 for success
 * return -1 for fail
 */
int start_packet_thread(void)
{
    int rs;
    pthread_attr_t tattr;

    stop_packet_thread();

    afx_event_loop_create(); /* XXX */
    if (initialize_pktagent() != 0 || connect_pktswitch() != 0) {
        return -1;
    }

    pthread_attr_init(&tattr);
    pthread_attr_setstacksize(&tattr, 0x100000);
    rs = pthread_create(&g_ctsPktAgt.thread, &tattr, run_pktagent_engine, NULL);
    if (rs != 0)
    {
        pktclnt_err("Create packet agent thread failed: %s",
                strerror(errno));
        return -1;
    }

    pktclnt_info("Start packet agent thread accomplished...");
    return 0;
}

int stop_packet_thread(void)
{
   if (g_ctsPktAgt.thread > 0) {
        /* let packet thread exit */
        afx_event_loop_exit();
    	 /* pthread_kill(g_ctsPktAgt.thread, SIGINT); */
        pthread_join(g_ctsPktAgt.thread, NULL);

    }

    if (g_nAgtSock > 0) {
        close(g_nAgtSock);
        g_nAgtSock = -1;
    }

    while (!TAILQ_EMPTY(&g_agtSessHdr)) {
        AgtSess_t *psess = (AgtSess_t *)TAILQ_FIRST(&g_agtSessHdr);
        TAILQ_REMOVE(&g_agtSessHdr, psess, entry);
        close_agent_client_session(psess);
    }

    memset(&g_ctsPktAgt, 0x00, sizeof(g_ctsPktAgt));
    return 0;
}


int client_sendto_agent (int fd, char *buf, int len, int type)
{
    char szbuf[M_MSG_PKT_MAX_LEN];
    AgtHdr_t *phdr = (AgtHdr_t *)szbuf;
    phdr->nType=htons(type);
    phdr->nLen=htons(len);
    memcpy(szbuf+M_AGT_HDR_LEN,buf,len);
    int ret;
    ret=writen(fd,szbuf,len+M_AGT_HDR_LEN);
    if (ret != len+M_AGT_HDR_LEN)
    {
		pktclnt_info("client_sendto_agent failed, need send len %d, real send len %d.", len+M_AGT_HDR_LEN,ret);
		assert(0);
		return -1;
    }
    return ret;
}


int client_recvfrom_agent (int fd,char *buf,int *len)
{
       AgtHdr_t *phdr = (AgtHdr_t *)buf;
	int nbytes;

	for(;;) {
		nbytes=readn(fd,buf,M_AGT_HDR_LEN);
		 if (nbytes==0)
              {
				close(fd);
				return -1;
              }
		if (nbytes!= M_AGT_HDR_LEN)
		{
			pktclnt_err("Recv message header from agent failed.");
			assert(0);
			return -1 ;
		}
		int pktlen=ntohs(phdr->nLen);

		nbytes=readn(fd,buf,pktlen);
		 if (nbytes==0)
              {
				close(fd);
				return -1;
              }

		if (nbytes!=pktlen)
		{
			pktclnt_err("Read message from client failed.");
			assert(0);
			return  -1;
		}
	    *len=nbytes;
           break;
	}
    return 0;
}

