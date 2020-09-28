
#include "proto.h"
#include "route.h"
#include "lib_netlink.h"

/* same define in fpm.h */

#define FPM_PORT_DEFAULT            (2620)
#define RT_FPM_HEADER_SIZE          (4)

typedef struct 
{
	uint8 version;
	uint8 msg_type;
	uint16 msg_len;
} rt_fpm_msg_hdr_t;

int32 g_fpm_fd = -1;

typedef struct
{
    rt_fpm_msg_hdr_t        hdr;
    uint8                   *data;
} rt_fpm_decode_t;

static msg_dec_ret_t
_rt_fpm_msg_decode_hdr(uint8 *data, uint32 length, rt_fpm_decode_t *p_msg)
{
    msg_dec_ret_t ret;
    uint32  dec_length = length;
    uint8  *dec_data = data;
    uint32 *size = &dec_length;
    uint8 **ppnt = &dec_data;

    if (length < RT_FPM_HEADER_SIZE)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
        return ret;
    }

    MSG_DECODE_GETC(p_msg->hdr.version);
    MSG_DECODE_GETC(p_msg->hdr.msg_type);
    MSG_DECODE_GETW(p_msg->hdr.msg_len);

    p_msg->data = dec_data;
    if (p_msg->hdr.msg_len > length)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
    }
    else
    {
        ret.consumed = p_msg->hdr.msg_len;
        ret.code = MSG_DEC_OK;
    }

    return ret;
}

int32
rt_fpm_msg_handle(ctc_sock_session_t* p_session, rt_fpm_decode_t *p_msg)
{
    struct nlmsghdr *h = NULL;
    struct sal_sockaddr_nl *snl = NULL;
    
    h = (struct nlmsghdr *)p_msg->data;

    DEBUG_ROUTE_NETLINK("RX FPM msg (ver: %u, type: %u, len: %u), NL (len %u, type: %s)", 
        p_msg->hdr.version, p_msg->hdr.msg_type, p_msg->hdr.msg_len, h->nlmsg_len, netlink_msg_type_desc(h->nlmsg_type));

    switch (h->nlmsg_type)
    {
    case RTM_NEWLINK:
    case RTM_DELLINK:
        //netlink_link_change(snl, h);
        break;
    case RTM_NEWADDR:
        //return netlink_addr_process(snl, h);
        break;
    case RTM_DELADDR:
        //return netlink_addr_process(snl, h);
        break;
    case RTM_NEWROUTE:
        #ifdef USE_FPM_IPUC
        return netlink_route_process(snl, h);
        #endif
        break;
    case RTM_DELROUTE:
        #ifdef USE_FPM_IPUC
        return netlink_route_process(snl, h);
        #endif
        break;
    case RTM_NEWNEIGH:
        //rc = netlink_neigh_add_process(snl, h);
        break;
    case RTM_GETNEIGH:
        //rc =  netlink_neigh_get_process(snl, h);
        break;
    case RTM_DELNEIGH:
        //rc = netlink_neigh_del_process(snl, h);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

static int32
_rt_fpm_sock_decode(void* p_sess, int32 read_len)
{
    ctc_sock_session_t* p_session = p_sess;
    int32 left = 0;
    rt_fpm_decode_t msg;
    msg_dec_ret_t dec_rc;
    uint8* p = NULL;
    
    p_session->rx_buf_len += read_len;
    p = p_session->rx_buf;
    left = p_session->rx_buf_len;

    for (;;)
    {
        dec_rc = _rt_fpm_msg_decode_hdr(p, left, &msg);
        switch (dec_rc.code)
        {
        case MSG_DEC_OK:
            p_session->rx_msg_seq++;
            rt_fpm_msg_handle(p_session, &msg);
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left == 0)
            {
                p_session->rx_buf_len = 0;
                return 0;
            }
            break;

        case MSG_DEC_MORE:
            if (left > 0 && p_session->rx_buf != p)
            {
                sal_memmove(p_session->rx_buf, p, left);
            }

            p_session->rx_buf_len = left;
            return 0;

        case MSG_DEC_FAIL:
            return -1;
        }
    }
}

static void
_rt_fpm_sock_read(void* p_data)
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
    rc = _rt_fpm_sock_decode(p_session, rc);
    if (rc < 0)
    {
        goto ERROR;
    }
    return;
    
ERROR:
    _ctc_sock_session_close(p_session);
    
    return;
}

static int32 
_rt_fpm_sock_server_accept_fn(int32 sock, ctc_sock_session_t *p_session, struct sal_sockaddr *p_addr)
{
    char addr_str[CMD_BUF_64];
    struct sal_sockaddr_in4 *p_in_addr = NULL;

    p_in_addr = (struct sal_sockaddr_in4*)p_addr;
    sal_inet_ntop(AF_INET, (void*)&p_in_addr->sin_addr, addr_str, CMD_BUF_64);
    log_sys(M_MOD_LIB, E_NOTICE, "FPM server accept address %s:%u", addr_str, (int32)p_in_addr->sin_port);
    return PM_E_NONE;
}

static int32 
_rt_fpm_sock_session_del_fn(ctc_sock_session_t* p_session)
{
    log_sys(M_MOD_LIB, E_NOTICE, "FPM socket closed");
    return PM_E_NONE;
}

int32
rt_fpm_init()
{
    char sock_addr[CMD_BUF_32];
    int32 sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;

    sock = ctc_socket(CTC_SOCK_TCP, 0, "FPMServer");
    if (sock < 0)
    {
        g_fpm_fd = INVALID_SOCK_ID;
        return PM_E_INIT_FAIL;
    }
    ctc_sock_register_read_fn(sock, _rt_fpm_sock_read);
    ctc_sock_register_accept_fn(sock, _rt_fpm_sock_server_accept_fn);
    ctc_sock_register_sess_del_fn(sock, _rt_fpm_sock_session_del_fn);
    sal_memset(sock_addr, 0, sizeof(sock_addr));
    sal_snprintf(sock_addr, CMD_BUF_32, "0.0.0.0:%u", FPM_PORT_DEFAULT);
    ctc_sock_bind(sock, sock_addr);
    g_fpm_fd = sock;

    return rc;
}

