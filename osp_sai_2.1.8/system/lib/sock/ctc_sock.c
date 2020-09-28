
#include <stddef.h>
#include "sal.h"
#include "sal_socket.h"
#include "ctc_task.h"
#include "ctc_msg.h"
#include "ctc_sock.h"
#include "ctclib_memory.h"

#define CTC_SOCK_RECONNECT_INTERVAL     500

#define CTC_SOCK_ERROR                  -1
#define CTC_SOCK_EAGAIN                 -2

uint32 g_sock_sync_error_count = 0;
static uint32 glb_pack_en = TRUE;
ctc_sock_master_t *g_p_sock_master = NULL;

static inline int32
_ctc_sock_send(ctc_sock_session_t *p_session, uint8 *buf, uint32 len, int32 flags);

static int32
_ctc_sock_check_msg_pack_cond(ctc_sock_session_t *p_session, uint32 length)
{
    /* left 64 for potential message header buffer */
    if (p_session->tx_buf_len + length > CTC_SOCK_BUF_SIZE - 64)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
_ctc_sock_read(void* p_data)
{
    int32 count = 0;
    int32 rc = 0;
    
    ctc_sock_session_t *p_session = (ctc_sock_session_t *)p_data;
    
    count = CTC_SOCK_BUF_SIZE - p_session->rx_buf_len;
    rc = sal_read(p_session->fd, p_session->rx_buf + p_session->rx_buf_len, count);
    if (rc < 0) 
    {
        switch (errno)
        {
        case EINTR:
        case EAGAIN:
        case EINPROGRESS:
            rc = errno; /* Record errno for debug. */
            return;
        }
        goto ERROR;
    }
    else if (rc == 0)
    {
        goto ERROR;
    }

    p_session->total_rx_bytes += rc;
    rc = ctc_msg_decode(p_session, rc);
    if (rc < 0)
    {
        goto ERROR;
    }
    return;
    
ERROR:
    //sal_printf("socket read error, and close session fd %d, peer (pid %d, pmid %d)\n", p_session->fd, (p_session->peer >> 16), (p_session->peer & 0xFFFF));
    _ctc_sock_session_close(p_session);
    
    return;
}

static void
_ctc_sock_connect(void* p_data)
{
    ctc_sock_t *p_sock = (ctc_sock_t *)p_data;
    ctc_sock_session_t* p_session = NULL;
    int32 non_blocking_inprogress = FALSE;
    int32 interval = 0;
    int32 rc;
    struct timeval tv;

    p_sock->t_connect = NULL;
    p_sock->connect_retry_count++;

    switch (p_sock->domain)
    {
    case AF_UNIX:
        rc = sal_connect(p_sock->fd, (struct sal_sockaddr *)&p_sock->addr.un_addr, sizeof(struct sal_sockaddr_un));
        break;
    case AF_INET:
        errno = 0;
        if (p_sock->send_to_ms)
        {
            tv.tv_sec = p_sock->send_to_ms/1000;
            tv.tv_usec = (p_sock->send_to_ms%1000)*1000;
            sal_setsockopt(p_sock->fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
        }
        rc = sal_connect(p_sock->fd, (struct sal_sockaddr *)&p_sock->addr.in_addr, sizeof(struct sal_sockaddr_in4));
        if (rc < 0)
        {
            if (p_sock->non_blocking && (EINPROGRESS == errno))
            {
                non_blocking_inprogress = TRUE;
            }
        }
        break;
    default:
        return;
    }

    if (rc == 0)
    {
        p_session = XCALLOC(MEM_LIB_SOCK_SESSION, sizeof(ctc_sock_session_t));
        if (p_session)
        {
            p_session->fd = p_sock->fd;
            ctclib_list_init(&p_session->tx_queue);
            p_session->t_read = ctc_task_add_read(TASK_PRI_NORMAL, p_sock->read_fn, p_session, p_session->fd);
            p_session->connected = TRUE;
            p_session->p_sock = p_sock;     /* get session's master */
            ctclib_list_insert_tail(&p_sock->session, &p_session->node);
            /* global connect cb */
            if (!(p_sock->flags & M_SOCK_FLAG_NO_GLOBAL_CONNECT)
                && g_p_sock_master->connect_fn)
            {
                g_p_sock_master->connect_fn(p_sock->sock);
            }
            /* per socket connect cb */
            if (p_sock->connect_fn)
            {
                p_sock->connect_fn(p_sock->sock);
            }
            return;
        }
    }

    if (!p_sock->immediate)
    {
        if (p_sock->retry_connect_fn)
        {
            p_sock->retry_connect_fn(p_sock->sock);
        }
        interval = p_sock->connect_retry_interval;
        if (non_blocking_inprogress)
        {
            interval = (1000 < interval) ? 1000 : interval;
        }
        p_sock->t_connect = ctc_task_add_timer_msec(TASK_PRI_NORMAL, _ctc_sock_connect, p_sock, interval);
    }

    return;
}

static int32
_ctc_sock_close(ctc_sock_t *p_sock)
{
    ctclib_list_node_t *p_node;
    ctc_sock_session_t *p_session;

    CTC_TASK_STOP_TIMER(p_sock->t_connect);
    if (p_sock->t_accept)
    {
        ctc_task_delete_read(p_sock->t_accept);
        p_sock->t_accept = NULL;
    }
    if (p_sock->tx_mutex)
    {
        sal_mutex_destroy(p_sock->tx_mutex);
        p_sock->tx_mutex = NULL;
    }

    while (!ctclib_list_empty(&p_sock->session))
    {
        p_node = ctclib_list_delete_head(&p_sock->session);
        p_session = ctclib_container_of(p_node, ctc_sock_session_t, node);

        if (p_sock->sess_del_fn)
        {
            p_sock->sess_del_fn(p_session);
        }
        if (p_session->t_read)
        {
            ctc_task_delete_read(p_session->t_read);
            p_session->t_read = NULL;
        }
        sal_close(p_session->fd);
        XFREE(MEM_LIB_SOCK_SESSION, p_session);
    }

    return 0;
}

int32
_ctc_sock_session_close(ctc_sock_session_t *p_session)
{
    ctc_sock_t *p_sock = p_session->p_sock;
    ctclib_list_delete(&p_sock->session, &p_session->node);

    if (p_sock->sess_del_fn)
    {
        p_sock->sess_del_fn(p_session);
    }
    
    if (p_session->t_read)
    {
        ctc_task_delete_read(p_session->t_read);
        p_session->t_read = NULL;
    }

    if (p_sock->is_server)
    {
        /* server close accept fd */
        sal_close(p_session->fd);
    }
    else
    {
        sal_close(p_sock->fd);
        p_sock->fd = -1;
        p_sock->fd = sal_socket(p_sock->domain, p_sock->type, 0);
        if (p_sock->fd >= 0)
        {
            /* client reconnect to server */
            if (!p_sock->immediate)
            {
                if (p_sock->retry_connect_fn)
                {
                    p_sock->retry_connect_fn(p_sock->sock);
                }
                p_sock->t_connect = ctc_task_add_timer_msec(TASK_PRI_NORMAL, _ctc_sock_connect, p_sock, p_sock->connect_retry_interval);
            }
        }        
    }
    XFREE(MEM_LIB_SOCK_SESSION, p_session);

    return 0;
}

static void
_ctc_sock_accept(void* p_data)
{
    ctc_sock_t *p_sock = (ctc_sock_t*)p_data;
    ctc_sock_session_t* p_session = NULL;
    struct sal_sockaddr_un un_addr;
    struct sal_sockaddr_in4 in_addr;
    struct sal_sockaddr *p_addr = NULL;
    uint32 addr_size = 0;
    int32 new_fd = 0;

    switch (p_sock->domain)
    {
    case AF_UNIX:
        addr_size = sizeof(struct sal_sockaddr_un);
        p_addr = (struct sal_sockaddr*)&un_addr;
        new_fd = sal_accept(p_sock->fd, p_addr, &addr_size);
        break;
    case AF_INET:    
        addr_size = sizeof(struct sal_sockaddr_in4);
        p_addr = (struct sal_sockaddr*)&in_addr;
        new_fd = sal_accept(p_sock->fd, p_addr, &addr_size);
        break;
    default:
        return;
    }
    
    if (new_fd < 0)
    {
        return;
    }
    p_session = XCALLOC(MEM_LIB_SOCK_SESSION, sizeof(ctc_sock_session_t));
    if (!p_session)
    {
        return;
    }
    
    p_session->fd = new_fd;
    ctclib_list_init(&p_session->tx_queue);
    p_session->t_read = ctc_task_add_read(TASK_PRI_NORMAL, p_sock->read_fn, p_session, p_session->fd);
    p_session->connected = TRUE;
    p_session->p_sock = p_sock;     /* get session's master */
    ctclib_list_insert_tail(&p_sock->session, &p_session->node);

    /* do accept callback process */
    if (p_sock->accept_fn)
    {
        p_sock->accept_fn(p_sock->sock, p_session, p_addr);
    }
    //sal_printf("_ctc_sock_accept new_fd %d\n", new_fd);

    return;
}

static void
_ctc_sock_do_resend(void* p_data)
{
    ctc_sock_session_t *p_session = (ctc_sock_session_t*)p_data;

    ctc_task_set_write(p_session->t_read, NULL);
    ctc_sock_session_resend(p_session);
    
    return;
}

int32
ctc_sock_session_resend(ctc_sock_session_t *p_session)
{
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_queue_node_t *queue_node = NULL;
    uint8 *tx_buf = NULL;
    int32 rc = 0;
    
    if (NULL == p_session->tx_resend_data || 0 == p_session->tx_resend_len)
    {
        return 0;
    }

    rc = _ctc_sock_send(p_session, p_session->tx_resend_data, p_session->tx_resend_len, 0);
    if (rc < 0)
    {
        return rc;
    }
    
    p_session->tx_resend_data = NULL;
    p_session->tx_resend_len = 0;
    p_session->tx_buf_len = 0;
    
    while (CTCLIB_LHEAD((&p_session->tx_queue)))
    {
        p_node = CTCLIB_LHEAD((&p_session->tx_queue));
        queue_node = ctclib_container_of(p_node, ctc_sock_queue_node_t, node);
        if (_ctc_sock_check_msg_pack_cond(p_session, queue_node->data_len))
        {
            rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
            p_session->tx_seq++;
            /* if has resend, break */
            if (p_session->tx_resend_data)
            {
                break;
            }
        }

        tx_buf = p_session->tx_buf + p_session->tx_buf_len;
        sal_memcpy(tx_buf, queue_node->data, queue_node->data_len);
        p_session->tx_buf_len += queue_node->data_len;
        ctclib_list_delete((&p_session->tx_queue), p_node);
        p_session->curr_tx_queue_cnt--;
        XFREE(MEM_LIB_SOCK_QUEUE, queue_node);
    };

    if (NULL == CTCLIB_LHEAD((&p_session->tx_queue)))
    {
        rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
        p_session->tx_seq++;
    }

    return rc;
}

int32
ctc_sock_resend(ctc_sock_t *p_sock)
{
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;
    
    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        ctc_sock_session_resend(p_sess);
    }

    return 0;
}

static int32
_ctc_sock_schedule_send_pack(ctc_sock_t *p_sock)
{
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;
    
    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        /* has data but not tx_resend_data */
        if (p_sess->tx_buf_len && (NULL == p_sess->tx_resend_data))
        {
            _ctc_sock_send(p_sess, p_sess->tx_buf, p_sess->tx_buf_len, 0);
            p_sess->tx_seq++;
        }
    }

    return 0;
}

