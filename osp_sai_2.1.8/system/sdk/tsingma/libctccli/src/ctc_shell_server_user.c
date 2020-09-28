#ifdef SDK_IN_USERMODE
#include "sal.h"
#include "ctc_shell.h"
#include "ctc_shell_server.h"
#include "ctc_cli.h"

STATIC int
ctc_vty_sendto(ctc_vti_t* vti, const char *szPtr, const int szPtr_len);
STATIC int
ctc_vty_send_quit(ctc_vti_t* vti);

static sal_sock_t ctc_master_cli_fd = -1;

STATIC ctc_vti_t* ctc_vty_lookup_by_pid_errno(unsigned int pid)
{
    if(g_ctc_vti->fd != pid)
    {
        g_ctc_vti->fd    = pid;
        g_ctc_vti->printf = ctc_vty_sendto;
        g_ctc_vti->quit   = ctc_vty_send_quit;
		g_ctc_vti->node   = CTC_SDK_MODE;
        ctc_vti_prompt(g_ctc_vti);
    }
    return g_ctc_vti;
}

STATIC int ctc_vty_send_quit(ctc_vti_t* vti)
{
	ctc_sdk_packet_t	packet;
	int					ret 	= -1;

	sal_memset(&packet,0,sizeof(packet));

	if(-1 == vti->fd)
	{
		return -1;
	}

	packet.hdr.msg_len		= sizeof(struct ctc_msg_hdr);
	packet.hdr.msg_type		= CTC_SDK_CMD_QUIT;
	packet.hdr.msg_flags	= 0;
	packet.hdr.msg_turnsize	= 0;
	packet.hdr.msg_pid		= 0;

	ret = sal_send(vti->fd, &packet, 2048,0);

	sal_close(vti->fd);

	vti->fd = -1;

	return ret;
}


STATIC int ctc_vty_sendto(ctc_vti_t* vti, const char *szPtr, const int szPtr_len)
{
	ctc_sdk_packet_t	packet;
	int					ret 	= -1;

	sal_memset(&packet,0,sizeof(packet));

	if(-1 == vti->fd)
	{
		return -1;
	}

	packet.hdr.msg_len		= sizeof(struct ctc_msg_hdr) + szPtr_len;
	packet.hdr.msg_type		= 0;
	packet.hdr.msg_flags	= 0;
	packet.hdr.msg_turnsize	= szPtr_len;
	packet.hdr.msg_pid		= 0;

	sal_memcpy(&packet.msg, szPtr,szPtr_len);
	ret = sal_send(vti->fd, &packet, 2048,0);

	return ret;
}

STATIC void ctc_vty_recv_thread(void *arg)
{
    struct  ctc_msg_hdr *msgh 		= NULL;
	sal_sock_t			client_fd 	= (intptr)arg;
	ctc_sdk_packet_t	packet;
	int32				len;

	sal_memset(&packet,0,sizeof(packet));

	while(len = sal_recv(client_fd,&packet,sizeof(packet),0), len > 0)
	{
		msgh = &packet.hdr;
		if((msgh->msg_len >= sizeof(struct ctc_msg_hdr))
            && (len >= msgh->msg_len)
            && (0 == msgh->msg_flags)
            && (msgh->msg_type <= CTC_SDK_CMD_QUIT))
        {

            ctc_vti_read_cmd(ctc_vty_lookup_by_pid_errno(client_fd),
                                 (int8*)packet.msg,
                                 msgh->msg_turnsize);
        }
		else
		{
			ctc_cli_out("data receive from pid is:%d\n",msgh->msg_pid);
		}

		sal_memset(&packet,0,sizeof(packet));
	}

	return ;
}

int ctc_vty_socket()
{
	sal_sock_t				tmp_fd 		= -1;
	intptr				client_fd 	= -1;
	struct sal_sockaddr_in 	serv_addr;
	struct sal_sockaddr_in 	client_addr;
	sal_socklen_t			sock_len;
	int						reusraddr   = 1;
    char                    prompt[32] = "";

	tmp_fd = sal_socket(AF_INET,SOCK_STREAM,0);

	if(tmp_fd < 0)
	{
		perror("Socket create failed.");
		return -1;
	}

    sal_memset(&serv_addr, 0, sizeof(serv_addr));
    sal_memset(&client_addr, 0, sizeof(client_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(CTC_SDK_TCP_PORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(setsockopt(tmp_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&reusraddr,sizeof(reusraddr)) < 0)
	{
		perror("Setsockopt SO_REUSEADDR failed");
		sal_close(tmp_fd);
		return -1;
	}

	if(sal_bind(tmp_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
	    snprintf(prompt,sizeof(prompt),"TCP port(%d) bind failed",CTC_SDK_TCP_PORT);
		perror(prompt);
		sal_close(tmp_fd);
		return -1;
	}

	listen(tmp_fd, 1);

	ctc_master_cli_fd = tmp_fd;

    ctc_cli_out("Server is up and running ...\n");

	sock_len = sizeof(client_addr);
	while(client_fd = sal_accept(ctc_master_cli_fd,(struct sockaddr*)&client_addr,&sock_len), client_fd >= 0)
	{
             static sal_task_t* ctc_sdk_client_fd = NULL;
             if (ctc_sdk_client_fd)
             {
                 sal_task_destroy(ctc_sdk_client_fd);
             }

		if (0 != sal_task_create(&ctc_sdk_client_fd,
								 "ctc_sdk_server",
								 SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, ctc_vty_recv_thread, (void*)client_fd))
		{
			sal_close(tmp_fd);
			sal_task_destroy(ctc_sdk_client_fd);
			return -1;
		}

	}

	return 0;
}

void ctc_vty_close()
{
	sal_close(ctc_master_cli_fd);
	ctc_master_cli_fd = -1;
}
#endif
