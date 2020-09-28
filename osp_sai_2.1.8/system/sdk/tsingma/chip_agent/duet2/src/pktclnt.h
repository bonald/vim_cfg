/****************************************************************************
* $Id$
*  Packet client APIs
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-10-11 15:36
* Reason        : First Create.
****************************************************************************/

#ifndef _PKTCLNT_H_
#define _PKTCLNT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* return 0 for success
 * return -1 for failed
 */


typedef struct AgtHdr {
#define M_AGT_PKT               10   /* message is a packet */
    unsigned short nType; /* message type */
    unsigned short nLen; /* total length (include header) */
} AgtHdr_t;


#define M_AGT_HDR_LEN    (sizeof(AgtHdr_t))

/* setting unix sock path and dut name */
int set_dut_name(char *pszName);
int set_sock_path_prefix(char *pszPathPrefix);
char *get_dut_name(void);
char *get_sock_path_prefix(void);

int set_send_pkt_file_name(char *sendpktfilename);
char *get_send_pkt_file_name(void);

/* 1. initialize packet agent
 * 2. connect to packet switch
 * 3. start monitor thread
 */
int start_packet_thread(void);
int stop_packet_thread(void);

/* connect to packet switch */
int connect_pktswitch(void);
/* start up packet agent */
int initialize_pktagent();
/* connect to packet agent */
int connect_pktagent(int *psock, int nPort, int bWriter);

/* send packet to pktswitch from port nSrcPortId */
int send_packet(int nSrcPortId, char *pbuf, size_t len);
void register_pkt_callback(pkt_cb_t cb, void *parg);
int send_register_message(int fd, char *pszProcName);
int client_sendto_agent (int fd, char *buf, int len, int type);
int client_recvfrom_agent (int fd,char *buf,int *len);

/* for multi chip SDK & Cmodel */
int start_remote_access_agent(void);
/* notify bits */
#define SDK_NOTIFY_TYPE_INTERRUPT       0x1
#define SDK_NOTIFY_TYPE_STATS           0x2
int send_notify_to_sdk(char *pmsg, int bufsz, u_int16_t type);

typedef int (*remote_access_cb_t)(unsigned short nType, char *buf, size_t len, void *arg);
typedef struct tagRemoteAccessCbArg {
    remote_access_cb_t cb;
    void *parg;
} RemoteAccessCbArg_t;

void register_remote_access_callback(remote_access_cb_t cb, void *parg);
int dummy_remote_access_cb(unsigned short nType, char *buf, size_t len, void *arg);


#ifdef __cplusplus
}
#endif

#endif /* !_PKTCLNT_H_ */
