#include "sal.h"
#include "sal_sock.h"
#include "sal_task.h"
#include <sys/time.h>
#include <pthread.h>

uint32 sal_eadp_debug = 0;

result_t
sal_sock_set_nonblocking (sal_sock_handle_t sock, int32 state)
{
    int8 val;
    int32 ret = 0;

    val = fcntl (sock, F_GETFL, 0);
    if (SAL_SOCK_ERROR != val)
    {
        ret = fcntl (sock, F_SETFL, (state ? val | O_NONBLOCK : val & (~O_NONBLOCK)));
        if(-1 == ret)
        {
            return errno;
        }
        return RESULT_OK;
    }
    else
    {
        return errno;
    }
}

sal_sock_handle_t s_sock = -1;
sal_sock_handle_t c_sock = -1;
unix_sock_client_t* g_sal_client_sock;
unix_sock_server_t* g_sal_server_sock;




int32
ctc_sal_sock_register_recv_cb(SCL_RECV_CB cb, bool b_server)
{
    if(b_server)
    {
        if(g_sal_server_sock)
        g_sal_server_sock->rcv_callback = cb;
    }
    else
    {
        if(g_sal_client_sock)
        g_sal_client_sock->rcv_callback = cb;
    }
    return SOCK_E_NONE;
}

void
server_send()
{
    uint8 buf[140] = {0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x8c,
                0x00,0x00,0x00,0x00,0x00,0x70,0x03,0x00,
                0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x3e,
                0x00,0x05,0x00,0x01,0x00,0xac,0x07,0xf0,
                0x00,0x00,0x40,0x47,0x00,0x40,0xa0,0x00,
                0x00,0x0a,0x00,0x0a,0x00,0x00,0x00,0x0c,
                0x80,0x08,0x00,0x0e,0x00,0x00,0x00,0x00,
                0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x01,
                0x08,0x00,0x45,0x00,0x00,0x3e,0x00,0x00,
                0x00,0x00,0x40,0x00,0x22,0xf5,0xc7,0xc7,
                0xc7,0x01,0xc8,0x01,0x01,0x01,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0x97,0x9c,0x0b,0x85};
    uint32 len = 140;

    sal_sock_server_send(buf,len);
}


void
sal_sock_swap32(uint32* data, uint32 len)
{
    uint32 cnt = 0;

    for (cnt = 0; cnt < len; cnt++)
    {
        data[cnt] = sal_ntohl(data[cnt]);
    }

    return;
}

int32 _sal_sock_client_reinit()
{
    int32 ret;
    int32 size;
    if(!g_sal_client_sock)
    {
        return -1;
    }

    if (NULL != g_sal_client_sock->recv_task)
    {
        sal_task_destroy(g_sal_client_sock->recv_task);
        g_sal_client_sock->recv_task = NULL;
    }

    if (g_sal_client_sock->sock > 0)
    {
        sal_close(g_sal_client_sock->sock);
        g_sal_client_sock->sock = -1;
    }

    g_sal_client_sock->sock = sal_socket(PF_UNIX, SOCK_STREAM, 0);
    if (g_sal_client_sock->sock  < 0) {
        CTC_SAL_DEBUG_OUT("hal socket create error");
        return -1;
    }

#define SOCKET_RCV_BUFF 32768 /*32k*/
    size = SOCKET_RCV_BUFF;
    ret = sal_setsockopt (g_sal_client_sock->sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if (ret < 0)
    {
        sal_close(g_sal_client_sock->sock);
        return -1;
    }

#define SOCKET_SEND_BUFF 32768 /*32k*/
    size = SOCKET_SEND_BUFF;
    ret = sal_setsockopt (g_sal_client_sock->sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size));
    if (ret < 0)
    {
        sal_close(g_sal_client_sock->sock);
        return -1;
    }

    g_sal_client_sock->socket_has_init = SAL_TRUE;
    return 0;
}