void
ctc_sock_schedule()
{
    ctc_sock_t *p_sock = NULL;
    uint32 i = 0;
    
    for (i = 0; i < CTC_SOCK_MAX; i++)
    {
        p_sock = g_p_sock_master->socks[i];
        if (p_sock)
        {
            if (p_sock->pack_en)
            {
                _ctc_sock_schedule_send_pack(p_sock);
            }
        }
    }

    return;
}

static int32
_ctc_sock_set_resend(ctc_sock_session_t *p_session, uint8 *ptr, int32 left_len)
{
    p_session->tx_resend_data = ptr;
    p_session->tx_resend_len = left_len;

    ctc_task_set_write(p_session->t_read, _ctc_sock_do_resend);
    
    return 0;
}

static inline int32
_ctc_sock_send(ctc_sock_session_t *p_session, uint8 *buf, uint32 len, int32 flags)
{
    ctc_sock_t *p_sock = NULL;
    uint8 *ptr = NULL;
    uint32 i = 0;
    int32 rc = 0;

    ptr = buf;
 
    p_sock = (ctc_sock_t*)p_session->p_sock;
    
    if (p_sock->queue_en)
    {
        rc = sal_send(p_session->fd, ptr, (size_t)len, flags);
        if (rc < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
            {
                p_session->tx_resend_cnt++;
                _ctc_sock_set_resend(p_session, ptr, len);
                return CTC_SOCK_EAGAIN;
            }
            else
            {
                return CTC_SOCK_ERROR;
            }

        }
        else if (rc == len)
        {
            p_session->total_tx_bytes += rc;
            p_session->tx_buf_len = 0;
            return 0;
        }
        else
        {
            ptr += rc;
            len -= rc;
            p_session->total_tx_bytes += rc;
            p_session->tx_resend_cnt++;
            _ctc_sock_set_resend(p_session, ptr, len);
            return CTC_SOCK_EAGAIN;
        }
    }
    else
    {
#define CTC_SOCK_SEND_COUNT 1000

        while (i < CTC_SOCK_SEND_COUNT)
        {
            rc = sal_send(p_session->fd, ptr, (size_t)len, flags);
            if (rc < 0)
            {
                if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
                {
                    rc = errno; /* Record errno for debug. */
                    sal_task_sleep(10);
                    i++;
                    continue;
                }
                p_session->tx_error_cnt++;
                return CTC_SOCK_ERROR;
            }
            else if (rc == len)
            {
                p_session->total_tx_bytes += rc;
                return 0;
            }
            else
            {
                /* send part of data */
                ptr += rc;
                len -= rc;
                p_session->total_tx_bytes += rc;
            }

            i++;
        }

        p_session->tx_error_cnt++;
        return CTC_SOCK_ERROR;
    }

    return 0;
}

