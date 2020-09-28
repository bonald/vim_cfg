/**
 @file ctc_master_cli.h

 @date 2014-12-22

 @version v2.0

 This file define the types used in APIs

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h>
#include <string.h>
#include "ctc_shell.h"
#include <errno.h>
/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
#include <arpa/inet.h>

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
typedef struct ctc_sdk_vty_base_s ctc_sdk_vty_base_t;

typedef int (*ctc_socket)(ctc_sdk_vty_base_t *);
typedef int (*ctc_recvfrom)(ctc_sdk_vty_base_t *);
typedef int (*ctc_sendto)(ctc_sdk_vty_base_t *, char *, const int);
typedef int (*ctc_close)(ctc_sdk_vty_base_t *);
typedef int (*ctc_start_thread)(ctc_sdk_vty_base_t *);

struct ctc_sdk_vty_base_s
{
    pthread_t           task_id;
    int                 socket_fd;
    ctc_sdk_packet_t    socket_recv_buf;
    ctc_sdk_packet_t    socket_send_buf;
    ctc_socket          socket;
    ctc_sendto          sendto;
    ctc_recvfrom        recvfrom;
    ctc_close           close;
    ctc_start_thread    start_thread;
};

ctc_sdk_vty_base_t  *p_gctc_sdk_vty = NULL;
int g_socket_valid = 0;

static int ctc_sdk_start_thread(ctc_sdk_vty_base_t *pctc_sdk_vty);


struct termios termios_old;
void
set_terminal_raw_mode(void)
{
    /*system("stty raw -echo");*/
    struct termios terminal_new;
    tcgetattr(0, &terminal_new);
    memcpy(&termios_old, &terminal_new, sizeof(struct termios));
    terminal_new.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                              | INLCR | IGNCR | ICRNL | IXON);
    /*
      OPOST (output post-processing) & ISIG (Input character signal generating enabled) need to be set
      terminal_new.c_oflag &= ~OPOST;
      terminal_new.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
      */
    terminal_new.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    terminal_new.c_cflag &= ~(CSIZE | PARENB);
    terminal_new.c_cflag |= CS8;

    tcsetattr(0, TCSANOW, &terminal_new);
}

void
restore_terminal_mode(void)
{
    /*system("stty cooked echo");*/
    tcsetattr(0, TCSANOW, &termios_old);
    printf("\n");
}


static int ctc_socket_close(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
/*
    if(pctc_sdk_vty->task_id)
    {
        pthread_cancel(pctc_sdk_vty->task_id);
        pctc_sdk_vty->task_id = -1;
    }
*/
    if(-1 != pctc_sdk_vty->socket_fd)
    {
        g_socket_valid = 0;
        close(pctc_sdk_vty->socket_fd);
        pctc_sdk_vty->socket_fd = -1;
    }
    return 0;
}

#define ________NETLINK________
typedef struct ctc_sdk_vty_netlink_s
{
    ctc_sdk_vty_base_t          base;
    struct sockaddr_nl          kpeer;
    int                         kpeer_len;
}ctc_sdk_vty_netlink_t;

