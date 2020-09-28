
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "mlag.h"
#include "mlag_sock.h"
#include "mlag_api.h"
#include "mlag_decode.h"

/* Tries to open TCP Connection to MLAG Peer */
int32
mpn_sock_cb_connect(tbl_mlag_peer_t *peer)
{
  int32 rc = PM_E_NONE;
#if 0  
  pal_sock_handle_t sck_fd;
  union sockunion su;

  /* Obtain Socket FD */
  sck_fd = stream_sock_cb_get_fd (peer->sock_cb, &peer->su, nzg);
  if (sck_fd < 0)
    {
      MLAG_PEER_FSM_EVENT_ADD (nzg, peer, MPF_EVENT_TCP_CONN_FAIL);

      goto EXIT;
    }

  /* Set Socket options */
  ret = mpn_sock_set_opt (peer, sck_fd, PAL_TRUE);
  if (ret < 0)
    {
      MLAG_PEER_FSM_EVENT_ADD (nzg, peer, MPF_EVENT_TCP_CONN_FAIL);

      goto EXIT;
    }

  /* Prepare the Sock-Union struct to connect */
  pal_mem_cpy (&su, &peer->su, sizeof (union sockunion));

  switch (su.sa.sa_family)
    {
    case AF_INET:
      su.sin.sin_port = pal_hton16 (MLAG_PORT_DEFAULT);
      break;
    }

  /* Connect the Stream Socket */
  ret = stream_sock_cb_connect (peer->sock_cb, &su, nzg);
  if (ret < 0)
    {
      MLAG_PEER_FSM_EVENT_ADD (nzg, peer, MPF_EVENT_TCP_CONN_FAIL);

      goto EXIT;
    }

 EXIT:
#endif
    return rc;
}

uint32
mpn_timer_generate_jitter(uint32 time)
{
    uint32 jittered_time;
    uint32 rand_val;

    rand_val = 75 + (uint32)(25.0 * sal_rand()/RAND_MAX);

    jittered_time = (time * rand_val / 100);

    return (jittered_time > 1 ? jittered_time : 1);
}

static msg_dec_ret_t
_mpn_msg_decode_hdr(uint8 *data, uint32 length, mlag_decode_t *p_msg)
{
    msg_dec_ret_t ret;
    uint32  dec_length = length;
    uint8  *dec_data = data;
    uint32 *size = &dec_length;
    uint8 **ppnt = &dec_data;

    if (length < MLAG_HEADER_SIZE)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
        return ret;
    }

    MSG_DECODE_GETC(p_msg->version);
    MSG_DECODE_GETC(p_msg->type);
    MSG_DECODE_GETW(p_msg->length);
    p_msg->data = dec_data;
    p_msg->pnt = p_msg->data;
    p_msg->size_left = p_msg->length - MLAG_HEADER_SIZE;
    p_msg->p_size_left = &p_msg->size_left;

    if (p_msg->length > length)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
    }
    else
    {
        ret.consumed = p_msg->length;
        ret.code = MSG_DEC_OK;
    }

    return ret;
}