int32
ctc_sal_sock_server_send(uint8* p_req,uint32 req_len)
{
    int ret = 0;
    uint32 len = 0;
    uint8 *ptr= NULL;
    uint16 i = 0;
    uint8 *pnt= NULL;

    ptr = p_req;
    len = req_len;
#define SEND_CNT 20

     /*sal_eadp_printf(sal_eadp_debug,"buf is below:    lenth[%d]\n",req_len);*/
    sal_eadp_printf(sal_eadp_debug,"buf is below:    lenth[%d]\n",req_len);


    pnt = p_req;
    for(i=0;i<req_len;i++)
    {
        if(i%16==0)
        {
            sal_eadp_printf(sal_eadp_debug,"\n");
        }

        if(i%4==0)
        {
            sal_eadp_printf(sal_eadp_debug," ");
        }

        sal_eadp_printf(sal_eadp_debug,"%02x",*pnt);
        pnt++;
    }

    sal_eadp_printf(sal_eadp_debug,"\n");

    i = 0;
    while (i < SEND_CNT)
    {
        ret = sal_send(g_sal_server_sock->csock, p_req, req_len, 0);
         /*ret = write(g_sal_server_sock->csock, p_req, req_len);*/

        if (ret < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
            {
                sal_eadp_printf(sal_eadp_debug,"#1_errno [%d]    \n",errno);
                ctc_sal_task_sleep(10);
                i++;
                continue;
            }
            sal_eadp_printf(sal_eadp_debug,"#2_errno [%d]   \n",errno);
        }
        else if (ret == req_len)
        {
            sal_eadp_printf(sal_eadp_debug,"%ret == req_len! sal_send successful!\n");
            return SOCK_E_NONE;
        }
        else
        {
            ptr += ret;
            len -= ret;
        }

        i++;
    }

    sal_eadp_printf(sal_eadp_debug,"%ret =%d sal_send failed\n",ret);

    return SOCK_E_SEND_ERR;
}

void
sal_sock_server_recv()
{
    sal_msg_t *msg;
    unix_sock_server_t *p_sal_sock;
    int nbytes = 0;
    int ret = 0;
    uint8 *data;
    int left;
    uint8* pnt = NULL;
    uint8 i  = 0;

    p_sal_sock = g_sal_server_sock;

   /*read msg as max buf number, then process msg one by one*/
    /*process msg one by one*/
    while(1)
    {
WHILE1:
        nbytes = sal_sock_read(p_sal_sock->csock, p_sal_sock->buf + p_sal_sock->len,
            (MESSAGE_MAX_LEN - p_sal_sock->len));
        if (nbytes < 0) {
            switch (errno)
            {
                case EINTR:
                case EAGAIN:
                case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
                case EWOULDBLOCK:
#endif
                    ret = 0;
                    continue;
            }

            ret = -1;
            continue;
        }
        else if (nbytes == 0) {
            ret = -1;
            continue;
        }

        data = p_sal_sock->buf ;
        p_sal_sock->len += nbytes;
        left = p_sal_sock->len;

        sal_eadp_printf(sal_eadp_debug,"sal_sock_server_recv successful!\n");
        sal_eadp_printf(sal_eadp_debug,"buf is below:    lenth[%d]",nbytes);

        pnt = data;
        for(i=0;i<nbytes;i++)
        {
            if(i%16==0)
            {
                sal_eadp_printf(sal_eadp_debug,"\n",pnt);
            }

            if(i%4==0)
            {
                sal_eadp_printf(sal_eadp_debug," ");
            }
            sal_eadp_printf(sal_eadp_debug,"%02x",*pnt);
            pnt++;
        }

        sal_eadp_printf(sal_eadp_debug,"\n");

        while (1)
        {
            if (left < SAL_MSG_HDR_LEN)
            {
                if (left > 0)
                    sal_memmove(p_sal_sock->buf, data, left);
                p_sal_sock->len = left;
                ret = 0;
                goto WHILE1;
            }

            msg = (sal_msg_t *)data;

            if (msg->msg_len > MESSAGE_MAX_LEN)
            {
                ret = -1;
                goto WHILE1;
            }

            if (left < msg->msg_len)
            {
                if (left > 0)
                    sal_memmove(p_sal_sock->buf, data, left);
                p_sal_sock->len = left;
                ret = 0;
                goto WHILE1;
            }

            /*process msg*/
            if (p_sal_sock->rcv_callback)
            {
            sal_eadp_printf(sal_eadp_debug,"call p_sal_sock->rcv_callback!!!!!!\n");
                ret = p_sal_sock->rcv_callback(msg);
                if (ret)
                {
                    continue;
                }
            }

            data += msg->msg_len;
            left -= msg->msg_len;
        }
    }

    return;
}