ctc_sock_session_t*
_ctc_sock_get_session(int32 sock, uint32 peer)
{
    ctc_sock_t *p_sock = NULL;
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;

    if (sock < 0)
    {
        return NULL;
    }
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return NULL;
    }

    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        if ((IGNORE_SOCK_PEER == peer) || (p_sess->peer == peer))
        {
            return p_sess;
        }
    }

    return NULL;       
}

static int32
_ctc_sock_get_session_count(ctc_sock_t *p_sock)
{
    int32 count = 0;
    ctclib_list_node_t *p_node = NULL;

    ctclib_list_for_each(p_node, &p_sock->session)
    {
        count++;
    }
    return count;
}

static int32
_ctc_sock_get_unused()
{
    int32 i = 0;
    
    for (i = 0; i < CTC_SOCK_MAX; i++)
    {
        if (!g_p_sock_master->socks[i])
        {
            return i;
        }
    }

    return INVALID_SOCK_ID;
}

static int32
_ctc_sock_str2inet(const char *str, struct sal_sockaddr_in4 *addr)
{
    char buf[64];
    char *p_ip = NULL;
    char *p_port = NULL;
    uint32 port = 0;

    sal_memset(buf, 0, sizeof(buf));
    sal_strncpy(buf, str, 64);
    sal_memset(addr, 0, sizeof(struct sal_sockaddr_in4));

    p_ip = buf;
    p_port = sal_strchr(buf, ':');
    if (NULL == p_port)
    {
        return -1;
    }
    p_port[0] = '\0';
    p_port++;
    
    if (0 == sal_strlen(p_port))
    {
        return -1;
    }
    
    if (1 == sal_inet_pton(AF_INET, p_ip, &addr->sin_addr))
    {
        addr->sin_family = AF_INET;
        if (1 == sal_sscanf(p_port, "%u", &port))
        {
            addr->sin_port = sal_htons(port);
            return 0;
        }
    }

    return -1;
}

int32
ctc_sock_init()
{
    ctc_sock_master_t *p_master = NULL;
    int32 i = 0;
    
    p_master = XCALLOC(MEM_LIB_SOCK_MASTER, sizeof(ctc_sock_master_t));
    for (i = 0; i < CTC_SOCK_MAX; i++)
    {
        p_master->socks[i] = NULL;
    }
    p_master->count = 0;

    g_p_sock_master = p_master;

    ctc_task_register_before_poll_cb(ctc_sock_schedule);
    
    return 0;
}

int32
ctc_sock_deinit()
{
    return 0;
}

void
ctc_socket_fds(fd_set *fdsread)
{
    int i;

    if (NULL == g_p_sock_master) {
        return;
    }

    for (i = 0; i < CTC_SOCK_MAX; i++) {
        ctc_sock_t* s = g_p_sock_master->socks[i];
        if (!s || s->fd < 0) {
            continue;
        }
        FD_SET(s->fd, fdsread);
    }
    return;
}

int32
ctc_sock_register_rx_fn(SOCK_RX_FN fn)
{
    g_p_sock_master->rx_fn = fn;
    return 0;
}

int32
ctc_sock_register_pm_connect_fn(SOCK_CONNECT_FN fn)
{
    g_p_sock_master->connect_fn = fn;
    return 0;
}

int32
ctc_sock_register_connect_fn(int32 sock, SOCK_CONNECT_FN fn)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->connect_fn = fn;
    return 0;
}

int32
ctc_sock_register_retry_connect_fn(int32 sock, SOCK_CONNECT_FN fn)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->retry_connect_fn = fn;
    return 0;
}

int32
ctc_sock_register_accept_fn(int32 sock, SOCK_ACCEPT_FN fn)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->accept_fn = fn;
    return 0;
}

int32
ctc_sock_register_read_fn(int32 sock, SOCK_READ_FN fn)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->read_fn = fn;
    return 0;
}

int32
ctc_sock_register_sess_del_fn(int32 sock, SOCK_SESSION_DEL_FN fn)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->sess_del_fn = fn;
    return 0;
}

int32
ctc_sock_get_connect_retry_count(int32 sock)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    return p_sock->connect_retry_count;
}

int32
ctc_sock_get_connect_remain_second(int32 sock)
{
    ctc_sock_t *p_sock = NULL;
    int32 remain_sec = 0;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    if (NULL == p_sock->t_connect)
    {
        return -1;
    }

    remain_sec = ctc_timer_get_remain_sec(p_sock->t_connect);
    return remain_sec;
}

int32
ctc_sock_set_connect_retry_interval(int32 sock, uint32 interval)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->connect_retry_interval = interval;
    return 0;
}

/*
 mode:      CTC_SOCK_XXX
 protocol:  only valid for CTC_SOCK_NETLINK
 */
int32
ctc_socket(int32 mode, int32 protocol, const char *desc)
{
    ctc_sock_t *p_sock = NULL;
    int32 fd = -1;
    int32 s_domain = 0;
    int32 s_type = 0;
    int32 sock = INVALID_SOCK_ID;
        
    switch (mode)
    {
    case CTC_SOCK_TCP:
        s_domain = AF_INET;
        s_type = SOCK_STREAM;
        protocol = 0;
        break;
    case CTC_SOCK_UDP:
        s_domain = AF_INET;
        s_type = SOCK_DGRAM;
        break;
    case CTC_SOCK_IPC:
        s_domain = AF_UNIX;
        s_type = SOCK_STREAM;
        break;
    case CTC_SOCK_IPC_PKT:
        s_domain = AF_UNIX;
        s_type = SOCK_STREAM;
        break;
    case CTC_SOCK_NETLINK:
        s_domain = AF_NETLINK;
        s_type = SOCK_RAW;
        break;
    default:
        return -1;
    }
    
    fd = sal_socket(s_domain, s_type, protocol);
    if (fd < 0)
    {
        return -1;
    }

    sock = _ctc_sock_get_unused();
    if (sock < 0)
    {
        return -1;
    }
    
    p_sock = XCALLOC(MEM_LIB_SOCK, sizeof(ctc_sock_t));
    p_sock->sock = sock;
    p_sock->desc = desc;
    p_sock->fd = fd;
    p_sock->domain = s_domain;
    p_sock->type = s_type;
    p_sock->queue_en = FALSE;
    p_sock->pack_en = FALSE;
    p_sock->connect_retry_interval = CTC_SOCK_RECONNECT_INTERVAL;
    ctclib_list_init(&p_sock->session);
    p_sock->read_fn = _ctc_sock_read;
    g_p_sock_master->socks[sock] = p_sock;
    g_p_sock_master->count++;
    return sock;
}

int32
ctc_sock_session_close(int32 sock, uint32 peer)
{
    ctc_sock_session_t *p_session = NULL;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return -1;
    }

    return _ctc_sock_session_close(p_session);
}

