
#ifndef __SAL_SOCK_H__
#define __SAL_SOCK_H__

/**
 * @file sal_sock.h
 */

/**
 * @defgroup Socket
 * @{
 */

/** Socket Object */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _SAL_LINUX_KM
typedef int32 sal_sock_t;
#else
typedef struct socket sal_sock_t;
#endif

#define sal_sockaddr_in sockaddr_in
#define sal_sockaddr    sockaddr
#define sal_sockaddr_un sockaddr_un
#define sal_socklen_t   socklen_t

#define sal_socket      socket
#define sal_accept      accept
#define sal_bind        bind
#define sal_listen      listen
#define sal_connect     connect
#define sal_sendto      sendto
#define sal_recvfrom    recvfrom
#define sal_send        send
#define sal_recv        recv
#define sal_close       close
#define sal_setsockopt  setsockopt
#define sal_getsockopt  getsockopt

#undef sal_sock_read
#define sal_sock_read read

typedef size_t sal_sock_len_t;
typedef int32 (* SCL_RECV_CB)(void *);
/* Macros.  */

/* The socket handle type. */
#define SAL_SOCK_ERROR ((sal_sock_handle_t)-1)
typedef int sal_sock_handle_t;
typedef signed int result_t;	/* result from a function call */
#define RESULT_OK                   0
#define SAL_MESSAGE_SIZE            4000
#define MESSAGE_MAX_LEN    8192

#define _container_of(p,len) \
    ((uint8*)p+(len))

typedef enum
{
  SAL_FALSE = 0,			/* Everybody calls zero false... */
  SAL_TRUE = (!SAL_FALSE)		/* Some want TRUE=1 or TRUE=-1 or TRUE!=0 */
}
bool_t;

enum sock_err
{
    SOCK_E_NONE = 0,
    SOCK_E_CREATE_ERR = -1000,
    SOCK_E_BIND_ERR,
    SOCK_E_ACCEPT_ERR,
    SOCK_E_READ_ERR,
    SOCK_E_SEND_ERR,
    SOCK_E_SET_OPT_ERR,
};

typedef struct unix_sock_server_s
{
    int32               sock;                       /* listen FD */
    int32               csock;                      /*accept FD */
    sal_task_t*         t_accept;                   /* accept task */
    SCL_RECV_CB         rcv_callback;
    sal_task_t*         recv_task;
    uint8 buf[MESSAGE_MAX_LEN];
    uint16 len;
} unix_sock_server_t;

struct unix_sock_client_s
{
    sal_sock_handle_t sock;
    int32 msg_id;

    uint8 buf[MESSAGE_MAX_LEN];
    uint16 len;
    bool socket_has_init;
    SCL_RECV_CB       rcv_callback;
    sal_task_t*       recv_task;
    sal_task_t*       t_connect;                   /* conncet task */

    struct sal_packet *send_head;
    struct sal_packet *send_tail;

    struct sal_packet *hp_unuse_head;	/* Packet unused list head.  */
    uint16 unuse_count;		/* Pakcet unused count.  */
    uint16 unuse_max;		/* Packet unused maximum number.  */
};
typedef struct unix_sock_client_s unix_sock_client_t;

enum sal_sock_msg_type_e
{
    SAL_SOCK_MSG_TYPE_MIN = 0,
    SAL_SOCK_MSG_TYPE_PKT,

    /*The maximum*/
    SAL_SOCK_MSG_TYPE_MAX
};
typedef enum sal_sock_msg_type_e sal_sock_msg_type_t;

struct sal_msg_s
{
    sal_sock_msg_type_t msg_type; /*certain message type from NSM and chsm to HAL*/
    uint32 msg_len; /*the length of message*/
    uint8 *msg_data; /*the information of message*/
};
typedef struct sal_msg_s sal_msg_t;

struct sal_session_info_s
{
    sal_sock_handle_t st_sock;
    uint32 ul_msg_id;
    uint8 auc_buf[MESSAGE_MAX_LEN];
    uint16 us_len;
};
typedef struct sal_session_info_s sal_session_info_t;

struct sal_packet
{
    struct sal_packet *next;
    struct sal_packet *prev;

    int16 length;

    /* Pointer to data stream.  */
    uint8 buf[SAL_MESSAGE_SIZE];
};

struct sal_gb_pkt_rx_s
{
    uint32          mode;        /**< [GB] Packet Transport mode */
    uint32          pkt_len;     /**< [GB] Packet length in bytes */
    uint32          dma_chan;    /**< [GB] DMA Channel ID of packet RX */
    uint32          eth_hdr_len; /**< [GB] CPUMAC header length in bytes, CTC_PKT_CPUMAC_LEN for ETH, 0 for DMA */
    uint32          pkt_hdr_len; /**< [GB] Packet header length in bytes, CTC_PKT_HEADER_LEN */
    uint32          stk_hdr_len; /**< [GB] Stacking header length in bytes, 0 for packet from local chip */
    uint32          buf_count;   /**< [GB] Count of packet buffer array */
    uint32          last_frag_len;
    uint8*          pkt_buf;     /**< [GB] Packet buffer array */
};
typedef struct sal_gb_pkt_rx_s sal_gb_pkt_rx_t;


#define SAL_MSG_HDR_LEN (sizeof(sal_msg_t) - 4)
#define SAL_GB_PKT_HDR_SIZE (sizeof(sal_gb_pkt_rx_t) - 4)
#define SAL_PKT_MAX_SIZE   3072


int32
ctc_sal_sock_server_init(char *user_path);
int32
ctc_sal_sock_client_init(char *user_path);
int32
ctc_sal_sock_server_send(uint8* p_req,uint32 req_len);
int32
ctc_sal_sock_client_send(uint8* buf,uint32 length);
int32
ctc_sal_sock_register_recv_cb(SCL_RECV_CB cb, bool b_server);


#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup Socket */

#endif /* !__SAL_SOCK_H__ */