void
client_send()
{
    uint8 buf[140] = {0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x8c,
                0x00,0x00,0x00,0x00,0x00,0x70,0x03,0x00,
                0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x3e,
                0x00,0x05,0x00,0x01,0x00,0xac,0x07,0xf0,
                0x00,0x00,0x40,0x47,0x00,0x40,0xa0,0x00,
                0x00,0x0a,0x00,0x0a,0x00,0x00,0x00,0x0c,
                0x80,0x08,0x00,0x0e,0x00,0x00,0x00,0x00,
                0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x01,
                0x08,0x00,0x45,0x00,0x00,0x3e,0x00,0x00,
                0x00,0x00,0x40,0x00,0x22,0xf5,0xc7,0xc7,
                0xc7,0x01,0xc8,0x01,0x01,0x01,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0xaa,0xdd,0xaa,0xaa,0xaa,0xdd,0xaa,0xaa,
                0x97,0x9c,0x0b,0x85};
    uint32 len = 140;

    sal_sock_client_send(buf,len);
}


int32
ctc_sal_sock_client_send(uint8* buf,uint32 length)
{
    int ret, len;
    uint8* ptr;

    if (!g_sal_client_sock->socket_has_init)
    {
         /*HAL_LOG_ALERT("hal socket not init" );*/
        return -1;
    }

    if (g_sal_client_sock->sock < 0)
        return -1;

    ptr = buf;
    len = length;

    ret = write(g_sal_client_sock->sock, ptr, len);
    if (ret < 0)
    {
        switch (errno)
        {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
            case EWOULDBLOCK:
#endif
            return -2;
         }

         /*HAL_LOG_ERR("hal_talk, write message error, ret = %d, errno = %s", ret, strerror(errno));*/
        /*if hal read a msg, in callback fun, send msg error, we can't free reinit socket here,
        in hal_client_read will reinit socket, in other case, reinit socket*/
        return -1;
    }
    else if (ret == len)
    {
        return 0;
    }
    else
    {
        len -= ret;
        ptr = ptr + ret;
        sal_memmove(buf, ptr, len);
        return 0;
    }

    return 0;
}