static int32
_mpn_sock_decode(void* p_sess, int32 read_len)
{
    ctc_sock_session_t* p_session = p_sess;
    int32 left = 0;
    mlag_decode_t msg;
    msg_dec_ret_t dec_rc;
    uint8* p = NULL;
    
    p_session->rx_buf_len += read_len;
    p = p_session->rx_buf;
    left = p_session->rx_buf_len;
    
    for (;;)
    {
        dec_rc = _mpn_msg_decode_hdr(p, left, &msg);
        switch (dec_rc.code)
        {
        case MSG_DEC_OK:
            p_session->rx_msg_seq++;
            mpd_msg_handle(p_session, &msg);
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
_mpn_sock_read(void* p_data)
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
    rc = _mpn_sock_decode(p_session, rc);
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

int32
mpn_sock_addr_check(addr_t *peer_addr, struct sal_sockaddr *remote_addr)
{
    struct sal_sockaddr_in4 *p_in_addr = NULL;

    p_in_addr = (struct sal_sockaddr_in4*)remote_addr;

    if (peer_addr->family != p_in_addr->sin_family)
    {
        return PM_E_FAIL;
    }

    switch (peer_addr->family)
    {
    case AF_INET:
        if (sal_memcmp(&peer_addr->u.prefix4, &p_in_addr->sin_addr, sizeof(addr_ipv4_t)))
        {
            return PM_E_FAIL;
        }
        break;
      
    default:
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

mlag_sock_connected_t
mpn_sock_check_connected(tbl_mlag_peer_t *peer)
{
    ctc_sock_t *p_server_sock = NULL;
    ctc_sock_t *p_client_sock = NULL;
    uint32 server_connected = FALSE;
    uint32 client_connected = FALSE;
    
    p_server_sock = ctc_sock_get_sock(peer->server_sock);
    if (p_server_sock)
    {
        if (!ctclib_list_empty(&p_server_sock->session))
        {
            server_connected = TRUE;
        }
    }
    p_client_sock = ctc_sock_get_sock(peer->client_sock);
    if (p_client_sock)
    {
        if (!ctclib_list_empty(&p_client_sock->session))
        {
            client_connected = TRUE;
        }
    }

    if (server_connected)
    {
        if (client_connected)
        {
            return MLAG_SOCK_CONNECTED_BOTH;
        }
        else
        {
            return MLAG_SOCK_CONNECTED_SERVER;
        }
    }
    else
    {
        if (client_connected)
        {
            return MLAG_SOCK_CONNECTED_CLIENT;
        }
        else
        {
            return MLAG_SOCK_CONNECTED_NONE;
        }
    }
    
    return PM_E_NONE;
}

static int32 
_mpn_sock_server_check_local_ip_address(addr_t *peer_addr)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode_addr = NULL;
    tbl_route_if_t *p_db_route_if = NULL;
    ds_connected_t *p_db_conn = NULL;
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_route_if, listnode)
    {
        if (NULL == p_db_route_if->ifc_ipv4.obj_list)
        {
            continue;
        }

        CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv4.obj_list, p_db_conn, listnode_addr)
        {
            if (0 == sal_memcmp(&p_db_conn->key.address.u.prefix4, &peer_addr->u.prefix4, sizeof(addr_ipv4_t)))
            {
                log_sys(M_MOD_INTERNAL, E_WARNING, "Server peer address is duplicated with local address, close this socket");
                return PM_E_FAIL;
            }
        }
    }
    
    return PM_E_NONE;
}

static int32 
_mpn_sock_server_accept_fn(int32 sock, ctc_sock_session_t *p_session, struct sal_sockaddr *p_addr)
{
    char expect_addr[CMD_BUF_32];
    char real_addr[CMD_BUF_32];
    struct sal_sockaddr_in4 *p_in_addr = NULL;
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    int32 rc = PM_E_NONE;

    p_in_addr = (struct sal_sockaddr_in4*)p_addr;

    cdb_addr_val2str(expect_addr, CMD_BUF_32, &peer->peer_addr);
    sal_inet_ntop(AF_INET, (void*)&p_in_addr->sin_addr, real_addr, CMD_BUF_32);

    //log_sys(M_MOD_INTERNAL, E_NOTICE, "Server accept expect %s, real %s", expect_addr, real_addr);
    if (!mlag_is_peer_ready())
    {
        //log_sys(M_MOD_INTERNAL, E_WARNING, "MLAG peer configuration is not ready, close this socket");
        _ctc_sock_session_close(p_session);
        return PM_E_NONE;
    }
    rc = mpn_sock_addr_check(&peer->peer_addr, p_addr);
    if (rc < 0)
    {
        log_sys(M_MOD_INTERNAL, E_WARNING, "Server accept address mismatch expect %s, real %s, close this socket", expect_addr, real_addr);
        _ctc_sock_session_close(p_session);
        return PM_E_NONE;        
    }
    rc = _mpn_sock_server_check_local_ip_address(&peer->peer_addr);
    if (rc < 0)
    {
        _ctc_sock_session_close(p_session);
        return PM_E_NONE;        
    }

    mpf_process_event(MPF_EVENT_TCP_CONN_CFM);
      
    return PM_E_NONE;
}

void 
_mpn_sock_conn_fail_fn(void *p_data)
{
    /* should be called in timer */
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    mlag_sock_connected_t connected;

    connected = mpn_sock_check_connected(peer);
    if (MLAG_SOCK_CONNECTED_NONE == connected)
    {
        /* no connection, should restart connection */
        mpf_process_event(MPF_EVENT_TCP_CONN_FAIL);
    }
}

static int32 
_mpn_sock_session_del_fn(ctc_sock_session_t* p_session)
{
    ctc_task_add_timer(TASK_PRI_NORMAL, _mpn_sock_conn_fail_fn, NULL, 0);
    return PM_E_NONE;
}

int32
mpn_sock_server_init()
{
    tbl_mlag_peer_t peer;
    tbl_mlag_peer_t *p_peer = &peer;
    tbl_mlag_peer_t *p_db_peer = tbl_mlag_peer_get_mlag_peer();
    char sock_addr[CMD_BUF_32];
    int32 sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;

    if (p_db_peer->server_sock > 0)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(p_peer, 0, sizeof(peer));

    sock = ctc_socket(CTC_SOCK_TCP, 0, "MlagServer");
    if (sock < 0)
    {
        p_peer->server_sock = INVALID_SOCK_ID;
        tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_SERVER_SOCK);
        return PM_E_INIT_FAIL;
    }
    ctc_sock_register_read_fn(sock, _mpn_sock_read);
    ctc_sock_register_accept_fn(sock, _mpn_sock_server_accept_fn);
    ctc_sock_register_sess_del_fn(sock, _mpn_sock_session_del_fn);
    sal_memset(sock_addr, 0, sizeof(sock_addr));
    sal_snprintf(sock_addr, CMD_BUF_32, "0.0.0.0:%u", MLAG_PORT_DEFAULT);
    ctc_sock_bind(sock, sock_addr);
    p_peer->server_sock = sock;
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_SERVER_SOCK);
    
    return rc;
}