int32
ctc_sock_close(int32 sock)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }
    
    _ctc_sock_close(p_sock);
    if (p_sock->is_server)
    {
        /* client is closed in session deinit */
        sal_close(p_sock->fd);
    }
    XFREE(MEM_LIB_SOCK, p_sock);
    g_p_sock_master->socks[sock] = NULL;
    g_p_sock_master->count--;
    
    return 0;
}

int32
ctc_sock_set_mutex(int32 sock, uint32 enable)
{
    ctc_sock_t *p_sock = NULL;
    int32 rc = 0;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    if (enable)
    {
        if (p_sock->tx_mutex)
        {
            return -1;
        }
        rc = sal_mutex_create(&p_sock->tx_mutex);
        if (rc < 0)
        {
            return -1;
        }
    }
    else
    {
        if (NULL == p_sock->tx_mutex)
        {
            return -1;
        }
        sal_mutex_destroy(p_sock->tx_mutex);
        p_sock->tx_mutex = NULL;
    }

    return 0;
}

int32
ctc_sock_bind(int32 sock, const char *addr)
{
    ctc_sock_t *p_sock = NULL;
    struct sal_sockaddr_un  *p_un_addr = NULL;
    struct sal_sockaddr_in4 *p_in_addr = NULL;
    struct sal_sockaddr_nl  nl_addr;
    socklen_t nl_addr_len = sizeof(struct sal_sockaddr_nl);
    uint32 nl_groups = 0;
    int32 flags = 1;
    int32 rc = 0;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }
    
    p_sock->is_server = TRUE;
    p_un_addr = &p_sock->addr.un_addr;
    p_in_addr = &p_sock->addr.in_addr;
    
    rc = setsockopt(p_sock->fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
    if (rc < 0)
    {
        return -1;   
    }

    switch (p_sock->domain)
    {
    case AF_UNIX:
        sal_memset(p_un_addr, 0, sizeof(struct sal_sockaddr_un));
        p_un_addr->sun_family = AF_UNIX;
        sal_strncpy(p_un_addr->sun_path, addr, sizeof(p_un_addr->sun_path)-1);
        rc = sal_bind(p_sock->fd, (struct sal_sockaddr *)p_un_addr, sizeof(struct sal_sockaddr_un));
        break;
    case AF_INET:
        rc = _ctc_sock_str2inet(addr, p_in_addr);
#if 0        
        rc = setsockopt(p_sock->fd, IPPROTO_TCP, TCP_NODELAY, (void*)&flags, sizeof(flags));
        if (rc < 0)
        {
        }
#endif
        rc = sal_bind(p_sock->fd, (struct sal_sockaddr *)p_in_addr, sizeof(struct sal_sockaddr_in4));
        break;
    case AF_NETLINK:
        if (!sal_isdigit(addr[0]))
        {
            return -1;
        }
        sal_sscanf(addr, "%u", &nl_groups);
        sal_memset(&nl_addr, 0, sizeof(nl_addr));
        nl_addr.nl_family = AF_NETLINK;
        nl_addr.nl_groups = nl_groups;
        rc = sal_bind(p_sock->fd, (struct sal_sockaddr *)&nl_addr, sizeof(struct sal_sockaddr_nl));
        if (sal_getsockname(p_sock->fd, (struct sal_sockaddr *)&p_sock->addr.nl_addr, &nl_addr_len) < 0)
        {
            return -1;
        }
        return 0;
    default:
        return -1;
    }

    rc = sal_listen(p_sock->fd, 4);
    if (rc < 0)
    {
        return rc;
    }

    p_sock->t_accept = ctc_task_add_read(TASK_PRI_NORMAL, _ctc_sock_accept, p_sock, p_sock->fd);
    return rc;
}

int32
ctc_sock_set_pmid(int32 sock, uint32 pmid)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }
    
    p_sock->pmid = pmid;
    
    return 0;
}

int32
ctc_sock_set_send_connect_msg(int32 sock, uint32 enable)
{
    ctc_sock_t *p_sock = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    if (enable)
    {
        p_sock->flags &= ~M_SOCK_FLAG_NO_GLOBAL_CONNECT;
    }
    else
    {
        p_sock->flags |= M_SOCK_FLAG_NO_GLOBAL_CONNECT;
    }
    
    return 0;
}

int32 
ctc_socket_nonblocking(int32 sock, int nonblock)
{
    ctc_sock_t *p_sock = NULL;
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock || p_sock->fd < 0) {
        return -1;
    }    

    p_sock->non_blocking = nonblock;
    return sal_sock_set_nonblocking(p_sock->fd, nonblock);
}

uint32
ctc_sock_is_connecting(uint32 sock)
{
    ctc_sock_t *p_sock = NULL;
    
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return FALSE;
    }

    if (p_sock->is_server)
    {
        return FALSE;
    }

    if (p_sock->t_connect)
    {
        return TRUE;
    }
    
    return FALSE;
}

int32
ctc_sock_disconnect(int32 sock)
{
    ctc_sock_t *p_sock = NULL;
    ctclib_list_node_t *p_node;
    ctc_sock_session_t *p_session;
    
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    if (p_sock->is_server)
    {
        /* disconnect is used for client */
        return -1;
    }

    /* delete connect task */
    CTC_TASK_STOP_TIMER(p_sock->t_connect);    

    /* delete connected sessions */
    while (!ctclib_list_empty(&p_sock->session))
    {
        p_node = ctclib_list_delete_head(&p_sock->session);
        p_session = ctclib_container_of(p_node, ctc_sock_session_t, node);
        if (p_session->t_read)
        {
            ctc_task_delete_read(p_session->t_read);
            p_session->t_read = NULL;
        }
        sal_close(p_session->fd);
        XFREE(MEM_LIB_SOCK_SESSION, p_session);
    }

    return 0;
}

int32
ctc_sock_set_send_to(int32 sock, uint32 ms)
{
    ctc_sock_t *p_sock = NULL;
    
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_sock->send_to_ms = ms;
    return 0;
}

int32
ctc_sock_connect(int32 sock, const char *addr, int32 immediate)
{
    ctc_sock_t *p_sock = NULL;
    struct sal_sockaddr_un  *p_un_addr = NULL;
    struct sal_sockaddr_in4 *p_in_addr = NULL;
    int32 has_wait = FALSE;
    int32 i = 0;
    int32 rc = 0;
    
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return -1;
    }

    p_un_addr = &p_sock->addr.un_addr;
    p_in_addr = &p_sock->addr.in_addr;
    p_sock->is_server = FALSE;

    switch (p_sock->domain)
    {
    case AF_UNIX:
        sal_memset(p_un_addr, 0, sizeof(struct sal_sockaddr_un));
        p_un_addr->sun_family = AF_UNIX;
        sal_strncpy(p_un_addr->sun_path, addr, sizeof(p_un_addr->sun_path)-1);
        break;
    case AF_INET:
        rc = _ctc_sock_str2inet(addr, p_in_addr);
        if (rc < 0)
        {
            return rc;
        }
        break;
    default:
        return -1;
    }

    p_sock->immediate = immediate;
    if (p_sock->immediate)
    {
        for (i = 0; i < p_sock->immediate; i++)
        {
            _ctc_sock_connect(p_sock);
            if (!ctclib_list_empty(&p_sock->session))
            {
                /* immediate connect success */
                if (has_wait)
                {
                    /* newline for return infomation */
                    sal_printf(" success\n");
                }
                p_sock->connect_retry_count = i;
                return 0;
            }
            sal_task_sleep(CTC_SOCK_RECONNECT_INTERVAL);
            if (!has_wait)
            {
                sal_printf("Connecting ");
                sal_fflush(stdout);
                has_wait = TRUE;
            }
            sal_printf(".");
            sal_fflush(stdout);
        }
        return -1;
    }
    else
    {
        p_sock->t_connect = ctc_task_add_timer(TASK_PRI_NORMAL, _ctc_sock_connect, p_sock, 0);
    }

    return 0;
}