static int ctc_netlink_create_socket(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
    int                     skfd = -1;
    struct sockaddr_nl      local;
    ctc_sdk_vty_netlink_t *pvty_netlink = (ctc_sdk_vty_netlink_t*)pctc_sdk_vty;
    skfd = socket(PF_NETLINK, SOCK_RAW, CTC_SDK_NETLINK);
    if(skfd < 0){
        perror("can't create socket:");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid 	= getpid();
    local.nl_groups = 0;

    if(bind(skfd, (struct sockaddr *)&local, sizeof(local)) != 0){
        perror("bind() error:");
        return -1;
    }

    memset(&pctc_sdk_vty->socket_send_buf, 0, sizeof(ctc_sdk_packet_t));
    pctc_sdk_vty->socket_send_buf.hdr.msg_len       = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_flags     = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_type      = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_turnsize  = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_pid       = local.nl_pid;

    pctc_sdk_vty->socket_fd = skfd;
    g_socket_valid = 1;
    pvty_netlink->kpeer.nl_family   = AF_NETLINK;
    pvty_netlink->kpeer.nl_pid      = 0;
    pvty_netlink->kpeer.nl_groups   = 0;

    pvty_netlink->kpeer_len = sizeof(struct sockaddr_nl);

    return 0;
}

static int ctc_netlink_sendto(ctc_sdk_vty_base_t *pctc_sdk_vty, char *send_buf, const int send_buf_count)
{
    int ret = -1;
    ctc_sdk_vty_netlink_t   *pvty_netlink = (ctc_sdk_vty_netlink_t*)pctc_sdk_vty;

    if (g_socket_valid)
    {
        memcpy(NLMSG_DATA(&pctc_sdk_vty->socket_send_buf), send_buf, send_buf_count);
        pctc_sdk_vty->socket_send_buf.hdr.msg_len       = NLMSG_SPACE(send_buf_count);
        pctc_sdk_vty->socket_send_buf.hdr.msg_turnsize  = send_buf_count;

        ret = sendto(pctc_sdk_vty->socket_fd,
                    &pctc_sdk_vty->socket_send_buf,
                    pctc_sdk_vty->socket_send_buf.hdr.msg_len,
                    0,(struct sockaddr *)&pvty_netlink->kpeer,
                    pvty_netlink->kpeer_len);
        if(ret < 0){
            perror("sendto kernel:");
        }

    }

    return ret;
}

static int ctc_netlink_recvfrom(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
    int read_size = -1;
 /*  ctc_sdk_vty_netlink_t *pvty_netlink = (ctc_sdk_vty_netlink_t*)pctc_sdk_vty;*/
    struct sockaddr_nl kpeer;

    socklen_t kpeerlen = sizeof(struct sockaddr_nl);

    read_size = recvfrom(pctc_sdk_vty->socket_fd, &pctc_sdk_vty->socket_recv_buf,
                    sizeof(pctc_sdk_vty->socket_recv_buf),0,
                    (struct sockaddr*)&kpeer, &kpeerlen);

    if(read_size < 0)
    {
        perror("ctc_netlink_recvfrom error:");
    }

    return read_size;
}

static ctc_sdk_vty_base_t* ctc_sdk_alloc_netlink()
{
    ctc_sdk_vty_base_t	*tmp_ctc_sdk_netlink = NULL;
    tmp_ctc_sdk_netlink = (ctc_sdk_vty_base_t *)malloc(sizeof(ctc_sdk_vty_netlink_t));
    if(!tmp_ctc_sdk_netlink)
    {
        return NULL;
    }

    memset(tmp_ctc_sdk_netlink,0,sizeof(ctc_sdk_vty_netlink_t));

    tmp_ctc_sdk_netlink->socket = ctc_netlink_create_socket;
    tmp_ctc_sdk_netlink->sendto = ctc_netlink_sendto;
    tmp_ctc_sdk_netlink->recvfrom = ctc_netlink_recvfrom;
    tmp_ctc_sdk_netlink->close    = ctc_socket_close;
    tmp_ctc_sdk_netlink->start_thread = ctc_sdk_start_thread;

    return tmp_ctc_sdk_netlink;
}

#define ________TCP_IP________
typedef struct ctc_sdk_vty_tcp_s
{
    ctc_sdk_vty_base_t      base;
}ctc_sdk_vty_tcp_t;

static int ctc_tcp_create_socket(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
    int                     skfd = -1;
    struct sockaddr_in      serv_addr;
 /*  ctc_sdk_vty_tcp_t       *pvty_tcp = (ctc_sdk_vty_tcp_t*)pctc_sdk_vty;*/
    skfd = socket(AF_INET, SOCK_STREAM, 0);

    if(skfd < 0){
        perror("can't create socket:");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(CTC_SDK_TCP_PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(skfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect error:");
        return -1;
    }

    memset(&pctc_sdk_vty->socket_send_buf, 0, sizeof(ctc_sdk_packet_t));
    pctc_sdk_vty->socket_send_buf.hdr.msg_len       = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_flags     = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_type      = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_turnsize  = 0;
    pctc_sdk_vty->socket_send_buf.hdr.msg_pid       = getpid();

    pctc_sdk_vty->socket_fd = skfd;
    g_socket_valid = 1;

    return 0;
}

static int ctc_tcp_sendto(ctc_sdk_vty_base_t *pctc_sdk_vty, char *send_buf, const int send_buf_count)
{
    int ret = -1;

    if (g_socket_valid)
    {
        memcpy(&pctc_sdk_vty->socket_send_buf.msg, send_buf, send_buf_count);
        pctc_sdk_vty->socket_send_buf.hdr.msg_len       = sizeof(struct ctc_msg_hdr ) + send_buf_count;
        pctc_sdk_vty->socket_send_buf.hdr.msg_turnsize  = send_buf_count;

        ret = send(pctc_sdk_vty->socket_fd,
                        &pctc_sdk_vty->socket_send_buf,
                        pctc_sdk_vty->socket_send_buf.hdr.msg_len,0);
        if(ret < 0){
            perror("send pid:");
        }
    }

    return ret;
}

static int ctc_tcp_recvfrom(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
    int read_size = 0;
    int total_len = 0;

    do {
        read_size = recv(pctc_sdk_vty->socket_fd,
                            ((unsigned char *)&pctc_sdk_vty->socket_recv_buf) + read_size,
                            2048 - total_len,0);
        if ((read_size <= 0) && (errno == EINTR))
        {
            continue;
        }
        else if (read_size <= 0)
        {
            return read_size;
        }

        total_len += read_size;
    } while (total_len != 2048);

    return total_len;
}

static ctc_sdk_vty_base_t* ctc_sdk_alloc_tcp()
{
    ctc_sdk_vty_base_t *tmp_ctc_sdk_tcp = NULL;
    tmp_ctc_sdk_tcp = (ctc_sdk_vty_base_t *)malloc(sizeof(ctc_sdk_vty_tcp_t));
    if(!tmp_ctc_sdk_tcp)
    {
        return NULL;
    }

    memset(tmp_ctc_sdk_tcp,0,sizeof(ctc_sdk_vty_tcp_t));

    tmp_ctc_sdk_tcp->socket = ctc_tcp_create_socket;
    tmp_ctc_sdk_tcp->sendto = ctc_tcp_sendto;
    tmp_ctc_sdk_tcp->recvfrom = ctc_tcp_recvfrom;
    tmp_ctc_sdk_tcp->close    = ctc_socket_close;
    tmp_ctc_sdk_tcp->start_thread = ctc_sdk_start_thread;

    return tmp_ctc_sdk_tcp;
}


static void sig_handle (int signo)
{
  restore_terminal_mode();
  exit(0);
}

void set_signal()
{
    signal(SIGHUP, sig_handle);
    signal(SIGUSR1,sig_handle);
    signal(SIGINT, sig_handle);
    signal(SIGTERM,sig_handle);
}

static void* ctc_socket_recv_print_thread(void* arg)
{
    ctc_sdk_vty_base_t *pctc_sdk_vty = NULL;
    int  recv_size      = 0;

    pctc_sdk_vty = (ctc_sdk_vty_base_t*)arg;
    while(1)
    {
        recv_size = pctc_sdk_vty->recvfrom(pctc_sdk_vty);

        if(recv_size <= 0)
        {
            if (g_socket_valid)
            {
                restore_terminal_mode();
                pctc_sdk_vty->close(pctc_sdk_vty);
            }
            pthread_exit(0);
        }

        if(CTC_SDK_CMD_QUIT == pctc_sdk_vty->socket_recv_buf.hdr.msg_type)
        {
            restore_terminal_mode();
            pctc_sdk_vty->close(pctc_sdk_vty);
            pthread_exit(0);
        }
        write(STDOUT_FILENO,pctc_sdk_vty->socket_recv_buf.msg,
                    pctc_sdk_vty->socket_recv_buf.hdr.msg_turnsize);
        fflush(stdout);

        memset(&pctc_sdk_vty->socket_recv_buf,
                    0,
                    sizeof(pctc_sdk_vty->socket_recv_buf));
    }

    return NULL;
}

static int ctc_sdk_start_thread(ctc_sdk_vty_base_t *pctc_sdk_vty)
{
    return pthread_create(&pctc_sdk_vty->task_id, NULL, ctc_socket_recv_print_thread, (void*)pctc_sdk_vty);
}

int main(int argc, char* argv[])
{
    int nbytes = 1;
    unsigned char buf[1536] = {' '};

    if(1 != argc)
    {
        p_gctc_sdk_vty = ctc_sdk_alloc_netlink();
    }
    else
    {
        p_gctc_sdk_vty = ctc_sdk_alloc_tcp();
    }

    if(!p_gctc_sdk_vty)
    {
        printf("Not Support\n");
        return 0;
    }

    if(p_gctc_sdk_vty->socket(p_gctc_sdk_vty))
    {
        return 0;
    }

    if(p_gctc_sdk_vty->start_thread(p_gctc_sdk_vty))
    {
        p_gctc_sdk_vty->close(p_gctc_sdk_vty);
        return 0;
    }

    if(p_gctc_sdk_vty->sendto(p_gctc_sdk_vty,(char*)buf,nbytes) <= 0)
    {
        p_gctc_sdk_vty->close(p_gctc_sdk_vty);
        return 0;
    }

    set_signal();

    set_terminal_raw_mode();

    while(1)
    {
        nbytes = read(STDIN_FILENO, buf, sizeof(buf));

        if(p_gctc_sdk_vty->sendto(p_gctc_sdk_vty,(char*)buf,nbytes) <= 0)
        {
            break;
        }
    }

    restore_terminal_mode();

    p_gctc_sdk_vty->close(p_gctc_sdk_vty);
    free(p_gctc_sdk_vty);

    return 0;
}

