/****************************************************************************
* $Id$
*  sdk remote access cmodel
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2016-10-27 10:50
* Reason        : First Create.
****************************************************************************/

#ifndef _CM_COM_REMOTE_H_
#define _CM_COM_REMOTE_H_

/* max remote sessions */
#define M_REMOTE_MAX_SESS       16
/* max remote socker send / recv buffer size */
#define M_REMOTE_SOCK_BUF_SZ    2048000

/* remote message header */
typedef struct tagRemoteMsgHdr {
    u_int32_t nReqId; /* request id */
    
#define M_REMOTE_MSG_REGISTER           1   /* register message from client to server */
#define M_REMOTE_MSG_REGISTER_REPLY     2   /* register reply */
#define M_REMOTE_MSG_NOTIFY             3   /* notify message */

#define M_REMOTE_MSG_IO                 10  /* IO */
#define M_REMOTE_MSG_DMA                11  /* DMA */
#define M_REMOTE_MSG_DMA_CLIENT         12  /* DMA: Cmodel request*/
#define M_REMOTE_MSG_DMA_CLIENT_REPLY   13

    unsigned short nType; /* message type */

    unsigned short nLen; /* total length (include header) */
    char data[1];
} RemoteMsgHdr_t;
#define M_REMOTE_MSG_HDR_LEN        (sizeof(RemoteMsgHdr_t) - 1)
#define M_REMOTE_MSG_MAX_LEN        20480  /* packet max length */

typedef struct {
    int32_t result; /* IO result */
    u_int32_t value; /* value for read if success */
} RemoteIO_Reply_t;

typedef struct {
    u_int16_t rw; /* 0: read, 1: write */
    u_int16_t lchip;
    u_int32_t offset;
    u_int32_t value; /* value for write */
} RemoteIO_Req_t;

typedef struct {
    int32_t result; /* DMA result */
    u_int32_t type; /* 0: reply for pkt sending, 1: reply for reg dump*/
    u_int16_t len; /* entry_size for reg dump */
} RemoteDMA_Reply_t;

typedef struct {
    u_int16_t type;/*0: pkt, 1:reg dump*/
    u_int16_t len;/* length for pkt/entry_size for reg dump*/
    u_int16_t tbid;/* table id for reg dump*/
    u_int16_t index;/* entry index for reg dump*/
} RemoteDMA_Req_t; 

int cm_send_io_message(u_int8_t chip_id, int32_t index, u_int32_t cmd, void* val);
int32 cm_sim_send_pkt2dma(uint8 lchip, uint8* packet, uint32 packet_len);
int32 cm_sim_reg_memory_dump(uint8 lchip, uint32 table_id, uint32 entry_index, uint32 entry_size, uint8* buf);
int32 cm_sim_dma_fifo(uint8 lchip, uint8* fifo_type, uint8* buf);
#endif /* !_CM_COM_REMOTE_H_ */