int32
ctc_sock_send_sync(int32 sock, uint32 peer, ctc_msg_t *p_req_msg, ctc_msg_t *p_resp_msg, uint32 sync_count)
{
    ctc_sock_session_t *p_session = NULL;
    int32 left = 0;
    msg_dec_ret_t dec_rc;
    uint8* p = NULL;
    uint32 repeat = 0;
    int32 count = 0;
    int32 rc = 0;
    int32 read_len = 0;
    int32 has_wait = FALSE;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return -1;
    }

    rc = ctc_sock_send(sock, peer, p_req_msg);
    if (rc < 0)
    {
        return -1;
    }
    
    while (repeat < sync_count)
    {
        count = CTC_SOCK_BUF_SIZE - p_session->rx_buf_len;
        read_len = sal_read(p_session->fd, p_session->rx_buf + p_session->rx_buf_len, count);
        //sal_printf("read_len: %d\n", read_len);
        if (read_len < 0) 
        {
            switch (errno)
            {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
                repeat++;
                if ((repeat % SOCK_TICS_PER_SEC) == 0)
                {
                    if (!has_wait)
                    {
#if 0
                        ctc_sock_t *p_sock = p_session->p_sock;
                        sal_printf("Waiting sock %s peer_pm %s oper %s", p_sock->desc, 
                            IPC_DEST_DESC(p_session->peer & 0xFFFF), ctc_msg_oper_desc(p_req_msg->hdr.operation));
                        switch (p_req_msg->hdr.operation)
                        {
                        case MSG_OPER_CFG:
                            sal_printf("cfg (%u %u %u)", p_req_msg->u_hdr.cfg.type, p_req_msg->u_hdr.cfg.id, p_req_msg->u_hdr.cfg.oper);
                            break;
                        case MSG_OPER_FEI:
                            sal_printf("fei (%u %u)", p_req_msg->u_hdr.fei.type, p_req_msg->u_hdr.fei.flags);
                            break;
                        default:
                            break;
                        }
#endif
                        sal_printf("Waiting ");
                        sal_fflush(stdout);
                        has_wait = TRUE;
                    }
                    sal_printf(".");
                    sal_fflush(stdout);
                }
                sal_task_sleep(SOCK_SYNC_WAIT_TIME);
                continue;
            }
            goto ERROR;
        }
        else if (read_len == 0)
        {
            goto ERROR;
        }
        
        p_session->rx_buf_len += read_len;
        p_session->total_rx_bytes += read_len;
        p = p_session->rx_buf;
        left = p_session->rx_buf_len;
        
        dec_rc = ctc_msg_decode_hdr(p, left, p_resp_msg);
        switch (dec_rc.code)
        {
        case MSG_DEC_OK:
            p_session->rx_msg_seq = p_resp_msg->hdr.seq;
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            //SAL_ASSERT(left == 0);
            if (left != 0)
            {
                g_sock_sync_error_count++;
            }
            p_session->rx_buf_len = 0;
            if (has_wait)
            {
                /* newline for return infomation */
                sal_printf(" success\n");
            }
            //sal_printf("get resp len %d consumed %d\n", read_len, dec_rc.consumed);
            return 0;

        case MSG_DEC_MORE:
            if (left > 0 && p_session->rx_buf != p)
            {
                sal_memmove(p_session->rx_buf, p, left);
            }
            p_session->rx_buf_len = left;
            break;

        case MSG_DEC_FAIL:
            goto ERROR;
        }
    }

ERROR:
    if (repeat == sync_count)
    {
        return -1;
    }
    else
    {
        return -2;
    }
}

int32
ctc_sock_send_sync_fei(int32 sock, uint32 peer, ctc_msg_t *p_req_msg, ctc_msg_t *p_resp_msg, uint32 sync_count, uint32 resp_len)
{
    ctc_sock_session_t *p_session = NULL;
    int32 left = 0;
    msg_dec_ret_t dec_rc;
    uint8* p = NULL;
    uint32 repeat = 0;
    int32 count = 0;
    int32 rc = 0;
    int32 read_len = 0;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return -1;
    }

    rc = ctc_sock_send(sock, peer, p_req_msg);
    if (rc < 0)
    {
        return -1;
    }
    
    while (repeat < sync_count)
    {
        count = CTC_SOCK_BUF_SIZE - p_session->rx_buf_len;
        read_len = sal_read(p_session->fd, p_session->rx_buf + p_session->rx_buf_len, count);
        if (read_len < 0) 
        {
            switch (errno)
            {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
                repeat++;
                sal_task_sleep(SOCK_SYNC_WAIT_TIME);
                continue;
            }
            goto ERROR;
        }
        else if (read_len == 0)
        {
            goto ERROR;
        }
        
        p_session->rx_buf_len += read_len;
        p_session->total_rx_bytes += read_len;
        p = p_session->rx_buf;
        left = p_session->rx_buf_len;

DECODE:
        dec_rc = ctc_msg_decode_hdr(p, left, p_resp_msg);
        switch (dec_rc.code)
        {
        case MSG_DEC_OK:
            p_session->rx_msg_seq = p_resp_msg->hdr.seq;
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            p_session->rx_buf_len -= dec_rc.consumed;
            if ((p_resp_msg->u_hdr.fei.type == p_req_msg->u_hdr.fei.type) && (resp_len == p_resp_msg->data_len))
            {
                return 0;
            }
            else
            {
                /* omit mismatch previous msg */
                ctc_msg_free(p_resp_msg);
                if (left)
                {
                    /* if left, decode again; otherwise, read again */
                    goto DECODE;
                }
            }
            break;

        case MSG_DEC_MORE:
            if (left > 0 && p_session->rx_buf != p)
            {
                sal_memmove(p_session->rx_buf, p, left);
            }
            p_session->rx_buf_len = left;
            break;

        case MSG_DEC_FAIL:
            goto ERROR;
        }
    }

ERROR:
    if (repeat == sync_count)
    {
        return -1;
    }
    else
    {
        return -2;
    }
}

