
#ifndef _CTC_SOCK_H_
#define _CTC_SOCK_H_

#include "ctc_msg.h"
#include "ctclib_list.h"

#define SOCK_SYNC_WAIT_TIME     10
#define MS_PER_SEC              1000
#define SOCK_TICS_PER_SEC       (MS_PER_SEC/SOCK_SYNC_WAIT_TIME)

#define CTC_SOCK_MAX            32
#define IGNORE_SOCK_PEER        0
#define INVALID_SOCK_ID         -1

#define CTC_SOCK_TCP            0   /* AF_INET, SOCK_STREAM */
#define CTC_SOCK_UDP            1   /* AF_INET, SOCK_DGRAM */
#define CTC_SOCK_IPC            2   /* AF_UNIX, SOCK_STREAM */
#define CTC_SOCK_IPC_PKT        3   /* AF_UNIX, SOCK_DGRAM */
#define CTC_SOCK_NETLINK        4   /* AF_NETLINK, SOCK_RAW */

#define SOCK_BUILD_PEER(_pid_, _src_)   (_pid_ << 16) | (_src_)
#define SOCK_GET_PEER_PMID(_peer_)      (_peer_ & 0xFFFF)
#define SOCK_GET_PEER_PID(_peer_)       ((_peer_ >> 16) & 0xFFFF)

#define CTC_SOCK_BUF_SIZE       MSG_SIZE

#define CTC_SOCK_TX_LOCK(_P_SOCK_) \
    if (_P_SOCK_->tx_mutex) sal_mutex_lock(_P_SOCK_->tx_mutex);
#define CTC_SOCK_TX_UNLOCK(_P_SOCK_) \
    if (_P_SOCK_->tx_mutex) sal_mutex_unlock(_P_SOCK_->tx_mutex);

typedef struct ctc_sock_queue_node_s
{
    ctclib_list_node_t node;
    uint32             data_len;
    uint8              data[0];
} ctc_sock_queue_node_t;

typedef struct ctc_sock_session_s
{
    ctclib_list_node_t node;
    int32       fd;
    uint32      peer;                       /* [16:31] pid, [0:15] src pmid */
    uint32      connected;                  /* connected state */
    uint32      tx_seq;                     /* TX sequence ID (packed msg without resend) */
    uint32      tx_msg_seq;                 /* TX msg sequence ID */
    uint32      rx_msg_seq;                 /* RX msg sequence ID */
    uint32      seq_mismatch;               /* RX seqID is not equal to seqID in msg header */
    uint8       rx_buf[CTC_SOCK_BUF_SIZE];  /* RX buffer */
    uint32      rx_buf_len;                 /* RX buffer length */
    uint8       tx_buf[CTC_SOCK_BUF_SIZE];  /* TX buffer */
    uint32      tx_buf_len;
    uint8      *tx_resend_data;
    uint32      tx_resend_len;
    uint32      tx_resend_cnt;
    uint32      tx_queue_cnt;
    uint32      total_tx_bytes;
    uint32      total_rx_bytes;
    uint32      tx_error_cnt;               /* TX error count without queue */
    uint32      curr_tx_queue_cnt;
    ctclib_list_t tx_queue;
    ctc_task_t *t_read;
    void       *p_sock;
} ctc_sock_session_t;

typedef int32 (*SOCK_CONNECT_FN)(int32 sock);
typedef int32 (*SOCK_ACCEPT_FN)(int32 sock, ctc_sock_session_t *p_session, struct sal_sockaddr *p_addr);
typedef void (*SOCK_READ_FN)(void* p_data);
typedef void (*SOCK_WRITE_FN)(void* p_data);
typedef int32 (*SOCK_SESSION_DEL_FN)(ctc_sock_session_t* p_session);

typedef struct ctc_sock_s
{
    ctclib_list_node_t node;
    const char     *desc;
    int32           sock;
    int32           fd;
    int32           domain;
    int32           type;
    int32           is_server;
    int32           immediate;              /* immediate connect for client */
    int32           connect_retry_count;    /* immediate connect for client */
    uint32          pmid;
    uint32          flags;
#define M_SOCK_FLAG_NO_GLOBAL_CONNECT   0x01
    uint32          queue_en;
    uint32          pack_en;
    uint32          connect_retry_interval; /* connect retry interval in ms */
    uint32          non_blocking;
    uint32          send_to_ms;
    union
    {
        struct sal_sockaddr_un  un_addr;
        struct sal_sockaddr_in4 in_addr;
        struct sal_sockaddr_nl  nl_addr;
    } addr;
    ctc_task_t     *t_connect;
    ctc_task_t     *t_accept;
    sal_mutex_t    *tx_mutex;
    
    SOCK_CONNECT_FN retry_connect_fn;
    SOCK_CONNECT_FN connect_fn;
    SOCK_ACCEPT_FN  accept_fn;
    SOCK_READ_FN    read_fn; /* message read callback */
    SOCK_SESSION_DEL_FN sess_del_fn;
    ctclib_list_t   session;
} ctc_sock_t;