void
sal_sock_client_recv()
{
    sal_msg_t *msg;
    unix_sock_client_t *p_sal_sock;
    int nbytes = 0;
    int ret = 0;
    uint8 *data;
    int left;

    p_sal_sock = g_sal_client_sock;

   /*read msg as max buf number, then process msg one by one*/
    /*process msg one by one*/
    while(1)
    {
WHILE1:
        nbytes = sal_sock_read(p_sal_sock->sock, p_sal_sock->buf + p_sal_sock->len,
            (MESSAGE_MAX_LEN - p_sal_sock->len));
        if (nbytes < 0) {
            switch (errno)
            {
                case EINTR:
                case EAGAIN:
                case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
                case EWOULDBLOCK:
#endif
                    ret = 0;
                    continue;
            }

            ret = -1;
            _sal_sock_client_reinit();
            return;
        }
        else if (nbytes == 0) {
            ret = -1;
            _sal_sock_client_reinit();
            return;
        }

        data = p_sal_sock->buf ;
        p_sal_sock->len += nbytes;
        left = p_sal_sock->len;

        /*process msg one by one*/
        while (1)
        {
            if (left < SAL_MSG_HDR_LEN)
            {
                if (left > 0)
                    sal_memmove(p_sal_sock->buf, data, left);
                p_sal_sock->len = left;
                ret = 0;
                goto WHILE1;
            }

            sal_sock_swap32((uint32*)data,(SAL_MSG_HDR_LEN+SAL_GB_PKT_HDR_SIZE)/4);
            msg = (sal_msg_t *)data;

        sal_eadp_printf(sal_eadp_debug,"sal_sock_client_recv msg successful msg->type[%d]  len[%d]\n",msg->msg_type,msg->msg_len);

            if (msg->msg_len > MESSAGE_MAX_LEN)
            {
                ret = -1;
                p_sal_sock->len = 0;
            sal_eadp_printf(sal_eadp_debug,"msg->msg_len 0x%x  error!!! continue\n",msg->msg_len);
                _sal_sock_client_reinit();
                return;
            }

            if (left < msg->msg_len)
            {
                if (left > 0)
                    sal_memmove(p_sal_sock->buf, data, left);
                p_sal_sock->len = left;
                ret = 0;
                goto WHILE1;
            }

            /*process msg*/
            if (p_sal_sock->rcv_callback)
            {
                ret = p_sal_sock->rcv_callback(msg);
                if (ret)
                {
                    continue;
                }
            }

            data += msg->msg_len;
            left -= msg->msg_len;
        }
    }

    return;
}


STATIC void
_scl_sock_server_accept(void* param)
{
    unix_sock_server_t *sock_server = (unix_sock_server_t *)param;
    sal_sock_handle_t sock;
    sal_sock_handle_t csock;
    struct sal_sockaddr_un addr;
    sal_sock_len_t len;

    sock = s_sock;
    len = sizeof (addr);

    for (;;)
    {
        csock = sal_accept (sock, (struct sal_sockaddr *) &addr, (uint32*)&len);
        if (csock < 0)
        {
             /*CHIP_AGT_ERR(CHIP_AGT_STR "Accept error\n");*/
            ctc_sal_task_sleep(500);
            continue;
        }

        /* Make socket non-blocking.  */
        sal_sock_set_nonblocking (csock, SAL_TRUE);
        sock_server->csock = csock;
        g_sal_server_sock->csock = csock;
        sal_eadp_printf(sal_eadp_debug,"_scl_sock_server_accept sucessful!!! csock %d \n",csock);

        ctc_sal_task_create(&g_sal_server_sock->recv_task, "dma_sock_Acpt",
                        SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, sal_sock_server_recv, NULL);
        ctc_sal_task_sleep(2000);
    }

    return;
}


int32
ctc_sal_sock_server_init(char *user_path)
{
    struct sal_sockaddr_un st_addr;
    int32 ret  = 0;
    int32 flags = 1;

    char path[32] = {0};

    sal_memcpy(path, user_path, sizeof(path));

    /*1. malloc memory for hal server - hal message master structure*/
    g_sal_server_sock = sal_malloc(sizeof(unix_sock_server_t));
    sal_memset(g_sal_server_sock, 0, sizeof(unix_sock_server_t));

    /*2. Init socket*/
    unlink(path);
    s_sock = sal_socket(PF_UNIX, SOCK_STREAM, 0);
    if (s_sock < 0)
    {
        sal_eadp_printf(sal_eadp_debug,"%s  server create socket error\n",__FUNCTION__);
        return SOCK_E_CREATE_ERR;
    }

    g_sal_server_sock->sock = s_sock;

    sal_memset(&st_addr, 0, sizeof(struct sal_sockaddr_un));
    st_addr.sun_family = AF_UNIX;
    sal_strncpy(st_addr.sun_path, path, sizeof(st_addr.sun_path)-1);

    ret = sal_setsockopt(g_sal_server_sock->sock , SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));

    if (ret < 0)
    {
        return SOCK_E_SET_OPT_ERR;
    }

    if (sal_bind(s_sock, (struct sal_sockaddr *)&st_addr, sizeof(struct sal_sockaddr_un)) < 0)
    {
        close(s_sock);
        s_sock = -1;
        sal_eadp_printf(sal_eadp_debug,"%s server bind error\n",__FUNCTION__);
        return SOCK_E_BIND_ERR;
    }

    sal_sock_set_nonblocking(s_sock, SAL_TRUE);
    sal_listen(s_sock, 16);

    ctc_sal_task_create(&g_sal_server_sock->t_accept, "dma_sock_Acpt",
                          SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _scl_sock_server_accept, g_sal_server_sock);

    return SOCK_E_NONE;
}