int32
ctc_sock_send_encode(uint8 *buf, int32 buf_size, ctc_msg_t *p_msg)
{
    int32   size_left;
    int32  *size = NULL;
    uint8  *pnt = NULL;
    uint8 **ppnt = NULL;

    size_left = buf_size;
    size = &size_left;
    pnt = buf;
    ppnt = &pnt;

    
    MSG_ENCODE_PUTC(p_msg->hdr.version);
    MSG_ENCODE_PUTC(p_msg->hdr.operation);
    MSG_ENCODE_PUTW(p_msg->hdr.length);
    MSG_ENCODE_PUTL(p_msg->hdr.seq);
    
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        MSG_ENCODE_PUTW(p_msg->u_hdr.connect.src);
        MSG_ENCODE_PUTW(p_msg->u_hdr.connect.pid);
        break;

    case MSG_OPER_SYNC:
        MSG_ENCODE_PUTW(p_msg->u_hdr.sync.oper);
        MSG_ENCODE_PUTC(p_msg->u_hdr.sync.dest);
        MSG_ENCODE_PUTC(p_msg->u_hdr.sync.src);
        MSG_ENCODE_PUTL(p_msg->u_hdr.sync.seq);
        break;

    case MSG_OPER_CFG:
        MSG_ENCODE_PUTC(p_msg->u_hdr.cfg.dest);
        MSG_ENCODE_PUTC(p_msg->u_hdr.cfg.src);
        MSG_ENCODE_PUTW(p_msg->u_hdr.cfg.pid);
        MSG_ENCODE_PUTC(p_msg->u_hdr.cfg.oper);
        MSG_ENCODE_PUTC(p_msg->u_hdr.cfg.type);
        MSG_ENCODE_PUTW(p_msg->u_hdr.cfg.id);
        MSG_ENCODE_PUTL(p_msg->u_hdr.cfg.cfg);
        break;

    case MSG_OPER_DATA:
        MSG_ENCODE_PUTC(p_msg->u_hdr.data.oper);
        MSG_ENCODE_PUTC(p_msg->u_hdr.data.type);
        MSG_ENCODE_PUTW(p_msg->u_hdr.data.tbl_id);
        MSG_ENCODE_PUTW(p_msg->u_hdr.data.ds_id);
        MSG_ENCODE_PUTW(p_msg->u_hdr.data.ds2_id);
        MSG_ENCODE_PUTL(p_msg->u_hdr.data.fields[0]);
        MSG_ENCODE_PUTL(p_msg->u_hdr.data.fields[1]);
        break;

    case MSG_OPER_FEI:
        MSG_ENCODE_PUTL(p_msg->u_hdr.fei.type);
        MSG_ENCODE_PUTL(p_msg->u_hdr.fei.flags);
        break;

    case MSG_OPER_PKT:
        MSG_ENCODE_PUTW(p_msg->u_hdr.pkt.ifindex);
        MSG_ENCODE_PUTW(p_msg->u_hdr.pkt.vid);
        MSG_ENCODE_PUTL(p_msg->u_hdr.pkt.type);
        MSG_ENCODE_PUTL(p_msg->u_hdr.pkt.sub_type);
        MSG_ENCODE_PUTL(p_msg->u_hdr.pkt.flags);
        break;

    case MSG_OPER_LCM:
        MSG_ENCODE_PUTL(p_msg->u_hdr.lcm.slot);
        break;

    case MSG_OPER_SUBSCRIBE:
        MSG_ENCODE_PUTC(p_msg->u_hdr.sub.type);
        MSG_ENCODE_PUTC(p_msg->u_hdr.sub.node_type);
        MSG_ENCODE_PUTW(p_msg->u_hdr.sub.tbl_id);
        MSG_ENCODE_PUTW(p_msg->u_hdr.sub.ds_id);
        MSG_ENCODE_PUTW(p_msg->u_hdr.sub.tbl_num);
        MSG_ENCODE_PUTL(p_msg->u_hdr.sub.field);
        MSG_ENCODE_PUTW(p_msg->u_hdr.sub.pid);
        MSG_ENCODE_PUTW(p_msg->u_hdr.sub.src);
        break;

    case MSG_OPER_SUB_REQ:
    case MSG_OPER_SUB_RESP:
        break;

    case MSG_OPER_BHM:
        MSG_ENCODE_PUTW(p_msg->u_hdr.bhm.src);
        MSG_ENCODE_PUTW(p_msg->u_hdr.bhm.pid);
        break;

    default:
        return -1;
    }
    
    /* build data */
    if (p_msg->data_len)
    {
        MSG_ENCODE_PUT(p_msg->data, p_msg->data_len);
    }

    /* build data1 */
    if (p_msg->data1_len)
    {
        MSG_ENCODE_PUT(p_msg->data1, p_msg->data1_len);
    }

    /* build data2 */
    if (p_msg->data2_len)
    {
        MSG_ENCODE_PUT(p_msg->data2, p_msg->data2_len);
    }

    ctc_msg_dump(p_msg, TRUE);

    return 0;
}

int32
ctc_sock_send_raw(int32 sock, uint32 peer, uint8 *p_data, uint32 length)
{
    ctc_sock_t *p_sock = NULL;
    ctc_sock_session_t *p_session = NULL;
    ctc_sock_queue_node_t *queue_node = NULL;
    uint8 *tx_buf = NULL;
    int32 rc = 0;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return -1;
    }
    p_sock = (ctc_sock_t*)p_session->p_sock;

    p_session->tx_msg_seq++;

    if (p_session->tx_resend_data)
    {
        queue_node = XMALLOC(MEM_LIB_SOCK_QUEUE, sizeof(ctc_sock_queue_node_t) + length);
        queue_node->data_len = length;
        sal_memcpy(queue_node->data, p_data, queue_node->data_len);
        ctclib_list_insert_tail(&p_session->tx_queue, &queue_node->node);
        p_session->tx_queue_cnt++;
        p_session->curr_tx_queue_cnt++;
        ctc_sock_session_resend(p_session);
        return 0;
    }
    else
    {
        if (glb_pack_en && p_sock->pack_en)
        {
            if (_ctc_sock_check_msg_pack_cond(p_session, length))
            {
                rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
                p_session->tx_seq++;
                if (CTC_SOCK_EAGAIN == rc)
                {
                    /* rc == CTC_SOCK_EAGAIN, means msg need resend/enqueue */
                    queue_node = XMALLOC(MEM_LIB_SOCK_QUEUE, sizeof(ctc_sock_queue_node_t) + length);
                    queue_node->data_len = length;
                    sal_memcpy(queue_node->data, p_data, queue_node->data_len);
                    ctclib_list_insert_tail(&p_session->tx_queue, &queue_node->node);
                    p_session->tx_queue_cnt++;
                    p_session->curr_tx_queue_cnt++;
                    ctc_sock_session_resend(p_session);
                    return 0;
                }
                else if (CTC_SOCK_ERROR == rc)
                {
                    return rc;
                }
                else
                {
                    /* 0 == rc, old tx_buf is sent */
                }
            }

            tx_buf = p_session->tx_buf + p_session->tx_buf_len;
            sal_memcpy(tx_buf, p_data, length);
            p_session->tx_buf_len += length;
            return 0;
        }
        else
        {
            sal_memcpy(p_session->tx_buf, p_data, length);
            p_session->tx_buf_len = length;

            rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
            p_session->tx_seq++;
            if (CTC_SOCK_EAGAIN == rc)
            {
                /* rc == CTC_SOCK_EAGAIN, means msg need resend/enqueue */
                rc = 0;
            }
            return rc;
        }
    }
}

