/****************************************************************************
* $Id$
*  Packet Switch Defines
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-10-09 14:40
* Reason        : First Create.
****************************************************************************/

#ifndef _PKTSWITCH_H_
#define _PKTSWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* for packet switch */
#define M_UNIX_SOCK_SUFFIX_PS       "ps"
/* for vtysh */
#define M_UNIX_SOCK_SUFFIX_CLI      "cli"

/* packet socket snd/rcv buf size */
#define M_PKT_SOCK_BUF_SZ           2048000

/* Process Name max length */
#define M_PROCESS_NAME_LEN          32

/* default processes count (for 6 DUT env) */
#define M_DEFAULT_PROCESS_COUNT     16

/* default ports count (48 + 8) */
#define M_DEFAULT_PORTS_COUNT       128
#define M_MAX_PORTS_ID              128
    
struct tagProcItem;

/* Port mapping */
typedef struct tagPortMapping {
    unsigned short nPortSrc; /* source port Id */
    unsigned short nPortDst; /* destination port id */
    struct tagProcItem *pProcess; /* destination process item pointer */
} PortMapping_t;

/* Process Topo Configuration Item */
typedef struct tagProcItem {
    char szName[M_PROCESS_NAME_LEN]; /* process Name: CTP or DUTx */
    
    PortMapping_t *pPortArr; /* port mapping array */
    int nCount; /* port array size */
    
    int connected; /* session established */
    int sock; /* socket id for this session */
    void *pArg; /* argument for this session */

    char cType; /* Main channel 'M' or interrupt channel 'I' */
} ProcItem_t;

typedef struct tagMsgHdr {
#define M_MSG_REGISTER          1   /* register message from client to server */
#define M_MSG_REGISTER_REPLY    2   /* register reply */
#define M_MSG_NOTIFY            3   /* notify message */

#define M_MSG_PKT               10   /* message is a packet */
    unsigned short nType; /* message type */
    unsigned short nLen; /* total length (include header) */
    char data[1];
} MsgHdr_t;
#define M_MSG_HDR_LEN           (sizeof(MsgHdr_t) - 1)
#define M_MSG_PKT_MAX_LEN       20480  /* packet max length */

typedef int (*pkt_cb_t)(unsigned short nPortId, char *buf, size_t len, void *arg);
typedef struct tagPktCbArg {
    pkt_cb_t cb;
    void *parg;
} PktCbArg_t;

#ifdef __cplusplus
}
#endif

#endif /* !_PKTSWITCH_H_ */