typedef struct ctc_sock_master_s
{
    uint32                  count;
    ctc_sock_t             *socks[CTC_SOCK_MAX];
    SOCK_CONNECT_FN         connect_fn;
    SOCK_RX_FN              rx_fn;
} ctc_sock_master_t;

int32
ctc_sock_init();

int32
ctc_sock_deinit();

int32
ctc_sock_set_mutex(int32 sock, uint32 enable);

int32
ctc_sock_register_rx_fn(SOCK_RX_FN fn);

int32
ctc_sock_register_pm_connect_fn(SOCK_CONNECT_FN fn);

int32
ctc_sock_register_connect_fn(int32 sock, SOCK_CONNECT_FN fn);

int32
ctc_sock_register_retry_connect_fn(int32 sock, SOCK_CONNECT_FN fn);

int32
ctc_sock_register_accept_fn(int32 sock, SOCK_ACCEPT_FN fn);

int32
ctc_sock_register_read_fn(int32 sock, SOCK_READ_FN fn);

int32
ctc_sock_register_sess_del_fn(int32 sock, SOCK_SESSION_DEL_FN fn);

int32
ctc_sock_set_connect_retry_interval(int32 sock, uint32 interval);

int32
ctc_sock_get_connect_retry_count(int32 sock);

int32
ctc_sock_get_connect_remain_second(int32 sock);

int32
ctc_socket(int32 mode, int32 protocol, const char *desc);

int32
ctc_sock_session_close(int32 sock, uint32 peer);

int32
ctc_sock_close(int32 sock);

int32
ctc_sock_bind(int32 sock, const char *addr);

int32
ctc_sock_set_pmid(int32 sock, uint32 pmid);

int32
ctc_sock_set_send_connect_msg(int32 sock, uint32 enable);

int32
ctc_sock_set_send_to(int32 sock, uint32 ms);

int32
ctc_sock_connect(int32 sock, const char *addr, int32 immediate);

uint32
ctc_sock_is_connecting(uint32 sock);

int32
ctc_sock_disconnect(int32 sock);

uint32
ctc_sock_get_tx_error_count(int32 sock, uint32 peer);

int32
ctc_sock_send(int32 sock, uint32 peer, ctc_msg_t *p_msg);

int32
ctc_sock_send_sync(int32 sock, uint32 peer, ctc_msg_t *p_req_msg, ctc_msg_t *p_resp_msg, uint32 sync_count);

int32
ctc_sock_send_sync_fei(int32 sock, uint32 peer, ctc_msg_t *p_req_msg, ctc_msg_t *p_resp_msg, uint32 sync_count, uint32 resp_len);

int32
ctc_sock_send_raw(int32 sock, uint32 peer, uint8 *p_data, uint32 length);

ctc_sock_t*
ctc_sock_get_sock(int32 sock);

ctc_sock_session_t*
ctc_sock_get_session_by_fd(int32 sock, int32 sess_fd);

int32
ctc_sock_dump(FILE *fp, uint32 detail);

int32
_ctc_sock_session_close(ctc_sock_session_t *p_session);

void
ctc_socket_fds(fd_set *fdsread);

int
ctc_socket_nonblocking(int32 sock, int nonblock);

int32
ctc_sock_session_set_buf(int32 sock, uint32 peer, uint32 is_rx, uint32 buf_size);

int32
ctc_sock_set_buf(int32 sock, uint32 is_rx, uint32 buf_size);

int32
ctc_sock_get_buf(int32 fd, uint32 is_rx);

int32
ctc_sock_set_queue_en(int32 sock, uint32 queue_en);

int32
ctc_sock_set_pack_en(int32 sock, uint32 pack_en);

int32
ctc_sock_session_resend(ctc_sock_session_t *p_session);

int32
ctc_sock_resend(ctc_sock_t *p_sock);

void
ctc_sock_schedule();

int32
ctc_sock_get_txqueue_depth(int32 sock);

#endif /* _CTC_SOCK_H_ */