uint32
ctc_sock_get_tx_error_count(int32 sock, uint32 peer)
{
    ctc_sock_session_t *p_session = NULL;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return 0;
    }

    return p_session->tx_error_cnt;
}

int32
ctc_sock_send(int32 sock, uint32 peer, ctc_msg_t *p_msg)
{
    ctc_sock_t *p_sock = NULL;
    ctc_sock_session_t *p_session = NULL;
    ctc_sock_queue_node_t *queue_node = NULL;
    uint8 *tx_buf = NULL;
    int32 left_len = 0;
    int32 rc = 0;

    p_session = _ctc_sock_get_session(sock, peer);
    if (!p_session)
    {
        return -1;
    }
    p_sock = (ctc_sock_t*)p_session->p_sock;

    CTC_SOCK_TX_LOCK(p_sock)
    
    if (MSG_OPER_CONNECT != p_msg->hdr.operation)
    {
        p_session->tx_msg_seq++;
    }
    p_msg->hdr.seq = p_session->tx_msg_seq;

    if (p_session->tx_resend_data)
    {
        queue_node = XMALLOC(MEM_LIB_SOCK_QUEUE, sizeof(ctc_sock_queue_node_t) + p_msg->hdr.length);
        queue_node->data_len = p_msg->hdr.length;
        rc = ctc_sock_send_encode(queue_node->data, queue_node->data_len, p_msg);
        if (rc < 0)
        {
            goto EXIT;
        }
        ctclib_list_insert_tail(&p_session->tx_queue, &queue_node->node);
        p_session->tx_queue_cnt++;
        p_session->curr_tx_queue_cnt++;
        ctc_sock_session_resend(p_session);
        rc = 0;
        goto EXIT;
    }
    else
    {
        if (glb_pack_en && p_sock->pack_en && (MSG_OPER_CONNECT != p_msg->hdr.operation))
        {
            if (_ctc_sock_check_msg_pack_cond(p_session, p_msg->hdr.length))
            {
                rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
                if (MSG_OPER_CONNECT != p_msg->hdr.operation)
                {
                    p_session->tx_seq++;
                }
                if (CTC_SOCK_EAGAIN == rc)
                {
                    /* rc == CTC_SOCK_EAGAIN, means msg need resend/enqueue */
                    queue_node = XMALLOC(MEM_LIB_SOCK_QUEUE, sizeof(ctc_sock_queue_node_t) + p_msg->hdr.length);
                    queue_node->data_len = p_msg->hdr.length;
                    rc = ctc_sock_send_encode(queue_node->data, queue_node->data_len, p_msg);
                    if (rc < 0)
                    {
                        goto EXIT;
                    }
                    ctclib_list_insert_tail(&p_session->tx_queue, &queue_node->node);
                    p_session->tx_queue_cnt++;
                    p_session->curr_tx_queue_cnt++;
                    ctc_sock_session_resend(p_session);
                    rc = 0;
                    goto EXIT;
                }
                else if (CTC_SOCK_ERROR == rc)
                {
                    goto EXIT;
                }
                else
                {
                    /* 0 == rc, old tx_buf is sent */
                }
            }

            tx_buf = p_session->tx_buf + p_session->tx_buf_len;
            left_len = CTC_SOCK_BUF_SIZE - p_session->tx_buf_len;
            rc = ctc_sock_send_encode(tx_buf, left_len, p_msg);
            if (rc < 0)
            {
                goto EXIT;
            }
            p_session->tx_buf_len += p_msg->hdr.length;
            rc = 0;
            goto EXIT;
        }
        else
        {
            rc = ctc_sock_send_encode(p_session->tx_buf, CTC_SOCK_BUF_SIZE, p_msg);
            if (rc < 0)
            {
                goto EXIT;
            }
            p_session->tx_buf_len = p_msg->hdr.length;

            rc = _ctc_sock_send(p_session, p_session->tx_buf, p_session->tx_buf_len, 0);
            if (MSG_OPER_CONNECT != p_msg->hdr.operation)
            {
                p_session->tx_seq++;
            }
            if (CTC_SOCK_EAGAIN == rc)
            {
                /* rc == CTC_SOCK_EAGAIN, means msg need resend/enqueue */
                rc = 0;
            }
            goto EXIT;
        }
    }
EXIT:
    CTC_SOCK_TX_UNLOCK(p_sock)
    return rc;
}

ctc_sock_t*
ctc_sock_get_sock(int32 sock)
{
    if (sock < 0)
    {
        return NULL;
    }
    if (sock > CTC_SOCK_MAX)
    {
        return NULL;
    }

    return g_p_sock_master->socks[sock];
}

ctc_sock_session_t*
ctc_sock_get_session_by_fd(int32 sock, int32 sess_fd)
{
    ctc_sock_t *p_sock = NULL;
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;

    if (sock < 0)
    {
        return NULL;
    }
    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock)
    {
        return NULL;
    }

    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        if (sess_fd == p_sess->fd)
        {
            return p_sess;
        }
    }

    return NULL;       
}

const char*
ctc_sock_domain_desc(int32 domain)
{
    switch (domain)
    {
    case AF_INET:
        return "INET";
    case AF_UNIX:
        return "UNIX";
    case AF_NETLINK:
        return "NETLINK";
    }
    return "Invalid";
}

const char*
ctc_sock_type_desc(int32 type)
{
    switch (type)
    {
    case SOCK_STREAM:
        return "STREAM";
    case SOCK_DGRAM:
        return "DGRAM";
    case SOCK_RAW:
        return "RAW";
    }
    return "Invalid";
}