int32
mpn_sock_server_deinit()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    if (INVALID_SOCK_ID == peer->server_sock)
    {
        return PM_E_NONE;
    }
    ctc_sock_close(peer->server_sock);
    peer->server_sock = INVALID_SOCK_ID;
    tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_SERVER_SOCK);

    return PM_E_NONE;
}

int32
mpn_sock_client_connect_retry_fn(int32 sock)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    uint32 sec = 0;
//    int32 retry_count = 0;
    
    sec = mpn_timer_generate_jitter(peer->v_connect);
//    retry_count = ctc_sock_get_connect_retry_count(sock);
    ctc_sock_set_connect_retry_interval(sock, MS_PER_SEC * sec);
//    log_sys(M_MOD_INTERNAL, E_NOTICE, "mpn_sock_client_connect_retry_fn %u times, waiting %u seconds", retry_count, sec);
    return PM_E_NONE;
}

static int32 
_mpn_sock_client_connected_fn(int32 sock)
{
    mpf_process_event(MPF_EVENT_TCP_CONN_CFM);
      
    return PM_E_NONE;
}

int32
mpn_sock_client_init()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    int32 sock = INVALID_SOCK_ID;

    if (peer->client_sock > 0)
    {
        return PM_E_FAIL;
    }

    sock = ctc_socket(CTC_SOCK_TCP, 0, "MlagClient");
    if (sock < 0)
    {
        peer->client_sock = INVALID_SOCK_ID;
        tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_CLIENT_SOCK);
        return PM_E_INIT_FAIL;
    }

    /* Added by kcao for bug 41084 2016-10-14, fix connect() blocking problem */
    ctc_socket_nonblocking(sock, TRUE);
    ctc_sock_register_connect_fn(sock, _mpn_sock_client_connected_fn);
    ctc_sock_register_read_fn(sock, _mpn_sock_read);
    ctc_sock_register_sess_del_fn(sock, _mpn_sock_session_del_fn);
    peer->client_sock = sock;
    tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_CLIENT_SOCK);
    ctc_sock_register_retry_connect_fn(sock, mpn_sock_client_connect_retry_fn);
    ctc_sock_set_send_connect_msg(sock, FALSE);
    
    return PM_E_NONE;
}

int32
mpn_sock_client_connect()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    char tmp[CMD_BUF_32];
    char sock_addr[CMD_BUF_32];

    sal_memset(sock_addr, 0, sizeof(sock_addr));
    cdb_addr_val2str(tmp, CMD_BUF_32, &peer->peer_addr);
    sal_snprintf(sock_addr, CMD_BUF_32, "%s:%u", tmp, MLAG_PORT_DEFAULT);
    ctc_sock_set_send_to(peer->client_sock, 500);
    ctc_sock_connect(peer->client_sock, sock_addr, 0);
    
    return PM_E_NONE;
}

static void
_mpn_sock_client_disconnect_fn()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    ctc_sock_disconnect(peer->client_sock);

    return;
}

int32
mpn_sock_client_disconnect()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    //log_sys(M_MOD_INTERNAL, E_NOTICE, "[FSM] mpn_sock_client_disconnect");
        
    /* stop connect timer */
    CTC_TASK_STOP_TIMER(peer->t_auto_start);
    ctc_task_add_timer(TASK_PRI_NORMAL, _mpn_sock_client_disconnect_fn, NULL, 0);
    return PM_E_NONE;
}

int32
mpn_sock_client_deinit()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    if (INVALID_SOCK_ID == peer->client_sock)
    {
        return PM_E_NONE;
    }
    ctc_sock_close(peer->client_sock);
    peer->client_sock = INVALID_SOCK_ID;
    tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_CLIENT_SOCK);
    
    return PM_E_NONE;
}

