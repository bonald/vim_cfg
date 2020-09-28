#ifdef SDK_IN_KERNEL

#include <linux/init.h>
#include <asm/types.h>
#include <asm/atomic.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/audit.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/inotify.h>
#include <linux/freezer.h>
#include <linux/tty.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include "ctc_shell.h"
#include "ctc_shell_server.h"
#include "ctc_cli.h"

STATIC int
ctc_vty_sendto(ctc_vti_t* vti, const char *szPtr, const int szPtr_len);
STATIC int
ctc_vty_send_quit(ctc_vti_t* vti);

static struct sock *ctc_master_cli_netlinkfd = NULL;

#if 0
LIST_HEAD(ctc_master_cli_vty_list);

typedef struct ctc_master_vty_item_s
{
    struct list_head    list;
    ctc_vti_t           *pvty;
}ctc_master_vty_item_t;
#endif
STATIC ctc_vti_t* ctc_vty_lookup_by_pid_errno(unsigned int pid)
{
#if 0
    ctc_master_vty_item_t   *pitem = NULL;
    list_for_each_entry(pitem,&ctc_master_cli_vty_list,list)
    {
        if(pitem->pvty->pid == pid)
        {
            return pitem->pvty;
        }
    }

    pitem = kmalloc(sizeof(ctc_master_vty_item_t),GFP_KERNEL);

    if(pitem)
    {
        pitem->pvty = ctc_vti_create(CTC_SDK_MODE);
        pitem->pvty->pid    = pid;
        pitem->pvty->printf = ctc_vty_sendto;
        ctc_vti_prompt(pitem->pvty);
    }

    return pitem->pvty;
#endif
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
    int size;
    struct sk_buff *skb;
    sk_buff_data_t old_tail;
    struct nlmsghdr *nlh;

    int retval;

    size = NLMSG_SPACE(CTC_SDK_NETLINK_MSG_LEN);
    skb =  alloc_skb(size, GFP_KERNEL);

	old_tail = skb->tail;
    nlh = nlmsg_put(skb, 0, 0, CTC_SDK_CMD_QUIT, NLMSG_SPACE(0), 0);
    old_tail = skb->tail;

    nlh->nlmsg_len  = skb->tail - old_tail;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,10))
    NETLINK_CB(skb).portid = 0;
#else
    NETLINK_CB(skb).pid = 0;
#endif
    NETLINK_CB(skb).dst_group = 0;

    retval = netlink_unicast(ctc_master_cli_netlinkfd, skb, vti->fd, MSG_DONTWAIT);

    if(retval < 0)
    {
        ctc_cli_out(KERN_DEBUG "%s:%d netlink_unicast return: %d\n", __FUNCTION__,__LINE__,retval);
    }

    return retval;
}


STATIC int ctc_vty_sendto(ctc_vti_t* vti, const char *szPtr, const int szPtr_len)
{
    int size;
    struct sk_buff *skb;
    sk_buff_data_t old_tail;
    struct nlmsghdr *nlh;

    int retval;

    if(0 == szPtr_len)
    {
        return 0;
    }

    size = NLMSG_SPACE(CTC_SDK_NETLINK_MSG_LEN);
    skb =  alloc_skb(size, GFP_KERNEL);

	old_tail = skb->tail;
    nlh = nlmsg_put(skb, 0, 0, 0, NLMSG_SPACE(strlen(szPtr) + 1) - sizeof(struct nlmsghdr), 0);

    old_tail = skb->tail;

    memcpy(NLMSG_DATA(nlh), szPtr, strlen(szPtr) + 1);

    nlh->nlmsg_len  = skb->tail - old_tail;
    nlh->nlmsg_seq  = szPtr_len;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,10))
    NETLINK_CB(skb).portid = 0;
#else
    NETLINK_CB(skb).pid = 0;
#endif
    NETLINK_CB(skb).dst_group = 0;

    retval = netlink_unicast(ctc_master_cli_netlinkfd, skb, vti->fd, 0);

    if(retval < 0)
    {
        ctc_cli_out(KERN_DEBUG "%s:%d netlink_unicast return: %d\n", __FUNCTION__,__LINE__,retval);
    }

    return retval;
}


STATIC void ctc_vty_recvfrom(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;

    if(skb->len >= sizeof(struct nlmsghdr))
    {
        nlh = (struct nlmsghdr *)skb->data;
        if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
            && (skb->len >= nlh->nlmsg_len))
        {

            ctc_vti_read_cmd(ctc_vty_lookup_by_pid_errno(nlh->nlmsg_pid),
                                 (char *)NLMSG_DATA(nlh),
                                 nlh->nlmsg_seq);
        }
    }
    else
    {
        ctc_cli_out("%s:%d receive error\n",__FUNCTION__,__LINE__);
    }
}


int ctc_vty_socket()
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,10))
    struct netlink_kernel_cfg cfg;
    cfg.groups = 0;
    cfg.flags = 0;
    cfg.input = ctc_vty_recvfrom;
    cfg.cb_mutex = NULL;
    cfg.bind = NULL;
    cfg.unbind = NULL;
    cfg.compare = NULL;
    ctc_master_cli_netlinkfd = netlink_kernel_create(&init_net, CTC_SDK_NETLINK, &cfg);
#else
    ctc_master_cli_netlinkfd = netlink_kernel_create(&init_net, CTC_SDK_NETLINK, 0, ctc_vty_recvfrom, NULL, THIS_MODULE);
#endif
    if(!ctc_master_cli_netlinkfd){
        ctc_cli_out("%s:%d can't create a netlink socket\n",__FUNCTION__,__LINE__);
        return -1;
    }

    return 0;
}

void ctc_vty_close()
{
    sock_release(ctc_master_cli_netlinkfd->sk_socket);
}
#endif