int32
ctc_sock_dump(FILE *fp, uint32 detail)
{
    int32 tx_buf = 0;
    int32 rx_buf = 0;
    int32 i = 0;
    int32 index = 0;
    int32 session_count = 0;
    uint32 peer_pid = 0;
    uint32 peer_pm = 0;
    uint32 first = TRUE;
    ctc_sock_t *p_sock = NULL;
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;
    char en_str[32];
    
    sal_fprintf(fp, "Pck: Pack\n");
    sal_fprintf(fp, "Que: Queue\n");
    sal_fprintf(fp, "Srv: Server/Client\n");
    sal_fprintf(fp, "Mtx: Mutex\n");
    sal_fprintf(fp, "Imm: Immediate\n");
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "Socket Count: %d\n", g_p_sock_master->count);
    sal_fprintf(fp, "%-5s %-5s %-6s %-15s %-7s %-7s %-6s %-3s %-5s %-10s\n", 
        "Index", "Sock", "FD", "Pck/Que/Srv/Mtx", "Session", "Domain", "Type", "Imm", "Retry", "Description");
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");
    for (i = 0; i < CTC_SOCK_MAX; i++)
    {
        p_sock = g_p_sock_master->socks[i];
        if (p_sock)
        {
            sal_snprintf(en_str, 32, "%u/%u/%u/%u", p_sock->pack_en, p_sock->queue_en, p_sock->is_server, (p_sock->tx_mutex)? TRUE : FALSE);
            session_count = _ctc_sock_get_session_count(p_sock);
            sal_fprintf(fp, "%-5d %-5d %-6d %-15s %-7d %-7s %-6s %-3d %-5d %-10s\n", 
                index, p_sock->sock, p_sock->fd, en_str, session_count, 
                ctc_sock_domain_desc(p_sock->domain), ctc_sock_type_desc(p_sock->type), 
                p_sock->immediate,
                p_sock->connect_retry_count,
                p_sock->desc);
            index++;
        }
    }

    if (detail)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "%-4s %-4s %-11s %-5s %-8s %-8s %-10s %-10s %-8s\n", 
            "Sock", "FD", "Description", "SesFD", "peer_pid", "peer_pm", "tx_msg_seq", "rx_msg_seq", "tx_seq");
        sal_fprintf(fp, "--------------------------------------------------------------------------------\n");

        for (i = 0; i < CTC_SOCK_MAX; i++)
        {
            p_sock = g_p_sock_master->socks[i];
            if (p_sock)
            {
                first = TRUE;
                sal_fprintf(fp, "%-4d %-4d %-11s ", p_sock->sock, p_sock->fd, p_sock->desc);
                ctclib_list_for_each(p_node, &p_sock->session)
                {
                    p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
                    tx_buf = ctc_sock_get_buf(p_sess->fd, FALSE);
                    rx_buf = ctc_sock_get_buf(p_sess->fd, TRUE);
                    peer_pid = (p_sess->peer >> 16) & 0xFFFF;
                    peer_pm = (p_sess->peer & 0xFFFF);
                    ctc_sock_get_buf(p_sock->sock, rx_buf);
                    if (first)
                    {
                        sal_fprintf(fp, "%-5d %-8u %-8s %-10u %-10u %-8u\n", 
                            p_sess->fd, peer_pid, IPC_DEST_DESC(peer_pm), 
                            p_sess->tx_msg_seq, p_sess->rx_msg_seq, p_sess->tx_seq);
                        first = FALSE;
                    }
                    else
                    {
                        sal_fprintf(fp, "%-21s %-5d %-8u %-8s %-10u %-10u %-8u\n", "", 
                            p_sess->fd, peer_pid, IPC_DEST_DESC(peer_pm), 
                            p_sess->tx_msg_seq, p_sess->rx_msg_seq, p_sess->tx_seq);
                    }
                }

                if (first)
                {
                    sal_fprintf(fp, "\n");
                }
            }
        }

        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "%-5s %-10s %-10s %-10s %-10s %-10s %-10s %-8s %-10s %-10s %-10s\n", 
            "SesFD", "tx_bytes", "rx_bytes", "resend_cnt", "queue_cnt", "resend_len", "curr_queue", "mismatch", "TX Buf", "RX Buf", "TX Error");
        sal_fprintf(fp, "-----------------------------------------------------------------------------------------------------------------\n");

        for (i = 0; i < CTC_SOCK_MAX; i++)
        {
            p_sock = g_p_sock_master->socks[i];
            if (p_sock)
            {
                ctclib_list_for_each(p_node, &p_sock->session)
                {
                    p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
                    tx_buf = ctc_sock_get_buf(p_sess->fd, FALSE);
                    rx_buf = ctc_sock_get_buf(p_sess->fd, TRUE);
                    peer_pid = (p_sess->peer >> 16) & 0xFFFF;
                    peer_pm = (p_sess->peer & 0xFFFF);
                    p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
                    sal_fprintf(fp, "%-5d %-10u %-10u %-10u %-10u %-10u %-10u %-8u %-10u %-10u %-10u\n", 
                        p_sess->fd, p_sess->total_tx_bytes, p_sess->total_rx_bytes,
                        p_sess->tx_resend_cnt, p_sess->tx_queue_cnt,
                        p_sess->tx_resend_len, p_sess->curr_tx_queue_cnt, p_sess->seq_mismatch,
                        tx_buf, rx_buf, p_sess->tx_error_cnt);

                }
            }
        }
    }
    
    return 0;
}

int32
ctc_sock_session_set_buf(int32 sock, uint32 peer, uint32 is_rx, uint32 buf_size)
{
    ctc_sock_t *p_sock = NULL;
    int32 option_name = 0;
    int32 rc = 0;
    int32 size = buf_size;
    ctc_sock_session_t *p_sess = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock || p_sock->fd < 0) {
        return -1;
    }    

    p_sess = _ctc_sock_get_session(sock, peer);
    if (!p_sess)
    {
        return -1;
    }
    
    option_name = (is_rx) ? SO_RCVBUF : SO_SNDBUF;
    rc = sal_setsockopt(p_sess->fd, SOL_SOCKET, option_name, &size, sizeof(size));

    if (rc)
    {
        return -1;
    }
    
    return 0;
}

int32
ctc_sock_set_buf(int32 sock, uint32 is_rx, uint32 buf_size)
{
    ctc_sock_t *p_sock = NULL;
    int32 option_name = 0;
    int32 rc = 0;
    int32 size = buf_size;
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;

    p_sock = ctc_sock_get_sock(sock);
    if (!p_sock || p_sock->fd < 0) {
        return -1;
    }    

    option_name = (is_rx) ? SO_RCVBUF : SO_SNDBUF;

    rc |= sal_setsockopt(p_sock->fd, SOL_SOCKET, option_name, &size, sizeof(size));
    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        rc |= sal_setsockopt(p_sess->fd, SOL_SOCKET, option_name, &size, sizeof(size));
    }

    if (rc)
    {
        return -1;
    }
    
    return 0;
}

int32
ctc_sock_get_buf(int32 fd, uint32 is_rx)
{
    int32 rc = 0;
    int32 option_name = 0;
    int32 size = 0;
    socklen_t len = sizeof(int32);

    option_name = (is_rx) ? SO_RCVBUF : SO_SNDBUF;
    
    rc = sal_getsockopt(fd, SOL_SOCKET, option_name, &size, &len);
    if (rc < 0)
    {
        return -1;
    }

    return size;
}

int32
ctc_sock_set_queue_en(int32 sock, uint32 queue_en)
{
    ctc_sock_t *p_sock = NULL;
    
    p_sock = ctc_sock_get_sock(sock);
    if (NULL == p_sock)
    {
        return -1;
    }

    p_sock->queue_en = queue_en;

    return 0;
}

int32
ctc_sock_set_pack_en(int32 sock, uint32 pack_en)
{
    ctc_sock_t *p_sock = NULL;
    
    p_sock = ctc_sock_get_sock(sock);
    if (NULL == p_sock)
    {
        return -1;
    }

    p_sock->pack_en = pack_en;

    return 0;
}

int32
ctc_sock_get_txqueue_depth(int32 sock)
{
    ctc_sock_t *p_sock = NULL;
    ctclib_list_node_t *p_node = NULL;
    ctc_sock_session_t *p_sess = NULL;
    int32 queue_depth = 0;
        
    p_sock = ctc_sock_get_sock(sock);
    if (NULL == p_sock)
    {
        return -1;
    }

    if (!p_sock->queue_en)
    {
        return 0;
    }
    
    ctclib_list_for_each(p_node, &p_sock->session)
    {
        p_sess = ctclib_container_of(p_node, ctc_sock_session_t, node);
        queue_depth += p_sess->curr_tx_queue_cnt;
    }

    return queue_depth;
}