STATIC void
_scl_sock_client_connect(void* param)
{
    struct sal_sockaddr_un addr;
    int32 ret;
    int32 cnt = 0;

    sal_memcpy(&addr,param,sizeof(struct sal_sockaddr_un));
    sal_free(param);
#define SOCKET_RECONN_CNT 20 /*2s*/

    while (1)
    {
        ret = sal_connect(g_sal_client_sock->sock, (struct sal_sockaddr *)&addr, sizeof(struct sal_sockaddr_un));
        if (ret < 0)
        {
            cnt++;
            /*print first and last log*/
            if ((1 == cnt) || (SOCKET_RECONN_CNT == cnt))
            {
                 /*HAL_LOG_DEBUG("hal client reconnect, cnt=%d", cnt);*/
            }
            ctc_sal_udelay(100000);
        }
        else
        {
            /*connect success, start read timer*/
            ctc_sal_task_create(&g_sal_client_sock->recv_task, "dma_sock_Acpt",
                              SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, sal_sock_client_recv, NULL);
        }
        ctc_sal_task_sleep(2000);
    }

    ctc_sal_print_func("sal_sock_client_init failed!!! server must init first!!! \n");
    return ;
}


int32 ctc_sal_sock_client_init(char *user_path)
{
    struct sal_sockaddr_un addr;
    struct sal_sockaddr_un* p_addr;
    int32 ret;
    int32 size;

    char path[32] = {0};
    sal_memcpy(path, user_path, sizeof(path));
    p_addr = sal_malloc(sizeof(struct sal_sockaddr_un));
    if (NULL == p_addr)
    {
        return -1;
    }

    sal_eadp_printf(sal_eadp_debug,"hal socket init");
    g_sal_client_sock = sal_malloc(sizeof(unix_sock_client_t));
    sal_memset(g_sal_client_sock, 0, sizeof(unix_sock_client_t));

    g_sal_client_sock->sock = sal_socket(PF_UNIX, SOCK_STREAM, 0);
    if (g_sal_client_sock->sock  < 0) {
        sal_eadp_printf(sal_eadp_debug,"hal socket create error");
        sal_free(p_addr);
        return -1;
    }

#define SOCKET_RCV_BUFF 32768 /*32k*/
    size = SOCKET_RCV_BUFF;
    ret = sal_setsockopt (g_sal_client_sock->sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if (ret < 0)
    {
        sal_close(g_sal_client_sock->sock);
        sal_free(p_addr);
        return -1;
    }

#define SOCKET_SEND_BUFF 32768 /*32k*/
    size = SOCKET_SEND_BUFF;
    ret = sal_setsockopt (g_sal_client_sock->sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size));
    if (ret < 0)
    {
        sal_close(g_sal_client_sock->sock);
        sal_free(p_addr);
        return -1;
    }

    g_sal_client_sock->socket_has_init = SAL_TRUE;

    sal_memset(&addr, 0, sizeof(struct sal_sockaddr_un));
    addr.sun_family = AF_UNIX;
    sal_strcpy(addr.sun_path, path);

    sal_sock_set_nonblocking(g_sal_client_sock->sock, SAL_TRUE);

    sal_memcpy(p_addr,&addr,sizeof(struct sal_sockaddr_un));
    ctc_sal_task_create(&g_sal_client_sock->t_connect, "dma_sock_Connect",
                      SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _scl_sock_client_connect, p_addr);

    if(p_addr)
    {
        sal_free(p_addr);
    }
    return 0;

}








